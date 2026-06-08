#pragma once

#include <Arduino.h>

static constexpr uint8_t  SYS_OUTPUT_PIN   = 19U;
static constexpr uint8_t  SYS_RF_INPUT_PIN = 26U;
static constexpr uint32_t SYS_SERIAL_BAUD  = 115200UL;

static constexpr unsigned long SYS_RF_COMMAND_A = 0xF55EA1;
static constexpr unsigned long SYS_RF_COMMAND_B = 0xF55EA2;
