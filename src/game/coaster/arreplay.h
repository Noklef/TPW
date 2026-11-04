// The coaster's action replay (the debug prints' "AR replay"): reimplementation of 0x4045b0, one recorded action read and done.
#pragma once

#include "../core/addrcalls.h"
#include "../../../re/types.h"

extern "C" {
// 0x4045b0 (thiscall, no arguments; name unconfirmed): the next action of the stream at this+0xc (slot 2: read (buffer, bytes), answering the bytes read). Answers 0 for any
// short read, else 1. A byte, the action; its kind this+0x34[action] (above 0xb, or an action not of the kind: nothing) says what follows:
// 0: bytes x, y; 0xf MapPress, 0x14 MapClick (x, y, 0, 0).          2: bytes x, y, word flags; 0x28 MapPress, 0x2d MapClick (x, y, flags, 0).
// 1: word; 0x404d80 (action, word).                                 3: word id, byte flag; 5 Global8186e0_SetRecorded (&id, flag).
// 4: dword, word; 0x39 Car_ApplyLengthAt, 0x3a Car_ApplyE0At.       5: dwords a, b, word; 0x3b Car_ApplyAnglesAt.
// 6: words a, b; 0x3f CoasterEdit_DeleteSpanBetween, 0x45 Coaster_SetMixAtPinned.     7: words a, b, c; 0x3c CoasterEdit_PickPiece.
// 8: nothing; 0x37 CoasterEdit_Tool1, 0x38 CoasterEdit_Tool2, 0x40 0x447d30, 0x41 0x447d90.
// 9: byte n, n words dropped ("no longer used"). 10: word w, byte n, n words; 0x404f70 (action, w, n, words). 11: 0x80 bytes dropped.
// sic: n words go into a 0x80-byte buffer on the stack, so n above 0x40 runs over the frame.
u32 __fastcall ArReplay_Step(u8 *self);

// 0x4041d0 (thiscall, no arguments; name unconfirmed): nothing unless this+0x24. 0x527940 (0) first. Mode (this+0x28) 0: every action stepped, then finished: the streams at +0xc and, when +4 is 0, +0x14 and +0x1c closed (slot 4 (0, 2)), +0x24 = 0, +8 = 1,
// eight frames of 0x5516b0, 0x439390, and 0x5195d0's object (of 0x7cf83c) +0x114 = 1. Modes 1 and 2 (others: nothing): when the wait (+0x30) is 0, stepped until it is not, or finished; then the wait counted down, and in mode 1, having stepped, cut to 2 (as unsigned).
void __fastcall ArReplay_Update(u8 *self);

// 0x404140 (thiscall (mode), pops 4; name invented, unconfirmed): nothing if +0x24 is set. Else +0x24 = 1, +0x28 = the mode, 0x5195d0's object (of 0x7cf83c) +0x114 = 0. With +4 clear and mode 1 or 2 the streams at +0x14 and +0x1c started (slot 4 (0, 0)); the stream at +0xc too; then ArReplay_Update.
void __fastcall ArReplay_Begin(u8 *self, void *, u32 mode);
}

// Its calls by the game's addresses; a table of replacements (tests/fncmp/arreplayupdate.py).
void ArReplay_SetCalls(const AddrCall *table, u32 count);

// What an action does (the bench's recorders; null: the game's).
struct ArReplayEnv {
    void(__cdecl *tool1)();
    void(__cdecl *tool2)();
    void(__cdecl *f447d30)();                                       // by address
    void(__cdecl *f447d90)();                                       // by address
    void(__cdecl *mapPress)(s32 x, s32 y, u32 flags, u32 stored);
    void(__cdecl *mapClick)(s32 x, s32 y, u32 flags, u32 stored);
    void(__fastcall *f404d80)(u8 *self, void *, u32 action, u32 word);                          // by address
    void(__fastcall *f404f70)(u8 *self, void *, u32 action, u32 word, u32 n, const u16 *words);  // by address
    void(__cdecl *setRecorded)(const u16 *id, u32 flag);
    void(__cdecl *applyLength)(u32 value, u32 index);
    void(__cdecl *applyE0)(u32 value, u32 index);
    void(__cdecl *applyAngles)(u32 a, u32 b, u32 index);
    void(__cdecl *deleteSpan)(u32 first, u32 second);
    void(__cdecl *setMix)(u32 index, u32 value);
    void(__cdecl *pickPiece)(u32 unused1, u32 unused2, u32 number);
};
void ArReplay_SetEnv(const ArReplayEnv *env);
