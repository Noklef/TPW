#include "soundchannel.h"

#include <cstdint>

#include <emmintrin.h>

#include "../math/vector.h"
#include "../math/x87.h"
#include "../files/handles.h"
#include "channelvoice.h"
#include "dequeiter.h"
#include "fieldops.h"
#include "gameheap.h"
#include "initializers.h"
#include "ptrlist.h"
#include "selectors.h"
#include "sfman.h"
#include "soundbank.h"
#include "soundcategory.h"
#include "soundeax.h"
#include "soundindex.h"
#include "soundoccluders.h"
#include "string.h"
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

u8 *__fastcall SoundChannel_NewResource(u8 *channel, void *, const u32 *desc) {
    return SoundResource_New(at<u8 *>(channel, 0x1c), desc, channel);
}

void __fastcall SoundChannel_FreeEax(u8 *channel) {
    if (!at<u8 *>(channel, 0x20)) return;
    EaxBuffer_Release(at<u8 *>(channel, 0x20));
    GameHeap_Free(at<u8 *>(channel, 0x20));
    at<u8 *>(channel, 0x20) = nullptr;
}

u32 *__fastcall SoundChannel_OpenSynth(u8 *channel, void *, u32 *out) {
    u8 *host = static_cast<u8 *>(GameHeap_Alloc(8));
    if (host) host = Init_6c47b0(host);
    at<u8 *>(channel, 0x1c) = host;
    SfHost_OpenOut(host, nullptr, out);
    return out;
}

void __fastcall SoundChannel_FreeSynth(u8 *channel) {
    if (!at<u8 *>(channel, 0x1c)) return;
    SfHost_Dtor(at<u8 *>(channel, 0x1c));
    GameHeap_Free(at<u8 *>(channel, 0x1c));
    at<u8 *>(channel, 0x1c) = nullptr;
}

u32 *__fastcall SoundChannel_SetFlag2(u8 *channel, void *, u32 *out, u32 on) {
    const u32 flags = at<u32>(channel, 0x28);
    if (!(flags & 1)) {
        *out = 0xffffffff;
        return out;
    }
    if (!on != !(flags & 2)) {
        at<u32>(channel, 0x28) = on ? flags | 2 : flags & ~2u;
        u32 functor[2];   // (the second dword left unset, sic)
        functor[0] = 0x0070a5b0;
        vcall<void>(channel, 0x10, &on, functor);
        SoundCategory_ReleaseBankResources();
    }
    *out = 0;
    return out;
}

