#include "soundchannel.h"

#include <cstdint>

#include <emmintrin.h>

#include "../math/vector.h"
#include "../math/x87.h"
#include "channelvoice.h"
#include "fieldops.h"
#include "initializers.h"
#include "soundeax.h"
#include "soundoccluders.h"
#include "thunks.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// 0x6b7910 / 0x6b7970 without an EAX buffer: the source's slot 0x14 (out, settings + 0xc) when its slot 0x13 answers nonzero.
u32 *source_eax(u8 *channel, u32 *out, u8 *settings) {
    u8 *const source = at<u8 *>(channel, 0x18);
    if (source && vcall<u8>(source, 0x4c)) {
        vcall<void>(at<u8 *>(channel, 0x18), 0x50, out, settings + 0xc);
        return out;
    }
    *out = 0xffffffff;
    return out;
}

// A voice's id as the source knows it: +0x2c shifted right by the byte at 0x782db8.
u32 voice_id(const u8 *voice) { return at<u32>(voice, 0x2c) >> (global<u8>(0x00782db8) & 31); }

// A settings record (0x70 bytes on the stack, Init_6b9c50's fields set) applied with the functor 0x70a278.
u32 *apply_settings(u8 *channel, u32 *out, u8 *record) {
    u32 functor[2] = {0x0070a278, (u32)(uintptr_t)record};
    SoundChannel_Apply(channel, nullptr, out, reinterpret_cast<u8 *>(functor));
    return out;
}

// fld; fistp (to nearest; out of range 0x80000000, as cvtss2si).
s32 fistp(float x) { return _mm_cvtss_si32(_mm_set_ss(x)); }

}  // namespace

