#include "frame.h"

#include <cstdint>

#include "../core/soundset.h"
#include "../core/vcall.h"
#include "../ui/dialogf82884child.h"
#include "../ui/window.h"

namespace {

template <typename T> T &at(u8 *base, u32 offset) { return *reinterpret_cast<T *>(base + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>(static_cast<uintptr_t>(address)); }

void Delete(u8 *&object) {
    if (object) vcall<void>(object, 0, 1u);   // (the deleting destructor)
    object = nullptr;
}

const FrameEnv kGameEnv = {&DialogF82884_ShowChild, &Window_CallDefault};
const FrameEnv *g_env = &kGameEnv;

}  // namespace

void Frame_SetEnv(const FrameEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __fastcall Frame_HandleMessage(u8 *frame, void *, u8 *window, u32 message, u32 wparam, u32 lparam) {
    const bool has_frame = global<u32>(0x00f82884) != 0;
    switch (message) {
        case 0x11:
            if (wparam == 1) g_env->showChild();
            break;
        case 0x14:
            if (has_frame) at<u32>(frame, 4) = 0;
            break;
        case 0x22:
            at<u32>(frame, 0x1c) = 0;
            break;
        case 0x10003:
            at<u32>(frame, 0x20) = wparam >> 16;
            at<u32>(frame, 0x24) = wparam & 0xffff;
            break;
        case 0x10004:
            if (wparam < 3) at<u8>(frame, 0x1c) &= static_cast<u8>(~(1u << wparam));
            break;
        case 0x10005:
            if (wparam < 3) at<u8>(frame, 0x1c) |= static_cast<u8>(1u << wparam);
            break;
    }
    if (global<u32>(0x00f82884)) {
        for (u8 *node = at<u8 *>(at<u8 *>(frame, 0x18), 4); node;) {
            u8 *const object = at<u8 *>(node, 0);
            node = at<u8 *>(node, 4);
            vcall<void>(object, 0x14, message, wparam, lparam);
        }
    }
    return g_env->callDefault(window, nullptr, message, wparam, lparam);
}

s32 __cdecl Frame_WindowProc(u8 *window, u32 message, u32 wparam, u32 lparam) {
    u8 *const frame = global<u8 *>(0x00f82884);
    return frame ? Frame_HandleMessage(frame, nullptr, window, message, wparam, lparam) : 0;
}

u32 __fastcall Class7029f0_Close(u8 *frame) {
    for (u8 *node = at<u8 *>(at<u8 *>(frame, 0x18), 4); node;) {
        u8 *const object = at<u8 *>(node, 0);
        node = at<u8 *>(node, 4);
        vcall<void>(object, 0x10);
    }
    Delete(global<u8 *>(0x00f83784));
    Delete(global<u8 *>(0x00f83778));
    Delete(global<u8 *>(0x00f83774));
    Delete(global<u8 *>(0x00f85614));
    Delete(at<u8 *>(frame, 0x10));
    Sound_ReleaseLobbySfx();
    Window_SendMessage(at<u8 *>(frame, 4), 4, 0, 0);
    return at<u32>(frame, 0x14);
}
}
