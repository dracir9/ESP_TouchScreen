
#include <TouchScreen.h>
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

// adjust pressure sensitivity - note works 'backwards'
#define MINPRESSURE 200
#define MAXPRESSURE 1000

void setup() {
  Serial.begin(115200);
  tft.begin();

  
  Serial.println("#########################");
  Serial.println("    Touch Calibration");
  Serial.println("#########################");

  // screen orientation and background
  String orientation;
  switch (rotation) {
    case 0: 
      orientation = "Portrait";
    break;
    case 1: 
      orientation = "Landscape";
    break;
    case 2: 
      orientation = "Portrait Inverted";
    break;
    case 3: 
      orientation = "Landscape Inverted";
    break;
  }
  Serial.println(orientation);
  tft.setRotation(rotation);  
  tft.fillScreen(BLACK);
}

void loop() {
  // display touched point with colored dot
  uint16_t pixel_x, pixel_y;
  if (touchMapXY(&pixel_x, &pixel_y))
  {
      tft.fillCircle(pixel_x, pixel_y, 2, brushColor);
      //Serial.print("X: ");
      //Serial.print(pixel_x);
      //Serial.print("Y: ");
      //Serial.println(pixel_y);
  }
}
