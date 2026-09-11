# OnePlay GameStream SDK

A small library your game links against so it can tell what device the player is really on,
and ask the platform to do things the game cannot do itself.

When a game runs on a cloud gaming host it renders on a server and the video is streamed to
the player's phone, tablet, TV, browser or laptop. The game sees an ordinary Windows
desktop, so on its own it cannot tell a phone from a television, and it has no way to raise
a phone's on-screen keyboard. This SDK answers those questions.

**Version 1.0.1** · Windows x64 · C and C++

---

## What it gives you

| | |
|---|---|
| **Am I streaming?** | Whether a player is connected right now |
| **Who is playing?** | Device type, OS, screen resolution, orientation, frame rate, bitrate, and what inputs they have |
| **Show a keyboard** | Raises the on-screen keyboard on the player's own device |
| **Open a link** | Opens a URL in the browser on the player's device |
| **Tell me when things change** | One event handler for the whole session |

---

## Getting started

```cpp
#include "opc_gamingsdk.h"

// Once, at startup. OPC_NOT_ON_SERVER simply means "not streaming" - it is
// what most of your players will get, and it is not an error.
opc::GamingSdk::Initialize("YourGame");

// Wherever you branch on the player's device.
const OpcSessionInfo info = opc::GamingSdk::GetSessionInfo();

if (info.capabilities & OPC_CAP_TV_SCREEN) {
    UseTenFootUI();
}

if (!(info.capabilities & OPC_CAP_PHYSICAL_KEYBOARD)) {
    HideKeyboardShortcutHints();
}

// Once, at shutdown.
opc::GamingSdk::Shutdown();
```

Both a plain C API and a header-only C++ wrapper are provided. The wrapper adds nothing to
the binary; it fills in the `structSize` fields for you.

### Text entry on a touch device

```cpp
if (info.capabilities & OPC_CAP_VIRTUAL_KEYBOARD) {
    const OpcRect field { x, y, width, height };   // your render resolution
    opc::GamingSdk::OpenVirtualKeyboard(field);
}
```

What the player types arrives as ordinary key input — no special handling needed. Pass the
field rectangle: on a phone the keyboard covers the lower third of the screen, which is
where a text field usually is, so without it the keyboard covers the field it was raised
for.

### Reacting to changes

```cpp
static void OPC_CALL OnStreamEvent(const OpcEvent* event, void* context)
{
    // Called on an SDK thread. Copy what you need, hand it off, return.
    switch (event->type)
    {
    case OPC_EVENT_RESOLUTION_CHANGED:
        static_cast<YourGame*>(context)->PostToGameThread(
            RelayoutRequest{ event->session->clientResolution });
        break;

    case OPC_EVENT_SESSION_ENDING:
        static_cast<YourGame*>(context)->PostToGameThread(SaveRequest{});
        break;

    default:
        break;    // correct - see below
    }
}

opc::GamingSdk::RegisterStreamEventCallback(&OnStreamEvent, this);
```

Entirely optional. `GetSessionInfo()` is a local read with no lock and no allocation, so
polling it whenever suits you is equally valid.

---

## Four things to know

**1. It must be safe when not streaming.** The same binary ships to players on their own
machines. There, `Initialize()` returns `OPC_NOT_ON_SERVER`, every call returns a benign
default, and nothing connects, loads or allocates. Treat it as a normal outcome, not an
error to log loudly.

**2. Nothing runs per frame.** There is no tick, no pump, no polling requirement. Your
frame loop is untouched.

**3. Event handlers run on an SDK thread.** Copy what you need, post it to your own thread,
and return. Do not block, and do not touch engine objects from inside one. Handlers fire
only when something happens — a handful of times across a whole session, never on a timer.

**4. Ignore event types you do not recognise.** New ones are added over time and a shipped
game receives them without being rebuilt. A `default:` case that does nothing is correct.

---

## Zero always means "not known"

Every enum reserves `0` for "we were not told", and every string field is empty rather than
guessed. `OPC_CLIENT_UNKNOWN` does **not** mean "PC" — it means nobody said.

This is deliberate. A game branching on the device needs to tell a real answer from a
missing one, so the SDK never substitutes a plausible-looking default.

---

## Capabilities versus input mode

Two different questions, and the distinction matters:

| | Answers |
|---|---|
| `OPC_CAP_NATIVE_TOUCH` | the player **could** send touch |
| `info.inputMode` | what they **are** sending, right now |

A phone whose owner has selected trackpad mode has every touch capability and sends
*mouse* input. `inputMode` reports `OPC_INPUT_CURSOR`, which is what your game will
actually receive.

Branch on capabilities for what to **offer**, and on `inputMode` for how to **interpret**
what arrives.

---

## API summary

| Function | Purpose |
|---|---|
| `Initialize(appName)` | Connect. Call once at startup |
| `Shutdown()` | Release everything. Call once at exit |
| `IsRunningOnOnePlayServer()` | True while a player is connected and playing |
| `GetSessionInfo()` | Everything about the session, in one struct |
| `GetPlatformType()` | Shorthand for the device type |
| `GetInputMode()` | Shorthand for what the player is sending |
| `OpenVirtualKeyboard(rect, hint)` | Show the player's on-screen keyboard |
| `CloseVirtualKeyboard()` | Hide it again |
| `OpenUrl(url)` | Open a link on the player's device. `http`/`https`, max 2048 bytes |
| `RegisterStreamEventCallback(cb, ctx)` | One handler for every session event |
| `UnregisterStreamEventCallback()` | Stop delivery |

Every enum, with its numeric values, is documented at the top of
[`include/opc_gamingsdk.h`](include/opc_gamingsdk.h).

---

## Linking

```
include/opc_gamingsdk.h      the only header you include
opc_gamingsdk.lib            link against this
opc_gamingsdk_d.lib          the same library with symbols and tracing
opc_gamingsdk.dll            the runtime
```

The libraries and runtime are distributed separately from this repository, which carries
the header and the documentation. See the releases page, or contact us for a build.

`opc_gamingsdk.lib` is a **static library, not an import library**. It loads the runtime on
first use, so a machine without it simply reports "not streaming" rather than failing to
start your game. Do not replace it with an import library.

Link `opc_gamingsdk_d.lib` in debug configurations. It is the same code with symbols and
tracing through `OutputDebugString` — visible in your debugger's output window, never
written to a file:

```
[opc_gamingsdk] loaded opc_gamingsdk.dll
[opc_gamingsdk] all 15 exports bound
[opc_gamingsdk] Initialize -> OPC_OK
```

---

## Compatibility

The header is a stable ABI. Enum values and struct layouts are only ever appended to —
never renumbered, never reordered, never removed — so a game built against 1.0.0 keeps
working against later runtimes without being rebuilt.

Structs are size-prefixed. Set `structSize` before passing one in; the C++ wrapper does it
for you.

---

## Testing without a host

A mock host is available that speaks the same protocol on an ordinary desktop, with no
server involved. It can also reach states a real session cannot be put into on demand —
rotating a phone, resizing a screen, connecting a gamepad, ending a session — which are the
paths a game is most likely to get wrong. Ask us for it.

---

## License

Apache License 2.0. See [LICENSE](LICENSE).
