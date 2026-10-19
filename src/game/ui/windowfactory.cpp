#include "windowfactory.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "radiogroup.h"
#include "window.h"
#include "window705950.h"
#include "window7059f0.h"
#include "window705a20.h"
#include "window705a50.h"
#include "window705af0.h"
#include "window705b20.h"
#include "window705b98.h"
#include "window705bc8.h"
#include "window705bf8.h"
#include "window705c28.h"
#include "window705c58.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

using Ctor = u8 *(__fastcall *)(u8 *, void *, u8 *, u32, u32, u32, u32, u32, u32);

struct Kind {
    u32 size;
    Ctor ctor;
};

const Kind kKinds[13] = {{0x138, Window_Ctor},       {0x15c, Window705a50_Ctor}, {0x15c, Window705b20_Ctor}, {0x158, Window705950_Ctor},
                         {0x168, Window705a20_Ctor}, {0x140, Window705c58_Ctor}, {0x178, Window7059f0_Ctor}, {0x13c, RadioGroup_Ctor},
                         {0x154, Window705c28_Ctor}, {0x150, Window705bf8_Ctor}, {0x168, Window705bc8_Ctor}, {0x158, Window705af0_Ctor},
                         {0x13c, Window705b98_Ctor}};

}  // namespace

extern "C" {

u8 *__fastcall Window_CreateChild(u8 *parent, void *, u32 kind, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom) {
    if (kind - 1 > 12) return nullptr;
    const Kind &k = kKinds[kind - 1];
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(k.size));
    u8 *const window = block ? k.ctor(block, nullptr, parent, flags, id, (u16)left, (u16)top, (u16)right, (u16)bottom) : nullptr;
    if (kind == 1 && (flags & 0x100)) at<u8 *>(window, 0x64) = parent;  // sic: not checked for 0
    if (u8 *last = at<u8 *>(parent, 0x5c)) {
        while (at<u8 *>(last, 0x54)) last = at<u8 *>(last, 0x54);
        at<u8 *>(last, 0x54) = window;
        at<u8 *>(window, 0x58) = last;  // sic: not checked for 0
    } else {
        at<u8 *>(parent, 0x5c) = window;
        at<u8 *>(window, 0x58) = nullptr;  // sic: not checked for 0
    }
    return window;
}
}
