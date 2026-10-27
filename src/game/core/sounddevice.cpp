#include "sounddevice.h"

#include <cstdint>

#include "gameheap.h"
#include "methods.h"
#include "vcall.h"
#include "waveformat.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename F> F import(std::uintptr_t slot) { return *reinterpret_cast<const F *>(slot); }
u8 *Ptr(u32 address) { return reinterpret_cast<u8 *>(static_cast<uintptr_t>(address)); }
using AtExitFn = void(__cdecl *)();

const SoundDeviceEnv kGameEnv = {reinterpret_cast<s32(__cdecl *)(AtExitFn)>((uintptr_t)0x0067a7d0)};
const SoundDeviceEnv *g_env = &kGameEnv;

}  // namespace

void SoundDevice_SetEnv(const SoundDeviceEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u8 *__fastcall ComInit_Ctor(u8 *self) {
    at<u8>(self, 0) = import<u32(__stdcall *)(void *)>(0x006fd44c)(nullptr) == 0;
    return self;
}

void __fastcall ComInit_Dtor(u8 *self) {
    if (at<u8>(self, 0)) import<u32(__stdcall *)()>(0x006fd450)();
}

u8 *__cdecl ComInit_Get() {
    u8 &guard = *Ptr(0xfb2720);
    if (!(guard & 1)) {
        guard |= 1;
        ComInit_Ctor(Ptr(0xfb271c));
        g_env->atExit(reinterpret_cast<AtExitFn>((uintptr_t)0x006d3b80));
    }
    return Ptr(0xfb271c);
}

void __cdecl ComInit_SingletonDtor() { ComInit_Dtor(Ptr(0xfb271c)); }

u8 *__fastcall SoundDevice_Ctor(u8 *device) {
    at<u32>(device, 0) = 0;
    at<u32>(device, 4) = 0;
    u32 status;
    SoundDevice_Open(device, nullptr, &status);
    return device;
}

u32 *__fastcall SoundDevice_CreatePrimary(u8 *device, void *, u32 *out) {
    u32 desc[5] = {0x14, 0x11, 0, 0, 0};
    if (static_cast<s32>(comcall<u32>(at<u8 *>(device, 0), 0xc, desc, device + 4, 0u)) < 0) {
        *out = 0xffffffff;
        return out;
    }
    SoundDevice_SetFormat(device, nullptr, out, 2, 0x10, 0x5622);
    return out;
}

u32 *__fastcall SoundDevice_CreatePrimaryOf(u8 *device, void *, u32 *out) {
    SoundDevice_CreatePrimary(device, nullptr, out);
    return out;
}

u32 *__fastcall SoundDevice_SetFormat(u8 *device, void *, u32 *out, u32 channels, u32 bits, u32 rate) {
    if (at<u8 *>(device, 4)) {
        u32 holder[7];
        at<u8>(holder, 0) = static_cast<u8>(channels);
        at<u8>(holder, 1) = static_cast<u8>(bits);
        at<u16>(holder, 2) = static_cast<u16>(rate);
        at<u16>(holder, 4) = 0;
        holder[6] = 0;
        u8 *const format = WaveFormat_Build(reinterpret_cast<u8 *>(holder));
        if (static_cast<s32>(comcall<u32>(at<u8 *>(device, 4), 0x38, format)) >= 0) {
            *out = 0;
            return out;
        }
    }
    *out = 0xffffffff;
    return out;
}

u32 *__fastcall SoundDevice_Open(u8 *device, void *, u32 *out) {
    ComInit_Get();
    u32 status;
    if (static_cast<s32>(*SoundDevice_CoCreate(device, nullptr, &status)) >= 0 &&
        static_cast<s32>(comcall<u32>(at<u8 *>(device, 0), 0x28, 0u)) >= 0) {
        const u32 window = import<u32(__stdcall *)()>(0x006fd370)();
        if (static_cast<s32>(comcall<u32>(at<u8 *>(device, 0), 0x18, window, 2u)) >= 0) {
            *out = 0;
            return out;
        }
    }
    SoundDevice_Release(device);
    *out = 0xffffffff;
    return out;
}

void __fastcall SoundDevice_Release(u8 *device) {
    static const u32 kOrder[2] = {4, 0};
    for (u32 offset : kOrder) {
        u8 *const object = at<u8 *>(device, offset);
        if (object) {
            comcall<u32>(object, 8);
            at<u8 *>(device, offset) = nullptr;
        }
    }
}

u32 *__fastcall SoundDevice_CoCreate(u8 *device, void *, u32 *out) {
    using Create = u32(__stdcall *)(u32, void *, u32, u32, u8 *);
    const Create create = import<Create>(0x006fd454);
    if (static_cast<s32>(create(0x0070aa40u, nullptr, 1u, 0x00707f70u, device)) >= 0) {
        *out = 0;
        return out;
    }
    *out = create(0x00707f50u, nullptr, 1u, 0x00707f70u, device);
    return out;
}

u8 *__cdecl SoundDevice_Get() {
    u8 &guard = *Ptr(0xfb2028);
    if (!(guard & 1)) {
        guard |= 1;
        SoundDevice_Ctor(Ptr(0xfb2030));
        g_env->atExit(reinterpret_cast<AtExitFn>((uintptr_t)0x006c4540));
    }
    return Ptr(0xfb2030);
}

u8 *__fastcall SoundCaps_Ctor(u8 *caps, void *, u8 *sound) {
    at<u8 *>(caps, 0) = sound;
    at<u32>(caps, 4) = 0;
    if (sound) comcall<u32>(sound, 4);   // AddRef
    return caps;
}

void __fastcall SoundCaps_Dtor(u8 *caps) {
    if (at<void *>(caps, 4)) GameHeap_Free(at<void *>(caps, 4));
    if (u8 *const sound = at<u8 *>(caps, 0)) comcall<u32>(sound, 8);   // Release
}

u32 *__fastcall SoundCaps_Query(u8 *caps, void *, u32 *out) {
    if (!at<u8 *>(caps, 0) &&
        static_cast<s32>(import<u32(__stdcall *)(const void *, void *, void *)>(0x006fd028)(nullptr, caps, nullptr)) < 0) {   // DirectSoundCreate
        *out = 0xffffffff;
        return out;
    }
    if (at<u32 *>(caps, 4)) {
        *out = 0;
        return out;
    }
    u32 *const block = static_cast<u32 *>(GameHeap_Alloc(0x60));
    for (u32 i = 0; i < 0x18; ++i) block[i] = 0;   // (not checked: a failed allocation faults here)
    at<u32 *>(caps, 4) = block;
    block[0] = 0x60;
    *out = comcall<u32>(at<u8 *>(caps, 0), 0x10, at<u32 *>(caps, 4));   // GetCaps
    return out;
}

u32 __fastcall SoundCaps_Max3DStreaming(u8 *caps) {
    Obj_CacheStatics(caps);
    return at<u32>(Ptr(0xfb201c), 0);
}

u32 __fastcall SoundCaps_Free3DStreaming(u8 *caps) {
    Obj_CacheStatics(caps);
    return at<u32>(Ptr(0xfb2020), 0);
}

}  // extern "C"
