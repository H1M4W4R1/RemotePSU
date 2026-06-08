#pragma once

#include <Arduino.h>

static constexpr uint32_t FW_SESSION_DEFAULT_TIME_LIMIT_SECONDS = 28800UL;

enum FwSessionStatus
{
    FW_SESSION_STATUS_OK = 0,
    FW_SESSION_STATUS_OUTPUT_ERROR = 1,
    FW_SESSION_STATUS_BAD_LIMIT = 2
};

FwSessionStatus fw_session_start(void);
FwSessionStatus fw_session_stop(void);
FwSessionStatus fw_session_poll(void);
FwSessionStatus fw_session_set_active(bool active);
FwSessionStatus fw_session_set_time_limit_seconds(uint32_t limit_seconds);
bool fw_session_is_active(void);
uint32_t fw_session_get_time_limit_seconds(void);
uint32_t fw_session_get_elapsed_seconds(void);
