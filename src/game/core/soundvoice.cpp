#include "soundvoice.h"

#include <cstdint>
#include <cmath>
#include <cstring>
#include <xmmintrin.h>

#include "../files/file.h"
#include "../math/vector.h"
#include "../math/x87.h"
#include "crt.h"
#include "fieldops.h"
#include "gameheap.h"
#include "initializers.h"
#include "selectors.h"
#include "shufflebag.h"
#include "soundcategory.h"
#include "soundchannel.h"
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

// The variation's byte range at +lo / +lo + 1 put in order: an XOR swap through the variation, +8 re-read at each step.
template <typename T> void order_range(u8 *voice, u32 lo) {
    u8 *v = at<u8 *>(voice, 8);
    if (static_cast<T>(v[lo + 1]) >= static_cast<T>(v[lo])) return;
    v[lo + 1] = v[lo] ^ v[lo + 1];
    v = at<u8 *>(voice, 8);
    v[lo] = v[lo] ^ v[lo + 1];
    v = at<u8 *>(voice, 8);
    v[lo + 1] = v[lo + 1] ^ v[lo];
}

// The game's random number advanced (0xfb1f20) and rotated right 13, all of it.
u32 next_random_full() {
    const u32 seed = global<u32>(0x00fb1f20) * 0x19660d + 0x3c6ef35f;
    global<u32>(0x00fb1f20) = seed;
    return seed >> 13 | seed << 19;
}

