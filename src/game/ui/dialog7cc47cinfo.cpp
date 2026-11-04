#include "dialog7cc47cinfo.h"

#include <cstdint>

#include "../core/buffers.h"
#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/globalsave.h"
#include "../core/vcall.h"
#include "../online/chatlists.h"
#include "window.h"

namespace {

s32 __cdecl Format(u16 *to, const u16 *format, s32 value) { return Crt().swprintf(to, format, value); }

const Dialog7cc47cInfoEnv kGameEnv = {&Window_SetShown, &Format, reinterpret_cast<decltype(Dialog7cc47cInfoEnv::censor)>((uintptr_t)0x005d3230)};
const Dialog7cc47cInfoEnv *g_env = &kGameEnv;

u8 *dialog() { return *reinterpret_cast<u8 **>((uintptr_t)0x007cc47c); }
u8 *child(u32 id) { return Window_FindChild(dialog(), nullptr, id); }

void showBoth(u32 shown) {
    g_env->setShown(child(0x49da), nullptr, shown);
    g_env->setShown(child(0x49d9), nullptr, shown);
}

}  // namespace

void Dialog7cc47cInfo_SetEnv(const Dialog7cc47cInfoEnv *env) { g_env = env ? env : &kGameEnv; }

void __cdecl Dialog7cc47cInfo_Show(const u8 *item) {
    StdWString text;
    text.allocator = 0;  // (the argument's low byte in the original; never read)
    CountedBuf_Reset(reinterpret_cast<u8 *>(&text), nullptr, 0);
    if (!dialog()) return;
    const u8 *const holder = item ? *reinterpret_cast<const u8 *const *>(item + 0x70) : nullptr;
    if (holder) {
        showBoth(1);
        u16 *const number = reinterpret_cast<u16 *>((uintptr_t)0x007ba058);
        g_env->format(number, reinterpret_cast<const u16 *>((uintptr_t)0x0074f9d8), *reinterpret_cast<const s32 *>(item + 0x24) + 1);
        vcall<void>(child(0x49da), 0x24, number);
        const u16 *const chars = *reinterpret_cast<const u16 *const *>(holder + 0x14);
        const u32 n = CRT_wcslen(chars);
        if (StdWString_Grow(&text, nullptr, n, 1)) {
            for (u32 i = 0; i < n; ++i) text.ptr[i] = chars[i];
            text.len = n;
            text.ptr[n] = 0;
        }
        if (GlobalSave_Byte25(GlobalSave_Instance())) g_env->censor(GlobalSave_Instance(), nullptr, &text, 1);
        vcall<void>(child(0x49d9), 0x24, text.ptr ? text.ptr : reinterpret_cast<const u16 *>((uintptr_t)0x006fd6c4));
    } else {
        showBoth(0);
    }
    if (text.ptr) {  // _Tidy(true), inlined
        u8 &refs = *reinterpret_cast<u8 *>(reinterpret_cast<uintptr_t>(text.ptr) - 1);
        if (refs != 0 && refs != 0xff)
            --refs;
        else
            GameHeap_Free(reinterpret_cast<u8 *>(text.ptr) - 2);
    }
}
