// Startup trace: redirects a few of tp.exe's imports (message boxes, quitting, the registry,
// failed file opens, DirectDraw/DirectSound creation, the CD queries) through loggers, and logs
// exceptions. Only the import table changes, not game code, so a passthrough run with the trace
// on still runs the original game. Each logged call carries its return address in tp.exe.
#pragma once

namespace tpw::hook {

void install_trace();

}  // namespace tpw::hook
