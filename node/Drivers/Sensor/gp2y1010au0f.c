/*
 * gp2y1010au0f.c
 *
 *  Created on: May 12, 2025
 *      Author: HLC
 */


#include "gp2y1010au0f.h"

#define GP2Y1010_SAMPLING_TIME_US  280
#define GP2Y1010_DELTA_TIME_US     40
#define GP2Y1010_SLEEP_TIME_US     9680

static void delay_us(uint16_t us)
{
    // Sử dụng DWT để delay micro giây (cần bật DWT trước ở main)
    uint32_t startTick = DWT->CYCCNT;
    uint32_t delayTicks = us * (HAL_RCC_GetHCLKFreq() / 1000000);
    while ((DWT->CYCCNT - startTick) < delayTicks);
}

void GP2Y1010_Init(GP2Y1010_HandleTypeDef* sensor)
{
    // Bật LED lên ban đầu (HIGH = tắt LED hồng ngoại)
    HAL_GPIO_WritePin(sensor->LED_GPIO_Port, sensor->LED_Pin, GPIO_PIN_SET);
}

float GP2Y1010_Read(GP2Y1010_HandleTypeDef* sensor)
{
    uint32_t adc_value = 0;
    float voltage = 0.0f;
    float dust_density = 0.0f;

    // Bật LED (LOW = bật)
    HAL_GPIO_WritePin(sensor->LED_GPIO_Port, sensor->LED_Pin, GPIO_PIN_RESET);
    delay_us(GP2Y1010_SAMPLING_TIME_US);

    // Đọc ADC
    HAL_ADC_Start(sensor->hadc);
    HAL_ADC_PollForConversion(sensor->hadc, HAL_MAX_DELAY);
    adc_value = HAL_ADC_GetValue(sensor->hadc);
    HAL_ADC_Stop(sensor->hadc);

    delay_us(GP2Y1010_DELTA_TIME_US);

    // Tắt LED
    HAL_GPIO_WritePin(sensor->LED_GPIO_Port, sensor->LED_Pin, GPIO_PIN_SET);
    delay_us(GP2Y1010_SLEEP_TIME_US);

    // Tính điện áp ADC (với độ phân giải 12-bit: 0–4095)
    voltage = ((float)adc_value) * sensor->Vcc / 4095.0f;

    // Tính mật độ bụi
    dust_density = (0.17f * voltage - 0.1f) * 1000.0f;

    return dust_density;
}


