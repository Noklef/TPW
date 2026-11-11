#include "thememenu.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/globalsave.h"
#include "../core/initializers.h"
#include "../core/soundplay.h"
#include "../core/soundset.h"
#include "../core/themenames.h"
#include "../core/vcall.h"
#include "../files/file.h"
#include "../files/userslots.h"
#include "../model/holder.h"
#include "../online/itemlist.h"
#include "../online/session.h"
#include "../render/lightsource.h"
#include "../render/state.h"
#include "../ui/dialog7cc4b4open.h"
#include "../ui/dialog7cc4b4update.h"
#include "lobbyscript.h"
#include "frame.h"
#include "../online/chatlists.h"
#include "../online/chatcommand.h"
#include "../coaster/arreplayaction.h"
#include "../core/staticobjects.h"
#include "../core/imports.h"
#include "../core/addrcalls.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const ThemeMenuEnv kGameEnv = {&UserSlots_Ctor, &GlobalSave_Construct, &GlobalSave_ThemeGet, &ThemeRecord_Name, &ThemeRecord_Suffix,
                               GameAddress<void(__fastcall *)(u8 *, void *, WString *, WString *, WString *)>(0x00409b10),
                               GameAddress<void(__fastcall *)(u8 *, void *, const u16 *, const u16 *)>(0x005d8470),
                               &File_Delete, &Online_Construct, &Owner2c_ValueByKey,
                               reinterpret_cast<void(__fastcall *)(u8 *, void *, const char *, u32, s32, u32, u32)>(&LobbyScript_Load),
                               &HolderUser_Acquire, &Render_SetAmbient, &LightSlot_Set, &Online_Instance, &Online_ResetConnections, &Dialog7cc4b4_Open, &Dialog7cc4b4_ShowTheme, &Time_GetMs, &ThemeMenu_Refresh, &Sound_PlayAt, &Sound_ApplySettings};
const ThemeMenuEnv *g_env = &kGameEnv;

u8 *const kNames = reinterpret_cast<u8 *>((uintptr_t)&gNamedObjects_786b68);

u8 *Singleton(u32 address, u32 size, u8 *(__fastcall *ctor)(u8 *)) {
    u8 *&object = global<u8 *>(address);
    if (!object) {
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(size));
        object = block ? ctor(block) : nullptr;
    }
    return object;
}

u8 *Session() { return Singleton(reinterpret_cast<uintptr_t>(&gChatcommand_f7d88c), 0x404, g_env->online); }

void WideFrom(WString *wide, const char *text) {
    String narrow;
    String_CtorStr(&narrow, nullptr, text);
    WString_AssignFromAnsi(wide, &narrow);
    String_Dtor(&narrow);
}

void ClearFonts() {
    u8 *&end = global<u8 *>(0x00789c20);
    u8 *const begin = global<u8 *>(0x00789c1c);
    const auto remove = *reinterpret_cast<u32(__stdcall **)(const char *)>((uintptr_t)IMPORT(Gdi32_RemoveFontResourceA));
    for (u8 *font = begin; font != end; font += 0x14) {
        remove(at<const char *>(font, 8));
        s32 result;
        g_env->deleteFile(&result, at<const char *>(font, 8));
    }
    u8 *const last = end;
    for (u8 *font = begin; font != last; font += 0x14)
        vcall<void>(font, 0xc, 0u);
    end = begin;
}

void RefreshItem(u8 *item) {
    WString a, b;
    WString_Ctor(&a);
    WString_Ctor(&b);
    if (at<s32>(Singleton(reinterpret_cast<uintptr_t>(&gChatlists_f81ebc), 0x110, g_env->userSlots), 0x60) == -1) {
        WString key;
        WString_Ctor(&key);
        WideFrom(&key, at<const char *>(item, 0x30));
        g_env->localise(kNames, nullptr, &key, &a, &b);
        WString_Dtor(&key);
    } else {
        String name;
        String_CtorStr(&name, nullptr, at<const char *>(item, 0x30));
        u8 *const info = Singleton(reinterpret_cast<uintptr_t>(&gArreplayaction_f7c58c), 0x50, g_env->globalSave);
        u8 *const record = g_env->themeGet(info, nullptr, reinterpret_cast<const u8 *>(&name));
        String_Dtor(&name);
        u16 text[0x100];
        g_env->name(record, nullptr, text, 0x100);
        vcall<void>(&a, 0x14, text);
        g_env->suffix(record, nullptr, text, 0x100);
        vcall<void>(&b, 0x14, text);
    }
    g_env->setTexts(at<u8 *>(item, 4), nullptr, a.data, b.data);
    g_env->setTexts(at<u8 *>(item, 8), nullptr, a.data, b.data);
    WString_Dtor(&b);
    WString_Dtor(&a);
}

