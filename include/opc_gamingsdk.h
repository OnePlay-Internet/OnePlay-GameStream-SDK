/**
 * opc_gamingsdk.h
 * OnePlay Gaming SDK  -  v1.0
 *
 * Lets your game find out what device the player is actually on, and ask the OnePlay
 * platform to do things the game cannot do itself.
 *
 * When your game runs on OnePlay, it renders on a server in our data centre and the video
 * is streamed to the player's phone, TV, browser or laptop. The game sees a Windows
 * desktop, so on its own it cannot tell a phone from a television, and it has no way to
 * raise a phone's on-screen keyboard. This SDK answers those questions.
 *
 *
 *   TYPICAL USE
 *   ---------------------------------------------------------------------------------
 *   #include "opc_gamingsdk.h"
 *
 *   // Once, at startup. OPC_NOT_ON_SERVER simply means "not running on OnePlay".
 *   opc::GamingSdk::Initialize("YourGame");
 *
 *   // Wherever you branch on the player's device.
 *   const OpcSessionInfo info = opc::GamingSdk::GetSessionInfo();
 *   if (info.capabilities & OPC_CAP_TV_SCREEN) {
 *       UseTenFootUI();
 *   }
 *
 *   // When a text field gains focus and the player has no real keyboard.
 *   if (info.capabilities & OPC_CAP_VIRTUAL_KEYBOARD) {
 *       opc::GamingSdk::OpenVirtualKeyboard(fieldRect);
 *   }
 *
 *   // Optional: one handler for everything that happens during a session.
 *   // Called on our thread - post the work to your own and return.
 *   opc::GamingSdk::RegisterStreamEventCallback(&OnOnePlayEvent, this);
 *
 *   // Once, at shutdown.
 *   opc::GamingSdk::Shutdown();
 *   ---------------------------------------------------------------------------------
 *
 *
 *   FOUR THINGS TO KNOW
 *
 *   1.  Safe off OnePlay.  The same build ships to players on their own PCs. There,
 *       Initialize() returns OPC_NOT_ON_SERVER, every call returns a harmless default,
 *       and nothing connects or allocates. That is a normal outcome, not an error.
 *
 *   2.  Nothing blocks.  Initialize() connects in the background and returns quickly.
 *       Every other call reads a local snapshot, so it is safe on your frame thread.
 *
 *   3.  Nothing to call per frame.  There is no tick, no pump, no polling requirement.
 *       Read GetSessionInfo() whenever you want it, and register an event handler if you
 *       want to be told about changes. Your frame loop is untouched.
 *
 *       Event handlers are called on our thread, not yours. Keep them short: copy what
 *       you need, post it to your own thread, return. Do not touch engine objects there.
 *
 *   4.  Set structSize before passing a struct in. The C++ wrapper below does it for you.
 *       It is what lets a game built against v1 keep working on a later platform.
 *
 *
 *   ENUM REFERENCE
 *
 *   Every value below is fixed. They are the wire contract between a game built once and
 *   a platform that keeps changing underneath it, so numbers are only ever appended -
 *   never renumbered, never reordered, never removed. Values are given because a partner
 *   reading a memory dump or a log needs them.
 *
 *   In every enum, 0 means "we were not told". It never means a real value. That is what
 *   lets a zeroed struct be distinguished from a genuine answer, and it is why
 *   OPC_CLIENT_UNKNOWN does not mean "PC".
 *
 *   OpcStatus                     result of a call
 *     0  OPC_OK                   succeeded
 *     1  OPC_NOT_ON_SERVER        not running on OnePlay. Expected, not an error
 *     2  OPC_NO_SESSION           on OnePlay, but no player is connected
 *     3  OPC_UNSUPPORTED          not available for this player's device
 *     4  OPC_NOT_INITIALIZED      Initialize() not called, or it did not succeed
 *     5  OPC_INVALID_ARGUMENT     null pointer, bad structSize, or a rejected URL
 *     6  OPC_DISCONNECTED         the platform went away. Try again later
 *     7  OPC_INTERNAL_ERROR
 *
 *   OpcClientType                 the kind of device the player is on
 *     0  OPC_CLIENT_UNKNOWN       not "PC" - we simply were not told
 *     1  OPC_CLIENT_ANDROID
 *     2  OPC_CLIENT_IOS
 *     3  OPC_CLIENT_WINDOWS
 *     4  OPC_CLIENT_MACOS
 *     5  OPC_CLIENT_LINUX
 *     6  OPC_CLIENT_WEB           playing in a browser
 *     7  OPC_CLIENT_TV            Android TV, Fire TV, Tizen, webOS, tvOS.
 *                                 Reported as TV, not the OS underneath
 *
 *   OpcTransport                  how the player's app connects
 *     0  OPC_TRANSPORT_UNKNOWN
 *     1  OPC_TRANSPORT_NATIVE     the OnePlay app
 *     2  OPC_TRANSPORT_WEBRTC     a browser
 *
 *   OpcInputMode                  what the player is ACTUALLY sending. Observed, not
 *                                 assumed - a touch-capable phone in trackpad mode
 *                                 reports CURSOR
 *     0  OPC_INPUT_UNKNOWN        nothing has arrived yet
 *     1  OPC_INPUT_CURSOR         mouse. The game sees a cursor
 *     2  OPC_INPUT_TOUCH          native touch. Real contacts with ids
 *
 *   OpcOrientation                handhelds only
 *     0  OPC_ORIENTATION_UNKNOWN
 *     1  OPC_ORIENTATION_PORTRAIT
 *     2  OPC_ORIENTATION_LANDSCAPE
 *
 *   OpcCapability                 bit flags, OR'd together in OpcSessionInfo.capabilities.
 *                                 A clear bit means "no, or we do not know"
 *     0x01  OPC_CAP_TOUCH             the player can touch the screen
 *     0x02  OPC_CAP_PHYSICAL_KEYBOARD a real keyboard is attached
 *     0x04  OPC_CAP_GAMEPAD           a gamepad is attached
 *     0x08  OPC_CAP_TV_SCREEN         ten-foot viewing distance
 *     0x10  OPC_CAP_VIRTUAL_KEYBOARD  OpenVirtualKeyboard() will work
 *     0x20  OPC_CAP_OPEN_URL          OpenUrl() will work
 *     0x40  OPC_CAP_NATIVE_TOUCH      the client CAN send real touch contacts.
 *                                     Whether it currently IS, is inputMode above
 *
 *   OpcSessionState
 *     0  OPC_SESSION_NONE         nobody is connected
 *     1  OPC_SESSION_ACTIVE       a player is playing
 *     2  OPC_SESSION_ENDING       closing down. Save now
 *
 *   OpcKeyboardHint               passed to OpenVirtualKeyboard()
 *     0  OPC_KEYBOARD_TEXT
 *     1  OPC_KEYBOARD_NUMBER
 *     2  OPC_KEYBOARD_EMAIL
 *
 *   OpcEventType                  delivered to your stream event handler.
 *                                 SWITCH ON THIS AND IGNORE WHAT YOU DO NOT RECOGNISE -
 *                                 we add types over time and a shipped game receives them
 *                                 without being rebuilt
 *      0  OPC_EVENT_UNKNOWN
 *      1  OPC_EVENT_SESSION_STARTED
 *      2  OPC_EVENT_SESSION_ENDING          your cue to save
 *      3  OPC_EVENT_CLIENT_DISCONNECTED     lost connection. The session stays alive
 *      4  OPC_EVENT_CLIENT_RESUMED          the player is back
 *      5  OPC_EVENT_RESOLUTION_CHANGED
 *      6  OPC_EVENT_ORIENTATION_CHANGED
 *      7  OPC_EVENT_GAMEPAD_CONNECTED       gamepadIndex says which
 *      8  OPC_EVENT_GAMEPAD_DISCONNECTED
 *      9  OPC_EVENT_KEYBOARD_SHOWN
 *     10  OPC_EVENT_KEYBOARD_HIDDEN         the player can dismiss it themselves
 */

