/*
 * pzem004t.h
 *
 *  Created on: May 11, 2025
 *      Author: HLC
 */

#ifndef SENSOR_PZEM004T_H_
#define SENSOR_PZEM004T_H_


#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

#define PZEM_SEND_TIMEOUT   100
#define PZEM_READ_TIMEOUT   100
// PZEM Commands
#define CMD_RHR             0x03
#define CMD_WSR             0x06
#define CMD_REST            0x42

// Registers
#define REG_VOLTAGE         0x0000
#define REG_CURRENT_L       0x0001
#define REG_CURRENT_H       0x0002
#define REG_POWER_L         0x0003
#define REG_POWER_H         0x0004
#define REG_ENERGY_L        0x0005
#define REG_ENERGY_H        0x0006
#define REG_FREQUENCY       0x0007
#define REG_PF              0x0008
#define REG_ALARM           0x0009

#define CMD_RHR         0x03
#define CMD_RIR         0X04
#define CMD_WSR         0x06
#define CMD_CAL         0x41
#define CMD_REST        0x42


#define WREG_ALARM_THR   0x0001
#define WREG_ADDR        0x0002

#define UPDATE_TIME     200

#define RESPONSE_SIZE 32
#define READ_TIMEOUT 100

#define INVALID_ADDRESS 0x00

#define PZEM_DEFAULT_ADDR   0xF8
#define PZEM_BAUD_RATE      9600
#define NUMBER_BYTES_SENSOR_DATA 0x19

typedef enum {
    PZEM004T_OK = 0,
    PZEM004T_ERROR_INVALID_UART = 1,
    PZEM004T_ERROR_SEND_COMMAND = 2,
    PZEM004T_ERROR_RECEIVE_RESPONSE = 3,
	PZEM004T_ERROR_WRONG_CRC = 4,
	PZEM004T_ERROR_NOT_SEND_CMD = 5

} PZEM004T_ErrorCode;

typedef enum {
	PZEM004T_ERROR,
	PZEM004T_SUCESS
} PZEM004T_Status;

typedef struct {
    double voltage;
    double current;
    double power;
    double energy;
    double frequency;
    double pf;
    uint16_t alarm;
} PZEM004T_Data;

typedef struct {
	UART_HandleTypeDef *uartHandle;
	PZEM004T_Data data;
} PZEM004T;
uint16_t PZEM004T_CalculateCRC16(const uint8_t *data, uint16_t len);
bool PZEM004T_Init(UART_HandleTypeDef *huart, PZEM004T* dev);
PZEM004T_Status PZEM004T_ReadData(PZEM004T* dev);
PZEM004T_Status PZEM_ResetEnergy(PZEM004T* dev);


#endif /* SENSOR_PZEM004T_H_ */
