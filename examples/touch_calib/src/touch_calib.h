
#ifndef _TOUCH_CALIB_H_
#define _TOUCH_CALIB_H_

#include <TouchScreen.h>
#include <TFT_eSPI.h>

void drawMark(uint16_t x, uint16_t y);
void waitTouch();
void waitRelase();
void fail();
void keepHolding();
void initialize();
TSPoint readValues(uint16_t posX, uint16_t posY);
void deletePoint(uint8_t strt, uint16_t (&arr)[16], uint8_t num);
void insertValue(uint16_t val, uint16_t (&arr)[16]);
void printResults();
void optimizeSamples();
void calcCoef();
void calibrate();
void map2Screen(uint16_t &x, uint16_t &y);

void setup();
void loop();

#endif
