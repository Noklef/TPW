// The "End" object (0x516b00): the first object definition named by the WString at 0x7cf9c0 ("End"), an object of it made at cell 1,
// its handle kept at 0x7cf4f4 (read by Word7cf4f4_NonZero) and the object passed on to 0x7b51f0's 0x481b60.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What the "End" object's making calls in the game that is not ported yet (the benches point it at recorders).
struct EndThingEnv {
    u8 *(__fastcall *construct)(u8 *block, void *, u32 cell, u32 id, s32 angle, u32 ride);   // 0x4db090 (agent C's object constructor, pops 0x10)
    void(__fastcall *pass)(void *owner, void *, u8 *object);                                   // 0x481b60 (agent C's; thiscall on 0x7b51f0, this not used, pops 4)
};
// The calls go to env (nullptr: the game's).
void EndThing_SetEnv(const EndThingEnv *env);

// 0x516b00 (cdecl; name invented, unconfirmed): the object definitions walked (ObjectDefs_First / ObjectDefs_Next on 0x7890a0) to the
// first whose name (+0x390) equals the WString at 0x7cf9c0 ("End"; wcsicmp with its data, [0x7cf9c8]). Found: a block of 0x450 bytes
// (Mem_Alloc) constructed by 0x4db090 (cell 1, the definition's id (its word +0), angle 0, no ride), the made object's handle
// (Thing_GetHandle; a failed allocation reads address 0 (sic)) to the word 0x7cf4f4; none: the empty Debug_Printf (0x5da3c0, with a
// copy of the string), 0x7cf4f4 cleared. Either way 0x481b60 on 0x7b51f0 then gets the object (0 when there is none).
// (The original passes the cell and the id with stack bytes in their high halves; 0x4db090 reads their low words only.)
void __cdecl EndThing_Create();
}
