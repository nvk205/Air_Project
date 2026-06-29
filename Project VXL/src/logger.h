#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include "sensors.h"

bool logger_init();
void logger_appendData(String dateStr, String timeStr, AirData &data);

#endif