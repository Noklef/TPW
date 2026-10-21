#include "radiogroup.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/vcall.h"
#include "window.h"
#include "window705a50.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
}

extern "C" {

u8 *__fastcall RadioGroup_Ctor(u8 *group, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom) {
    Window_Ctor(group, nullptr, parent, flags, id, left, top, right, bottom);
    at<u32>(group, 0) = 0x705ac0;
    at<u32>(group, 0x138) = 0xffffffffu;
    at<u32>(group, 0x114) = 0x66a31c;
    at<u32>(group, 0x118) = at<u32>(group, 0x114);
    return group;
}

void __fastcall RadioGroup_SetSelectedState(u8 *group, void *, u8 state) {
    if (u8 *const child = Window_FindChild(group, nullptr, at<u32>(group, 0x138))) Window705a50_SetState(child, nullptr, state, 1);
}

u16 __fastcall RadioGroup_Select(u8 *group, void *, u32 id) {
    if (id == at<u32>(group, 0x138)) return 0xffff;
    if (id == 0xffffffffu) {
        RadioGroup_SetSelectedState(group, nullptr, 0);
        at<u32>(group, 0x138) = 0xffffffffu;
        return 0xffff;
    }
    if (!Window_FindChild(group, nullptr, id)) return 0xffff;
    const u32 before = at<u32>(group, 0x138);
    RadioGroup_SetSelectedState(group, nullptr, 0);
    at<u32>(group, 0x138) = id;
    RadioGroup_SetSelectedState(group, nullptr, 1);
    return (u16)before;
}

u32 __cdecl RadioGroup_Proc(u8 *group, u32 message, u32 a, u32 b) {
    if (message == 0x15) {
        if ((Fn_65d57c(group) & 0x10) && Fn_66a281(group) == 0xffffffffu) {
            u8 *const first = reinterpret_cast<u8 *>((uintptr_t)Fn_65d547(group, nullptr, 0));
            if (first && vcall<u32>(first, 0xc) == 2) RadioGroup_Select(group, nullptr, Fn_65d7bc(first));
        }
    } else if (message == 0x100) {
        u8 *const child = Window_FindChild(group, nullptr, a);
        if (child && group) {
            if (b == 1) {
                if (Fn_66a281(group) != 0xffffffffu) {
                    Window_PostMessage(reinterpret_cast<u8 *>((uintptr_t)Fn_65d55a(group)), 0x101, Fn_66a281(group), 0);
                }
                RadioGroup_Select(group, nullptr, a);
                Window_PostMessage(reinterpret_cast<u8 *>((uintptr_t)Fn_65d55a(group)), 0x101, a, 1);
            } else if (!(Fn_65d57c(group) & 0x10)) {
                RadioGroup_Select(group, nullptr, 0xffffffffu);
                Window_PostMessage(reinterpret_cast<u8 *>((uintptr_t)Fn_65d55a(group)), 0x101, a, 0);
            } else {
                RadioGroup_Select(group, nullptr, a);
                Window705a50_SetState(child, nullptr, 1, 1);
            }
        }
    }
    return Window_DefaultProc(group, message, a, b);
}
}
