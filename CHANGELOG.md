# Changelog

## 1.0.0

First public release.

- Session information: device type, OS and version, client and server resolution,
  orientation, frame rate, bitrate, and a capability bitmask
- `inputMode` — whether the player is sending cursor or native touch input, observed rather
  than inferred from the device type
- `OpenVirtualKeyboard()` / `CloseVirtualKeyboard()` — raise and dismiss the on-screen
  keyboard on the player's device, with a target rectangle so it does not cover the field
- `OpenUrl()` — open a link in the browser on the player's device
- One event handler for the whole session: start and end, connection loss and resume,
  resolution and orientation changes, gamepads, keyboard shown and hidden
- Plain C API with a header-only C++ wrapper
- Static stub library, so a game that ships to machines without the runtime still starts
- Debug library with tracing through `OutputDebugString`
