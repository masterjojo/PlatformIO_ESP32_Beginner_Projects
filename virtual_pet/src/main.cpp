#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <driver/gpio.h>
#include "Pet.h"
#include "Pizza.h"
#include "Donut.h"
#include "Ball.h"

// screen, buttons, and buzzer pins
#define SCREEN_DATA_PIN         A1
#define SCREEN_CLOCK_PIN        A2
#define SCREEN_CHIP_SELECT_PIN  A3
#define SCREEN_DATA_COMMAND_PIN A4
#define SCREEN_RESET_PIN        A5
#define SCREEN_BACKLIGHT_PIN    A6
#define SCROLL_BUTTON_PIN       D3
#define SELECT_BUTTON_PIN       D4
#define BACK_BUTTON_PIN         D5
#define BUZZER_PIN              D8

Adafruit_ST7735 display = Adafruit_ST7735(SCREEN_CHIP_SELECT_PIN, SCREEN_DATA_COMMAND_PIN, 
  SCREEN_DATA_PIN, SCREEN_CLOCK_PIN, SCREEN_RESET_PIN);

#define BACKGROUND_COLOR  ST77XX_WHITE
#define TEXT_COLOR        ST7735_BLACK          
#define HEADER_COLOR      0x52AA

// enum type assigns a name to any possible action the user can take on on the title screen
enum ButtonPress { NO_BUTTON, SCROLL_BUTTON, SELECT_BUTTON, BACK_BUTTON };

#define PET_SCREEN_COLOR    0xE73F
const int PET_WIDTH = 40, PET_HEIGHT = 40;
int petPositionX, petPositionY;

enum Screen { TITLE, PET };
Screen currentScreen = TITLE;

int petDirection = 1;                         // direction the pet is moving; 1 is right and -1 is left
const int PET_STEP_SIZE = 5;                  // how many pixels the pet moves each time
const unsigned long PET_STEP_INTERVAL = 550;  // how long the pet waits between steps; number is milliseconds
unsigned long lastPetStepTime = 0;            // remembers the moment the pet took a step last; it ensures that enough time has passed before the pet moves again

//function centers the text in the middle of the screen
void printCentered(const char* text, int y, int textSize, uint16_t color) {
  display.setTextSize(textSize);
  display.setTextColor(color);
  display.setCursor((display.width() - strlen(text) * 6 * textSize) / 2, y);
  display.print(text);
}

// basic sound for general button presses
// plays a frequency on the buzzer for 40 milliseconds
void playBeep() { tone(BUZZER_PIN, 1200, 40); }


// music for when the device powers on
// the loop plays through all fives notes and waits for it to finish
void playStartupJingle() {
  int noteFrequencies[] = { 659, 784, 880, 988, 1047 }; // list of pitches
  int noteDurations[]   = { 150, 150, 150, 150, 300 };  // how long each pitch plays for
  for (int i = 0; i < 5; i++) {
    tone(BUZZER_PIN, noteFrequencies[i], noteDurations[i]); delay(noteDurations[i] + 30);
  }
  noTone(BUZZER_PIN); // this function makes sure that the buzzer goes quiet when it's done
}

void drawTitleScreen() {
  display.fillScreen(BACKGROUND_COLOR); //paint whole screen white to start fresh
  printCentered("MY",               44, 2, HEADER_COLOR);
  printCentered("VIRTUAL",          68, 2, HEADER_COLOR);
  printCentered("PET",              92, 2, HEADER_COLOR);
  printCentered("Press any button", 132, 1, TEXT_COLOR);
}

const int buttonPins[3] = { SCROLL_BUTTON_PIN, SELECT_BUTTON_PIN, BACK_BUTTON_PIN };
const ButtonPress buttonNames[3] = { SCROLL_BUTTON, SELECT_BUTTON, BACK_BUTTON };
bool previousButtonStates[3] = { HIGH, HIGH, HIGH };

ButtonPress getPressedButton() {
  ButtonPress pressedButton = NO_BUTTON;
  for (int i = 0; i<3; i++) {
    bool isPressedNow = digitalRead(buttonPins[i]);
    if (previousButtonStates[i] == HIGH && isPressedNow == LOW) pressedButton = buttonNames[i];
      previousButtonStates[i] = isPressedNow;
  }
  if (pressedButton != NO_BUTTON) delay(20);
  return pressedButton;
}

// draws out each of the images we want to use in our game
// the function only displays pixels that are not white
// if a pixel is white then the function ignores it and it becomes transparent
void drawSpriteTransparent(int x, int y, const uint16_t* spriteImage, int width, int height) {
  display.startWrite();

  for (int row = 0; row < height; row++)
    for (int column = 0; column < width; column++) {
      uint16_t pixelColor = pgm_read_word(&spriteImage[row * width + column]);
      if (pixelColor != 0xFFFF)display.writePixel(x + column, y + row, pixelColor);
    }
  display.endWrite();
}

void drawPet(int x, int y) { drawSpriteTransparent(x, y, epd_bitmap_virtual_pet_hippo, PET_WIDTH, PET_HEIGHT); }

void enterPetScreen() {
  display.fillScreen(PET_SCREEN_COLOR);

  petPositionY = display.height() / 2 - PET_HEIGHT / 2;
  petPositionX = display.width() / 2 - PET_WIDTH / 2;

  drawPet(petPositionX, petPositionY);
}

void updatePetMovement() {
  if (millis() - lastPetStepTime < PET_STEP_INTERVAL) return;   // the timer; millis is how long the board has been running
  lastPetStepTime = millis();
  int previousX = petPositionX;
  petPositionX += petDirection * PET_STEP_SIZE;   // saves the old position and move the pet position X by one step
  if (petPositionX < 2) { petPositionX = 2; petDirection = 1; }
  int maxRightPosition = display.width() - PET_WIDTH - 2;
  if (petPositionX > maxRightPosition) { petPositionX = maxRightPosition; petDirection = -1; }
  display.fillRect(previousX, petPositionY, PET_WIDTH, PET_HEIGHT, PET_SCREEN_COLOR); // to make the pet look like its walking, we paint a lavendar rectangle over where the pet was to erase it
  drawPet(petPositionX, petPositionY); //then we draw it in its new position
}

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SCREEN_BACKLIGHT_PIN, OUTPUT);
  pinMode(SCROLL_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SELECT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(BACK_BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(SCREEN_BACKLIGHT_PIN, HIGH);

  display.initR(INITR_GREENTAB);
  display.setRotation(0);

  uint8_t colorOrderCommand = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY;
  display.sendCommand(ST77XX_MADCTL, &colorOrderCommand, 1);

  drawTitleScreen();
  playStartupJingle();
}
 
void loop() {
  ButtonPress pressedButton = getPressedButton();
  if (currentScreen == TITLE && pressedButton != NO_BUTTON) {
    playBeep();
    enterPetScreen();
    currentScreen = PET;
  }

  if (currentScreen == PET) {
    updatePetMovement();
  }
}
