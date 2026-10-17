// The action recorder (the global at 0x785a08, "AR - %d ..." in its debug prints): reimplementation of its
// constructor, destructor, Start, Restart (0x403480-0x403b00), record writers 0x403be0-0x403e60 and the recorded actions 0x447e10,
// 0x52f1b0, 0x52f880, 0x52fa90 and 0x52fad0.
//
// +0 recording (writers do nothing while 0), +4 the mode, +0xc / +0x14 / +0x1c three streams (StreamBase, vtable
// 0x6fd6a0), +0x24 playing back (writers do nothing while set). A writer appends one record to the first stream
// (its vtable +0xc, the write: buffer, count): an action code byte, then its operands, bytes and 16-bit words, each
// with a write of its own. The debug print that follows each record is compiled out of the retail game.
#pragma once

#include "../../../re/types.h"

struct ActionRec {
    u32 recording;
    u32 mode;
    u32 unknown8;
    void *stream[3][2];  // StreamBase objects (vtable, source)
    u32 playback;
};
static_assert(sizeof(ActionRec) == 0x28, "the writers' fields end at +0x28");

extern "C" {
// 0x403480 (thiscall): the constructor. The streams constructed with the vtable 0x6fd6a0, playback 0, a table of
// settings from +0x34 to +0x148 and +0x3d0, then started as Start(0) would (mode 0, all three streams opened);
// +8 is cleared last. Returns the recorder.
ActionRec *__fastcall ActionRec_Ctor(ActionRec *r);
// 0x4036e0 (thiscall): the destructor: each stream closed (vtable +4; only the first in a mode other than 0), then
// destroyed, the last first.
void __fastcall ActionRec_Dtor(ActionRec *r);
// 0x403a40 (thiscall; mode, pops 4): recording in this mode: the first stream opened as a memory stream of 0x200
// (the others too in mode 0), then the start record, the code 0xe7 and 0x80 zero bytes.
void __fastcall ActionRec_Start(ActionRec *r, void *, u32 mode);
// 0x403b00 (thiscall): recording stopped, its streams closed as the destructor does, then started again in its mode.
void __fastcall ActionRec_Restart(ActionRec *r);
// 0x403be0 (thiscall; code, pops 4): the code byte.
void __fastcall ActionRec_Write(ActionRec *r, void *, u32 code);
// 0x403c20 (thiscall; code, a, b; pops 0xc): three bytes.
void __fastcall ActionRec_WriteBB(ActionRec *r, void *, u32 code, u32 a, u32 b);
// 0x403c80 (thiscall; code, a, b, c; pops 0x10): three bytes (one write), then the word c.
void __fastcall ActionRec_WriteBBW(ActionRec *r, void *, u32 code, u32 a, u32 b, u32 c);
// 0x403d00 (thiscall; code, a; pops 8): the code byte, the word a.
void __fastcall ActionRec_WriteW(ActionRec *r, void *, u32 code, u32 a);
// 0x403d60 (thiscall; code, a, b; pops 0xc): the code byte, the words a and b.
void __fastcall ActionRec_WriteWW(ActionRec *r, void *, u32 code, u32 a, u32 b);
// 0x403dd0 (thiscall; code, a, b, c; pops 0x10): the code byte, the words a, b and c.
void __fastcall ActionRec_WriteWWW(ActionRec *r, void *, u32 code, u32 a, u32 b, u32 c);
// 0x403e60 (thiscall; code, a, b; pops 0xc): the code byte, the word a, the byte b.
void __fastcall ActionRec_WriteWB(ActionRec *r, void *, u32 code, u32 a, u32 b);

// Recorded actions: each writes its record to the global recorder 0x785a08, then (most) applies it.
// 0x447e10 (cdecl; coaster, value): the record 0x45 (the coaster's word +0x14, the word value); nothing else.
void __cdecl ActionRec_RecordCoaster45(const u8 *coaster, u32 value);
// 0x52fa90 / 0x52fad0 (cdecl; value): the record 0x23 / 0x1e (the word value), then the dword 0x81b0cc / 0x81ecac = value.
void __cdecl Global81b0cc_SetRecorded(u32 value);
void __cdecl Global81ecac_SetRecorded(u32 value);
// 0x52f1b0 (cdecl; value, force byte): the record 0xa (the word value) unless the value is the dword 0x81b134 and force
// is 0; then 0x81b134 = the old 0x81d7a4 and 0x81d7a4 = value.
void __cdecl Global81d7a4_SetRecorded(u32 value, u32 force);
// 0x52f880 (cdecl; &id, flag): the record 5 (the word id, the byte flag), the ride marked built in the world (0x7cf83c,
// World_MarkRideBuilt); then the word 0x8186ec = the word 0x8186e0, the dword 0x8187f8 = flag and 0x8186e0 = id.
void __cdecl Global8186e0_SetRecorded(const u16 *id, u32 flag);
}
