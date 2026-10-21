#include "movieplayer.h"

namespace {

template <typename T> T &at(u8 *base, u32 offset) { return *reinterpret_cast<T *>(base + offset); }

// The settings, read again at each use as the listing does.
u8 *settings(u8 *player) { return at<u8 *>(player, 0); }

u32 word(const u8 *p) { return (u32)p[0] | (u32)p[1] << 8; }

s32 signed_word(const u8 *p) { return (s16)(u16)word(p); }

u8 scaled(u8 value, u32 brightness) {
    const u32 v = (u32)value * brightness / 100;
    return (u8)(v > 0xff ? 0xff : v);
}

// The kind 3 / 4 header's size (signed words 0 and 2) and depth (byte 4).
void read_size(u8 *player, const u8 *data) {
    at<s32>(settings(player), 0x64) = signed_word(data);
    at<s32>(settings(player), 0x60) = signed_word(data + 2);
    at<u32>(player, 0x48) = data[4];
}

// A run of bytes copied as REP MOVSD then REP MOVSB do (forward, so an overlap repeats as theirs does).
void copy_forward(u8 *out, const u8 *source, u32 count) {
    for (u32 k = 0; k < count >> 2; ++k, out += 4, source += 4) *reinterpret_cast<u32 *>(out) = *reinterpret_cast<const u32 *>(source);
    for (u32 k = 0; k < (count & 3); ++k) *out++ = *source++;
}

}  // namespace

