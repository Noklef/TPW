// The company registry key: see regkey.h.
#include "regkey.h"

#include <cstdint>
#include <cstring>

#include "language.h"

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const RegKeyEnv kGameEnv = {
    at<void(__fastcall *)(RegKey *)>(0x00581da0),
    at<s32(__stdcall *const *)(u32)>(0x006fd000),
    at<s32(__stdcall *const *)(u32, const char *, u32 *)>(0x006fd004),
    at<s32(__stdcall *const *)(u32, const char *, u32 *, u32 *, void *, u32 *)>(0x006fd008),
    at<s32(__stdcall *const *)(u32, const char *, u32, char *, u32, u32, void *, u32 *, u32 *)>(0x006fd00c),
    at<const char *>(0x00775748),
};
const RegKeyEnv *g_env = &kGameEnv;

const u32 kCurrentUser = 0x80000001;   // HKEY_CURRENT_USER
const u32 kLocalMachine = 0x80000002;  // HKEY_LOCAL_MACHINE
const u32 kAllAccess = 0xf003f;        // KEY_ALL_ACCESS
const u32 kVolatile = 1;               // REG_OPTION_VOLATILE
const s32 kDword = 4, kDwordBigEndian = 5, kLink = 6;

u32 bswap(u32 v) { return v >> 24 | (v >> 8 & 0xff00) | (v << 8 & 0xff0000) | v << 24; }

}  // namespace

void RegKey_SetEnv(const RegKeyEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

RegKey *__fastcall RegKey_Ctor(RegKey *k) {
    g_env->ctor(k);
    return k;
}

void __fastcall RegKey_Dtor(RegKey *k) {
    s32 result;
    RegKey_Close(k, nullptr, &result);
}

s32 *__fastcall RegKey_Open(RegKey *k, void *, s32 *out, const char *name, u32 currentUser) {
    if (k->handle) {
        s32 result;
        RegKey_Close(k, nullptr, &result);
    }
    const u32 company = Reg_CreateKey(g_env->companyPath, currentUser ? kCurrentUser : kLocalMachine, 0);
    if (!company) {
        *out = -1;
        return out;
    }
    if (name) {
        k->handle = Reg_CreateKey(name, company, 0);
        (*g_env->regCloseKey)(company);
    } else {
        k->handle = company;
    }
    *out = k->handle ? 0 : -1;
    return out;
}

s32 *__fastcall RegKey_Close(RegKey *k, void *, s32 *out) {
    if (k->handle && (*g_env->regCloseKey)(k->handle) != 0) {
        *out = -1;
        return out;
    }
    k->handle = 0;
    *out = 0;
    return out;
}

u32 __cdecl Reg_CreateKey(const char *path, u32 root, u32 isVolatile) {
    u32 key = isVolatile;  // the result is written over this argument (masked when it fails)
    const s32 error = (*g_env->regCreateKeyEx)(root, path, 0, nullptr, isVolatile ? kVolatile : 0, kAllAccess,
                                               nullptr, &key, nullptr);
    return error == 0 ? key : 0;
}

s32 *__fastcall RegKey_Query(RegKey *k, void *, s32 *out, const char *name, void *data, u32 size) {
    if (!name || !k->handle) {
        *out = -1;
        return out;
    }
    u32 type = 0;
    *out = (*g_env->regQueryValueEx)(k->handle, name, nullptr, &type, data, &size) == 0 ? 0 : -1;
    return out;
}

s32 *__fastcall RegKey_GetU32(RegKey *k, void *, s32 *out, const char *name, u32 *value) {
    u32 raw = reinterpret_cast<u32>(name);  // read into the name's argument slot (sic)
    s32 result;
    if (*RegKey_Query(k, nullptr, &result, name, &raw, sizeof raw) >= 0) {
        const s32 type = RegKey_TypeOf(k, nullptr, name);
        if (type == kDword || type == kDwordBigEndian) {
            *value = raw;
            *out = 0;
            return out;
        }
        if (type == kLink) {  // meant for kDwordBigEndian
            *value = bswap(raw);
            *out = 0;
            return out;
        }
    }
    *out = -1;
    return out;
}

s32 *__fastcall RegKey_GetU32Or(RegKey *k, void *, s32 *out, const char *name, u32 *value, u32 fallback) {
    s32 result;
    if (*RegKey_GetU32(k, nullptr, &result, name, value) >= 0) {
        *out = 0;
    } else {
        *value = fallback;
        *out = -1;
    }
    return out;
}

s32 __fastcall RegKey_TypeOf(RegKey *k, void *, const char *name) {
    u32 type = reinterpret_cast<u32>(name);  // the name's argument slot again
    if ((*g_env->regQueryValueEx)(k->handle, name, nullptr, &type, nullptr, nullptr) == 0) return static_cast<s32>(type);
    u32 sub;
    if ((*g_env->regOpenKey)(k->handle, name, &sub) == 0) {
        (*g_env->regCloseKey)(sub);
        return -2;
    }
    return -1;
}

void __fastcall Ptr_Ctor(u32 *p) { *p = 0; }

u8 __cdecl Registry_GameInstalled() {
    RegKey key, unused;  // (the second is only made and closed)
    RegKey_Ctor(&key);
    RegKey_Ctor(&unused);
    s32 result;
    u32 value;
    const bool ok = *RegKey_Open(&key, nullptr, &result, *reinterpret_cast<const char *const *>(0x007a2b08), 0) >= 0 &&
                    *RegKey_GetU32(&key, nullptr, &result, *reinterpret_cast<const char *const *>(0x007a2358), &value) >= 0;
    RegKey_Dtor(&unused);
    RegKey_Dtor(&key);
    return ok ? 1 : 0;
}


s32 *__cdecl Registry_LoadLanguage(s32 *out) {
    RegKey key;
    RegKey_Ctor(&key);
    s32 result;
    u32 value = 0x809;
    if (*RegKey_Open(&key, nullptr, &result, *reinterpret_cast<const char *const *>(0x00f7f6b8), 0) >= 0 &&
        *RegKey_GetU32Or(&key, nullptr, &result, *reinterpret_cast<const char *const *>(0x00f7ee18), &value, 0x809) >= 0) {
        u32 flag;
        switch (value) {
        case 6: flag = 0x100; break;
        case 7: flag = 4; break;
        case 10: flag = 8; break;
        case 0x10: flag = 0x20; break;
        case 0x11: flag = 0x200; break;
        case 0x13: flag = 0x40; break;
        case 0x15: flag = 0x80; break;
        case 0x1d: flag = 0x10; break;
        case 0x404: flag = 0x400; break;
        case 0x409: flag = 0x8000; break;
        case 0x40c: flag = 2; break;
        case 0x40d: flag = 0x4000; break;
        case 0x412: flag = 0x2000; break;
        case 0x41e: flag = 0x1000; break;
        case 0x804: flag = 0x800; break;
        case 0x809: flag = 1; break;
        default: flag = 0x809; break;  // (not a single bit, sic: its lowest, entry 0, is taken)
        }
        *reinterpret_cast<u32 *>(0x0079d868) = flag;
        u32 bit = 0;
        while ((flag >> bit & 1) == 0) ++bit;
        std::strcpy(reinterpret_cast<char *>(0x00f7f230), *reinterpret_cast<const char *const *>(0x00f7ec30 + bit * 0x14));
        Language_SetId(reinterpret_cast<u8 *>(0x0078b530), nullptr, value);
    }
    *out = 0;
    RegKey_Dtor(&key);
    return out;
}
}  // extern "C"
