#include "savegame.h"

extern "C" {

u32 __stdcall Save_FieldName(u32, const char *, u32) { return 0; }

void __cdecl Save_InitUnusedOne() { *reinterpret_cast<float *>(0x0078aa2c) = 1.0f; }

void __cdecl Save_InitUnusedEpsilon() { *reinterpret_cast<float *>(0x0078b11c) = 1.0f / 8388608.0f; }
}
