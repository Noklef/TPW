// The action recorder (the global at 0x785a08, "AR - %d ..." in its debug prints): reimplementation of its
// record writers 0x403be0-0x403e60.
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
}
