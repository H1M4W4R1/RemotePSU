#pragma once

#include <Arduino.h>

static constexpr uint32_t FW_SESSION_DEFAULT_TIME_LIMIT_SECONDS = 28800UL;

enum FwSessionStatus
{
    FW_SESSION_STATUS_OK = 0,
    FW_SESSION_STATUS_OUTPUT_ERROR = 1
};

FwSessionStatus fw_session_start(void);
FwSessionStatus fw_session_stop(void);
FwSessionStatus fw_session_poll(void);
bool fw_session_is_active(void);
