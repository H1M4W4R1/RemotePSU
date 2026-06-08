#include "sys_ble.h"

#include "fw_session.h"

#include <BLE2901.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <assert.h>

static const char *SYS_BLE_DEVICE_NAME = "OST-RFO";
static const char *SYS_BLE_SERVICE_UUID = "ae615000-0000-4000-8000-12670255c8fa";
static const char *SYS_BLE_LIMIT_UUID = "ae615000-0001-4000-8000-12670255c8fa";
static const char *SYS_BLE_STATUS_UUID = "ae615000-0002-4000-8000-12670255c8fa";
static const char *SYS_BLE_TIME_UUID = "ae615000-0003-4000-8000-12670255c8fa";
static const uint32_t SYS_BLE_NOTIFY_PERIOD_MS = 1000UL;
static const size_t SYS_BLE_U32_STRING_MAX_SIZE = 10U;
static const size_t SYS_BLE_U8_VALUE_SIZE = 1U;
static const size_t SYS_BLE_VALUE_BUFFER_SIZE = 11U;

static BLEServer *g_ble_server = nullptr;
static BLEService *g_session_service = nullptr;
static BLECharacteristic *g_limit_characteristic = nullptr;
static BLECharacteristic *g_status_characteristic = nullptr;
static BLECharacteristic *g_time_characteristic = nullptr;
static BLEAdvertising *g_advertising = nullptr;
static uint16_t g_service_handle = 0U;
static uint16_t g_limit_handle = 0U;
static uint16_t g_status_handle = 0U;
static uint16_t g_time_handle = 0U;
static uint32_t g_last_time_notify_ms = 0UL;
static uint32_t g_last_elapsed_seconds = UINT32_MAX;
static uint8_t g_last_status_value = UINT8_MAX;
static bool g_ble_initialized = false;
static bool g_advertising_restart_needed = false;

static void sys_ble_write_u32_string(BLECharacteristic *characteristic, uint32_t value)
{
    char data[SYS_BLE_VALUE_BUFFER_SIZE] = {0};
    const int length = snprintf(data, SYS_BLE_VALUE_BUFFER_SIZE, "%lu", static_cast<unsigned long>(value));

    assert(characteristic != nullptr);
    assert((length > 0) && (length < static_cast<int>(SYS_BLE_VALUE_BUFFER_SIZE)));

    if ((length > 0) && (length < static_cast<int>(SYS_BLE_VALUE_BUFFER_SIZE)))
    {
        characteristic->setValue(reinterpret_cast<uint8_t *>(data), static_cast<size_t>(length));
    }
}

static bool sys_ble_read_u32_string(BLECharacteristic *characteristic, uint32_t *value)
{
    size_t length = 0U;
    uint8_t *data = nullptr;
    uint32_t parsed_value = 0UL;
    uint32_t digit_value = 0UL;
    bool parsed = true;

    assert(characteristic != nullptr);
    assert(value != nullptr);

    if ((characteristic == nullptr) || (value == nullptr))
    {
        return false;
    }

    length = characteristic->getLength();
    data = characteristic->getData();

    if ((length == 0U) || (length > SYS_BLE_U32_STRING_MAX_SIZE) || (data == nullptr))
    {
        parsed = false;
    }

    for (size_t index = 0U; index < SYS_BLE_U32_STRING_MAX_SIZE; index++)
    {
        if ((index < length) && parsed)
        {
            if ((data[index] < static_cast<uint8_t>('0')) || (data[index] > static_cast<uint8_t>('9')))
            {
                parsed = false;
            }
            else
            {
                digit_value = static_cast<uint32_t>(data[index] - static_cast<uint8_t>('0'));
                if (parsed_value > ((UINT32_MAX - digit_value) / 10UL))
                {
                    parsed = false;
                }
            }

            if (parsed)
            {
                parsed_value *= 10UL;
                parsed_value += digit_value;
            }
        }
    }

    if (parsed)
    {
        *value = parsed_value;
    }

    return parsed;
}

static void sys_ble_write_status_value(uint8_t value)
{
    uint8_t data[SYS_BLE_U8_VALUE_SIZE] = {'0'};

    assert(g_status_characteristic != nullptr);
    assert(value <= 1U);

    data[0] = static_cast<uint8_t>('0' + value);
    g_status_characteristic->setValue(data, SYS_BLE_U8_VALUE_SIZE);
}

