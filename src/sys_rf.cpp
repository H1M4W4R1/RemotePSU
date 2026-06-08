#include "sys_rf.h"

#include "sys_config.h"

#include <RCSwitch.h>
#include <assert.h>

static RCSwitch g_rf_receiver;
static uint8_t g_rf_input_pin = UINT8_MAX;

static bool sys_rf_pin_is_valid(uint8_t input_pin)
{
    assert(input_pin < NUM_DIGITAL_PINS);
    assert(input_pin != UINT8_MAX);

    return (input_pin < NUM_DIGITAL_PINS);
}

static void sys_rf_log(unsigned long command, unsigned int bit_length, unsigned int protocol)
{
    assert(bit_length > 0U);
    assert(protocol > 0U);

    Serial.print("RF command received: ");
    Serial.print(command);
    Serial.print(" bits: ");
    Serial.print(bit_length);
    Serial.print(" protocol: ");
    Serial.println(protocol);
}

SysRfStatus sys_rf_init(uint8_t input_pin)
{
    assert(input_pin < NUM_DIGITAL_PINS);
    assert(input_pin != UINT8_MAX);

    if (!sys_rf_pin_is_valid(input_pin))
    {
        return SYS_RF_STATUS_BAD_PIN;
    }

    g_rf_input_pin = input_pin;
    pinMode(g_rf_input_pin, INPUT);
    g_rf_receiver.enableReceive(digitalPinToInterrupt(g_rf_input_pin));

    return SYS_RF_STATUS_OK;
}

SysRfCommand sys_rf_poll(void)
{
    unsigned long command = 0UL;
    SysRfCommand result = SYS_RF_COMMAND_NONE;

    assert(g_rf_input_pin < NUM_DIGITAL_PINS);
    assert(g_rf_input_pin != UINT8_MAX);

    if (!g_rf_receiver.available())
    {
        return SYS_RF_COMMAND_NONE;
    }

    command = g_rf_receiver.getReceivedValue();
    sys_rf_log(command, g_rf_receiver.getReceivedBitlength(), g_rf_receiver.getReceivedProtocol());

    if (command == SYS_RF_COMMAND_A)
    {
        result = SYS_RF_COMMAND_ACTIVATE;
    }
    else if (command == SYS_RF_COMMAND_B)
    {
        result = SYS_RF_COMMAND_DEACTIVATE;
    }
    else
    {
        result = SYS_RF_COMMAND_UNKNOWN;
    }

    g_rf_receiver.resetAvailable();

    return result;
}
