/*!
@file
@brief Типы приложения
@author V-Nezlo (vlladimirka@gmail.com)
@date 02.11.2024
@version 1.0
*/

#ifndef INCLUDE_TYPES_HPP
#define INCLUDE_TYPES_HPP

enum class MainState {
	Sleep,
	ConfigTime,
	Working,
	Disabling,
	Block
};

enum class Events {
	ButShortPress,
	ButLongPress,
	Timeout,
	WorkTimeEnded,
	Error
};

enum class ConfigTimeState {
	ShortTime,
	MidTime,
	LongTime,
};

enum class LedEffects {
	SleepCyan,
	ConfigGreen,
	ConfigYellow,
	ConfigRed,
	WorkingFlame,
	Critical
};

#endif