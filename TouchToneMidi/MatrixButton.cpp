/*
	MatrixButton - Extension of Button to scan input after writing to output pin

	MIT licensed.
*/

#include "MatrixButton.h"
#include <Arduino.h>

MatrixButton::MatrixButton(uint8_t outputPin, uint8_t inputPin, uint16_t debounce_ms)
:  _inputPin(inputPin)
,  _delay(debounce_ms)
,  _state(HIGH)
,  _ignore_until(0)
,  _has_changed(false)
,  _outputPin(outputPin)
{
}

void MatrixButton::begin()
{
  pinMode(_inputPin, INPUT_PULLUP);
  pinMode(_outputPin, OUTPUT);
}

// 
// public methods
// 

bool MatrixButton::read(bool &has_changed){
    _has_changed = false;

	// ignore pin changes until after this delay time
	if (_ignore_until > millis())
	{
		return _state;
	}

  digitalWrite(_outputPin, LOW);
  
  if (digitalRead(_inputPin) == LOW)
  {
    // newly pressed
    if (_state != PRESSED)
    {
      _state = PRESSED;
      _has_changed = true;
      _ignore_until = millis() + _delay;
    }
  } // released
  else if (_state == PRESSED)
  {
    _state = RELEASED;
    _has_changed = true;
    _ignore_until = millis() + _delay;
  }
  
  digitalWrite(_outputPin, HIGH);

  has_changed = _has_changed;
  return _state;
}