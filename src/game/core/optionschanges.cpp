#include "optionschanges.h"

#include <cstdint>

#include "string.h"
#include "vcall.h"
#include "../files/userslots.h"

namespace {

AddrCalls g_calls;

constexpr u32 kSaves = 0x00f7b560;

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T ptr(u32 address) { return reinterpret_cast<T>((uintptr_t)address); }
u32 word(const void *p) { return static_cast<u32>(reinterpret_cast<uintptr_t>(p)); }

// What Dialog_Box reads: the text and its buttons' setting.
struct Box {
    const u16 *text;
    u8 colour[3];
    u32 kind;
};

// The settings' file written: the tag and the display's settings.
void writeFile(u8 *options, const char *path) {
    s32 status, handle;
    if (*ptr<const s32 *>(g_calls.Cd(0x005f5ad0, &status, &handle, path, 0xc0000010u)) < 0) return;
    const s32 stream = handle;
    u32 tag = 1;
    s32 written = 0;
    s32 result;
    const bool whole = *ptr<const s32 *>(g_calls.Cd(0x005f5e50, &result, stream, &tag, 4u, &written)) >= 0 && written == 4;
    if (whole) g_calls.Th(0x004242c0, word(options), stream, 1u, tag);
    g_calls.Cd(0x005f5b80, &result, stream);
}

}  // namespace

void OptionsChanges_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

void __fastcall Options_ApplyChanges(u8 *options) {
    bool keepOpen = false;
    g_calls.Th(0x00423740, word(options));
    const s32 level = at<s32>(options, 0x94);
    if (level == 1) {
        const u32 flags = at<u32>(options, 0x98);
        if (!global<u32>(0x00f82884)) g_calls.Th(0x005ac630, kSaves, 0x0078dcf0u);
        if (flags & 1) g_calls.Cd(0x0051bc70);
        g_calls.Cd(0x0051b920, at<u32>(options, 0x30));
        if (!global<u32>(0x00f82884)) g_calls.Th(0x005ac5f0, kSaves, 0x0078dcf0u);
    } else if (level == 2) {
        u16 text[0x100];
        g_calls.Cd(0x006acd60, global<u32>(0x0078ba94), text, 0xffu, 0x193u);
        Box box;
        box.text = text;
        box.colour[0] = box.colour[1] = box.colour[2] = 0xff;
        box.kind = 4;
        g_calls.Cd(0x0047f020, &box, 2u, 0x00424c10u, 1u, 0x00424bb0u, 0u, 0u, 0u);
        keepOpen = true;
    }
    g_calls.Th(0x00423dd0, word(options));
    WString path;
    WString_Ctor(&path);
    String narrow;
    String_Ctor(&narrow);
    {
        u8 *const slots = UserSlots_Instance();
        WString folder;
        const u32 made = g_calls.Th(0x005c7bd0, word(slots), &folder);
        vcall<void>(&path, 0x18, ptr<const WString *>(made));
        WString_Dtor(&folder);
    }
    vcall<void>(&path, 0x28, ptr<const WString *>(0x0078e6f8));
    vcall<void>(&path, 0x28, ptr<const WString *>(0x0078e230));
    String_AssignFromWide(&narrow, &path);
    writeFile(options, narrow.data);
    if (at<u8>(options, 0x3a)) g_calls.Cd(0x0042aff0);
    g_calls.Cd(0x0051bd70);
    if (!keepOpen) g_calls.Cd(0x004a4580);
    String_Dtor(&narrow);
    WString_Dtor(&path);
}

u32 __cdecl Options_ConfirmSave() {
    g_calls.Cd(0x005508b0, 2u);
    const u32 session = global<u32>(0x00f82884);
    global<u32>(0x00786b80) = 1;
    if (session) {
        at<u32>(ptr<void *>(session), 0x14) = 3;
    } else {
        g_calls.Th(0x005ac610, kSaves, 0x0078da20u);
        global<u32>(0x00786b80) = 1;
    }
    g_calls.Cd(0x004a4580);
    return 1;
}

void __cdecl Options_LoadStartup() {
    global<u32>(0x007a1a14) = 0;
    global<u32>(0x007a1a8c) = 0x000c0e15;
    String path;
    String_CtorStr(&path, nullptr, ptr<const char *>(0x0074ce84));
    const bool loaded = g_calls.Th(0x00424a60, 0x0078d8d8u, &path) == 1;
    String_Dtor(&path);
    if (loaded) global<u32>(0x0078d968) = 1;
    if (global<u32>(0x0078d968)) {
        global<u32>(0x007a1a8c) = global<u32>(0x0078d950);
        global<u32>(0x0079d86c) = global<u32>(0x0078d954);
        g_calls.Cd(0x00409790, global<u32>(0x0078d95c));
        global<u32>(0x0079d9c8) = global<u32>(0x0078d964);
    }
}
