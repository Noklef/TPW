// The handler of the dialog [0x7cc188]: see dialog7cc188handler.h.
#include "dialog7cc188handler.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/language.h"
#include "../core/stdlist.h"
#include "../core/trivial.h"
#include "../core/vcall.h"
#include "../files/charsets.h"
#include "../files/userslots.h"
#include "../online/session.h"
#include "dialog7ca098.h"
#include "dialog7cc188.h"
#include "window.h"
#include "window7059f0.h"
#include "dialog7cc188address.h"
#include "dialog7cc184handler.h"
#include "window7cc8c0handler.h"
#include "../core/textarg.h"
#include "dialog7cc654.h"
#include "../core/staticobjects.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const Dialog7cc188HandlerEnv kGameEnv = {&UserSlots_Instance, &Window7059f0_NthItemData, &Window_SendMessage, &Game_CancelAdvisor, &Language_WideText,
                                         reinterpret_cast<s32(__cdecl *)(u8 *, u16 *, u32, u32, const void *)>(&Charset_FormatWide),
                                         reinterpret_cast<void(__cdecl *)(const u8 *, u32, u32, u32, u32, u32, u32, u32)>((uintptr_t)0x0047f020), &Window_CallDefault,
                                         &List_EraseNthCounted, &Dialog7cc188_FillList};
const Dialog7cc188HandlerEnv *g_env = &kGameEnv;

u16 *recipients() { return reinterpret_cast<u16 *>((uintptr_t)&gDialog7cc654_7cb4d0); }

const char *textOf(const StdString *string) {
    const char *const text = at<const char *>(string, 4);
    return text ? text : reinterpret_cast<const char *>((uintptr_t)0x006fe5b0);
}

// wcsstr (0x67c3d0): an empty part is found, but not in an empty text.
bool contains(const u16 *text, const u16 *part) {
    for (; *text; ++text) {
        const u16 *a = text;
        const u16 *b = part;
        while (*a && *b && *a == *b) ++a, ++b;
        if (!*b) return true;
    }
    return false;
}

// The name of the row whose data nth has left in *data.
const u16 *nameOf(u8 *list, s32 row, u32 *data) {
    g_env->nth(list, nullptr, row, data);
    s32 count = static_cast<s32>(*data);
    u8 *const strings = g_env->source() + 0x100;
    if (count < 0) return at<const u16 *>(reinterpret_cast<const void *>((uintptr_t)Obj_Get3a4(Online_Instance())), 0x9c);
    const char *text = textOf(StdListString_First(strings));
    for (; count; --count) text = textOf(StdListString_Next(strings));
    u16 *const name = reinterpret_cast<u16 *>(gDialog7cc188handler_7cbeb8);
    Crt().swprintf(name, reinterpret_cast<const u16 *>((uintptr_t)&gDialog7cc188handler_75442c), text);
    return name;
}

void askDelete() {
    u8 *const list = Window_FindChild(gDialog7cc188handler_7cc188, nullptr, 0x26);
    const s32 row = at<s32>(list, 0x14c);
    if (row < 0) return;
    u32 data;
    const u16 *const name = nameOf(list, row, &data);
    struct {
        u32 vtable;
        u32 tag;
        u32 number;
        const u16 *text;
    } arg = {(u32)(uintptr_t)TextArgString_vtable, 0x5641524d, 3, name};
    u16 text[0x80];
    g_env->format(global<u8 *>((u32)(uintptr_t)&gLanguage_78b530 + 0x564), text, 0x7f, 0xd7, &arg);
    struct {
        u16 *text;
        u8 colour[4];
        u32 kind;
        u32 data;
    } record;
    record.text = text;
    record.colour[0] = record.colour[1] = record.colour[2] = 0xff;
    record.kind = 4;
    record.data = data;
    g_env->box(reinterpret_cast<const u8 *>(&record), 2, 0, 1, 0x004a7bc0, 0, 0, 0);
}

void addRecipient(u8 *list, u32 row) {
    u32 data;
    const u16 *const name = nameOf(list, static_cast<s32>(row), &data);
    if (contains(recipients(), name)) return;
    u16 entry[0x80];
    Crt().swprintf(entry, reinterpret_cast<const u16 *>(gDialog7cc188address_754438), name);
    if (Crt().wcscmp(recipients(), g_env->wideText(0xd3)) == 0) Crt().wcscpy(recipients(), entry);
    else Crt().wcscat(recipients(), entry);
    vcall<void>(Window_FindChild(gDialog7cc184handler_7cc184, nullptr, 0x5da5), 0x24, recipients());
}

}  // namespace

void Dialog7cc188Handler_SetEnv(const Dialog7cc188HandlerEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __cdecl Dialog7cc188_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x14) {
        gDialog7cc188handler_7cc188 = nullptr;
        g_env->cancel(0x134);
    } else if (message == 0x100) {
        if (a == 0xffffffff) {
            g_env->send(gDialog7cc188handler_7cc188, 4, 0, 0);
            return 0;
        }
        if (a == 0x24) askDelete();
    } else if (message == 0x400) {
        u8 *const list = Window_FindChild(window, nullptr, a);
        if (list && b != 0xffffffff) addRecipient(list, b);
    } else if (message == 0x401) {
        u8 *const list = Window_FindChild(window, nullptr, a);
        if (list) {
            u32 data;
            const void *const text = b == 0xffffffff ? reinterpret_cast<const void *>(gWindow7cc8c0handler_78a374) : nameOf(list, static_cast<s32>(b), &data);
            vcall<void>(Window_FindChild(window, nullptr, 0x25), 0x24, text);
        }
    }
    return g_env->handler(window, nullptr, message, a, b);
}

u32 __cdecl Dialog7cc188_EraseName(u32 index) {
    g_env->erase(g_env->source() + 0x100, nullptr, index);
    g_env->fillList(1);
    return 1;
}
}
