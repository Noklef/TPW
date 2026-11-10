#include "imaadpcm.h"

#include <cstdint>

namespace {

// The step dword at the byte offset index + 4 * nibble of the steps (unaligned for an index that is not a whole step).
u32 step(s32 index, u32 nibble) { return *reinterpret_cast<const u32 *>((uintptr_t)gImaAdpcmSteps + (u32)index + nibble * 4); }

// One channel's nibble: the channel's sample and index updated, the sample written.
void channel(u32 nibble, s32 &sample_at, s32 &index_at, s16 *out) {
    const s32 index = index_at;
    s32 sample = (s32)((u32)sample_at + step(index, nibble));
    if (sample > 0x7fff) sample = 0x7fff;
    else if (sample < -0x8000) sample = -0x8000;
    *out = (s16)sample;
    sample_at = sample;
    s32 next = (s32)((u32)index + (u32)gImaAdpcmIndexMoves[nibble & 7]);
    if (next > 0x1600) next = 0x1600;
    else if (next < 0) next = 0;
    index_at = next;
}

}  // namespace

extern "C" {

void __cdecl ImaAdpcm_DecodeStereo(const u8 *data, s16 *out, s32 count, u32 *samples, u32 *indices) {
    ImaAdpcmState &state = gImaAdpcmState;
    const u32 index_pair = *indices;
    state.index[0] = (s16)index_pair;
    state.index[1] = (s16)(index_pair >> 16);
    const u32 sample_pair = *samples;
    state.sample[0] = (s16)sample_pair;
    state.sample[1] = (s16)(sample_pair >> 16);
    s32 i = 0;
    do {
        channel((u32)data[i] >> 4, state.sample[0], state.index[0], out + i * 2);
        channel((u32)data[i] & 0xf, state.sample[1], state.index[1], out + i * 2 + 1);
        ++i;
    } while (i < count);
    *samples = (u32)state.sample[1] << 16 | ((u32)state.sample[0] & 0xffff);
    *indices = (u32)state.index[1] << 16 | ((u32)state.index[0] & 0xffff);
}

void __cdecl ImaAdpcm_DecodeMono(s32 start, const u8 *data, s16 *out, s32 count, s32 *sample_io, s32 *index_io) {
    const s32 end = (s32)((u32)count + (u32)start);
    s32 index = *index_io;
    s32 sample = (s16)*sample_io;
    s32 i = start;
    do {
        const u32 nibble = (i & 1) ? *data++ & 0xfu : (u32)*data >> 4;
        sample = (s32)((u32)sample + step(index, nibble));
        if (sample > 0x7fff) sample = 0x7fff;
        else if (sample < -0x8000) sample = -0x8000;
        *out++ = (s16)sample;
        ++i;
        index = (s32)((u32)index + (u32)gImaAdpcmIndexMoves[nibble & 7]);
        if (index > 0x1600) index = 0x1600;
        else if (index < 0) index = 0;
    } while (i < end);
    *sample_io = sample;
    *index_io = index;
}
}
