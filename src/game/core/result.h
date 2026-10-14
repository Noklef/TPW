// The Win32 error as a COM result: reimplementation of 0x5fa650.
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x5fa650 (&out): GetLastError as an HRESULT (0x8007xxxx, the code's low 16 bits), 0 when there
// is none; returns out.
s32 *__cdecl Result_FromLastError(s32 *out);
}

// The call goes through getLastError (nullptr: the game's import table, 0x6fd138).
void Result_SetEnv(u32(__stdcall *const *getLastError)());