void LoadItem(u8 *self, u8 *item, s32 index) {
    WString key, a, b;
    WString_Ctor(&key);
    WideFrom(&key, reinterpret_cast<const char *>((uintptr_t)g_env->valueByKey(at<u8 *>(Session(), 0xf8), nullptr, at<u32>(item, 0x4c))));
    WString_Ctor(&a);
    WString_Ctor(&b);
    g_env->localise(kNames, nullptr, &key, &a, &b);
    const u32 value = g_env->valueByKey(at<u8 *>(Session(), 0xf8), nullptr, at<u32>(item, 0x4c));
    g_env->load(self, nullptr, reinterpret_cast<const char *>((uintptr_t)value), reinterpret_cast<uintptr_t>(item), index,
                static_cast<u32>(reinterpret_cast<uintptr_t>(a.data)), static_cast<u32>(reinterpret_cast<uintptr_t>(b.data)));
    WString_Dtor(&b);
    WString_Dtor(&a);
    WString_Dtor(&key);
}

}  // namespace

void ThemeMenu_SetEnv(const ThemeMenuEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall ThemeMenu_Refresh(u8 *self) {
    u8 *node = at<u8 *>(at<u8 *>(self, 8), 4);
    while (node) {
        u8 *const item = at<u8 *>(node, 0);
        node = at<u8 *>(node, 4);
        RefreshItem(item);
    }
    ClearFonts();
}

void __fastcall ThemeMenu_Load(u8 *self, void *, u8 *list) {
    s32 index = 0;
    u8 *node = at<u8 *>(list, 4);
    while (node) {
        u8 *const item = at<u8 *>(node, 0);
        node = at<u8 *>(node, 4);
        LoadItem(self, item, index++);
    }
    at<u32>(self, 0xe4) = 1;
    at<u32>(self, 0xc) = at<u32>(at<u8 *>(at<u8 *>(self, 8), 4), 0);
    ClearFonts();
}

void __fastcall ThemeMenu_Enter(u8 *self) {
    if (vcall<s32>(self, 0x24)) return;
    if (!vcall<s32>(self, 0x24)) {
        at<u32>(self, 4) = 1;
        g_env->holder(gFrame_f83774);
        g_env->ambient(gFrame_f83778, nullptr, 1, 0x444444);
        const u32 colour = 0xeeeeee;
        g_env->light(gFrame_f83778, nullptr, 1, &colour, nullptr, 1);
        Fn_5e1bb0(gFrame_f83778, nullptr, 0x40000000, 0x41000000, 1);
        at<float>(gFrame_f83778, 4) = at<float>(at<u8 *>(global<u8 *>(0x00f82884), 0x10), 4);
        Render_FogOn(gFrame_f83778);
        Render_SetFog(gFrame_f83778, nullptr, 0xff44ddff, 0x42480000, 0x437a0000);
        at<u32>(self, 0x14) = 0;
        at<u32>(self, 0xc) = 0;
        at<u32>(self, 0x10) = 0;
        g_env->reset(g_env->instance());
    }
    g_env->dialogOpen();
    vcall<void>(self, 0x28);
    u8 *node = global<u8 *>((u32)(uintptr_t)&gNamedObjects_786b68 + 0x28);
    while (node) {
        u8 *const name = at<u8 *>(node, 0);
        node = at<u8 *>(node, 4);
        WString key, a, b;
        WString_Ctor(&key);
        WString_AssignFromAnsi(&key, reinterpret_cast<const String *>(name));
        WString_Ctor(&a);
        WString_Ctor(&b);
        g_env->localise(kNames, nullptr, &key, &a, &b);
        g_env->load(self, nullptr, reinterpret_cast<const String *>(name)->data, 0, -1, static_cast<u32>(reinterpret_cast<uintptr_t>(a.data)), static_cast<u32>(reinterpret_cast<uintptr_t>(b.data)));
        WString_Dtor(&b);
        WString_Dtor(&a);
        WString_Dtor(&key);
    }
    ClearFonts();
    at<u32>(self, 0xc) = at<u32>(at<u8 *>(at<u8 *>(self, 8), 4), 0);
    node = at<u8 *>(at<u8 *>(self, 8), 4);
    while (node) {
        u8 *const item = at<u8 *>(node, 0);
        node = at<u8 *>(node, 4);
        if (!CRT_stricmp(at<const char *>(item, 0x30), global<const char *>((u32)(uintptr_t)&gString_f85480 + 0x8))) at<u8 *>(self, 0xc) = item;
    }
    g_env->dialogShow();
    at<u32>(self, 0xe4) = g_env->time();
    g_env->refresh(self);
    u8 *const chosen = at<u8 *>(self, 0xc);
    if (chosen) {
        at<u32>(chosen, 0x1c) = g_env->play(at<u32>(chosen, 0x1c), at<u32>(gSoundCategories.lobbyPairs, at<u32>(chosen, 0x18) * 8 + 4), 1, 0, 0, 0);
        at<u32>(chosen, 0x20) = g_env->play(at<u32>(chosen, 0x20), at<u32>(gSoundCategories.lobbyPairs, at<u32>(chosen, 0x18) * 8), 2, 0, 0, 0);
        g_env->applySound();
    }
}
}
