#include "Application.hpp"
#include "GpioWrapper.hpp"
#include "SerialWrapper.hpp"
#include <Arduino.h>

static constexpr uint8_t kDSPin{5};
static constexpr uint8_t kButtonPin{1};
static constexpr uint8_t kPwmPin{3};
static constexpr uint8_t kLedsPin{4};

static constexpr uint8_t kWhiteLed1Pin{6};
static constexpr uint8_t kWhiteLed2Pin{7};
static constexpr uint8_t kWhiteLed3Pin{8};

static constexpr uint8_t kLedsCount{3};

void setup() {}
void loop() {
	SerialWrapper serial(115200);

	Application<kDSPin, kLedsPin, kLedsCount> app(kPwmPin, kButtonPin, kWhiteLed1Pin, kWhiteLed2Pin, kWhiteLed3Pin, serial);
	app.init();

	while(true) {
		app.run();

		// Unreachable
		return;
	}
}


