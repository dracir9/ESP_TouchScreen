
#include <ESP32_TouchScreen.h>
#include <TFT_eSPI.h>

// adjust pressure sensitivity - note works 'backwards'
#define MINPRESSURE 200
#define MAXPRESSURE 10000


#define TOUCH_PIN_YP 33
#define TOUCH_PIN_XM 15
#define TOUCH_PIN_YM 13
#define TOUCH_PIN_XP 12

TFT_eSPI tft = TFT_eSPI();
TouchScreen ts(TOUCH_PIN_XP, TOUCH_PIN_YP, TOUCH_PIN_XM, TOUCH_PIN_YM, 300);

void drawBoard()
{
    tft.fillScreen(TFT_BLACK);
    tft.drawLine(0, 0, 40, 40, TFT_RED);
    tft.drawLine(0, 40, 40, 0, TFT_RED);
    tft.fillRect(40, 0, 40, 40, TFT_RED);
    tft.fillRect(80, 0, 40, 40, TFT_GREEN);
    tft.fillRect(120, 0, 40, 40, TFT_BLUE);
    tft.fillRect(160, 0, 50, 40, TFT_YELLOW);
    tft.fillRect(200, 0, 40, 40, TFT_BLACK);
    tft.fillRect(240, 0, 40, 40, TFT_ORANGE);
    tft.fillRect(280, 0, 40, 40, TFT_PINK);
}

void setup()
{
    Serial.begin(9600);
    tft.begin();
    ts.enableRestore();

    tft.setRotation(0);

    drawBoard();
}

uint32_t brushColor = TFT_YELLOW;
uint32_t cnt = 0;
int64_t trigger = 0;
void loop()
{
    // getPoint(x_max, y_max) reads the touchscreen and maps the returned x and y 
    // coordinates between 0 and x_max/y_max respectively.
    // If the touchscreen wasn't touched while reading the returned x and y values are invalid.

    TSPoint p = ts.getPoint(tft.width(), tft.height());
    
    if (p.z > MINPRESSURE && p.z < MAXPRESSURE) // Check if screen was touched
    {
        if (p.y < 40)
        {
          if (p.x < 40)
          {
            drawBoard();
          }
          else if (p.x < 80)
          {
            brushColor = TFT_RED;
          }
          else if (p.x < 120)
          {
            brushColor = TFT_GREEN;
          }
          else if (p.x < 160)
          {
            brushColor = TFT_BLUE;
          }
          else if (p.x < 200)
          {
            brushColor = TFT_YELLOW;
          }
          else if (p.x < 240)
          {
            brushColor = TFT_BLACK;
          }
          else if (p.x < 280)
          {
            brushColor = TFT_ORANGE;
          }
          else
          {
            brushColor = TFT_PINK;
          }
        }
        else
        {
          tft.fillCircle(p.x, p.y, 2, brushColor);
        }
        // Serial.print("X: ");
        // Serial.print(p.x);
        // Serial.print(" Y: ");
        // Serial.println(p.y);
        // Serial.println(p.z);
    }

    cnt++;
    if (esp_timer_get_time() > trigger)
    {
        trigger = esp_timer_get_time() + 1000000LL;
        Serial.printf("Readings per second: %d\n", cnt);
        cnt = 0;
    }
}
