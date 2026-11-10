#include "soundsource.h"

#include <cstdint>
#include <cstring>

#include "../math/x87.h"
#include "gameheap.h"
#include "initializers.h"
#include "methods.h"
#include "sounddevice.h"
#include "soundslot.h"
#include "sync.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename F> F Import(u32 slot) { return *reinterpret_cast<const F *>((uintptr_t)slot); }

constexpr u32 kHelper = 0x00fb1f04;

}  // namespace

u32 *__fastcall SoundSource711920_SetRate(u8 *source, void *, u32 *out, u32 low) {
    u32 status;   // (the original's is the low argument's own slot)
    if (static_cast<s32>(*Obj_SetRateAtC(source, nullptr, &status, low)) < 0) {
        *out = 0;
        return out;
    }
    const u32 rate = at<u32>(source, 0xc);
    SoundDevice_SetFormat(SoundDevice_Get(), nullptr, out, 2, 0x10, rate);
    return out;
}

u32 *__fastcall SoundSource711920_Configure(u8 *source, void *, u32 *out, const u8 *info) {
    if (!at<u8 *>(source, 0x20)) {
        u32 status;   // (the original's is info's own slot)
        Obj_Pick40Or44(source, nullptr, &status, info);
        Obj_SetRateAtC(source, nullptr, &status, at<u32>(info, 0x68));
        u8 *const config = static_cast<u8 *>(GameHeap_Alloc(0x40));
        std::memset(config, 0, 0x40);
        at<u8 *>(source, 0x20) = config;
        at<u32>(config, 0xc) = at<u32>(SoundDevice_Get(), 0);
        at<u32>(config, 0) = 0x40;
        at<u32>(config, 0x14) = at<u32>(source, 4);
        at<u32>(config, 0x18) = 0;
        at<u32>(config, 0x2c) = at<u32>(source, 4);
        at<u32>(config, 0x30) = at<u32>(source, 4);
        at<u32>(config, 0x38) = at<u32>(source, 4);
        at<u32>(config, 0x3c) = at<u32>(source, 4);
        u8 caps[8];
        SoundCaps_Ctor(caps, nullptr, at<u8 *>(SoundDevice_Get(), 0));
        if (SoundCaps_Max3DStreaming(caps)) {
            at<u32>(config, 0x34) = at<u32>(source, 4);
            at<u32>(config, 0x28) = at<u32>(source, 4);
        }
        if (info[0x4c] & 2) at<u32>(config, 0x24) = at<u32>(info, 0x48);
        const u32 size = at<u32>(source, 4) << 2;
        void *const block = GameHeap_Alloc(size);
        std::memset(block, 0, size);
        at<void *>(source, 0x1c) = block;
        at<u32>(config, 8) = at<u32>(source, 0xc);
        SoundCaps_Dtor(caps);
    }
    *out = 0;
    return out;
}

void __fastcall SoundSource711920_SetFlags(u8 *source) {
    u8 caps[8];
    SoundCaps_Ctor(caps, nullptr, at<u8 *>(SoundDevice_Get(), 0));
    at<u32>(source, 0x24) = 0x10000;
    if (SoundCaps_Max3DStreaming(caps)) at<u32>(source, 0x24) |= 0x20000;
    SoundCaps_Dtor(caps);
}

u32 __fastcall SoundSource711920_GetSound(u8 *, void *, u32 *out) {
    const u32 sound = at<u32>(SoundDevice_Get(), 0);
    *out = sound;
    return (sound & 0xffffff00) | (sound ? 1 : 0);
}

u32 *__fastcall SoundSource711b08_GetSlotPosition(u8 *source, void *, u32 *out, u32 index, u8 *vector) {
    SoundSlot_GetPosition(*reinterpret_cast<u8 **>(source + 0x28) + index * 0x2c, nullptr, out, vector);
    return out;
}

u32 *__fastcall SoundSource711b08_SetOrientation(u8 *source, void *, u32 *out, const u32 *front, const u32 *top) {
    u8 *const listener = at<u8 *>(source, 0x1c);
    if (!listener) {
        *out = 0xffffffff;
        return out;
    }
    *out = comcall<u32>(listener, 0x2c, X87_FloatThrough(front[0]), 0u, X87_FloatThrough(front[2]), X87_FloatThrough(top[0]),
                        X87_FloatThrough(top[1]), X87_FloatThrough(top[2]));
    return out;
}

u32 *__fastcall SoundSource711b08_SetVector(u8 *source, void *, u32 *out, u32, const u32 *vector, u32, u32) {
    const auto isNan = [](u32 bits) { return (bits & 0x7fffffff) > 0x7f800000; };   // (fld; fstp double; _isnan 0x67b410)
    if (u8 *const object = at<u8 *>(source, 0x1c)) {
        const u32 x = vector[0], y = vector[1], z = vector[2];
        if (!isNan(x) && !isNan(y) && !isNan(z)) {
            *out = comcall<u32>(object, 0xc, x, y, z);
            return out;
        }
    }
    *out = 0xffffffff;
    return out;
}

