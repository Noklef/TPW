#include "radiogroup.h"

#include <cstdint>

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
}
