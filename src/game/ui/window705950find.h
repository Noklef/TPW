// The list window's row found by its text (window705950.h has the class). Reimplementation of 0x65b864. The name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// What it calls (the game's by default; the bench's recorder: the library's own).
struct Window705950FindEnv {
    s32(__cdecl *compare)(const void *a, const void *b);   // 0x67c290: 0 when they are the same
};
void Window705950Find_SetEnv(const Window705950FindEnv *env);

extern "C" {

// 0x65b864 (thiscall (text), pops 4): with nodes (+0x144, each's next at +0x44) and a text: the first node's number whose text (its +0) compares the same; else -1.
s32 __fastcall Window705950_FindRow(u8 *self, void *, const void *text);
}
