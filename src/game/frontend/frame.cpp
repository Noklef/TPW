#include "frame.h"

#include <cstdint>

#include "../core/soundset.h"
#include "../core/vcall.h"
#include "../ui/window.h"

namespace {

template <typename T> T &at(u8 *base, u32 offset) { return *reinterpret_cast<T *>(base + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>(static_cast<uintptr_t>(address)); }

void Delete(u8 *&object) {
    if (object) vcall<void>(object, 0, 1u);   // (the deleting destructor)
    object = nullptr;
}

}  // namespace

extern "C" {

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
