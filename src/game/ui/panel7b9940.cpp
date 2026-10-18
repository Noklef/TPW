#include "panel7b9940.h"

#include <cstdint>

#include "../core/vwrap.h"
#include "../render/renderer.h"
#include "../render/state.h"
#include "dialogs.h"
#include "guimanager.h"
#include "window.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// The offset k of the vbtable at self + table.
u32 vbase(u8 *self, u32 table, u32 k) { return *reinterpret_cast<const u32 *>(at<u8 *>(self, table) + k); }

u32 *const kSceneFlags = reinterpret_cast<u32 *>((uintptr_t)0x008bd508);
const u32 kOne = 0x3f800000;  // 1.0f
}

extern "C" {

u8 *__fastcall Panel7b9940_Ctor(u8 *self, void *, u32 complete) {
    if (complete) {
        at<u32>(self, 0) = 0x6fdd00;
        at<u32>(self, 0xc) = 0x6fdcf8;
        at<u32>(self, 8) = 0x6fdce8;
        at<u32>(self, vbase(self, 0xc, 4) + 0xc) = 0x6fdcd8;
        const u32 d = vbase(self, 0xc, 4);
        at<u32>(self, d + 8) = d - 0x60;
        at<u32>(self, 0x5c) = 0;
    }
    at<u32>(self, vbase(self, 0, 4)) = 0x6fdcc8;
    const u32 d = vbase(self, 0, 4);
    at<u32>(self, d - 4) = d - 8;
    at<u32>(self, vbase(self, 0, 8) + 0x50) = 2;
    u8 *base = self + vbase(self, 0, 8);
    SceneFlags_Touch(kSceneFlags);
    at<u32>(base, 0x2c) = 0;
    at<u32>(base, 0x30) = 0;
    at<u32>(base, 0x34) = kOne;
    base = self + vbase(self, 0, 8);
    SceneFlags_Touch(kSceneFlags);
    for (u32 off = 4; off <= 0x10; off += 4) at<u32>(base, off) = kOne;
    at<u32>(base, 0x54) = 0xffffffff;
    return self;
}

u32 __cdecl Panel7b9940_Detach() {
    GuiManager_CallSlot4();
    const u8 *const table = global<const u8 *>(0x007b9940);
    const u32 base = 0x007b9940 + *reinterpret_cast<const u32 *>(table + 4);
    return Renderer_RemoveFromSlot(global<u8 *>(0x0087a6d0), nullptr, base, 0x20);
}

void __cdecl Panel7b9940_Close() {
    Windows7c26b0_Message5();
    const u8 *const table = global<const u8 *>(0x007b9940);
    const u32 base = 0x007b9940 + *reinterpret_cast<const u32 *>(table + 4);
    Renderer_RemoveFromSlot(global<u8 *>(0x0087a6d0), nullptr, base, 0x20);
    GuiManager_CallSlot4();
    Global7ca1e0_Release();
    Globals7cc2fc_Release();
    Global7ca1bc_Release();
    Dialog7ca098_Message4IfUp();
    Global7ca1c8_Release();
    Window_SendMessage(global<u8 *>(0x007cb2ac), 4, 0, 0);
    Window_SendMessage(global<u8 *>(0x007cb2b0), 4, 0, 0);
    Table7c26b0_ReleaseIdle();
}
}
