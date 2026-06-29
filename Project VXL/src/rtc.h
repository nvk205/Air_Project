#ifndef RTC_H
#define RTC_H

#include <Arduino.h>

void rtc_init();
String rtc_getDateStr();
String rtc_getTimeStr();

#endif