static void sys_ble_sync_values(void)
{
    const uint32_t limit_seconds = fw_session_get_time_limit_seconds();
    const uint32_t elapsed_seconds = fw_session_get_elapsed_seconds();
    const uint8_t status_value = static_cast<uint8_t>(fw_session_is_active() ? 1U : 0U);

    assert(g_limit_characteristic != nullptr);
    assert(g_time_characteristic != nullptr);

    sys_ble_write_u32_string(g_limit_characteristic, limit_seconds);
    sys_ble_write_u32_string(g_time_characteristic, elapsed_seconds);
    sys_ble_write_status_value(status_value);
}

static SysBleStatus sys_ble_cache_handles(void)
{
    assert(g_session_service != nullptr);
    assert(g_limit_characteristic != nullptr);
    assert(g_status_characteristic != nullptr);

    g_service_handle = g_session_service->getHandle();
    g_limit_handle = g_limit_characteristic->getHandle();
    g_status_handle = g_status_characteristic->getHandle();
    g_time_handle = g_time_characteristic->getHandle();

    if ((g_service_handle == 0U) || (g_limit_handle == 0U) ||
        (g_status_handle == 0U) || (g_time_handle == 0U))
    {
        assert(g_service_handle != 0U);
        return SYS_BLE_STATUS_BAD_HANDLE;
    }

    return SYS_BLE_STATUS_OK;
}

static void sys_ble_add_cud(BLECharacteristic *characteristic, const char *description)
{
    BLE2901 *user_description = new BLE2901();

    assert(characteristic != nullptr);
    assert(description != nullptr);

    user_description->setDescription(description);
    characteristic->addDescriptor(user_description);
}

class SysBleLimitCallbacks : public BLECharacteristicCallbacks
{
public:
    void onRead(BLECharacteristic *characteristic)
    {
        const uint32_t limit_seconds = fw_session_get_time_limit_seconds();

        assert(characteristic != nullptr);
        assert(limit_seconds > 0UL);

        sys_ble_write_u32_string(characteristic, limit_seconds);
    }

    void onWrite(BLECharacteristic *characteristic)
    {
        uint32_t limit_seconds = 0UL;
        bool parsed = false;
        FwSessionStatus session_status = FW_SESSION_STATUS_BAD_LIMIT;

        assert(characteristic != nullptr);

        if (characteristic == nullptr)
        {
            return;
        }

        parsed = sys_ble_read_u32_string(characteristic, &limit_seconds);
        if (parsed)
        {
            session_status = fw_session_set_time_limit_seconds(limit_seconds);
            if (session_status == FW_SESSION_STATUS_OK)
            {
                sys_ble_write_u32_string(characteristic, fw_session_get_time_limit_seconds());
            }
        }
    }
};

class SysBleStatusCallbacks : public BLECharacteristicCallbacks
{
public:
    void onRead(BLECharacteristic *characteristic)
    {
        const uint8_t status_value = static_cast<uint8_t>(fw_session_is_active() ? 1U : 0U);

        assert(characteristic != nullptr);
        assert(status_value <= 1U);

        sys_ble_write_status_value(status_value);
    }

    void onWrite(BLECharacteristic *characteristic)
    {
        size_t length = 0U;
        uint8_t *data = nullptr;
        FwSessionStatus session_status = FW_SESSION_STATUS_OK;

        assert(characteristic != nullptr);

        if (characteristic == nullptr)
        {
            return;
        }

        length = characteristic->getLength();
        data = characteristic->getData();

        if ((length == SYS_BLE_U8_VALUE_SIZE) && (data != nullptr) &&
            ((data[0] == static_cast<uint8_t>('0')) || (data[0] == static_cast<uint8_t>('1'))))
        {
            session_status = fw_session_set_active(data[0] == static_cast<uint8_t>('1'));
            assert(session_status == FW_SESSION_STATUS_OK);
            sys_ble_write_status_value(static_cast<uint8_t>(data[0] - static_cast<uint8_t>('0')));
            characteristic->notify();
        }
    }
};

class SysBleTimeCallbacks : public BLECharacteristicCallbacks
{
public:
    void onRead(BLECharacteristic *characteristic)
    {
        const uint32_t elapsed_seconds = fw_session_get_elapsed_seconds();

        assert(characteristic != nullptr);
        assert(g_time_characteristic != nullptr);

        sys_ble_write_u32_string(characteristic, elapsed_seconds);
    }
};

