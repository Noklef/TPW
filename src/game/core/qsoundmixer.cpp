// The QSound mixer source's calls on the QMixer library: see qsoundmixer.h.
#include "qsoundmixer.h"

#include <cstdint>
#include <emmintrin.h>

#include "../math/x87.h"
#include "gameheap.h"
#include "methods.h"
#include "sounddevice.h"
#include "soundsource.h"
#include "vcall.h"
#include "waveformat.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// The library's calls, through the import table (0x6fd294-0x6fd2fc).
template <typename... A> u32 library(u32 slot, A... args) { return (*reinterpret_cast<u32(__stdcall *const *)(A...)>((uintptr_t)slot))(args...); }

u32 &session(u8 *source) { return at<u32>(source, 0x18); }
bool is_open(u8 *source) { return vcall<u32>(source, 8) != 0; }
bool swap_axes(const u8 *source) { return !(at<u8>(source, 8) & 1); }
bool is_nan(u32 bits) { return (bits & 0x7fffffff) > 0x7f800000; }   // (fld; fstp double; _isnan 0x67b410)
bool any_nan(const u32 *v) { return is_nan(v[0]) || is_nan(v[1]) || is_nan(v[2]); }

u32 *answer(u32 *out, u32 result) {
    *out = result ? 0xffffffff : 0;
    return out;
}
u32 *fail(u32 *out) {
    *out = 0xffffffff;
    return out;
}

u32 *channel_call(u8 *source, u32 *out, u32 slot, u32 channel) {
    if (!is_open(source)) return fail(out);
    return answer(out, library(slot, session(source), channel, 0u));
}

void axes(const u8 *source, u32 *to, const u32 *from) {
    to[0] = from[0];
    const bool swap = swap_axes(source);
    to[1] = from[swap ? 2 : 1];
    to[2] = from[swap ? 1 : 2];
}

}  // namespace

