#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// Struct lưu trữ toàn bộ dữ liệu môi trường
struct AirData {
    // BME680
    float temperature;
    float humidity;
    float pressure;
    float gas_resistance;
    
    // PMS5003
    uint16_t pm10_env;
    uint16_t pm25_env;
    uint16_t pm100_env;
    
    // MQ135
    uint16_t mq_adc;
    float mq_voltage;
    float mq_rs;
    float co2_estimate;
};

void sensors_init();
void sensors_update(AirData &data);
String evaluateAirQuality(uint16_t pm25, float co2);

#endif