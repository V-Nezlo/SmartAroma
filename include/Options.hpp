/*!
@file
@brief Настройки приложения
@author V-Nezlo (vlladimirka@gmail.com)
@date 02.11.2024
@version 1.0
*/

#ifndef INCLUDE_OPTIONS_HPP_
#define INCLUDE_OPTIONS_HPP_

#include <stdint.h>

// Seconds
static constexpr uint32_t kShortWorkTime{2 * 60 * 60};
static constexpr uint32_t kMidWorkTime{4 * 60 * 60};
static constexpr uint32_t kLongWorkTime{6 * 60 * 60};

static constexpr uint32_t kTimeoutS {5};
static constexpr uint32_t kMaxBrightness{80};

#endif