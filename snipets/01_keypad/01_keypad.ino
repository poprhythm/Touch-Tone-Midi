#include "MatrixButton.h"

// input-output pin combinations for all the keypad buttons
MatrixButton keypad[] = {
  {2, 9},    /* 1 */  {2, 8}, /* 2 */  {2, 6}, /* 3 */
  {4, 9},    /* 4 */  {4, 8}, /* 5 */  {4, 6}, /* 6 */
  {5, 9},    /* 7 */  {5, 8}, /* 8 */  {5, 6}, /* 9 */
  {10,  11}, /* * */  {7, 8}, /* 0 */  {7, 3}  /* # */
};

// LED pin
int led = 13;

void setup() {
  // initialize keypad button states
  for (byte i = 0; i < 12; i++) 
      keypad[i].begin();

  // init the LED
  pinMode(led, OUTPUT);
}

void loop() {
  bool anyPressed = false;
  
  // scan keypad for key presses
  for (byte i = 0; i < 12; i++)
  {
    bool has_changed;
    bool state = keypad[i].read(has_changed);

    if (state == MatrixButton::PRESSED)
      anyPressed = true;
  }

  // light up the LED if one (or more) buttons are pressed
  if (anyPressed)
    digitalWrite(led, HIGH);
  else
    digitalWrite(led, LOW);
}
