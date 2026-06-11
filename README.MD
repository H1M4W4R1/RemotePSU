<div align="center">
  <h1>Remote PSU</h1>
  <img src="https://github.com/H1M4W4R1/RemotePSU/blob/master/gh_images/photo.jpg" alt="Device photo"/>
</div>

This project is a remotely-controlled (BLE and/or 433MHz ASK Remote) device used to enable/disable various
machines or devices from a distance. A good example of such usage is enabling/disabling desktop mill vacuum cleaner
to evacuate chips.

## Hardware
- Board: WEMOS D1 R32 / ESP32
- Relay: 5V relay module controlled
- Relay control: FR120 N-MOSFET module
- 433MHz ASK Receiver Module (SYN480R recommended)
- 1mm² (1.5mm² recommended) wire

## Configuration
- Framework: Arduino via PlatformIO
- Relay output pin: `19` (5V relay controlled using FR120 N-MOSFET Module)
- RF input pin: `26` (433MHz SYN480R ASK Receiver)
- Serial baud: `115200`

The relay module output is active-low: activation drives the output pin `LOW`, and deactivation drives it `HIGH`.

## RF Control
- Command `0xF55EA1`: activate relay and start the session timer
- Command `0xF55EA2`: deactivate relay and stop the session timer

Unknown RF commands are logged over serial only, you can replace above commands if your remote has different ones.

## Sessions

A session starts when the relay activates and stops when the relay deactivates. The default session time limit is `28800` seconds, equal to 8 hours. When the configured limit expires, the firmware deactivates the relay automatically.

## BLE Control

BLE device name: `OST-RFO`

Custom service:

- `ae615000-0000-4000-8000-12670255c8fa`

Characteristics use readable ASCII string values:

| UUID                                   | Name                         | Properties          | Value                                |
|----------------------------------------|------------------------------|---------------------|--------------------------------------|
| `ae615000-0001-4000-8000-12670255c8fa` | Session Max Time Seconds     | Read, Write         | Decimal seconds, for example `28800` |
| `ae615000-0002-4000-8000-12670255c8fa` | Session Active Status        | Read, Write, Notify | `0` inactive, `1` active             |
| `ae615000-0003-4000-8000-12670255c8fa` | Session Current Time Seconds | Read, Notify        | Decimal elapsed session seconds      |

Writing `1` to session status activates the relay and starts the session timer. Writing `0` deactivates the relay and stops the session timer.

## Build

```powershell
pio run
```