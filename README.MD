# RFOutlet

RFOutlet is ESP32 firmware for a remotely controlled relay outlet. It listens for RF commands, exposes BLE session controls, and automatically deactivates the relay when the active session reaches its configured time limit.

## Hardware

- Board: WEMOS D1 R32 / ESP32
- Framework: Arduino via PlatformIO
- Relay output pin: `19` (5V relay controlled using N-MOSFET)
- RF input pin: `26` (433MHz SYN480R ASK Receiver)
- Serial baud: `115200`

The relay output is active-low: activation drives the output pin `LOW`, and deactivation drives it `HIGH`.

## RF Control

- Command `0xF55EA1`: activate relay and start the session timer
- Command `0xF55EA2`: deactivate relay and stop the session timer

Unknown RF commands are logged over serial only.

## Sessions

A session starts when the relay activates and stops when the relay deactivates. The default session time limit is `28800` seconds, equal to 8 hours. When the configured limit expires, the firmware deactivates the relay automatically.

## BLE Control

BLE device name: `RF Outlet`

Custom service:

- `ae615000-0000-4000-8000-1860a67e9f16`

Characteristics use readable ASCII string values:

| UUID                                   | Name                         | Properties          | Value                                |
|----------------------------------------|------------------------------|---------------------|--------------------------------------|
| `ae615000-0001-4000-8000-1860a67e9f16` | Session Max Time Seconds     | Read, Write         | Decimal seconds, for example `28800` |
| `ae615000-0002-4000-8000-1860a67e9f16` | Session Active Status        | Read, Write, Notify | `0` inactive, `1` active             |
| `ae615000-0003-4000-8000-1860a67e9f16` | Session Current Time Seconds | Read, Notify        | Decimal elapsed session seconds      |

Writing `1` to session status activates the relay and starts the session timer. Writing `0` deactivates the relay and stops the session timer.

## Build

```powershell
pio run
```

The project is built with `-Wall`, `-Wextra`, and `-Werror`.
