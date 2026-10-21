#include "soundvoice.h"

#include <cstdint>
#include <cstring>

#include "../files/file.h"
#include "../math/x87.h"
#include "fieldops.h"
#include "gameheap.h"
#include "initializers.h"
#include "soundcategory.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *system() { return global<u8 *>(0x00fb1f24); }

float as_float(u32 bits) {
    float f;
    std::memcpy(&f, &bits, 4);
    return f;
}
u32 float_bits(float f) {
    u32 bits;
    std::memcpy(&bits, &f, 4);
    return bits;
}

// The game's random number advanced (0xfb1f20), rotated right 13, its high word.
u32 next_random() {
    const u32 seed = global<u32>(0x00fb1f20) * 0x19660d + 0x3c6ef35f;
    global<u32>(0x00fb1f20) = seed;
    return (seed >> 13 | seed << 19) >> 16;
}

// A sound stopped: the sound system's slot 4 with the stack functor (0x70a2d0, the sound).
void stop_sound(u32 sound) {
    u32 out;
    u32 functor[2] = {0x0070a2d0, sound};
    vcall<void>(system(), 0x10, &out, functor);
}

// The record that 0x6bc900 and 0x6bcb80 hand to the sound system: the fields it reads cleared, three floats set (the others left unset,
// sic).
void start_record(u32 *record) {
    static const u8 kCleared[] = {0, 1, 3, 4, 5, 0xb, 0xc, 0xe, 0xf, 0x10, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x22};
    for (const u8 i : kCleared) record[i] = 0;
    record[0x20] = 0x3e800000;   // 0.25f
    record[0x21] = 0x3f000000;   // 0.5f
    record[0x23] = 0x3f800000;   // 1.0f
}

// The record handed to the sound system's slot 4 (out, a stack functor: vtable 0x70a850, the voice's sound, the record).
void send_record(const u8 *voice, u32 *out, u32 *record) {
    u32 functor[3] = {0x0070a850, at<u32>(voice, 0x24), static_cast<u32>(reinterpret_cast<uintptr_t>(record))};
    vcall<void>(global<u8 *>(0x00fb1f24), 0x10, out, functor);
}

}  // namespace

extern "C" {

void __fastcall SoundVoice_EnsureParams(u8 *voice) {
    if (at<u8 *>(voice, 0x4c)) return;
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x20));
    if (block) {
        for (u32 offset = 8; offset <= 0x18; offset += 4) at<u32>(block, offset) = 0;
        block[0] = 0;
        at<u32>(block, 4) = 0;
    }
    at<u8 *>(voice, 0x4c) = block;
}

u32 *__fastcall SoundVoice_SetPending(u8 *voice, void *, u32 *out, u32 value) {
    if (!at<u32>(voice, 0x24)) {
        at<u16>(voice, 0x48) = static_cast<u16>(value);
        at<u32>(voice, 0x30) |= 0x8000;
        *out = 0xffffffff;
        return out;
    }
    u32 record[0x27];
    start_record(record);
    SetThenFlag28_14_6c5130(reinterpret_cast<u8 *>(record), nullptr, value);
    send_record(voice, out, record);
    return out;
}

u32 __fastcall SoundVoice_SendByte44(u8 *voice) {
    if (!(at<u32>(voice, 0x30) & 0x4000)) {
        u32 record[0x28];
        start_record(record);
        const u8 value = voice[0x44];
        u32 result;   // (dropped)
        if (value == 0x64) {
            SetThenFlag28_14_6c5130(reinterpret_cast<u8 *>(record), nullptr, 0);
            send_record(voice, &result, record);
        } else {
            if (at<u32>(voice, 0x24)) {
                SetThenFlag9c_14_6c50d0(reinterpret_cast<u8 *>(record), nullptr, value);
                send_record(voice, &result, record);
            }
            if (at<u32>(voice, 0x24) && (record[6] & 2) && record[0x27]) return 0;
        }
    }
    at<u32>(voice, 0x30) = (at<u32>(voice, 0x30) & ~0x400u) | 0x100;
    return 1;
}

void __fastcall SoundVoice_ApplyVariation(u8 *voice) {
    const u8 *variation = at<u8 *>(voice, 8);
    if (!at<u16>(variation, 0x16) && !at<u16>(variation, 0x1a)) {
        u8 *const params = at<u8 *>(voice, 0x4c);
        if (params) params[1] = params[2] = params[3] = 0;
        return;
    }
    SoundVoice_EnsureParams(voice);
    at<u8 *>(voice, 0x4c)[1] = at<u8 *>(voice, 8)[0x16];
    at<u8 *>(voice, 0x4c)[2] = at<u8 *>(voice, 8)[0x1a];
    at<u8 *>(voice, 0x4c)[3] = 0;
}

