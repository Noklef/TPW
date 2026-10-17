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
