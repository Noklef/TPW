// The company registry key: see regkey.h.
#include "regkey.h"

#include <cstdint>

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

}  // extern "C"
