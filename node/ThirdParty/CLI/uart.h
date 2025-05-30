/*
 * uart.h
 *
 *  Created on: May 31, 2025
 *      Author: HLC
 */

#ifndef CLI_UART_H_
#define CLI_UART_H_
#include "string.h"
#include "stdint.h"
#include "stm32f4xx_hal.h"
void CLI_UART_Init(UART_HandleTypeDef *huart);
void CLI_UART_Handle(void);
void CLI_UART_RXCallback(UART_HandleTypeDef *huart);




#endif /* CLI_UART_H_ */
