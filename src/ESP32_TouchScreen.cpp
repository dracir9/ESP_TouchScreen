
#include "ESP32_TouchScreen.h"


#define NUMSAMPLES 2

TSPoint::TSPoint(void)
{
    x = y = 0;
}

TSPoint::TSPoint(int16_t x0, int16_t y0, int16_t z0)
{
    x = x0;
    y = y0;
    z = z0;
}

bool TSPoint::operator==(TSPoint p1)
{
    return  ((p1.x == x) && (p1.y == y) && (p1.z == z));
}

bool TSPoint::operator!=(TSPoint p1)
{
    return  ((p1.x != x) || (p1.y != y) || (p1.z != z));
}

#if (NUMSAMPLES > 2)
static void insert_sort(int array[], uint8_t size)
{
    uint8_t j;
    int save;
    
    for (int i = 1; i < size; i++)
    {
        save = array[i];
        for (j = i; j >= 1 && save < array[j - 1]; j--)
            array[j] = array[j - 1];
        array[j] = save; 
    }
}
#endif

//
// Touch Screen
//

#ifdef USE_FAST_PINIO
/***************************************************************************************
** Function name:           setPin
** Description:             Sets the specified pins high
***************************************************************************************/
void TouchScreen::setPin(RwReg mask)
{
    GPIO.out_w1ts = mask & ~3;
    GPIO.out1_w1ts.data = mask & 3;
}

/***************************************************************************************
** Function name:           clearPin
** Description:             Sets the specified pins low (Supports pins 2-33)
***************************************************************************************/
void TouchScreen::clearPin(RwReg mask)
{
    GPIO.out_w1tc = mask & ~3;
    GPIO.out1_w1tc.data = mask & 3;
}

/***************************************************************************************
** Function name:           gpioMode
** Description:             Set pin to input or output
***************************************************************************************/
#include "driver/rtc_io.h"
void TouchScreen::gpioMode(uint8_t gpio, uint8_t mode)
{
    uint32_t rtc_reg = rtc_io_desc[rtc_io_num_map[gpio]].reg;

    //RTC pins PULL settings
    if(rtc_reg) {
        ESP_REG(rtc_reg) = ESP_REG(rtc_reg) & ~(rtc_io_desc[rtc_io_num_map[gpio]].pullup | rtc_io_desc[rtc_io_num_map[gpio]].pulldown);
    }

    if(gpio < 32)
    {
        if(mode == INPUT) GPIO.enable_w1tc = ((uint32_t)1 << gpio);
        else GPIO.enable_w1ts = ((uint32_t)1 << gpio);
    }
    else
    {
        gpio -= 32;
        if(mode == INPUT) GPIO.enable1_w1tc.data = ((uint32_t)1 << gpio);
        else GPIO.enable1_w1ts.data = ((uint32_t)1 << gpio);
        // Restore original gpio number...
        gpio += 32;
    }

    if(mode == INPUT)
    ESP_REG(GPIO_PIN_MUX_REG[gpio]) = ((uint32_t)2 << FUN_DRV_S) | (FUN_IE) | ((uint32_t)2 << MCU_SEL_S);
    else
    ESP_REG(GPIO_PIN_MUX_REG[gpio]) = ((uint32_t)2 << FUN_DRV_S) | ((uint32_t)2 << MCU_SEL_S);
    GPIO.pin[gpio].val = 0;
}
#endif

void TouchScreen::savePinstate()
{
    if (!restore) return;
    oldMode = GPIO.enable & ~3;
    oldMode |= GPIO.enable1.data & 3;

    oldState = GPIO.out & ~3;
    oldState |= GPIO.out1.data & 3;
}

void TouchScreen::restorePinstate()
{
    if (!restore) return;
    ((oldMode & yp_pin) > 0) ? gpioMode(_yp, OUTPUT) : gpioMode(_yp, INPUT);
    ((oldMode & ym_pin) > 0) ? gpioMode(_ym, OUTPUT) : gpioMode(_ym, INPUT);
    ((oldMode & xp_pin) > 0) ? gpioMode(_xp, OUTPUT) : gpioMode(_xp, INPUT);
    ((oldMode & xm_pin) > 0) ? gpioMode(_xm, OUTPUT) : gpioMode(_xm, INPUT);
    setPin(oldState & (yp_pin | ym_pin | xp_pin | xm_pin));
    clearPin(~oldState & (yp_pin | ym_pin | xp_pin | xm_pin));
}

