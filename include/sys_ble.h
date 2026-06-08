#pragma once

#include <Arduino.h>

enum SysBleStatus
{
    SYS_BLE_STATUS_OK = 0,
    SYS_BLE_STATUS_BAD_HANDLE = 1
};

SysBleStatus sys_ble_init(void);
SysBleStatus sys_ble_poll(void);
