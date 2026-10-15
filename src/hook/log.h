// tpw.log, shared by the hook installer, the FPU watcher and the startup trace. Each line is
// flushed as written, so the log survives a crash.
#pragma once

namespace tpw::hook {

// Starts the log (truncating it) at `path`; lines before this are dropped.
void log_open(const char *path);

// printf-style; a newline is added. Safe from any thread.
void log_line(const char *format, ...);

// Seconds since log_open, for timestamps.
double log_seconds();

}  // namespace tpw::hook
