#include "batch.h"

#include <cstdint>

#include "drawbuffer.h"
#include "vertexfog.h"

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

namespace {

void fog(u8 *object, u32 render, u8 *vertices, u32 count) {
    if (render & 0x4000) {
        VertexFog_Colour(vertices, count, at<s32>(object, 0x1c));
        at<u32>(at<u8 *>(object, 0x20), 4) &= ~0x100000u;
        at<u32>(at<u8 *>(object, 0x20), 4) |= 0x80000;
    } else if (render & 0x8000) {
        VertexFog_Alpha(vertices, count, at<s32>(object, 0x1c));
    }
}

}  // namespace

void __fastcall Batch_FogVertices(u8 *object, void *, u8 *vertices, u32 count) {
    const u32 render = global<u32>(0x008bcbc8);
    if (render & 0x2000)
        fog(object, render, vertices, count);
}

void __fastcall Batch_Fog(u8 *object, void *) {
    if (global<u32>(0x008bd524) != 1)
        return;
    u8 *batch = at<u8 *>(object, 0x20);
    fog(object, global<u32>(0x008bcbc8), at<u8 *>(batch, 0x28), at<u32>(batch, 0x20));
}
}
