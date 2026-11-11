// The DirectSound source's open, close and slot calls: see dsoundsource.h.
#include "dsoundsource.h"

#include <cstdint>

#include "gameheap.h"
#include "methods.h"
#include "releaselist.h"
#include "sounddevice.h"
#include "soundslot.h"
#include "soundsource.h"
#include "soundwait.h"
#include "soundworker.h"
#include "vcall.h"
#include "waveformat.h"
#include "imports.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename F> F game(u32 address) { return reinterpret_cast<F>((uintptr_t)address); }
void *address(u32 a) { return reinterpret_cast<void *>((uintptr_t)a); }
template <typename... A> u32 import(u32 slot, A... args) { return (*reinterpret_cast<u32(__stdcall *const *)(A...)>((uintptr_t)slot))(args...); }

const DSoundSourceEnv kGameEnv = {
    game<u32(__cdecl *)(u32, u32, void *)>(0x0067e050),
    game<void(__stdcall *)(u8 *, u32, u32, u32, u32)>(0x0067acf0),
    game<void(__stdcall *)(u8 *, u32, u32, u32)>(0x0067afa0),
    &SoundWorker_RefillVoice,
    &SoundSource_WaitLoop,
};
const DSoundSourceEnv *g_env = &kGameEnv;

bool failed(u32 answer) { return (s32)answer < 0; }
u8 *direct_sound(const u8 *source) { return at<u8 *>(source, 0x18); }
u8 *slot_of(const u8 *source, u32 index) { return at<u8 *>(source, 0x28) + index * 0x2c; }
void enter(u8 *source) { import(IMPORT(Kernel32_EnterCriticalSection), source + 0x38); }
void leave(u8 *source) { import(IMPORT(Kernel32_LeaveCriticalSection), source + 0x38); }

void release(u8 *source, u32 offset) {
    u8 *&object = at<u8 *>(source, offset);
    if (object) comcall<u32>(object, 8);
    object = nullptr;
}

void set_up_extra_interfaces(u8 *source) {
    u8 *&extra = at<u8 *>(source, 0x20);
    u8 *&inner = at<u8 *>(source, 0x24);
    if (!failed(comcall<u32>(extra, 0x8c, &inner))) {
        comcall<u32>(inner, 0x38, 0u, 0u);
        comcall<u32>(inner, 0x30, 0x3f4ccccdu, 0x3f4ccccdu);
        comcall<u32>(extra, 0x90, inner);
    }
    comcall<u32>(extra, 0xc, 0x40u);
    comcall<u32>(extra, 0xc, 2u);
    comcall<u32>(extra, 0x30, 0x41200000u);
    comcall<u32>(extra, 0x28, 0x3f800000u);
}

bool open_source(u8 *source, const u8 *info) {
    ComInit_Get();
    if (failed(import(IMPORT(Ole32_CoCreateInstance), address(0x00711a68), 0u, 1u, address(0x00711aa8), &at<u8 *>(source, 0x18)))) return false;
    if (failed(comcall<u32>(direct_sound(source), 0x3c, 0u, 0xc2u, 0u))) return false;
    if (!(u8)SoundSource711b08_HasCaps(source)) return false;
    const u32 window = at<u8>(info, 0x4c) & 2 ? at<u32>(info, 0x48) : import(IMPORT(User32_GetForegroundWindow));
    if (failed(comcall<u32>(direct_sound(source), 0x4c, window, 1u))) return false;
    comcall<u32>(direct_sound(source), 0x5c, 1u);
    if (!comcall<u32>(direct_sound(source), 0x40, 0x40u) || !comcall<u32>(direct_sound(source), 0x40, 2u)) return false;
    if (!failed(comcall<u32>(direct_sound(source), 0, address(0x00711ab8), &at<u8 *>(source, 0x20)))) set_up_extra_interfaces(source);
    if (failed(comcall<u32>(direct_sound(source), 0, address(0x00711ad8), &at<u8 *>(source, 0x1c)))) return false;
    comcall<u32>(at<u8 *>(source, 0x1c), 0xc, 0u, 0u, 0u);
    const u32 count = at<u32>(info, 0x44);
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(count * 0x2c + 4));
    if (block) {
        at<u32>(block, 0) = count;
        g_env->vectorCtor(block + 4, 0x2c, count, 0x006d6dd0, 0x005da3c0);
    }
    at<u8 *>(source, 0x28) = block ? block + 4 : nullptr;
    u32 status;
    Obj_Pick40Or44(source, nullptr, &status, info);
    SoundSource711b08_CreateEvents(source);
    at<u32>(source, 8) |= 0x40;
    const u32 thread = g_env->beginthread(0x006d2d60, 0, source);
    return !failed(import(IMPORT(Kernel32_SetThreadPriority), thread, 0xfu));
}

}  // namespace

