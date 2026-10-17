#include "small64.h"

#include <cstdint>

#include "../render/drawbuffer.h"
#include "gameheap.h"
#include "small20.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

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

u32 __fastcall Object_ReleaseSavedPairs(u8 *object) {
    if ((at<u32>(object, 4) & 0x20000000) != 0 && at<s32>(object, 0x12c) > 0) {
        u8 *pairs = at<u8 *>(object, 0x124);
        for (s32 i = at<s32>(object, 0x12c) - 1; i >= 0; i--)
            *at<u32 *>(pairs, 4 + i * 8) = at<u32>(pairs, i * 8);
        if (at<u8 *>(object, 0x124)) {
            GameHeap_Free(at<u8 *>(object, 0x124));
            at<u32>(object, 0x124) = 0;
        }
        if (u8 *elements = at<u8 *>(object, 0x128)) {
            CRT_VectorDtorIterator(elements, 0x10, at<s32>(elements, -4), reinterpret_cast<void(__fastcall *)(void *)>(0x00575bf0));
            GameHeap_Free(elements - 4);
            at<u32>(object, 0x128) = 0;
        }
        at<u32>(object, 4) = at<u32>(object, 4) & 0xdfffffff;
    }
    at<u32>(object, 0x12c) = 0;
    return 1;
}

}  // extern "C"
