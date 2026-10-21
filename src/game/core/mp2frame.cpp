#include "mp2frame.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace {

template <typename T> T Get(u32 address) {
    T v;
    std::memcpy(&v, reinterpret_cast<const void *>(static_cast<uintptr_t>(address)), sizeof v);
    return v;
}
template <typename T> void Put(u32 address, T v) { std::memcpy(reinterpret_cast<void *>(static_cast<uintptr_t>(address)), &v, sizeof v); }
u32 Addr(const void *p) { return static_cast<u32>(reinterpret_cast<uintptr_t>(p)); }

constexpr u32 kRates = 0x7114c8;     // s32[]: the sampling rates by row
constexpr u32 kBitrates = 0x711308;  // s32[]: the bitrates by 16 (3 lsf + layer) + index
constexpr u32 kHalfRate = 0xfb2708;  // (a dword: nonzero, half the samples)

// The stream's slot 1: read (buffer, bytes).
void StreamRead(u32 decoder, u32 buffer, u32 bytes) {
    const u32 stream = Get<u32>(decoder + 0x2c7c);
    const auto read = reinterpret_cast<u32(__fastcall *)(u32, void *, u32, u32)>(static_cast<uintptr_t>(Get<u32>(Get<u32>(stream) + 4)));
    read(stream, nullptr, buffer, bytes);
}

}  // namespace

extern "C" {

s32 __fastcall Mp2Decoder_ParseHeader(u8 *decoder, void *, u32 h) {
    const u32 d = Addr(decoder);
    if ((h & 0xfff00000u) != 0xfff00000u) return -1;
    const u8 layer = static_cast<u8>(4 - (h >> 17 & 3));
    Put<u8>(d + 0x2442, layer);
    Put<u8>(d + 0x2444, static_cast<u8>(h >> 16 & 1));
    const u8 index = static_cast<u8>(h >> 12 & 0xf);
    Put<u8>(d + 0x2445, index);
    const u8 mode = static_cast<u8>(h >> 6 & 3);
    Put<u8>(d + 0x2448, mode);
    Put<u8>(d + 0x2449, static_cast<u8>(h >> 4 & 3));
    Put<u8>(d + 0x244a, static_cast<u8>(h >> 3 & 1));
    const u8 id = static_cast<u8>(h >> 19 & 1), padding = static_cast<u8>(h >> 9 & 1);
    Put<u8>(d + 0x2443, id);
    Put<u8>(d + 0x2447, padding);
    Put<u8>(d + 0x244b, static_cast<u8>(h >> 2 & 1));
    if (h & 0x100000) {
        Put<u8>(d + 0x243e, 0);
        Put<u8>(d + 0x243f, id == 0);
    } else {
        Put<u8>(d + 0x243f, 1);
        Put<u8>(d + 0x243e, 1);
    }
    const u8 rate = static_cast<u8>(h >> 10 & 3);
    if (Get<u8>(d + 0x243e)) {
        Put<u8>(d + 0x2446, static_cast<u8>(rate + 6));
    } else {
        const u8 lsf = Get<u8>(d + 0x243f);
        Put<u8>(d + 0x2446, static_cast<u8>(lsf * 3 + rate));
        Put<u8>(d + 0x2440, static_cast<u8>((lsf ? 0 : 3) + rate));
    }
    const u32 channels = mode != 3 ? 2 : 1;
    Put<u32>(d + 0x244c, channels);
    const s32 hz = Get<s32>(kRates + 4 * Get<u8>(d + 0x2446));
    Put<s32>(d + 0x2408, hz);
    Put<s32>(d + 0x2424, hz);
    if (Get<u32>(kHalfRate)) {
        Put<s32>(d + 0x2408, Get<s32>(d + 0x2408) / 2);
        Put<u32>(d + 0x2450, channels << 4);
    } else {
        Put<u32>(d + 0x2450, channels << 5);
    }
    if (!index) return -1;
    const s32 bitrate = Get<s32>(kBitrates + 4 * (((Get<u8>(d + 0x243f) * 3u + layer) << 4) + index));
    Put<s32>(d + 0x2420, bitrate);
    if (layer == 1) {
        const s32 q = static_cast<s32>(static_cast<u32>(bitrate) * 12000u) / Get<s32>(d + 0x2424);  // (idiv: 0 faults, sic)
        Put<u32>(d + 0x2470, (static_cast<u32>(q) + padding) * 4 - 4);
        return 0;
    }
    s32 q = static_cast<s32>(static_cast<u32>(bitrate) * 144000u) / Get<s32>(d + 0x2424);  // (idiv: 0 faults, sic)
    Put<s32>(d + 0x2470, q);
    if (padding) Put<s32>(d + 0x2470, ++q);
    Put<u32>(d + 0x2470, Get<u32>(d + 0x2470) - 4);
    return 0;
}

s32 __fastcall Mp2StreamDecoder_ReadFrame(u8 *decoder, void *) {
    const u32 d = Addr(decoder);
    u32 header;  // (as the stream leaves it)
    StreamRead(d, Addr(&header), 4);
    if (Mp2Decoder_ParseHeader(decoder, nullptr, _byteswap_ulong(header))) return -1;
    StreamRead(d, d + 0x247c, Get<u32>(d + 0x2470));
    Put<u32>(d + 0x242c, 0);
    return 0;
}

s32 __fastcall Mp2StreamDecoder_ReadHeader(u8 *decoder, void *) {
    u32 header;  // (as the stream leaves it)
    StreamRead(Addr(decoder), Addr(&header), 4);
    return Mp2Decoder_ParseHeader(decoder, nullptr, _byteswap_ulong(header));
}

s32 __fastcall Mp2BufferDecoder_NextFrame(u8 *decoder, void *) {
    const u32 d = Addr(decoder);
    const s32 position = Get<s32>(d + 0x2430);
    Put<s32>(d + 0x242c, position);
    if (Mp2Decoder_ParseHeader(decoder, nullptr, _byteswap_ulong(Get<u32>(Get<u32>(d + 0x2428) + static_cast<u32>(position >> 3))))) return -1;
    const u32 next = Get<u32>(d + 0x242c) + 0x20;
    Put<u32>(d + 0x242c, next);
    Put<u32>(d + 0x2430, next + Get<u32>(d + 0x2470) * 8);
    return 0;
}

s32 __fastcall Mp2BufferDecoder_ReadHeader(u8 *decoder, void *) {
    return Mp2Decoder_ParseHeader(decoder, nullptr, _byteswap_ulong(Get<u32>(Get<u32>(Addr(decoder) + 0x2428))));
}

}  // extern "C"
