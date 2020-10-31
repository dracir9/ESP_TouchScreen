
#include "Blackboard.h"

// adjust pressure sensitivity - note works 'backwards'
#define MINPRESSURE 200
#define MAXPRESSURE 10000

#define TOUCH_PIN_YP 33
#define TOUCH_PIN_XM 15
#define TOUCH_PIN_YM 13
#define TOUCH_PIN_XP 12

TFT_eSPI tft = TFT_eSPI();
TouchScreen ts(TOUCH_PIN_XP, TOUCH_PIN_YP, TOUCH_PIN_XM, TOUCH_PIN_YM, 300);

void waitTouch()
{
    uint16_t z = ts.pressure();
    while(z > MINPRESSURE && z < MAXPRESSURE) z = ts.pressure();
    while(!(z > MINPRESSURE && z < MAXPRESSURE)) z = ts.pressure();
}

void waitRelase()
{
    uint16_t z = ts.pressure();
    while(!(z > MINPRESSURE && z < MAXPRESSURE)) z = ts.pressure();
    while(z > MINPRESSURE && z < MAXPRESSURE) z = ts.pressure();
}

void initialize()
{
    tft.setCursor(0, 10);
    tft.drawRect(0, 8, 200, 110, TFT_RED);
    tft.println("Use a stylus or something");
    tft.println("similar to draw in the screen.");
    tft.println("Touch anywhere to continue");

    waitRelase();
}

void setup()
{
    Serial.begin(9600);
    tft.begin();
    ts.enableRestore();

    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);

    initialize();
    tft.fillScreen(TFT_BLACK);
}

uint32_t brushColor = TFT_YELLOW;
void loop()
{
    // display touched point with colored dot
    TSPoint p =ts.getPoint(tft.width(), tft.height());
    if (p.z > MINPRESSURE && p.z < MAXPRESSURE)
    {
        tft.fillCircle(p.x, p.y, 2, brushColor);
        // Serial.print("X: ");
        // Serial.print(x);
        // Serial.print(" Y: ");
        // Serial.println(y);
        // Serial.println(p.z);
    }
}
