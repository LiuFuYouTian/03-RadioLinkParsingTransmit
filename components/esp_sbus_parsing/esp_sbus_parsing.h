#ifndef ESP_SBUS_PARSING_H
#define ESP_SBUS_PARSING_H

#include <stdio.h>
#include "driver/uart.h"
#include "driver/gpio.h"

typedef enum {
    SBUS_CHANNEL_00 = 0x0,
    SBUS_CHANNEL_01,
    SBUS_CHANNEL_02,    
    SBUS_CHANNEL_03,    
    SBUS_CHANNEL_04,    
    SBUS_CHANNEL_05,
    SBUS_CHANNEL_06,
    SBUS_CHANNEL_07,    
    SBUS_CHANNEL_08,    
    SBUS_CHANNEL_09,    
    SBUS_CHANNEL_10,
    SBUS_CHANNEL_11,
    SBUS_CHANNEL_12,    
    SBUS_CHANNEL_13,    
    SBUS_CHANNEL_14,    
    SBUS_CHANNEL_15,
    SBUS_CHANNEL_MAX,
} sbus_channel_x;

/**
  * @brief     Callback function of sending esp_sbus data
  * @param     byte sbus data
  * @param     len 一版情况下SBUS数据长度固定是25
  */
typedef void (*esp_subs_cb_t)(uint8_t *byte, uint8_t len);

typedef struct {
    uint8_t uart_num;
    uint8_t rx_gpio_num;
    uint8_t tx_gpio_num;

    esp_subs_cb_t esp_subs_receiv_cb;
} esp_sbus_parsing_t;

/**
 * @brief 提取SUBS格式数据下指定通道数据
 * @note 
 *
 * @param byte：待解析原始数据
 * @param channelx：需要获取的通道编号
 * @return 指定通道值
 */
uint16_t esp_sbus_channel_data_parsing(uint8_t *byte,sbus_channel_x channelx);

/**
 * @brief sbus解析任务
 * @note 
 *
 * @param 
 * @param 
 * @return 
 */
void esp_subs_parsing_task(void *pvParameters);

#endif
