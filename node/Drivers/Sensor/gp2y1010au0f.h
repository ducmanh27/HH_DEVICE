/*
 * gp2y1010au0f.h
 *
 *  Created on: May 12, 2025
 *      Author: HLC
 */

#ifndef SENSOR_GP2Y1010AU0F_H_
#define SENSOR_GP2Y1010AU0F_H_


#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_adc.h"

typedef struct {
    GPIO_TypeDef* LED_GPIO_Port;
    uint16_t LED_Pin;
    ADC_HandleTypeDef* hadc;
    float Vcc;
} GP2Y1010_HandleTypeDef;

void GP2Y1010_Init(GP2Y1010_HandleTypeDef* sensor);
float GP2Y1010_Read(GP2Y1010_HandleTypeDef* sensor);

#endif /* SENSOR_GP2Y1010AU0F_H_ */