// A roll in the variation's range (+lo, +lo + 1 of type T): the parameter's percentage (slot 16 with key) or a random draw.
template <typename T> u32 roll(u8 *voice, u32 lo, u32 key) {
    order_range<T>(voice, lo);
    const u8 *v = at<u8 *>(voice, 8);
    u32 range = static_cast<u32>(static_cast<s32>(static_cast<T>(v[lo + 1])) - static_cast<s32>(static_cast<T>(v[lo])));
    if (at<u32>(voice, 0x4c)) {
        u32 percent;
        if (vcall<bool>(voice, 0x40, key, &percent))
            return percent * range / 100 + static_cast<u32>(static_cast<s32>(static_cast<T>(at<u8 *>(voice, 8)[lo])));
    }
    if (range) range = next_random_full() % range;
    return range + static_cast<u32>(static_cast<s32>(static_cast<T>(at<u8 *>(voice, 8)[lo])));
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

// 0x6bd800 / 0x6bdf50: the voice started (counted: with flag 4 the record's +0x44 = 1).
u32 *start_voice(u8 *voice, u32 *out, u32 now, bool counted) {
    if (!global<u8 *>(0x00fb1f24) || !at<u32>(voice, 8)) {
        *out = 0xffffffff;
        return out;
    }
    if (at<u32>(voice, 0x30) & 0x11) {
        *out = 0;
        return out;
    }
    at<u32>(voice, 0x30) |= 1;
    u32 record[0x27];
    start_record(record);
    if (!vcall<u32>(voice, 0x44, now, record)) {
        at<u32>(voice, 0x30) |= 0x10;
        *out = 0xffffffff;
        return out;
    }
    if (counted && (voice[0x30] & 4)) SetThenFlag44_14_6c5110(reinterpret_cast<u8 *>(record), nullptr, 1);
    if (!SoundChannel_PlayRecord(global<u8 *>(0x00fb1f24), nullptr, reinterpret_cast<u8 *>(record), &at<u32>(voice, 0x24)))
        at<u32>(voice, 0x30) &= ~1u;
    *out = 0;
    return out;
}

// 0x6be610 / 0x6be680's end: the voice's slot 4 (out: the factory's memory argument's own stack slot, sic).
u8 *init_voice(u8 *voice, u32 *out, u8 *owner, u8 *definition, u8 *info) {
    if (!voice) return nullptr;
    if (static_cast<s32>(*vcall<u32 *>(voice, 0x10, out, owner, definition, info + 0xc, at<u32>(info, 0x18))) < 0) return nullptr;
    return voice;
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

double __fastcall SoundVoice_Distance(u8 *voice) {
    at<u16>(voice, 0x22) = at<u16>(at<u8 *>(voice, 4), 0xc);
    float distance = 0.0f;
    if (const float *const listener = at<const float *>(system(), 0x60)) {
        const X87Reg dx = std::fabs((X87Reg(at<float>(voice, 0x10)) - listener[0]).v);
        const X87Reg dz = std::fabs((X87Reg(at<float>(voice, 0x18)) - listener[2]).v);
        distance = static_cast<float>(dx * dx + dz * dz);
    }
    if (!(distance > global<float>(0x0070a834))) {
        at<u16>(voice, 0x20) = 0xffff;
        return distance;
    }
    const float scaled = static_cast<float>(X87Reg(distance) * global<float>(0x0070a838));
    const u32 level = static_cast<u32>(_mm_cvtss_si32(_mm_set_ss(scaled)));   // fistp
    if (level <= 0xffff) at<u16>(voice, 0x20) = static_cast<u16>(0xffff - level);
    return distance;
}

void __fastcall SoundVoice_Cull(u8 *voice) {
    at<u32>(voice, 0x30) |= 0x800;
    if (voice[0x30] & 1 && at<u32>(voice, 0x24)) stop_sound(at<u32>(voice, 0x24));
}

u32 *__fastcall SoundVoice_Command(u8 *voice, void *, u32 *out, u8 *command_arg) {
    u8 *const command = command_arg;   // (kept in EDI: the sound system's write into the argument's slot below is not read back)
    if (at<u32>(voice, 0x24)) vcall<void>(system(), 0x10, &command_arg, command);   // (the command argument's own slot as the out, sic)
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

void __fastcall SoundVoice_SetParam(u8 *voice, void *, u32 key, u32 value) {
    if (!at<u8 *>(voice, 0x4c)) SoundVoice_EnsureParams(voice);
    bool changed = false;
    for (u8 i = 0; i < 4; ++i) {
        u8 *const slot = at<u8 *>(voice, 0x4c) + i;
        if (static_cast<u8>(key) != slot[0]) continue;
        slot[4] = static_cast<u8>(value);
        if (i) changed = true;   // (the switch's own byte 0 not counted, sic)
    }
    if (changed && !(at<u32>(voice, 0x30) & 0x4000)) vcall<void>(voice, 0x34);
}

void __fastcall SwitchVoice_SetParam(u8 *voice, void *, u32 key, u32 value) {
    SoundVoice_SetParam(voice, nullptr, key, value);
    if (!(voice[0x30] & 0x40) || at<u8 *>(voice, 0x4c)[4] == 0x7f) return;
    if (!vcall<u8>(voice, 0x38, 1u) || !at<u32>(voice, 8)) return;
    u8 *const sample = SoundVoice_PickSample(voice, nullptr, 1);
    at<u8 *>(voice, 0xc) = sample;
    if (sample) at<u32>(voice, 0x30) &= ~0x40u;
}

u8 *__cdecl SwitchVoice_Create(u8 *owner, u8 *memory, u8 *definition, u8 *info) {
    u8 *const voice = memory ? SwitchVoice_Ctor(memory) : nullptr;
    return init_voice(voice, reinterpret_cast<u32 *>(&memory), owner, definition, info);
}

u8 *__cdecl SequenceVoice_Create(u8 *owner, u8 *memory, u8 *definition, u8 *info) {
    u8 *const voice = memory ? SequenceVoice_Ctor(memory) : nullptr;
    return init_voice(voice, reinterpret_cast<u32 *>(&memory), owner, definition, info);
}

u8 *__cdecl Voice70a390_Create(u8 *owner, u8 *memory, u8 *definition, u8 *info) {
    u8 *const voice = memory ? Voice70a390_Ctor(memory) : nullptr;
    return init_voice(voice, reinterpret_cast<u32 *>(&memory), owner, definition, info);
}

u8 *__cdecl Voice70a2e0_Create(u8 *owner, u8 *memory, u8 *definition, u8 *info) {
    u8 *const voice = memory ? Voice70a2e0_Ctor(memory) : nullptr;
    return init_voice(voice, reinterpret_cast<u32 *>(&memory), owner, definition, info);
}

u8 *__cdecl Voice70a3e8_Create(u8 *owner, u8 *memory, u8 *definition, u8 *info) {
    u8 *const voice = memory ? Voice70a3e8_Ctor(memory) : nullptr;
    return init_voice(voice, reinterpret_cast<u32 *>(&memory), owner, definition, info);
}

u8 *__cdecl Voice70a878_Create(u8 *owner, u8 *memory, u8 *definition, u8 *info) {
    u8 *const voice = memory ? Voice70a878_Ctor(memory) : nullptr;
    return init_voice(voice, reinterpret_cast<u32 *>(&memory), owner, definition, info);
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

u8 *__fastcall LinkedVoice_Ctor(u8 *voice, void *, u8 *parent) {
    Class70a7e0_Ctor(voice);
    at<u32>(voice, 0) = 0x0070a970;
    SoundVoice_EnsureParams(voice);
    at<u8 *>(at<u8 *>(voice, 0x4c), 0x1c) = parent;
    return voice;
}

u8 *__fastcall Voice70a878_Ctor(u8 *voice) {
    Class70a7e0_Ctor(voice);
    at<u32>(voice, 0) = 0x0070a878;
    SoundVoice_EnsureParams(voice);
    at<u32>(at<u8 *>(voice, 0x4c), 0x1c) = 0;   // (no block when the allocation fails: a fault, sic)
    return voice;
}

u8 *__fastcall Voice70a390_Ctor(u8 *voice) {
    Class70a7e0_Ctor(voice);
    at<u32>(voice, 0) = 0x0070a390;
    SoundVoice_EnsureParams(voice);
    return voice;
}

u8 *__fastcall Voice70a2e0_Ctor(u8 *voice) {
    Class70a7e0_Ctor(voice);
    at<u32>(voice, 0) = 0x0070a390;
    SoundVoice_EnsureParams(voice);
    at<u32>(voice, 0) = 0x0070a2e0;
    return voice;
}

u8 *__fastcall LinkedSwitchVoice_Ctor(u8 *voice, void *, u8 *parent) {
    LinkedVoice_Ctor(voice, nullptr, parent);
    at<u32>(voice, 0) = 0x0070a9e0;
    return voice;
}

u8 __fastcall LinkedVoice_Init(u8 *voice, void *, u8 *definition, u32 time) {
    at<u8 *>(voice, 4) = definition;
    SoundVoice_SetFlags(voice, nullptr, 0);
    voice[0x47] = at<u8 *>(at<u8 *>(voice, 0x4c), 0x1c)[0x47];
    if (!vcall<u8>(voice, 0x38, 0u)) return 0;
    const u32 sample = vcall<u32>(voice, 0x3c, 0u);
    at<u32>(voice, 0xc) = sample;
    if (!sample) return 0;
    at<u32>(voice, 0x1c) = vcall<u32>(voice, 0x4c) + time;
    return 1;
}

void __fastcall LinkedVoice_FollowParent(u8 *voice) {
    if (!at<u32>(voice, 0x24)) return;
    u32 record[0x24];   // (the fields not named here are left unset, sic)
    static const u8 kCleared[] = {0, 1, 3, 4, 5, 0xb, 0xc, 0xe, 0xf, 0x10, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x22};
    for (const u8 i : kCleared) record[i] = 0;
    record[0x20] = 0x3e800000;   // 0.25f
    record[0x21] = 0x3f000000;   // 0.5f
    record[0x23] = 0x3f800000;   // 1.0f
    if (!(voice[0x30] & 8)) {
        const u32 flags = at<u32>(system(), 0x28);
        record[3] = flags & 0x43 && flags & 0x80 ? 0x820 : 0x20;
    }
    record[5] = 1;
    record[0xc] = static_cast<u32>(reinterpret_cast<uintptr_t>(at<u8 *>(at<u8 *>(voice, 0x4c), 0x1c) + 0x10));
    u32 out;
    u32 functor[3] = {0x0070a850, at<u32>(voice, 0x24), static_cast<u32>(reinterpret_cast<uintptr_t>(record))};
    vcall<void>(system(), 0x10, &out, functor);
}

u8 __fastcall LinkedSwitchVoice_PickBranch(u8 *voice, void *, u32) {
    u8 *const parent = at<u8 *>(at<u8 *>(voice, 0x4c), 0x1c);
    const u32 value = at<u8 *>(parent, 0x4c)[4];
    at<u32>(voice, 8) = at<u32>(parent, 8);
    if (!at<u8 *>(voice, 8) && at<u8 *>(voice, 4)) {
        at<u8 *>(voice, 8) = at<u8 *>(at<u8 *>(voice, 4), 8);
        if (SfxVariation_SampleCount(at<u8 *>(voice, 8))) {
            at<u32>(parent, 8) = at<u32>(voice, 8);
            return 1;
        }
    }
    if (!at<u32>(at<u8 *>(voice, 8), 4)) {
        at<u32>(voice, 8) = 0;
        at<u32>(parent, 8) = 0;
        return 0;
    }
    auto entry = [voice](u32 i) { return at<const u8 *>(at<u8 *>(voice, 8), 0x26) + 8 * i; };
    auto within = [value](const u8 *e) { return e[6] <= value && e[7] >= value; };
    u32 matches = 0, sum = 0;
    for (u32 i = 0; i < at<u32>(at<u8 *>(voice, 8), 4); ++i)
        if (within(entry(i))) {
            ++matches;
            sum += at<u32>(at<const u8 *>(entry(i), 0), 0x1e);
        }
    if (!matches) {
        at<u8 *>(parent, 0x4c)[4] = 0x7f;
        at<u32>(parent, 0x30) |= 0x40;
        return 0;
    }
    const u32 seed = global<u32>(0x00fb1f20) * 0x19660d + 0x3c6ef35f;
    global<u32>(0x00fb1f20) = seed;
    const u32 target = (seed >> 13 | seed << 19) % sum;
    u32 run = 0;
    for (u32 i = 0; i < at<u32>(at<u8 *>(voice, 8), 4); ++i) {
        if (!within(entry(i))) continue;
        run += at<u32>(at<const u8 *>(entry(i), 0), 0x1e);
        if (run >= target) {
            const u32 chosen = at<u32>(entry(i), 0);
            at<u32>(voice, 8) = chosen;
            at<u32>(parent, 8) = chosen;
            return 1;
        }
    }
    return 0;
}

u32 __fastcall SoundVoice_RollVolume(u8 *voice) {
    if (!at<u8 *>(voice, 8)) return 100;
    return roll<u8>(voice, 0xc, 1);
}

u32 __fastcall SoundVoice_RollPitch(u8 *voice) {
    if (!at<u8 *>(voice, 8)) return 0;
    return roll<s8>(voice, 0xe, 2);
}

u32 __fastcall SoundVoice_SetRecordBankLinked(u8 *voice, void *, u8 *record) {
    if (!SoundVoice_SetRecordBank(voice, nullptr, record)) return 0;
    SetThenFlag4c_14_6c50f0(record, nullptr, static_cast<u32>(reinterpret_cast<uintptr_t>(voice)));
    const u8 *const params = at<u8 *>(voice, 8);
    if (at<u16>(params, 0x10) || at<u16>(params, 0x12)) FlagThenSet14_58_6c5220(record, nullptr, vcall<u32>(voice, 0x4c));
    return 1;
}

u32 __fastcall SoundVoice_SetRecordBank(u8 *voice, void *, u8 *record) {
    u8 *const resource = SoundChannel_LoadBank(system(), nullptr, at<u16>(at<u8 *>(voice, 0xc), 0xc));
    if (!resource) return 0;
    Obj_SetFirstFlag80(record, nullptr, at<u32>(at<u8 *>(voice, 0xc), 0));
    Obj_SetSecondFlag8(record, nullptr, static_cast<u32>(reinterpret_cast<uintptr_t>(resource)));
    if (at<u32>(voice, 0x30) & 0x200) {
        SetThenFlag8_14_6c50b0(record, nullptr, at<u32>(voice, 4));
        at<u32>(record, 0xc) |= 0x200;
    }
    at<u32>(record, 0xc) |= (at<u32>(voice, 0x30) & 0x20000) ? 4u : 0xc0u;
    return 1;
}

u32 __fastcall SoundVoice_RollWord(u8 *voice) {
    const u8 *v = at<u8 *>(voice, 8);
    if (!at<u16>(v, 0x10) && !at<u16>(v, 0x12)) return 0;
    if (at<u32>(voice, 0x4c)) {
        u32 percent;
        if (vcall<bool>(voice, 0x40, 4u, &percent)) {
            v = at<u8 *>(voice, 8);
            const s32 scaled = static_cast<s32>(static_cast<u32>(at<u16>(v, 0x12) - at<u16>(v, 0x10)) * percent);
            return static_cast<u32>(scaled / 100) + at<u16>(at<u8 *>(voice, 8), 0x10);
        }
    }
    u8 *w = at<u8 *>(voice, 8);
    if (at<u16>(w, 0x12) < at<u16>(w, 0x10)) {
        at<u16>(w, 0x12) = at<u16>(w, 0x10) ^ at<u16>(w, 0x12);
        w = at<u8 *>(voice, 8);
        at<u16>(w, 0x10) ^= at<u16>(w, 0x12);
        w = at<u8 *>(voice, 8);
        at<u16>(w, 0x12) ^= at<u16>(w, 0x10);
    }
    w = at<u8 *>(voice, 8);
    u32 range = static_cast<u32>(at<u16>(w, 0x12)) - at<u16>(w, 0x10);
    if (range) range = next_random_full() % range;
    return at<u16>(at<u8 *>(voice, 8), 0x10) + range;
}

u32 __fastcall SoundVoice_SetRecordLevel(u8 *voice, void *, u8 *record) {
    if (voice[0x30] & 8) return FlagThenSet14_5c_6c51d0(record, nullptr, 0);
    const u32 on = Obj_Flags28_Bit1IfBit0(system());
    if (!on) return on;
    const float level = static_cast<float>(vcall<double>(system(), 0x28, voice + 0x10));
    return FlagThenSet14_5c_6c51d0(record, nullptr, float_bits(level));
}

u32 __fastcall LinkedVoice_FillRecord(u8 *voice, void *, u32, u8 *record) {
    if (!at<u32>(voice, 0xc) || !at<u32>(voice, 8) || !system()) return 0;
    if (!vcall<u32>(voice, 0x48, record)) return 0;
    SetThenFlag28_14_6c5130(record, nullptr, SoundVoice_RollVolume(voice));
    Obj_Set94_98_Flag40000(record, nullptr, static_cast<u32>(reinterpret_cast<uintptr_t>(system() + 0x58)), voice[0x47]);
    SetThenFlag24_14_6c5170(record, nullptr, SoundVoice_RollPitch(voice));
    SetThenFlag2c_14_6c5150(record, nullptr, at<u32>(at<u8 *>(at<u8 *>(voice, 0x4c), 0x1c), 0x20));
    SoundVoice_SetRecordLevel(voice, nullptr, record);
    if (voice[0x30] & 8) {
        volatile u32 local;   // (never set: the record keeps its address, sic)
        const u32 flags = at<u32>(record, 0x14);
        at<u32>(record, 0x30) = static_cast<u32>(reinterpret_cast<uintptr_t>(&local));
        const u32 c = at<u32>(record, 0xc);
        at<u32>(record, 0x14) = flags | 1;
        at<u32>(record, 0xc) = c | 1;
    } else {
        const double radius = SoundVoice_Radius(voice);
        u32 stored = float_bits(static_cast<float>(radius));
        if (!(radius > 2.0)) stored = 0x40066666;   // 2.1f
        const float level = static_cast<float>(vcall<double>(system(), 0x2c));
        at<u32>(record, 0x38) = 0x40000000;   // 2.0f
        at<u32>(record, 0x3c) = stored;
        at<u32>(record, 0x40) = float_bits(level);
        const u32 flags = at<u32>(record, 0x14) | 0x200;
        at<u32>(record, 0x14) = flags;
        at<u32>(record, 0x14) = flags | 1;
        at<u32>(record, 0x30) = static_cast<u32>(reinterpret_cast<uintptr_t>(at<u8 *>(at<u8 *>(voice, 0x4c), 0x1c) + 0x10));
        u32 c = at<u32>(record, 0xc) | 0x20;
        at<u32>(record, 0xc) = c;
        if (at<u32>(system(), 0x28) >> 3 & 1) at<u32>(record, 0xc) = c | 0x1000;
        const u32 sys = at<u32>(system(), 0x28);
        if (sys & 0x43 && sys >> 7 & 1) at<u32>(record, 0xc) |= 0x800;
    }
    if (voice[0x30] & 4) SetThenFlag44_14_6c5110(record, nullptr, 9999);
    return 1;
}

void __fastcall SoundVoice_SetValue20(u8 *voice, void *, u32 value) {
    at<u32>(voice, 0x20) = value;
    if (!(voice[0x30] & 1)) return;
    u32 record[0x27];
    start_record(record);
    SetThenFlag2c_14_6c5150(reinterpret_cast<u8 *>(record), nullptr, value);
    u32 result;   // (dropped)
    send_record(voice, &result, record);
}

void __fastcall SoundVoice_SendLevels(u8 *voice) {
    if (!at<u32>(voice, 0x24) || !system()) return;
    u32 record[0x27];
    start_record(record);
    if (!(at<u32>(voice, 0x30) & 0x400))
        SetThenFlag28_14_6c5130(reinterpret_cast<u8 *>(record), nullptr, SoundVoice_RollVolume(voice));
    SetThenFlag24_14_6c5170(reinterpret_cast<u8 *>(record), nullptr, SoundVoice_RollPitch(voice));
    u32 result;   // (dropped)
    send_record(voice, &result, record);
}

u32 *__fastcall SoundVoice_SetPosition(u8 *voice, void *, u32 *out, const u32 *position) {
    float *const at10 = &at<float>(voice, 0x10);
    for (u32 i = 0; i < 3; ++i) at<u32>(voice, 0x10 + 4 * i) = position[i];
    if (at<u32>(voice, 0x24)) {
        u32 record[0x27];
        start_record(record);
        if (voice[0x30] & 8) {
            Fn_6bb220(reinterpret_cast<u8 *>(at10), nullptr, 0, 1, 0);   // (ints: 1 is no 1.0f, sic)
        } else {
            Vec3_Scale(at10, nullptr, at<float>(system(), 0x2c));
            record[3] |= 0x20;
            const u32 sys = at<u32>(system(), 0x28);
            if (sys & 0x43 && sys >> 7 & 1) record[3] |= 0x800;
        }
        record[5] |= 1;
        record[0xc] = static_cast<u32>(reinterpret_cast<uintptr_t>(at10));
        u32 result;   // (dropped)
        send_record(voice, &result, record);
    }
    *out = 0;
    return out;
}

u32 __fastcall SoundVoice_FillRecordUnowned(u8 *voice, void *, u32 now, u8 *record) {
    if (!SoundVoice_FillRecord(voice, nullptr, now, record)) return 0;
    at<u32>(record, 0x50) = 0;
    at<u32>(record, 0x14) &= ~0x2000u;
    return 1;
}

u32 __fastcall SoundVoice_FillRecord(u8 *voice, void *, u32 now, u8 *record) {
    if (!at<u32>(voice, 0xc) || !at<u32>(voice, 8) || !system()) return 0;
    if (!vcall<u32>(voice, 0x48, record)) return 0;
    SetThenFlag28_14_6c5130(record, nullptr, at<u32>(voice, 0x30) & 0x8000 ? at<u16>(voice, 0x48) : SoundVoice_RollVolume(voice));
    Obj_Set94_98_Flag40000(record, nullptr, static_cast<u32>(reinterpret_cast<uintptr_t>(system() + 0x58)), voice[0x47]);
    SetThenFlag24_14_6c5170(record, nullptr,
                            at<u32>(voice, 0x30) & 0x10000 ? static_cast<u32>(static_cast<s32>(at<s16>(voice, 0x4a))) : SoundVoice_RollPitch(voice));
    SetThenFlag2c_14_6c5150(record, nullptr, at<u32>(voice, 0x20));
    const u32 f14 = at<u32>(record, 0x14);
    at<u8 *>(record, 0x50) = voice;
    at<u32>(record, 0x14) = f14 | 0x2000;
    SetThenFlag48_14_6c5090(record, nullptr, at<u32>(voice, 0x2c));
    if (!at<u32>(at<u8 *>(voice, 0xc), 8)) at<u32>(voice, 0xc) = at<u32>(at<u8 *>(voice, 8), 8);
    const u32 since = now - (at<u32>(voice, 0x1c) - at<u32>(at<u8 *>(voice, 0xc), 8) - 0xfa);
    at<u32>(voice, 0x3c) = vcall<u32>(voice, 0x4c);
    if (Obj_PastEnd(voice, nullptr, now)) return 0;
    SoundVoice_SetRecordLevel(voice, nullptr, record);
    const u32 flags = at<u32>(voice, 0x30);
    float *const at10 = &at<float>(voice, 0x10);
    if (flags & 8) {
        Fn_6bb220(reinterpret_cast<u8 *>(at10), nullptr, 0, 1, 0);   // (ints: 1 is no 1.0f, sic)
        at<u32>(record, 0xc) |= 1;
    } else {
        const u32 length = at<u32>(at<u8 *>(voice, 0xc), 8);
        const u32 into = since % length;   // (a zero length faults, as the original's DIV)
        if (into > 1000 && !(flags & 2) && length > 1000 && length - into < 1000) return 0;
        const double radius = SoundVoice_Radius(voice);
        u32 stored = float_bits(static_cast<float>(radius));
        if (!(radius > 2.0)) stored = 0x40066666;   // 2.1f
        const float level = static_cast<float>(vcall<double>(system(), 0x2c));
        at<u32>(record, 0x38) = 0x40000000;   // 2.0f
        at<u32>(record, 0x3c) = stored;
        at<u32>(record, 0x40) = float_bits(level);
        at<u32>(record, 0x14) |= 0x200;
        Vec3_Scale(at10, nullptr, at<float>(system(), 0x2c));
        u32 c = at<u32>(record, 0xc) | 0x20;
        at<u32>(record, 0xc) = c;
        if (at<u32>(system(), 0x28) >> 3 & 1) at<u32>(record, 0xc) = c | 0x1000;
        const u32 sys = at<u32>(system(), 0x28);
        if (sys & 0x43 && sys >> 7 & 1) at<u32>(record, 0xc) |= 0x800;
    }
    const u32 f = at<u32>(record, 0x14);
    at<u32>(record, 0x30) = static_cast<u32>(reinterpret_cast<uintptr_t>(at10));
    at<u32>(record, 0x14) = f | 1;
    if (voice[0x30] & 4) SetThenFlag44_14_6c5110(record, nullptr, 9999);
    return 1;
}

u32 *__fastcall SoundVoice_Start(u8 *voice, void *, u32 *out, u32 now) { return start_voice(voice, out, now, false); }

u32 *__fastcall SoundVoice_StartCounted(u8 *voice, void *, u32 *out, u32 now) { return start_voice(voice, out, now, true); }

u32 __fastcall SoundVoice_PickChannelVoice(u8 *voice, void *, u32 a, u32) {
    const u32 variation = vcall<u32>(voice, 0x38, 1);
    if (!static_cast<u8>(variation)) return variation;   // (the slot's whole answer, sic)
    u8 *const system = global<u8 *>(0x00fb1f24);
    if (!system) return 0;
    if (!at<u32>(voice, 8)) return static_cast<u32>(reinterpret_cast<uintptr_t>(system));   // sic
    const u32 sample = vcall<u32>(voice, 0x3c, 1);
    at<u32>(voice, 0xc) = sample;
    if (!sample) return 0;
    u32 record[0x27];
    start_record(record);
    if (!vcall<u32>(voice, 0x48, record)) return 0;
    u32 picked;   // (dropped)
    return SoundChannel_PickVoice(global<u8 *>(0x00fb1f24), nullptr, a, &picked, reinterpret_cast<u8 *>(record));
}

void __fastcall SoundVoice_EnsureShuffleBag(u8 *voice, void *, u8 *list) {
    u8 *const params = *reinterpret_cast<u8 **>(voice + 0x4c);
    u8 *&bag = *reinterpret_cast<u8 **>(params + 0x1c);
    if (bag) {
        if (*reinterpret_cast<u8 **>(bag) != list) ShuffleBag_SetList(bag, nullptr, list);
        return;
    }
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x10));
    bag = block ? ShuffleBag_Ctor(block, nullptr, list) : nullptr;
}

u8 __fastcall SoundVoice_ShuffleVariation(u8 *voice, void *, u32) {
    SoundVoice_EnsureShuffleBag(voice, nullptr, *reinterpret_cast<u8 **>(voice + 4));
    u8 *const bag = *reinterpret_cast<u8 **>(*reinterpret_cast<u8 **>(voice + 0x4c) + 0x1c);
    u8 *const next = ShuffleBag_Next(bag, nullptr, *reinterpret_cast<u32 *>(voice + 0x30) & 4);
    *reinterpret_cast<u8 **>(voice + 8) = next;
    return next != nullptr;
}

u8 *__fastcall Voice70a3e8_Ctor(u8 *voice) {
    Class70a7e0_Ctor(voice);
    at<u32>(voice, 0) = 0x0070a3e8;
    return voice;
}

u8 *__fastcall Voice70a4f0_Ctor(u8 *voice) {
    Class70a7e0_Ctor(voice);
    at<u32>(voice, 0) = 0x0070a4f0;
    return voice;
}

u8 *__fastcall Voices70a4f0_New(u8 *holder, void *, u32 count) {
    u8 *array = nullptr;
    if (u32 *const block = static_cast<u32 *>(GameHeap_Alloc(count * 0x50 + 4))) {
        block[0] = count;
        array = reinterpret_cast<u8 *>(block + 1);
        CRT_VectorCtorIterator(array, 0x50, static_cast<s32>(count), reinterpret_cast<void(__fastcall *)(void *)>(Voice70a4f0_Ctor),
                               reinterpret_cast<void *>(static_cast<uintptr_t>(0x006b7230)));
    }
    at<u8 *>(holder, 0) = array;
    return holder;
}

u8 *__fastcall Voice70a390_NewLinked(u8 *voice) {
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x50));
    return block ? LinkedVoice_Ctor(block, nullptr, voice) : nullptr;
}

