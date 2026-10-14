// The software mouse cursor (0x5fad80-0x5fb8dd, thiscall unless noted; vtable 0x703640): a
// function-static singleton at 0xfa7a80 that blits a cursor image onto a target surface and puts
// back what was under it.
//
// It keeps three SurfaceBuffers: 0 holds the image, 1 and 2 take turns holding what lies under the
// cursor, so a flipping screen can still restore the frame before. Drawing at a new position
// saves what is under the new place into the next buffer (taking the part the old cursor covers
// from the old buffer instead), puts the old background back (first painting the new cursor's
// overlapping part into it), paints the image and swaps buffers.
//
// Surface_GetInfo (0x5fad40, cdecl) sits just before it in the original and is kept here.
//
// The originals' quirks are kept: SetImage lays out the buffers' rectangles with the old image
// size before taking the new one; the buffers grow but never shrink, and growing first erases
// the cursor through a Draw at its top left with no target; the size and hotspot compares are
// unsigned; every blit clears the busy byte (0x775874), which nothing reads, while only Draw sets
// it; Draw ignores EraseShown's and Paint's results.
#pragma once

#include <cstddef>

#include "../../../re/types.h"
#include "../math/rect.h"
#include "surface.h"
#include "surfbuf.h"

struct Cursor {             // 0xfa7a80
    u32 vtable;             // 0x703640: the deleting destructor only
    u8 unknown04;
    u8 created;             // +5: the buffers exist
    u8 key[5];              // +6: keyRgb (0xff000000 at first) and keyIndex, given to each buffer
    u8 pad0b;
    u32 maxWidth;           // +0xc: the buffers' size
    u32 maxHeight;
    u32 width;              // +0x14: the image's size
    u32 height;
    s32 next;               // +0x1c: the buffer to save into next (1 or 2; 0: none)
    s32 shown;              // +0x20: the buffer holding what the drawn cursor covers (0: not drawn)
    s32 parked;             // +0x24: a buffer Update set aside for Restore
    s32 overlap;            // +0x28: the new place overlaps the shown one
    s32 dirty;              // +0x2c: the image changed; draw even when it has not moved
    Rect clip;              // +0x30: the target's bounds
    s32 hotX;               // +0x40: minus the hotspot
    s32 hotY;
    s32 x;                  // +0x48: the last position (0x7fffffff at first)
    s32 y;
    SurfaceBuffer buffers[3];  // +0x50
    Rect place[3];          // +0x164: each buffer's picture within it (at 0, 0)
    Rect source[3];         // +0x194: the part of each buffer that is on the target
    Rect screen[3];         // +0x1c4: where on the target each buffer's part goes
    Rect overlapRect[3];    // +0x1f4: the old and new places' overlap, in each buffer
    Surface *target[3];     // +0x224: the surface each was drawn on
};
static_assert(offsetof(Cursor, clip) == 0x30, "Cursor::clip is at +0x30");
static_assert(offsetof(Cursor, buffers) == 0x50, "Cursor::buffers is at +0x50");
static_assert(offsetof(Cursor, place) == 0x164, "Cursor::place is at +0x164");
static_assert(sizeof(Cursor) == 0x230, "Cursor is 0x230 bytes");

struct CursorEnv {
    Cursor *instance;               // 0xfa7a80
    u8 *guard;                      // 0xfa7a78: bit 0, constructed
    u8 *busy;                       // 0x775874
    u32 vtable;                     // 0x703640
    s32(__cdecl *atExit)(void(__cdecl *)());  // 0x67a7d0
    void(__cdecl *onExit)();        // 0x5fadc0, what Get registers
    void(__cdecl *free)(void *);    // 0x67acb0
};
// nullptr restores the game's.
void Cursor_SetEnv(const CursorEnv *e);
const CursorEnv *Cursor_GetEnv();

extern "C" {
SurfaceInfo *__cdecl Surface_GetInfo(SurfaceInfo *out, Surface *s);  // 0x5fad40: Refresh, copied
Cursor *__cdecl Cursor_Get();      // 0x5fad80: made on first use
void __cdecl Cursor_AtExit();      // 0x5fadc0
Cursor *__fastcall Cursor_Ctor(Cursor *c);                                   // 0x5fadd0
Cursor *__fastcall Cursor_DeletingDtor(Cursor *c, void *edx, u32 flags);     // 0x5faeb0
void __fastcall Cursor_Dtor(Cursor *c);                                      // 0x5faed0
// 0x5faef0 (&{x, y}, target): moves and redraws, unless not moved and not dirty since shown.
void __fastcall Cursor_Draw(Cursor *c, void *edx, const s32 *pos, Surface *target);
void __fastcall Cursor_Discard(Cursor *c);  // 0x5faf70: forgets the shown one; releases the buffers
s32 *__fastcall Cursor_CreateBuffers(Cursor *c, void *edx, s32 *out);  // 0x5faf80: maxWidth x maxHeight
void __fastcall Cursor_Flip(Cursor *c);            // 0x5fb020: shown = next; next = the other one
void __fastcall Cursor_ReleaseBuffers(Cursor *c);  // 0x5fb060
// 0x5fb090 (&out, image, &rect, &{x, y} hotspot): copies the rect of image into buffer 0; no
// image clears it.
s32 *__fastcall Cursor_SetImage(Cursor *c, void *edx, s32 *out, Surface *image, const Rect *rect, const s32 *hot);
u8 __fastcall Cursor_SaveUnder(Cursor *c);   // 0x5fb290: 0 when a blit fails (and next = 0)
u8 __fastcall Cursor_EraseShown(Cursor *c);  // 0x5fb400
u8 __fastcall Cursor_Paint(Cursor *c);       // 0x5fb4d0
void __fastcall Cursor_Move(Cursor *c, void *edx, const s32 *pos);   // 0x5fb520: the rectangles
s32 *__fastcall Cursor_ClearImage(Cursor *c, void *edx, s32 *out);   // 0x5fb790: size 0; 0
void __fastcall Cursor_SetKey(Cursor *c, void *edx, const u8 *key);  // 0x5fb7b0: keyRgb, keyIndex
// 0x5fb7f0 (&pos, unused, target, park, also): parks shown when both are set, then draws afresh.
void __fastcall Cursor_Update(Cursor *c, void *edx, const s32 *pos, u32 unused, Surface *target, u8 park,
                              u8 also);
// 0x5fb820 (target, dst, erase, parked): with erase, puts back the shown (or parked) buffer's
// background onto dst; the shown one's target becomes target; unparks.
void __fastcall Cursor_Restore(Cursor *c, void *edx, Surface *target, Surface *dst, u8 erase, u8 useParked);
}