#ifndef OPC_GAMINGSDK_H
#define OPC_GAMINGSDK_H

#include <stddef.h>
#include <stdint.h>

#define OPC_SDK_VERSION_MAJOR 1
#define OPC_SDK_VERSION_MINOR 0
#define OPC_SDK_VERSION_PATCH 1

#if defined(_WIN32)
  #define OPC_CALL __cdecl
#else
  #define OPC_CALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================== */
/*  Result codes                                                               */
/* =========================================================================== */

typedef enum OpcStatus {
  OPC_OK                = 0,
  OPC_NOT_ON_SERVER     = 1,  /* Not running on OnePlay. Expected, not an error. */
  OPC_NO_SESSION        = 2,  /* On OnePlay, but no player is connected.         */
  OPC_UNSUPPORTED       = 3,  /* Not available for this player's device.         */
  OPC_NOT_INITIALIZED   = 4,  /* Initialize() not called, or it did not succeed. */
  OPC_INVALID_ARGUMENT  = 5,  /* Null pointer, or structSize not set.            */
  OPC_DISCONNECTED      = 6,  /* Platform went away. Try again later.            */
  OPC_INTERNAL_ERROR    = 7
} OpcStatus;

/* =========================================================================== */
/*  The player's device                                                        */
/* =========================================================================== */