double __fastcall SoundVoice_Radius(const u8 *voice) {
    if (voice[0x30] & 0x80) return (X87Reg(at<float>(system(), 0x2c)) * at<float>(voice, 0x40)).v;
    const u8 *const variation = at<u8 *>(voice, 8);
    return variation ? at<float>(variation, 0x22) : global<float>(0x0070a834);
}

u8 *__fastcall SoundVoice_PickSample(u8 *voice, void *, u8 avoidRepeat) {
    const u8 *const variation = at<u8 *>(voice, 8);
    if (!variation) return nullptr;
    u32 count = SfxVariation_SampleCount(variation);
    if (count == 1) return at<u8 *>(variation, 8);
    if (!count) return nullptr;
    const u32 r = next_random();
    u8 *const samples = at<u8 *>(at<u8 *>(voice, 8), 8);
    u8 *sample = samples;
    for (; at<u32>(sample, 4) < r; sample += 0x10)
        if (!--count) return nullptr;
    u8 index = static_cast<u8>(static_cast<s32>(sample - samples) >> 4);
    if (avoidRepeat && SfxVariation_SampleCount(at<u8 *>(voice, 8)) > 2 && index == voice[0x46]) {
        const u32 n = SfxVariation_SampleCount(at<u8 *>(voice, 8));
        index = static_cast<u8>(static_cast<u32>(static_cast<s32>(static_cast<s8>(index + 1))) % n);
        sample = at<u8 *>(at<u8 *>(voice, 8), 8) + static_cast<s32>(static_cast<s8>(index)) * 0x10;
    }
    voice[0x46] = index;
    return sample;
}

u8 __fastcall SoundVoice_CategoryEnabled(u8 *voice) {
    const u8 *const sys = system();
    const u8 categories = voice[0x47];
    bool enabled = true;
    for (u32 i = 0; i < 4 && enabled; ++i)
        if (categories >> i & 1 && !sys[0x44 + i]) enabled = false;
    if (enabled) {
        at<u32>(voice, 0x30) &= ~0x4000u;
        return 1;
    }
    if (at<u32>(voice, 0x30) & 0x4000) return 0;
    const u8 *const definition = at<u8 *>(voice, 4);
    if (definition && definition[0x10] & 0xc) at<u32>(voice, 0x30) |= 0x4000;
    if (at<u32>(voice, 0x24)) {
        stop_sound(at<u32>(voice, 0x24));
        at<u32>(voice, 0x24) = 0;
    }
    return 0;
}

u8 __fastcall SoundVoice_PickVariation(u8 *voice, void *, u8 avoidRepeat) {
    const u8 *const definition = at<u8 *>(voice, 4);
    if (!definition) return 0;
    if (at<u32>(definition, 4) == 1) {
        at<u8 *>(voice, 8) = at<u8 *>(definition, 8);
        return 1;
    }
    const u32 r = next_random();
    u8 *const variations = at<u8 *>(definition, 8);
    u8 *variation = variations;
    u32 sum = 0;
    for (u32 left = at<u32>(definition, 4);; variation += 0x2a) {
        if (!left) {
            at<u8 *>(voice, 8) = variations;
            return 1;
        }
        sum += at<u32>(variation, 0x1e);
        if (sum >= r) break;
        --left;
    }
    at<u8 *>(voice, 8) = variation;
    const u32 count = at<u32>(definition, 4);
    u8 index = static_cast<u8>(static_cast<s32>(variation - variations) / 0x2a);
    if (avoidRepeat && count > 2 && index == voice[0x45]) {
        index = static_cast<u8>(static_cast<u32>(static_cast<s32>(static_cast<s8>(index + 1))) % count);
        at<u8 *>(voice, 8) = variations + static_cast<s32>(static_cast<s8>(index)) * 0x2a;
    }
    voice[0x45] = index;
    SoundVoice_ApplyVariation(voice);
    return 1;
}

u8 __fastcall SoundVoice_UpdateAudible(u8 *voice) {
    if (at<u32>(voice, 0x30) & 0x1000) SoundVoice_FollowListener(voice);
    const float radius = static_cast<float>(SoundVoice_Radius(voice));
    const double distance = vcall<double>(voice, 0x1c);
    if (!((X87Reg(radius) * radius).v >= distance) && !(voice[0x30] & 8)) {   // (fcompp: below or unordered)
        SoundVoice_Cull(voice);
        return 0;
    }
    at<u32>(voice, 0x30) &= ~0x800u;
    vcall<void>(voice, 0x20, at<u32>(voice, 0x20));
    return 1;
}

