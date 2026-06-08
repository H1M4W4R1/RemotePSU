#pragma once

#include <Arduino.h>

enum SysRfCommand
{
    SYS_RF_COMMAND_NONE = 0,
    SYS_RF_COMMAND_ACTIVATE = 1,
    SYS_RF_COMMAND_DEACTIVATE = 2,
    SYS_RF_COMMAND_UNKNOWN = 3
};

enum SysRfStatus
{
    SYS_RF_STATUS_OK = 0,
    SYS_RF_STATUS_BAD_PIN = 1
};

SysRfStatus sys_rf_init(uint8_t input_pin);
SysRfCommand sys_rf_poll(void);
