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
}