void __fastcall SoundSourceBase_Dtor(u8 *source) {
    at<u32>(source, 0) = (u32)(uintptr_t)SoundSourceBase_vtable;
    SoundHelper_Free();
}

void __fastcall SoundSource711b08_CreateEvents(u8 *source) {
    using CreateEvent = u32(__stdcall *)(void *, u32, u32, const char *);
    const u32 count = at<u32>(source, 4) * 2 + 1;
    at<u32>(source, 0x30) = count;
    at<void *>(source, 0x2c) = GameHeap_Alloc(count * 4);
    for (u32 i = 0; i < at<u32>(source, 0x30); ++i) {
        const u32 event = Import<CreateEvent>(0x006fd0d4)(nullptr, 0, 0, nullptr);   // CreateEventA
        at<u32 *>(source, 0x2c)[i] = event;
    }
}

u32 __fastcall SoundSource711b08_HasCaps(u8 *source) {
    u32 caps[9];   // (only its size set: the rest the slot's to fill)
    caps[0] = 0x24;
    comcall<u32>(at<u8 *>(source, 0x18), 0x2c, caps);
    const u8 flags = static_cast<u8>(caps[1]);
    return (flags & 8) && (flags & 2) && (flags & 0x40) ? 1 : 0;
}

void __fastcall SoundSource_Destroy(void *, void *, u8 **slot) {
    vcall<void>(*slot, 0x1c);   // (not tested for null, sic)
    if (u8 *const source = *slot) vcall<void>(source, 0, 1u);
    *slot = nullptr;
}

u8 *__fastcall SoundSource711b08_Ctor(u8 *source, void *, u32 arg) {
    Init_6d6c50(source);
    for (u32 off = 0x18; off <= 0x2c; off += 4) at<u32>(source, off) = 0;
    at<u32>(source, 0x34) = arg;
    CritSec_Ctor(source + 0x38);
    at<u32>(source, 0) = (u32)(uintptr_t)SoundSource711b08_vtable;
    at<u8 *>(reinterpret_cast<void *>((uintptr_t)0x00fb2724), 0) = source;
    return source;
}

u8 *__fastcall SoundSource711920_Ctor(u8 *source) {
    Init_6d6c50(source);
    at<u32>(source, 0x18) = at<u32>(source, 0x1c) = at<u32>(source, 0x20) = 0;
    at<u32>(source, 0) = (u32)(uintptr_t)SoundSource711920_vtable;
    u32 *const helper = static_cast<u32 *>(GameHeap_Alloc(4));
    if (helper) *helper = (u32)(uintptr_t)Class711910_vtable;
    SoundHelper_Set(helper);
    return source;
}

void __cdecl SoundHelper_Set(void *helper) { at<void *>(reinterpret_cast<void *>((uintptr_t)kHelper), 0) = helper; }

void __cdecl SoundHelper_Free() {
    if (void *const helper = at<void *>(reinterpret_cast<void *>((uintptr_t)kHelper), 0)) GameHeap_Free(helper);
}

u32 __fastcall SoundHelperFn70a27c_Run(const u8 *) { return vcall<u32>(at<u8 *>(reinterpret_cast<void *>((uintptr_t)kHelper), 0), 0); }

void __fastcall SoundSource711920_Dtor(u8 *source) {
    at<u32>(source, 0) = (u32)(uintptr_t)SoundSource711920_vtable;
    if (void *const block = at<void *>(source, 0x20)) GameHeap_Free(block);
    if (void *const block = at<void *>(source, 0x1c)) GameHeap_Free(block);
    at<u32>(source, 0) = (u32)(uintptr_t)SoundSourceBase_vtable;
    SoundHelper_Free();
}

u8 *__fastcall SoundSource711920_DeletingDtor(u8 *source, void *, u32 flags) {
    SoundSource711920_Dtor(source);
    if (flags & 1) GameHeap_Free(source);
    return source;
}

namespace {
const SoundMakeEnv kMakeEnv = {&SoundSource711b08_Ctor, &SoundSource711920_Ctor};
const SoundMakeEnv *g_make = &kMakeEnv;
}  // namespace

void SoundSource_SetMakeEnv(const SoundMakeEnv *env) { g_make = env ? env : &kMakeEnv; }

u32 *__fastcall SoundSource_Make(void *self, void *, u32 *out, u8 **slot, const u8 *info, u32 arg) {
    uintptr_t local = arg;
    u8 *block = static_cast<u8 *>(GameHeap_Alloc(0x50));
    u8 *source = block ? g_make->ctorB(block, nullptr, arg) : nullptr;
    *slot = source;
    if (*vcall<s32 *>(source, 0x18, &local, info) >= 0) {
        *out = 0;
        return out;
    }
    SoundSource_Destroy(self, nullptr, slot);
    block = static_cast<u8 *>(GameHeap_Alloc(0x28));
    local = reinterpret_cast<uintptr_t>(block);
    source = block ? g_make->ctorA(block) : nullptr;
    *slot = source;
    if (*vcall<s32 *>(source, 0x18, &local, info) >= 0) {
        *out = 0;
        return out;
    }
    SoundSource_Destroy(self, nullptr, slot);
    *out = 0xffffffffu;
    return out;
}