u8 *__fastcall Voice70a2e0_NewLinkedSwitch(u8 *voice) {
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x50));
    return block ? LinkedSwitchVoice_Ctor(block, nullptr, voice) : nullptr;
}

void __fastcall Class70a7e0_Dtor(u8 *voice) {
    u8 *const params = at<u8 *>(voice, 0x4c);
    at<u32>(voice, 0) = 0x0070a7e0;
    at<u32>(voice, 0x2c) &= 0xffff;
    if (params) GameHeap_Free(params);
    if (u8 *const owner = at<u8 *>(voice, 0x28)) vcall<void>(owner, 0);
}

u8 *__fastcall Class70a7e0_DeletingDtor(u8 *voice, void *, u8 flags) {
    Class70a7e0_Dtor(voice);
    if (flags & 1) GameHeap_Free(voice);
    return voice;
}

void __fastcall Voice70a3e8_Dtor(u8 *voice) {
    at<u32>(voice, 0) = 0x0070a3e8;
    Class70a7e0_Dtor(voice);
}

void __fastcall Voice70a338_Dtor(u8 *voice) {
    at<u32>(voice, 0) = 0x0070a338;
    Class70a7e0_Dtor(voice);
}

u8 *__fastcall Voice70a3e8_DeletingDtor(u8 *voice, void *, u8 flags) {
    Voice70a3e8_Dtor(voice);
    if (flags & 1) GameHeap_Free(voice);
    return voice;
}

