
#ifndef _ESP32_TOUCHSCREEN_H_
#define _ESP32_TOUCHSCREEN_H_
#include <Arduino.h>

#ifndef ESP32
    #error "Not an ESP32 board. Make sure the correct board is selected."
#endif

#ifndef TOUCH_SETUP_LOADED
    #include "Touch_Setup.h"
#endif

//##############################
// Set defaults if not specified
//##############################
#ifndef aXM
    #define aXM 35  // analog input pin connected to LCD_RS 
#endif
#ifndef aYP
    #define aYP 39  // analog input pin connected to LCD_WR
#endif
#ifndef NOISE_LEVEL
    #define NOISE_LEVEL 4
#endif
#ifndef ADC_RESOLUTION
    #define ADC_RESOLUTION 10
#endif
#ifndef GRID_POINTS_X
    #define GRID_POINTS_X 2
#endif
#ifndef GRID_POINTS_Y
    #define GRID_POINTS_Y 2
#endif
#ifndef GRID_X
    #define GRID_X {65535, 0}
#endif
#ifndef GRID_Y
    #define GRID_Y {65535, 0}
#endif
//##############################
// END

typedef volatile uint32_t RwReg;
#define USE_FAST_PINIO
#define ADC_MAX ((1 << ADC_RESOLUTION)-1) // maximum value for ESP32 ADC (default 11db, 12 bits)

class TSPoint {
public:
    TSPoint(void);
    TSPoint(int16_t x, int16_t y, int16_t z);
  
    bool operator==(TSPoint);
    bool operator!=(TSPoint);

    int16_t x, y, z;
};

class TouchScreen {
public:
    TouchScreen(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym, uint16_t rx = 0);

    bool getTouchRaw(uint16_t &x, uint16_t &y, uint16_t &z);
    void remap(uint16_t &x, uint16_t &y, uint16_t x_max, uint16_t y_max);
    TSPoint getPoint(uint16_t x_max = 4095, uint16_t y_max = 4095);
    uint16_t pressure();
    int readTouchY();
    int readTouchX();

    void savePinstate();
    void restorePinstate();
    void enableRestore(){ restore = true; }
    void disableRestore(){ restore = false; }

private:
    uint8_t _yp, _ym, _xm, _xp;
    uint16_t _rxplate;

    bool restore = false;

    RwReg oldMode = 0;
    RwReg oldState = 0;

    #ifndef POLYNOMIAL
    // Each element in the array encodes the ADC value at one of 16 points.
    uint16_t grid_x[GRID_POINTS_X] = GRID_X;
    uint16_t grid_y[GRID_POINTS_Y] = GRID_Y;
    #endif

#ifdef USE_FAST_PINIO
    RwReg xp_pin, xm_pin, yp_pin, ym_pin;
    void gpioMode(uint8_t gpio, uint8_t mode);
    void setPin(RwReg mask);
    void clearPin(RwReg mask);
#endif
};

#endif