void __fastcall SoundVoice_Cull(u8 *voice) {
    at<u32>(voice, 0x30) |= 0x800;
    if (voice[0x30] & 1 && at<u32>(voice, 0x24)) stop_sound(at<u32>(voice, 0x24));
}

u32 *__fastcall SoundVoice_Command(u8 *voice, void *, u32 *out, u8 *command) {
    if (at<u32>(voice, 0x24)) vcall<void>(system(), 0x10, &out, command);   // (out's own slot as the sound system's out, sic)
    if (vcall<u32>(command, 0) == 1) {
        at<u32>(voice, 0x30) |= 0x10;
    } else if (vcall<u32>(command, 0) == 2) {
        if (!at<u32>(voice, 0x3c)) {
            at<u32>(voice, 0x30) |= 0x2000;
            at<u32>(voice, 0x3c) = Time_GetMs();
        }
    } else if (vcall<u32>(command, 0) == 3 && at<u32>(voice, 0x30) & 0x2000) {
        at<u32>(voice, 0x1c) += Time_GetMs() - at<u32>(voice, 0x3c);
    }
    *out = 0;
    return out;
}

void __fastcall SoundVoice_FollowListener(u8 *voice) {
    const u32 *const listener = at<u32 *>(system(), 0x60);
    const s32 *const box = reinterpret_cast<const s32 *>(at<u8 *>(voice, 0x4c));
    u32 x = listener[0], z = listener[2];
    const u32 y = listener[1];
    if (!(as_float(x) >= static_cast<double>(box[2]))) {
        x = float_bits(static_cast<float>(box[2]));
    } else if (as_float(x) > static_cast<double>(box[4])) {
        x = float_bits(static_cast<float>(box[4]));
    }
    if (as_float(z) > static_cast<double>(box[3])) {
        z = float_bits(static_cast<float>(box[3]));
    } else if (!(as_float(z) >= static_cast<double>(box[5]))) {
        z = float_bits(static_cast<float>(box[5]));
    }
    Fn_6bb220(voice + 0x10, nullptr, x, y, z);
    if (!(voice[0x30] & 1)) return;
    u32 record[0x24];   // (the fields not named here are left unset, sic)
    static const u8 kCleared[] = {0, 1, 4, 0xb, 0xe, 0xf, 0x10, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x22};
    for (const u8 i : kCleared) record[i] = 0;
    record[0x20] = 0x3e800000;   // 0.25f
    record[0x21] = 0x3f000000;   // 0.5f
    record[0x23] = 0x3f800000;   // 1.0f
    record[0xc] = static_cast<u32>(reinterpret_cast<uintptr_t>(voice + 0x10));
    record[5] = 1;
    const u32 flags = at<u32>(system(), 0x28);
    record[3] = flags & 0x43 && flags & 0x80 ? 0x820 : 0x20;
    u32 out;
    u32 functor[3] = {0x0070a850, at<u32>(voice, 0x24), static_cast<u32>(reinterpret_cast<uintptr_t>(record))};
    vcall<void>(system(), 0x10, &out, functor);
}

u8 __fastcall SoundVoice_PickBranch(u8 *voice, void *, u32 value) {
    if (!at<u8 *>(voice, 8) && at<u8 *>(voice, 4)) {
        at<u8 *>(voice, 8) = at<u8 *>(at<u8 *>(voice, 4), 8);
        if (SfxVariation_SampleCount(at<u8 *>(voice, 8))) return 1;
    }
    const u8 *node = at<u8 *>(voice, 8);
    const u32 count = at<u32>(node, 4);
    if (!count) {
        at<u32>(voice, 8) = 0;
        return 0;
    }
    auto within = [value](const u8 *entry) { return entry[6] <= value && entry[7] >= value; };
    u32 matches = 0, sum = 0;
    for (const u8 *entry = at<const u8 *>(node, 0x26), *const end = entry + 8 * count; entry != end; entry += 8)
        if (within(entry)) {
            ++matches;
            sum += at<u32>(at<const u8 *>(entry, 0), 0x1e);
        }
    if (!matches) {
        at<u8>(at<u8 *>(voice, 0x4c), 4) = 0x7f;
        at<u32>(voice, 0x30) |= 0x40;
        return 0;
    }
    const u32 seed = global<u32>(0x00fb1f20) * 0x19660d + 0x3c6ef35f;
    global<u32>(0x00fb1f20) = seed;
    const u32 target = (seed >> 13 | seed << 19) % sum;
    node = at<u8 *>(voice, 8);
    const u8 *const entries = at<const u8 *>(node, 0x26);
    u32 run = 0;
    for (u32 i = 0; i < at<u32>(node, 4); ++i) {
        const u8 *const entry = entries + 8 * i;
        if (!within(entry)) continue;
        run += at<u32>(at<const u8 *>(entry, 0), 0x1e);
        if (run >= target) {
            at<u32>(voice, 8) = at<u32>(entry, 0);
            return 1;
        }
    }
    return 1;
}

