// The user slots object at [0xf81ebc] (0x110 bytes, made by 0x5c7420; subject unconfirmed): four records of 24 bytes from +0, each a
// wide String (+4 its length) and a dword at +0x14, then +0x60 = -1, a 0x5cadf0 object at +0x64 and a list head at +0x100. Its
// filler (0x5c7590) walks the save folder's "users" subfolders (DirEntry_IsRealDir); the dialog code at 0x4a6xxx asks it for the
// slots in use. Reimplementation of the record accessors 0x5c7bb0, 0x5c7d00 and 0x5c7d20.
#pragma once

#include "../../../re/types.h"

struct WString;

struct UserSlotsEnv {
    u8 *(__fastcall *ctor)(u8 *slots);                     // UserSlots_Ctor
    u16 *(__cdecl *itow)(s32 value, u16 *out, s32 radix);  // 0x67e010: _itow, unported
};
void UserSlots_SetEnv(const UserSlotsEnv *env);  // nullptr: the game's

extern "C" {

// 0x5c7420 (thiscall; name invented, unconfirmed): the four records' empty wide strings (CRT_VectorCtorIterator with WString_CtorReturning, Wrap_WString_Dtor_5c74a0), the part at +0x64
// (UserSlotsPart_Ctor), the list head at +0x100 (ListHead_Ctor), +0x60 = -1. Returns the object.
u8 *__fastcall UserSlots_Ctor(u8 *self);

// 0x5c7d30 (thiscall (out, i), pops 8; name invented, unconfirmed): out = i + 1 in decimal (_itow) and record i's name (Records24_IfUsed; a record not in use is a null string appended: a fault). Returns out.
WString *__fastcall UserSlots_SlotName(const u8 *self, void *, WString *out, s32 i);

// 0x5c7de0 (thiscall (out, i), pops 8; name invented, unconfirmed): out = GamePaths_UsersFolder, the global "\" (0xf82508) and UserSlots_SlotName. Returns out.
WString *__fastcall UserSlots_SlotFolder(const u8 *self, void *, WString *out, s32 i);

// 0x5c8870 (thiscall (out), pops 4; name invented, unconfirmed): UserSlots_SlotFolder of the slot chosen (+0x60). Returns out.
WString *__fastcall UserSlots_ChosenFolder(const u8 *self, void *, WString *out);

// 0x5c8890 (thiscall (out), pops 4; name invented, unconfirmed): out = UserSlots_ChosenFolder, the global "\" (0xf82508) and the theme's label (the String 0x786b68, widened by WString_AssignFromAnsi). Returns out.
WString *__fastcall UserSlots_ThemeFolder(const u8 *self, void *, WString *out);

// 0x5b8160 (stdcall; out, the ECX unused; name invented, unconfirmed): out = UserSlots_ThemeFolder of UserSlots_Instance (a null instance faults) and the global wide strings 0xf7d740, 0xf7d668 and 0xf7cf30. Returns out.
WString *__stdcall UserSlots_ThemePath(WString *out);

// 0x5cadf0 (thiscall; name invented, unconfirmed): the object at the slots' +0x64: empty strings at +0, +0x10, +0x20, +0x3c (StdString_Tidy(false)), a list at +0x30 (ListHead_CtorFrom), lists at
// +0x58, +0x64, +0x70 (sentinels of 0xc bytes linked to themselves, the counts 0), +0x84, +0x88, +0x8c, +0x94, +0x98 = 0, the word +0x90 = 0x19. The allocator bytes +0, +0x10, +0x20, +0x30, +0x3c,
// +0x58, +0x64, +0x70 and +0x80 are the stack's garbage there: left as they are. Returns it.
u8 *__fastcall UserSlotsPart_Ctor(u8 *self);

// 0x48bcd0 (cdecl; name invented, unconfirmed): [0xf81ebc], made when 0: a block of 0x110 bytes and 0x5c7420's answer, or 0 without a block.
u8 *__cdecl UserSlots_Instance();

// 0x5c7bb0 (thiscall): how many of the four 24-byte records have a non-zero dword at +4.
u32 __fastcall Records24_CountUsed(const u8 *records);

// 0x5c7d00 (thiscall; i, pops 4): the address of record i of 24 bytes when its dword at +4 is not 0, else 0.
u8 *__fastcall Records24_IfUsed(u8 *records, void *, u32 i);

// 0x5c7d20 (thiscall; i, pops 4): the dword at +0x14 of record i of 24 bytes.
u32 __fastcall Records24_Field14(const u8 *records, void *, u32 i);
}