/* Televisions report as OPC_CLIENT_TV rather than the OS they run on, because a
 * ten-foot UI is usually the decision that follows. */
typedef enum OpcClientType {
  OPC_CLIENT_UNKNOWN = 0,     /* We were not told. Not the same as "PC".         */
  OPC_CLIENT_ANDROID = 1,
  OPC_CLIENT_IOS     = 2,
  OPC_CLIENT_WINDOWS = 3,
  OPC_CLIENT_MACOS   = 4,
  OPC_CLIENT_LINUX   = 5,
  OPC_CLIENT_WEB     = 6,     /* Playing in a browser.                           */
  OPC_CLIENT_TV      = 7      /* Android TV, Fire TV, Tizen, webOS, tvOS.        */
} OpcClientType;

/* How the player's app connects. */
typedef enum OpcTransport {
  OPC_TRANSPORT_UNKNOWN = 0,
  OPC_TRANSPORT_NATIVE  = 1,  /* The OnePlay app.                                */
  OPC_TRANSPORT_WEBRTC  = 2   /* A browser.                                      */
} OpcTransport;

/* How the player's input is actually reaching the game.
 *
 * This is observed, not configured: it reports the kind of input packets the platform has
 * seen from this player, not what their device is theoretically capable of. A phone whose
 * user has selected trackpad mode sends mouse input, and this says CURSOR - which is what
 * the game needs to know, because that is what it will receive. */
typedef enum OpcInputMode {
  OPC_INPUT_UNKNOWN = 0,  /* Nothing has arrived yet, or nobody is connected.     */
  OPC_INPUT_CURSOR  = 1,  /* Mouse input. The game sees a cursor.                 */
  OPC_INPUT_TOUCH   = 2   /* Native touch. The game sees real contacts with ids.  */
} OpcInputMode;

typedef enum OpcOrientation {
  OPC_ORIENTATION_UNKNOWN   = 0,
  OPC_ORIENTATION_PORTRAIT  = 1,
  OPC_ORIENTATION_LANDSCAPE = 2
} OpcOrientation;

/* Bit flags in OpcSessionInfo.capabilities. A clear bit means no, or unknown.
 *
 * Prefer these over OpcClientType where you can: they answer the question you
 * actually have ("is there a keyboard?") instead of one you have to infer. */
typedef enum OpcCapability {
  OPC_CAP_TOUCH             = 1u << 0,  /* Touch controls are available.         */
  OPC_CAP_PHYSICAL_KEYBOARD = 1u << 1,  /* A real keyboard is attached.          */
  OPC_CAP_GAMEPAD           = 1u << 2,  /* A gamepad is attached.                */
  OPC_CAP_TV_SCREEN         = 1u << 3,  /* Ten-foot viewing distance.            */
  OPC_CAP_VIRTUAL_KEYBOARD  = 1u << 4,  /* OpenVirtualKeyboard() will work.      */
  OPC_CAP_OPEN_URL          = 1u << 5,  /* OpenUrl() will work.                  */
  OPC_CAP_NATIVE_TOUCH      = 1u << 6   /* The player's client can send real touch
                                           contacts, not just mouse input.       */
} OpcCapability;

typedef enum OpcSessionState {
  OPC_SESSION_NONE   = 0,     /* Nobody is connected.                            */
  OPC_SESSION_ACTIVE = 1,     /* A player is playing.                            */
  OPC_SESSION_ENDING = 2      /* The session is closing. Save now.               */
} OpcSessionState;

typedef struct OpcResolution {
  int32_t width;              /* 0 when not known. */
  int32_t height;             /* 0 when not known. */
} OpcResolution;

