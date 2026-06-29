#include "display.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define i2c_Address 0x3c 

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void display_init() {
    Serial.println("[OLED] Khoi tao man hinh...");
    delay(250); 
    
    if(!display.begin(i2c_Address, true)) {
        Serial.println("[LỖI] Khong the cap phat bo nho OLED!");
        while(1); 
    }
    
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE); // Đổi SSD1306_WHITE thành SH110X_WHITE
    display.setTextSize(1);
    display.setCursor(0,20);
    display.println("Air Quality Monitor");
    display.println("Initializing...");
    display.display();
    delay(1000);
}


void display_showPage(int page, AirData &data, String timeStr, bool sdStatus) {
    display.clearDisplay();
    
    // Header chung cho mọi trang
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Time: ");
    display.print(timeStr);
    if (!sdStatus) {
        display.setCursor(95, 0);
        display.print("SD!E"); // Cảnh báo lỗi thẻ nhớ góc phải
    }
    display.drawLine(0, 10, 128, 10, SH110X_WHITE);

    // Nội dung theo từng trang
    display.setCursor(0, 15);
    switch (page) {
        case 0: // Trang 1
            display.println("-- BASIC ENV --");
            display.print("Temp: "); display.print(data.temperature, 1); display.println(" C");
            display.print("Humi: "); display.print(data.humidity, 1); display.println(" %RH");
            break;
            
        case 1: // Trang 2
            display.println("-- PRESSURE & GAS --");
            display.print("Pres: "); display.print(data.pressure, 1); display.println(" hPa");
            display.print("Gas R: "); display.print(data.gas_resistance, 2); display.println(" kOhm");
            break;
            
        case 2: // Trang 3
            display.println("-- PARTICULATE --");
            display.print("PM1.0: "); display.print(data.pm10_env); display.println(" ug/m3");
            display.print("PM2.5: "); display.print(data.pm25_env); display.println(" ug/m3");
            display.print("PM10 : "); display.print(data.pm100_env); display.println(" ug/m3");
            break;
            
        case 3: // Trang 4
            display.println("-- AIR TOXICITY --");
            display.print("MQ ADC : "); display.println(data.mq_adc);
            display.print("MQ Volt: "); display.print(data.mq_voltage, 2); display.println(" V");
            display.print("eCO2   : "); display.print(data.co2_estimate, 0); display.println(" ppm");
            break;
            
        case 4: // Trang 5
            display.println("-- OVERALL STATUS --");
            display.setTextSize(2);
            display.setCursor(0, 30);
            display.print(evaluateAirQuality(data.pm25_env, data.co2_estimate));
            break;
    }
    
    // Footer hiển thị chỉ báo trang
    display.setTextSize(1);
    display.setCursor(45, 55);
    display.print("Page "); display.print(page + 1); display.print("/5");
    
    display.display();
}