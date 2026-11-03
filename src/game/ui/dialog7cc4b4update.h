// The dialog [0x7cc4b4] brought up to date with the park's theme and the save's levels. Reimplementation of 0x4b9340; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../core/string.h"

// The calls (the ports by default; 0x5b09c0 unported, by address; the bench's recorders: their own benches have them).
struct Dialog7cc4b4UpdateEnv {
    s32(__cdecl *atExit)(void(__cdecl *fn)());                                  // crt_atexit, raw
    u8 *(__fastcall *construct)(u8 *info);                                      // GlobalSave_Construct
    u8 *(__fastcall *findTheme)(u8 *info, void *, const String *name);          // 0x5b09c0
    u8 *(__cdecl *instance)();                                                  // GlobalSave_Instance
    s32(__fastcall *countLevels)(const u8 *info);                               // GlobalSave_CountSetLevel
    u32(__fastcall *field14)(u8 *theme);                                        // ThemeRecord_GlobalSamField14
    u32(__fastcall *setShown)(u8 *window, void *, u32 shown);                // Window_SetShown
    void(__fastcall *notify1)(u8 *window, void *, u32 value);                   // Window_Notify1
    u16(__fastcall *getRect)(u8 *window, void *, u8 *rect, u32 a);           // Window_GetRect
    s32(__cdecl *spawn)(s32 effect, s32 x, s32 y, s32 z);                       // Particle_Spawn
    void(__cdecl *end)(u32 handle);                                             // ParticleSystem_End
    s32(__cdecl *swprintf)(u16 *to, const u16 *format, s32 value);              // Crt().swprintf
    void(__cdecl *setFlag)(u32 flag);                                           // Dialog7cc4b4_SetFlag
    void(__fastcall *copyName)(u8 *theme, void *, u16 *to, u32 size);           // 0x5b12e0 (Dialog7cc4b4_ShowTheme's), agent B's, unported
};
void Dialog7cc4b4_SetUpdateEnv(const Dialog7cc4b4UpdateEnv *env);

extern "C" {

// 0x4b9340 (cdecl): nothing without the dialog. With a game [0xf85614], its +4 and that one's +0xc: the theme of the name [+0x30] (String_CtorStr, 0x5b09c0 on the save info [0xf7c58c], made the first time). Found, the child 0x1e0ea: in mode 2 (the setting 0xfb3b7c) hidden, and 0x1e0f0 and 0x1e0f1 told slot 5 (0), 0x1e0ea
// and 0x1e0ec hidden; else all of those shown or told 1 and, with the set levels at least the theme's field 0x14, 0x1e0ea's Window_Notify1 (0) and its child 0x1e0eb's (field - 1), its rectangle's middle (y * 75000 / 0x600) a spawn of the effect 0x61 at (50000, 0, y) unless [0x7cc4bc] has one; below it Window_Notify1 (1) and (field + 4), the effect ended and forgotten.
// Then the child 0x1e0ec: in mode 2 hidden and Dialog7cc4b4_SetFlag (0). Else with no set levels hidden; with some shown, the four again told 1, and its child 0x1e0ee's slot 9 the count formatted (0x758194, the buffer 0x7ba058). Last [0x7cc4b8] = the set levels.
void __cdecl Dialog7cc4b4_Update();

// 0x4b90f0 (cdecl; name invented, unconfirmed): nothing without the dialog. The theme of the name [+0x30] of the game's part [[[0xf85614] + 4] + 0xc] (none checked; 0x5b09c0). Found: in mode 2 the child 0x1e0ea hidden, else its figures as Update's by whether [0x7cc4b8] is at least the theme's field 0x14 (signed);
// its name copied (0x5b12e0) to 0x7ba058, 0x3fff, which is the text; not found the part's +0x50 is. The child 0x1e0ef's slot 9 the text.
void __cdecl Dialog7cc4b4_ShowTheme();
}
