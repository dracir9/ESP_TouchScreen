
#ifndef _TOUCH_CALIB_H_
#define _TOUCH_CALIB_H_

#include <ESP32_TouchScreen.h>
#include <TFT_eSPI.h>

void waitTouch();
void waitRelase();
void initialize();

void setup();
void loop();

#endif
