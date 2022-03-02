#include "SSD1306Wire.h"

#define BOMB_OUT 25
#define LED_COUNT 26
#define UP_BTN 32
#define DOWN_BTN 13
#define ARM_BTN 33

// Selecciona uno según tu display.
//SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_128_32);
SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_64_48);

void serialTask();
void btnsTask();
void bombTask();




void setup() {

  serialTask();
  btnsTask();
  bombTask();



}


boolean evBtns = false;
uint8_t evBtnsData = 0;

void loop() {

  serialTask();
  btnsTask();
  bombTask();

}

void btnsTask() {
  enum class BtnsStates {INIT, WAITING_PRESS, WAITING_STABLE, WAITING_RELEASE};
  static BtnsStates btnsStates = BtnsStates::INIT;
  static uint32_t referenceTime;
  const uint32_t STABLETIMEOUT = 100;
  static uint8_t lastBtn = 0;

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
          lastBtn =DOWN_BTN;
          btnsStates = BtnsStates::WAITING_STABLE;
        }
        else if (digitalRead(UP_BTN) == LOW) {
          referenceTime = millis();
          lastBtn = UP_BTN;
          btnsStates = BtnsStates::WAITING_STABLE;
        }
        else if (digitalRead(ARM_BTN) == LOW) {
          referenceTime = millis();
          lastBtn = ARM_BTN;
          btnsStates = BtnsStates::WAITING_STABLE;
        }
        break;
      }
    case BtnsStates::WAITING_STABLE: {

        if (digitalRead(lastBtn) == HIGH) {
          btnsStates = BtnsStates::WAITING_PRESS;
        }
        else if ( (millis() - referenceTime) >= STABLETIMEOUT) {
          btnsStates = BtnsStates::WAITING_RELEASE;
        }


        break;
      }
    case BtnsStates::WAITING_RELEASE: {
        if (digitalRead(lastBtn) == HIGH) {
          evBtns = true;
          evBtnsData = lastBtn;
            Serial.println(lastBtn);
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
        display.drawString(10, 20, String(counter));
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
            display.drawString(10, 20, String(counter));
            display.display();
          }
          else if (evBtnsData == UP_BTN) {
            if (counter < 60) {
              counter++;
            }
            display.clear();
            display.drawString(10, 20, String(counter));
            display.display();
          }
           else if (evBtnsData == ARM_BTN) {
           bombStates = BombStates::COUNTING;
           Serial.println("BombStates::COUNTING");
           Serial.println("La Bomba ha sido armado con Exito");
           display.clear();
           display.drawString(10, 20, String("Armado Exitoso"));
           display.display();
          }
        }
        break;
      }
    case BombStates::COUNTING: {
        const uint32_t TimeLED_COUNT = 500; 
        static uint32_t previousTMinus = 0;
        static uint8_t led_countState = LOW;
        uint32_t currentTMinus = millis();

        if (currentTMinus - previousTMinus >= TimeLED_COUNT) {
          previousTMinus = currentTMinus;
          if (led_countState == LOW) {
            led_countState = HIGH;
          } else {
            led_countState = LOW;
            counter--; 
            display.clear();
            display.drawString(10, 20, String(counter));
            display.display();
          }
          digitalWrite(LED_COUNT, led_countState);
        }
        
        if (counter == 0) {
          bombStates = BombStates::BOOM;
        }
            
    
        
      
        break;
      }
  
    case BombStates::BOOM: {
        digitalWrite(LED_COUNT, LOW); 
        digitalWrite(BOMB_OUT, HIGH); 
        Serial.println("BOOM"); 
        display.clear();
        display.drawString(10, 20, "BOOM!"); 
        display.display();
        delay(2500); 
        bombStates = BombStates::INIT;
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
          else if (dataIn == 'u') {
            evBtns = true;
            evBtnsData = UP_BTN;
            Serial.println("UP_BTN");
          }
          else if (dataIn == 'a') {
            evBtns = true;
            evBtnsData = ARM_BTN;
            Serial.println("ARM_BTN");
          }
          else if (dataIn == 'a') {
          evBtns = true;
          evBtnsData = ARM_BTN;
          Serial.println("ARM_BTN");
          }
        }

        break;
      }
    default:
      break;

  }
}
