#include "sensors.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "Adafruit_PM25AQI.h"

#define MQ135_PIN 34
#define RL_VALUE 10.0 // Điện trở tải RL trên board MQ135 (kOhm)

Adafruit_BME680 bme;
Adafruit_PM25AQI aqi;

HardwareSerial pmSerial(2); // Dùng UART2 cho PMS5003 (RX=16, TX=17)

void sensors_init() {
    Serial.println("[SENSOR] Dang khoi tao cac cam bien...");

    // 1. Khởi tạo BME680
    if (!bme.begin(0x76) && !bme.begin(0x77)) { 
        Serial.println("[LỖI] Khong tim thay BME680!");
    } else {
        bme.setTemperatureOversampling(BME680_OS_8X);
        bme.setHumidityOversampling(BME680_OS_2X);
        bme.setPressureOversampling(BME680_OS_4X);
        bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
        bme.setGasHeater(320, 150); // 320°C trong 150ms
        Serial.println("[SENSOR] BME680 OK");
    }

    // 2. Khởi tạo PMS5003
    pmSerial.begin(9600, SERIAL_8N1, 16, 17);
    if (!aqi.begin_UART(&pmSerial)) {
        Serial.println("[LỖI] Khong the ket noi PMS5003!");
    } else {
        Serial.println("[SENSOR] PMS5003 OK");
    }
    
    // 3. Khởi tạo ADC cho MQ135
    analogReadResolution(12); // ESP32 ADC 12-bit (0-4095)
}

void sensors_update(AirData &data) {
    // Đọc BME680
    if (bme.performReading()) {
        data.temperature = bme.temperature;
        data.humidity = bme.humidity;
        data.pressure = bme.pressure / 100.0; // Chuyển sang hPa
        data.gas_resistance = bme.gas_resistance / 1000.0; // kOhms
    }

    // Đọc PMS5003 (Cần đọc liên tục để clear buffer)
    PM25_AQI_Data pmData;
    if (aqi.read(&pmData)) {
        data.pm10_env = pmData.pm10_env;
        data.pm25_env = pmData.pm25_env;
        data.pm100_env = pmData.pm100_env;
    }

    // Đọc MQ135
    data.mq_adc = analogRead(MQ135_PIN);
    data.mq_voltage = data.mq_adc * (3.3 / 4095.0);
    
    if(data.mq_voltage == 0) data.mq_voltage = 0.001; // Tránh chia cho 0
    // Tính Rs: Rs = (Vcc - Vout) / Vout * RL
    data.mq_rs = ((3.3 - data.mq_voltage) / data.mq_voltage) * RL_VALUE;
    
    // Ước lượng CO2 thô (Chưa hiệu chuẩn). Chỉ để demo thuật toán
    // Mapping tuyến tính giả lập dựa trên ADC để hiển thị sự thay đổi
    data.co2_estimate = map(data.mq_adc, 0, 4095, 400, 5000); 
}

String evaluateAirQuality(uint16_t pm25, float co2) {
    if (pm25 < 35 && co2 < 1000) return "GOOD";
    if (pm25 < 75 && co2 < 2000) return "MODERATE";
    return "UNHEALTHY";
}