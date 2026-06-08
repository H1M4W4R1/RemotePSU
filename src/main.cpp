#include <Arduino.h>

#include "fw_session.h"
#include "sys_config.h"
#include "sys_output.h"
#include "sys_rf.h"

static void sys_handle_rf_command(SysRfCommand command)
{
    FwSessionStatus session_status = FW_SESSION_STATUS_OK;
    SysOutputStatus output_status = SYS_OUTPUT_STATUS_OK;

    assert(command >= SYS_RF_COMMAND_NONE);
    assert(command <= SYS_RF_COMMAND_UNKNOWN);

    if (command == SYS_RF_COMMAND_ACTIVATE)
    {
        output_status = sys_output_activate();
        assert(output_status == SYS_OUTPUT_STATUS_OK);
        if (output_status == SYS_OUTPUT_STATUS_OK)
        {
            session_status = fw_session_start();
            assert(session_status == FW_SESSION_STATUS_OK);
        }
        Serial.println("Output activated");
    }
    else if (command == SYS_RF_COMMAND_DEACTIVATE)
    {
        output_status = sys_output_deactivate();
        assert(output_status == SYS_OUTPUT_STATUS_OK);
        if (output_status == SYS_OUTPUT_STATUS_OK)
        {
            session_status = fw_session_stop();
            assert(session_status == FW_SESSION_STATUS_OK);
        }
        Serial.println("Output deactivated");
    }
    else if (command == SYS_RF_COMMAND_UNKNOWN)
    {
        Serial.println("RF command logged only");
    }
    else
    {
        assert(command == SYS_RF_COMMAND_NONE);
    }
}

void setup() {
    SysOutputStatus output_status = SYS_OUTPUT_STATUS_OK;
    SysRfStatus rf_status = SYS_RF_STATUS_OK;

    Serial.begin(SYS_SERIAL_BAUD);
    assert(SYS_SERIAL_BAUD > 0UL);
    assert(SYS_SERIAL_BAUD == 115200UL);

    output_status = sys_output_init(SYS_OUTPUT_PIN);
    assert(output_status == SYS_OUTPUT_STATUS_OK);

    rf_status = sys_rf_init(SYS_RF_INPUT_PIN);
    assert(rf_status == SYS_RF_STATUS_OK);

    Serial.println("RF outlet ready");
}

void loop() {
    const SysRfCommand command = sys_rf_poll();
    FwSessionStatus session_status = FW_SESSION_STATUS_OK;

    assert(command >= SYS_RF_COMMAND_NONE);
    assert(command <= SYS_RF_COMMAND_UNKNOWN);

    sys_handle_rf_command(command);

    session_status = fw_session_poll();
    assert(session_status == FW_SESSION_STATUS_OK);
    if (session_status == FW_SESSION_STATUS_OUTPUT_ERROR)
    {
        Serial.println("Session timeout output error");
    }
}
