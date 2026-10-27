#include "substream.h"

#include <cstdint>

#include "../core/semaphore.h"
#include "../core/vcall.h"
#include "handles.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *construct(u8 *self, const u8 *stream, u32 start) {
    at<u32>(self, 4) = 0;
    at<u32>(self, 8) = start;
    StreamBase_CopyCtor(reinterpret_cast<StreamBase *>(self + 0xc), nullptr, reinterpret_cast<const StreamBase *>(stream));
    Semaphore_Ctor(reinterpret_cast<Semaphore *>(self + 0x14));
    at<u32>(self, 0) = 0x0070ad50;
    return self;
}

}  // namespace

extern "C" {

u8 *__fastcall SubStream_Ctor(u8 *self, void *, const u8 *stream) { return construct(self, stream, 0); }

void __fastcall SubStream_Dtor(u8 *self) {
    at<u32>(self, 0) = 0x0070ad50;
    Semaphore_Dtor(reinterpret_cast<Semaphore *>(self + 0x14));
    StreamBase_Dtor(reinterpret_cast<StreamBase *>(self + 0xc));
}

u8 *__fastcall SubStream_CtorAt(u8 *self, void *, const u8 *stream, u32 start) { return construct(self, stream, start); }

u32 __fastcall SubStream_Read(u8 *self, void *, void *buffer, u32 count) {
    SemaphoreLock lock;
    SemaphoreLock_Ctor(&lock, nullptr, reinterpret_cast<const Semaphore *>(self + 0x14), 0);
    SemaphoreLock_Acquire(&lock, nullptr, 1);
    u8 *const stream = self + 0xc;
    const u32 position = at<u32>(self, 4);
    if (vcall<u32>(stream, 0x18) != at<u32>(self, 8) + position) vcall<u32>(self, 8, position);
    const u32 read = vcall<u32>(stream, 8, buffer, count);
    const u32 now = at<u32>(self, 4) + read;
    at<u32>(self, 4) = now;
    SemaphoreLock_Dtor(&lock);
    return now;
}

u32 __fastcall SubStream_Seek(u8 *self, void *, u32 offset) {
    const u32 answer = vcall<u32>(self + 0xc, 0x10, offset + at<u32>(self, 8), 0u);
    at<u32>(self, 4) = answer - at<u32>(self, 8);
    return answer;
}

u32 __fastcall SubStream_Rewind(u8 *self) {
    at<u32>(self, 4) = 0;
    return vcall<u32>(self + 0xc, 0x10, at<u32>(self, 8), 0u);
}

}  // extern "C"
