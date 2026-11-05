// The user slots object at [0xf81ebc] (0x110 bytes, made by 0x5c7420; subject unconfirmed): four records of 24 bytes from +0, each a
// wide String (+4 its length) and a dword at +0x14, then +0x60 = -1, a 0x5cadf0 object at +0x64 and a list head at +0x100. Its
// filler (0x5c7590) walks the save folder's "users" subfolders (DirEntry_IsRealDir); the dialog code at 0x4a6xxx asks it for the
// slots in use. Reimplementation of the record accessors 0x5c7bb0, 0x5c7d00 and 0x5c7d20.
#pragma once

#include "../../../re/types.h"

struct String;
struct WString;
struct Handle;

struct UserSlotsEnv {
    u8 *(__fastcall *ctor)(u8 *slots);                     // UserSlots_Ctor
    u16 *(__cdecl *itow)(s32 value, u16 *out, s32 radix);  // 0x67e010: _itow, unported
    void(__cdecl *removeTree)(const String *path);         // GamePaths_RemoveTree
};
void UserSlots_SetEnv(const UserSlotsEnv *env);  // nullptr: the game's

// UserSlots_Scan's calls (the bench's recorders; each has a bench of its own, the atoi, the digit check and the global save's two are still the game's).
struct UserSlotsScanEnv {
    WString *(__stdcall *saveFolder)(WString *out);                       // GamePaths_SaveFolder
    WString *(__stdcall *usersFolder)(WString *out);                      // GamePaths_UsersFolder
    WString *(__stdcall *onlineFolder)(WString *out);                     // GamePaths_OnlineFolder
    s32 *(__fastcall *open)(Handle *h, void *, s32 *result, u32 path);    // Handle_Open28
    s32 *(__fastcall *first)(Handle *h, void *, s32 *result, u32 pattern, u32 entry);  // Handle_Vfn1
    s32 *(__fastcall *next)(Handle *h, void *, s32 *result, u32 entry);   // Handle_Vfn2
    s32 *(__cdecl *create)(s32 *result, const char *path);                // Dir_Create
    u32(__cdecl *isRealDir)(const u8 *entry);                             // DirEntry_IsRealDir
    s32(__cdecl *isDigit)(s32 c);                                         // 0x67a560
    s32(__cdecl *atoi)(const char *text);                                 // 0x67a520
    WString *(__fastcall *slotFolder)(const u8 *self, void *, WString *out, s32 i);  // UserSlots_SlotFolder
    u8 *(__fastcall *saveCtor)(u8 *info);                                 // GlobalSave_Construct
    void(__fastcall *saveDtor)(u8 *info);                                 // 0x5af250
    void(__fastcall *saveLoad)(u8 *info, void *, const char *path);       // 0x5afd70
    u8(__fastcall *saveByte24)(const u8 *info);                           // GlobalSave_Byte24
    void(__fastcall *makeFolders)(void *, void *, const WString *root);   // GamePaths_MakeFolders
};
void UserSlots_SetScanEnv(const UserSlotsScanEnv *env);  // nullptr: the game's

// UserSlots_Choose's and UserSlots_Close's other calls (the bench's recorders).
struct UserSlotsChoiceEnv {
    void(__fastcall *mailboxOpen)(u8 *mailbox, void *, const char *path);    // Mailbox_Open
    void(__fastcall *bookLoad)(u8 *book, void *, const char *path);          // AddressBook_Load
    void(__fastcall *bookSave)(u8 *book, void *, const char *path);          // AddressBook_Save
    void(__fastcall *outboxSave)(u8 *mailbox);                               // Mailbox_SaveOutbox
    void(__fastcall *saveWrite)(u8 *info, void *, const char *path);         // GlobalSave_WriteFile
    u8 *(__cdecl *saveInstance)();                                           // GlobalSave_Instance
    u8 *(__fastcall *themeGet)(u8 *info, void *, const u8 *name);            // GlobalSave_ThemeGet
    void(__fastcall *settingRead)(void *setting);                            // Setting_ReadMode
    void(__fastcall *settingSet)(void *setting, void *, u32 mode);           // Setting_SetMode
    s32(__cdecl *atexit)(u32 function);                                      // CRT_atexit
    void(__fastcall *optionsSave)(void *options, void *, const String *path);  // Options_SaveFile
    void(__fastcall *copyThemes)(u8 *self, void *, const WString *folder, u32 flag);  // 0x5c8190
    void(__fastcall *setByte24)(u8 *info, void *, u32 value);                // GlobalSave_SetByte24
    void(__fastcall *mailboxDtor)(u8 *mailbox);                              // Mailbox_Dtor
};
void UserSlots_SetChoiceEnv(const UserSlotsChoiceEnv *env);  // nullptr: the game's

