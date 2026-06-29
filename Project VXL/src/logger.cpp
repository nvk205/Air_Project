#include "logger.h"
#include <SPI.h>
#include <SD.h>

#define SD_CS_PIN 5

bool logger_init() {
    Serial.println("[SD] Khoi tao The Nho...");
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("[LỖI] Khong tim thay the SD hoac the hong!");
        return false;
    }
    
    File dataFile = SD.open("/airlog.csv", FILE_READ);
    if (!dataFile) {
        // Nếu file chưa tồn tại, tạo mới và ghi Header
        dataFile = SD.open("/airlog.csv", FILE_WRITE);
        if (dataFile) {
            dataFile.println("Date,Time,Temp,Humidity,Pressure,GasRes,CO2,PM1,PM25,PM10");
            dataFile.close();
            Serial.println("[SD] Da tao file airlog.csv moi.");
        }
    } else {
        dataFile.close();
        Serial.println("[SD] File log da ton tai, san sang append.");
    }
    return true;
}

void logger_appendData(String dateStr, String timeStr, AirData &data) {
    File dataFile = SD.open("/airlog.csv", FILE_APPEND);
    
    if (dataFile) {
        dataFile.print(dateStr); dataFile.print(",");
        dataFile.print(timeStr); dataFile.print(",");
        dataFile.print(data.temperature, 2); dataFile.print(",");
        dataFile.print(data.humidity, 2); dataFile.print(",");
        dataFile.print(data.pressure, 2); dataFile.print(",");
        dataFile.print(data.gas_resistance, 2); dataFile.print(",");
        dataFile.print(data.co2_estimate, 0); dataFile.print(",");
        dataFile.print(data.pm10_env); dataFile.print(",");
        dataFile.print(data.pm25_env); dataFile.print(",");
        dataFile.println(data.pm100_env);
        dataFile.close();
        Serial.println("[SD] Da ghi 1 dong vao airlog.csv");
    } else {
        Serial.println("[LỖI] Khong the mo file de ghi!");
    }
}