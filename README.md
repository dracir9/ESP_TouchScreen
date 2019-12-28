# ESP_TouchScreen

This is a 4-wire resistive touch screen library for ESP32.

Added modification to allow the 4-wire resistive touchscreen of LCDs with parallel data interfaces to be used with ESP32 UNO style boards whilst WiFi is enabled.


## Reason for Modification

ESP32 WiFi disables ADC2 channel so some pins attached to the touchscreen no longer have analog input capability. We alternative pins to perform analog read without problems but requires additional wiring. Any pin from 32 to 39 may be used but pins from 34 to 39 are recomended. Pins 34 to 39 are input only so always present a high impedance to avoid the risk of two outputs shorting.

## Prerequisites

An extra wiring mod is needed in addition to those shown in the [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) instructions, but do not affect the software functionality or configuration.

Wiring for ESP UNO type board, with extra wire shown in green:

![image1](extras/wiring.jpg)   
   


## Installing

Download and install the library using your IDE, eg Arduino or PlatformIO. 
If using Arduino IDE navigate to your library folder, open TouchSetup.h and set your preferences. Defaults should work for most common configurations.

If using PlatformIO we recomment using "-build_flags" in your platformio.ini file.


## Using

No changes are required to existing sketches, just recompilation.

Compatible with both [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) and [MCUFRIEND_kbv](https://github.com/prenticedavid/MCUFRIEND_kbv/) libraries

Touchscreen needs to be calibrated before use! We recomment using included [touch calibration sketch](examples/touch_calib) 

![image3](extras/example.jpg)   
