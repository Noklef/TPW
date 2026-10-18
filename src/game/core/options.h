// The game options (0x78d8d8, the options screen's): reimplementation of 0x423690, 0x423ad0-0x423ba0 and 0x423dd0
// (thiscall). +0..+0x3c are the options in force (+0 procedural terrain, +4 the resolution when
// the balance file gives none), +0x3c..+0x78 a copy to go back to; +0x94 a level of change (2 when
// the display must be reset) and +0x98 flags whose low three bits mark changes.
#pragma once

#include "../../../re/types.h"

struct Stream;

extern "C" {

// 0x423ad0: the copy back over the options; +0x94 = 0, the change bits cleared.
void __fastcall Options_Revert(u8 *options);
// 0x423b00: the options over the copy; +0x94 at least 0, the change bits cleared.
void __fastcall Options_Keep(u8 *options);

// 0x423b40: +0x30 = value.
void __fastcall Options_Set30(u8 *options, void *, u32 value);
// 0x423b60 / 0x423ba0 / 0x423b80: +4 / +8 / +0xc = value (written only when it differs).
void __fastcall Options_Set4(u8 *options, void *, u32 value);
void __fastcall Options_Set8(u8 *options, void *, u32 value);
void __fastcall Options_SetC(u8 *options, void *, u32 value);

// 0x550d80 (thiscall; mode, pops 4): the dword at +0 = mode (the original reports mode >= 3 to its empty debug printf); for 0 / 1 / 2 the bits 23..25
// of the dword at 0x7a1a8c are set to 1 / 2 / 4 (bit 23 / 24 / 25 alone: the other two cleared); other modes leave it.
void __fastcall Setting_SetMode(u8 *object, void *, u32 mode);

// 0x550ca0 (thiscall): the mode is read from the bits 23..25 of the dword at 0x7a1a8c (the highest set of 25 / 24 / 23 gives 2 / 1 / 0; none gives 0), stored
// at +0 and the bits set to that single one (as Setting_SetMode does). Returns the object.
u8 *__fastcall Setting_ReadMode(u8 *object);

// 0x41a9d0 (no arguments): in the 0x4000 render size, 8 / 6 for a 8 / 16 bit depth, 4 for 32 and else 0; otherwise the level 0 to 3 of gBalanceGraphics (0x7858d0) as 2, 4, 6,
// 8, and 8 for any larger.
u32 __cdecl Graphics_Level();

// 0x41aa40 (no arguments; name unconfirmed): in the 0x4000 render size, 2 for a 8 or 16 bit depth, 1 for 32 and else 0; otherwise 1 for
// gBalanceGraphics (0x7858d0) 0, else 2.
u32 __cdecl Graphics_Tier();

// 0x423dd0 (cdecl): the game options' (0x78d8d8) sound settings applied: for each switch (the bytes +0x18, +0x20, +0x10)
// its sound flag (Sound_Set803adc / ae0 / ad8) set 1 and its volume (+0x1c / +0x24 / +0x14: Sound_Set803ac0 / ac4 /
// abc) passed on, or the flag set 0; then Global802ba8_SetMasked(the dword +0x2c * 1023 / 100, truncated) when the
// byte +0x28 is set, else 0.
void __cdecl Options_ApplySound();

// 0x423690 (thiscall): the defaults: +0, +4, +8 = 1, +0xc = 0; the volumes +0x14 / +0x1c / +0x24 / +0x2c from the dwords at 0x785928 / 2c / 30 / 34
// and, when the dword at 0x785924 is set, the switches +0x10 / +0x18 / +0x20 / +0x28 (bytes) = 1 (left as they were otherwise); +0x30 = the dword at
// 0x78591c, the bytes +0x34..+0x37, +0x39, +0x3a = 1 and +0x38 = 0; then the copy at +0x3c = the options and Options_ApplySound (which reads the
// game's options at 0x78d8d8, whatever this is).
void __fastcall Options_Defaults(u8 *options);

// The options' save and load (thiscall; stream, saving, an unused dword; pops 12; names unconfirmed): saving, each field written whole (Stream_Write); loading,
// read (Stream_Read, or quietly by name as the save game's fields: Save_ReadU8 / Save_ReadU32 / Save_Read8Bytes). 1 at the first that fails,
// else 0.
// 0x423e90: the volumes +0x10, +0x18, +0x20, +0x28 (8 bytes each: SFX, music, speech, movie) and the switches +0x34..+0x3a (AdvisorOn,
// TutorialOn, TooltipsOn, ConfirmDeleteOn, RMBScrollOn, RMBCancelOn, IsometricOn); then the sound applied (Options_ApplySound).
u32 __fastcall Options_SerializeSound(u8 *options, void *, Stream *stream, u32 saving, u32);
// 0x4242c0: +0, +4, +8 (read with Stream_Read), +0xc (VideoCard), +0x28 (MovieVolume, 8 bytes), +0x30 (SoundQuality); then the sound applied.
u32 __fastcall Options_SerializeDisplay(u8 *options, void *, Stream *stream, u32 saving, u32);
// 0x424560: +0x78, +0x7c, +0x80 (read with Stream_Read), +0x84 (CURRENTTheme), +0x88 (DEBUGGuardUInts), +0x8c (TextureCacheSize).
u32 __fastcall Options_SerializeMisc(u8 *options, void *, Stream *stream, u32 saving, u32);

// The options file (thiscall; path, a String whose text is at +8; pops 4; names unconfirmed): 1 when it went through, else 0 (the reports to the
// debug printf left out).
// 0x424820: opened for writing (FileStream_Open, mode 0xc0000010; nothing more when that fails), the version 1 written (4 bytes), then
// Options_SerializeDisplay (saving, the version); closed (Stream_Close) after the stream was used.
u32 __fastcall Options_SaveFile(u8 *options, void *, const u8 *path);
// 0x424930: the defaults first (Options_Defaults); opened for reading (mode 0x80000001), the version read (4 bytes; 0 is refused), then
// Options_SerializeDisplay (loading, the version); closed after the stream was used.
u32 __fastcall Options_LoadFile(u8 *options, void *, const u8 *path);
// 0x424a60: +0x78, +0x7c, +0x80 = 0, +0x84 = 1, +0x88..+0x90 = 0 first; opened for reading (mode 0x80000001), the version read (4 bytes; below 2,
// unsigned, is refused), then Options_SerializeMisc (loading, the version); closed after the stream was used.
u32 __fastcall Options_LoadMiscFile(u8 *options, void *, const u8 *path);

// 0x423bc0 (thiscall (mode), pops 4; name unconfirmed; 0x423740 calls it when the graphics mode +8 differs from its copy +0x44): a graphics balance file
// (vtable 0x6fd630) laid out on the stack (BalanceFile_Init) and loaded (keep 0) from the data folder (the wide string of 0xf816c0, through String's
// AssignWide, ending in "\") and the name for the mode (0 / 1 / 2: the Strings 0x78e338 / 0x78e368 / 0x78e5c0; others: the folder alone). Then, when it
// loaded, its +0xc, +0x68, +0x34, +0x3c, +0x40 and +0x44 against gBalanceGraphics' (0x785874, 0x7858d0, 0x78589c, 0x7858a4, 0x7858a8, 0x7858ac): each
// that differs raises the change level +0x94 to 2. The file gets its vtable back and is released either way. Returns 1 (AL) when it loaded, else 0.
u8 __fastcall Options_CheckGraphicsMode(u8 *options, void *, u32 mode);

// 0x45b1f0 (cdecl; name unconfirmed; called from 0x45aa17): the data folder (the wide string of 0xf816c0, through String's AssignWide, ending in "\") and
// in turn the names 0x7a1c28, 0x7a0c58 and 0x7a17c8 (Strings): gBalanceSound (0x7858f0) loaded (keep 0); when that gives 1, the game options' defaults
// (Options_Defaults on 0x78d8d8) and gBalanceOnline (0x785828) loaded; when that gives 1, Online_ApplyBalance and, when the third file exists
// (File_Exists gives 1), gBalanceResolution (0x7858e0) loaded, its result unused.
void __cdecl Options_LoadBalanceFiles();

// 0x423740 (thiscall; name unconfirmed; 0x4237f0 calls it): the options' changes against their copy: a graphics mode (+8) other than the copy's
// (+0x44) checked (Options_CheckGraphicsMode); when +0x6c and +0x30 lie on different sides of the dword 0x785910 (signed: one above it, the other
// not), the change level +0x94 at least 1 and bit 0 of +0x98 set; then +0, +4 and +0xc each unlike its copy (+0x3c, +0x40, +0x48) raises the level
// to 2.
void __fastcall Options_NoteChanges(u8 *options);
}
