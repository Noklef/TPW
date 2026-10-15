// Startup trace: redirects a few of tp.exe's imports (message boxes, quitting, the registry,
// failed file opens, DirectDraw/DirectSound creation, the CD queries) through loggers, and logs
// exceptions. Each logged call carries its return address in tp.exe. Game code is untouched with
// one exception: the retail build's empty debug print (0x5da3c0, a bare RET) becomes a logger
// that keeps every register and flag, so the game's own error messages reach the log while a
// passthrough run still behaves as the original game.
#pragma once

namespace tpw::testing {

void install_trace();

}  // namespace tpw::testing
