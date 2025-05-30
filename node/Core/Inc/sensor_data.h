/*
 * sensor_data.h
 *
 *  Created on: May 18, 2025
 *      Author: HLC
 */

#ifndef INC_SENSOR_DATA_H_
#define INC_SENSOR_DATA_H_
#include <stdint.h>
typedef struct
{
	// Read from PZEM004T sensor
	double voltage;
	double current;
	double power;
	double energy;
	double pf;
	double frequency;
	// Read from GP2Y1010AU0F sensor
	float dust_density;
	uint32_t time_stamp;
	// Read from sht31 sensor
	float temperature;
	float humidity;
} sensor_data_t;



#endif /* INC_SENSOR_DATA_H_ */