extern "C" {

u32 *__fastcall SoundChannel_Apply(u8 *channel, void *, u32 *out, u8 *functor) {
    vcall<void>(functor, 0, out, channel);
    return out;
}

u32 *__fastcall SoundChannelFn70a260_Run(const u8 *, void *, u32 *out, u8 *channel) {
    vcall<void>(at<u8 *>(channel, 0x18), 4);
    *out = 0;
    return out;
}

u32 *__fastcall SoundChannel_GetSettings(u8 *channel, void *, u32 *out, u8 *settings) {
    u8 *const record = settings;   // (the argument's slot is EaxBuffer_QueryAll's out below)
    if (at<u32>(record, 4) & 0x100) {
        bool clear = at<u32>(channel, 0x20) != 0;
        if (!clear) {
            u8 *const source = at<u8 *>(channel, 0x18);
            clear = source && !vcall<u8>(source, 0x4c);
        }
        if (clear) at<u32>(record, 4) &= ~0x100u;
    }
    if ((at<u32>(record, 4) & 0x200) && !at<u32>(channel, 0x20)) at<u32>(record, 4) &= ~0x200u;
    const u32 asked = at<u32>(record, 8);
    if ((asked & 0x100) && at<u8 *>(channel, 0x14)) {
        const u32 value = at<u32>(at<u8 *>(channel, 0x14), 0);
        const u32 flags = at<u32>(record, 0x4c) | 0x100;
        at<u32>(record, 0x44) = value;
        at<u32>(record, 0x4c) = flags;
    }
    if ((asked & 0x80) && at<u8 *>(channel, 0x14)) {
        at<u32>(record, 0x40) = at<u32>(at<u8 *>(channel, 0x14), 8);
        at<u32>(record, 0x4c) |= 0x80;
    }
    if (asked & 1) {
        vcall<void>(at<u8 *>(channel, 0x18), 0xc, record);
        if (at<u32>(record, 0)) at<u32>(record, 0x4c) |= 1;
    }
    if ((record[8] & 0x40) && at<u8 *>(channel, 0x20)) {
        const u32 *const status = EaxBuffer_QueryAll(at<u8 *>(channel, 0x20), nullptr, reinterpret_cast<u32 *>(&settings), 0);
        if (static_cast<s32>(*status) >= 0) at<u32>(record, 0x4c) |= 0x40;
    }
    if (at<u32>(record, 8) & 0x400) {
        at<u32>(record, 0x6c) = global<u32>(0x00fb2058);
        at<u32>(record, 0x4c) |= 0x400;
    }
    *out = 0;
    return out;
}

u32 *__fastcall SoundChannel_SetEaxProperty0b(u8 *channel, void *, u32 *out, u8 *settings) {
    if (u8 *const eax = at<u8 *>(channel, 0x20)) {
        EaxBuffer_SetProperty0b(eax, nullptr, out, at<u32>(settings, 0x60));
        return out;
    }
    return source_eax(channel, out, settings);
}

u32 *__fastcall SoundChannel_SetEaxAll(u8 *channel, void *, u32 *out, u8 *settings) {
    if (u8 *const eax = at<u8 *>(channel, 0x20)) {
        EaxBuffer_SetAll(eax, nullptr, out, reinterpret_cast<const u32 *>(settings + 0xc));
        return out;
    }
    return source_eax(channel, out, settings);
}

u32 *__fastcall SoundChannel_ReleaseVoice(u8 *channel, void *, u32 *out, u8 *voice, u32 value) {
    u8 *const target = voice;   // (the argument's slot is the source's out below)
    if (target) {
        const u32 id = at<u32>(target, 0x2c) >> (global<u8>(0x00782db8) & 31);
        if (static_cast<s32>(*vcall<u32 *>(at<u8 *>(channel, 0x18), 0x28, reinterpret_cast<u32 *>(&voice), id)) >= 0)
            ChannelVoice_Release(target, nullptr, value);
    }
    *out = 0xffffffff;
    return out;
}

u32 *__fastcall SoundChannelFn70a278_Run(const u8 *functor, void *, u32 *out, u8 *channel) {
    u32 local;   // (the original's is the channel argument's own slot)
    auto record = [functor] { return at<u8 *>(functor, 4); };
    if (at<u32>(record(), 4)) vcall<void>(at<u8 *>(channel, 0x18), 0x14, &local, at<u32>(record(), 4));
    const u32 flags = at<u32>(record(), 0x4c);
    if (flags) {
        if (flags & 0x80) ChannelVoices_SetCount(at<u8 *>(channel, 0x14), nullptr, record());
        if (at<u8>(record(), 0x4c) & 4) SoundChannel_SetPosition(channel, nullptr, &local, record());
        if (at<u32>(record(), 0x4c) & 8) {
            const float *const v = at<const float *>(record(), 0x54);
            const X87Reg x = v[0], y = v[1], z = v[2];
            float copy[3] = {v[0], v[1], v[2]};
            const float scale = (float)(1.0f / X87_Sqrt(x * x + y * y + z * z));
            Vec3_Scale(copy, nullptr, scale);
            vcall<void>(at<u8 *>(channel, 0x18), 0x8c, out, copy, at<u32>(record(), 0x58));
            return out;   // (sic: *out not set)
        }
        if (at<u32>(record(), 0x4c) & 0x10) vcall<void>(at<u8 *>(channel, 0x18), 0x64, &local, at<u32>(record(), 0x5c));
        if (at<u32>(record(), 0x4c) & 0x200) {
            Wrap_Globalfb2708_Set_6d1bc0_6baa10(at<u32>(record(), 0x68));
            vcall<void>(at<u8 *>(channel, 0x18), 0x40, &local, at<u32>(record(), 0x68));
        }
        if (at<u8>(record(), 0x4c) & 0x20) SoundChannel_SetEaxProperty0b(channel, nullptr, &local, record());
        if (at<u8>(record(), 0x4c) & 0x40) SoundChannel_SetEaxAll(channel, nullptr, &local, record());
    }
    *out = 0;
    return out;
}

u32 *__fastcall SoundChannel_SetPosition(u8 *channel, void *, u32 *out, u8 *settings) {
    u32 local;   // (the original's is the settings argument's own slot)
    const u32 *const status = vcall<u32 *>(at<u8 *>(channel, 0x18), 0x88, &local, channel + 8, at<u32>(settings, 0x50), 0u,
                                           (~at<u32>(settings, 4) >> 6) & 1);
    if (static_cast<s32>(*status) < 0) {
        *out = 0xffffffff;
        return out;
    }
    if (const u32 *const p = at<const u32 *>(settings, 0x50)) {
        at<u32>(channel, 8) = p[0];
        at<u32>(channel, 0xc) = p[1];
        at<u32>(channel, 0x10) = p[2];
    }
    if (at<u32>(settings, 4) & 0x800) {
        const u8 eax = at<u32>(channel, 0x20) != 0;
        if (eax || vcall<u8>(at<u8 *>(channel, 0x18), 0x4c)) SoundChannel_UpdateOcclusion(channel, nullptr, eax);
    }
    *out = 0;
    return out;
}

void __fastcall SoundChannel_UpdateOcclusion(u8 *channel, void *, u8 eax) {
    u8 *const pool = at<u8 *>(channel, 0x14);
    u8 *voice = at<u8 *>(pool, 4);
    u8 *const end = voice + at<u32>(pool, 8) * 0x8c;
    if (!voice) return;
    do {
        if (ChannelVoice_Sound(voice) && (at<u32>(voice, 0x14) & 0x21) == 0x20) {
            u32 local;
            float position[3] = {0.0f, 0.0f, 0.0f};
            vcall<void>(at<u8 *>(channel, 0x18), 0x78, &local, voice_id(voice), position);
            u32 properties[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0x3e800000, 0x3f000000, 0, 0x3f800000};   // (0.25, 0.5, 1.0)
            float occlusion;
            bool changed = false;
            if (SoundOccluders_Occlusion(reinterpret_cast<u8 *>((uintptr_t)0x00fb2038), nullptr, reinterpret_cast<const float *>(channel + 8),
                                         position, &occlusion)) {
                const float was = at<float>(voice, 0x28);
                if (was < occlusion || was > occlusion) {
                    at<float>(voice, 0x28) = occlusion;
                    properties[5] = static_cast<u32>(fistp((float)(X87Reg(occlusion) * -2000.0f)));
                    properties[7] = static_cast<u32>(fistp((float)(X87Reg(occlusion) * -1000.0f)));
                    ChannelVoice_SetFlag(voice, nullptr, 1, 1);
                    changed = true;
                }
            } else if (at<u8>(voice, 0x10) & 1) {
                const float was = at<float>(voice, 0x28);
                if (was < 0.0f || was > 0.0f) {
                    at<u32>(voice, 0x28) = 0;
                    ChannelVoice_SetFlag(voice, nullptr, 0, 1);
                    changed = true;
                }
            }
            if (changed) {
                if (eax) {
                    vcall<void>(at<u8 *>(channel, 0x18), 0x48, voice_id(voice), properties);
                } else {
                    const float half = 0.5f, level = (float)(X87Reg(1.0f) - occlusion);
                    const float *const pick = half > level ? &half : &level;   // (by address: no maxss)
                    vcall<void>(at<u8 *>(channel, 0x18), 0x54, &local, voice_id(voice), *pick);
                }
            }
        }
        voice += 0x8c;
    } while (voice < end);
}

u32 *__fastcall SoundChannel_SetMode(u8 *channel, void *, u32 *out, u32 a, u32 b, u32 c) {
    if (a)
        at<u32>(channel, 0x28) |= 8;
    else
        at<u32>(channel, 0x28) &= ~8u;
    u8 record[0x70];
    Init_6b9c50(record);
    if (b) {
        at<u32>(channel, 0x28) |= 0x100;
        at<u32>(record, 4) |= 4;
    }
    if (c) at<u32>(record, 4) |= 8;
    return apply_settings(channel, out, record);
}

u32 *__fastcall SoundChannel_SetPlacement(u8 *channel, void *, u32 *out, u32 position, u32 direction, u32 value) {
    u8 record[0x70];
    Init_6b9c50(record);
    at<u32>(channel, 0x60) = position;
    SetThenFlag50_4c_6b9c70(record, nullptr, position);
    Obj_Set54And58Flag8(record, nullptr, direction, value);
    const u32 flags = at<u32>(channel, 0x28);
    if ((flags & 0x43) && (flags & 0x80)) at<u32>(record, 4) |= 0x800;
    return apply_settings(channel, out, record);
}

}  // extern "C"
