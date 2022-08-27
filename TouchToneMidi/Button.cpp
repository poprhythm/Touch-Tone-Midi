/*
	Button - a small library for Arduino to handle button debouncing
	* added has_changed reference variable to read func
	
	MIT licensed (c) 2018 Michael D K Adams
*/

#include "Button.h"
#include <Arduino.h>

Button::Button(uint8_t pin, uint16_t debounce_ms)
:  _pin(pin)
,  _delay(debounce_ms)
,  _state(HIGH)
,  _ignore_until(0)
,  _has_changed(false)
{
}

void Button::begin()
{
	pinMode(_pin, INPUT_PULLUP);
}

// 
// public methods
// 

bool Button::read(bool &has_changed)
{
	_has_changed = false;
	// ignore pin changes until after this delay time
	if (_ignore_until > millis())
	{
		// ignore any changes during this period
	}
	
	// pin has changed 
	else if (digitalRead(_pin) != _state)
	{
		_ignore_until = millis() + _delay;
		_state = !_state;
		_has_changed = true;
	}

	has_changed = _has_changed;
	return _state;
}

// has the button been toggled from on -> off, or vice versa
bool Button::toggled()
{
	read(_has_changed);
	return has_changed();
}

// mostly internal, tells you if a button has changed after calling the read() function
bool Button::has_changed()
{
	if (_has_changed)
	{
		_has_changed = false;
		return true;
	}
	return false;
}

// has the button gone from off -> on
bool Button::pressed()
{
	return (read(_has_changed) == PRESSED && has_changed());
}

// has the button gone from on -> off
bool Button::released()
{
	return (read(_has_changed) == RELEASED && has_changed());
}


