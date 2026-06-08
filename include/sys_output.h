#pragma once

#include <Arduino.h>

enum SysOutputStatus
{
    SYS_OUTPUT_STATUS_OK = 0,
    SYS_OUTPUT_STATUS_BAD_PIN = 1
};

SysOutputStatus sys_output_init(uint8_t output_pin);
SysOutputStatus sys_output_activate(void);
SysOutputStatus sys_output_deactivate(void);
bool sys_output_is_active(void);
