#include "rtc.h"
#include <ThreeWire.h>
#include <RtcDS1302.h>

// Cấu hình chân DS1302: DAT = 13, CLK = 14, RST = 27
ThreeWire myWire(13, 14, 27);
RtcDS1302<ThreeWire> Rtc(myWire);

void rtc_init() {
    Serial.println("[RTC] Dang khoi tao DS1302...");
    Rtc.Begin();
    
    if (!Rtc.IsDateTimeValid()) {
        Serial.println("[LỖI] RTC mat thoi gian, dang cai dat lai!");
        RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
        Rtc.SetDateTime(compiled);
    }

    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    Rtc.SetDateTime(compiled);
    
    if (Rtc.GetIsWriteProtected()) {
        Rtc.SetIsWriteProtected(false);
    }
    if (!Rtc.GetIsRunning()) {
        Rtc.SetIsRunning(true);
    }
}

String rtc_getDateStr() {
    RtcDateTime now = Rtc.GetDateTime();
    char datestring[20];
    snprintf(datestring, sizeof(datestring), "%02u/%02u/%04u", 
             now.Day(), now.Month(), now.Year());
    return String(datestring);
}

String rtc_getTimeStr() {
    RtcDateTime now = Rtc.GetDateTime();
    char timestring[20];
    snprintf(timestring, sizeof(timestring), "%02u:%02u:%02u", 
             now.Hour(), now.Minute(), now.Second());
    return String(timestring);
}