u8 *__fastcall Voice70a338_DeletingDtor(u8 *voice, void *, u8 flags) {
    Voice70a338_Dtor(voice);
    if (flags & 1) GameHeap_Free(voice);
    return voice;
}

u8 *__fastcall Voice70a4f0_VectorDeletingDtor(u8 *voice, void *, u8 flags) {
    if (flags & 2) {
        CRT_VectorDtorIterator(voice, 0x50, at<s32>(voice, (u32)-4), reinterpret_cast<void(__fastcall *)(void *)>(Voice70a3e8_Dtor));
        GameHeap_Free(voice - 4);   // (sic: flag 1 not tested)
        return voice;
    }
    Voice70a3e8_Dtor(voice);
    if (flags & 1) GameHeap_Free(voice);
    return voice;
}

void __fastcall Voice70a878_Dtor(u8 *voice) {
    at<u32>(voice, 0) = 0x0070a878;
    if (u8 *const bag = at<u8 *>(at<u8 *>(voice, 0x4c), 0x1c)) GameHeap_Free(bag);
    at<u32>(voice, 0) = 0x0070a338;
    Class70a7e0_Dtor(voice);
}

u8 *__fastcall Voice70a878_DeletingDtor(u8 *voice, void *, u8 flags) {
    Voice70a878_Dtor(voice);
    if (flags & 1) GameHeap_Free(voice);
    return voice;
}

