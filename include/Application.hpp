/*!
@file
@brief Класс приложения
@author V-Nezlo (vlladimirka@gmail.com)
@date 02.11.2024
@version 1.0
*/

#ifndef INCLUDE_APPLICATION_HPP_
#define INCLUDE_APPLICATION_HPP_

#include "GpioWrapper.hpp"
#include "SerialWrapper.hpp"
#include "TimeWrapper.hpp"
#include "Types.hpp"
#include "Options.hpp"
#include <EncButton.h>
#include <microDS18B20.h>
#include <microLED.h>
#include <GyverPID.h>

typedef GyverPID Regulator;

class AbstractButtonHandler {
public:
	virtual void handleButtonEvent() = 0;
};

AbstractButtonHandler *g_dev{nullptr};

template<uint8_t DSPin, uint8_t LedPin>
class Application : public AbstractButtonHandler  {
public:
	Application(uint8_t aPwmPin, uint8_t aButtonPin, uint8_t aWhiteLed1Pin, uint8_t aWhiteLed2Pin, uint8_t aWhiteLed3Pin,
		SerialWrapper &aSerial):
		pwm{aPwmPin, OUTPUT},
		button{aButtonPin, INPUT},

		whiteLed1{aWhiteLed1Pin, OUTPUT},
		whiteLed2{aWhiteLed2Pin, OUTPUT},
		whiteLed3{aWhiteLed3Pin, OUTPUT},

		serial{aSerial},

		tempSensor{},
		regulator{0.1f, 0.05f, 0, kTempReadPeriodMs},

		leds{},
		lastTempReadTime{0},
		lastSecondIncrementTime{0},
		lastLedSwitchingTime{0},
		nextLedsActionTime{0},

		timeout{0},
		targetTemperature{0.f},
		state{MainState::Sleep},
		timeState{ConfigTimeState::ShortTime},

		startTimeS{0},
		stopTimeS{0},
		error{false},
		curLedEffect{LedEffects::SleepCyan}
	{
		regulator.setLimits(0, 255); // ШИМ
		regulator.setDirection(false);
		regulator.setpoint = 50;

		g_dev = this;
	}

	// Столько костылей чтобы обойти Гайверовские ограничения, решительно осуждаю
	static void buttonHandler()
	{
		if (g_dev) {
			g_dev->handleButtonEvent();
		}
	}

	void handleButtonEvent() override
	{
		timeout = TimeWrapper::seconds() + kTimeoutS;

		const uint16_t event = button.action();
		switch (event) {
			case EB_CLICK:
				handleEvent(Events::ButShortPress);
				break;
			case EB_HOLD:
				handleEvent(Events::ButLongPress);
				break;
		}
	}

	void handleEvent(Events aEvent)
	{
		switch (state) {
			case MainState::Sleep:
				if (aEvent == Events::ButShortPress) {
					state = MainState::ConfigTime;
					timeState = ConfigTimeState::ShortTime;
				}
				break;
			case MainState::ConfigTime:
				switch (timeState) {
					case ConfigTimeState::ShortTime:
						if (aEvent == Events::ButShortPress) {
							timeState = ConfigTimeState::MidTime;
						}
						break;
					case ConfigTimeState::MidTime:
						if (aEvent == Events::ButShortPress) {
							timeState = ConfigTimeState::LongTime;
						}
						break;
					case ConfigTimeState::LongTime:
						if (aEvent == Events::ButShortPress) {
							timeState = ConfigTimeState::ShortTime;
						}
						break;
					}

					if (aEvent == Events::Timeout) {
						state = MainState::Sleep;
					} else if (aEvent == Events::ButLongPress && !error) {
						state = MainState::Working;
						startTimeS = TimeWrapper::seconds();
						stopTimeS = startTimeS + getWorkTimeSecByEnum(timeState);
					}

				break;

			case MainState::Working:
				if (aEvent == Events::ButLongPress) {
					state = MainState::Disabling;
					timeState = ConfigTimeState::ShortTime;
				} else if (aEvent == Events::WorkTimeEnded) {
					state = MainState::Disabling;
					timeState = ConfigTimeState::ShortTime;
				} else if (aEvent == Events::Error) {
					state = MainState::Block;
				}
				break;

			case MainState::Disabling:
				break;
			case MainState::Block:
				break;
		}
	}

	void init()
	{
		serial.init();
		tempSensor.setResolution(12);
		button.attach(buttonHandler);
	}

	void run()
	{
		uint32_t currentTime = TimeWrapper::milliseconds();
		button.tick();

		if (lastTempReadTime + kTempReadPeriodMs < currentTime) {
			lastTempReadTime = currentTime;

			if (tempSensor.readTemp()) {
				regulator.input = tempSensor.getTemp();
			} else {
				regulator.input = 0;
				handleEvent(Events::Error);
			}
		}
		if (lastSecondIncrementTime + kSecondIncPeriodMs < currentTime) {
			lastSecondIncrementTime = currentTime;
			TimeWrapper::secondHook();
		}
		if (currentTime > nextLedsActionTime) {
			nextLedsActionTime = currentTime + processLed(curLedEffect);
		}

		if (state == MainState::Working && lastLedSwitchingTime + kLedSwitchPeriodMs < currentTime) {
			lastSecondIncrementTime = currentTime;
			setLedByRemainingTime(getRemainingTimeInPercent());
		}
		if (state == MainState::Working && TimeWrapper::seconds() >= stopTimeS) {
			handleEvent(Events::WorkTimeEnded);
		}
		if (state == MainState::ConfigTime && TimeWrapper::seconds() >= timeout) {
			handleEvent(Events::Timeout);
		}

		processMode();
	}

private:
	Gpio pwm;
	Button button;

