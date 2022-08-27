/*
	MatrixButton - Extension of Button to scan input after writing to output pin

	MIT licensed.
*/

#ifndef MatrixButton_h
#define MatrixButton_h
#include <Arduino.h>

class MatrixButton
{
	public:
		MatrixButton(uint8_t outputPin, uint8_t inputPin, uint16_t debounce_ms = 10);
		void begin();
		bool read(bool &);

	const static bool PRESSED = LOW;
	const static bool RELEASED = HIGH;
	
	protected:
		uint8_t  _inputPin;
        uint8_t  _outputPin;
		uint16_t _delay;
		bool     _state;
		uint32_t _ignore_until;
		bool     _has_changed;
};

#endif
