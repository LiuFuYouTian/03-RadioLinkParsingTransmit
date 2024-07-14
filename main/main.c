#include <stdio.h>
#include <esp_sbus_parsing.h>
#include <espnow_transmit.h>
#include "nvs_flash.h"
#include "nvs.h"

SemaphoreHandle_t xMutex = NULL;

void app_sbus_test(uint8_t * byte,uint8_t len)
{
    if(byte[0] != 0x0F || len != 25) return;
    if(byte[22] != 0x80) return;

    // 获取互斥信号量
    // if(xSemaphoreTake(xMutex,20) == pdTRUE)
    // {
        espnow_transmit_send(byte,len);
    // }
    // printf("SBUS_CHANNEL_00 = %d\r\n",esp_sbus_channel_data_parsing(byte,SBUS_CHANNEL_00));
}

void app_espnow_rx_test(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{
    // printf("espnow_rx_src_mac:");
    // for (size_t i = 0; i < ESP_NOW_ETH_ALEN; i++)
    // {
    //     printf("0x%02X ",recv_info->src_addr[i]);
    // }
    // printf("\r\n");

    // printf("espnow_rx_des_mac:");
    // for (size_t i = 0; i < ESP_NOW_ETH_ALEN; i++)
    // {
    //     printf("0x%02X ",recv_info->des_addr[i]);
    // }
    // printf("\r\n");

    // printf("espnow_rx_data:");
    // for (size_t i = 0; i < len; i++)
    // {
    //     printf("0x%02X ",data[i]);
    // }
    // printf("\r\n");

    printf("espnow_rx_data:");
    for (size_t i = 0; i < SBUS_CHANNEL_MAX; i++)
    {
        printf("%d,",esp_sbus_channel_data_parsing(data,i));
    }
    printf("\r\n");
}

void app_espnow_tx_test(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    // 释放互斥信号量
    // xSemaphoreGive(xMutex);
    // printf("espnow_tx_mac:");
    // for (size_t i = 0; i < ESP_NOW_ETH_ALEN; i++)
    // {
    //     printf("0x%02X ",mac_addr[i]);
    // }

    // printf("espnow_tx_status:%d\r\n",status);
}

esp_sbus_parsing_t esp_sbus_parsing = 
{
    .uart_num = UART_NUM_1,
    .rx_gpio_num = GPIO_NUM_18,   
    .tx_gpio_num = GPIO_NUM_19,
    .esp_subs_receiv_cb = app_sbus_test,
};

espnow_transmit_t espnow_transmit_ = 
{
    .enpnow_channel = ESPNOW_CHANNEL_01,
    .enpnow_need_pairing = false,
    .enpnow_rate = WIFI_PHY_RATE_LORA_250K,
    .espnow_recv_cb = app_espnow_rx_test,
    .espnow_send_cb = app_espnow_tx_test,
};

uint8_t buff[10];

void app_main(void)
{
    gpio_reset_pin(GPIO_NUM_22);
    gpio_set_direction(GPIO_NUM_22, GPIO_MODE_INPUT);
    gpio_reset_pin(GPIO_NUM_23);
    gpio_set_direction(GPIO_NUM_23, GPIO_MODE_INPUT);

    xMutex = xSemaphoreCreateMutex();

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
    xTaskCreate(espnow_teansmit_task, "espnow_teansmit_task", 4096, &espnow_transmit_, 12, NULL);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    xTaskCreate(esp_subs_parsing_task, "esp_subs_parsing_task", 2048, &esp_sbus_parsing, 12, NULL);

    //xTaskCreatePinnedToCore(esp_subs_parsing_task, "esp_subs_parsing_task", 2048, &esp_sbus_parsing, 12, NULL,1);
    for (size_t i = 0; i < 10; i++)
    {
        buff[i] = i;
    }
    

    while (1)
    {
        if(gpio_get_level(GPIO_NUM_22) == 0)
        {
            espnow_set_pairing(true);
        }
        else
        {
            espnow_set_pairing(false);
        }

        if(gpio_get_level(GPIO_NUM_23) == 0)
        {
            buff[0]++;
            espnow_transmit_send(buff,10);
        }

        vTaskDelay(1000);
    }
    

}