void __fastcall Voice70a8d0_Dtor(u8 *voice) {
    at<u32>(voice, 0) = 0x0070a8d0;
    if (u8 *const bag = at<u8 *>(at<u8 *>(voice, 0x4c), 0x1c)) {
        GameHeap_Free(at<u8 *>(bag, 8));
        GameHeap_Free(bag);
    }
    at<u32>(voice, 0) = 0x0070a338;
    Class70a7e0_Dtor(voice);
}

u8 *__fastcall Voice70a8d0_DeletingDtor(u8 *voice, void *, u8 flags) {
    Voice70a8d0_Dtor(voice);
    if (flags & 1) GameHeap_Free(voice);
    return voice;
}

// (the children: the parameter block's +0x18, each the next's in its own block's +0x18)
void __fastcall Voice70a390_Dtor(u8 *voice) {
    at<u32>(voice, 0) = 0x0070a390;
    for (u8 *child = at<u8 *>(at<u8 *>(voice, 0x4c), 0x18); child;) {
        u8 *const next = at<u8 *>(at<u8 *>(child, 0x4c), 0x18);
        vcall<void>(child, 8, 1u);
        child = next;
    }
    at<u32>(voice, 0) = 0x0070a3e8;
    Class70a7e0_Dtor(voice);
}

