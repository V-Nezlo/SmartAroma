/*!
@file
@brief ООП обертка для времени
@author V-Nezlo (vlladimirka@gmail.com)
@date 02.11.2024
@version 1.0
*/

#ifndef INCLUDE_TIMEWRAPPER_HPP_
#define INCLUDE_TIMEWRAPPER_HPP_

#include <Arduino.h>

class TimeWrapper {
public:
	static uint32_t milliseconds()
	{
		return millis();
	}

	static uint32_t seconds()
	{
		return second;
	}

	static void secondHook()
	{
		++second;
	}

private:
	static uint32_t second;
};

uint32_t TimeWrapper::second{0};

#endif // INCLUDE_TIMEWRAPPER_HPP_
