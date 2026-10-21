#include "imaadpcm.h"

#include <cstdint>

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// One channel's nibble: the sample and index globals updated, the sample written.
void channel(u32 nibble, u32 sample_at, u32 index_at, s16 *out) {
    const s32 index = global<s32>(index_at);
    s32 sample = (s32)((u32)global<s32>(sample_at) + global<u32>(0x0077c6c0 + (u32)index + nibble * 4));
    if (sample > 0x7fff) sample = 0x7fff;
    else if (sample < -0x8000) sample = -0x8000;
    *out = (s16)sample;
    global<s32>(sample_at) = sample;
    s32 next = (s32)((u32)index + global<u32>(0x0077dd00 + (nibble & 7) * 4));
    if (next > 0x1600) next = 0x1600;
    else if (next < 0) next = 0;
    global<s32>(index_at) = next;
}

}  // namespace

extern "C" {

void __cdecl ImaAdpcm_DecodeStereo(const u8 *data, s16 *out, s32 count, u32 *samples, u32 *indices) {
    const u32 index_pair = *indices;
    global<s32>(0x0077dd28) = (s16)index_pair;
    global<s32>(0x0077dd2c) = (s16)(index_pair >> 16);
    const u32 sample_pair = *samples;
    global<s32>(0x0077dd20) = (s16)sample_pair;
    global<s32>(0x0077dd24) = (s16)(sample_pair >> 16);
    s32 i = 0;
    do {
        channel((u32)data[i] >> 4, 0x0077dd20, 0x0077dd28, out + i * 2);
        channel((u32)data[i] & 0xf, 0x0077dd24, 0x0077dd2c, out + i * 2 + 1);
        ++i;
    } while (i < count);
    *samples = global<u32>(0x0077dd24) << 16 | (global<u32>(0x0077dd20) & 0xffff);
    *indices = global<u32>(0x0077dd2c) << 16 | (global<u32>(0x0077dd28) & 0xffff);
}

void __cdecl ImaAdpcm_DecodeMono(s32 start, const u8 *data, s16 *out, s32 count, s32 *sample_io, s32 *index_io) {
    const s32 end = (s32)((u32)count + (u32)start);
    s32 index = *index_io;
    s32 sample = (s16)*sample_io;
    s32 i = start;
    do {
        const u32 nibble = (i & 1) ? *data++ & 0xfu : (u32)*data >> 4;
        sample = (s32)((u32)sample + global<u32>(0x0077c6c0 + (u32)index + nibble * 4));
        if (sample > 0x7fff) sample = 0x7fff;
        else if (sample < -0x8000) sample = -0x8000;
        *out++ = (s16)sample;
        ++i;
        index = (s32)((u32)index + global<u32>(0x0077dd00 + (nibble & 7) * 4));
        if (index > 0x1600) index = 0x1600;
        else if (index < 0) index = 0;
    } while (i < end);
    *sample_io = sample;
    *index_io = index;
}
}
