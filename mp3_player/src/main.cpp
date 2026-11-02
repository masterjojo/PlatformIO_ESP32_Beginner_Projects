#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
//#include <ESP32-audioI2S.h>

// screen pins
#define SCREEN_SCL_PIN  26  // serial clock
#define SCREEN_SDA_PIN  25  // serial data

// amp pins
#define AMP_BCL_PIN     32 // bit clock pin
#define AMP_LRC_PIN     35  //left and right channel frame clock pin
#define AMP_DIN_PIN     34

// SD card reader pins
#define SD_CHIP_SELECT_PIN        16 // Active LOW to select this device
#define SD_MOSI_PIN               4 // Master Out, Slave In
#define SD_CLOCK_PIN              0
#define SD_MISO_PIN               2 // Master In, Slave Out

// button pins
#define REWIND_BUTTON_PIN       16  
#define PLAY_PAUSE_BUTTON_PIN   17
#define FAST_FORWARD_BUTTON_PIN 5

// screen dimensions
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64

Adafruit_SSD1306 screen = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  // pinMode(SD_CHIP_SELECT_PIN, OUTPUT);
  pinMode(REWIND_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PLAY_PAUSE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(FAST_FORWARD_BUTTON_PIN, INPUT_PULLUP);
  screen.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  screen.clearDisplay();

  // Display Text
	screen.setTextSize(1);
	screen.setTextColor(WHITE);
	screen.setCursor(0,28);
	screen.println("Circuit Schools");
	screen.display();
	delay(2000);
	screen.clearDisplay();
}

void loop() {
  // put your main code here, to run repeatedly:
}
