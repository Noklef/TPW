#include "batch.h"

#include <cstdint>

#include "drawbuffer.h"
#include "texformat.h"
#include "../gfx/texset.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u8 *__fastcall Model_BeginBatch(u8 *model, void *, u32 index, u32 *size) {
    const u8 *const settings = at<u8 *>(model, 4);
    const u32 mode = (at<u32>(settings, 0) & 0xc0000) ? 2 : 1;
    const u8 *const entry = at<u8 *>(settings, 0x6c) + (index << 4);
    const u8 *const material = at<u8 *>(entry, 0);
    const u8 *const set = material ? at<u8 *>(material, 4) : nullptr;
    const u32 i = material && (global<u8>(0x008bd550) & 0x40) ? at<u16>(material, 2) : 0;
    const u8 *const texture = set ? reinterpret_cast<const u8 *>((uintptr_t)TexSet_Texture(set, nullptr, (s32)i)) : nullptr;
    const bool shaded = texture && (at<u32>(texture, 8) & 0x400) && !at<u32>(texture, 0x3c);
    const u32 kind = (at<u32>(settings, 0) & 0x2000000) || shaded ? 0x2c4 : 0x1c4;
    const u32 extra = texture && (at<u32>(texture, 8) & 0x1000) ? at<u32>(texture, 0x1c) : 0;
    const u32 flags = TexLoad_DriverFlags(model, nullptr, entry, texture);
    u8 *const batch = Batch_Begin(global<u8 *>(0x00879ab0), nullptr, mode, 4, kind, flags, extra);
    if (!batch)
        return nullptr;
    const u32 word = (at<u32>(model, 8) >> 24) & 2;
    at<u32>(batch, 0) = shaded ? word | 0x80 : word;
    at<u32>(batch, 0x34) = texture ? at<u32>(texture, 0x44) : 0xffffffff;
    *size = kind == 0x1c4 ? 0x20 : 0x28;
    return batch;
}

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
