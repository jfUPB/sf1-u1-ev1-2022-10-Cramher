#include "SSD1306Wire.h"

#define BOMB_OUT 25
#define LED_COUNT 26
#define UP_BTN 13
#define DOWN_BTN 32
#define ARM_BTN 33

// Selecciona uno según tu display.
//SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_128_32);
SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_64_48); 

void serialTask();
void btnsTask();
void bombTask();


bool evBtns = false;
uint8_t evBtnsData = 0;

void setup() {

  serialTask();
  btnsTask();
  bombTask();  
}

void loop() {

}

void btnsTask() {
  enum class BtnsStates {INIT, WAITING_PRESS, WAITING_STABLE, WAITING_RELEASE};
  static BtnsStates btnsStates = BtnsStates::INIT;
  static uint32_t referenceTime;
  const uint32_t STABLETIMEOUT = 100;

  switch (btnsStates) {
    case BtnsStates::INIT: {
        pinMode(UP_BTN, INPUT_PULLUP); //Decimos que son pulsadores
        pinMode(DOWN_BTN, INPUT_PULLUP);
        pinMode(ARM_BTN, INPUT_PULLUP);
        btnsStates = BtnsStates::WAITING_PRESS;
        break;
      }
    case BtnsStates::WAITING_PRESS: {
        if (digitalRead(DOWN_BTN) == LOW) {
          referenceTime = millis();
          btnsStates = BtnsStates::WAITING_STABLE;
        }
        break;
      }
    case BtnsStates::WAITING_STABLE: {
        if (digitalRead(DOWN_BTN) == HIGH) {
          btnsStates = BtnsStates::WAITING_PRESS;
        }
        else if ( (millis() - referenceTime) >= STABLETIMEOUT) {
          btnsStates = BtnsStates::WAITING_RELEASE;
        }
        break;
      }
    case BtnsStates::WAITING_RELEASE: {
        if (digitalRead(DOWN_BTN) == HIGH) {
          evBtns = true;
          evBtnsData = DOWN_BTN;
          Serial.println("DOWN_BTN");
          btnsStates = BtnsStates::WAITING_PRESS;
        }
        break;
      }
    default:
      break;

  }
}

void bombTask() {

  enum class BombStates {INIT, WAITING_CONFIG, COUNTING, BOOM};
  static BombStates bombStates = BombStates::INIT;
  static uint8_t counter;

  switch (bombStates) {
    case BombStates::INIT: {
        pinMode(LED_COUNT, OUTPUT);
        pinMode(BOMB_OUT, OUTPUT);
        display.init(); //Para usar la pantalla
        display.setContrast(255);
        display.clear();
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        counter = 20;

        display.clear();
        display.drawString(0, 5, String(counter));
        display.display();
        bombStates = BombStates::WAITING_CONFIG;
        break;
      }
    case BombStates::WAITING_CONFIG: {

        if (evBtns == true) {
          evBtns = false;

          if (evBtnsData == DOWN_BTN) {
            if (counter > 10) {
              counter--;
            }
            display.clear();
            display.drawString(0, 5, String(counter));
            display.display();
          }

        }
        break;
      }
    case BombStates::COUNTING: {
        break;
      }
    case BombStates::BOOM: {
        break;
      }
    default:
      break;

  }

}

void serialTask() {
  enum class SerialStates {INIT, READING_COMMANDS};
  static SerialStates serialStates = SerialStates::INIT;

  switch (serialStates) {
    case SerialStates::INIT: {
        Serial.begin(115200);
        serialStates = SerialStates::READING_COMMANDS;
        break;
      }
    case SerialStates::READING_COMMANDS: {

        if (Serial.available() > 0) {
          int dataIn = Serial.read();
          if (dataIn == 'd') {
            evBtns = true;
            evBtnsData = DOWN_BTN;
            Serial.println("DOWN_BTN");
          }
        }

        break;
      }
    default:
      break;

  }
}