u8 *__fastcall SoundChannel_LoadBank(u8 *channel, void *, u32 index) {
    u8 *const category = SoundCategory_Get();
    u8 *bank = nullptr;
    if (index < at<u32>(category, 0x94)) {
        u32 it[4], found[4];
        for (int i = 0; i < 4; ++i) it[i] = at<u32>(category, 0x6c + 4 * i);
        bank = *reinterpret_cast<u8 **>(static_cast<uintptr_t>(DequeIter_Plus(it, nullptr, found, static_cast<s32>(index))[2]));
    }
    if (!bank) return nullptr;
    if (at<u8 *>(bank, 8)) return at<u8 *>(bank, 8);
    String path, name, empty;
    String_Ctor(&path);
    String_Ctor(&name);
    String_CtorStr(&empty, nullptr, reinterpret_cast<const char *>(static_cast<uintptr_t>(0x00786de4)));
    u8 *const opener = bank + 0x24;
    s32 status;
    vcall<s32 *>(opener, 0x10, &status, &name, &empty);
    String_Dtor(&empty);
    String_AssignWide(&path, nullptr, reinterpret_cast<const WString *>(bank + 0xc));
    u32 self;
    if (*reinterpret_cast<s32 *>(SoundBank_Open(&self, nullptr, reinterpret_cast<u32 *>(&status), opener, &path,
                                                static_cast<u32>(reinterpret_cast<uintptr_t>(&name)), Obj_Flags28_Bit1IfBit0(channel))) < 0) {
        String_Dtor(&name);
        String_Dtor(&path);
        return nullptr;
    }
    StreamBase stream;
    StreamBase_Ctor(&stream);
    u32 desc[4] = {};   // (+4: the original's is uninitialised on flags 1 and 2, sic; the loads ignore it)
    const auto open = [&](u32 mode, u32 type) {
        if (*vcall<s32 *>(opener, 0xc, &status, &stream, path.data, mode, 0u) < 0) return;
        desc[0] = type;
        desc[2] = static_cast<u32>(reinterpret_cast<uintptr_t>(&stream));
        desc[3] = static_cast<u32>(reinterpret_cast<uintptr_t>(path.data));
        at<u8 *>(bank, 8) = SoundChannel_NewResource(channel, nullptr, desc);
    };
    const u32 flags = at<u32>(bank, 0x20);
    if (flags & 1) {
        open(0x81000001, 2);
    } else if (flags & 2) {
        open(0x80008001, 1);
    } else if (flags & 4) {   // (a SoundFont: name + path up to its last ".", then ".sf2")
        String dir;
        String_CtorCopy(&dir, nullptr, reinterpret_cast<const StringView *>(&name));
        String_Append(&dir, nullptr, reinterpret_cast<const StringView *>(&path));
        const s32 dot = String_ReverseFind(&dir, nullptr, reinterpret_cast<const char *>(static_cast<uintptr_t>(0x00747800)), 0xffffffff);
        String_Assign(&dir, nullptr, String_Left(&dir, nullptr, reinterpret_cast<StringView *>(desc), static_cast<u32>(dot)));
        String_AppendStr(&dir, nullptr, reinterpret_cast<const char *>(static_cast<uintptr_t>(0x00782db0)));
        const u32 sf2 = Fn_6c58d0(reinterpret_cast<u8 *>(desc), nullptr, 3, static_cast<u32>(reinterpret_cast<uintptr_t>(dir.data)));
        at<u8 *>(bank, 8) = SoundChannel_NewResource(channel, nullptr, reinterpret_cast<const u32 *>(static_cast<uintptr_t>(sf2)));
        if (!at<u8 *>(bank, 8)) open(0x81000001, 2);
        String_Dtor(&dir);
    }
    u8 *const result = at<u8 *>(bank, 8);
    StreamBase_Dtor(&stream);
    String_Dtor(&name);
    String_Dtor(&path);
    return result;
}

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

