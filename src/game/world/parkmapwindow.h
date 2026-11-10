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

// ParkMap_PrepareSprites's calls.
struct ParkMapSpritesEnv {
    void(__fastcall *firstThing)(u8 *world, void *, u16 *out);                  // World_FirstThing
    u16 *(__fastcall *nextThing)(u8 *world, void *, u16 *out, u8 *thing);       // World_NextThing
    void(__fastcall *arrayInit)(u8 *array, void *, u32 frames, u32 flags);      // PtrArray_Init
    void(__fastcall *addToRenderer)(u8 *array);                                 // Obj_AddToRenderer
    u32(__fastcall *addImage)(u8 *array, void *, u32 image);                    // 0x56b380
    void(__fastcall *addSprites)(u8 *map);                                      // ParkMap_AddSprites
};
void ParkMapSprites_SetEnv(const ParkMapSpritesEnv *env);

// ParkMap_Open's calls, and the two handlers'.
struct ParkMapOpenEnv {
    u32(__cdecl *hasSource)();                                                  // Global7c2534_HasSource
    void(__fastcall *pause)(u8 *self, void *, s32 mode, u32 flag);              // GameOwner_Pause
    void(__cdecl *setFlag662411)();                                             // Fn_662411
    void(__cdecl *f5996d0)(u32 value);                                          // 0x5996d0 (agent B's)
    u32(__fastcall *stateFlags)(const u8 *renderer);                            // Renderer_StateFlags
    void(__fastcall *setFlags)(u8 *renderer, void *, u32 flags);                // Renderer_SetStateFlags
    void(__fastcall *releaseItems)(u8 *cache, void *, u32 mode);                // TexCache_ReleaseItems
    u32(__cdecl *focusClose)();                                                 // Window7c24c8_Close
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);                  // Window_SendMessage
    u8 *(__fastcall *build)(u8 *window, void *, u8 *script, u32 handler);       // WindowScript_Build
    u32(__cdecl *focusSet)(u8 *window);                                         // Window7c24c8_Set
    u32(__fastcall *setFocus)(u8 *window, void *, u32 a);                       // Window_SetFocus
    u8 *(__fastcall *findChild)(u8 *window, void *, u32 id);                    // Window_FindChild
    u8 *(__fastcall *textPart)(u8 *part, void *, u32 value);                    // TextPart_Ctor
    void(__fastcall *setImpl)(u8 *window, void *, u8 *impl);                    // Window_SetImpl
    void(__fastcall *notifyC00)(u8 *window, void *, u32 a, u32 b);              // Window_NotifyC00
    void(__fastcall *notify3000)(u8 *window, void *, u32 a, u32 b);             // Window_Notify3000
    u8 *(__cdecl *font)(u32 index);                                             // Language_Font
    void(__fastcall *set4c)(u8 *window, void *, u8 *object);                    // Window_Set4c
    void(__fastcall *notify40000)(u8 *window, void *, u32 a, u32 b, u32 c, u32 d);  // Window_Notify40000
    void(__fastcall *notifyImpl100)(u8 *window, void *, u32 value);             // Window_NotifyImpl100
    void(__fastcall *setDepth)(u8 *window, void *, u32 depth);                  // Window_SetDepth
    u16(__fastcall *setState)(u8 *window, void *, u8 state, u8 notify);         // Window705a50_SetState
    u16(__fastcall *select)(u8 *group, void *, u32 id);                         // RadioGroup_Select
    u32(__fastcall *setPair)(u8 *window, void *, u32 a, u32 b);                 // Window705a50_SetPair
    u16(__fastcall *setHandler)(u8 *window, void *, u32 handler);               // Window_SetHandler
    void(__cdecl *gridSize)(u32 *width, u32 *height);                           // Terrain_GridSize
    void(__fastcall *classify)(u8 *map);                                        // ParkMap_Classify
    void(__fastcall *prepare)(u8 *map);                                         // ParkMap_PrepareSprites
    void(__fastcall *loadPicture)(u8 *map, void *, u32 keep);                   // ParkMap_LoadPicture
    u32(__fastcall *backdropCreate)(u8 *backdrop, void *, u8 *pixels, s32 width, s32 height, s32 pitch);  // Backdrop_Create
    void(__fastcall *backdropClear)(u8 *backdrop);                              // Backdrop_Clear5c
    void(__fastcall *draw)(const u8 *map);                                      // ParkMap_Draw
    void(__fastcall *uncacheRect)(u8 *backdrop, void *, s32 x0, s32 y0, s32 x1, s32 y1);  // Backdrop_UncacheRect
    u8(__cdecl *advisor)(u32 id);                                               // Advisor_PostId
    u32(__cdecl *helpPlace)(u32 x, u32 y);                                      // HelpBg_Place
    s32(__fastcall *handleWindow)(u8 *map, void *, u8 *window, u32 message, u32 a, u32 b);  // ParkMap_HandleWindow
    s32(__fastcall *handleMouse)(u8 *map, void *, u8 *window, u32 message, u32 a, u32 b);   // ParkMap_HandleMouse
};
void ParkMapOpen_SetEnv(const ParkMapOpenEnv *env);