void DSoundSource_SetEnv(const DSoundSourceEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl SoundSource711b08_Thread(u8 *source) { g_env->waitLoop(source); }

u32 *__fastcall SoundSource711b08_Open(u8 *source, void *, u32 *out, const u8 *info) {
    if (open_source(source, info)) {
        *out = 0;
    } else {
        at<u32>(source, 8) &= 0xffffffbf;
        *out = 0xffffffff;
    }
    return out;
}

void __fastcall SoundSource711b08_Close(u8 *source) {
    if (at<u8>(source, 8) & 0x40) {
        import(IMPORT(Kernel32_SetEvent), at<u32 *>(source, 0x2c)[at<u32>(source, 0x30) - 1]);
        while (*reinterpret_cast<volatile u8 *>(source + 8) & 0x80) {
        }
    }
    ComList_ReleaseAll(reinterpret_cast<u8 *>(ReleaseList_Get()));
    release(source, 0x1c);
    release(source, 0x20);
    release(source, 0x24);
    release(source, 0x18);
    if (at<u32 *>(source, 0x2c)) {
        for (u32 i = 0; i < at<u32>(source, 0x30); ++i) import(IMPORT(Kernel32_CloseHandle), at<u32 *>(source, 0x2c)[i]);
        GameHeap_Free(at<u32 *>(source, 0x2c));
        at<u32 *>(source, 0x2c) = nullptr;
    }
    if (u8 *const slots = at<u8 *>(source, 0x28)) {
        g_env->vectorDtor(slots, 0x2c, at<u32>(slots - 4, 0), 0x005da3c0);
        GameHeap_Free(slots - 4);
    }
    at<u32>(source, 8) &= 0xffffffbf;
}

u32 *__fastcall SoundSource711b08_SetDistances(u8 *source, void *, u32 *out, u32 index, const u32 *vector) {
    if (!direct_sound(source)) {
        *out = 0xffffffff;
        return out;
    }
    enter(source);
    SoundSlot_SetDistances(slot_of(source, index), nullptr, out, vector);
    leave(source);
    return out;
}

u32 __fastcall SoundSource711b08_IsSet(u8 *source, void *, u32 index) {
    enter(source);
    const u32 result = SoundSlot_IsSet(slot_of(source, index)) & 0xff;
    leave(source);
    return result;
}

u32 *__fastcall SoundSource711b08_FreeSlot(u8 *source, void *, u32 *out, u32 index) {
    enter(source);
    SoundSlot_Free(slot_of(source, index), nullptr, out);
    leave(source);
    return out;
}

u32 *__fastcall SoundSource711b08_FreeAll(u8 *source, void *, u32 *out) {
    if (!direct_sound(source)) {
        *out = 0xffffffff;
        return out;
    }
    enter(source);
    u32 status;
    for (u32 i = 0; i < at<u32>(source, 4); ++i) SoundSlot_Free(slot_of(source, i), nullptr, &status);
    comcall<u32>(direct_sound(source), 0x34);
    *out = 0;
    leave(source);
    return out;
}

void __fastcall SoundSource711b08_Flush(u8 *source) {
    if (!direct_sound(source)) return;
    enter(source);
    ComList_ReleaseAll(reinterpret_cast<u8 *>(ReleaseList_Get()));
    comcall<u32>(direct_sound(source), 0x34);
    leave(source);
}

u32 *__fastcall SoundSource711b08_PlayWave(u8 *source, void *, u32 *out, u8 *info) {
    const u32 index = at<u32>(info, 0x2c) >> (global<u8>(0x00782db8) & 31);
    const u32 kind = at<u8>(info, 0x14) & 1 ? 7 : 6;
    enter(source);
    u8 *sound;
    if (!failed(comcall<u32>(direct_sound(source), 0x44, kind, &sound))) {
        u8 *const slot = slot_of(source, index);
        SoundSlot_Set(slot, nullptr, sound);
        u8 holder[0x1c];
        at<u16>(holder, 4) = 0;
        at<u8 *>(holder, 0x18) = info + 0x3c;
        SoundSlot_Call7(slot, nullptr, reinterpret_cast<uintptr_t>(WaveFormat_Build(holder)));
        const u32 size = Obj_Get20Or2400(info);
        u32 status;
        if ((s32)*SoundSlot_Call5(slot, nullptr, &status, size * 2) >= 0 && (s32)*SoundSlot_Call59(slot, nullptr, &status, 0, at<u32 *>(source, 0x2c)[index]) >= 0 &&
            (s32)*SoundSlot_Call59(slot, nullptr, &status, size, at<u32 *>(source, 0x2c)[index + at<u32>(source, 4)]) >= 0) {
            g_env->refill(source, nullptr, index + at<u32>(source, 4));
            SoundSlot_Restart(slot, nullptr, out);
            leave(source);
            return out;
        }
        SoundSlot_Free(slot, nullptr, &status);
    }
    *out = 0xffffffff;
    leave(source);
    return out;
}

u32 *__fastcall SoundSource711b08_SetPosition(u8 *source, void *, u32 *out, u32 index, const u32 *vector, u32) {
    enter(source);
    SoundSlot_SetPosition(slot_of(source, index), nullptr, out, vector);
    leave(source);
    return out;
}

u32 *__fastcall SoundSource711b08_SetPitch(u8 *source, void *, u32 *out, u32, u32 index, s32 steps) {
    enter(source);
    SoundSlot_SetPitch(slot_of(source, index), nullptr, out, steps);
    leave(source);
    return out;
}

u32 *__fastcall SoundSource711b08_SetGain(u8 *source, void *, u32 *out, u32 index, u32 gain) {
    enter(source);
    SoundSlot_SetGain(slot_of(source, index), nullptr, out, gain);
    leave(source);
    return out;
}

u32 *__fastcall SoundSource711b08_Set48(u8 *source, void *, u32 *out, u32 index, u32 value) {
    enter(source);
    SoundSlot_Set48(slot_of(source, index), nullptr, out, value);
    leave(source);
    return out;
}

u8 *__fastcall SoundSource711b08_DeletingDtor(u8 *source, void *, u32 flags) {
    SoundSource711b08_Dtor(source);
    if (flags & 1) GameHeap_Free(source);
    return source;
}

void __fastcall SoundSource711b08_Dtor(u8 *source) {
    at<u32>(source, 0) = (u32)(uintptr_t)SoundSource711b08_vtable;
    import(IMPORT(Kernel32_DeleteCriticalSection), source + 0x38);
    at<u32>(source, 0) = (u32)(uintptr_t)SoundSourceBase_vtable;
    SoundHelper_Free();
}
}
