// The C++ runtime's terminate (MSVC's EH runtime, the C library's part).
#pragma once

#include "../../../re/types.h"

struct AddrCall;
void CrtTerminate_SetCalls(const AddrCall *table, u32 count);  // (the comparison driver's seam: _getptd and abort)

extern "C" {

// 0x682590 (cdecl; terminate): the thread's terminate handler (_getptd()'s +0x60), when set, called inside a __try whose filter takes every exception
// (_getptd called again for the call); then abort. Called by address (CRT_CxxUnhandledExceptionFilter for a C++ exception nothing caught).
void __cdecl CRT_terminate();
}