extern "C" {

// 0x5c83b0 (thiscall (index), pops 4; name invented, unconfirmed): +0x60 = the slot; the slot's folder gives the mailbox at +0x64 and the address book at +0x100 their files; the global save info replaced and loaded from the folder, "\" (0xf82508) and the name at
// 0xf818a8. The mode setting 0xfb3b7c not 1: set to 2 or 0 by the save's byte +0x24. Each name of the list [0x786b90] has its theme got.
void __fastcall UserSlots_Choose(u8 *self, void *, s32 index);

// 0x5c8650 (thiscall; name invented, unconfirmed): the address book and the outbox saved, the global save written to the slot's file and deleted; the options saved in the save folder's file ("\" and 0xf82028); +0x60 = -1.
void __fastcall UserSlots_Close(u8 *self);

// 0x5c8a20 (thiscall (i), pops 4; name invented, unconfirmed): the global save info, made when there is none, written to the slot's folder, "\" (0xf82508) and the name at 0xf818a8.
void __fastcall UserSlots_WriteSave(u8 *self, void *, s32 i);

// 0x5c7f40 (thiscall (i, name, flag), pops 0xc; name invented, unconfirmed): a slot in use has its folder removed and is cleared; it gets the wide name (the vtable's +0x18) and the flag, 0x5c8190 fills its folder, the global save info is made new with its byte
// +0x24 from the flag and written (as UserSlots_WriteSave). No block: 0 asked to set the byte.
void __fastcall UserSlots_Create(u8 *self, void *, s32 i, const WString *name, u32 flag);

// 0x5c74b0 (thiscall; name invented, unconfirmed): the strings' list at +0x104 erased and its head freed, the mailbox at +0x64 destroyed, the four records' strings destroyed (CRT_VectorDtorIterator with 0x5c74a0).
void __fastcall UserSlots_Dtor(u8 *self);

// 0x5c7420 (thiscall; name invented, unconfirmed): the four records' empty wide strings (CRT_VectorCtorIterator with WString_CtorReturning, Wrap_WString_Dtor_5c74a0), the part at +0x64
// (UserSlotsPart_Ctor), the list head at +0x100 (ListHead_Ctor), +0x60 = -1. Returns the object.
u8 *__fastcall UserSlots_Ctor(u8 *self);

// 0x5c7590 (thiscall; name invented, unconfirmed): the four records' names assigned the global 0x78a374, their dwords cleared. The save, users and online folders opened by a folder handle (vtable 0x6fd9e8), each made
// (Dir_Create) when it does not open: save not opening makes all three. Each entry of users found with the pattern [0xf825d0]: a real folder of two characters or more, a first digit 1 to 4: its record i (the digit - 1) gets the
// rest of the name, and +0x14 the byte +0x24 of the global save info (0xf7c58c, replaced) loaded from the slot's folder, "\" (0xf82508) and the name at 0xf818a8; the slot's folders made (GamePaths_MakeFolders).
void __fastcall UserSlots_Scan(u8 *self);

// 0x5c7d30 (thiscall (out, i), pops 8; name invented, unconfirmed): out = i + 1 in decimal (_itow) and record i's name (Records24_IfUsed; a record not in use is a null string appended: a fault). Returns out.
WString *__fastcall UserSlots_SlotName(const u8 *self, void *, WString *out, s32 i);

// 0x5c7de0 (thiscall (out, i), pops 8; name invented, unconfirmed): out = GamePaths_UsersFolder, the global "\" (0xf82508) and UserSlots_SlotName. Returns out.
WString *__fastcall UserSlots_SlotFolder(const u8 *self, void *, WString *out, s32 i);

// 0x5c7e90 (thiscall (i), pops 4; the SEH frame left out; name invented, unconfirmed): the slot's folder removed (GamePaths_RemoveTree, the path in ANSI), the slot's name
// assigned the global at 0x78a374 through its vtable (+0x14), its dword at +0x14 cleared.
void __fastcall UserSlots_ClearSlot(u8 *self, void *, s32 i);

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
