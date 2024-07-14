#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include <esp_sbus_parsing.h>

#define SUBS_TX_BUF_SIZE (256)
#define SUBS_RX_BUF_SIZE (256)
#define PATTERN_CHR_NUM  (2)         /*!< Set the number of consecutive and identical characters received by receiver which defines a UART pattern*/

static const char *TAG = "ESP_SUBS_PARSING";
static QueueHandle_t uart_queue;


/**
 * @brief 提取SUBS格式数据下指定通道数据
 * @note 
 *
 * @param byte：待解析原始数据
 * @param channelx：需要获取的通道编号
 * @return 指定通道值
 */
uint16_t esp_sbus_channel_data_parsing(uint8_t *byte,sbus_channel_x channelx)
{
    if(byte[0]  != 0x0F) return 0xFFFF;//

    switch (channelx)
    {
        case SBUS_CHANNEL_00:
            return (byte[1]  >> 0 | byte[2]  << 8) & 0x7ff;
        break;
        case SBUS_CHANNEL_01:
            return (byte[2]  >> 3 | byte[3]  << 5) & 0x7ff;
        break;
        case SBUS_CHANNEL_02:
            return (byte[3]  >> 6 | byte[4]  << 2 | byte[5] << 10) & 0x7ff;
        break;
        case SBUS_CHANNEL_03:
            return (byte[5]  >> 1 | byte[6]  << 7) & 0x7ff;
        break;
        case SBUS_CHANNEL_04:
            return (byte[6]  >> 4 | byte[7]  << 4) & 0x7ff;
        break;
        case SBUS_CHANNEL_05:
            return (byte[7]  >> 7 | byte[8]  << 1 | byte[9] << 9) & 0x7ff;
        break;
        case SBUS_CHANNEL_06:
            return (byte[9]  >> 2 | byte[10] << 6) & 0x7ff;
        break;
        case SBUS_CHANNEL_07:
            return (byte[10] >> 5 | byte[11] << 3) & 0x7ff;
        break;
        case SBUS_CHANNEL_08:
            return (byte[12] >> 0 | byte[13] << 8) & 0x7ff;
        break;
        case SBUS_CHANNEL_09:
            return (byte[13] >> 3 | byte[14] << 5) & 0x7ff;
        break;
        case SBUS_CHANNEL_10:
            return (byte[14] >> 6 | byte[15] << 2 | byte[16] << 10) & 0x7ff;
        break;
        case SBUS_CHANNEL_11:
            return (byte[16] >> 1 | byte[17] << 7) & 0x7ff;
        break;
        case SBUS_CHANNEL_12:
            return (byte[17] >> 4 | byte[18] << 4) & 0x7ff;
        break;
        case SBUS_CHANNEL_13:
            return (byte[18] >> 7 | byte[19] << 1 | byte[20] << 9) & 0x7ff; 
        break;
        case SBUS_CHANNEL_14:
            return (byte[20] >> 2 | byte[21] << 6) & 0x7ff;
        break;
        case SBUS_CHANNEL_15:
            return (byte[21] >> 5 | byte[22] << 3) & 0x7ff;
        default:
            return 0xFFFF;
    }
}


/**
 * @brief sbus解析任务
 * @note 
 *
 * @param 
 * @param 
 * @return 
 */
