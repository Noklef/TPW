// The park map's window: the handlers of its two windows (names invented, unconfirmed), which pass what they do not take to
// Window_CallDefault. The map's fields as in parkmap.h; the mouse at +0x10 (x) and +0x14 (y), in a 0x800 x 0x600 space
// that the factors at 0x703014 and 0x703018 bring to the screen; the item under the mouse at +0x20060, the thing on the
// square under it at +0x20064; the list of items at +0x20080 (the nodes from +4: item, next; an item's position at +0xc
// and +0x10, shown at +0x2c, thing at +0x18); +0x20084 the right button down.
#pragma once

#include "../../../re/types.h"

struct ParkMapWindowEnv {
    void(__fastcall *scroll)(u8 *map, void *, u32 direction, float amount);  // ParkMap_Scroll
    void(__fastcall *loadPicture)(u8 *map, void *, u32 keep);                // ParkMap_LoadPicture
    void(__fastcall *draw)(const u8 *map);                                   // ParkMap_Draw
    void(__fastcall *uncacheRect)(u8 *backdrop, void *, s32 x0, s32 y0, s32 x1, s32 y1);  // Backdrop_UncacheRect
    u16 *(__fastcall *staffName)(u8 *staff);                                 // Staff_GetName
    s32(__cdecl *formatWide)(u8 *charset, u16 *dest, u32 size, u32 entry, ...);  // Charset_FormatWide
    void(__fastcall *signFormat)(const u8 *sign, void *, u16 *to, u32 count);    // SignText_Format
    u8 *(__fastcall *findChild)(u8 *window, void *, u32 id);                 // Window_FindChild
    s32(__fastcall *applyView)(u8 *map);                                     // ParkMap_ApplyView
    void(__cdecl *thingPanel)(u8 *thing);                                    // ThingPanel_Show
    s32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b);  // Window_CallDefault
};
void ParkMapWindow_SetEnv(const ParkMapWindowEnv *env);

extern "C" {
// 0x5f18d0 (thiscall on the map; window, message, a, b; pops 0x10):
// - 0x1e: with [0x78d910] set and the right button down, ParkMap_Scroll 1 and 3 by the mouse's distance from the middle.
// - 0x10001, 0x10003 (a: y | x << 16): the mouse kept, and the nearest shown item within 5 of it in x and in y kept at
//   +0x20060. A person's (kinds 4 to 8) coming or going reloads the picture, redraws and uncaches the whole of it. Then the
//   square under the mouse's thing at +0x20064, and the item's or the square's name in the window's parent's child 0x991.
// - 0x10004, 0x10005: a 1 clears, sets +0x20084.
// - 0x10006, a 0: the view moved by the mouse's point (x, y), ParkMap_ApplyView, ThingPanel_Show of the item's or the
//   square's thing; returns 0.
s32 __fastcall ParkMap_HandleMouse(u8 *map, void *, u8 *window, u32 message, u32 a, u32 b);
}
