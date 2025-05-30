// SensorData.h
#ifndef SENSORDATA_H
#define SENSORDATA_H

struct SensorData {
  float voltage;
  float current;
  float power;
  float energy;
  float pf;
  float dust;
  float frequency;
  float dust_density;
  float temperature;
  float humidity;
  uint32_t time_stamp;
};

#endif // SENSORDATA_H