u8 *__fastcall Voice70a390_DeletingDtor(u8 *voice, void *, u32 flags) {
    Voice70a390_Dtor(voice);
    if (flags & 1) GameHeap_Free(voice);
    return voice;
}

u32 *__fastcall Voice70a390_Command(u8 *voice, void *, u32 *out, u8 *command) {
    u8 *scratch = command;   // (the children's out: the command's own argument slot, sic)
    for (u8 *child = at<u8 *>(at<u8 *>(voice, 0x4c), 0x18); child;) {
        u8 *const next = at<u8 *>(at<u8 *>(child, 0x4c), 0x18);
        at<u32>(command, 4) = at<u32>(child, 0x24);
        vcall<void>(child, 0x18, &scratch, command);
        if (vcall<u32>(command, 0) == 1) vcall<void>(child, 8, 1u);
        child = next;
    }
    at<u32>(at<u8 *>(voice, 0x4c), 0x18) = 0;   // (the children kept forgotten too, sic)
    SoundVoice_Command(voice, nullptr, out, command);
    return out;
}

u32 *__fastcall Voice70a390_SetPosition(u8 *voice, void *, u32 *out, const u32 *position) {
    float *const at10 = &at<float>(voice, 0x10);
    for (u32 i = 0; i < 3; ++i) at<u32>(voice, 0x10 + 4 * i) = position[i];
    if (voice[0x30] & 8) {
        Fn_6bb220(reinterpret_cast<u8 *>(at10), nullptr, 0, 1, 0);   // (ints: 1 is no 1.0f, sic)
    } else {
        Vec3_Scale(at10, nullptr, at<float>(system(), 0x2c));
    }
    for (u8 *child = at<u8 *>(at<u8 *>(voice, 0x4c), 0x18); child; child = at<u8 *>(at<u8 *>(child, 0x4c), 0x18))
        LinkedVoice_FollowParent(child);
    *out = 0;
    return out;
}