/***************************************************************************************
** Function name:           remap
** Description:             Modify x, y coordinates to compensate non linearity errors
***************************************************************************************/
void TouchScreen::remap(uint16_t &x, uint16_t &y, uint16_t x_max, uint16_t y_max)
{
#ifndef POLYNOMIAL
    // Map X
    uint8_t i = 0;
    int in_min, out_min;
    while (x < (grid_x[i + 1] >> 4) && i < GRID_POINTS_X - 1) i++; 
    //x = map(x, grid_x[i]>>4, grid_x[i + 1]>>4, (grid_x[i]&15) * x_max/15, (grid_x[i + 1]&15) * x_max/15);
    in_min = grid_x[i]>>4;
    out_min = (grid_x[i++]&15) * x_max/15;
    
    int divisor = ((grid_x[i]>>4) - in_min);
    x = (x - in_min) * ((grid_x[i]&15) * x_max/15 - out_min) / divisor + out_min;

    // Map Y
    i = 0;
    while (y < (grid_y[i + 1] >> 4) && i < GRID_POINTS_Y - 1) i++; 
    //y = map(y, grid_y[i]>>4, grid_y[i + 1]>>4, (grid_y[i]&15) * y_max/15, (grid_y[i + 1]&15) * y_max/15);
    in_min = grid_y[i]>>4;
    out_min = (grid_y[i++]&15) * y_max/15;
    
    divisor = ((grid_y[i]>>4) - in_min);
    y = (y - in_min) * ((grid_y[i]&15) * y_max/15 - out_min) / divisor + out_min;
#else
        float dx = x;
        float dy = y;
        x = POLY_X;
        y = POLY_Y;
#endif
}

/***************************************************************************************
** Function name:           getTouchRaw
** Description:             read raw touch position.  Always returns true.
***************************************************************************************/
bool TouchScreen::getTouchRaw(uint16_t &x, uint16_t &y, uint16_t &z)
{
    uint16_t samples[NUMSAMPLES];
    uint8_t i;
    bool valid;

    valid = true;

    // Save state of pins to allow restoring them
    savePinstate();

    gpioMode(_yp, INPUT);
    gpioMode(_ym, INPUT);
    gpioMode(_xp, OUTPUT);
    gpioMode(_xm, OUTPUT);

#if defined (USE_FAST_PINIO)
    setPin(xp_pin);
    clearPin(xm_pin);
#else
    digitalWrite(_xp, HIGH);
    digitalWrite(_xm, LOW);
#endif

    for (i=0; i<NUMSAMPLES; i++)
    {
#if defined (ESP32_WIFI_TOUCH)
        samples[i] = analogRead(aYP);
#else
        samples[i] = analogRead(_yp);
#endif
    }

#if NUMSAMPLES > 2
    insert_sort(samples, NUMSAMPLES);
#endif
#if NUMSAMPLES == 2
    // Allow small amount of measurement noise, because capacitive
    // coupling to a TFT display's signals can induce some noise.
    if (samples[0] - samples[1] < -NOISE_LEVEL || samples[0] - samples[1] > NOISE_LEVEL) {
        valid = false;
    } else {
        samples[1] = (samples[0] + samples[1]) >> 1; // average 2 samples
    }
#endif

    x = (ADC_MAX-samples[NUMSAMPLES/2]);

//############################################################
// LCD Critical pin change
// _xm -> INPUT
//############################################################

    gpioMode(_xp, INPUT);
    gpioMode(_xm, INPUT);
    gpioMode(_yp, OUTPUT);
    gpioMode(_ym, OUTPUT);

#if defined (USE_FAST_PINIO)
    setPin(yp_pin);
    clearPin(ym_pin);
#else
    digitalWrite(_ym, LOW);
    digitalWrite(_yp, HIGH);
#endif

    for (i=0; i<NUMSAMPLES; i++)
    {
    #if defined (ESP32_WIFI_TOUCH)
        samples[i] = analogRead(aXM);
    #else
        samples[i] = analogRead(_xm);
    #endif
    }

#if NUMSAMPLES > 2
    insert_sort(samples, NUMSAMPLES);
#endif
#if NUMSAMPLES == 2
    // Allow small amount of measurement noise, because capacitive
    // coupling to a TFT display's signals can induce some noise.
    if (samples[0] - samples[1] < -NOISE_LEVEL || samples[0] - samples[1] > NOISE_LEVEL) {
        valid = false;
    } else {
        samples[1] = (samples[0] + samples[1]) >> 1; // average 2 samples
    }
#endif

    y = (ADC_MAX-samples[NUMSAMPLES/2]);

//############################################################
// LCD Critical pin change
// _yp -> INPUT
//############################################################
    // Set X+ to ground
    // Set Y- to VCC
    // Hi-Z X- and Y+
    gpioMode(_xp, OUTPUT);
    gpioMode(_yp, INPUT);

#if defined (USE_FAST_PINIO)
    setPin(ym_pin);
    clearPin(xp_pin);
#else
    digitalWrite(_xp, LOW);
    digitalWrite(_ym, HIGH); 
#endif

#if defined (ESP32_WIFI_TOUCH)
    int z1 = analogRead(aXM); 
    int z2 = analogRead(aYP);
#else
    int z1 = analogRead(_xm); 
    int z2 = analogRead(_yp);
#endif

    if (_rxplate != 0)
    {
        float rtouch;
        rtouch = z2;
        rtouch /= z1;
        rtouch -= 1;
        rtouch *= x;
        rtouch *= _rxplate;
        rtouch /= ADC_MAX+1;

        z = rtouch;
    } else {
        z = (ADC_MAX-(z2-z1));
    }

    // Restore shared TFT pins if needed
    restorePinstate();
    return valid;
}

