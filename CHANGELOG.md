# Changelog

## 1.1.0

> **Binaries follow.** Until the 1.1.0 zip is on the Releases page, build against the 1.0.2
> library as before - it links with this header for every call except `SetCursorVisible()`.
> The changes to existing calls below reach your game from the OnePlay platform itself, with
> no rebuild.

- **New: `SetCursorVisible(bool)`** (`OpcSetCursorVisible` in C) shows or hides the mouse
  cursor on the player's screen. On touch devices OnePlay hides the cursor; a game can now
  bring it back for a screen that wants a pointer, and hide it again. Your choice holds
  while your game runs, and the platform's own setting returns when it exits.
- A game built against 1.1 on a OnePlay server still running 1.0 keeps every other call;
  only `SetCursorVisible()` returns `OPC_UNSUPPORTED` there.
- **Fixed: `OpenVirtualKeyboard()` could stall your game for up to a second** when called
  repeatedly while the platform was busy. No SDK call writes to the platform on your
  thread any more, so a busy, hung or missing platform can never hold your game up.
- **`OpenVirtualKeyboard()`, `CloseVirtualKeyboard()`, `OpenUrl()` and
  `SetCursorVisible()` now always return `OPC_OK`**, instead of `OPC_UNSUPPORTED`,
  `OPC_INVALID_ARGUMENT` or `OPC_DISCONNECTED`. If your game branched on those results,
  check the capability (`OPC_CAP_VIRTUAL_KEYBOARD`, `OPC_CAP_OPEN_URL`) before calling
  instead. No rebuild needed: this comes with the platform's library.

Additive: a game built against 1.0 keeps working unchanged. Rebuild against the 1.1 header
and library only if you want the new call.

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
