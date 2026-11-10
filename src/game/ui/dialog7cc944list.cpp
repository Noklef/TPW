#include "dialog7cc944list.h"

#include <cstdint>

#include "../core/buffers.h"
#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/globalsave.h"
#include "../core/namedobjects.h"
#include "../core/wtraits.h"
#include "../files/charsets.h"
#include "../online/itemlist.h"
#include "../online/chat.h"
#include "window.h"
#include "window7059f0.h"
#include "window705a50.h"
#include "../online/chatcommand.h"

extern "C" const u16 *__cdecl Language_WideText(u32 id);

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const Dialog7cc944ListEnv kGameEnv = {
    &Window7059f0_Clear,
    &Language_WideText,
    &Window7059f0_AddItem,
    &GlobalSave_Construct,
    &GlobalSave_Byte25,
    reinterpret_cast<void(__fastcall *)(u8 *, void *, StdWString *, u32)>((uintptr_t)0x005d3230),
    reinterpret_cast<u8 *(__fastcall *)(u8 *)>((uintptr_t)0x005b50b0),
    &Owner2c_ValueByKey,
    &NamedObjects_SetThemeLabelAnsi,
    &Owner2c_FindByKey,
    &Window705a50_SetState,
};
const Dialog7cc944ListEnv *g_env = &kGameEnv;

u8 *Made(u32 holder, u32 size, u8 *(__fastcall *ctor)(u8 *)) {
    if (!global<u8 *>(holder)) {
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(size));
        global<u8 *>(holder) = block ? ctor(block) : nullptr;
    }
    return global<u8 *>(holder);
}
u8 *Save() { return Made(0x00f7c58c, 0x50, g_env->saveCtor); }
u8 *Online() { return Made(reinterpret_cast<uintptr_t>(&gChatcommand_f7d88c), 0x404, g_env->onlineCtor); }

}  // namespace

extern "C" {

void Dialog7cc944List_SetEnv(const Dialog7cc944ListEnv *env) { g_env = env ? env : &kGameEnv; }

void __cdecl Dialog7cc944_FillList(const u8 *reply) {
    const Dialog7cc944ListEnv *const E = g_env;
    if (!global<u8 *>(0x007cc944)) return;
    u8 *const list = Window_FindChild(global<u8 *>(0x007cc944), nullptr, 0xe05b);
    E->clear(list);
    u32 count = at<u32>(reply, 0xc);
    const u8 *node = at<const u8 *>(reply, 4);
    StdWString text;
    text.allocator = static_cast<u8>(count);
    CountedBuf_Reset(reinterpret_cast<u8 *>(&text), nullptr, 0);
    u32 row[6];
    if (!count) {
        row[0] = reinterpret_cast<u32>(E->wideText(0x133));
        row[1] = 0x0078a374;
        row[2] = 0x0078a374;
        row[3] = 0;
        row[4] = 0;
        row[5] = 0;
        E->addItem(list, nullptr, row, 0, -1, 1);
    } else {
        while (node) {
            u8 *const record = at<u8 *>(node, 0);
            node = at<const u8 *>(node, 4);
            const u16 *const name = at<const u16 *>(record, 0x14);
            const u32 length = Crt().wcslen(name);
            if (StdWString_Grow(&text, nullptr, length, 1)) {   // (0x4051d0, assign)
                WTraits_Copy(text.ptr, name, length);
                ImeWString_SetLength(reinterpret_cast<u8 *>(&text), nullptr, length);
            }
            if (E->saveByte25(Save())) E->filterText(Save(), nullptr, &text, 1);
            row[0] = text.ptr ? reinterpret_cast<u32>(text.ptr) : 0x006fd6c4;
            WString label;
            WString_Ctor(&label);
            {
                const u32 key = at<u32>(record, 0x4c);
                const u32 value = E->valueByKey(at<u8 *>(Online(), 0xf8), nullptr, key);
                String theme;
                String_CtorStr(&theme, nullptr, reinterpret_cast<const char *>((uintptr_t)value));
                E->themeLabel(reinterpret_cast<u8 *>((uintptr_t)0x00786b68), nullptr, &theme, reinterpret_cast<u8 *>(&label));
                String_Dtor(&theme);
            }
            row[1] = at<u32>(&label, 8);
            const u32 key = at<u32>(record, 0x48);
            row[2] = at<u32>(E->findByKey(at<u8 *>(Online(), 0xe0), nullptr, key), 0x14);
            row[3] = at<u32>(record, 0xb8);
            row[4] = at<u32>(record, 0x50);
            row[5] = at<u32>(record, 0x54);
            --count;
            E->addItem(list, nullptr, row, reinterpret_cast<u32>(record), -1, count == 0);
            WString_Dtor(&label);
        }
    }
    E->setState(Window_FindChild(global<u8 *>(0x007cc944), nullptr, 0xe059), nullptr, 0, 1);
    if (text.ptr) {
        u8 *const refs = reinterpret_cast<u8 *>(text.ptr) - 1;
        if (*refs == 0 || *refs == 0xff)
            GameHeap_Free(reinterpret_cast<u8 *>(text.ptr) - 2);
        else
            --*refs;
    }
}
}