/* Everything we know about the current session.
 *
 * Anything we were not told is left at zero - an empty string, a 0x0 resolution,
 * or an _UNKNOWN value. Zero always means "not known", never a guess, so you can
 * tell the difference between a real answer and no answer. */
typedef struct OpcSessionInfo {
  uint32_t structSize;              /* Set to sizeof(OpcSessionInfo).            */

  int32_t         onOpcServer;      /* Nonzero if this machine is a OnePlay server. */
  int32_t         runningOnVm;      /* Nonzero if that server is a virtual machine. */
  OpcSessionState state;
  OpcTransport    transport;
  OpcClientType   clientType;
  OpcOrientation  orientation;
  uint32_t        capabilities;     /* OpcCapability flags, OR'd together.       */

  OpcResolution   serverResolution; /* What your game is rendering at.           */
  OpcResolution   clientResolution; /* The player's actual screen. Often smaller
                                     * or a different aspect - useful for text
                                     * size and safe-area decisions.            */

  int32_t         fps;              /* Streaming frame rate. Fixed per session.  */
  int32_t         bitrateKbps;      /* Current bitrate. THIS MOVES - we adapt it
                                     * continuously to network conditions, so it
                                     * is a reading, not a setting. Re-read it,
                                     * and smooth it before showing a player.   */

  char clientTypeRaw[32];           /* Raw device string, always filled in even
                                     * when clientType is UNKNOWN - so a device
                                     * we add later is still visible to you.    */
  char clientOs[32];                /* e.g. "Android"                           */
  char clientOsVersion[32];         /* e.g. "14"                                */
  char clientAppVersion[32];        /* OnePlay app version.                     */

  /* What kind of input this player is actually sending. Observed, not assumed. */
  OpcInputMode    inputMode;

  uint8_t reserved[60];             /* Room for future fields. Ignore.          */
} OpcSessionInfo;

/* =========================================================================== */
/*  On-screen keyboard                                                         */
/* =========================================================================== */

typedef struct OpcRect {
  int32_t x;
  int32_t y;
  int32_t width;
  int32_t height;
} OpcRect;

typedef enum OpcKeyboardHint {
  OPC_KEYBOARD_TEXT   = 0,
  OPC_KEYBOARD_NUMBER = 1,
  OPC_KEYBOARD_EMAIL  = 2
} OpcKeyboardHint;

typedef struct OpcKeyboardRequest {
  uint32_t structSize;        /* Set to sizeof(OpcKeyboardRequest).              */

  /* Where your text field is, in your own render resolution. The player's device
   * pans its view so the keyboard does not cover it. Worth filling in: on a
   * phone the keyboard covers the lower third of the screen. */
  OpcRect fieldRect;

  OpcKeyboardHint hint;

  uint8_t reserved[32];
} OpcKeyboardRequest;

/* =========================================================================== */
/*  Events                                                                     */
/*                                                                             */
/*  One callback for everything. Register once, switch on event->type.         */
/* =========================================================================== */

typedef enum OpcEventType {
  OPC_EVENT_UNKNOWN = 0,

  /* Session lifecycle */
  OPC_EVENT_SESSION_STARTED       = 1,   /* A player connected and is playing.  */
  OPC_EVENT_SESSION_ENDING        = 2,   /* Closing down. Save now.             */

  /* Connection - the session stays alive across a brief drop */
  OPC_EVENT_CLIENT_DISCONNECTED   = 3,   /* Player lost the connection.         */
  OPC_EVENT_CLIENT_RESUMED        = 4,   /* Player is back.                     */

  /* Display */
  OPC_EVENT_RESOLUTION_CHANGED    = 5,   /* Player's screen size changed.       */
  OPC_EVENT_ORIENTATION_CHANGED   = 6,   /* Phone or tablet rotated.            */

  /* Input */
  OPC_EVENT_GAMEPAD_CONNECTED     = 7,
  OPC_EVENT_GAMEPAD_DISCONNECTED  = 8,
  OPC_EVENT_KEYBOARD_SHOWN        = 9,   /* On-screen keyboard came up.         */
  OPC_EVENT_KEYBOARD_HIDDEN       = 10   /* Player dismissed it.                */
} OpcEventType;

