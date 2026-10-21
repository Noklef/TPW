#include "soundindex.h"

#include <cstdint>
#include <cstring>

#include "../files/handles.h"
#include "gameheap.h"
#include "misc.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__fastcall SoundResource_Ctor(u8 *self, void *, u8 *source) {
    Class703770_Ctor(self);
    at<u32>(self, 8) = 0;
    at<u32>(self, 0xc) = 0;
    at<u8 *>(self, 0x10) = source;
    vcall<void>(source, 4);
    at<u32>(self, 0) = 0x0070ab00;
    return self;
}

void __fastcall SoundResource_Dtor(u8 *self) {
    at<u32>(self, 0) = 0x0070ab00;
    if (u8 *const source = at<u8 *>(self, 0x10)) vcall<void>(source, 0);
    at<u32>(self, 0) = 0x00703770;
}

u8 *__fastcall SoundResource_DeletingDtor(u8 *self, void *, u32 flags) {
    SoundResource_Dtor(self);
    if (flags & 1) GameHeap_Free(self);
    return self;
}

u32 *__fastcall SoundStreamIndex_Load(u8 *self, void *, u32 *out, u32, u32) {
    u8 *const stream = self + 0x14;
    if (vcall<s32>(stream, 0x10, 0u, 0u) != -1 && vcall<u32>(stream, 8, self + 0xc, 4u) == 4) {
        at<void *>(self, 8) = GameHeap_Alloc(at<u32>(self, 0xc) << 2);
        vcall<u32>(stream, 8, at<void *>(self, 8), at<u32>(self, 0xc) << 2);
        *out = 0;
        return out;
    }
    vcall<void>(self, 0x10);
    *out = 0xffffffff;
    return out;
}

void __fastcall SoundStreamIndex_Unload(u8 *self) {
    vcall<void>(self + 0x14, 4);
    if (void *const block = at<void *>(self, 8)) {
        GameHeap_Free(block);
        at<u32>(self, 8) = 0;
    }
}

u32 __fastcall SoundStreamIndex_Offset(const u8 *self, void *, u32 i) { return at<const u32 *>(self, 8)[i - 1]; }

u32 *__fastcall SoundStreamIndex_ReadRecord(u8 *self, void *, u32 *out, u8 *const *record, u32 i) {
    if (i > at<u32>(self, 0xc)) {
        *out = 0xffffffff;
        return out;
    }
    u8 *const stream = self + 0x14;
    vcall<u32>(stream, 0x10, at<const u32 *>(self, 8)[i - 1], 0u);
    vcall<u32>(stream, 8, *record, 0x28u);
    *out = 0;
    return out;
}

void __fastcall SoundStreamIndex_Dtor(u8 *self) {
    at<u32>(self, 0) = 0x0070aa80;
    SoundStreamIndex_Unload(self);
    StreamBase_Dtor(reinterpret_cast<StreamBase *>(self + 0x14));
    SoundResource_Dtor(self);
}

u8 *__fastcall SoundStreamIndex_DeletingDtor(u8 *self, void *, u32 flags) {
    SoundStreamIndex_Dtor(self);
    if (flags & 1) GameHeap_Free(self);
    return self;
}

u32 *__fastcall SoundMapIndex_Map(u8 *self, void *, u32 *out, u32, u32) {
    const u32 *const map = vcall<const u32 *>(self + 0x18, 0x1c);
    at<const u32 *>(self, 8) = map;
    if (map) {
        at<u32>(self, 0xc) = map[0];
        const u32 size = map[0] * 4;
        void *const block = GameHeap_Alloc(size);
        std::memset(block, 0, size);
        at<void *>(self, 0x14) = block;
        *out = 0;
        return out;
    }
    vcall<void>(self, 0x10);
    *out = 0xffffffff;
    return out;
}

void __fastcall SoundMapIndex_Unmap(u8 *self) {
    vcall<void>(self + 0x18, 4);
    if (void *const block = at<void *>(self, 0x14)) {
        GameHeap_Free(block);
        at<u32>(self, 0x14) = 0;
    }
}

void __fastcall SoundMapIndex_Dtor(u8 *self) {
    at<u32>(self, 0) = 0x0070aac0;
    SoundMapIndex_Unmap(self);
    StreamBase_Dtor(reinterpret_cast<StreamBase *>(self + 0x18));
    SoundResource_Dtor(self);
}

u8 *__fastcall SoundMapIndex_DeletingDtor(u8 *self, void *, u32 flags) {
    SoundMapIndex_Dtor(self);
    if (flags & 1) GameHeap_Free(self);
    return self;
}

}  // extern "C"
