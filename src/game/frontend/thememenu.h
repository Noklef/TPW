// The lobby's menus of themes: their texts from the theme records or the localised names, the fonts added for them removed.
#pragma once

#include "../core/string.h"
#include "../../../re/types.h"

// The callees (tests/fncmp/thememenu.py gives them recorders).
struct ThemeMenuEnv {
    u8 *(__fastcall *userSlots)(u8 *self);                                                     // UserSlots_Ctor
    u8 *(__fastcall *globalSave)(u8 *info);                                                    // GlobalSave_Construct
    u8 *(__fastcall *themeGet)(u8 *info, void *, const u8 *name);                              // GlobalSave_ThemeGet
    void(__fastcall *name)(u8 *self, void *, u16 *to, u32 count);                              // ThemeRecord_Name
    void(__fastcall *suffix)(u8 *self, void *, u16 *to, u32 count);                            // ThemeRecord_Suffix
    void(__fastcall *localise)(u8 *table, void *, WString *key, WString *a, WString *b);       // 0x409b10
    void(__fastcall *setTexts)(u8 *item, void *, const u16 *a, const u16 *b);                  // 0x5d8470
    s32 *(__cdecl *deleteFile)(s32 *result, const char *path);                                 // File_Delete
    u8 *(__fastcall *online)(u8 *session);                                                     // Online_Construct
    u32(__fastcall *valueByKey)(u8 *owner, void *, u32 key);                                   // Owner2c_ValueByKey
    void(__fastcall *load)(u8 *self, void *, const char *name, u32 item, s32 index, u32 a, u32 b);   // LobbyScript_Load
    void(__fastcall *holder)(u8 *user);                                                        // HolderUser_Acquire
    void(__fastcall *ambient)(void *view, void *, u32 on, u32 colour);                         // Render_SetAmbient
    void(__fastcall *light)(u8 *slot, void *, u32 on, const u32 *colour, const u32 *direction, u32 value);   // LightSlot_Set
    u8 *(__cdecl *instance)();                                                                 // Online_Instance
    void(__fastcall *reset)(u8 *session);                                                      // Online_ResetConnections
    void(__cdecl *dialogOpen)();                                                               // Dialog7cc4b4_Open
    void(__cdecl *dialogShow)();                                                               // Dialog7cc4b4_ShowTheme
    u32(__cdecl *time)();                                                                      // Time_GetMs
    void(__fastcall *refresh)(u8 *self);                                                       // ThemeMenu_Refresh
    u32(__cdecl *play)(u32 owner, u32 category, u32 sound, s32 x, s32 y, s32 z);               // Sound_PlayAt
    void(__cdecl *applySound)();                                                               // Sound_ApplySettings
};
void ThemeMenu_SetEnv(const ThemeMenuEnv *env);  // nullptr: the game's

extern "C" {

// 0x5e2010 (thiscall; name invented, unconfirmed): for each item of the list at +8 (the sentinel's +4; nodes {item, next}) two wide strings: with no chosen slot (the user slots' +0x60 is -1)
// the table 0x786b68's for the wide form of the name at the item's +0x30, else the theme record's name and suffix. Both go to the item's objects +4 and +8. Then the fonts of 0x789c1c removed
// (RemoveFontResource, File_Delete) and destroyed, the vector's end at its begin.
void __fastcall ThemeMenu_Refresh(u8 *self);

// 0x5e27b0 (thiscall; list, pops 4; name invented, unconfirmed): for each item of the list (nodes as above) the table's wide strings for the wide form of the session's value for the item's key
// (+0x4c), LobbyScript_Load with that value, the item, its index and the strings. +0xe4 = 1, +0xc = the first item of the list at +8. The fonts as above.
void __fastcall ThemeMenu_Load(u8 *self, void *, u8 *list);

// 0x5e18d0 (thiscall; name invented, unconfirmed): the virtual +0x24 not 0: nothing. When a second asking of it is 0: +4 = 1, the lobby's view set up ([0xf83774]'s holder, the ambient, light and fog of [0xf83778], its +4 from [[0xf82884]+0x10]+4),
// +0xc, +0x10, +0x14 = 0, the session's connections reset. Then the dialog opened, the virtual +0x28; each string of the list [0x786b90] loaded with the table's texts (LobbyScript_Load, item 0, index -1); the fonts removed; +0xc the first item of the
// list at +8, or the one named [0xf85488]; the dialog shows the theme; +0xe4 the time; ThemeMenu_Refresh; the item's two sounds played ([0x803a4c] by its +0x18) and the settings applied.
void __fastcall ThemeMenu_Enter(u8 *self);
}