u8 __fastcall SoundVoice_SwitchVariation(u8 *voice, void *, u32) {
    if (!SoundVoice_PickBranch(voice, nullptr, at<u8>(at<u8 *>(voice, 0x4c), 4))) return 0;
    SoundVoice_ApplyVariation(voice);
    return 1;
}

u8 __fastcall SoundVoice_NextVariation(u8 *voice, void *, u32) {
    const u8 *const definition = at<u8 *>(voice, 4);
    if (!definition) return 1;
    u8 *variation = at<u8 *>(voice, 8);
    if (!variation) {
        at<u8 *>(voice, 8) = at<u8 *>(definition, 8);
        return 1;
    }
    variation += 0x2a;
    at<u8 *>(voice, 8) = variation;
    const s32 index = static_cast<s32>(variation - at<u8 *>(definition, 8)) / 0x2a;
    if ((u32)index < at<u32>(definition, 4)) return 1;
    if (!(voice[0x30] & 4)) return 0;
    at<u8 *>(voice, 8) = at<u8 *>(definition, 8);
    return 1;
}

void __fastcall SoundVoice_SetFlags(u8 *voice, void *, u32 scale) {
    const u8 *const definition = at<u8 *>(voice, 4);
    at<u32>(voice, 0x30) = 0;
    if (definition[0x10] & 8) at<u32>(voice, 0x30) = 6;
    if (definition[0x10] & 4) at<u32>(voice, 0x30) |= 2;
    if (at<u16>(definition, 0x10) & 0x200) at<u32>(voice, 0x30) |= 8;
    if (definition[0x10] & 0x20) at<u32>(voice, 0x30) |= 0x200;
    if (definition[0x11] & 0x40) at<u32>(voice, 0x30) |= 0x20000;
    const float s = as_float(scale);
    if (s < 0.0f || s > 0.0f) {
        at<u32>(voice, 0x40) = scale;
        at<u32>(voice, 0x30) |= 0x80;
    }
}

u32 *__fastcall SoundVoice_Init(u8 *voice, void *, u32 *out, u8 *owner, u8 *definition, const u32 *position, u32 scale) {
    at<u8 *>(voice, 0x28) = owner;
    vcall<void>(owner, 4);
    at<u8 *>(voice, 4) = definition;
    at<u32>(voice, 0x10) = position[0];
    at<u32>(voice, 0x14) = position[1];
    at<u32>(voice, 0x18) = position[2];
    voice[0x47] = at<u8 *>(owner, 8)[8];
    if (at<u16>(definition, 0x12)) {
        SoundVoice_EnsureParams(voice);
        at<u8 *>(voice, 0x4c)[0] = at<u8 *>(voice, 4)[0x12];
    }
    SoundVoice_SetFlags(voice, nullptr, scale);
    if (vcall<u8>(voice, 0x38, 0u) || at<u32>(voice, 8)) {
        SoundVoice_ApplyVariation(voice);
        if (at<u32>(voice, 0xc) || (at<u32>(voice, 0xc) = vcall<u32>(voice, 0x3c, 0u)) != 0) {
            at<u32>(voice, 0x1c) = Time_GetMs() + at<u32>(at<u8 *>(voice, 0xc), 8) + 0xfa;
            *out = 0;
            return out;
        }
    }
    vcall<void>(at<u8 *>(voice, 0x28), 0);
    at<u32>(voice, 0x28) = 0;
    *out = 0xffffffff;
    return out;
}

u8 *__fastcall SwitchVoice_Ctor(u8 *voice) {
    Class70a7e0_Ctor(voice);
    at<u32>(voice, 0) = 0x0070a498;
    return voice;
}

u8 *__fastcall SequenceVoice_Ctor(u8 *voice) {
    Class70a7e0_Ctor(voice);
    at<u32>(voice, 0) = 0x0070a440;
    return voice;
}

}  // extern "C"
