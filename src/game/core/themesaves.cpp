#include "themesaves.h"

#include <cstdint>

#include "string.h"
#include "vcall.h"
#include "../files/handles.h"
#include "../files/userslots.h"
#include "staticobjects.h"

namespace {

AddrCalls g_calls;

const u32 kSaves = (u32)(uintptr_t)&gStatic_f7b560, kThemeLabel = (u32)(uintptr_t)&gNamedObjects_786b68;
constexpr u32 kLoadA = 0x005ac5d0, kSaveA = 0x005ac610, kSaveB = 0x005ac630, kThemeFolder = 0x005c8890;
constexpr u32 kStreamCtor = 0x005f7180, kStreamDtor = 0x005f71e0, kStreamClose = 0x005f71f0, kOpenPath = 0x005f7ed0;

u32 word(const void *p) { return static_cast<u32>(reinterpret_cast<uintptr_t>(p)); }
template <typename T> T ptr(u32 address) { return reinterpret_cast<T>((uintptr_t)address); }

u32 withLabel(u32 call) {
    String label;
    String_CtorCopy(&label, nullptr, ptr<const StringView *>(kThemeLabel));
    WString wide;
    WString_Ctor(&wide);
    WString_AssignFromAnsi(&wide, &label);
    g_calls.Th(call, kSaves, &wide);
    WString_Dtor(&wide);
    String_Dtor(&label);
    return 1;
}

}  // namespace

void ThemeSaves_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

u32 __cdecl ThemeSaves_Save() { return withLabel(kSaveA); }

u32 __cdecl ThemeSaves_Load() { return withLabel(kLoadA); }

void __cdecl ThemeSaves_EnsureFile() {
    StreamBase stream;
    g_calls.Th(kStreamCtor, word(&stream));
    stream.vtable = ptr<void *const *>((u32)(uintptr_t)FileStream_vtable);
    u8 *const slots = UserSlots_Instance();
    WString path;
    g_calls.Th(kThemeFolder, word(slots), &path);
    vcall<void>(&path, 0x28, ptr<const WString *>((u32)(uintptr_t)&gWString_878f18));
    vcall<void>(&path, 0x28, ptr<const WString *>((u32)(uintptr_t)&gWString_878a68));
    vcall<void>(&path, 0x28, ptr<const WString *>((u32)(uintptr_t)&gWString_877f38));
    String narrow;
    String_Ctor(&narrow);
    String_AssignFromWide(&narrow, &path);
    s32 result;
    if (*ptr<const s32 *>(g_calls.Th(kOpenPath, word(&stream), &result, narrow.data, 0x80000000u)) < 0)
        g_calls.Th(kSaveB, kSaves, (u32)(uintptr_t)&gWString_878a68);
    else
        g_calls.Th(kStreamClose, word(&stream));
    String_Dtor(&narrow);
    WString_Dtor(&path);
    g_calls.Th(kStreamDtor, word(&stream));
}
