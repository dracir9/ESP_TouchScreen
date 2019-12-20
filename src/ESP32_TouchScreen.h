// Touch screen library with X Y and Z (pressure) readings as well
// as oversampling to avoid 'bouncing'
// (c) ladyada / adafruit
// Code under MIT License

#ifndef _ADAFRUIT_TOUCHSCREEN_H_
#define _ADAFRUIT_TOUCHSCREEN_H_
#include <stdint.h>
#include <Arduino.h>

//#########################
// ESP32 specific configuration
//#########################
#ifndef USER_SETUP_LOADED
    #define ESP32_WIFI_TOUCH // uncomment to use parallel MCU Friend LCD touchscreen with ESP32 UNO Wifi
    #ifdef ESP32 
        #define ADC_MAX 4095  // maximum value for ESP32 ADC (default 11db, 12 bits)
        #define aXM 35  // analog input pin connected to LCD_RS 
        #define aYP 39  // analog input pin connected to LCD_WR
    #else
    #endif 
    #define NOISE_LEVEL 4  // Allow small amount of measurement noise
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

//#########################################################
// END

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega32U4__) || defined(TEENSYDUINO) || defined(__AVR_ATmega2560__)
typedef volatile uint8_t RwReg;
#elif defined(ARDUINO_STM32_FEATHER)
typedef volatile uint32 RwReg;
#elif defined(NRF52_SERIES) || defined(ESP32) || defined(ARDUINO_ARCH_STM32)
typedef volatile uint32_t RwReg;
#endif

#if defined (__AVR__) || defined(TEENSYDUINO) || defined(ARDUINO_ARCH_SAMD) || defined(ESP32)
    #define USE_FAST_PINIO
#endif

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
    TouchScreen(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym, uint16_t rx);

    bool getTouchRaw(uint16_t *x, uint16_t *y, uint16_t *z);
    uint16_t pressure(void);
    int readTouchY();
    int readTouchX();
    TSPoint getPoint();
    int16_t pressureThreshhold;

private:
    uint8_t _yp, _ym, _xm, _xp;
    uint16_t _rxplate;

#ifdef ESP32
    bool init = true;
#endif

#ifdef USE_FAST_PINIO
    #ifdef ESP32
        RwReg xp_pin, xm_pin, yp_pin, ym_pin;
        void setPin(RwReg mask)
        {
            GPIO.out_w1ts = mask & ~3;
            GPIO.out1_w1ts.data = mask & 3;
        }
        void clearPin(RwReg mask)
        {
            GPIO.out_w1tc = mask & ~3;
            GPIO.out1_w1tc.data = mask & 3;
        }
    #else
        volatile RwReg *xp_port, *yp_port, *xm_port, *ym_port;
        RwReg xp_pin, xm_pin, yp_pin, ym_pin;
    #endif
#endif
};

#endif
