#include "fw_session.h"

#include "sys_output.h"

#include <assert.h>

static const uint32_t FW_SESSION_MILLISECONDS_PER_SECOND = 1000UL;
static const uint32_t FW_SESSION_MAX_TIME_LIMIT_SECONDS = UINT32_MAX / FW_SESSION_MILLISECONDS_PER_SECOND;

static bool g_session_active = false;
static uint32_t g_session_started_ms = 0UL;
static uint32_t g_session_limit_seconds = FW_SESSION_DEFAULT_TIME_LIMIT_SECONDS;

static bool fw_session_limit_is_valid(uint32_t limit_seconds)
{
    return ((limit_seconds > 0UL) && (limit_seconds <= FW_SESSION_MAX_TIME_LIMIT_SECONDS));
}

FwSessionStatus fw_session_start(void)
{
    assert(fw_session_limit_is_valid(g_session_limit_seconds));
    assert(g_session_limit_seconds > 0UL);

    g_session_started_ms = millis();
    g_session_active = true;

    return FW_SESSION_STATUS_OK;
}

FwSessionStatus fw_session_stop(void)
{
    assert(fw_session_limit_is_valid(g_session_limit_seconds));
    assert(g_session_limit_seconds > 0UL);

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
    assert(g_session_limit_seconds > 0UL);

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

FwSessionStatus fw_session_set_active(bool active)
{
    SysOutputStatus output_status = SYS_OUTPUT_STATUS_OK;
    FwSessionStatus session_status = FW_SESSION_STATUS_OK;

    assert(fw_session_limit_is_valid(g_session_limit_seconds));
    assert((active == true) || (active == false));

    if (active)
    {
        output_status = sys_output_activate();
        if (output_status != SYS_OUTPUT_STATUS_OK)
        {
            assert(output_status == SYS_OUTPUT_STATUS_OK);
            return FW_SESSION_STATUS_OUTPUT_ERROR;
        }

        session_status = fw_session_start();
    }
    else
    {
        output_status = sys_output_deactivate();
        if (output_status != SYS_OUTPUT_STATUS_OK)
        {
            assert(output_status == SYS_OUTPUT_STATUS_OK);
            return FW_SESSION_STATUS_OUTPUT_ERROR;
        }

        session_status = fw_session_stop();
    }

    assert(session_status == FW_SESSION_STATUS_OK);
    return session_status;
}

FwSessionStatus fw_session_set_time_limit_seconds(uint32_t limit_seconds)
{
    if (!fw_session_limit_is_valid(limit_seconds))
    {
        return FW_SESSION_STATUS_BAD_LIMIT;
    }

    assert(limit_seconds > 0UL);
    assert(limit_seconds <= FW_SESSION_MAX_TIME_LIMIT_SECONDS);

    g_session_limit_seconds = limit_seconds;

    return FW_SESSION_STATUS_OK;
}

bool fw_session_is_active(void)
{
    assert(fw_session_limit_is_valid(g_session_limit_seconds));
    assert(g_session_limit_seconds > 0UL);

    return g_session_active;
}

uint32_t fw_session_get_time_limit_seconds(void)
{
    assert(fw_session_limit_is_valid(g_session_limit_seconds));
    assert(g_session_limit_seconds > 0UL);

    return g_session_limit_seconds;
}

uint32_t fw_session_get_elapsed_seconds(void)
{
    uint32_t elapsed_seconds = 0UL;

    assert(fw_session_limit_is_valid(g_session_limit_seconds));
    assert(g_session_limit_seconds > 0UL);

    if (g_session_active)
    {
        elapsed_seconds = (millis() - g_session_started_ms) / FW_SESSION_MILLISECONDS_PER_SECOND;
    }

    return elapsed_seconds;
}
