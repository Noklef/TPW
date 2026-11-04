// The park map's window: the handlers of its two windows (names invented, unconfirmed), which pass what they do not take to
// Window_CallDefault. The map's fields as in parkmap.h; the mouse at +0x10 (x) and +0x14 (y), in a 0x800 x 0x600 space
// that the factors at 0x703014 and 0x703018 bring to the screen; the item under the mouse at +0x20060, the thing on the
// square under it at +0x20064; the list of items at +0x20080 (the nodes from +4: item, next; an item's position at +0xc
// and +0x10, shown at +0x2c, thing at +0x18); +0x20084 the right button down.
#pragma once

#include "../../../re/types.h"

struct SurfaceInfo;

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
    void(__fastcall *layOut)(const u8 *map);                                 // ParkMap_LayOut
    u32(__fastcall *removeImage)(u8 *array, void *, u32 image);              // PtrArray_RemoveImage
    u32(__fastcall *release)(u8 *array);                                     // Obj_ReleaseFromRenderer
    u32(__fastcall *releaseAll)(u8 *array);                                  // PtrArray_ReleaseAll
    void(__fastcall *surfaceFree)(SurfaceInfo *info);                        // SurfaceInfo_Free
    void(__fastcall *setFlags)(u8 *renderer, void *, u32 flags);             // Renderer_SetStateFlags
    void(__fastcall *restore)(u8 *owner);                                    // 0x409300
    void(__cdecl *restartTimers)();                                          // WindowTimers_RestartAll
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);               // Window_SendMessage
    void(__cdecl *cancelAdvisor)(u32 id);                                    // Game_CancelAdvisor
    void(__cdecl *focusSet)(u8 *window);                                     // Window7c24c8_Set
    void(__cdecl *focusRestore)();                                           // Focus_Restore
    void(__cdecl *helpPlace)(s32 x, s32 y);                                  // HelpBg_Place
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
// 0x5f1130 (thiscall on the map; window, message, a, b; pops 0x10):
// - 0x14: the map closed: the list at +0x20080 and its objects deleted, the six images at +0x20068 removed from the
//   array at 0x879ce0, its bitmap freed, [0x8bd47c] and the renderer's flags given back from +8 and +0xc, the game's windows
//   put back.
// - 0x15: ParkMap_LayOut.
// - 0x1e: with [0x78d910] clear, ParkMap_Scroll by how far the mouse is into the screen's edge.
// - 0x100 (a a command): -1 ParkMap_ApplyView, returns 0; 0x985, 0x986 the zoom 0.5 less or more, within 1 and 3; 0x982,
//   0x983, 0x987, 0x989, 0x993, 0x994 a bit of the filter at +0x18 flipped, and the picture reloaded as for the mouse.
// - 0x101 (a a button, b 1 pressed): +0x1c or +0x20 set or cleared, the picture reloaded.
// - 0x10001, 0x10003: the mouse kept. 0x10008, 0x10009 (a a button), 0x1000a (a a key): a step of scroll or zoom.
// - 0x1000b: Escape (a 0x1b, no bits 0xff0000 in b) closes the window (message 4); returns 0.
s32 __fastcall ParkMap_HandleWindow(u8 *map, void *, u8 *window, u32 message, u32 a, u32 b);
}
