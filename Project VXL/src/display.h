#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include "sensors.h"
#include <Adafruit_SH110X.h> 

void display_init();
void display_showPage(int page, AirData &data, String timeStr, bool sdStatus);

#endif