extern "C" {
// 0x5f0b00 (cdecl; window, message, a, b): ParkMap_HandleWindow on the map at 0xf86db8; the handler of the map's window.
s32 __cdecl ParkMap_WindowHandler(u8 *window, u32 message, u32 a, u32 b);
// 0x5f0b20 (cdecl; window, message, a, b): ParkMap_HandleMouse on the map at 0xf86db8; the handler of its child 0x992.
s32 __cdecl ParkMap_MouseHandler(u8 *window, u32 message, u32 a, u32 b);
// 0x5f0b40 (thiscall on the map; the SEH frame left out): the park map opened, unless Global7c2534_HasSource or a window at +4. The game
// held (GameOwner_Pause on 0x786b68; [0x786ba4] 0; Fn_662411; 0x5996d0 (1)), [0x8bd47c] kept at +8 and cleared, the renderer's flags kept at +0xc
// and set to 0x2821, the texture cache released, the old window closed and the frame's sent 6. The script 0x774da0 built on [0xfaa5d8] with the handler
// 0x5f0b00 is the new window (+4, focused). Its child 0x991 gets a text part (none when the allocation fails) in white on no box, font 4;
// 0x990 and 0x995 are hidden (slot 5, 0); the depth of 0x990 + 2 goes to 0x982, 0x987, 0x989, 0x993, 0x994 and 0x983, each set when its bit of
// the filter +0x18 is (8, 1, 0x10, 2, 4, 0x20). The radio group 0x98b selects by +0x1c (1 to 4: 0x98e, 0x98f, 0x98c, 0x98d), 0x996 by +0x20 (1 to 3:
// 0x998, 0x997, 0x999). 0x98a, 0x984, 0x981, 0x988 get the pair (0, 10); 0x992 the handler 0x5f0b20. The mouse fields cleared, the grid's size at
// +0x24, +0x28; the view origin (+0x54, +0x58) from the camera ([0x790ab8], [0x790ac0] times [0x70300c], truncated, kept to 0 and the terrain's
// size [0x7a0314], [0x7a0374] less 1) kept to [0x703010] and the size. Then the squares classified, the sprites made ready, the picture loaded and
// cut into the backdrop 0x87aee8, loaded again and drawn on, the backdrop uncached, advisor message 0x130 and the help box at (-1, 10).
void __fastcall ParkMap_Open(u8 *map);
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
// 0x5f3710 (thiscall on the map): the sprites counted: 2 for each thing of kind 1, 6 or 7 and 1 of kind 4 or 5 in the world's
// walk; 1 for each square with a word at +0x2c0 of its 0x44-byte cell; 1 for each thing with flag 0x10 (+0x32) on the
// chain from the world's +0x1da746 (next at +0xc). The array at 0x879ce0 made for those and 6 more, and added to the renderer;
// the six images of the table at 0xf86d48 (0x14 bytes each) added and kept at +0x20068; a new list (vtable 0x703080) at
// +0x20080, or 0; then ParkMap_AddSprites.
void __fastcall ParkMap_PrepareSprites(u8 *map);
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(const float, gParkmapwindow_703018, 0x00703018, 0x4);
GAME_DATA(const float, gParkmapwindow_70301c, 0x0070301c, 0x4);
GAME_DATA(const float, gParkmapwindow_703020, 0x00703020, 0x4);
GAME_DATA(const float, gParkmapwindow_703024, 0x00703024, 0x4);
GAME_DATA(const float, gParkmapwindow_70302c, 0x0070302c, 0x4);
GAME_DATA(const float, gParkMap_HandleMouse_703034, 0x00703034, 0x4);
GAME_DATA(const float, gParkMap_HandleMouse_703038, 0x00703038, 0x4);
GAME_DATA(const float, gParkmapwindow_703040, 0x00703040, 0x4);

// The vtables hold only ports: tpw.exe's loader points their slots at them (.docs/09-globals.md).
GAME_DATA(void *[1], Class703080_vtable, 0x00703080, 0x4);

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(const float, gParkMap_Open_70300c, 0x0070300c, 0x4);
GAME_DATA(const float, gParkmapwindow_703010, 0x00703010, 0x4);
GAME_DATA(const float, gParkmapwindow_703014, 0x00703014, 0x4);
