
#ifndef TOUCH_SETUP_H
#define TOUCH_SETUP_H

//#########################
// Configuration
//#########################
    #define ESP32_WIFI_TOUCH // uncomment to allow Wifi usage
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

    #define ADC_RESOLUTION 10 // Resolution for ESP32 ADC (default 12 bits)
    #define NOISE_LEVEL 4  // Allow small amount of measurement noise

//#########################
// Mapping
//#########################
    #define POLYNOMIAL  // Uncomment to enable polynomial mapping between raw analog read and screen coordinates
                        // Default mode uses a lookup table (see below)
    #ifdef POLYNOMIAL
        #define POLY_X 7.393350097e-05F*dx*dx + 3.308962286e-01F*dx + -5.089157104e+01F
        #define POLY_Y -1.246797328e-04F*dy*dy + -4.144772887e-01F*dy + 5.260565186e+02F
    #else
        // Lookup table mapping betwen raw analog read and screen coordinates
        // It works by having a set of points with known screen coordinates and analog readings.
        // This data is encoded in uint16 so that bit number 
        // MSB |15|14|13|12|11|10| 9| 8| 7| 6| 5| 4 | 3 | 2 | 1 | 0 |LSB
        //     |^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^|^^^^^^^^^^^^^^^|
        //     |>            ADC reading           <|> Corresponding point in screen <|
        // Each point from 0-15 correspond to coordinates x = point*("LCD_dimensions_X" - 1)/15

        /*
          0,3----1,3----2,3
           |       |      |
          0,2----1,2----2,2
           |       |      |
        Y 0,1----1,1----2,1
        ^  |       |      |
        | 0,0----1,0----2,0
        0--> X
        */

        #define GRID_POINTS_X 3
        #define GRID_POINTS_Y 4
        #define GRID_X {3700, 1900, 600}
        #define GRID_Y {4000, 2900, 1200, 400}
    #endif
#endif
