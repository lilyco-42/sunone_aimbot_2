# Input Methods

## Control Method Guide

Set the control method with:

```ini
input_method = WIN32
```

Valid values:

```text
WIN32, GHUB, RAZER, ARDUINO, RP2350, TEENSY41_HID, KMBOX_NET, KMBOX_A, MAKCU
```

Hardware methods are explicit. If you select `RAZER` or `TEENSY41_HID` and the matching runtime is not available, the app does not switch to another method for you.

## Razer Setup

Use:

```ini
input_method = RAZER
```

The runtime wrapper loads:

```text
rzctl.dll
```

Expected project path:

```text
sunone_aimbot_2\rzctl.dll
```

The DLL can also be placed beside `ai.exe`. If the DLL is missing or exports are wrong, Razer movement will not work and no fallback control method is used.

Quick checks:

- Confirm the DLL filename is exactly `rzctl.dll`.
- Confirm it is the same architecture as the app, usually x64.
- Confirm `input_method = RAZER`.
- Watch console logs for `[Razer]` messages.

## Teensy 4.1 RawHID Setup

Use:

```ini
input_method = TEENSY41_HID
```

Default HID filters:

```ini
teensy_hid_serial = AUTO
teensy_hid_vid_filter = AUTO
teensy_hid_pid_filter = AUTO
teensy_hid_usage_page = 65451
teensy_hid_usage_id = 512
teensy_hid_open_index = 0
```

The firmware must match the expected RawHID-style packet interface. The current path sends report-ID-prefixed 64-byte packets.

Button mapping used by the control path:

| Button ID | Meaning |
|---:|---|
| `1` | Shoot / left mouse. |
| `2` | Zoom / right mouse. |
| `5` | Aiming / side button state. |

Quick checks:

- Confirm the Teensy is visible as a HID device.
- Start with all filters set to `AUTO`.
- If multiple matching devices exist, adjust `teensy_hid_open_index`.
- Watch console logs for `[Mouse] Using TEENSY41_HID input.`

Related docs:

- [Input method config](../config.md#input-method)
- [Common recipes](recipes.md)
