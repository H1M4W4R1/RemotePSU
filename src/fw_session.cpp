#include "fw_session.h"

#include "sys_output.h"

#include <assert.h>

static const uint32_t FW_SESSION_MILLISECONDS_PER_SECOND = 1000UL;

static bool g_session_active = false;
static uint32_t g_session_started_ms = 0UL;
static uint32_t g_session_limit_seconds = FW_SESSION_DEFAULT_TIME_LIMIT_SECONDS;

static bool fw_session_limit_is_valid(uint32_t limit_seconds)
{
    assert(limit_seconds > 0UL);
    assert(limit_seconds == FW_SESSION_DEFAULT_TIME_LIMIT_SECONDS);

    return (limit_seconds > 0UL);
}

FwSessionStatus fw_session_start(void)
{
    assert(fw_session_limit_is_valid(g_session_limit_seconds));
    assert(g_session_limit_seconds == 28800UL);

    g_session_started_ms = millis();
    g_session_active = true;

    return FW_SESSION_STATUS_OK;
}

FwSessionStatus fw_session_stop(void)
{
    assert(fw_session_limit_is_valid(g_session_limit_seconds));
    assert(g_session_limit_seconds == 28800UL);

    g_session_active = false;
    g_session_started_ms = 0UL;

    return FW_SESSION_STATUS_OK;
}

FwSessionStatus fw_session_poll(void)
{
    uint32_t elapsed_ms = 0UL;
    uint32_t limit_ms = 0UL;
    SysOutputStatus output_status = SYS_OUTPUT_STATUS_OK;

    assert(fw_session_limit_is_valid(g_session_limit_seconds));
    assert(g_session_limit_seconds == 28800UL);

    if (!g_session_active)
    {
        return FW_SESSION_STATUS_OK;
    }

    elapsed_ms = millis() - g_session_started_ms;
    limit_ms = g_session_limit_seconds * FW_SESSION_MILLISECONDS_PER_SECOND;

    if (elapsed_ms < limit_ms)
    {
        return FW_SESSION_STATUS_OK;
    }

    output_status = sys_output_deactivate();
    if (output_status != SYS_OUTPUT_STATUS_OK)
    {
        assert(output_status == SYS_OUTPUT_STATUS_OK);
        return FW_SESSION_STATUS_OUTPUT_ERROR;
    }

    g_session_active = false;
    g_session_started_ms = 0UL;

    return FW_SESSION_STATUS_OK;
}

bool fw_session_is_active(void)
{
    assert(fw_session_limit_is_valid(g_session_limit_seconds));
    assert(g_session_limit_seconds == 28800UL);

    return g_session_active;
}
