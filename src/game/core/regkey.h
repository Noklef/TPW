// A registry key under "software\Bullfrog Productions Ltd" (0x5f8ae0-0x5f8da0): opened (made if
// missing) in HKEY_CURRENT_USER or HKEY_LOCAL_MACHINE, its values read whole or as a u32.
//
// The originals' quirks are kept:
// - Open closes a key it holds first, keeping the new one even if the close failed; a subkey is
//   made with the company key closed after it. The keys are made with KEY_ALL_ACCESS.
// - Close leaves a key whose RegCloseKey fails held (and fails).
// - GetU32 reads 4 bytes into the name's argument slot, then asks the value's type separately:
//   REG_DWORD and REG_DWORD_BIG_ENDIAN (5) are taken as read, REG_LINK (6) byte-swapped (one
//   off from the big-endian type); anything below 4 or above 6 fails. Bytes the read leaves
//   unwritten are the name pointer's.
// - TypeOf gives -2 for a name that is a subkey rather than a value, -1 for neither.
#pragma once

#include "../../../re/types.h"

struct RegKey {
    u32 handle;  // HKEY; 0 when closed
};

struct RegKeyEnv {
    void(__fastcall *ctor)(RegKey *);  // 0x581da0: the handle zeroed
    s32(__stdcall *const *regCloseKey)(u32 key);  // the import table's (0x6fd000)
    s32(__stdcall *const *regOpenKey)(u32 key, const char *name, u32 *result);  // 0x6fd004
    s32(__stdcall *const *regQueryValueEx)(u32 key, const char *name, u32 *reserved, u32 *type, void *data,
                                           u32 *size);  // 0x6fd008
    s32(__stdcall *const *regCreateKeyEx)(u32 key, const char *name, u32 reserved, char *cls, u32 options,
                                          u32 access, void *security, u32 *result,
                                          u32 *disposition);  // 0x6fd00c
    const char *companyPath;  // 0x775748: "software\Bullfrog Productions Ltd"
};
// nullptr restores the game's.
void RegKey_SetEnv(const RegKeyEnv *e);

extern "C" {
RegKey *__fastcall RegKey_Ctor(RegKey *k);
void __fastcall RegKey_Dtor(RegKey *k);
// (&out, subkey of the company key or null for it, nonzero: HKEY_CURRENT_USER, else
// HKEY_LOCAL_MACHINE): 0, or -1 when it can't be opened or made.
s32 *__fastcall RegKey_Open(RegKey *k, void *edx, s32 *out, const char *name, u32 currentUser);
// (&out): 0, or -1 when RegCloseKey fails.
s32 *__fastcall RegKey_Close(RegKey *k, void *edx, s32 *out);
// RegCreateKeyExA(root, path, ..., volatile ? REG_OPTION_VOLATILE : 0, KEY_ALL_ACCESS): the key, or 0.
u32 __cdecl Reg_CreateKey(const char *path, u32 root, u32 isVolatile);
// (&out, name, buffer, size): the value read; 0, or -1 (no key or name, or the read fails).
s32 *__fastcall RegKey_Query(RegKey *k, void *edx, s32 *out, const char *name, void *data, u32 size);
// (&out, name, &value): 0, or -1 when it isn't a 4-byte number (value untouched).
s32 *__fastcall RegKey_GetU32(RegKey *k, void *edx, s32 *out, const char *name, u32 *value);
// GetU32, the default stored when it fails.
s32 *__fastcall RegKey_GetU32Or(RegKey *k, void *edx, s32 *out, const char *name, u32 *value, u32 fallback);
// The value's REG_ type, -2 for a subkey, -1 for neither.
s32 __fastcall RegKey_TypeOf(RegKey *k, void *edx, const char *name);
// 0x581da0 (thiscall): the shared trivial constructor RegKey's calls (and the renderer's, 0x576b01):
// the first dword zeroed.
void __fastcall Ptr_Ctor(u32 *p);
// 0x45f6b0: 1 when the company key's subkey named at [0x7a2b08] opens in HKEY_LOCAL_MACHINE
// and its value named at [0x7a2358] reads as a u32 (which is dropped), else 0; the startup's
// check (0x45f640) that the game is installed. (A second key is made and closed, unused.)
u8 __cdecl Registry_GameInstalled();
// 0x5c0d00 (cdecl (&out); name unconfirmed): the language id read from the company key's subkey named at [0xf7f6b8] (in
// HKEY_LOCAL_MACHINE), its value named at [0xf7ee18] (0x809 when it can't be read as a u32). Once both succeed the id becomes a flag at
// 0x79d868 (6: 0x100, 7: 4, 10: 8, 0x10: 0x20, 0x11: 0x200, 0x13: 0x40, 0x15: 0x80, 0x1d: 0x10, 0x404: 0x400, 0x409: 0x8000,
// 0x40c: 2, 0x40d: 0x4000, 0x412: 0x2000, 0x41e: 0x1000, 0x804: 0x800, 0x809: 1, any other: 0x809, sic), the name of the table entry
// [0xf7ec30 + 0x14 x its lowest set bit] is copied to the language buffer 0xf7f230, and the language object 0x78b530 takes the id as
// read. Out = 0 always; returns &out.
s32 *__cdecl Registry_LoadLanguage(s32 *out);
}
