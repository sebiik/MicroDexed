#ifndef DIN_H_INCLUDED
#define DIN_H_INCLUDED

#define NUM_DIN_BUTTONS 3

const uint8_t dinPinList[9] = {2, 3, 4, 5, 24, 25, 29, 28, 30};

Bounce myButtons[NUM_DIN_BUTTONS] = {
  Bounce(dinPinList[3], BUT_DEBOUNCE_MS),
  Bounce(dinPinList[4], BUT_DEBOUNCE_MS),
  Bounce(dinPinList[5], BUT_DEBOUNCE_MS)
};

void init_myButtons(void) {
  for (uint8_t i=3; i<NUM_DIN_BUTTONS+3; i++)
    pinMode(dinPinList[i], INPUT_PULLUP);
}

void handle_myButtons(void) { //seb
  for (uint8_t i = 0; i<NUM_DIN_BUTTONS; i++) {
    myButtons[i].update();

    if (myButtons[i].fallingEdge()) {
      long_button_pressed = 0;
      #ifdef DEBUG
      Serial.print(F("falling edge"));
      Serial.println(i, DEC);
      #endif
    }

    if (myButtons[i].risingEdge()) {
      #ifdef DEBUG
      Serial.print(F("rising edge"));
      Serial.println(i, DEC);
      #endif
      uint32_t button_released = long_button_pressed;

      if (button_released > LONG_BUTTON_PRESS) {
        // long press of button detected
        #ifdef DEBUG
        Serial.print(F("Long press detected for myButton "));
        Serial.println(i, DEC);
        #endif
        switch (i) {
          case 0: // long press for myButton[0]
          break;
          case 1: // long press for myButton[1]
          break;
          case 2: // long press for myButton[2]
          break;
        }
      }
      else { //seb short press detected
        #ifdef DEBUG
        Serial.print(F("Pressed myButton "));
        Serial.println(i, DEC);
        #endif
        switch (i) {
          case 0: // short press for myButton[0]
          if (activeLayer > 0)
          activeLayer--;
          break;
          case 1: // short press for myButton[1]
          if (activeLayer < 3)
          activeLayer++;
          break;
          case 2: // short press for myButton[2]
          break;
        }
        activeLayer %= 4;
      }
    }
  }
}

#endif //DIN_H_INCLUDED