void __fastcall Voice70a390_SetParam(u8 *voice, void *, u32 key, u32 value) {
    SoundVoice_SetParam(voice, nullptr, key, value);
    if (voice[0x30] & 0x40 && at<u8 *>(voice, 0x4c)[4] != 0x7f) at<u32>(voice, 0x30) &= ~0x40u;
}

void __fastcall Voice70a390_SendLevels(u8 *voice) {
    for (u8 *child = at<u8 *>(at<u8 *>(voice, 0x4c), 0x18); child; child = at<u8 *>(at<u8 *>(child, 0x4c), 0x18)) vcall<void>(child, 0x34);
}

u8 __fastcall Voice70a390_AddChild(u8 *voice, void *, u8 *prev, u32 time) {
    at<u8 *>(at<u8 *>(prev, 0x4c), 0x18) = vcall<u8 *>(voice, 0x54);
    if (LinkedVoice_Init(at<u8 *>(at<u8 *>(prev, 0x4c), 0x18), nullptr, at<u8 *>(voice, 4), time)) {
        SoundVoice_ApplyVariation(voice);
        return 1;
    }
    if (u8 *const child = at<u8 *>(at<u8 *>(prev, 0x4c), 0x18)) vcall<void>(child, 8, 1u);
    at<u32>(at<u8 *>(prev, 0x4c), 0x18) = 0;
    return 0;
}

