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

enum Screen { TITLE, PET, MENU, STATUS_SCREEN };
Screen currentScreen = TITLE;

int petDirection = 1;                         // direction the pet is moving; 1 is right and -1 is left
const int PET_STEP_SIZE = 5;                  // how many pixels the pet moves each time
const unsigned long PET_STEP_INTERVAL = 550;  // how long the pet waits between steps; number is milliseconds
unsigned long lastPetStepTime = 0;            // remembers the moment the pet took a step last; it ensures that enough time has passed before the pet moves again

#define BAR_EMPTY_COLOR     0xEF7D
#define BAR_OUTLINE_COLOR   0xC618
#define HUNGER_BAR_COLOR    0xFDB88
#define FUN_BAR_COLOR       0x9FD3
#define HAPPY_BAR_COLOR     0xAE3F


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

//moves the pet on the screen
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

const char* menuItems[3] = { "Status", "Food", "Play" };  // holds the menu choices
int selectedMenuItem = 0;                                 // keeps track of which menu item is being highlighted
#define MENU_SCREEN_COLOR 0xD65D

//drawing one of the options on our menu
void drawMenuItem(int itemIndex) {
  int rowTop = 52 + itemIndex * 32;
  bool isSelected = (itemIndex == selectedMenuItem);
  display.fillRect(6, rowTop - 3, display.width() - 12, 28, MENU_SCREEN_COLOR);
  if (isSelected) display.fillRoundRect(10, rowTop, display.width() - 20, 22, 10, ST77XX_WHITE);
  const char* label = menuItems[itemIndex];
  display.setTextSize(1);
  display.setTextColor(TEXT_COLOR);
  display.setCursor((display.width() - (int)strlen(label) * 6) /2, rowTop + 7);
  display.print(label);
}

//draws the entire menu
void drawMenuScreen(){
  display.fillScreen(MENU_SCREEN_COLOR);
  printCentered("MENU", 28, 2, HEADER_COLOR);
  for (int i = 0; i < 3; i++)
    drawMenuItem(i);
}

int hungerLevel = 70, funLevel = 70, happinessLevel = 70;   // the 3 statuses available in the Status page
unsigned long lastStatDecreaseTime = 0;                     // remembers the last time the stats dropped so we can keep track of how much time has passed by
const unsigned long STAT_DECREASE_INTERVAL = 15000;         //represents how often the stats drop; in this case it's every 15 seconds

//function to make sure each status stays between 0 and 100
int keepBetween0And100(int value) {
  if (value < 0) return 0;
  if (value > 100) return 100;
  return value;
}

void drawStaBar(int y, const char* label, int value, uint16_t barColor) {
  display.setTextSize(1);
  display.setTextColor(TEXT_COLOR);
  display.setCursor(12,y);
  display.print(label);
  int barX = 12, barY = y + 12, barWidth = display.width() - 24, barHeight = 14, cornerRadius = 7;
  display.fillRoundRect(barX, barY, barWidth, barHeight, cornerRadius, BAR_EMPTY_COLOR);
  int filledWidth = (value * barWidth) / 100;
  if (value > 0 && filledWidth < 2 * cornerRadius) filledWidth = 2 * cornerRadius;
  if (filledWidth > barWidth) filledWidth = barWidth;
  if (value > 0) display.fillRoundRect(barX, barY, filledWidth, barHeight, cornerRadius, barColor);
  display.drawRoundRect(barX, barY, barWidth, barHeight, cornerRadius, BAR_OUTLINE_COLOR);
}

void drawStatusScreen() {
  display.fillScreen(BACKGROUND_COLOR);
  printCentered("STATUS", 26, 2, HEADER_COLOR);
  drawStaBar(54, "HUNGER", hungerLevel, HUNGER_BAR_COLOR);
  drawStaBar(90, "FUN", funLevel, FUN_BAR_COLOR);
  drawStaBar(126, "HAPPY", happinessLevel, HAPPY_BAR_COLOR);
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

  if (currentScreen == PET && millis() - lastStatDecreaseTime > STAT_DECREASE_INTERVAL) {
    lastStatDecreaseTime = millis();
    hungerLevel = keepBetween0And100(hungerLevel - 1);
    funLevel = keepBetween0And100(funLevel - 1);
    happinessLevel = keepBetween0And100(happinessLevel - 1);
  }

  switch (currentScreen) {
    case TITLE:
      if (pressedButton != NO_BUTTON) {
        playBeep();
        enterPetScreen();
        currentScreen = PET;
      }
      break;
    
    case PET:
      updatePetMovement();
      if (pressedButton == SCROLL_BUTTON) {
        playBeep();
        drawMenuScreen();
        currentScreen = MENU;
      }
      break;
    
    case MENU:
      if (pressedButton == SCROLL_BUTTON) {
        playBeep();
        int previousMenuItem = selectedMenuItem;
        if (selectedMenuItem == 2) 
          selectedMenuItem = 0;
        else selectedMenuItem = selectedMenuItem + 1;
        drawMenuItem(previousMenuItem);
        drawMenuItem(selectedMenuItem);
      } else if (pressedButton == SELECT_BUTTON) {
        if (selectedMenuItem == 0) {
          playBeep();
          drawStatusScreen();
          currentScreen = STATUS_SCREEN;
        }
      } else if (pressedButton == BACK_BUTTON) {
          playBeep();
          enterPetScreen();
          currentScreen = PET;
      }
      break;

    case STATUS_SCREEN:
    if (pressedButton == BACK_BUTTON) {
      playBeep();
      drawMenuScreen();
      currentScreen = MENU;
    }
  }

}
