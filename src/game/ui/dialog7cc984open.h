// The dialog [0x7cc984] opened: a park's details from its online record. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../core/stdwstring.h"
#include "../core/string.h"

extern "C" {

// 0x4c0ae0 (cdecl; record, parent, callback, a, b; name invented, unconfirmed). Nothing when [0x7cc984] is set. The record assigned to 0x7cc988 (Class70293c_Assign), the three after the parent to [0x7cca48] / 4c / 50. No parent: the
// owner's window ([0xf82884] + 4), else 200 down (Window_Offset). The script 0x759170 built with the handler 0x4c0880 as [0x7cc984], at the depth Gui_TopDepth(0, 0) + 4. Under its child 0xe5932 each row gets a text part, the font 6 and its text.
// The record's type [0x7cc9d0] known to the session's list (holder +0xe0, its +0x60 set): only the row 0xe5937, "theme : label", the child 0xe592f hidden. Else the row 0xe5937 "text 0x113 : [0x7cc9c4]", the child 0xe592f shown when that is the
// session's own name (0x5b6480, case blind; the save's censor flag: a copy censored and thrown away, sic), 0xe5934 text 0x115 and the formatted text 0x1c0 of the words [0x7cca30] / 32 / 34, 0xe5936 text 0x114 and [0x7cca24] (empty: text 0x11c),
// 0xe5933 and 0xe5938 text 0x117 and 0x118 with [0x7cc9d8] and [0x7cc9dc], 0xe5939 the theme's label, 0xe593a text 0x111 with [0x7cca40] (0x7fffffff: "-"), 0xe5935 text 0x11b and text 0x14c, or 0x14d when [0x7cca44].
// Then the child 0xe5930 an empty text part with [0x7cc9b0] (censored by the save's flag) and the child 0xe5931 a TextPart6ff530 with [0x7cc99c] (the same), the font 5, yellow on hover; the focus on the dialog (0); the scene queue pushed 0x251.
void __cdecl Dialog7cc984_Open(const u8 *record, u8 *parent, u32 callback, u32 a, u32 b);
}

// Open's calls (the bench's recorders; their own benches have them). 0x5b50b0 and 0x5d3230 are unported (agent B's range). The window's find, the strings and the CRT run.
struct Dialog7cc984OpenEnv {
    u8 *(__fastcall *assign)(u8 *item, void *, const u8 *other);                  // Class70293c_Assign
    s32(__cdecl *topDepth)(u8 *window, u8 *shape);                                // Gui_TopDepth
    u8 *(__fastcall *build)(u8 *window, void *, u8 *script, u32 handler);         // WindowScript_Build
    void(__fastcall *setDepth)(u8 *window, void *, u32 depth);                    // Window_SetDepth
    u32(__fastcall *offset)(u8 *window, void *, u32 dx, u32 dy);                  // Window_Offset
    u8 *(__fastcall *holderCtor)(u8 *block);                                      // 0x5b50b0
    u8 *(__fastcall *findByKey)(u8 *owner, void *, u32 key);                      // Owner2c_FindByKey
    u32(__fastcall *valueByKey)(u8 *owner, void *, u32 key);                      // Owner2c_ValueByKey
    void(__fastcall *setTheme)(u8 *self, void *, const String *theme, u8 *label); // NamedObjects_SetThemeLabelAnsi
    u16 *(__cdecl *text)(u32 index);                                              // Language_WideText
    u8 *(__fastcall *partCtor)(u8 *part, void *, u32 value);                      // TextPart_Ctor
    void(__fastcall *setImpl)(u8 *window, void *, u8 *impl);                      // Window_SetImpl
    u8 *(__cdecl *font)(u32 index);                                               // Language_Font
    void(__fastcall *set4c)(u8 *window, void *, u8 *object);                      // Window_Set4c
    void(__fastcall *notify100)(u8 *window, void *, u32 a);                       // Window_NotifyImpl100
    u32(__fastcall *setShown)(u8 *window, void *, u32 shown);                     // Window_SetShown
    u8 *(__fastcall *saveCtor)(u8 *info);                                         // GlobalSave_Construct
    u8(__fastcall *byte25)(const u8 *info);                                       // GlobalSave_Byte25
    void(__fastcall *censor)(u8 *save, void *, StdWString *text, u32 a);          // 0x5d3230
    void(__fastcall *notify3000)(u8 *window, void *, u32 a, u32 b);               // Window_Notify3000
    void(__fastcall *notifyC00)(u8 *window, void *, u32 a, u32 b);                // Window_NotifyC00
    u8 *(__fastcall *part2Ctor)(u8 *part, void *, u32 value);                     // TextPart6ff530_Ctor
    void(__cdecl *hover)(u8 *window);                                             // HoverLabel_SetYellow
    u32(__fastcall *setFocus)(u8 *window, void *, u32 a);                         // Window_SetFocus
    void(__fastcall *push)(u8 *object, void *, u32 id, u32 clear);                // Class702da0_Push
};
void Dialog7cc984Open_SetEnv(const Dialog7cc984OpenEnv *env);

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8[0x10], gDialog7cc984open_759160, 0x00759160, 0x10);   // a window script (or a text / table before one): to the next named address
GAME_DATA(u8[0x230], gDialog7cc984open_759170, 0x00759170, 0x230);   // a window script (or a text / table before one): to the next named address
