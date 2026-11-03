// The figures of an object on the dialog [0x7cc1f8]. Reimplementation of 0x4abb00; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders).
struct Dialog7cc1f8StatsEnv {
    u8 *(__fastcall *find)(u8 *defs, void *, u16 id);          // ObjectDefs_Find
    u32(__stdcall *control)(u32 id);                           // ObjectControls_GetWord18
    u16 *(__cdecl *text)(u32 index);                           // Language_WideText
    void(__fastcall *notify)(u8 *window, void *, u32 a);       // Window_NotifyImpl100
};
void Dialog7cc1f8_SetStatsEnv(const Dialog7cc1f8StatsEnv *env);

extern "C" {

// 0x4abb00 (cdecl (id: a pointer to its word)): nothing without the child 0x1ed or for a negative word. The definition of the word (ObjectDefs_Find, not checked). The child 0x1f5 (when there is one) told (Window_NotifyImpl100) the number of ObjectControls_GetWord18, or with 0 the text 0x88 as a format (sic), on a stack
// buffer of 10 units. In the mode [0x7cc1fc] 2 the child 0x1ee's (when there is one) slot 7 given def +0x13c * 1024 / 100 (a dword, wrapping). In mode 0 that too, then the child 0x1f2's slot 9 the number def +0x19c (in 0x7ba058) and the child 0x1ef's slot 7 def +0x1b0 as +0x13c.
void __cdecl Dialog7cc1f8_ShowFigures(const s16 *id);
}