TSPoint TouchScreen::getPoint(uint16_t x_max, uint16_t y_max)
{
    uint16_t x, y, z;
    bool valid = getTouchRaw(x,y,z);
    /* Mapping */
    remap(x, y, x_max, y_max);

    if (!valid) z = 0;

    return TSPoint(x,y,z);
}

TouchScreen::TouchScreen(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym, uint16_t rxplate)
{
    _yp = yp;
    _xm = xm;
    _ym = ym;
    _xp = xp;
    _rxplate = rxplate;

#if defined (USE_FAST_PINIO)
    xp_pin = digitalPinToBitMask(_xp);
    yp_pin = digitalPinToBitMask(_yp);
    xm_pin = digitalPinToBitMask(_xm);
    ym_pin = digitalPinToBitMask(_ym);

    if(_xp < 2 || _yp < 2 || _xm < 2 || _ym < 2)
    {
        Serial.println("WARNING: Using unexpected pins for touch screen. Undefined behavior may occur");
        xp_pin = yp_pin = xm_pin = ym_pin = 0;
    }
#endif
    analogReadResolution(ADC_RESOLUTION);
}

int TouchScreen::readTouchX(void)
{
    savePinstate();

    gpioMode(_yp, INPUT);
    gpioMode(_ym, INPUT);
    gpioMode(_xp, OUTPUT);
    gpioMode(_xm, OUTPUT);
#if defined (USE_FAST_PINIO)
    setPin(xp_pin);
    clearPin(xm_pin | ym_pin | yp_pin);
#else
    digitalWrite(_yp, LOW);
    digitalWrite(_ym, LOW);
    digitalWrite(_xp, HIGH);
    digitalWrite(_xm, LOW);
#endif
    
#if defined (ESP32_WIFI_TOUCH)
    uint16_t val = (ADC_MAX-analogRead(aYP));
#else
    uint16_t val = (ADC_MAX-analogRead(_yp));
#endif

    restorePinstate();

    return val;
}


int TouchScreen::readTouchY(void)
{
    savePinstate();

    gpioMode(_xp, INPUT);
    gpioMode(_xm, INPUT);
    gpioMode(_yp, OUTPUT);
    gpioMode(_ym, OUTPUT);
    
#if defined (USE_FAST_PINIO)
    setPin(yp_pin);
    clearPin(xm_pin | ym_pin | xp_pin);
#else
    digitalWrite(_xp, LOW);
    digitalWrite(_xm, LOW);
    digitalWrite(_yp, HIGH);
    digitalWrite(_ym, LOW);
#endif

#if defined (ESP32_WIFI_TOUCH)
    uint16_t val = (ADC_MAX-analogRead(aXM));
#else
    uint16_t val = (ADC_MAX-analogRead(_xm));
#endif

    restorePinstate();

    return val;
}


uint16_t TouchScreen::pressure(void)
{
    savePinstate();

    // Set X+ to ground
    // Set Y- to VCC
    // Hi-Z X- and Y+
    gpioMode(_xp, OUTPUT);
    gpioMode(_ym, OUTPUT);
    gpioMode(_xm, INPUT);
    gpioMode(_yp, INPUT);

#if defined (USE_FAST_PINIO)
    setPin(ym_pin);
    clearPin(xp_pin | xm_pin | yp_pin);
#else
    digitalWrite(_xp, LOW);
    digitalWrite(_ym, HIGH); 
    digitalWrite(_xm, LOW);
    digitalWrite(_yp, LOW);
#endif

#if defined (ESP32_WIFI_TOUCH)
    int z1 = analogRead(aXM); 
    int z2 = analogRead(aYP);
#else
    int z1 = analogRead(_xm); 
    int z2 = analogRead(_yp);
#endif

    restorePinstate();

    if (_rxplate != 0)
    {
        // now read the x 
        float rtouch;
        rtouch = z2;
        rtouch /= z1;
        rtouch -= 1;
        rtouch *= readTouchX();
        rtouch *= _rxplate;
        rtouch /= ADC_MAX+1;
    
        return rtouch;
    } else {
        return (ADC_MAX-(z2-z1));
    }
}
