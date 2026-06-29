#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <SD.h>

#include "sensors.h"
#include "rtc.h"
#include "display.h"
#include "logger.h"

// Biến toàn cục hệ thống
AirData currentData; 
bool isSdOk = false;
int currentPage = 0;

// Các mốc thời gian (Scheduler)
unsigned long lastSensorUpdate = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastLogUpdate = 0;

// Thông tin WiFi
const char* ssid = "5G VIP";
const char* password = "12345679";

AsyncWebServer server(80);

String formatBytes(size_t bytes) {
    if (bytes < 1024) return String(bytes) + " B";
    else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
    else return String(bytes / 1024.0 / 1024.0) + " MB";
}

void setupWebServer() {
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    server.on("/api/data", HTTP_GET, [](AsyncWebServerRequest *request){
        StaticJsonDocument<1024> doc; // Tăng size JSON lên 1024 cho an toàn
        doc["temperature"] = currentData.temperature;
        doc["humidity"] = currentData.humidity;
        doc["pressure"] = currentData.pressure;
        doc["gas_res"] = currentData.gas_resistance;
        doc["pm1"] = currentData.pm10_env;
        doc["pm25"] = currentData.pm25_env;
        doc["pm10"] = currentData.pm100_env;
        doc["co2"] = currentData.co2_estimate;
        doc["aqi_status"] = evaluateAirQuality(currentData.pm25_env, currentData.co2_estimate);
        doc["time"] = rtc_getTimeStr();
        doc["sd_ok"] = isSdOk;

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    server.on("/api/system", HTTP_GET, [](AsyncWebServerRequest *request){
        StaticJsonDocument<256> doc;
        doc["ip"] = WiFi.localIP().toString();
        doc["rssi"] = WiFi.RSSI();
        doc["free_heap"] = formatBytes(ESP.getFreeHeap());
        doc["uptime_sec"] = millis() / 1000;
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    server.on("/system", HTTP_GET, [](AsyncWebServerRequest *request){
        String html = R"=====(
            <!DOCTYPE html><html data-bs-theme='dark'><head>
            <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet"></head>
            <body class='container mt-5 text-white bg-dark'>
            <a href='/' class='btn btn-secondary mb-3'><- Về Dashboard</a>
            <h2>Thông số hệ thống ESP32</h2>
            <ul class='list-group list-group-flush'>
        )=====";
        html += "<li class='list-group-item bg-dark text-white'>IP: " + WiFi.localIP().toString() + "</li>";
        html += "<li class='list-group-item bg-dark text-white'>WiFi RSSI: " + String(WiFi.RSSI()) + " dBm</li>";
        html += "<li class='list-group-item bg-dark text-white'>Free RAM: " + formatBytes(ESP.getFreeHeap()) + "</li>";
        html += "<li class='list-group-item bg-dark text-white'>Uptime: " + String(millis() / 1000) + " giây</li>";
        html += "</ul></body></html>";
        request->send(200, "text/html", html);
    });

    server.on("/log", HTTP_GET, [](AsyncWebServerRequest *request){
        String html = R"=====(
            <!DOCTYPE html><html data-bs-theme='dark'><head>
            <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet"></head>
            <body class='container mt-5 text-white bg-dark text-center'>
            <a href='/' class='btn btn-secondary mb-3'><- Về Dashboard</a>
            <h2>Quản lý dữ liệu Thẻ nhớ (SD Card)</h2><br>
        )=====";
        if(isSdOk && SD.exists("/airlog.csv")) {
            html += "<p>File airlog.csv đang hoạt động và lưu trữ dữ liệu.</p>";
            html += "<a href='/download' class='btn btn-success btn-lg'>Tải xuống file airlog.csv</a>";
        } else {
            html += "<div class='alert alert-danger'>Lỗi: Không tìm thấy thẻ SD hoặc file log chưa được tạo.</div>";
        }
        html += "</body></html>";
        request->send(200, "text/html", html);
    });

    server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request){
        if(isSdOk && SD.exists("/airlog.csv")) request->send(SD, "/airlog.csv", "text/csv", true); 
        else request->send(404, "text/plain", "File not found");
    });

    server.begin();
    Serial.println("[WEB] Server da khoi dong!");
}

void setup() {
    Serial.begin(115200);

    if(!LittleFS.begin(true)){
        Serial.println("[LỖI] Khong the khoi tao LittleFS");
        return;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("[WIFI] Dang ket noi.");
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.println(String("\n[WIFI] Ket noi thanh cong! IP: ") + WiFi.localIP().toString());

    // Khởi tạo phần cứng
    display_init(); 
    rtc_init();
    sensors_init();
    isSdOk = logger_init(); // Kích hoạt thẻ nhớ

    setupWebServer();
}

void loop() {
    unsigned long currentMillis = millis();
    
    // 1. Task: Đọc cảm biến (Mỗi 5 giây)
    if (currentMillis - lastSensorUpdate >= 5000) {
        lastSensorUpdate = currentMillis;
        sensors_update(currentData); // Cập nhật dữ liệu vào biến currentData
        
        // LỆNH IN DỮ LIỆU RA SERIAL ĐỂ LƯU FILE CSV (Bạn đang bị thiếu đoạn này)
        Serial.printf("%s,%s,%.2f,%.2f,%.2f,%.2f,%.0f,%d,%d,%d\r\n", 
            rtc_getDateStr().c_str(), rtc_getTimeStr().c_str(),
            currentData.temperature, currentData.humidity, 
            currentData.pressure, currentData.gas_resistance, 
            currentData.co2_estimate, currentData.pm10_env, 
            currentData.pm25_env, currentData.pm100_env);
    }

    // 2. Task: Cập nhật giao diện OLED (Mỗi 3 giây)
    if (currentMillis - lastDisplayUpdate >= 5000) {
        lastDisplayUpdate = currentMillis;
        display_showPage(currentPage, currentData, rtc_getTimeStr(), isSdOk);
        currentPage++;
        if (currentPage > 4) currentPage = 0;
    }

    // 3. Task: Ghi Log SD Card (Mỗi 60 giây - sẽ tự động bỏ qua nếu thẻ lỗi)
    if (currentMillis - lastLogUpdate >= 60000) {
        lastLogUpdate = currentMillis;
        if (isSdOk) logger_appendData(rtc_getDateStr(), rtc_getTimeStr(), currentData);
    }
}