	/*
 * pzem004t.c
 *
 *  Created on: May 11, 2025
 *      Author: ManhPD9
 */

#include "pzem004t.h"
#include <string.h>
#include <math.h>

// CRC16 table (Modbus polynomial 0xA001)
static const uint16_t PZEM004T_CrcTable[] = {
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

// CRC16 - Modbus calculation in C
uint16_t PZEM004T_CalculateCRC16(const uint8_t *data, uint16_t len) {
    uint8_t nTemp;
    uint16_t crc = 0xFFFF;

    while (len--) {
        nTemp = *data++ ^ crc;
        crc >>= 8;
        crc ^= PZEM004T_CrcTable[nTemp];
    }
    return crc;
}

static void PZEM004T_SetCRC(uint8_t *buf, uint16_t len){
    if(len <= 2) // Sanity check
        return;

    uint16_t crc = PZEM004T_CalculateCRC16(buf, len - 2); // CRC of data

    // Write high and low byte to last two positions
    buf[len - 2] = crc & 0xFF; // Low byte first
    buf[len - 1] = (crc >> 8) & 0xFF; // High byte second
}

static bool PZEM004T_CheckCRC(const uint8_t *buf, uint16_t len){
    if(len <= 2) // Sanity check
        return false;

    uint16_t crc = PZEM004T_CalculateCRC16(buf, len - 2); // Compute CRC of data
    return ((uint16_t)buf[len-2]  | (uint16_t)buf[len-1] << 8) == crc;
}



static bool PZEM004T_ValidateUARTSettings(UART_InitTypeDef initSetting)
{
	//  check baud rate, start, stop bits and no parity
	if ( (initSetting.BaudRate != 9600)
			|| (initSetting.Parity != UART_PARITY_NONE)
			|| (initSetting.WordLength != UART_WORDLENGTH_8B)
			|| (initSetting.StopBits != UART_STOPBITS_1)
		)
	{
		return false;
	}
	else
	{
		return true;
	}

}
static PZEM004T_Status PZEM004T_ReceiveReponseBuffer(PZEM004T* dev, uint8_t *respBuf, uint16_t len)
{
	for (uint16_t bufIdx = 0; bufIdx < len; bufIdx ++)
	{
		respBuf[bufIdx] = 0x00;
	}

	HAL_StatusTypeDef receiveStatus = HAL_UART_Receive(dev->uartHandle, respBuf, len, PZEM_READ_TIMEOUT);
    if (receiveStatus != HAL_OK)
    {
    	return PZEM004T_ERROR;
    }
    else
    {
        // Check CRC with the number of bytes read
        if(PZEM004T_CheckCRC(respBuf, len)){
            return PZEM004T_SUCESS;
        }
        else
        {
        	return PZEM004T_ERROR;
        }
    }
}
// Read holding registers
static PZEM004T_Status PZEM004T_SendCommand(PZEM004T* dev, uint8_t cmd, uint16_t rAddr, uint16_t val, bool isRecevieResponse, uint16_t slave_addr) {
    uint8_t sendBuffer[8] ; // Send buffer
    uint8_t respBuffer[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Response buffer (only used when check is true)

    sendBuffer[0] = slave_addr;                   // Set slave address
    sendBuffer[1] = cmd;                     // Set command

    sendBuffer[2] = (rAddr >> 8) & 0xFF;     // Set high byte of register address
    sendBuffer[3] = (rAddr) & 0xFF;          // Set low byte =//=

    sendBuffer[4] = (val >> 8) & 0xFF;       // Set high byte of register value
    sendBuffer[5] = (val) & 0xFF;            // Set low byte =//=

    PZEM004T_SetCRC(sendBuffer, 8);                   // Set CRC of frame

    HAL_StatusTypeDef status = HAL_UART_Transmit(dev->uartHandle, sendBuffer, 8, PZEM_SEND_TIMEOUT);
    if (status != HAL_OK)
    {
    	return false;
    }
    if(isRecevieResponse) {
    	PZEM004T_Status receiveStatus = PZEM004T_ReceiveReponseBuffer(dev, respBuffer, 8);
    	if (receiveStatus == PZEM004T_SUCESS)
    	{
            // Check if response is same as send
            for(uint8_t i = 0; i < 8; i++){
                if(sendBuffer[i] != respBuffer[i])
                    return PZEM004T_ERROR;
            }
    	}
    	else
    	{
    		return PZEM004T_ERROR;
    	}
    }
    return PZEM004T_SUCESS;
}

bool PZEM004T_Init(UART_HandleTypeDef *huart, PZEM004T* dev)
{
	dev->uartHandle = huart;
	dev->data.voltage = 0.0f;
	dev->data.current = 0.0f;
	dev->data.power = 0.0f;
	dev->data.energy = 0.0f;
	dev->data.frequency = 0.0f;
	dev->data.pf = 0.0f;
	dev->data.alarm = 0u;
	return PZEM004T_ValidateUARTSettings(dev->uartHandle->Init);

}
PZEM004T_Status PZEM004T_ReadData(PZEM004T* dev)
{
	static uint8_t response[25];

	if (PZEM004T_SendCommand(dev, CMD_RIR, 0x00, 0x0A, false, PZEM_DEFAULT_ADDR) == PZEM004T_SUCESS)
	{
		if(PZEM004T_ReceiveReponseBuffer(dev, response, (uint16_t)NUMBER_BYTES_SENSOR_DATA) == PZEM004T_SUCESS){
			// Update the current values
			dev->data.voltage = ((uint32_t)response[3] << 8 | // Raw voltage in 0.1V
									  (uint32_t)response[4])/10.0;

			dev->data.current = ((uint32_t)response[5] << 8 | // Raw current in 0.001A
									  (uint32_t)response[6] |
									  (uint32_t)response[7] << 24 |
									  (uint32_t)response[8] << 16) / 1000.0;

			dev->data.power =   ((uint32_t)response[9] << 8 | // Raw power in 0.1W
									  (uint32_t)response[10] |
									  (uint32_t)response[11] << 24 |
									  (uint32_t)response[12] << 16) / 10.0;

			dev->data.energy =  ((uint32_t)response[13] << 8 | // Raw Energy in 1Wh
									  (uint32_t)response[14] |
									  (uint32_t)response[15] << 24 |
									  (uint32_t)response[16] << 16) / 1000.0;

			dev->data.frequency=((uint32_t)response[17] << 8 | // Raw Frequency in 0.1Hz
									  (uint32_t)response[18]) / 10.0;

			dev->data.pf =      ((uint32_t)response[19] << 8 | // Raw pf in 0.01
									  (uint32_t)response[20])/100.0;

			dev->data.alarm =  ((uint32_t)response[21] << 8 | // Raw alarm value
									  (uint32_t)response[22]);
		}
		else
		{
			return PZEM004T_ERROR;
		}
	}
	else
	{
		return PZEM004T_ERROR;
	}

	return PZEM004T_SUCESS;
}

PZEM004T_Status PZEM_ResetEnergy(PZEM004T* dev)
{
    uint8_t buffer[] = {0x00, CMD_REST, 0x00, 0x00};
    uint8_t reply[5];
    buffer[0] = PZEM_DEFAULT_ADDR;
    PZEM004T_SetCRC(buffer, 4);
    if (HAL_UART_Transmit(dev->uartHandle, buffer, 4, PZEM_SEND_TIMEOUT) == HAL_OK)
    {
        uint16_t length = PZEM004T_ReceiveReponseBuffer(dev, reply, 5);
        if(length == 0 || length == 5){
            return PZEM004T_ERROR;
        }
        return PZEM004T_SUCESS;
    }
    else
    {
    	return PZEM004T_ERROR;
    }

}

