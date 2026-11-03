// How much of a class of the map's cells a kind of staff patrols. Reimplementation of 0x4cd500; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The call (the port by default; the bench's recorder: its own bench has it).
struct PatrolCoverageEnv {
    void(__stdcall *mark)(u32 kind);   // Staff_MarkPatrolCells
};
void PatrolCoverage_SetEnv(const PatrolCoverageEnv *env);

extern "C" {

// 0x4cd500 (stdcall (kind, group); ECX not read; name invented, unconfirmed): Staff_MarkPatrolCells (kind), then over all 128 x 128 cells of the map [0x80239c] + 0x2d8 (read once, after the
// mark) those of the group: 0 type 2, 7 or 0x1e; 1 type 1, 3 or 9, 9, 0xa, 4, 0xb.. (MapHandle_IsTypeBDOr10Or12Or19) or 0x15; 2 type 9; 3 type 1, 3 or 9; any other none. The answer is 100 times
// those with the dword +0x3c above 0, over all of them (signed, truncated); 0 for none. Zero_5d60c0 is one more of the tests in the groups 0 and 1, and never says yes.
s32 __stdcall PatrolCoverage_Percent(u32 kind, u32 group);
}
