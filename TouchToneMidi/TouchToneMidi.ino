/*
Touch Tone Midi - 

MIT licensed.

Libraries:
  MIDI - https://github.com/FortySevenEffects/arduino_midi_library
  Blinkenlight - https://github.com/tfeldmann/Arduino-Blinkenlight
*/
#include <MIDI.h>
#include <BaseBlinker.h>
#include <Blinkenlight.h>
#include "Button.h"
#include "MatrixButton.h"

Blinkenlight leds[] = {A0, A1, A2, A3};

#define REDIAL_PIN A4
#define SWITCH_PIN A5
#define HANGUP_PIN 12
#define MIDDLE_C 60
#define MIDI_CHANNEL_A 1
#define MIDI_CHANNEL_B 2

Button metaBtn(REDIAL_PIN);
Button switchBtn(SWITCH_PIN);

MatrixButton keypad[] = {
  {2, 9},    /* 1 */  {2, 8}, /* 2 */  {2, 6}, /* 3 */
  {4, 9},    /* 4 */  {4, 8}, /* 5 */  {4, 6}, /* 6 */
  {5, 9},    /* 7 */  {5, 8}, /* 8 */  {5, 6}, /* 9 */
  {10,  11}, /* * */  {7, 8}, /* 0 */  {7, 3}  /* # */
};

struct Tones { int8_t t1; int8_t t2; int8_t t3; int8_t t4; };

struct KeyScale { Tones tones[12]; };

#define nn -126 // placeholder for "no note"
#define SCALE_COUNT 4

const int8_t scales[SCALE_COUNT][12][4] PROGMEM = {
{ // SCALE 1 - pentatonic duotones
  {-5, -12, nn, nn}, { 0, -8, nn, nn},  { 2, -7, nn, nn}, 
  {-5,  4,  nn, nn}, { 7,  2, nn, nn},  { 9,  4, nn, nn},
  { 7,  0, nn, nn},  {12,  4, nn, nn},  {12,  5, nn, nn}, 
  { 11, 16, nn, nn}, { 9, 12, 16, 17},  {19, 12, nn, nn}
},
{ // SCALE 2 - maj 7 duotones
  {-1,  4, nn, nn}, { 0,  7, nn, nn},  { 2, nn, nn, nn}, 
  { 0,  5, nn, nn}, { 2,  9, nn, nn},  { 4, nn, nn, nn},
  { 4,  9, nn, nn}, { 5, 12, nn, nn},  { 7, nn, nn, nn}, 
  { 5, 11, nn, nn}, { 7, 14, nn, nn},  { 9, 16, nn, nn} 
},
{ // SCALE 3 - aolian duotones
  {-2,  3, nn, nn}, { 0,  7, nn, nn},  { 2, nn, nn, nn}, 
  { 0,  5, nn, nn}, { 2,  9, nn, nn},  { 3, nn, nn, nn},
  { 3, 10, nn, nn}, { 5, 12, nn, nn},  { 7, nn, nn, nn}, 
  { 5, 10, nn, nn}, { 7, 14, nn, nn},  {10, 15, nn, nn}
},
{ // SCALE 4 - phone tones
  {-7,  2, nn, nn}, {-7,  4, nn, nn},  {-7,  6, nn, nn}, 
  {-5,  2, nn, nn}, {-5,  4, nn, nn},  {-5,  6, nn, nn},
  {-4,  2, nn, nn}, {-4,  4, nn, nn},  {-4,  6, nn, nn}, 
  {-2,  2, nn, nn}, {-2,  4, nn, nn},  {-2,  6, nn, nn}
}
};

KeyScale keyScale;

void loadKeyScale(byte index)
{
  //free(&keyScale);
  for (byte i = 0; i < 12; i++ )
  {
    keyScale.tones[i].t1 = (int8_t) pgm_read_byte_near(&scales[index][i][0]);
    keyScale.tones[i].t2 = (int8_t) pgm_read_byte_near(&scales[index][i][1]);
    keyScale.tones[i].t3 = (int8_t) pgm_read_byte_near(&scales[index][i][2]);
    keyScale.tones[i].t4 = (int8_t) pgm_read_byte_near(&scales[index][i][3]);
  }
}

byte scaleIndex = 0;

byte rootNote = MIDDLE_C;

int8_t fifthsIndex = 0;

int8_t octave = 0;

byte midiChannel = MIDI_CHANNEL_A;

bool metaActionTaken;

// meta functions - when the meta/super key is active
void (*metaFuncs[])(byte) = {
  decreaseOctave, resetOctave, increaseOctave,
  prevFifth, resetFifth, nextFifth,
  decreaseScale, resetScale, increaseScale,
  NULL, NULL, sendMidiAllNotesOff
};

void (*sendKeyOnFuncs[])(byte) = {
  sendKeyOn, sendKeyOn, sendKeyOn,
  sendKeyOn, sendKeyOn, sendKeyOn,
  sendKeyOn, sendKeyOn, sendKeyOn,
  sendKeyOn, sendKeyOn, sendKeyOn
};

void (*sendKeyOffFuncs[])(byte) = {
  sendKeyOff, sendKeyOff, sendKeyOff,
  sendKeyOff, sendKeyOff, sendKeyOff,
  sendKeyOff, sendKeyOff, sendKeyOff,
  sendKeyOff, sendKeyOff, sendKeyOff
};

