#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include <Wire.h>
// #include <Adafruit_SSD1306.h>
#include <Audio.h>
#include <buffers.hpp>

// screen pins
// #define SCREEN_SCL_PIN    22 // serial clock
// #define SCREEN_SDA_PIN    21 // serial data

// amp pins
#define AMP_BCL_PIN     33 // bit clock pin
#define AMP_LRC_PIN     32 //left and right channel frame clock pin
#define AMP_DIN_PIN     23

// SD card reader pins
#define SD_CHIP_SELECT_PIN        25  // Active LOW to select this device
#define SD_MOSI_PIN               26   // Master Out, Slave In
#define SD_CLOCK_PIN              27
#define SD_MISO_PIN               14   // Master In, Slave Out

// button pins
// #define REWIND_BUTTON_PIN       5  
// #define PLAY_PAUSE_BUTTON_PIN   18
// #define FAST_FORWARD_BUTTON_PIN 19
// #define VOLUME_UP_BUTTON_PIN    34
// #define VOLUME_DOWN_BUTTON_PIN  35  

// screen dimensions
//#define SCREEN_WIDTH    128
//#define SCREEN_HEIGHT   64


Audio speaker;

// Adafruit_SSD1306 screen = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int volume = 5;

//function to make sure each status stays between 0 and 21
//int keepBetween0And21(int value) {
//  if (value < 0) return 0;
//  if (value > 21) return 21;
//  return value;
// }

// enum type assigns a name to any possible action the user can take on on the mp3 player
// enum ButtonPress { REWIND_BUTTON, PLAY_PAUSE_BUTTON, FORWARD_BUTTON, VOLUME_UP_BUTTON, VOLUME_DOWN_BUTTON };
// const ButtonPress buttonNames[5] = { REWIND_BUTTON, PLAY_PAUSE_BUTTON, FORWARD_BUTTON, VOLUME_UP_BUTTON, VOLUME_DOWN_BUTTON };
// const int buttonPins[5] = { SCROLL_BUTTON_PIN, SELECT_BUTTON_PIN, BACK_BUTTON_PIN };
// bool previousButtonStates[5] = { HIGH, HIGH, HIGH, HIGH, HIGH };
// 
// ButtonPress getPressedButton() {
//   ButtonPress pressedButton = NO_BUTTON;
//   for (int i = 0; i<3; i++) {
//     bool isPressedNow = digitalRead(buttonPins[i]);
//     if (previousButtonStates[i] == HIGH && isPressedNow == LOW) pressedButton = buttonNames[i];
//       previousButtonStates[i] = isPressedNow;
//   }
//   if (pressedButton != NO_BUTTON) delay(20);
//   return pressedButton;
// }
void audio_info(const char *info) { Serial.println(info); }
void audio_error(const char *info) { Serial.println(info); }

void setup() {
  Serial.begin(115200);

  pinMode(SD_CHIP_SELECT_PIN, OUTPUT);
  // pinMode(REWIND_BUTTON_PIN, INPUT_PULLUP);
  // pinMode(PLAY_PAUSE_BUTTON_PIN, INPUT_PULLUP);
  // pinMode(FAST_FORWARD_BUTTON_PIN, INPUT_PULLUP);
  // pinMode(VOLUME_DOWN_BUTTON_PIN, INPUT_PULLUP);
  // pinMode(VOLUME_UP_BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(SD_CHIP_SELECT_PIN, HIGH);
  // digitalWrite(REWIND_BUTTON_PIN, HIGH);
  // digitalWrite(PLAY_PAUSE_BUTTON_PIN, HIGH);
  // digitalWrite(FAST_FORWARD_BUTTON_PIN, HIGH);
  // digitalWrite(VOLUME_DOWN_BUTTON_PIN, HIGH);
  // digitalWrite(VOLUME_UP_BUTTON_PIN, HIGH);
  SPI.begin(SD_CLOCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CHIP_SELECT_PIN);
  SD.begin(SD_CHIP_SELECT_PIN);

  if (!SD.begin(SD_CHIP_SELECT_PIN)) {
    Serial.println("SD mount failed");
    return;
  }
  Serial.println("SD mounted");

  speaker.setPinout(AMP_BCL_PIN, AMP_LRC_PIN, AMP_DIN_PIN);
  speaker.setVolume(volume); // 0...21
  speaker.forceMono(true);
  if (!speaker.connecttoFS(SD, "/test.mp3")) {
    Serial.println("Failed to open file");
    return;
  }
  Serial.printf("isRunning: %d\n", speaker.isRunning());
  Serial.println("Playing...");
  Serial.printf("Heap after connect: %d\n", ESP.getFreeHeap());
}

void loop() {
  // ButtonPress pressedButton = getPressedButton();
  speaker.loop();
  
}