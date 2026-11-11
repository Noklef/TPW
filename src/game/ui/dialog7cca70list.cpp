// The dialog [0x7cca70]'s list filled from a session's answer: see dialog7cca70list.h.
#include "dialog7cca70list.h"

#include <cstdint>

#include "../core/buffers.h"
#include "../core/gameheap.h"
#include "../core/globalsave.h"
#include "../core/namedobjects.h"
#include "../core/wtraits.h"
#include "../frontend/scenequeue.h"
#include "../online/itemlist.h"
#include "../online/session.h"
#include "window.h"
#include "window7059f0.h"
#include "../online/chatcommand.h"
#include "../core/staticobjects.h"
#include "../core/addrcalls.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T at(const u8 *object, u32 offset) { return *reinterpret_cast<const T *>(object + offset); }

const Dialog7cca70ListEnv kGameEnv = {
    Window7059f0_Clear,
    Window7059f0_AddItem,
    Window7059f0_RefreshRows,
    GameAddress<void(__fastcall *)(u8 *, void *, StdWString *, u32)>(0x005d3230),
    GameAddress<u8 *(__fastcall *)(u8 *)>(0x005b50b0),
    Owner2c_ValueByKey,
    NamedObjects_SetThemeLabelAnsi,
    Online_Instance,
    GameAddress<void(__fastcall *)(u8 *)>(0x005b6070),
    reinterpret_cast<void(__fastcall *)(u8 *)>(&Class702da0_Clear),
    Window_SendMessage,
};
const Dialog7cca70ListEnv *g_env = &kGameEnv;

}  // namespace

void Dialog7cca70List_SetEnv(const Dialog7cca70ListEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl Dialog7cca70_FillList(const u8 *answer) {
    u8 *const list = Window_FindChild(global<u8 *>(0x007cca70), nullptr, 0x14beb);
    g_env->clear(list);
    const u8 *node = at<const u8 *>(answer, 4);
    u32 countdown = at<u32>(answer, 0xc);
    StdWString text;
    CountedBuf_Reset(reinterpret_cast<u8 *>(&text), nullptr, 0);
    while (node) {
        u8 *const item = at<u8 *>(node, 0);
        node = at<const u8 *>(node, 4);
        const u16 *const wide = at<const u16 *>(item, 0x14);
        u32 length = 0;
        while (wide[length]) ++length;
        if (StdWString_Grow(&text, nullptr, length, 1)) {
            WTraits_Copy(text.ptr, wide, length);
            ImeWString_SetLength(reinterpret_cast<u8 *>(&text), nullptr, length);
        }
        if (GlobalSave_Byte25(GlobalSave_Instance())) g_env->censor(GlobalSave_Instance(), nullptr, &text, 1);
        u32 row[5];
        row[0] = text.ptr ? (u32)(uintptr_t)text.ptr : 0x006fd6c4u;
        WString label;
        WString_Ctor(&label);
        const u32 key = at<u32>(item, 0x4c);
        u8 *&holder = gChatcommand_f7d88c;
        if (!holder) {
            u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x404));
            holder = block ? g_env->holderCtor(block) : nullptr;
        }
        String theme;
        String_CtorStr(&theme, nullptr, reinterpret_cast<const char *>((uintptr_t)g_env->valueByKey(at<u8 *>(holder, 0xf8), nullptr, key)));
        g_env->setTheme(&global<u8>((u32)(uintptr_t)&gNamedObjects_786b68), nullptr, &theme, reinterpret_cast<u8 *>(&label));
        String_Dtor(&theme);
        row[1] = (u32)(uintptr_t)label.data;
        row[2] = at<u32>(item, 0xb8);
        row[3] = at<u32>(item, 0x50);
        row[4] = at<u32>(item, 0x54);
        --countdown;
        g_env->addItem(list, nullptr, row, (u32)(uintptr_t)item, -1, countdown == 0);
        WString_Dtor(&label);
    }
    g_env->refresh(list);
    if (text.ptr) {  // _Tidy(true), inlined
        u8 &refs = *(reinterpret_cast<u8 *>(text.ptr) - 1);
        if (refs != 0 && refs != 0xff) {
            --refs;
        } else {
            GameHeap_Free(reinterpret_cast<u8 *>(text.ptr) - 2);
        }
    }
}

void __cdecl Dialog7cca70_ListCallback(u32 error, u32, const u8 *answer) {
    if (!error) {
        Dialog7cca70_FillList(answer);
        return;
    }
    g_env->closeSession(g_env->online());
    g_env->clearScene(at<u8 *>(global<u8 *>(0x00f85614), 0x14));
    g_env->send(global<u8 *>(0x007cca70), 4, 0, 0);
}
}
