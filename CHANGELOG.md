# Changelog

## 1.0.2

- **Fixed: `OpenVirtualKeyboard()`, `CloseVirtualKeyboard()` and `OpenUrl()` could freeze the
  calling thread.** On a session where nothing was changing, the call waited until the
  platform next sent an update, which could be never. They now return at once; if the
  platform does not take the request within a second, you get `OPC_DISCONNECTED` instead
  of a hang.
- Fixed: calling `Shutdown()` from inside your event handler crashed. It now works.
- Fixed: an app name or URL that is not valid UTF-8 crashed `Initialize()` / `OpenUrl()`.
- Fixed: a malformed message from the platform could crash the game. It is now ignored.
- `KEYBOARD_SHOWN` and `KEYBOARD_HIDDEN` now arrive on a live OnePlay session after your
  own `OpenVirtualKeyboard()` / `CloseVirtualKeyboard()`, as they already did on the mock
  host. (This part is a platform change; it needs no new library.)

No API or ABI change. Replace the library; you do not need to recompile.

## 1.0.1

- `Initialize()` now waits for the platform's first session snapshot before returning
  `OPC_OK`. Previously it returned as soon as the connection was open, so a game calling
  `GetSessionInfo()` on the next line could read an empty struct and conclude it was not
  streaming at all.
- `Initialize()` can now return `OPC_DISCONNECTED`, when the platform is reachable but
  does not answer within `connectTimeoutMs`. Treat it the same way as `OPC_NOT_ON_SERVER`.

No API or ABI change. Replace the library; you do not need to recompile.

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
