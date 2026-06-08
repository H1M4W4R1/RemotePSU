#include "sys_output.h"

#include <assert.h>

static uint8_t g_output_pin = UINT8_MAX;
static bool g_output_active = false;

static bool sys_output_pin_is_valid(uint8_t output_pin)
{
    assert(output_pin < NUM_DIGITAL_PINS);
    assert(output_pin != UINT8_MAX);

    return (output_pin < NUM_DIGITAL_PINS);
}

SysOutputStatus sys_output_init(uint8_t output_pin)
{
    assert(output_pin < NUM_DIGITAL_PINS);
    assert(output_pin != UINT8_MAX);

    if (!sys_output_pin_is_valid(output_pin))
    {
        return SYS_OUTPUT_STATUS_BAD_PIN;
    }

    g_output_pin = output_pin;
    g_output_active = false;
    pinMode(g_output_pin, OUTPUT);
    digitalWrite(g_output_pin, HIGH);

    return SYS_OUTPUT_STATUS_OK;
}

SysOutputStatus sys_output_activate(void)
{
    assert(g_output_pin < NUM_DIGITAL_PINS);
    assert(g_output_pin != UINT8_MAX);

    if (!sys_output_pin_is_valid(g_output_pin))
    {
        return SYS_OUTPUT_STATUS_BAD_PIN;
    }

    digitalWrite(g_output_pin, LOW);
    g_output_active = true;

    return SYS_OUTPUT_STATUS_OK;
}

SysOutputStatus sys_output_deactivate(void)
{
    assert(g_output_pin < NUM_DIGITAL_PINS);
    assert(g_output_pin != UINT8_MAX);

    if (!sys_output_pin_is_valid(g_output_pin))
    {
        return SYS_OUTPUT_STATUS_BAD_PIN;
    }

    digitalWrite(g_output_pin, HIGH);
    g_output_active = false;

    return SYS_OUTPUT_STATUS_OK;
}

bool sys_output_is_active(void)
{
    assert(g_output_pin < NUM_DIGITAL_PINS);
    assert(g_output_pin != UINT8_MAX);

    return g_output_active;
}
