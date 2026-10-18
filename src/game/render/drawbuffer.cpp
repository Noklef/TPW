#include "drawbuffer.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "engineerrors.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// Stream n's record: {vertices, used, room, indices, used, room}.
u32 *stream(const u8 *buffer, u32 n) { return reinterpret_cast<u32 *>((uintptr_t)buffer + n * 24 - 0x10); }
u32 *current(const u8 *buffer) { return stream(buffer, at<u32>(buffer, 4)); }

void PushError() { EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 3, 0xb0754000); }

// A stream's record made (inlined twice in 0x5823f0): 1 MB of vertices and 0x20000 bytes of indices. On a failure the error is
// pushed (and what was got freed); the caller pushes it again.
bool MakeStream(u8 *s) {
    at<u32>(s, 0) = TrackedGlobal_Alloc(0x100000);
    if (!at<u32>(s, 0)) {
        PushError();
        return false;
    }
    at<u32>(s, 8) = 0x100000;
    at<u32>(s, 0xc) = TrackedGlobal_Alloc(0x20000);
    if (!at<u32>(s, 0xc)) {
        PushError();
        if (at<u32>(s, 0xc)) {
            TrackedGlobal_Free(at<u32>(s, 0xc));
            at<u32>(s, 0xc) = 0;
        }
        if (at<u32>(s, 0)) {
            TrackedGlobal_Free(at<u32>(s, 0));
            at<u32>(s, 0) = 0;
        }
        at<u32>(s, 8) = 0;
        at<u32>(s, 0x14) = 0;
        return false;
    }
    at<u32>(s, 0x14) = 0x10000;
    at<u32>(s, 4) = 0;
    at<u32>(s, 0x10) = 0;
    return true;
}

// The batches (+0x44) and the scratch block (+0x60, with +0x58) freed when set, after a failure in 0x5823f0.
u32 FreeBlocks(u8 *buffer) {
    if (at<u32>(buffer, 0x44)) {
        TrackedGlobal_Free(at<u32>(buffer, 0x44));
        at<u32>(buffer, 0x44) = 0;
    }
    if (at<u32>(buffer, 0x60)) {
        TrackedGlobal_Free(at<u32>(buffer, 0x60));
        at<u32>(buffer, 0x60) = 0;
        at<u32>(buffer, 0x58) = 0;
    }
    return 0;
}

}  // namespace

extern "C" {

u32 __fastcall DrawStream_Free(u8 *stream) {
    if (const u32 indices = at<u32>(stream, 0xc)) {
        TrackedGlobal_Free(indices);
        at<u32>(stream, 0xc) = 0;
    }
    if (const u32 vertices = at<u32>(stream, 0)) {
        TrackedGlobal_Free(vertices);
        at<u32>(stream, 0) = 0;
    }
    at<u32>(stream, 8) = 0;
    at<u32>(stream, 0x14) = 0;
    return 1;
}

u32 __fastcall DrawBuffer_Create(u8 *buffer) {
    u8 *const two = buffer + 0x20, *const one = buffer + 8;
    if (!MakeStream(two)) {
        PushError();
        return 0;
    }
    if (!MakeStream(one)) {
        PushError();
        DrawStream_Free(two);   // (inlined)
        DrawStream_Free(one);
        return FreeBlocks(buffer);
    }
    at<u32>(buffer, 0x44) = TrackedGlobal_Alloc(0x38000);
    if (at<u32>(buffer, 0x44)) {
        at<u32>(buffer, 0x48) = 0x1000;
        at<u32>(buffer, 0x60) = TrackedGlobal_Alloc(0x10000f);
        if (u32 scratch = at<u32>(buffer, 0x60)) {
            while (scratch & 0xf)
                ++scratch;
            at<u32>(buffer, 0x58) = scratch;
            at<u32>(buffer, 0x3c) = scratch + 0x100000;
            DrawBuffer_Reset(buffer, nullptr, 0xffffffff);
            return 1;
        }
    }
    PushError();
    DrawStream_Free(two);
    DrawStream_Free(one);
    return FreeBlocks(buffer);
}

u32 __cdecl DrawBuffer_Init() {
    if (global<u8 *>(0x008bcbc4))
        return 0;   // (one already: 0, sic)
    u8 *buffer = static_cast<u8 *>(GameHeap_Alloc(0x64));
    if (buffer) {
        for (u8 *s = buffer + 8; s != buffer + 0x38; s += 0x18) {
            at<u32>(s, 0) = 0;
            at<u32>(s, 0xc) = 0;
            at<u32>(s, 8) = 0;
            at<u32>(s, 4) = 0;
            at<u32>(s, 0x14) = 0;
            at<u32>(s, 0x10) = 0;
        }
        at<u32>(buffer, 0) = 0;
        at<u32>(buffer, 0x44) = 0;
        at<u32>(buffer, 0x48) = 0;
        at<u32>(buffer, 0x4c) = 0;
        at<u32>(buffer, 0x60) = 0;
        at<u32>(buffer, 0x5c) = 0;
        at<u32>(buffer, 0x58) = 0;
        at<u32>(buffer, 0x50) = 0;
        at<u32>(buffer, 0x54) = 0;
    }
    global<u8 *>(0x008bcbc4) = buffer;
    if (!buffer)
        return 0;
    if (DrawBuffer_Create(buffer))
        return 1;
    GameHeap_Free(global<u8 *>(0x008bcbc4));   // (the new one)
    global<u8 *>(0x008bcbc4) = nullptr;
    PushError();   // (the creation's own pushed too)
    return 0;
}

u32 __cdecl DrawBuffer_Destroy() {
    u8 *const buffer = global<u8 *>(0x008bcbc4);
    if (!buffer)
        return 0;
    DrawStream_Free(buffer + 0x20);
    DrawStream_Free(buffer + 8);
    if (const u32 batches = at<u32>(buffer, 0x44)) {
        TrackedGlobal_Free(batches);
        at<u32>(buffer, 0x44) = 0;
    }
    if (const u32 scratch = at<u32>(buffer, 0x60)) {
        TrackedGlobal_Free(scratch);
        at<u32>(buffer, 0x60) = 0;
        at<u32>(buffer, 0x58) = 0;
    }
    GameHeap_Free(global<u8 *>(0x008bcbc4));
    global<u32>(0x008bcbc4) = 0;
    return 1;
}

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
