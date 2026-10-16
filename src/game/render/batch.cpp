#include "batch.h"

#include <cstdint>

#include "drawbuffer.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u8 *__fastcall Batch_Begin(u8 *object, void *, u32 mode, u32 texture, u32 kind, u32 flags, u32 extra) {
    if ((mode & 2) && (at<u32>(object, 0xc) & flags) && !(global<u32>(0x008bd508) & 0x4000000))
        mode |= 4;
    u8 *buffer = global<u8 *>(0x008bcbc4);
    u8 *batch = DrawBuffer_Begin(buffer, nullptr, mode);
    at<u8 *>(object, 0x20) = batch;
    if (!batch)
        return nullptr;
    at<u32>(batch, 4) = flags;
    at<u32>(at<u8 *>(object, 0x20), 0x18) = texture;
    at<u32>(at<u8 *>(object, 0x20), 0x1c) = kind;
    at<u32>(at<u8 *>(object, 0x20), 8) = extra;
    at<u32>(at<u8 *>(object, 0x20), 0x34) = 0xffffffff;
    u32 size = 0;
    switch ((s32)kind) {
    case 0xc4:
        size = 0x18;
        break;
    case 0x1c4:
        size = 0x20;
        break;
    case 0x2c4:
        size = 0x28;
        break;
    case 0x3c4:
        size = 0x30;
        break;
    }
    at<u32>(object, 0x1c) = size;
    at<u32>(object, 0x28) = DrawBuffer_VertexLimit(global<u8 *>(0x008bcbc4));
    at<u32>(object, 0x2c) = DrawBuffer_IndexLimit(global<u8 *>(0x008bcbc4));
    return at<u8 *>(object, 0x20);
}
}
