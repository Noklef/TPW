#include "guidepth.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/vcall.h"
#include "guirect.h"
#include "window.h"
#include "window7059f0.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
}

extern "C" {

s32 __cdecl Gui_TopDepthIn(u8 *window, u8 *shape) {
    s32 &top = global<s32>(0x00faa594);
    u32 &flags = global<u32>(0x00faa570);
    s32 &depth = global<s32>(0x00faa5c8);
    if (window == global<u8 *>(0x00faa5d8)) top = (s16)Fn_659730(window);
    for (u8 *child = reinterpret_cast<u8 *>((uintptr_t)Fn_65d547(window, nullptr, 0)); child;
         child = reinterpret_cast<u8 *>((uintptr_t)Fn_65d56b(child))) {
        flags = Fn_65d57c(child);
        if (!(flags & 1) || (flags & 2)) continue;
        depth = vcall<u32>(child, 0xc) == 7 ? (s16)Window7059f0_Item148Depth(child) : (s16)Fn_659730(child);
        if (depth > top) {
            if (shape) {
                u8 rect[12];
                GuiRect_Ctor(rect);
                Window_GetRect(child, nullptr, rect, 1);
                if ((u16)GuiRect_Overlap(rect, nullptr, shape)) top = depth + 1;
                GuiRect_Dtor(rect);
            } else {
                top = depth + 1;
            }
        }
        Gui_TopDepthIn(child, shape);
    }
    return top;
}

s32 __cdecl Gui_TopDepth(u8 *window, u8 *shape) {
    if (!window) window = global<u8 *>(0x00faa5d8);
    return window ? Gui_TopDepthIn(window, shape) : -1;
}
}
