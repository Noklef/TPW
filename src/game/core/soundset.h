// The sound module's settings (read by its update, 0x51bd80, when the changed flag 0x803ac8 is
// set, which it then clears): reimplementation of the cdecl setters 0x51bb60-0x51bc20, 0x51e6f0
// and 0x51e710, which store their value and set the changed flag, of 0x51c1c0 and 0x51c6f0, and of
// the sound commands' base constructor 0x51bb40. The options code sets them in pairs, a flag around a value
// (0x51bbe0 1, 0x51bb80 value, 0x51bbe0 0; likewise 0x51bc20 / 0x51bbc0 and 0x51bc00 / 0x51bba0);
// the update scales 0x803ac0 and 0x803abc by the percentage at 0x785914, like volumes.
#pragma once

#include "../../../re/types.h"

struct StringView;

extern "C" {

void __cdecl Sound_Set803ab8(u32 value);  // 0x51bb60
void __cdecl Sound_Set803ac0(u32 value);  // 0x51bb80
void __cdecl Sound_Set803abc(u32 value);  // 0x51bba0
void __cdecl Sound_Set803ac4(u32 value);  // 0x51bbc0
void __cdecl Sound_Set803adc(u32 value);  // 0x51bbe0
void __cdecl Sound_Set803ad8(u32 value);  // 0x51bc00
void __cdecl Sound_Set803ae0(u32 value);  // 0x51bc20
void __cdecl Sound_Set803acc(u32 value);  // 0x51e6f0
void __cdecl Sound_Set803ad0(u32 value);  // 0x51e710: its low 16 bits, a u16

// 0x51c1c0: the u16 at 0x803ad2 (when set, 0x51c1d0 plays at the fixed volume [0x700b28]), the
// changed flag left alone.
void __cdecl Sound_Set803ad2(u32 value);

// 0x51c6f0: the sound-on flag 0x802bc8 (set by 0x51b660; the play functions do nothing without it).
u32 __cdecl Sound_IsOn();

// 0x51bb40 (thiscall; value): a sound command's base, abstract (vtable 0x700b58, _purecall
// first), +4 value; returns it. Its callers then set their own vtable and post it to 0x802bcc.
u8 *__fastcall SoundCmd_Ctor(u8 *cmd, void *, u32 value);

// 0x51b4f0 (cdecl; a, b): 0 unless sound is on (0x802bc8) and started (0x802bd4); else the sound manager (0x802bcc) is
// asked through its vtable +4 (&a, a, b, &b) and b as it left it is returned. Its caller asks with a category's name
// ("cat_ride") and keeps the answer.
u32 __cdecl Sound_Query(u32 a, u32 b);

// 0x51b530 (cdecl; name, folders; name unconfirmed): the name looked up in a folder list (Class6fd9d8_FindFirstMatch with a String of the name and an
// empty String for what it finds); 0 when it is not found (the match's AL not 1), or sound is not on (0x802bc8) and started (0x802bd4). Else the
// sound manager (0x802bcc) is asked through its vtable +4 (&folders, the found String's text, name, &name) and name as it left it is returned.
u32 __cdecl Sound_QueryFound(const char *name, u8 *folders);

// 0x51bc40 (cdecl (sound, parameter, value); name unconfirmed): 0 unless sound is on (0x802bc8) and started (0x802bd4); else the sound
// manager (0x802bcc)'s SoundManager_Call3 (its engine's slot 3) with them, whose answer is returned (its callers keep it as the sound).
u32 __cdecl Sound_SetParam(u32 sound, u32 parameter, u32 value);

// 0x51c270 (cdecl (sound, x, y, z); name unconfirmed): 0 unless sound is on and started; else the manager's SoundManager_Call5 (its
// engine's slot 5) with the sound and the position as three floats on the stack (each converted from the int), whose answer is returned.
u32 __cdecl Sound_SetPosition(u32 sound, s32 x, s32 y, s32 z);

// The sound commands' senders (cdecl; names invented): nothing unless sound is on (0x802bc8) and started (0x802bd4); else the manager
// [0x802bcc]'s SoundManager_Apply with a stack command and an out that is one of the sender's own argument slots (EAX: that address, or 0
// when off; not kept).
// 0x51c0a0 (id): the command 0x700ba0 {id} (the sound object's functor 3 send), the out the id's slot.
void __cdecl Sound_VoiceFunctor3(u32 id);
// 0x51c0e0 (id, value): the command 0x700ba4 {value, id} (the mixer's pending value), the out the value's slot.
void __cdecl Sound_SetVoicePending(u32 id, u32 value);
// 0x51c2c0 (value): the command 0x700ba8 {value} (the sound object's slot 6), the out the value's slot.
void __cdecl Sound_Forward6(u32 value);
// 0x51c300 (id): the command 0x700bac {id, 0x3c} (the voice's slot 0xc with 0x3c), the out the id's slot.
void __cdecl Sound_Stop(u32 id);
// 0x51c640 (x, unused, y, radius, z): the command 0x700bc0 {x, y, z, radius, &x} (the occluder add: the node into x's slot), the out y's
// slot; returns x's slot after it (0 when off).
u32 __cdecl Sound_AddOccluder(u32 x, u32 unused, u32 y, u32 radius, u32 z);
// 0x51c6b0 (node): the command 0x700bc4 {&node} (the occluder remove), the out a local (unset before the run).
void __cdecl Sound_RemoveOccluder(u32 node);
// 0x51bc70 / 0x51bcf0 / 0x51bd30 (names invented): the command 0x700b5c / 0x700b64 / 0x700b68 alone (its runs read no value), the out a
// local (unset before the run).
void __cdecl Sound_Post700b5c();
void __cdecl Sound_Post700b64();
void __cdecl Sound_Post700b68();
// 0x51bcb0 (name invented): the command 0x700b60 {0x5a}, the out a local (unset before the run).
void __cdecl Sound_Post700b60();
// 0x51bd70 (name invented): with the changed flag [0x803ac8] set, the settings posted as commands (each {value}, the out one local, a
// second its functor): 0x700b88 with 0 when the u16 [0x803ad0] is set, else [0x803ac0]; 0x700b84 with [0x803abc] (both times [0x785914]
// / 100, signed, when [0x803acc] and [0x803ae0] are set and [0x803ac4] > 0); 0x700b80 [0x803ab8]; 0x700b7c [0x803ac4]; 0x700b78
// [0x803ae0]; 0x700b74 [0x803adc]; 0x700b70 [0x803ad8] (built by SoundCmd_Ctor); then the flag cleared. Then (changed or not) the
// command 0x700b6c, its out the dword after its vtable.
void __cdecl Sound_ApplySettings();
// 0x51c700 (level; name invented): the command 0x700bc8 {on, radius} (the occlusion switch), the out the level's slot: with the ground-level
// camera (gCameraFlags 0x790ab0 & 0x16), signed: above [0x785968] {1, [0x785964]}; above [0x785960] {1, (level - [0x785960]) *
// ([0x785964] - [0x78595c]) / ([0x785968] - [0x785960]) + [0x78595c]}; above [0x785958] the same between [0x785958] / [0x785954] and
// [0x785960] / [0x78595c]; above [0x78590c] {1, [0x785954]}; else {0, 0} (x87 single precision; the level's slot then holds the
// divisor). Without that camera {0, 0}.
void __cdecl Sound_SetCameraOcclusion(s32 level);
// 0x51b920 (level; name from its callees, unconfirmed): while on and started, the manager's posts for a camera level (the out the level's
// slot; each flag the level above a global, signed): 0x700b54 {[0x7858fc], [0x785900], [0x785904]} (SoundChannel_SetMode); 0x700b50 with
// [0x785908] (when not above it, and still on and started with an EAX record kept in 0x803ab4, first that cleared and 0x700b30 with record
// 0); 0x700b4c with not above [0x785910]; Sound_SetCameraOcclusion and Sound_SetVoicesForLevel; 0x700b48 with [0x785920]; then (on and
// started again) 0x700b44 (built by SoundCmd_Ctor) with not above [0x785918] (the channel shown or hidden).
void __cdecl Sound_SetCameraLevel(s32 level);
// 0x51c570 (id; name invented): a result 1, then (sound on and started) the command 0x700bb8 {&result, id} (SoundObject_VoiceInfo), the
// out the id's slot. Returns the result.
u32 __cdecl Sound_VoiceInfo(u32 id);

// 0x51b7c0 (cdecl): when on and started, the sound manager's vtable +0 (&result, not read after); then not started. The
// warning when the allocation count 0x802bd8 is not back to 0 is compiled out.
void __cdecl Sound_Shutdown();

// The sound categories, each Sound_QueryFound of a category name in a folder object the named objects registry (0x786b9c, NamedList_Find)
// holds under a key's name; a folder not found leaves its categories as they were. Their releases (cdecl; names invented) give each one
// that is not 0 to SoundCategory_Release, then clear them:
// 0x51ea50: the lobby sfx 0x803a48. 0x51ea70: with an array at 0x803a4c, both of each of its 0x803a50 pairs (signed; not tested for 0),
// then the array to Sound_Free; 0x803a4c and 0x803a50 cleared. 0x51eda0: 0x803a20, 0x803a28, 0x803a2c, 0x803a24, 0x803a30 and 0x803a34 in
// that order. 0x51ee40: the theme's 0x803a38-0x803a44.
void __cdecl Sound_ReleaseLobbySfx();
void __cdecl Sound_ReleaseLobbyCategories();
void __cdecl Sound_ReleaseGlobalCategories();
void __cdecl Sound_ReleaseThemeCategories();
//
// 0x51e8f0 (cdecl; name; name unconfirmed): the folder named the global String 0x803768 with name appended (String_AppendStr): -1 when not
// found; else its "cat_locallobbysfx" and "cat_locallobbymusic" are appended as a pair (+0, +4) to the array at 0x803a4c of 0x803a50 pairs (the
// count incremented, a new block from Sound_Alloc, the old pairs copied and the old block given to Sound_Free) and the pair's index returned.
s32 __cdecl Sound_RegisterLobbyCategories(const char *name);

// 0x51eae0 (cdecl): from the folder named the global String 0x8030d8, cat_ambient, cat_rides, cat_ui, cat_kids and cat_staff to 0x803a20,
// 0x803a28, 0x803a2c, 0x803a24 and 0x803a30; from the one named 0x803558, cat_speech to 0x803a34. (The debug prints of the six being 0
// beforehand are left out.)
void __cdecl Sound_RegisterGlobalCategories();

// 0x51e890 (cdecl): from the folder named the global String 0x8030d8, cat_globallobbysfx to 0x803a48. (The debug print of 0x803a48 being
// set beforehand is left out.)
void __cdecl Sound_RegisterGlobalLobbySfx();

// 0x51ec50 (cdecl; theme, a string or view): the String 0x803768 with the theme appended (String_Append): its folder's cat_ambient and
// cat_rides to 0x803a38 and 0x803a3c; then assigned 0x8039c0 (through the string's slot 6, String_AssignWide: its data as chars) with the theme:
// cat_speech to 0x803a40; then 0x802e68 with the theme: cat_music to 0x803a44.
void __cdecl Sound_RegisterThemeCategories(const StringView *theme);

// The sound globals' save and load (cdecl (stream); what EAX holds after is not a result). 0x51c350: the started flag (0x802bd4), the on
// flag (0x802bc8), 0x54 and the 0x54 bytes at 0x803aa0 written (Stream_Write); while on and started, the manager posts 0x700bb0 (the
// sound object's save, the stream). 0x51c400: the on flag kept, then read (Stream_Read, the results unchecked) the started dword, the on
// flag, a size and that many bytes into 0x803aa0 (the size unchecked, sic). Started (the dword read, not 0x802bd4): with a manager, it posts
// 0x700bb4 (the sound object's load, the stream, 1) and 0x700b30 (the EAX record 0x803ab4); without one, the record skipped as the sound
// object's load reads it (three dwords, then the mixer's size: a seek past the rest, size - 4 from here). Not started: an on flag that was
// set is put back.
void __cdecl Sound_Save(u8 *stream);
void __cdecl Sound_Load(u8 *stream);

// 0x51b810 (cdecl (level); name invented): the sound object's voice count (0x700b3c, SoundObject_SetVoices) for a level against the levels
// 0x785938-0x78594c (a, b, c, d, -, e; no on / started test). Above e: the level slot handed to 0x700b40 (the channel's query, 0x6b9060, may
// write it), then (level - d) * (that - c) / (100 - d) (unsigned) + c. Above d: c. Else (level - b) * (c - a) / (d - b) (signed) + a.
void __cdecl Sound_SetVoicesForLevel(s32 level);

// The manager's posts while on and started (0x802bc8, 0x802bd4; cdecl, void; the out each one's first argument's slot). 0x51b620: 0x700b2c
// (the sound object's slot 8 with the value). 0x51c060 (name invented): 0x700b9c (the id's voice's functor 2, SoundObject_SendVoiceFunctor2).
// 0x51c5d0: 0x700bbc with the sound and the area (x, z)..(x + w, z + d). 0x51c8d0 (name invented): an EAX record other than 0x803ab4's
// kept there and posted (0x700b30).
void __cdecl SoundCategory_Release(u32 value);
void __cdecl Sound_SendVoiceFunctor2(u32 id);
void __cdecl Sound_SetArea(u32 sound, u32 x, u32 z, u32 w, u32 d);
void __cdecl Sound_SetEaxRecord(u32 index);
// 0x51c1d0 (cdecl, nine dwords; name invented): while on and started, SoundManager_SetPlacement (out the first argument's slot) with the
// position (x, y, z; y the float [0x700b28] when the word 0x803ad2 is set; through the x87), the direction and the value triples.
void __cdecl Sound_SetPlacement(u32 x, u32 y, u32 z, u32 dx, u32 dy, u32 dz, u32 vx, u32 vy, u32 vz);
}
