#include <MIDI.h>
#include "MatrixButton.h"

MatrixButton keypad[] = {
  {2, 9},    /* 1 */  {2, 8}, /* 2 */  {2, 6}, /* 3 */
  {4, 9},    /* 4 */  {4, 8}, /* 5 */  {4, 6}, /* 6 */
  {5, 9},    /* 7 */  {5, 8}, /* 8 */  {5, 6}, /* 9 */
  {10,  11}, /* * */  {7, 8}, /* 0 */  {7, 3}  /* # */
};

// musical tone scale, pentatonic + maj 7
const int8_t music_scale[12] = {
  0, 2, 4,
  7, 9, 11,
  12, 14, 16,
  19, 21, 23
};

byte midiChannel = 1;

MIDI_CREATE_DEFAULT_INSTANCE();

void setup() {
  // initialize keypad button states
  for (byte i = 0; i < 12; i++) 
      keypad[i].begin();

  MIDI.begin();
}

void loop() {
  // scan keypad for key presses
  for (byte i = 0; i < 12; i++)
  {
    bool has_changed;
    bool state = keypad[i].read(has_changed);

    if (has_changed)
    {
      if (state == MatrixButton::PRESSED) 
        MIDI.sendNoteOn(60 + music_scale[i], 127, midiChannel);
      else
        MIDI.sendNoteOff(60 + music_scale[i], 0, midiChannel);
    }
  }
}
