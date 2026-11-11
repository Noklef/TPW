// The dialog [0x7cc188]'s address box: see dialog7cc188address.h.
#include "dialog7cc188address.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/language.h"
#include "../core/stdstring.h"
#include "../core/vcall.h"
#include "../files/charsets.h"
#include "../files/userslots.h"
#include "dialog7cc188.h"
#include "window.h"
#include "window7059f0.h"
#include "dialog7cc188handler.h"
#include "dialog7cc184handler.h"
#include "../core/textarg.h"
#include "dialog7cc654.h"
#include "../core/staticobjects.h"
#include "../core/addrcalls.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T game(u32 address) { return GameAddress<T>(address); }

const Dialog7cc188AddressEnv kGameEnv = {game<void(__fastcall *)(u8 *, void *, const StdString *)>(0x005cd510), &Dialog7cc188_FillList, &Window7059f0_ScrollTo, &Language_WideText,
                                         reinterpret_cast<s32(__cdecl *)(u8 *, u16 *, u32, u32, const void *)>(&Charset_FormatWide),
                                         game<void(__cdecl *)(const u8 *, u32, u32, u32, u32, u32, u32, u32)>(0x0047f020), &Window_CallDefault};
const Dialog7cc188AddressEnv *g_env = &kGameEnv;

u16 *address() { return reinterpret_cast<u16 *>(gDialog7cc188_7cb8d8); }
u16 *recipients() { return reinterpret_cast<u16 *>((uintptr_t)&gDialog7cc654_7cb4d0); }

// wcschr (0x67c2f0): the terminator counts as found.
u16 *find(u16 *text, u16 c) {
    for (;; ++text) {
        if (*text == c) return text;
        if (!*text) return nullptr;
    }
}

// wcsstr (0x67c3d0); part is never empty here.
bool contains(const u16 *text, const u16 *part) {
    for (; *text; ++text) {
        const u16 *a = text;
        const u16 *b = part;
        while (*a && *b && *a == *b) ++a, ++b;
        if (!*b) return true;
    }
    return false;
}

void refuse() {
    struct {
        u32 vtable;
        u32 tag;
        u32 number;
        const u16 *text;
    } arg = {(u32)(uintptr_t)TextArgString_vtable, 0x5641524d, 3, address()};
    u16 text[0x80];
    g_env->format(global<u8 *>((u32)(uintptr_t)&gLanguage_78b530 + 0x564), text, 0x7f, 0xd5, &arg);
    struct {
        u16 *text;
        u8 colour[4];
        u32 kind;
    } record;
    record.text = text;
    record.colour[0] = record.colour[1] = record.colour[2] = 0xff;
    record.kind = 4;
    g_env->box(reinterpret_cast<const u8 *>(&record), 1, 0, 0, 0, 0, 0, 0);
}

}  // namespace

void Dialog7cc188Address_SetEnv(const Dialog7cc188AddressEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl Dialog7cc188_AddAddress() {
    u16 *at = find(address(), '@');
    if (reinterpret_cast<uintptr_t>(at) <= reinterpret_cast<uintptr_t>(address())) {
        refuse();
        return;
    }
    u16 *const dot = find(at, '.');
    at += 1;
    if (reinterpret_cast<uintptr_t>(dot) <= reinterpret_cast<uintptr_t>(at)) {
        refuse();
        return;
    }
    char narrow[0x40];
    Crt().sprintf(narrow, gDialog7cc188_AddAddress_754440, address());
    u8 *const slots = UserSlots_Instance();
    StdString name;
    name.ptr = nullptr;
    name.len = 0;
    name.res = 0;
    u32 length = 0;
    while (narrow[length]) ++length;
    StdString_AssignChars(&name, nullptr, narrow, length);
    g_env->addName(slots + 0x100, nullptr, &name);
    StdString_Dtor(&name, nullptr);
    g_env->fillList(1);
    g_env->scrollTo(Window_FindChild(gDialog7cc188handler_7cc188, nullptr, 0x26), nullptr, -1);
    if (contains(recipients(), address())) return;
    u16 entry[0x80];
    Crt().swprintf(entry, reinterpret_cast<const u16 *>(gDialog7cc188address_754438), address());
    if (Crt().wcscmp(recipients(), g_env->wideText(0xd3)) == 0) Crt().wcscpy(recipients(), entry);
    else Crt().wcscat(recipients(), entry);
    vcall<void>(Window_FindChild(gDialog7cc184handler_7cc184, nullptr, 0x5da5), 0x24, recipients());
}

s32 __cdecl Dialog7cc188_AddressHandler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x1000b && a == 0xd) Dialog7cc188_AddAddress();
    return g_env->handler(window, nullptr, message, a, b);
}
}