typedef struct OpcEvent {
  uint32_t     structSize;    /* Size of this struct, set by us.                */
  OpcEventType type;

  /* The session as it was when the event happened. Read this rather than
   * calling GetSessionInfo() from inside the handler - it is the state the
   * event is describing, and it cannot race with a later change.
   *
   * Valid only for the duration of the callback. Copy what you need. */
  const OpcSessionInfo *session;

  /* Which gamepad, for the GAMEPAD_ events. 0 otherwise. */
  int32_t gamepadIndex;

  uint8_t reserved[32];
} OpcEvent;

/* Your handler.
 *
 * CALLED ON A OnePlay THREAD, NOT YOURS. Treat it exactly as you would any other
 * asynchronous notification: copy what you need out of the event, post it to your
 * own thread, and return. Do not block, do not take a lock your frame loop holds,
 * and do not touch engine objects.
 *
 * We do not call you back on a timer, only when something actually happens - so
 * for a typical session this fires a handful of times, not per frame. */
typedef void(OPC_CALL *OpcStreamEventCallback)(const OpcEvent *event, void *context);

/* =========================================================================== */
/*  API                                                                        */
/* =========================================================================== */

typedef struct OpcInitParams {
  uint32_t structSize;        /* Set to sizeof(OpcInitParams).                   */
  const char *appName;        /* Your title, for our logs. May be NULL.          */
  uint32_t connectTimeoutMs;  /* 0 uses the default (250 ms). Never exceeded.    */
  uint8_t reserved[32];
} OpcInitParams;

/* Call once at startup. Safe to call again; later calls return the first result.
 *
 * Returns OPC_NOT_ON_SERVER when not running on OnePlay, which is not a failure.
 *
 * On OPC_OK the session is already populated, so calling GetSessionInfo() on the
 * very next line gives you real values rather than an empty struct.
 *
 * Returns OPC_DISCONNECTED if we were reachable but did not answer within
 * connectTimeoutMs. Treat it as "not streaming": the same fallback you use for
 * OPC_NOT_ON_SERVER is the right one. */
OpcStatus OPC_CALL OpcInitialize(const OpcInitParams *params);

/* Call once at shutdown. Safe to call even if Initialize() was not. */
void OPC_CALL OpcShutdown(void);

/* Version of the loaded SDK, which may be newer than this header. */
void OPC_CALL OpcGetVersion(uint32_t *major, uint32_t *minor, uint32_t *patch);

/* Fills in the current session. Set info->structSize first. */
OpcStatus OPC_CALL OpcGetSessionInfo(OpcSessionInfo *info);

/* Nonzero while a player is connected and playing.
 *
 * A OnePlay machine sitting idle between players returns 0 here - read
 * OpcSessionInfo.onOpcServer if you want "is this a OnePlay machine at all". */
int32_t OPC_CALL OpcIsRunningOnOnePlayServer(void);

/* Shorthand for OpcGetSessionInfo().clientType. */
OpcClientType OPC_CALL OpcGetPlatformType(void);

/* Asks the player's device to show its on-screen keyboard.
 *
 * What the player types arrives as ordinary key input - you do not need to handle
 * anything special to receive it.
 *
 * Check OPC_CAP_VIRTUAL_KEYBOARD first. Without it this returns OPC_UNSUPPORTED
 * and no keyboard appears, which would leave a player with no way to type.
 *
 * request may be NULL, meaning "no rect, plain text". */
OpcStatus OPC_CALL OpcOpenVirtualKeyboard(const OpcKeyboardRequest *request);

/* Hides a keyboard shown by OpcOpenVirtualKeyboard(). */
OpcStatus OPC_CALL OpcCloseVirtualKeyboard(void);

/* Opens a URL in the browser on the player's own device.
 *
 * For anything the player has to complete outside the game - an account link, a purchase,
 * a support page. Opening it on the server would put the page on a machine they cannot
 * reach; this puts it in front of them.
 *
 * Check OPC_CAP_OPEN_URL first. Without it this returns OPC_UNSUPPORTED.
 *
 * Must be http or https, and at most 2048 bytes. Anything else is rejected with
 * OPC_INVALID_ARGUMENT rather than passed on - a URL from a game becomes a link on a
 * player's phone, and the scheme is the difference between opening a web page and handing
 * an arbitrary handler on their device something to act on.
 *
 * The platform does not report whether the player actually opened it. */