class SysBleServerCallbacks : public BLEServerCallbacks
{
public:
    void onConnect(BLEServer *server)
    {
        assert(server != nullptr);
        assert(g_ble_server != nullptr);
    }

    void onDisconnect(BLEServer *server)
    {
        assert(server != nullptr);
        assert(g_ble_server != nullptr);

        g_advertising_restart_needed = true;
    }

    void onDisconnect(BLEServer *server, esp_ble_gatts_cb_param_t *param)
    {
        assert(server != nullptr);
        assert(param != nullptr);

        g_advertising_restart_needed = true;
    }
};

static SysBleLimitCallbacks g_limit_callbacks;
static SysBleStatusCallbacks g_status_callbacks;
static SysBleTimeCallbacks g_time_callbacks;
static SysBleServerCallbacks g_server_callbacks;

static void sys_ble_create_characteristics(void)
{
    assert(g_session_service != nullptr);
    assert(g_limit_characteristic == nullptr);

    g_limit_characteristic = g_session_service->createCharacteristic(
        SYS_BLE_LIMIT_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    g_status_characteristic = g_session_service->createCharacteristic(
        SYS_BLE_STATUS_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
    g_time_characteristic = g_session_service->createCharacteristic(
        SYS_BLE_TIME_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

    assert(g_limit_characteristic != nullptr);
    assert(g_status_characteristic != nullptr);
    assert(g_time_characteristic != nullptr);
}

static void sys_ble_configure_characteristics(void)
{
    assert(g_limit_characteristic != nullptr);
    assert(g_status_characteristic != nullptr);

    g_limit_characteristic->setCallbacks(&g_limit_callbacks);
    g_status_characteristic->setCallbacks(&g_status_callbacks);
    g_time_characteristic->setCallbacks(&g_time_callbacks);
    sys_ble_add_cud(g_limit_characteristic, "Session Max Time Seconds");
    sys_ble_add_cud(g_status_characteristic, "Session Active Status");
    sys_ble_add_cud(g_time_characteristic, "Session Current Time Seconds");
    g_status_characteristic->addDescriptor(new BLE2902());
    g_time_characteristic->addDescriptor(new BLE2902());
    sys_ble_sync_values();
}

SysBleStatus sys_ble_init(void)
{
    SysBleStatus handle_status = SYS_BLE_STATUS_OK;

    assert(!g_ble_initialized);
    assert(SYS_BLE_NOTIFY_PERIOD_MS == 1000UL);

    BLEDevice::init(SYS_BLE_DEVICE_NAME);
    g_ble_server = BLEDevice::createServer();
    assert(g_ble_server != nullptr);
    g_ble_server->setCallbacks(&g_server_callbacks);

    g_session_service = g_ble_server->createService(String(SYS_BLE_SERVICE_UUID), 15U);
    assert(g_session_service != nullptr);
    sys_ble_create_characteristics();
    sys_ble_configure_characteristics();
    g_session_service->start();

    handle_status = sys_ble_cache_handles();
    assert(handle_status == SYS_BLE_STATUS_OK);

    g_advertising = BLEDevice::getAdvertising();
    assert(g_advertising != nullptr);
    g_advertising->addServiceUUID(SYS_BLE_SERVICE_UUID);
    g_advertising->setScanResponse(true);
    BLEDevice::startAdvertising();
    g_ble_initialized = true;

    return handle_status;
}

SysBleStatus sys_ble_poll(void)
{
    const uint8_t status_value = static_cast<uint8_t>(fw_session_is_active() ? 1U : 0U);
    const uint32_t elapsed_seconds = fw_session_get_elapsed_seconds();
    const uint32_t now_ms = millis();

    assert(g_ble_initialized);
    assert(g_status_characteristic != nullptr);

    if (status_value != g_last_status_value)
    {
        sys_ble_write_status_value(status_value);
        g_status_characteristic->notify();
        g_last_status_value = status_value;
    }

    if (((now_ms - g_last_time_notify_ms) >= SYS_BLE_NOTIFY_PERIOD_MS) ||
        (elapsed_seconds != g_last_elapsed_seconds))
    {
        sys_ble_write_u32_string(g_time_characteristic, elapsed_seconds);
        g_time_characteristic->notify();
        g_last_time_notify_ms = now_ms;
        g_last_elapsed_seconds = elapsed_seconds;
    }

    if (g_advertising_restart_needed)
    {
        BLEDevice::startAdvertising();
        g_advertising_restart_needed = false;
    }

    return SYS_BLE_STATUS_OK;
}
