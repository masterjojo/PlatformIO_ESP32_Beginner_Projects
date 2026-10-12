// add in relevant libraries

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <DHT22.h>


#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>

// create global variables

// screen pins
const int SCREEN_DATA_PIN         = 14; // DIN
const int SCREEN_CLOCK_PIN        = 27; // CLK
const int SCREEN_CHIP_SELECT_PIN  = 26; // CS
const int SCREEN_DATA_COMMAND_PIN = 25; // DC
const int SCREEN_RESET_PIN        = 33; // RST
const int SCREEN_BACKLIGHT_PIN    = 32; // BL

const uint16_t BACKGROUND_COLOR = 0xfdb8; //uint16_t type aka rgb565 color format; pink color
const uint16_t CARD_COLOR       = ST77XX_WHITE; // color of screen behind the texty
const uint16_t TITLE_COLOR      = ST77XX_BLACK; // color of text for title
const uint16_t TEXT_COLOR       = ST77XX_BLACK; // color of text for the rest of the display

// screen dimensions
const int SCREEN_WIDTH  = 128;
const int SCREEN_HEIGHT = 160;

// variable that represents the LCD
Adafruit_ST7735 screen = Adafruit_ST7735(SCREEN_CHIP_SELECT_PIN, SCREEN_DATA_COMMAND_PIN, SCREEN_DATA_PIN,
                                         SCREEN_CLOCK_PIN, SCREEN_RESET_PIN);

const int SENSOR_PIN = 22;
DHT22 sensor(SENSOR_PIN);

const int TEMPERATURE_LEFT  = 9;
const int TEMPERATURE_TOP   = 62;
GFXcanvas16 temperatureCanvas(110, 44);


const int DEGREE_SYMBOL   = 248;
const int BOTTOM_ROW_LEFT = 9;
const int BOTTOM_ROW_TOP  = 117;
GFXcanvas16 bottomRowCanvas(110, 12);

//function centers the text in the middle of the screen
void printCentered(const char* text, int y, int textSize, uint16_t color) {
  screen.setTextSize(textSize);
  screen.setTextColor(color);
  screen.setCursor((screen.width() - strlen(text) * 6 * textSize) / 2,y); // come back to to understand
  screen.print(text);
}


int canvasTextWidth(const char* text) {
  int16_t cornerX, cornerY;
  uint16_t width, height;
  temperatureCanvas.getTextBounds(text, 0, 0, &cornerX, &cornerY, &width, &height);
  return width;
}


void printOnCanvas(const char* text, int leftX, int topY) {
  int16_t cornerX, cornerY;
  uint16_t width, height;
  temperatureCanvas.getTextBounds(text, 0, 0, &cornerX, &cornerY, &width, &height);
  temperatureCanvas.setCursor(leftX - cornerX, topY - cornerY);
  temperatureCanvas.print(text);
}


void printTemperature(float degrees, const char* unit) {
  char numberText[8];
  sprintf(numberText, "%d", (int)round(degrees));
  
  temperatureCanvas.fillScreen(CARD_COLOR);
  temperatureCanvas.setTextColor(TEXT_COLOR);

  temperatureCanvas.setFont(&FreeSansBold24pt7b);
  int numberWidth = canvasTextWidth(numberText);

  temperatureCanvas.setFont(&FreeSans9pt7b);
  int letterWidth = canvasTextWidth(unit);

  int startX = (110 - (numberWidth + 10 + letterWidth)) / 2;

  temperatureCanvas.setFont(&FreeSansBold24pt7b);
  printOnCanvas(numberText, startX, 4);
  
  int ringX = startX + numberWidth + 5;
  temperatureCanvas.drawCircle(ringX, 10, 4, TEXT_COLOR);
  temperatureCanvas.drawCircle(ringX, 10, 3, TEXT_COLOR);

  temperatureCanvas.setFont(&FreeSans9pt7b);
  printOnCanvas(unit, ringX + 6, 6);

  screen.drawRGBBitmap(TEMPERATURE_LEFT, TEMPERATURE_TOP,
                      temperatureCanvas.getBuffer(), 110, 44);
}

void printSmallTemperature(float degrees, const char* unit) {
  char text[12];
  sprintf(text, "%d%c%s", (int)round(degrees), DEGREE_SYMBOL, unit);

  bottomRowCanvas.fillScreen(CARD_COLOR);
  bottomRowCanvas.cp437(true);
  bottomRowCanvas.setFont();
  bottomRowCanvas.setTextSize(1);
  bottomRowCanvas.setTextColor(TEXT_COLOR);
  bottomRowCanvas.setCursor(105 - strlen(text) * 6, 2);
  bottomRowCanvas.print(text);
}


void printHumidity(float percent) {
  char text[12];
  sprintf(text, "H:%d%%", (int)round(percent));

  bottomRowCanvas.setFont();
  bottomRowCanvas.setTextSize(1);
  bottomRowCanvas.setTextColor(TEXT_COLOR);
  bottomRowCanvas.setCursor(5, 2);
  bottomRowCanvas.print(text);
}

void showBottomRow(){
  screen.drawRGBBitmap(BOTTOM_ROW_LEFT, BOTTOM_ROW_TOP,
                      bottomRowCanvas.getBuffer(), 110, 12);
}

void setup() {
  pinMode(SCREEN_BACKLIGHT_PIN, OUTPUT);
  digitalWrite(SCREEN_BACKLIGHT_PIN, HIGH);

  screen.initR(INITR_GREENTAB);
  screen.setRotation(0);

  uint8_t colorOrderCommand = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY;
  screen.sendCommand(ST77XX_MADCTL, &colorOrderCommand, 1); // come back to to understand

  screen.fillScreen(BACKGROUND_COLOR);

  screen.fillRoundRect(8, 20,  112,  20,  10,  CARD_COLOR); // draws white rectangle to sit behind the text
  printCentered("My Thermometer", 26, 1, TITLE_COLOR); // figure out pixel math later

  screen.fillRoundRect(8, 46, 112, 94, 8, CARD_COLOR);
}

void loop() {
  sensor.getRawData();

  float celsius = sensor.getTemperature();
  float humidity = sensor.getHumidity();

  if (sensor.getLastError() == 0) {
    float fahrenheit = sensor.convertCtoF(celsius);
    printTemperature((int)round(fahrenheit), "F"); 
    printSmallTemperature((int)round(celsius), "C");
    printHumidity((int)round(humidity));
    showBottomRow();
  }

  delay(2000);

}