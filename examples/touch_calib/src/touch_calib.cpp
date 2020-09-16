
#include <TouchScreen.h>
#include <TFT_eSPI.h>

// adjust pressure sensitivity - note works 'backwards'
#define MINPRESSURE 200
#define MAXPRESSURE 10000
#define TOLERANCE 5


#define TOUCH_PIN_YP 33
#define TOUCH_PIN_XM 15
#define TOUCH_PIN_YM 13
#define TOUCH_PIN_XP 12

TFT_eSPI tft = TFT_eSPI();
TouchScreen ts(TOUCH_PIN_XP, TOUCH_PIN_YP, TOUCH_PIN_XM, TOUCH_PIN_YM, 300);
uint16_t pointsX[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint16_t pointsY[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t divX;
uint8_t divY;

float Xcoef[3];
float Ycoef[3];

void drawMark(uint16_t x, uint16_t y)
{
    tft.drawRect(x-6, y-6, 13, 13, TFT_WHITE);
    tft.drawFastHLine(x-4, y, 9, TFT_WHITE);
    tft.drawFastVLine(x, y-4, 9, TFT_WHITE);
}

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

void fail()
{
    uint16_t x = tft.width()/2 -105;
    uint16_t y = tft.height()/2 -25;
    tft.fillScreen(TFT_BLACK);
    tft.fillRect(x, y, 210, 50, TFT_RED);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.setCursor(x+90,y+6);
    tft.println("ERROR\n");
    tft.setCursor(x+7, tft.getCursorY());
    tft.println("Failed to calibrate touch screen.");
    tft.setCursor(x+4, tft.getCursorY());
    tft.println("Please check wiring and try again.");

    Serial.println("############");
    Serial.println("   ERROR:");
    Serial.println("############");
    Serial.println("Failed to calibrate touch screen.");
    Serial.println("Please check wiring and try again.");
    while(true);
}

void keepHolding()
{
    uint16_t x = tft.width()/2 -115;
    uint16_t y = tft.height()/2 -25;
    tft.fillScreen(TFT_BLACK);
    tft.fillRect(x, y, 230, 70, 0xCE00);
    tft.setTextColor(TFT_WHITE, 0xCE00);
    tft.setCursor(x+95,y+6);
    tft.println("WARNING\n");
    tft.setCursor(x+5, tft.getCursorY());
    tft.println("It seems you relased the touchscreen");
    tft.setCursor(x+25, tft.getCursorY());
    tft.println("before completing the reading.\n");
    tft.setCursor(x+50, tft.getCursorY());
    tft.println("Please keep holding");
    tft.setCursor(x+40, tft.getCursorY());
    tft.println("until screen turns red.");

    Serial.println("##############");
    Serial.println("   WARNING:");
    Serial.println("##############");
    Serial.println("It seems you relased the touchscreen before completing the reading");
    Serial.println("Please keep holding until screen turns red.");
    Serial.println("Tab the screen to continue.");
}

void initialize()
{
    tft.setCursor(0, 10);
    tft.drawRect(0, 8, 200, 110, TFT_RED);
    tft.println("Use a stylus or something");
    tft.println("similar to touch as close");
    tft.println("to the center of the");
    tft.println("crosshair as possible");
    tft.println("Keep pressing until");
    tft.println("the crosshair is removed");
    tft.println("Repeat for all");
    tft.println("crosshairs in sequence.\n");
    tft.println("Report can be pasted from Serial\n");
    tft.println("Touch anywhere to continue");

    waitRelase();
}

TSPoint readValues(uint16_t posX, uint16_t posY)
{
    uint16_t cnt, x, y, z, invalid, released;
    uint32_t MX, MY;
    MX = MY = 0;
    x = y = z = cnt = 0;

    invalid = 0;
    while (cnt < 6400)
    {
        released = cnt = 0;
        tft.fillScreen(0x01E0);
        drawMark(posX, posY);

        waitTouch();

        while (cnt < 6400)
        {
            if (ts.getTouchRaw(x, y, z))
            {
                if (z > MINPRESSURE && z < MAXPRESSURE)
                {
                    MX += x;
                    MY += y;
                    cnt++;
                }
                else
                {
                    released++;
                }
            }
            else
            {
                invalid++;
            }

            if (released > 1000)
            {
                keepHolding();
                waitTouch();
                break;
            }
            else if (invalid > 6400) 
            {
                fail();
            }
        }
    }

    tft.fillScreen(TFT_RED);
    waitRelase();

    return TSPoint(MX/cnt, MY/cnt, z);
}

void deletePoint(uint8_t strt, uint16_t (&arr)[16], uint8_t num = 1)
{
    uint8_t i;
    if (num < 1) return;
    for (i = strt; i < (16 - num); i++)
    {
        arr[i] = arr[i+num];
    }
    for (i = 0; i < num; i++)
    {
        arr[15-i] = 0;
    }
}

void insertValue(uint16_t val, uint16_t (&arr)[16])
{
    int8_t i = 0;
    while(val < arr[i] && i < 16) i++;
    
    for (int8_t j = 14; j >= i; j--)
    {
        arr[j+1] = arr[j];
    }
    
    arr[i] = val;
}

void printResults()
{
    #ifdef PLATFORMIO
        Serial.println("If using linear interpolation add this lines to build_flags");
        Serial.println("vvvvvvvvvvvv");
        Serial.printf("-D GRID_POINTS_X=%d\n", divX+1);
        Serial.printf("-D GRID_POINTS_Y=%d\n", divY+1);
        Serial.print("-D GRID_X=\"");
    #else
        Serial.println("If using linear interpolation add this lines to Touch_Setup.h");
        Serial.println("vvvvvvvvvvvv");
        Serial.printf("#define GRID_POINTS_X %d\n", divX+1);
        Serial.printf("#define GRID_POINTS_Y %d\n", divY+1);
        Serial.print("#define GRID_X ");
    #endif

    Serial.print("{");
    for (uint8_t i = 0; i < divX; i++)
    {
        Serial.printf("%d, ", pointsX[i]);
    }
    Serial.printf("%d}", pointsX[divX]);
    
    #ifdef PLATFORMIO
        Serial.println("\"");
        Serial.print("-D GRID_Y=\"");
    #else
        Serial.print("\n#define GRID_X ");
    #endif

    Serial.print("{");
    for (uint8_t i = 0; i < divY; i++)
    {
        Serial.printf("%d, ", pointsY[i]);
    }
    Serial.printf("%d}", pointsY[divY]);

    #ifdef PLATFORMIO
        Serial.println("\"");
    #else
        Serial.println();
    #endif
    Serial.println("^^^^^^^^^^^^");

    #ifdef PLATFORMIO
        Serial.println("If using polynomial interpolation add this lines to build_flags");
        Serial.println("vvvvvvvvvvvv");
        Serial.println("-D POLYNOMIAL=1");
        Serial.printf("-D POLY_X=\"%.9eF*dx*dx + %.9eF*dx + %.9eF\"\n", Xcoef[2], Xcoef[1], Xcoef[0]);
        Serial.printf("-D POLY_Y=\"%.9eF*dy*dy + %.9eF*dy + %.9eF\"\n", Ycoef[2], Ycoef[1], Ycoef[0]);
        Serial.println("^^^^^^^^^^^^");
    #else
        Serial.println("If using polynomial interpolation add this lines to Touch_Setup.h");
        Serial.println("vvvvvvvvvvvv");
        Serial.println("-D POLYNOMIAL=1");
        Serial.printf("-D POLY_X=\"%f*dx*dx + %f*dx + %f\"\n", Xcoef[0], Xcoef[1], Xcoef[2]);
        Serial.printf("-D POLY_Y=\"%f*dy*dy + %f*dy + %f\"\n", Ycoef[0], Ycoef[1], Ycoef[2]);
        Serial.println("^^^^^^^^^^^^");
    #endif
}

void optimizeSamples()
{
    // Sample reduction
    uint8_t i = 0;
    uint8_t k = 2;
    while(i+k <= divX)
    {
        uint16_t t0 = pointsX[i] >> 4;
        uint16_t t1 = pointsX[i+k] >> 4;
        uint16_t s0 = (pointsX[i]&15)*(tft.width()-1)/15;
        uint16_t s1 = (pointsX[i+k]&15)*(tft.width()-1)/15;

        for (uint8_t j = 1; j < k; j++)
        {
            uint16_t tx = pointsX[i+j] >> 4;
            uint16_t sx = (pointsX[i+j]&15)*(tft.width()-1)/15;
            if (abs(map(tx, t0, t1, s0, s1) - sx) > TOLERANCE)
            {
                deletePoint(i+1, pointsX, k-2);
                divX -= k-2;
                i++;
                k = 1;
                break;
            }
        }
        k++;
    }
    deletePoint(i+1, pointsX, divX-i-1);
    divX -= divX-i-1;

    while(i+k <= divY)
    {
        uint16_t t0 = pointsY[i] >> 4;
        uint16_t t1 = pointsY[i+k] >> 4;
        uint16_t s0 = (pointsY[i]&15)*(tft.height()-1)/15;
        uint16_t s1 = (pointsY[i+k]&15)*(tft.height()-1)/15;

        for (uint8_t j = 1; j < k; j++)
        {
            uint16_t ty = pointsY[i+j] >> 4;
            uint16_t sy = (pointsY[i+j]&15)*(tft.height()-1)/15;
            if (abs(map(ty, t0, t1, s0, s1) - sy) > TOLERANCE)
            {
                deletePoint(i+1, pointsY, k-2);
                divY -= k-2;
                i++;
                k = 1;
                break;
            }
        }
        k++;
    }
    deletePoint(i+1, pointsY, divY-i-1);
    divY -= divY-i-1;
}

void calcCoef()
{
    const uint8_t dimX = divX+1;
    const uint8_t dimY = divY+1;
    double A[16][3];
    double B[3][3];
    double Binv[3][3];
    double C[3][16];

    ///#####################
    /// X axis calculations
    ///#####################

    // Fill matrix
    for (uint8_t i = 0; i < dimX; i++)
    {
        for (uint8_t j = 0; j < 3; j++)
        {
            A[i][j] = pow(pointsX[i]>>4, j);
        }
    }
    
    // Matrix multiplication At*A
    for (uint8_t i = 0; i < 3; i++)
    {
        for (uint8_t j = 0; j < 3; j++)
        {
            B[i][j] = 0;
            for (uint8_t k = 0; k < dimX; k++)
            {
                B[i][j] += A[k][i] * A[k][j];
            }
        }
    }

    // Calculate determinant
    double detB = B[0][0]*B[1][1]*B[2][2] + B[0][1]*B[1][2]*B[2][0] + B[0][2]*B[1][0]*B[2][1] 
                - B[0][2]*B[1][1]*B[2][0] - B[0][0]*B[1][2]*B[2][1] - B[1][0]*B[0][1]*B[2][2];
    
    if (detB == 0) return;

    // Adjoint of transposed
    Binv[0][0] = B[1][1]*B[2][2]-B[1][2]*B[2][1];
    Binv[0][1] = -B[0][1]*B[2][2]+B[0][2]*B[2][1];
    Binv[0][2] = B[0][1]*B[1][2]-B[0][2]*B[1][1];
    Binv[1][0] = -B[1][0]*B[2][2]+B[1][2]*B[2][0];
    Binv[1][1] = B[0][0]*B[2][2]-B[0][2]*B[2][0];
    Binv[1][2] = -B[0][0]*B[1][2]+B[0][2]*B[1][0];
    Binv[2][0] = B[1][0]*B[2][1]-B[1][1]*B[2][0];
    Binv[2][1] = -B[0][0]*B[2][1]+B[0][1]*B[2][0];
    Binv[2][2] = B[0][0]*B[1][1]-B[0][1]*B[1][0];

    // Calculate inverse
    for (uint8_t i = 0; i < 3; i++)
    {
        for (uint8_t j = 0; j < 3; j++)
        {
            Binv[i][j] /= detB;
        }
    }

    // (At*A)^-1 * At
    for (uint8_t i = 0; i < 3; i++)
    {
        for (uint8_t j = 0; j < dimX; j++)
        {
            C[i][j] = 0;
            for (uint8_t k = 0; k < 3; k++)
            {
                C[i][j] += Binv[i][k] * A[j][k];
            }
        }
    }
    
    // (At*A)^-1 * At * b
    for (uint8_t i = 0; i < 3; i++)
    {
        Xcoef[i] = 0;
        for (uint8_t k = 0; k < dimX; k++)
        {
            Xcoef[i] += C[i][k] * ((pointsX[k]&15)*(tft.width() - 1)/15);
        }
    }

    ///#####################
    /// Y axis calculations
    ///#####################

    // Fill matrix
    for (uint8_t i = 0; i < dimY; i++)
    {
        for (uint8_t j = 0; j < 3; j++)
        {
            A[i][j] = pow(pointsY[i]>>4, j);
        }
    }
    
    // Matrix multiplication At*A
    for (uint8_t i = 0; i < 3; i++)
    {
        for (uint8_t j = 0; j < 3; j++)
        {
            B[i][j] = 0;
            for (uint8_t k = 0; k < dimY; k++)
            {
                B[i][j] += A[k][i] * A[k][j];
            }
        }
    }

    // Calculate determinant
    detB = B[0][0]*B[1][1]*B[2][2] + B[0][1]*B[1][2]*B[2][0] + B[0][2]*B[1][0]*B[2][1] 
         - B[0][2]*B[1][1]*B[2][0] - B[0][0]*B[1][2]*B[2][1] - B[1][0]*B[0][1]*B[2][2];
    
    if (detB == 0) return;

    // Adjoint of transposed
    Binv[0][0] = B[1][1]*B[2][2]-B[1][2]*B[2][1];
    Binv[0][1] = -B[0][1]*B[2][2]+B[0][2]*B[2][1];
    Binv[0][2] = B[0][1]*B[1][2]-B[0][2]*B[1][1];
    Binv[1][0] = -B[1][0]*B[2][2]+B[1][2]*B[2][0];
    Binv[1][1] = B[0][0]*B[2][2]-B[0][2]*B[2][0];
    Binv[1][2] = -B[0][0]*B[1][2]+B[0][2]*B[1][0];
    Binv[2][0] = B[1][0]*B[2][1]-B[1][1]*B[2][0];
    Binv[2][1] = -B[0][0]*B[2][1]+B[0][1]*B[2][0];
    Binv[2][2] = B[0][0]*B[1][1]-B[0][1]*B[1][0];

    // Calculate inverse
    for (uint8_t i = 0; i < 3; i++)
    {
        for (uint8_t j = 0; j < 3; j++)
        {
            Binv[i][j] /= detB;
        }
    }

    // (At*A)^-1 * At
    for (uint8_t i = 0; i < 3; i++)
    {
        for (uint8_t j = 0; j < dimY; j++)
        {
            C[i][j] = 0;
            for (uint8_t k = 0; k < 3; k++)
            {
                C[i][j] += Binv[i][k] * A[j][k];
            }
        }
    }
    
    // (At*A)^-1 * At * b
    for (uint8_t i = 0; i < 3; i++)
    {
        Ycoef[i] = 0;
        for (uint8_t k = 0; k < dimY; k++)
        {
            Ycoef[i] += C[i][k] * ((pointsY[k]&15)*(tft.height() - 1)/15);
        }
    }
}

void calibrate()
{
    tft.fillScreen(TFT_BLACK);
    divX = min(tft.width()/50, 15);
    divY = min(tft.height()/50, 15);
    delay(100);

    // Initial measurements
    for (uint8_t i = 0; i <= divX; i++)
    {
        uint16_t x = i*15/divX*(tft.width() - 1)/15;

        insertValue((readValues(x, tft.height()/2).x << 4) + i*15/divX, pointsX);
        delay(100);
    }

    for (uint8_t i = 0; i <= divY; i++)
    {
        uint16_t y = i*15/divY*(tft.height() - 1)/15;

        insertValue((readValues(tft.width()/2, y).y << 4) + i*15/divY, pointsY);
        delay(100);
    }

    calcCoef();

    optimizeSamples();

    printResults();
}

void map2Screen(uint16_t &x, uint16_t &y)
{
    // Map X
    uint8_t i = 0;
    while (x < (pointsX[i + 1] >> 4) && i <= divX - 1) i++; 
    x = map(x, pointsX[i]>>4, pointsX[i + 1]>>4, (pointsX[i]&15) * tft.width()/15, (pointsX[i + 1]&15) * tft.width()/15);

    // Map Y
    i = 0;
    while (y < (pointsY[i + 1] >> 4) && i <= divY - 1) i++; 
    y = map(y, pointsY[i]>>4, pointsY[i + 1]>>4, (pointsY[i]&15) * tft.height()/15, (pointsY[i + 1]&15) * tft.height()/15);
}

void setup()
{
    Serial.begin(9600);
    tft.begin();
    ts.enableRestore();

    Serial.println("#########################");
    Serial.println("    Touch Calibration");
    Serial.println("#########################");

    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);

    initialize();
    calibrate();
    tft.fillScreen(TFT_BLACK);
}

uint32_t brushColor = TFT_YELLOW;
uint32_t cnt = 0;
int64_t trigger = 0;
void loop()
{
    // display touched point with colored dot
    uint16_t x,y,z;
    ts.getTouchRaw(x, y, z);
    if (z > MINPRESSURE && z < MAXPRESSURE)
    {
        map2Screen(x,y);
        tft.fillCircle(x, y, 2, brushColor);
        // Serial.print("X: ");
        // Serial.print(x);
        // Serial.print(" Y: ");
        // Serial.println(y);
        // Serial.println(p.z);
    }

    cnt++;
    if (esp_timer_get_time() > trigger)
    {
        trigger = esp_timer_get_time() + 1000000LL;
        printf("Readings per second: %d\n", cnt);
        cnt = 0;
    }
}
