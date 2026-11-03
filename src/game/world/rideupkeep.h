// What the park's opened rides cost to keep up. Reimplementation of 0x4c8240; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../core/funnytime.h"

// The calls (the ports by default; the bench's recorders: their own benches have them).
struct RideUpkeepEnv {
    u8 *(__fastcall *def)(u8 *defs, void *, u16 id);                                // ObjectDefs_Find
    u32(__fastcall *open)(u8 *object);                                              // Object_QueueOpen
    u8 *(__fastcall *uncached)(u8 *table, void *, u32 id, u32 quiet);               // ObjectControls_FindUncached
    u8 *(__fastcall *controls)(u8 *table, void *, u32 id, u32 quiet);               // ObjectControls_Find
    FileTime *(__fastcall *since)(const u8 *clock, void *, FileTime *out, u32 ticks);   // FunnyTime_SpanSince
    s32(__fastcall *age)(const u8 *object);                                         // Object_AgeDays
};
void RideUpkeep_SetEnv(const RideUpkeepEnv *env);

extern "C" {

// 0x4c8240 (thiscall on the analyser, not used; name invented, unconfirmed): the sum over the chain of things from the game [0x80239c]'s word +0x1da746 (the slots 0x7cfb90, 20 bytes each; next +0xc) with a definition (+0xe; ObjectDefs_Find on 0x7890a0)
// whose +0x4ac is 0 or 2, that Object_QueueOpen answers and whose controls (ObjectControls_FindUncached on game +0x1d943c, quiet 0) have +0x18 over 0. With the definition's +0x9c 0: days = the 64-bit span since the controls' +0x1c (ObjectControls_Find;
// FunnyTime_SpanSince on game +0x2a0) / 0xc92a69c000 (low dword), share among the controls' +0x18; else days = Object_AgeDays, share among 1. step = days / the definition's +0x5c (signed); its value the dword +0x268 + 4 * step for a step of 0 to 2, else 0.
// Each thing adds (the definition's +0x58 + that) / the share (signed). A zero divisor faults, as the original does.
s32 __fastcall RideUpkeep_Total(void *analyser);
}
