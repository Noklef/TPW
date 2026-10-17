#include "drawbuffer.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// Stream n's record: {vertices, used, room, indices, used, room}.
u32 *stream(const u8 *buffer, u32 n) { return reinterpret_cast<u32 *>((uintptr_t)buffer + n * 24 - 0x10); }
u32 *current(const u8 *buffer) { return stream(buffer, at<u32>(buffer, 4)); }

}  // namespace

extern "C" {

void __fastcall DrawBuffer_Reset(u8 *buffer, void *, u32 streams) {
    u8 which = (u8)streams;
    if (which & 1) {
        at<u32>(buffer, 0xc) = 0;
        at<u32>(buffer, 0x18) = 0;
        at<s32>(buffer, 0x54) = at<s32>(buffer, 0x48) - 1;
    } else {
        for (s32 i = at<s32>(buffer, 0x54) + 1; i < at<s32>(buffer, 0x48); ++i)
            at<u32>(at<u8 *>(buffer, 0x44), i * 0x38) = 0;
    }
    if (which & 2) {
        at<u32>(buffer, 0x24) = 0;
        at<u32>(buffer, 0x30) = 0;
        at<u32>(buffer, 0x50) = 0;
    }
    at<u32>(buffer, 0x5c) = at<u32>(buffer, 0x58);
}

u32 __fastcall DrawBuffer_Alloc(u8 *buffer, void *, u32 n) {
    u32 start = at<u32>(buffer, 0x5c);
    u32 end = (n << 4) + start;
    if (end > at<u32>(buffer, 0x3c))
        return 0;
    at<u32>(buffer, 0x5c) = end;
    return start;
}

u8 *__fastcall DrawBuffer_Begin(u8 *buffer, void *, u32 mode) {
    u32 n = mode & 3;
    u32 slot = n == 1 ? at<u32>(buffer, 0x54) : at<u32>(buffer, 0x50);
    if ((mode & 4) == 0)
        at<u32>(buffer, 0x4c) = slot;
    at<u32>(buffer, 0) = mode;
    at<u32>(buffer, 4) = n;
    u32 *batch = reinterpret_cast<u32 *>(at<u8 *>(buffer, 0x44) + slot * 0x38);
    at<u32 *>(buffer, 0x38) = batch;
    for (int i = 0; i < 14; ++i)
        batch[i] = 0;
    u32 *s = current(buffer);
    at<u32>(at<u8 *>(buffer, 0x38), 0x28) = s[0] + s[1];
    s = current(buffer);
    at<u32>(at<u8 *>(buffer, 0x38), 0x2c) = s[3] + s[4];
    return at<u8 *>(buffer, 0x38);
}

u32 __fastcall DrawBuffer_Vertices(const u8 *buffer) {
    const u32 *s = current(buffer);
    return s[0] + s[1];
}

u8 *__fastcall DrawBuffer_Resume(u8 *buffer, void *, u32 mode) {
    u32 slot = (mode & 3) == 1 ? at<u32>(buffer, 0x54) : at<u32>(buffer, 0x50);
    u32 filled = at<u32>(buffer, 0x4c);
    if (filled != slot) {
        u8 *batches = at<u8 *>(buffer, 0x44);
        u32 *to = reinterpret_cast<u32 *>(batches + slot * 0x38);
        const u32 *from = reinterpret_cast<const u32 *>(batches + filled * 0x38);
        at<u32 *>(buffer, 0x38) = to;
        for (int i = 0; i < 14; ++i)
            to[i] = from[i];
        u32 *s = current(buffer);
        at<u32>(at<u8 *>(buffer, 0x38), 0x2c) = s[3] + s[4];
    }
    return at<u8 *>(buffer, 0x38);
}

void __fastcall DrawBuffer_End(u8 *buffer, void *, u32 vertexEnd, u32 indexEnd) {
    if (at<u8>(buffer, 0) & 4)
        return;
    u32 *s = current(buffer);
    s[1] = vertexEnd - s[0];
    s = current(buffer);
    s[4] = indexEnd - s[3];
    if (at<u32>(buffer, 4) == 1)
        --at<s32>(buffer, 0x54);
    else
        ++at<s32>(buffer, 0x50);
}

u32 __fastcall DrawBuffer_VertexLimit(const u8 *buffer) {
    const u32 *s = current(buffer);
    return s[0] + s[2];
}

u32 __fastcall DrawBuffer_IndexLimit(const u8 *buffer) {
    const u32 *s = current(buffer);
    return s[3] + s[5];
}
u8 *__fastcall DrawBatch_Open(u8 *object, void *, u32 mode, u32 a, u32 kind, u32 b, u32 c) {
    u8 *batch = DrawBuffer_Resume(global<u8 *>(0x008bcbc4), nullptr, mode);
    at<u8 *>(object, 0x20) = batch;
    if (!batch)
        return nullptr;
    at<u32>(batch, 4) = b;
    at<u32>(batch, 0x18) = a;
    at<u32>(batch, 0x1c) = kind;
    at<u32>(batch, 8) = c;
    at<u32>(batch, 0x34) = 0xffffffff;
    switch (kind) {
    case 0x1c4: at<u32>(object, 0x1c) = 0x20; break;
    case 0xc4: at<u32>(object, 0x1c) = 0x18; break;
    case 0x2c4: at<u32>(object, 0x1c) = 0x28; break;
    case 0x3c4: at<u32>(object, 0x1c) = 0x30; break;
    default: at<u32>(object, 0x1c) = 0; break;
    }
    return at<u8 *>(object, 0x20);
}

}
