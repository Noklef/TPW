#include "log.h"

#include <cstdarg>
#include <cstdio>
#include <share.h>
#include <windows.h>

namespace tpw::hook {

namespace {

CRITICAL_SECTION g_lock;
FILE *g_file;
DWORD g_start;

}  // namespace

void log_open(const char *path) {
    InitializeCriticalSection(&g_lock);
    g_start = GetTickCount();
    g_file = _fsopen(path, "w", _SH_DENYWR);
}

void log_line(const char *format, ...) {
    if (!g_file) return;
    EnterCriticalSection(&g_lock);
    va_list args;
    va_start(args, format);
    std::vfprintf(g_file, format, args);
    va_end(args);
    std::fputc('\n', g_file);
    std::fflush(g_file);
    LeaveCriticalSection(&g_lock);
}

double log_seconds() { return (GetTickCount() - g_start) / 1000.0; }

}  // namespace tpw::hook