	Gpio whiteLed1;
	Gpio whiteLed2;
	Gpio whiteLed3;

	SerialWrapper &serial;

	MicroDS18B20<DSPin> tempSensor;
	Regulator regulator;

	microLED<3, LedPin, MLED_NO_CLOCK, LED_WS2812, ORDER_RBG> leds;

	uint32_t lastTempReadTime;
	static constexpr uint32_t kTempReadPeriodMs{100};
	uint32_t lastSecondIncrementTime;
	static constexpr uint32_t kSecondIncPeriodMs{1000};
	uint32_t lastLedSwitchingTime;
	static constexpr uint32_t kLedSwitchPeriodMs{5000};
	uint32_t nextLedsActionTime;

	uint32_t timeout;
	float targetTemperature;
	MainState state;
	ConfigTimeState timeState;
	uint32_t startTimeS;
	uint32_t stopTimeS;
	bool error;
	LedEffects curLedEffect;

	void processMode()
	{
		switch (state) {
			case MainState::Sleep:
				curLedEffect = LedEffects::SleepCyan;
				break;
			case MainState::ConfigTime:
				switch(timeState) {
					case ConfigTimeState::ShortTime:
						curLedEffect = LedEffects::ConfigGreen;
						break;
					case ConfigTimeState::MidTime:
						curLedEffect = LedEffects::ConfigYellow;
						break;
					case ConfigTimeState::LongTime:
						curLedEffect = LedEffects::ConfigRed;
						break;
				}
				break;
			case MainState::Working: {
				pwm.analogWrite(static_cast<int>(regulator.getResultNow()));
				curLedEffect = LedEffects::WorkingFlame;
			} break;
			case MainState::Disabling:
				pwm.analogWrite(0);
				whiteLed1.reset();
				whiteLed2.reset();
				whiteLed3.reset();
				state = MainState::Sleep;
				break;
			case MainState::Block:
				pwm.analogWrite(0);
				whiteLed1.reset();
				whiteLed2.reset();
				whiteLed3.reset();

				curLedEffect = LedEffects::Critical;
				break;
		}
	}

	/// @brief Обработать леды
	/// @param aEffect текущий эффект
	/// @return время в миллисекундах, которое требуется подождать для след. итерации
	uint32_t processLed(LedEffects aEffect)
	{
		uint8_t red{0}, green{0}, blue{0};     // Цветовые компоненты

		switch (aEffect) {
			case LedEffects::WorkingFlame: {
				// Имитирует огонь с изменением цвета и случайными интервалами
				red = random(200, 255);
				green = random(50, 151);
				blue = random(0, 51);
				mData color{red, green, blue};
				leds.set(1, color);
				leds.show();
				return random(30, 150);
			}

			case LedEffects::SleepCyan: {
				static uint8_t brightness = 0;
				static bool increasing = true;
				// Переливы между синим и зеленым
				if (increasing) {
					brightness++;
					if (brightness >= 255) increasing = false;
				} else {
					brightness--;
					if (brightness <= 0) increasing = true;
				}

				green = brightness;
				blue = 255 - brightness; // Обратная пропорция к зеленому
				mData color{red, green, blue};
				leds.set(1, color);
				leds.show();
				return 50; // Обновляем каждые 50 мс
			}

			case LedEffects::ConfigGreen: {
				// Зажигает зеленый цвет
				green = 255;
				mData color{red, green, blue};
				leds.set(1, color);
				leds.show();
				return 200; // Постоянный цвет, медленное обновление
			}

			case LedEffects::ConfigYellow: {
				// Зажигает желтый цвет (смешение красного и зеленого)
				red = 255;
				green = 255;
				mData color{red, green, blue};
				leds.set(1, color);
				leds.show();
				return 200;
			}

			case LedEffects::ConfigRed: {
				// Зажигает красный цвет
				red = 255;
				mData color{red, green, blue};
				leds.set(1, color);
				leds.show();
				return 200;
			}

			case LedEffects::Critical: {
				// Мигает красным с интервалом
				static bool isOn = false;
				if (isOn) {
					red = 255; // Включаем красный
				} else {
					red = 0; // Выключаем
				}
				isOn = !isOn; // Переключаем состояние
				mData color{red, green, blue};
				leds.set(1, color);
				leds.show();
				return 300; // Мигание каждые 300 мс
			}
		}

		return 100; // Значение по умолчанию, если эффект не распознан
	}

	uint8_t getRemainingTimeInPercent()
	{
		uint32_t fullTimeS = getWorkTimeSecByEnum(timeState);
		return static_cast<uint8_t>((TimeWrapper::seconds() * 100) / fullTimeS);
	}

	void setLedByRemainingTime(uint8_t aPercent)
	{
		const bool white1 = aPercent >= 0 ? true : false;
		const bool white2 = aPercent >= 33 ? true : false;
		const bool white3 = aPercent >= 66 ? true : false;

		whiteLed1.setState(white1);
		whiteLed2.setState(white2);
		whiteLed3.setState(white3);
	}

	static inline uint32_t getWorkTimeSecByEnum(ConfigTimeState aTime)
	{
		switch (aTime) {
			case ConfigTimeState::ShortTime:
				return kShortWorkTime;
			case ConfigTimeState::MidTime:
				return kMidWorkTime;
			case ConfigTimeState::LongTime:
				return kLongWorkTime;
		}

		return 0;
	}

};

#endif