MIDI_CREATE_DEFAULT_INSTANCE();

void setup() 
{
  for (byte i=0; i<4; i++) leds[i].off();
  
  loadKeyScale(scaleIndex);
  setScaleLedPatttern();
  setFifthsLedPattern();
  
  metaBtn.begin();
  switchBtn.begin();
  switchAction(true);
  
  for (byte i = 0; i < 12; i++) 
    keypad[i].begin();

  pinMode(HANGUP_PIN, INPUT_PULLUP);

  MIDI.begin();
}

void setScaleLedPatttern()
{
  leds[3].pattern(scaleIndex+1, SPEED_FAST);
}

void setFifthsLedPattern()
{
  if (fifthsIndex > 0)
  {
    leds[2].pattern(fifthsIndex, SPEED_RAPID);
  }
  else if (fifthsIndex < 0)
  {
    leds[2].pattern(-fifthsIndex, SPEED_FAST);
  }
}

bool switchAction() { switchAction(false); }
bool switchAction(bool force)
{
  bool switchChanged = false;
  bool switchState = switchBtn.read(switchChanged);
  if (switchChanged || force)
  {
    if (switchState == Button::PRESSED)
    {
      midiChannel = MIDI_CHANNEL_B;
      leds[0].on();
    }
    else
    {
      midiChannel = MIDI_CHANNEL_A;
      leds[0].off();
    }
  }
}

void loop() 
{
  switchAction();
  
  bool metaChanged = false;
  bool metaState = metaBtn.read(metaChanged);
  if (metaChanged)
  {
    if (metaState == Button::PRESSED)
    {
      metaActionTaken = false;
      leds[1].off();
      leds[2].off();
      leds[3].blink(SPEED_RAPID);
    } 
    else
    {
      setScaleLedPatttern();
      setFifthsLedPattern();
    }
  }
  
  if (metaState == Button::RELEASED)
  {
    if (keypadAction(sendKeyOnFuncs, sendKeyOffFuncs))
      leds[1].on();
    else
      leds[1].off(); 
  }
  else if (metaState == Button::PRESSED)
  {
    if(keypadAction(metaFuncs, NULL)) 
    {
      metaActionTaken = true;
      leds[1].on();
    }
    else 
      leds[1].off();
  }

  for (byte i=0; i<4; i++) leds[i].update();
}

bool keypadAction(void (*pressed_f[])(byte), void (*released_f[])(byte))
{
  bool anyPressed = false;

  // scan keypad for key presses
  for (byte i = 0; i < 12; i++)
  {
    bool has_changed;
    
    bool state = keypad[i].read(has_changed);
    
    if (has_changed)
    {
      if (state == Button::PRESSED && pressed_f[i] != NULL) 
        pressed_f[i](i);
      else if (released_f != NULL && released_f[i] != NULL)
        released_f[i](i);
    }

    if (state == Button::PRESSED)
      anyPressed = true;
  }
  return anyPressed;
}

void sendKeyOn(byte key)
{
  Tones ts = keyScale.tones[key];
  sendMidiToneOn(ts.t1);
  sendMidiToneOn(ts.t2);
  sendMidiToneOn(ts.t3);
  sendMidiToneOn(ts.t4);
}

void sendKeyOff(byte key)
{
  Tones ts = keyScale.tones[key];
  sendMidiToneOff(ts.t1);
  sendMidiToneOff(ts.t2);
  sendMidiToneOff(ts.t3);
  sendMidiToneOff(ts.t4);
}

void sendMidiToneOn(int tone)
{
  if (tone != nn) { MIDI.sendNoteOn(rootNote + tone + (12 * octave), 127, midiChannel); };
}
void sendMidiToneOff(int tone)
{
  if (tone != nn) { MIDI.sendNoteOff(rootNote + tone + (12 * octave), 0, midiChannel); };
}

void prevFifth(byte i)
{
  fifthsIndex --;
  if (fifthsIndex == -7) fifthsIndex = 5;
  rootNote -= 7;
  if (rootNote < MIDDLE_C)
    rootNote += 12;
}

void resetFifth(byte i)
{
  fifthsIndex = 0;
  rootNote = MIDDLE_C;
}

void nextFifth(byte i)
{
  fifthsIndex ++;
  if (fifthsIndex == 7) fifthsIndex = -5;
  rootNote += 7;
  if (rootNote >= MIDDLE_C + 12)
    rootNote -= 12;
}

void decreaseOctave(byte i)
{
  if (octave > -2)
    octave --;
}

void resetOctave(byte i)
{
  octave = 0;
}

void increaseOctave(byte i)
{
  if (octave < 2)
    octave ++;
}

void decreaseScale(byte i)
{
  scaleIndex =
    scaleIndex == 0 
    ? SCALE_COUNT - 1
    : scaleIndex - 1;
  
  loadKeyScale(scaleIndex);
}

void increaseScale(byte i)
{
  scaleIndex = (scaleIndex + 1) % SCALE_COUNT;
  loadKeyScale(scaleIndex);
}

void resetScale(byte i)
{
  scaleIndex = 0;
  loadKeyScale(scaleIndex);
}

// reset the midi note state
void sendMidiAllNotesOff(byte i)
{
  MIDI.sendControlChange(midi::MidiControlChangeNumber::AllNotesOff, 0, midiChannel);
}
