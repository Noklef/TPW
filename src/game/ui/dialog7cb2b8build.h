// The game's main bar: its windows built once from the script at 0x752940 into [0x7cb2ac].
#pragma once

#include "../../../re/types.h"

// the calls the bench takes over (tests/fncmp/dialog7cb2b8build.py): crt_atexit by its stub, GuiText_Measure (its s8 arguments as the
// dwords pushed), BarMesh_Construct, Window_SetObject98, Window_SetHandler (the handlers are game code the driver cannot run, and the rectangles set after them call them)
struct Dialog7cb2b8BuildEnv {
    s32(__cdecl *atExit)(void(__cdecl *)());
    void(__cdecl *measure)(const u16 *text, const s32 *rect, u32 font, u32 hAlign, u32 vAlign, u32 fit, s32 *box);
    u8 *(__fastcall *barMesh)(u8 *bar, void *, const char *name);
    void(__fastcall *setObject)(u8 *window, void *, u8 *object);
    u16(__fastcall *setHandler)(u8 *window, void *, u32 handler);
};

void Dialog7cb2b8_SetBuildEnv(const Dialog7cb2b8BuildEnv *env);

// 0x4a1d70 (cdecl; name invented, unconfirmed): nothing when [0x7cb2b8] is set. Sets [0x7cb28c] 1, [0x7cb2a0] 3, [0x7cb290] 0xa, then
// builds the script (WindowScript_Build, no handler). When that succeeds: child 0x33 of [0x7cb2ac] kept at [0x7cb2a4] (depth 0, its slot 5,
// handler 0x4a0fb0), its 0x37 and (unless the mode setting [0xfb3b7c] is 2, when 0x34 is hidden instead of told 3) its 0x36 dressed as the
// counter L"0  x"; child 0x2d kept at [0x7cb2c4] and, with its child 0x2e, the handlers 0x4a11e0 / 0x4a1190, 0x2e moved to end at 0x2d's top
// and 0x2d's top put 8 above its bottom; child 0x2f sized from the text at 0x752f10's measure (font 1, mirrored alignment) and centred in
// its 0x32, its parts 0x30 (hidden) and 0x31 too. Whatever the build answered: child 0x1d (when found) gets its children 0x21 / 0x22 / 0x23
// (the last kept at [0x7cb2d0]), 0x24, 0x1e / 0x1f (a BarMesh of the name at 0x752f04), 0x20 ([0x7cb2d4] 3) and 0x25 and is kept at [0x7cb2b8].
void __cdecl Dialog7cb2b8_Build();

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u32, gDialog7cb2b8build_7cb28c, 0x007cb28c, 0x4);
GAME_DATA(u32, gDialog7cb2b8build_7cb290, 0x007cb290, 0x4);
GAME_DATA(u32, gDialog7cb2b8build_7cb2a0, 0x007cb2a0, 0x4);
GAME_DATA(u8 *, gDialog7cb2b8_Build_7cb2a4, 0x007cb2a4, 0x4);
GAME_DATA(u8 *, gDialog7cb2b8build_7cb2b8, 0x007cb2b8, 0x4);
GAME_DATA(u8 *, gDialog7cb2b8build_7cb2c4, 0x007cb2c4, 0x4);
GAME_DATA(u8 *, gDialog7cb2b8build_7cb2d0, 0x007cb2d0, 0x4);
GAME_DATA(u32, gDialog7cb2b8_Build_7cb2d4, 0x007cb2d4, 0x4);
GAME_DATA(u8, gDialog7cb2b8build_fb34c1, 0x00fb34c1, 0x1);

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8[0x4c0], gDialog7cb2b8_Build_752940, 0x00752940, 0x4c0);   // a window script (or a text / table before one): to the next named address