extern "C" {

u32 *__fastcall SoundSource711920_Open(u8 *source, void *, u32 *out, const u8 *info) {
    u32 slot;
    SoundSource711920_Configure(source, nullptr, &slot, info);
    const u32 handle = library(0x006fd2ec, at<u32>(source, 0x20));
    session(source) = handle;
    if (!handle || library(0x006fd2f0, handle, 1u)) return fail(out);
    SoundDevice_CreatePrimaryOf(SoundDevice_Get(), nullptr, &slot);
    if (library(0x006fd2fc, session(source), at<u32>(source, 4), 2u)) return fail(out);
    u32 zero[3] = {0, 0, 0};
    library(0x006fd2f8, session(source), zero, 0u);
    u32 placed;
    slot = 0x0070a27c;
    vcall<u32 *>(source, 0x64, &placed, &slot);
    SoundSource711920_SetFlags(source);
    *out = 0;
    return out;
}

u32 *__fastcall SoundSource711920_SetOptions(u8 *source, void *, u32 *out, u32 flags) {
    u32 mask = flags & 4 ? 1 : 0;
    if (flags & 8) mask |= 2;
    if (flags & 2) mask |= 8;
    return answer(out, library(0x006fd2f4, session(source), mask, 0xffffffffu));
}

u32 __fastcall SoundSource711920_GetBuffer(u8 *source, void *, u32 which, u32 *out) {
    *out = 0;
    if (!session(source)) return 0;
    const u32 result = library(0x006fd2e8, session(source), which, out);
    if (result) *out = 0;
    return result;
}

void __fastcall SoundSource711920_Close(u8 *source) {
    if (!session(source)) return;
    library(0x006fd2e4, session(source));
    session(source) = 0;
}

u32 *__fastcall SoundSource711920_EnableChannel(u8 *source, void *, u32 *out, u32 channel, u32 flags) {
    library(0x006fd2dc, session(source), channel, 0u, flags & 0x400 ? 0x1f4u : 0x21u);
    return answer(out, library(0x006fd2e0, session(source), channel, 8u, Obj_RemapFlags(source, nullptr, flags)));
}

u32 *__fastcall SoundSource711920_PlayWave(u8 *source, void *, u32 *out, u8 *info) {
    if (!is_open(source)) return fail(out);
    const u32 channel = at<u32>(info, 0x2c) >> (global<u8>(0x00782db8) & 31);
    const u32 bits = at<u32>(info, 0x14);
    u8 holder[0x1c];
    at<u16>(holder, 4) = 0;
    at<u8 *>(holder, 0x18) = info + 0x3c;
    library(0x006fd2cc, session(source), channel, at<u32>(source, 0x24) | (bits & 1 ? 0x1000000u : 0x2000000u), WaveFormat_Build(holder), 0u);
    struct {
        u8 *format;
        u32 size;
        u32 callback;
        u8 *info;
        u32 zero;
    } wave_info = {WaveFormat_Build(holder), Obj_Get20Or2400(info), 0x006bb4f0, info, 0};
    const u32 wave = library(0x006fd2d0, session(source), &wave_info, 0x11u);
    if (!wave) return fail(out);
    u32 status;
    if ((s32)*vcall<u32 *>(source, 0x68, &status, channel, bits) >= 0) {
        const u32 older = at<u32 *>(source, 0x1c)[channel];
        if (older) library(0x006fd2d4, session(source), older);
        at<u32 *>(source, 0x1c)[channel] = wave;
        if (!library(0x006fd2d8, session(source), channel, 0x20u, wave, 0u, 0u)) {
            *out = 0;
            return out;
        }
        at<u32 *>(source, 0x1c)[channel] = 0;
    }
    library(0x006fd2d4, session(source), wave);
    return fail(out);
}

u32 *__fastcall SoundSource711920_StopChannel(u8 *source, void *, u32 *out, u32 channel) { return channel_call(source, out, 0x006fd2c8, channel); }
u32 *__fastcall SoundSource711920_PauseChannel(u8 *source, void *, u32 *out, u32 channel) { return channel_call(source, out, 0x006fd2c4, channel); }
u32 *__fastcall SoundSource711920_RestartChannel(u8 *source, void *, u32 *out, u32 channel) { return channel_call(source, out, 0x006fd2c0, channel); }

u32 *__fastcall SoundSource711920_StopAll(u8 *source, void *, u32 *out) { return answer(out, library(0x006fd2c8, session(source), 0u, 1u)); }
u32 *__fastcall SoundSource711920_PauseAll(u8 *source, void *, u32 *out) { return answer(out, library(0x006fd2c4, session(source), 0u, 1u)); }
u32 *__fastcall SoundSource711920_RestartAll(u8 *source, void *, u32 *out) { return answer(out, library(0x006fd2c0, session(source), 0u, 1u)); }

u32 *__fastcall SoundSource711920_GetSourcePosition(u8 *source, void *, u32 *out, u32 channel, u32 *vector) {
    u32 position[3];
    if (channel >= at<u32>(source, 4) || library(0x006fd2bc, session(source), channel, position)) return fail(out);
    vector[0] = position[0];
    vector[1] = position[2];
    vector[2] = position[1];
    *out = 0;
    return out;
}

u32 *__fastcall SoundSource711920_SetSourcePosition(u8 *source, void *, u32 *out, u32 channel, const u32 *vector, u32 relative) {
    if (channel >= at<u32>(source, 4)) return fail(out);
    u32 position[3] = {vector[0], vector[1], vector[2]};
    if (!relative) return answer(out, library(0x006fd2b8, session(source), channel, 0u, position));
    if (any_nan(vector)) return fail(out);
    axes(source, position, vector);
    return answer(out, library(0x006fd2b4, session(source), channel, 0u, position));
}

u32 *__fastcall SoundSource711920_SetListenerVelocity(u8 *source, void *, u32 *out, const float *from, const float *to) {
    float velocity[3];
    velocity[0] = (float)(X87Reg(to[0]) - from[0]);
    if (!swap_axes(source)) {
        const X87Reg y = X87Reg(to[1]) - X87Reg(from[1]);
        velocity[2] = (float)(X87Reg(to[2]) - from[2]);
        velocity[1] = (float)y;
    } else {
        const X87Reg z = X87Reg(to[2]) - X87Reg(from[2]);
        velocity[2] = (float)(X87Reg(to[1]) - from[1]);
        velocity[1] = (float)z;
    }
    return answer(out, library(0x006fd2b0, session(source), velocity, 0u));
}

u32 *__fastcall SoundSource711920_SetFrequencyStep(u8 *source, void *, u32 *out, u32 frequency, u32 channel, s32 step) {
    u32 value = frequency;
    if (step) {
        const float factor = global<float>(step > 0 ? (u32)(uintptr_t)&gQsoundmixer_782f40 + 4 * step : ((u32)(uintptr_t)&gQsoundmixer_782f40 + 0x600) - 4 * step);
        const float scaled = static_cast<float>(X87_MulUIntDouble(frequency, factor));
        value = static_cast<u32>(_mm_cvtss_si32(_mm_set_ss(scaled)));   // (fld; fistp)
    }
    return answer(out, library(0x006fd2ac, session(source), channel, 0u, value));
}

u32 *__fastcall SoundSource711920_SetFrequency(u8 *source, void *, u32 *out, u32 channel, u32 frequency) {
    return answer(out, library(0x006fd2ac, session(source), channel, 0u, frequency));
}

u32 *__fastcall SoundSource711920_SetSourceCone(u8 *source, void *, u32 *out, u32 channel, const u32 *cone) {
    u32 moved[3] = {X87_FloatThrough(cone[0]), X87_FloatThrough(cone[1]), X87_FloatThrough(cone[2])};
    u32 vector[3];
    axes(source, vector, moved);
    return answer(out, library(0x006fd2a8, session(source), channel, 0u, vector, cone[3], cone[4]));
}

u32 *__fastcall SoundSource711920_SetDistanceMapping(u8 *source, void *, u32 *out, u32 channel, const u32 *mapping) {
    u32 block[4] = {0x10, mapping[0], mapping[1], mapping[2]};
    return answer(out, library(0x006fd2a4, session(source), channel, 0u, block));
}

u32 *__fastcall SoundSource711920_SetVolume(u8 *source, void *, u32 *out, u32 channel, s32 volume) {
    if (!is_open(source) || channel > at<u32>(source, 4) || volume < 0 || volume > 0x7f) return fail(out);
    return answer(out, library(0x006fd2a0, session(source), channel, 0u, (u32)gQsoundmixer_782e40[volume]));
}

u8 *__fastcall SoundSourceBase_DeletingDtor(u8 *source, void *, u32 flags) {
    SoundSourceBase_Dtor(source);
    if (flags & 1) GameHeap_Free(source);
    return source;
}

u32 *__fastcall SoundSource711920_SetListenerPosition(u8 *source, void *, u32 *out, const u32 *previous, const u32 *vector, u32 moving, u32 follow) {
    if (!is_open(source)) return fail(out);
    if (!moving && follow) {
        u32 status;
        if ((s32)*vcall<u32 *>(source, 0x84, &status, previous, vector) < 0) return fail(out);
    }
    if (any_nan(vector)) return fail(out);
    u32 position[3];
    axes(source, position, vector);
    return answer(out, library(0x006fd2f8, session(source), position, 0u));
}

u32 *__fastcall SoundSource711920_SetListenerOrientation(u8 *source, void *, u32 *out, const u32 *front, const u32 *up) {
    if (!is_open(source) || any_nan(front) || any_nan(up)) return fail(out);
    u32 front_axes[3], up_axes[3];
    axes(source, front_axes, front);
    axes(source, up_axes, up);
    return answer(out, library(0x006fd298, session(source), front_axes, up_axes, 0u));
}

u32 *__fastcall SoundSource711920_SetPanRateAll(u8 *source, void *, u32 *out, u32 rate) {
    if (!is_open(source)) return fail(out);
    return answer(out, library(0x006fd2dc, session(source), 0u, 1u, rate));
}

u32 *__fastcall SoundSource711920_SetSpeedOfSound(u8 *source, void *, u32 *out, u32 speed) {
    if (!is_open(source)) return fail(out);
    return answer(out, library(0x006fd29c, session(source), speed, 0u));
}

u32 *__fastcall SoundSource711920_SetSpeakerPlacement(u8 *source, void *, u32 *out, u8 *object) {
    if (!is_open(source)) return fail(out);
    const u32 placement = vcall<u32>(object, 0);
    return answer(out, library(0x006fd294, session(source), placement));
}
}
