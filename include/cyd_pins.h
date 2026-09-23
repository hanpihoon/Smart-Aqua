#pragma once
#include <Arduino.h>

// ESP32-2432S028 / common CYD 2.8" hardware.
// LCD pins are configured in platformio.ini for TFT_eSPI.
constexpr uint8_t PIN_TFT_BL = 21;

// XPT2046 touch uses the common CYD dedicated SPI wiring.
constexpr uint8_t PIN_TOUCH_CS   = 33;
constexpr uint8_t PIN_TOUCH_IRQ  = 36;
constexpr uint8_t PIN_TOUCH_CLK  = 25;
constexpr uint8_t PIN_TOUCH_MISO = 39;
constexpr uint8_t PIN_TOUCH_MOSI = 32;

// IMPORTANT:
// External aquarium I/O is intentionally NOT enabled in this UI-preview build.
// Later use an isolated I/O board / MCP23017 rather than driving valves from CYD GPIO.
