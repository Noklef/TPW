// The sprites of decoded textures (class 0x703088): reimplementation of the constructor 0x5f4370 and the shown flag's setter 0x5f4490 (its destructor 0x5f4400 is
// Class703088_ScalarDtor, core/vdel.h).
//
// Each owns a sprite frame (+0x28) appended to the pointer array at 0x879ce0 (PtrArray_AddSpriteFrame) and is kept in the global
// pointer list at [0xfa6e38]. Its fields +0x14, +0x1c, +0x20 and +0x24 are the constructor's arguments (their meaning unknown so far);
// +0x2c is a flag cleared at construction (while it is 0 the frame's flags have bit 0 set).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5f4370 (thiscall; image, a, b, c, d, pops 0x14; name unconfirmed): the vtable 0x703088, a new frame for the image (the object's +0x28;
// +0x2c = 0, so the frame's flags get bit 0 and 0x30000), +0x14 / +0x1c / +0x20 / +0x24 = a / b / c / d, and the object appended to the list
// at [0xfa6e38]. Returns the object. (A failed frame allocation faults, as in the original.)
u8 *__fastcall TexSprite_Ctor(u8 *object, void *, u32 image, u32 a, u32 b, u32 c, u32 d);

// 0x5f4490 (thiscall; value, pops 4): when the dword at +0x2c differs from the value it is set, and the object
// at +0x28 (through Fn_56b620, which returns it unchanged) has bit 0 of its dword at +0x10 cleared for a
// non-zero value, set for 0. Returns that object, or the value when nothing changed.
u32 __fastcall Obj_SetShown(u8 *object, void *, u32 value);
}
