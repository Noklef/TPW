// The engine's error list (0x56af20; names unconfirmed; the game's at 0x879ad0): up to 0x40 (kind, code) pairs of dwords from +0, the count at
// +0x200. Pushing onto a full list does nothing. A kind 3 push with the engine debug flag (0x1000000 of the scene flags at 0x8bd508) first writes
// the code's message (DxResult_Message, through sprintf "%s" into a 100-byte buffer, which 19 of the messages, up to 165 characters, overrun:
// sic) to the engine debug log (0x564650: enginedebug.txt, made anew on the first write of a run, then appended to).
#pragma once

#include "../../../re/types.h"

struct EngineErrorsEnv {
    void(__cdecl *log)(const char *text);  // 0x564650
};
// nullptr restores the game's.
void EngineErrors_SetEnv(const EngineErrorsEnv *e);
const EngineErrorsEnv *EngineErrors_GetEnv();

extern "C" {
// 0x56af20 (thiscall on the list; kind, code, pops 8).
void __fastcall EngineErrors_Push(u8 *list, void *, u32 kind, u32 code);
}
