// The GUI library's mouse cursors: a table [0xfaa604] of +0 the cursor objects and +4 the word count, the current cursor id the word
// 0x77c484 (its object [0xfaa608]), the default id the word 0xfaa600, and each window's id at its word +0x50 (applied while it is the
// hovered window [0xfaa5e0]). The game sets its cursor on the window [0x7cb2ac], or with none on the two at 0x7cc8c0 and 0x7cc8bc.
// The game's cursor files fill two tables: 0x7b9e88 (the .ani / .cur files) and 0x7b9ab0 (the .tga ones).
// Reimplementation of 0x661d96, 0x661c36, 0x661cca, 0x661d3f, 0x659248, 0x6590f7, 0x65f11d, 0x65f150, 0x4a2aa0, 0x4814a0, 0x52f950,
// 0x489720, 0x4899c0, 0x661935, 0x661dd0 and 0x661e30. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

// The cursor files' loaders (cdecl (name): the cursor object, or 0), not ported. nullptr restores the game's.
struct GameCursorsEnv {
    u8 *(__cdecl *loadAni)(const char *name);   // 0x4895b0
    u8 *(__cdecl *loadTga)(const char *name);   // 0x489670
};
void GameCursors_SetEnv(const GameCursorsEnv *e);
const GameCursorsEnv *GameCursors_GetEnv();

extern "C" {

// 0x661d96 (thiscall (index), pops 4): the entry at the index (its low word) when that is below the count, else 0.
u8 *__fastcall CursorTable_Get(const u8 *table, void *, u32 index);

// 0x661c36 (thiscall): with entries, each set one deleted (its slot 0, the deleting destructor (1)), the array read again each time,
// then the array freed (Mem_Free); the count word 0 either way.
void __fastcall CursorTable_Clear(u8 *table);

// 0x661cca (thiscall (count: a word), pops 4): the entries a new block of count dwords (Mem_Alloc), each 0, and the count word set; 1 in
// AX, 0 when the allocation fails (the count then unchanged). The rest of EAX not kept.
u16 __fastcall CursorTable_Init(u8 *table, void *, u32 count);

// 0x661d3f (thiscall (index, cursor), pops 8): the cursor stored at the index (its low word) when that is below the count, the cursor is
// set and the entry is empty: 1 in AX; else 0 in AX. The rest of EAX not kept.
u16 __fastcall CursorTable_Set(u8 *table, void *, u32 index, u8 *cursor);

// 0x489720 (cdecl): the .ani / .cur cursors loaded (the env's loadAni) into the table 0x7b9e88 (CursorTable_Set; a failed load skipped):
// c_norm 0, c_busy 1, c_path 3, c_queue 4, c_link 5, c_place 7, c_noplace.cur 8, c_pick 2, c_carry 9, c_erase 10, c_pylon 11, c_cam 0x13,
// c_cash 0x14, c_track 6, c_height 0xc, c_rotate 0xd, c_pivot 0xe, c_swapdown 0xf, c_swapup 0x10, c_end 0x12, c_move 0x11, c_crosshair
// 0x15, in that order.
void __cdecl GameCursors_LoadAni();

// 0x4899c0 (cdecl): the same for the .tga cursors (the env's loadTga) into the table 0x7b9ab0, the same slots in the same order (CNor,
// Cbus, Cpat, Cque, Clin, Cpla, Cnog, Cpic, Ccar, Cera, Cpyl, Ccam, Cash, Ctra, Chei, Crot, Cpiv, Csdn, Csup, CEnd, CMov, CCro), the
// carry cursor's bytes +5 and +6 then set to 10 and 0x40 and the crosshair's to 0x10 and 0x10 (Fn_6618c7; a hot spot?), when loaded.
void __cdecl GameCursors_LoadTga();

// 0x6590f7 (cdecl (id); only its low word is read): nothing with no table or when the id is the current one (both signed); -1 means the
// default id. The current cursor's slot 4 (0) and its byte +4 (Fn_66190a) taken; then the new one (or the default one when it has none)
// stored at 0xfaa608 and its id at 0x77c484, and when it is set its slot 4 (1), then its slot 2 when that byte was nonzero. The table
// is read again for each lookup.
void __cdecl Gui_SetCursor(u32 id);

// 0x659248 (cdecl (table); name unconfirmed): nothing for none or the table already in use. Else the current cursor [0xfaa608] (when set)
// gets slot 4 (0), the table becomes [0xfaa604], and the current id's (0x77c484) entry there (CursorTable_Get) becomes the current cursor
// (0 when it has none), first getting slot 4 (1) when set.
void __cdecl Gui_SetCursorTable(u8 *table);

// 0x65f11d (thiscall (id), pops 4): the word +0x50 = id; when the window is the hovered one [0xfaa5e0], Gui_SetCursor(word +0x50).
void __fastcall Window_SetCursor(u8 *window, void *, u32 id);

// 0x65f150 (thiscall): Gui_SetCursor(word +0x50).
void __fastcall Window_ApplyCursor(const u8 *window);

// 0x4a2aa0 (cdecl (id)): with the window [0x7cb2ac] set, Window_SetCursor on it, then the dword 0x7cb2d8 = the id's low word sign-extended.
void __cdecl GameCursor_Set(u32 id);

// 0x4814a0 (thiscall on the object at 0x7b51f0, this not used (id), pops 4): GameCursor_Set with the window [0x7cb2ac] set; otherwise
// Window_SetCursor on each of the windows [0x7cc8c0] and [0x7cc8bc] that is set.
void __fastcall Obj7b51f0_SetCursor(void *self, void *, u32 id);

// 0x52f950 (cdecl): the cursor for the game mode [0x81ae2c] (GameCursor_Set): mode 1, 3, 0xb and 0x10 give 5 with the dword 0x816d60
// set, else 0x12 with 0x816d4c set, else 3, 4, 6 and 6; mode 0x15 and 0x16 give 0x12 with 0x816d4c set, else 0xb and 0x11; mode 4 and
// 0x3b give 7, mode 0x39 gives 3; other modes none. Then 8 when Global816d48_Get() is nonzero and 0x14 when the dword 0x816d5c is set.
void __cdecl GameCursor_Update();

// The cursor objects: the base class's vtable 0x7059b0 (Fn_661870 builds it: +4 1 (shown), +5 / +6 0, +0x10 0; Fn_6618a6 its
// destructor), the system cursor's 0x7059d0 (its handle +0x14, shown and hidden by the imports 0x6fd334 / 0x6fd30c). Names unconfirmed.
// 0x661935 (thiscall; called by 0x48961e): the system cursor built: the base's (Fn_661870), the vtable 0x7059d0, the handle +0x14 0.
// Returns the cursor.
u8 *__fastcall SysCursor_Ctor(u8 *cursor);

// 0x661dd0 (thiscall (flags), pops 4; slot 0 of the vtable 0x7059b0): the base's scalar deleting destructor: Fn_6618a6, then with flag
// 1 the block freed (Mem_Free). Returns the cursor.
u8 *__fastcall Cursor_Delete(u8 *cursor, void *, u32 flags);

// 0x661e30 (thiscall (red, green, blue: bytes), pops 12; called by 0x661a47 (the cursor code's white); name unconfirmed): a palette
// colour (gfx/palette's 5-byte kind) built: PalColour_Set with the three bytes. Returns the colour.
u8 *__fastcall PalColour_Ctor(u8 *colour, void *, u32 red, u32 green, u32 blue);

}