extern "C" {

void __fastcall MoviePlayer_ReadPalette(u8 *player, const u8 *data) {
    at<u32>(settings(player), 0x64) = word(data);
    at<u32>(settings(player), 0x60) = word(data + 2);
    at<u32>(player, 0x628) = word(data + 4);
    at<u32>(player, 0x62c) = word(data + 6);
    for (s32 i = 0; i < (s32)at<u32>(player, 0x62c); ++i) {
        const u8 *entry = data + 0xc + i * 3;
        const u32 brightness = at<u32>(settings(player), 0x78);
        const u8 r = scaled(entry[0], brightness), g = scaled(entry[1], brightness), b = scaled(entry[2], brightness);
        settings(player)[0xb4 + i * 3] = r;
        settings(player)[0xb5 + i * 3] = g;
        settings(player)[0xb6 + i * 3] = b;
    }
}

void __fastcall MoviePlayer_ReadSizeAt8(u8 *player, const u8 *data) {
    at<s32>(settings(player), 0x64) = signed_word(data + 8);
    at<s32>(settings(player), 0x60) = signed_word(data + 0xa);
}

void __fastcall MoviePlayer_ReadHeader(u8 *player, void *) {
    if (at<u32>(settings(player), 0x88)) return;
    at<u32>(player, 0x3a8) = at<u32>(player, 0x1f4);
    const u8 *data = at<const u8 *>(player, 0x1e8);
    if (!data) return;
    switch (at<u32>(player, 0x3a8)) {
    case 1: MoviePlayer_ReadPalette(player, data); return;
    case 3: read_size(player, data); return;
    case 4:
        read_size(player, data);
        at<u32>(player, 0x40) = data[7] & 2u;
        at<u32>(player, 0x44) = data[7] & 1u;   // (byte 7 read again after the store, as the listing does)
        return;
    case 5:
    case 6:
    case 7: MoviePlayer_ReadSizeAt8(player, data); return;
    default: return;
    }
}

void __fastcall MoviePlayer_BuildPixels(u8 *player, void *) {
    const u32 red = 8 - at<u32>(settings(player), 0x2c), green = 8 - at<u32>(settings(player), 0x30),
              blue = 8 - at<u32>(settings(player), 0x34);
    for (u32 i = 0; i < 0x100; ++i) {
        u8 *s = settings(player);
        u32 pixel = (u32)s[0xb4 + i * 3] >> (red & 31) << (at<u32>(s, 0x38) & 31) |
                    (u32)s[0xb5 + i * 3] >> (green & 31) << (at<u32>(s, 0x3c) & 31) |
                    (u32)s[0xb6 + i * 3] >> (blue & 31) << (at<u32>(s, 0x40) & 31);
        if (s[4] & 0x40) pixel |= pixel << 16;
        at<u32>(s, 0x3b4 + i * 4) = pixel;
    }
}

void __fastcall MoviePlayer_UnpackFields(s32 *out, const u8 *data, u32 count, u32 bits) {
    const u32 sign = 1u << ((bits - 1) & 31), high = 0xffffffffu << (bits & 31), mask = (1u << (bits & 31)) - 1;
    u32 position = (count - 1) * bits;
    out += count;
    for (u32 n = count; n; --n) {
        u32 v = *reinterpret_cast<const u32 *>(data + (position >> 3)) >> (position & 7) & mask;
        position -= bits;
        if (v & sign) v |= high;
        *--out = (s32)v;
    }
}

void __fastcall MoviePlayer_PlaceView(u8 *view, void *) {
    at<u32>(view, 0x14) = 0;
    at<u32>(view, 0x18) = 0;
    u8 *s = settings(view);
    at<u32>(view, 0xc) = at<u32>(s, 0x64);
    at<u32>(view, 0x10) = at<u32>(s, 0x60);
    if (s[4] & 0x40) at<u32>(view, 0xc) <<= 1;
    if (s[4] & 0x30) at<u32>(view, 0x10) <<= 1;
    if (s[4] & 4) at<s32>(view, 0x14) = (s32)(at<u32>(view, 4) - at<u32>(view, 0xc)) >> 1;
    if (s[4] & 8) at<s32>(view, 0x18) = (s32)(at<u32>(view, 8) - at<u32>(view, 0x10)) >> 1;
    if (s[4] & 1) at<u32>(view, 0x14) += at<u32>(s, 0x68);
    if (s[4] & 2) at<u32>(view, 0x18) += at<u32>(s, 0x6c);
    if (s[4] & 0x80) at<u32>(view, 0xc) = at<u32>(s, 0x70);
    if (s[5] & 1) at<u32>(view, 0x10) = at<u32>(s, 0x74);
}

void __fastcall MoviePlayer_DrawFrame(u8 *player, u8 *out, u32, u32 pitch) {
    const u8 *source = at<const u8 *>(player, 0x648);
    if (at<u32>(settings(player), 0x48) == 8) {
        if (settings(player)[4] & 0x40) {
            MoviePixels_Double(source, out, at<u32>(settings(player), 0x60) * at<u32>(settings(player), 0x64));
            return;
        }
        if (at<u32>(settings(player), 0x60) == 0) return;
        const u32 step = (u32)((s32)pitch >> 2 << 2);
        u32 row = 0;
        do {
            copy_forward(out, source, at<u32>(settings(player), 0x64));
            out += step;
            source += at<u32>(settings(player), 0x64);
        } while (++row < at<u32>(settings(player), 0x60));
        return;
    }
    const bool wide = (settings(player)[4] & 0x40) != 0;
    const u32 *pixels = reinterpret_cast<const u32 *>(settings(player) + 0x3b4);
    if (at<u32>(settings(player), 0x60) == 0) return;
    u32 row = 0;
    do {
        if (at<u32>(settings(player), 0x64) != 0) {
            u32 x = 0;
            do {
                x += 8;
                for (int k = 0; k < 8; ++k) {
                    if (wide) {
                        reinterpret_cast<u32 *>(out)[k] = pixels[source[k]];
                    } else {
                        reinterpret_cast<u16 *>(out)[k] = *reinterpret_cast<const u16 *>(&pixels[source[k]]);
                    }
                }
                source += 8;
                out += wide ? 0x20 : 0x10;
            } while (x < at<u32>(settings(player), 0x64));
        }
        const u32 width = at<u32>(settings(player), 0x64);
        out += wide ? (pitch - width * 4) >> 2 << 2 : (pitch - width * 2) >> 1 << 1;
    } while (++row < at<u32>(settings(player), 0x60));
}

void __fastcall MoviePlayer_ExpandBlocks(const u8 *records, u8 *out, s32 count) {
    for (; count > 0; --count, records += 8, out += 0x10) {
        const u32 indices = *reinterpret_cast<const u32 *>(records + 4);
        for (s32 k = 0; k < 0x10; ++k) out[0xf - k] = records[indices >> (k * 2) & 3];
    }
}
}
