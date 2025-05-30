/*
 * uart.c
 *
 *  Created on: May 31, 2025
 *      Author: HLC
 */

#include "uart.h"
#include "cli_command.h"
#include <string.h>
#include <stdio.h>
#define MAX_DATA_UART 128

static char uart_buff[MAX_DATA_UART];
static uint8_t uart_len = 0;
static uint8_t uart_flag = 0;

static UART_HandleTypeDef *huart_handle = NULL;  // Lưu handle truyền vào
static uint8_t rx_byte = 0;                      // Nhận từng byte

static void CLI_UART_ReceiveData(uint8_t data_rx)
{
    if (uart_len >= MAX_DATA_UART - 1)  // chống tràn bộ đệm
    {
        uart_len = 0;
        return;
    }

    if (data_rx == '\n')
    {
        uart_buff[uart_len++] = '\0';
        uart_flag = 1;
    }
    else
    {
        uart_buff[uart_len++] = data_rx;
    }
}

void CLI_UART_Handle()
{
    if (uart_flag)
    {
        cli_command_excute(uart_buff, uart_len);
        uart_flag = 0;
        uart_len = 0;
    }
}

// register in HAL_UART_RxCpltCallback
void CLI_UART_RXCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == huart_handle->Instance)
    {
    	CLI_UART_ReceiveData(rx_byte);
        HAL_UART_Receive_IT(huart_handle, &rx_byte, 1);
    }
}

void CLI_UART_Init(UART_HandleTypeDef *huart)
{
    huart_handle = huart;
    uart_len = 0;
    uart_flag = 0;
    HAL_UART_Receive_IT(huart_handle, &rx_byte, 1);
}

