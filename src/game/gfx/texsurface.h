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
#include "../core/addrcalls.h"

void TexSurface_SetCalls(const AddrCall *table, u32 count);

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

// 0x58c960 (thiscall on class B's sub-object at +0x10; flags; pops 4; behind the thunk 0x58c950, slot 9 of the base's vtable
// 0x702250): the scalar deleting destructor. On the object (ECX - 0x10), through the vbtable's offset (re-read each time): the base's
// vtable = 0x702250, the dword before it = offset - 0x10; then the base's slot 1 (FreePixels' thunk 0x58c8f0) on it; +0x10's vtable =
// 0x702198 (TexCacheItem's); with flags bit 0 the object freed (Mem_Free). Returns the object.
u8 *__fastcall TexSurfaceB_DeletingDtor(u8 *sub, void *, u32 flags);

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

// 0x58b470 (thiscall on a sub-object 0x3c above the object, behind the thunk 0x58c353; x87; name invented, unconfirmed): the virtual base (its offset at
// [+-0x3c]+4, read again each time) has its slots 0x10 and 0x14 called in turn 16 times between two readings of the clock (0x57cb90 on 0x8bba80).
// Returns the area ([base+0xc] * [base+8] * 16) over the ticks * 16, in integers.
double __fastcall TexSurface_LockRate(u8 *sub);

// 0x58b4f0 (thiscall on class A's sub-object; pops 0x10; name invented, unconfirmed): the surfaces of a texture: a surface description of the pixel
// format's record (0x573820; none: the error 0xb0754003 on the engine's list, 0), `levels` mip levels (0 is 1) and caps 0x10405008 goes to the device's
// CreateSurface (slot 0x18), with the caps' bit 0x10000000 changed to 0x20000000 if it fails (it failing again: its result on the list, 0), the video
// memory used noted at 0x8c4898. Then the format's bits, the sizes in the virtual base (the vbtable read for each), the format and levels stored; each
// level's attached surface (slot 0x30) goes into the array at ECX - 0x28, and each level's interface (slot 0, IID 0x7075d0; last level first) into the
// array at ECX - 0x38. A result below 0 goes to the debug log; any non-zero one to the list, 0. Returns 1.
u32 __fastcall TexSurfaceA_Create(u8 *sub, void *, u32 width, u32 height, u32 format, u32 levels);

// 0x58b790 (thiscall on class A's sub-object; name unconfirmed; behind the thunk 0x58c2a0): for each index from count - 1 down to 0 (count the
// dword at ECX - 0x10, at least 1), the COM objects of the two arrays at ECX - 0x38 and ECX - 0x28 (that order) that are set are released
// (slot 2) and cleared; then the dword at ECX - 0x18 is cleared. Returns 1.
u32 __fastcall TexSurfaceA_ReleaseSurfaces(u8 *sub);

// 0x58c750 (thiscall on class B's sub-object; texture, x, y; pops 0xc; name unconfirmed; behind the thunk 0x58c930): the texture (gfx/texture)
// drawn scaled into the pixels at ECX - 0xc (dwords, the pitch in pixels at ECX + off, off = vbtable[1] of the vbtable at ECX - 0x10) at (x, y),
// its s16 +0x14 x +0x16 the size drawn. With texture flag 0x10000000 its object at +0x40 is asked first (virtual call 1: a stack record of
// the destination, the width, the height and the pitch in bytes, and the flag 0x40000): not 0, done. Else from Texture_Lock's pixels
// (the s16 pitch +0x34 in pixels) of size +0x10 x +0x12, 16.16 steps (source size << 16 over the size drawn: one IDIV each, faulting, sic,
// on a size of 0); flag 0x40000 reads the rows from the last up. A height of 0 draws nothing. Returns 1.
u32 __fastcall TexSurfaceB_Blit(u8 *sub, void *, u8 *texture, s32 x, s32 y);

// 0x58bd10 (thiscall on class A's sub-object; texture, x, y; pops 0xc; name unconfirmed; behind the thunk 0x58c340): the texture drawn
// scaled into the frame at 0x8c48cc (pitch 0x8c48b8 in bytes, the pixel size at ECX - 0x14) at (x, y), into the s16 +0x14 x +0x16
// box. A box wider than the source (+0x10) is cut to it, centred (the margin's left half rounded down), and taller than it (+0x12)
// likewise; each cut sets the texture's flag 0x80000000 and zeroes its margins' bytes (rows from the last up: left, then right, then
// top, then bottom). With texture flag 0x10000000 its object at +0x40 is asked first (virtual call 1: a stack record of the
// destination, the width, the height and the pitch, and the flag 0x40000): not 0, done. Else as TexSurfaceB_Blit (the IDIVs first:
// faulting, sic, on a size of 0), its rows from the s16 pitch +0x34 in pixels, 2-byte pixels unless the pixel size is 4, each copied
// or, with texture flag 0x400 and a shade map at +0x3c (bytes, the same pitch), multiplied per channel by the dword the map's byte picks
// from 0x879e38 (4-bit channels below the top one for 2-byte pixels, bytes below the top one for 4-byte), >> 8. Returns 1.
u32 __fastcall TexSurfaceA_Blit(u8 *sub, void *, u8 *texture, s32 x, s32 y);

// 0x58baf0 (thiscall on class A's sub-object; name invented, unconfirmed): each mip level past the first: a surface of half the side from the pool (0x8c47d8, 0x58b1f0), locked (slot 0x19), the level above halved into it (0x58b890, its last description 0x8c48a8), the one held at ECX - 0x18 unlocked and loaded into the level's array entry (ECX - 0x28, slot 5), the new one then held; the last the same way. A failure goes to the list (kind 2), the surfaces in hand unlocked, 0. Returns 1.
u32 __fastcall TexSurfaceA_BuildMips(u8 *sub);
// 0x58b810 (thiscall on class A's sub-object; name invented, unconfirmed): the shared description 0x8c48a8 cleared (0x7c bytes) and sized; SurfacePool_Next (the virtual base's -0x34, -0xc) kept at -0x18 and locked
// (slot 0x64 (0, the description, 1, 0)); 1 when that gave 0.
u32 __fastcall TexSurfaceA_LockNext(u8 *sub);
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(const float, gTexSurface_Zero_702248, 0x00702248, 0x4);

// The vtables hold only ports: tpw.exe's loader points their slots at them (.docs/09-globals.md).
GAME_DATA(void *[10], Class702250_vtable, 0x00702250, 0x28);