void esp_subs_parsing_task(void *pvParameters)
{
    esp_sbus_parsing_t * esp_sbus_parsing = (esp_sbus_parsing_t*)pvParameters;

    uart_event_t event;
    size_t buffered_size;
    uint8_t* dtmp = (uint8_t*) malloc(SUBS_RX_BUF_SIZE);

    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 100000,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    //Install UART driver, and get the queue.
    uart_driver_install(esp_sbus_parsing->uart_num, SUBS_RX_BUF_SIZE * 2, SUBS_TX_BUF_SIZE * 2, 20, &uart_queue, 0);
    uart_param_config(esp_sbus_parsing->uart_num, &uart_config);

    //Set UART log level
    esp_log_level_set(TAG, ESP_LOG_WARN);
    //Set UART pins (using UART0 default pins ie no changes.)
    uart_set_pin(esp_sbus_parsing->uart_num,esp_sbus_parsing->tx_gpio_num, esp_sbus_parsing->rx_gpio_num, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    //Set uart pattern detect function.
    uart_enable_pattern_det_baud_intr(esp_sbus_parsing->uart_num, '+', PATTERN_CHR_NUM, 9, 0, 0);
    //Reset the pattern queue length to record at most 20 pattern positions.
    uart_pattern_queue_reset(esp_sbus_parsing->uart_num, 20);

    while (1)
    {
        //Waiting for UART event.
        if(xQueueReceive(uart_queue, (void * )&event, (TickType_t)portMAX_DELAY)) 
        {
            bzero(dtmp, SUBS_RX_BUF_SIZE);
            switch(event.type) {
                case UART_DATA:
                    //ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                    uart_read_bytes(esp_sbus_parsing->uart_num, dtmp, event.size, portMAX_DELAY);

                    /*有设置回调函数执行回调函数，否则直接打印所有通道数据*/
                    if(esp_sbus_parsing->esp_subs_receiv_cb != NULL)
                    {
                        esp_sbus_parsing->esp_subs_receiv_cb(dtmp,event.size);
                    }
                    else if(event.size == 25 && dtmp[0] == 0x0F)
                    {
                        printf("sbus data:");
                        for (size_t i = 0; i < SBUS_CHANNEL_MAX; i++)
                        {
                            printf("%d,",esp_sbus_channel_data_parsing(dtmp,i));
                        }
                        printf("\n");
                    }
                    else
                    {
                        ESP_LOGE(TAG, "ReveiveDataError: %d", event.size);
                        printf("ErrorData:");
                        for (size_t i = 0; i < event.size; i++)
                        {
                            printf("0x%02X,",dtmp[i]);
                        }
                        printf("\n");
                    }
                    
                    break;
                case UART_FIFO_OVF:
                    ESP_LOGI(TAG, "hw fifo overflow");
                    uart_flush_input(esp_sbus_parsing->uart_num);
                    xQueueReset(uart_queue);
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG, "ring buffer full");
                    // If buffer full happened, you should consider encreasing your buffer size
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(esp_sbus_parsing->uart_num);
                    xQueueReset(uart_queue);
                    break;
                //Event of UART RX break detected
                case UART_BREAK:
                    //ESP_LOGI(TAG, "uart rx break");
                    break;
                //Event of UART parity check error
                case UART_PARITY_ERR:
                    ESP_LOGI(TAG, "uart parity error");
                    break;
                //Event of UART frame error
                case UART_FRAME_ERR:
                    ESP_LOGI(TAG, "uart frame error");
                    break;
                //UART_PATTERN_DET
                case UART_PATTERN_DET:
                    uart_get_buffered_data_len(esp_sbus_parsing->uart_num, &buffered_size);
                    int pos = uart_pattern_pop_pos(esp_sbus_parsing->uart_num);
                    ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
                    if (pos == -1) {
                        // There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
                        // record the position. We should set a larger queue size.
                        // As an example, we directly flush the rx buffer here.
                        uart_flush_input(esp_sbus_parsing->uart_num);
                    } else {
                        uart_read_bytes(esp_sbus_parsing->uart_num, dtmp, pos, 100 / portTICK_PERIOD_MS);
                        uint8_t pat[PATTERN_CHR_NUM + 1];
                        memset(pat, 0, sizeof(pat));
                        uart_read_bytes(esp_sbus_parsing->uart_num, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
                        ESP_LOGI(TAG, "read data: %s", dtmp);
                        ESP_LOGI(TAG, "read pat : %s", pat);
                    }
                    break;
                //Others
                default:
                    ESP_LOGI(TAG, "uart event type: %d", event.type);
                    break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

