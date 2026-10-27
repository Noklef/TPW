#include "soundindex.h"

#include <cstdint>
#include <cstring>

#include "../files/handles.h"
#include "gameheap.h"
#include "misc.h"
#include "sfbank.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__stdcall SoundResource_New(const u8 *owner, const u32 *desc, u8 *source) {
    u32 *const out = reinterpret_cast<u32 *>(&desc);   // (sic: the arguments' own slots)
    if (static_cast<s32>(*SoundResource_Create(out, &source, owner, desc, source)) < 0) return nullptr;
    u8 *const ref = static_cast<u8 *>(GameHeap_Alloc(0xc));
    if (!ref) return nullptr;
    return SoundResourceRef_Ctor(ref, nullptr, source);
}

u8 *__fastcall SoundResourceRef_Ctor(u8 *ref, void *, u8 *resource) {
    Class703770_Ctor(ref);
    at<u32>(ref, 0) = 0x0070af18;
    at<u8 *>(ref, 8) = resource;
    return ref;
}

void __fastcall SoundResourceRef_Dtor(u8 *ref) {
    at<u32>(ref, 0) = 0x0070af18;
    vcall<void>(at<u8 *>(ref, 8), 0);
    at<u32>(ref, 0) = 0x00703770;
}

u32 __fastcall SoundResourceRef_Slot11(u8 *ref) {
    u8 *const resource = at<u8 *>(ref, 8);
    return resource ? vcall<u32>(resource, 0x2c) : 0;
}

u32 __fastcall SoundResourceRef_Slot12(u8 *ref) {
    u8 *const resource = at<u8 *>(ref, 8);
    return resource ? vcall<u32>(resource, 0x30) : 0;
}

u32 __fastcall SoundResourceRef_Slot13(u8 *ref) {
    u8 *const resource = at<u8 *>(ref, 8);
    return resource ? vcall<u32>(resource, 0x34) : 0;
}

u32 __fastcall SoundResourceRef_Offset(u8 *ref, void *, u32 i) {
    u8 *const resource = at<u8 *>(ref, 8);
    return resource ? vcall<u32>(resource, 0x24, i) : 0;
}

u32 *__fastcall SoundResourceRef_Slot15(u8 *ref, void *, u32 *out, u32 a) {
    u8 *const resource = at<u8 *>(ref, 8);
    if (!resource) {
        *out = 0xffffffff;
        return out;
    }
    u32 slot = a;   // (the a argument's own stack slot)
    *out = *vcall<const u32 *>(resource, 0x3c, &slot, a);
    return out;
}

u32 __fastcall SoundResourceRef_ReadRecord(u8 *ref, void *, u32 record, u32 i) {
    u8 *const resource = at<u8 *>(ref, 8);
    if (!resource) return 0;
    u32 slot = i;   // (the i argument's own stack slot)
    return static_cast<s32>(*vcall<const u32 *>(resource, 0x18, &slot, record, i)) >= 0 ? 1 : 0;
}

u8 *__fastcall SoundResourceRef_DeletingDtor(u8 *ref, void *, u32 flags) {
    SoundResourceRef_Dtor(ref);
    if (flags & 1) GameHeap_Free(ref);
    return ref;
}

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

u32 *__stdcall SoundResource_Create(u32 *out, u8 **resource, const u8 *owner, const u32 *desc, u8 *source) {
    *resource = nullptr;
    switch (desc[0]) {
    case 1: {
        u8 *const p = static_cast<u8 *>(GameHeap_Alloc(0x1c));
        if (p) {
            const StreamBase *const stream = reinterpret_cast<const StreamBase *>((uintptr_t)desc[2]);
            SoundResource_Ctor(p, nullptr, source);
            StreamBase_CopyCtor(reinterpret_cast<StreamBase *>(p + 0x14), nullptr, stream);
            at<u32>(p, 0) = 0x0070aa80;
        }
        *resource = p;
        break;
    }
    case 2: {
        u8 *const p = static_cast<u8 *>(GameHeap_Alloc(0x20));
        if (p) {
            const StreamBase *const stream = reinterpret_cast<const StreamBase *>((uintptr_t)desc[2]);
            SoundResource_Ctor(p, nullptr, source);
            at<u32>(p, 0x14) = 0;
            StreamBase_CopyCtor(reinterpret_cast<StreamBase *>(p + 0x18), nullptr, stream);
            at<u32>(p, 0) = 0x0070aac0;
        }
        *resource = p;
        break;
    }
    case 3:
        if (owner) {
            u8 *const p = static_cast<u8 *>(GameHeap_Alloc(0x1c));
            *resource = p ? SfBank_Ctor(p, nullptr, source, at<u8 *>(owner, 4)) : nullptr;
        }
        break;
    }
    if (u8 *const r = *resource) {
        vcall<u32 *>(r, 0xc, out, desc[3], desc[1]);
        return out;
    }
    *out = 0xffffffff;
    return out;
}

}  // extern "C"