void __fastcall SoundChannel_ReleaseVoices(u8 *channel) {
    u8 *const pool = at<u8 *>(channel, 0x14);
    u8 *voice = at<u8 *>(pool, 4);
    u8 *const end = voice + at<u32>(pool, 8) * 0x8c;
    if (!voice) return;
    do {
        u32 status;
        SoundChannel_ReleaseVoice(channel, nullptr, &status, voice, 0);
        voice += 0x8c;
    } while (voice < end);
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

namespace {

// ChannelVoices_Holds by the functor's handle (+4; a local); its index out the channel argument's own stack slot (sic).
bool holds(const u8 *functor, u8 *channel, u32 *index) {
    const u32 handle = at<u32>(functor, 4);
    *index = static_cast<u32>(reinterpret_cast<uintptr_t>(channel));
    return ChannelVoices_Holds(at<u8 *>(channel, 0x14), nullptr, &handle, index) != 0;
}

u32 *source_index(const u8 *functor, u32 *out, u8 *channel, u32 offset) {
    u32 index;
    if (!holds(functor, channel, &index)) {
        *out = 0xffffffff;
        return out;
    }
    vcall<void>(at<u8 *>(channel, 0x18), offset, out, index);
    return out;
}

}  // namespace

u32 *__fastcall SoundChannelFn70a2d0_Run(const u8 *functor, void *, u32 *out, u8 *channel) {
    u32 index;
    if (!holds(functor, channel, &index)) {
        *out = 0;
        return out;
    }
    SoundChannel_ReleaseVoice(channel, nullptr, out, at<u8 *>(at<u8 *>(channel, 0x14), 4) + index * 0x8c, 0);
    return out;
}

u32 *__fastcall SoundChannelFn70a5b8_Run(const u8 *functor, void *, u32 *out, u8 *channel) { return source_index(functor, out, channel, 0x2c); }

u32 *__fastcall SoundChannelFn70a5c0_Run(const u8 *functor, void *, u32 *out, u8 *channel) { return source_index(functor, out, channel, 0x30); }

u32 *__fastcall SoundChannelFn70a5b0_Run(const u8 *, void *, u32 *out, u8 *channel) {
    SoundChannel_ReleaseVoices(channel);
    *out = 0;
    return out;
}

u32 *__fastcall SoundChannelFn70a268_Run(const u8 *, void *, u32 *out, u8 *channel) {
    vcall<void>(at<u8 *>(channel, 0x18), 0x38, out);
    return out;
}

u32 *__fastcall SoundChannelFn70a270_Run(const u8 *, void *, u32 *out, u8 *channel) {
    vcall<void>(at<u8 *>(channel, 0x18), 0x3c, out);
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

void __fastcall SoundChannel_ClearOcclusion(u8 *channel, void *, u8 eax) {
    u8 *const pool = at<u8 *>(channel, 0x14);
    u8 *voice = at<u8 *>(pool, 4);
    u8 *const end = voice + at<u32>(pool, 8) * 0x8c;
    if (!voice) return;
    u32 properties[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0x3e800000, 0x3f000000, 0, 0x3f800000};   // (0.25, 0.5, 1.0)
    do {
        if (ChannelVoice_Sound(voice) && (at<u32>(voice, 0x10) & 1)) {
            at<u32>(voice, 0x10) &= ~1u;
            const float was = at<float>(voice, 0x28);
            if (was < 0.0f || was > 0.0f) {
                at<u32>(voice, 0x28) = 0;
                if (eax) {
                    vcall<void>(at<u8 *>(channel, 0x18), 0x48, voice_id(voice), properties);
                } else {
                    u32 local;
                    vcall<void>(at<u8 *>(channel, 0x18), 0x54, &local, voice_id(voice), 1.0f);
                }
            }
        }
        voice += 0x8c;
    } while (voice < end);
}

void __fastcall SoundChannel_UpdateVoiceOcclusion(u8 *channel, void *, u32 id, u8 *voice, const u8 *record) {
    u8 *const source = at<u8 *>(channel, 0x18);
    if (!source) return;
    float *const position = at<float *>(record, 0x30);
    if (!position) return;
    const u32 bit5 = at<u32>(record, 0xc) & 0x20;
    u32 local;
    vcall<void>(source, 0x74, &local, id, position, bit5);
    if (!(at<u32>(record, 0xc) & 0x800)) return;
    const u8 eax = at<u32>(channel, 0x20) != 0;
    if (!eax && !vcall<u8>(source, 0x4c)) return;
    if (!bit5) return;
    u32 properties[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0x3e800000, 0x3f000000, 0, 0x3f800000};   // (0.25, 0.5, 1.0)
    float occlusion;
    if (SoundOccluders_Occlusion(reinterpret_cast<u8 *>((uintptr_t)0x00fb2038), nullptr, reinterpret_cast<const float *>(channel + 8), position,
                                 &occlusion)) {
        const float was = at<float>(voice, 0x28);
        if (!(was < occlusion || was > occlusion)) return;
        at<float>(voice, 0x28) = occlusion;
        properties[5] = static_cast<u32>(fistp((float)(X87Reg(occlusion) * -2000.0f)));
        properties[7] = static_cast<u32>(fistp((float)(X87Reg(occlusion) * -1000.0f)));
        at<u32>(voice, 0x10) |= 1;
    } else {
        if (!(at<u8>(voice, 0x10) & 1)) return;
        const float was = at<float>(voice, 0x28);
        if (!(was < 0.0f || was > 0.0f)) return;
        at<u32>(voice, 0x28) = 0;
        at<u32>(voice, 0x10) &= ~1u;
    }
    if (eax) {
        vcall<void>(source, 0x48, id, properties);
        return;
    }
    const float half = 0.5f, level = (float)(X87Reg(1.0f) - occlusion);
    const float *const pick = half > level ? &half : &level;   // (by address: no maxss)
    vcall<void>(source, 0x54, &local, id, *pick);
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

u32 __fastcall SoundChannel_PickVoice(u8 *channel, void *, u32 a, u32 *picked, u8 *record) {
    const u32 voice = vcall<u32>(channel, 0x18, a, record);
    *picked = voice;
    if (!voice && (at<u32>(record, 0x14) & 0x2000)) return 0;
    return vcall<u32>(channel, 0x14, voice ? voice : a, record);
}

u32 __fastcall SoundChannel_PlayRecord(u8 *channel, void *, u8 *record, u32 *voice) {
    const u32 flags = at<u32>(channel, 0x28);
    if ((flags & 8) && (flags & 0x100)) at<u32>(record, 0xc) |= 2;
    const u32 result = SoundChannel_PickVoice(channel, nullptr, 0, voice, record);
    if (!result) return 0;
    if (!*voice) *voice = result;
    u32 functor[2] = {0x0070a618, *voice};
    u32 slot = (u32)(uintptr_t)record;   // (the out: the record argument's own stack slot, sic)
    if (*vcall<s32 *>(channel, 0x10, &slot, functor) < 0) return 0;
    return result;
}

u32 *__fastcall SoundChannel_SetOcclusion(u8 *channel, void *, u32 *out, u8 on, u32 radius) {
    const u8 was = global<u8>(0xfb2050);
    const float old = global<float>(0xfb204c), now = *reinterpret_cast<const float *>(&radius);
    if (was != on || old < now || old > now) {
        global<u32>(0xfb204c) = radius;
        global<u8>(0xfb2050) = on;
        const u8 eax = at<u32>(channel, 0x20) != 0;
        if (eax || vcall<u8>(at<u8 *>(channel, 0x18), 0x4c)) {
            if (!on && was) SoundChannel_ClearOcclusion(channel, nullptr, eax);
            else if (on && !was) SoundChannel_UpdateOcclusion(channel, nullptr, eax);
        }
    }
    *out = 0;
    return out;
}

u32 *__fastcall SoundChannel_SwitchOcclusion(u8 *channel, void *, u32 *out, u32 on, u32 radius) {
    u8 flag = 1;
    if (on && (at<u32>(channel, 0x28) & 0x42)) {
        at<u32>(channel, 0x28) |= 0x80;
    } else {
        flag = 0;
        at<u32>(channel, 0x28) &= ~0x80u;
    }
    SoundChannel_SetOcclusion(channel, nullptr, out, flag, radius);
    return out;
}

u32 *__fastcall SoundChannel_TryAddOccluder(u8 *channel, void *, u32 *out, u32 x, u32 y, u32 z, u32 radius, u8 **node) {
    *node = nullptr;
    if (at<u8>(channel, 0x28) & 0x41) {
        u8 *const made = SoundChannel_AddOccluder(channel, nullptr, x, y, z, radius);
        *node = made;
        if (made) {
            *out = 0;
            return out;
        }
    }
    *out = 0xffffffff;
    return out;
}

u8 *__fastcall SoundChannel_AddOccluder(u8 *channel, void *, u32 x, u32 y, u32 z, u32 radius) {
    const u8 eax = at<u32>(channel, 0x20) != 0;
    if (!eax && !vcall<u8>(at<u8 *>(channel, 0x18), 0x4c)) return nullptr;
    const u32 occluder[4] = {x, y, z, radius};
    u8 *const node = PtrList_InsertCopyByKey(reinterpret_cast<u8 *>(static_cast<uintptr_t>(0x00fb2038)), nullptr,
                                             reinterpret_cast<const u8 *>(occluder));
    if (node && global<u8>(0xfb2050)) SoundChannel_UpdateOcclusion(channel, nullptr, eax);
    return node;
}

u32 *__fastcall SoundChannel_RemoveOccluder(u8 *channel, void *, u32 *out, u8 **node) {
    if (*node) {
        const u8 eax = at<u32>(channel, 0x20) ? 1 : vcall<u8>(at<u8 *>(channel, 0x18), 0x4c);
        if (eax) {
            u32 status;
            SoundOccluders_Remove(reinterpret_cast<u8 *>(static_cast<uintptr_t>(0x00fb2038)), nullptr, &status, node);
            if (global<u8>(0xfb2050)) SoundChannel_UpdateOcclusion(channel, nullptr, eax);
            *out = status;
            return out;
        }
    }
    *out = 0;
    return out;
}

}  // extern "C"