u8 *__fastcall Voice70a390_FirstChild(u8 *voice, void *, u32 time) {
    u8 *const params = at<u8 *>(voice, 0x4c);
    if (!at<u8 *>(params, 0x18)) {
        if (params[0] && !(voice[0x30] & 0x20)) {
            at<u32>(voice, 8) = 0;
            at<u32>(voice, 0x30) |= 0x20;
        }
        Voice70a390_AddChild(voice, nullptr, voice, time);
    }
    return at<u8 *>(at<u8 *>(voice, 0x4c), 0x18);
}

void __fastcall Voice70a390_StartChildren(u8 *voice, void *, u32 time) {
    if (voice[0x30] & 0x10) return;
    u32 scratch = time;   // (the children's out: the time's own argument slot, sic)
    for (u8 *child = Voice70a390_FirstChild(voice, nullptr, time); child;) {
        u8 *const next = at<u8 *>(at<u8 *>(child, 0x4c), 0x18);
        vcall<void>(child, 0x14, &scratch, time);
        if (time > at<u32>(child, 0x1c) && !at<u8 *>(at<u8 *>(child, 0x4c), 0x18)) Voice70a390_AddChild(voice, nullptr, child, time);
        child = next;
    }
}

void __fastcall Voice70a390_PruneChildren(u8 *voice, void *, u32 time) {
    u8 *prev = voice;
    for (u8 *child = at<u8 *>(at<u8 *>(voice, 0x4c), 0x18); child;) {
        if (time < at<u32>(child, 0x1c)) {
            prev = child;
            child = at<u8 *>(at<u8 *>(child, 0x4c), 0x18);
            continue;
        }
        u8 *const next = at<u8 *>(at<u8 *>(child, 0x4c), 0x18);
        at<u8 *>(at<u8 *>(prev, 0x4c), 0x18) = next;
        vcall<void>(child, 8, 1u);
        child = next;
    }
}

u32 *__fastcall Voice70a390_Start(u8 *voice, void *, u32 *out, u32 time) {
    if (!(voice[0x30] & 0x50)) {
        Voice70a390_StartChildren(voice, nullptr, time);
        Voice70a390_PruneChildren(voice, nullptr, time);
    }
    *out = 0;
    return out;
}

}  // extern "C"
