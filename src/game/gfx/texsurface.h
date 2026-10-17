// The texture cache's surface classes: the virtual methods behind the adjustor thunks at 0x58c2b0
// and 0x58c900-0x58c910, reimplementation of 0x58b3e0, 0x58b790, 0x58c2c0, 0x58c2e0, 0x58c330, 0x58c610, 0x58c710, 0x58c8a0 and
// 0x58c8b0.
//
// Each thunk subtracts [ECX - 4] and jumps here, so ECX is a sub-object of the surface (the
// cache's surfaces derive virtually from the base at 0x702198, see texcache.h). The class at
// 0x58c2c0 keeps its vbtable pointer at ECX - 0x3c, and the one at 0x58c8a0 at ECX - 0x10; the
// vbtable's second dword (+4) is the offset from ECX to the virtual base's fields.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x58b3e0 (thiscall; mostDerived): the constructor of class A. When most derived: vtable 0x702218
// and the virtual base (+0x3c) built by TexCacheItem_Ctor. Then, through the vtable's second
// dword (the base's offset, re-read each time as the writes can reach the vtable pointer): the
// base's vtable = 0x7021f0, the dword before it = offset - 0x3c, its +8 and +0xc zeroed; and
// +4-+0x24, +0x28-+0x34 zeroed. Returns the object.
u8 *__fastcall TexSurfaceA_Ctor(u8 *surface, void *, u32 mostDerived);

// 0x58c610 (thiscall; mostDerived): the same for class B: vtable 0x702278, the base at +0x10
// with vtable 0x702250, the dword before it = offset - 0x10, its +8 and +0xc zeroed, +4 and +8
// of the object zeroed. Returns the object.
u8 *__fastcall TexSurfaceB_Ctor(u8 *surface, void *, u32 mostDerived);

// 0x58c2c0 (thiscall on the sub-object): the dword at ECX - 0x38.
u32 __fastcall TexSurfaceA_Field38(const u8 *sub);

// 0x58c2e0: the virtual base's two dwords (at ECX + off - 0x34 and - 0x30, off = vbtable[1] of
// the vbtable at ECX - 0x3c) multiplied: an area; 32 bits.
u32 __fastcall TexSurfaceA_Area(const u8 *sub);

// 0x58c330: the dword at ECX - 0x10.
u32 __fastcall TexSurfaceA_Field10(const u8 *sub);

// 0x58c8a0 (the other class): the dword at ECX - 0xc.
u32 __fastcall TexSurfaceB_FieldC(const u8 *sub);

// 0x58c8b0: the same area from ECX + off - 8 and - 4, off = vbtable[1] of the vbtable at ECX - 0x10.
u32 __fastcall TexSurfaceB_Area(const u8 *sub);

// 0x58c8d0 (thiscall, this unused; name unconfirmed; behind the thunk 0x58c940): the float at 0x702248 (0.0) on ST0.
float __fastcall TexSurfaceB_Zero(const u8 *sub);

// 0x58c710 (thiscall on class B's sub-object; name unconfirmed; nothing refers to it): the pixels at ECX - 0xc, when set, are freed
// (TrackedGlobal_Free, core/gameheap) unless the virtual base's flags (ECX + off - 0xc, off = vbtable[1] of the vbtable at ECX - 0x10) have
// 0x10000000, and cleared. Returns 1.
u32 __fastcall TexSurfaceB_FreePixels(u8 *sub);

// 0x58c670 (thiscall on class B's sub-object; width, height, two unused; pops 0x10; name unconfirmed): with pixels already (ECX - 0xc)
// 0. Else a square of width a side from the texture pages (TexAtlas_Alloc into ECX - 0xc; gfx/texatlas) and 0x8c4898 = height * width
// * 4; none got: 0. Got: the virtual base's flags (ECX + off - 0xc) get 0x10000000 when it was cut from a page already there (so
// FreePixels leaves it), its +off - 8 = width, - 4 = height and +off = 0x100 (the offset re-read for each), ECX - 8 = the dword at
// 0x702220 for the display kind (0x8bd554). Returns 1.
u32 __fastcall TexSurfaceB_AllocPixels(u8 *sub, void *, u32 width, u32 height, u32, u32);

// 0x5738d0 (thiscall, this unused; name unconfirmed; behind the thunk 0x5738c0, slot 4 of the vtable 0x701630): a new class A surface (0x50 bytes,
// TexSurfaceA_Ctor most derived); its virtual base (the object + its vbtable's second dword), or null when the allocation fails.
u8 *__fastcall Renderer_NewSurfaceA(void *self);

// 0x573e70 (thiscall, this unused; name unconfirmed; behind the thunk 0x573f00): the same for class B (0x24 bytes, TexSurfaceB_Ctor).
u8 *__fastcall SoftRenderer_NewSurfaceB(void *self);

// 0x58b790 (thiscall on class A's sub-object; name unconfirmed; behind the thunk 0x58c2a0): for each index from count - 1 down to 0 (count the
// dword at ECX - 0x10, at least 1), the COM objects of the two arrays at ECX - 0x38 and ECX - 0x28 (that order) that are set are released
// (slot 2) and cleared; then the dword at ECX - 0x18 is cleared. Returns 1.
u32 __fastcall TexSurfaceA_ReleaseSurfaces(u8 *sub);
}
