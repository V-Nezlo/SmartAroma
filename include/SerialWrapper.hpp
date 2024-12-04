/*!
@file
@brief Обертка для сериала
@author V-Nezlo (vlladimirka@gmail.com)
@date 06.04.2024
@version 1.0
*/

#ifndef INCLUDE_SERIALWRAPPER_HPP
#define INCLUDE_SERIALWRAPPER_HPP

#include <Arduino.h>
#include <string.h>
#include <stdint.h>


class SerialWrapper {
public:
	SerialWrapper(uint32_t aSpeed) : speed{aSpeed}
	{}

	void init()
	{
		return Serial.begin(speed);
	}

	size_t bytesAvaillable()
	{
		return Serial.available();
	}

	size_t read(uint8_t *aBuffer, size_t aLength)
	{
		return Serial.readBytes(aBuffer, aLength);
	}

	size_t write(const uint8_t *aData, size_t aLength)
	{
		return Serial.write(aData, aLength);
	}

private:
	uint32_t speed;
};

#endif // INCLUDE_SERIALWRAPPER_HPP