OpcStatus OPC_CALL OpcOpenUrl(const char *url);

/* Registers your single event handler. Everything that happens during a session
 * arrives here - session start and end, the player dropping and coming back,
 * screen and orientation changes, gamepads, the on-screen keyboard.
 *
 * Entirely optional. If you would rather poll, GetSessionInfo() is a local read
 * with no lock and no allocation; call it whenever suits you and skip this.
 *
 * Registering again replaces the previous handler.
 *
 * SWITCH ON event->type AND IGNORE ANYTHING YOU DO NOT RECOGNISE. We add event
 * types over time, and a game built against this header will start receiving
 * them without being rebuilt. A default case that does nothing is correct. */
OpcStatus OPC_CALL OpcRegisterStreamEventCallback(OpcStreamEventCallback callback,
                                                  void *context);

/* Stops delivery. Also happens automatically on Shutdown(). */
OpcStatus OPC_CALL OpcUnregisterStreamEventCallback(void);

/* Static string for logging. Never NULL. */
const char *OPC_CALL OpcEventTypeToString(OpcEventType type);

/* Static strings for logging. Never NULL. */
const char *OPC_CALL OpcStatusToString(OpcStatus status);
const char *OPC_CALL OpcClientTypeToString(OpcClientType type);
const char *OPC_CALL OpcInputModeToString(OpcInputMode mode);

#ifdef __cplusplus
}  /* extern "C" */

/* =========================================================================== */
/*  C++ convenience wrapper                                                    */
/*                                                                             */
/*  Header-only and inline - it adds nothing to the DLL and simply fills in the */
/*  structSize fields for you. Use it or the C API above, whichever you prefer. */
/* =========================================================================== */

namespace opc {

  class GamingSdk {
  public:
    static OpcStatus Initialize(const char *appName = nullptr) {
      OpcInitParams params {};
      params.structSize = static_cast<uint32_t>(sizeof(params));
      params.appName = appName;
      return OpcInitialize(&params);
    }

    static void Shutdown() {
      OpcShutdown();
    }

    /** True while a player is connected and playing. */
    static bool IsRunningOnOnePlayServer() {
      return OpcIsRunningOnOnePlayServer() != 0;
    }

    static OpcClientType GetPlatformType() {
      return OpcGetPlatformType();
    }

    /** Returns a zeroed struct if there is no session, so it is always safe to read. */
    static OpcSessionInfo GetSessionInfo() {
      OpcSessionInfo info {};
      info.structSize = static_cast<uint32_t>(sizeof(info));
      if (OpcGetSessionInfo(&info) != OPC_OK) {
        info = OpcSessionInfo {};
        info.structSize = static_cast<uint32_t>(sizeof(info));
      }
      return info;
    }

    /** Shorthand: GamingSdk::Has(OPC_CAP_TOUCH) */
    static bool Has(OpcCapability capability) {
      return (GetSessionInfo().capabilities & static_cast<uint32_t>(capability)) != 0;
    }

    static OpcStatus OpenVirtualKeyboard(const OpcRect &fieldRect,
                                         OpcKeyboardHint hint = OPC_KEYBOARD_TEXT) {
      OpcKeyboardRequest request {};
      request.structSize = static_cast<uint32_t>(sizeof(request));
      request.fieldRect = fieldRect;
      request.hint = hint;
      return OpcOpenVirtualKeyboard(&request);
    }

    static OpcStatus CloseVirtualKeyboard() {
      return OpcCloseVirtualKeyboard();
    }

    /** Opens a URL in the browser on the player's device. Check OPC_CAP_OPEN_URL first. */
    static OpcStatus OpenUrl(const char *url) {
      return OpcOpenUrl(url);
    }

    /** What kind of input this player is actually sending. */
    static OpcInputMode GetInputMode() {
      return GetSessionInfo().inputMode;
    }

    /** Optional. The handler is called on a OnePlay thread - see its declaration. */
    static OpcStatus RegisterStreamEventCallback(OpcStreamEventCallback callback,
                                                 void *context) {
      return OpcRegisterStreamEventCallback(callback, context);
    }

    static OpcStatus UnregisterStreamEventCallback() {
      return OpcUnregisterStreamEventCallback();
    }
  };

}  // namespace opc

#endif /* __cplusplus */
#endif /* OPC_GAMINGSDK_H */
