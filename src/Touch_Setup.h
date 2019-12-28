
#ifndef TOUCH_SETUP_H
#define TOUCH_SETUP_H

//#########################
// Configuration
//#########################
    #define ESP32_WIFI_TOUCH // uncomment to use parallel MCU Friend LCD touchscreen with ESP32 UNO Wifi
    /*
    ESP32 WiFi disables ADC2 channel so some pins attached to the touchscreen no longer have
    analog input capability. This provides alternative pins to perform analog read without problems but requires additional wiring.
    Any pin from 32 to 39 may be used but pins from 34 to 39 are recomended.
    Pins 34 to 39 are input only so always present a high impedance to avoid the risk of two outputs shorting.
    */
    #ifdef ESP32_WIFI_TOUCH
        #define aXM 35  // analog input pin connected to LCD_RS 
        #define aYP 39  // analog input pin connected to LCD_WR
    #endif

    #define ADC_MAX 4095  // maximum value for ESP32 ADC (default 11db, 12 bits)
    #define NOISE_LEVEL 4  // Allow small amount of measurement noise

//#########################
// Mapping
//#########################
    #define MAP_TOUCH // Uncomment to enable mapping between raw analog read and screen coordinates
    #ifdef MAP_TOUCH
        // Uncomment to select mapping method
        // SELECT ONLY ONE
        #define LINEAR_MAP        // Linear interpolation
        #define POLYNOMIAL_MAP    // Polinomial interpolation


        #if defined(LINEAR_MAP)
            #define GRID_POINTS_X 3
            #define GRID_POINTS_Y 4

            /*
              o----o----o
              |    |    |
              o----o----o
              |    |    |
            Y o----o----o
            ^ |    |    |
            | o----o----o
            0--> X
            */


            #define GRID_X {3700, 1900, 600}
            #define GRID_Y {4000, 2900, 1200, 400}
        #elif defined(POLYNOMIAL_MAP)
            #define POLYNOMIAL_GRADE 2
            // k1*x^n + k2*x^(n-1) + k3*x^(n-2) +...+ k(n-1)*x + kn
        #endif
    #endif

#endif
