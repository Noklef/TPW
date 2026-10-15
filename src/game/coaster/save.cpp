#include "save.h"

#include <cstdint>
#include <cstring>

#include "../files/file.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

template <typename T> void set(u8 *record, u32 offset, T value) { std::memcpy(record + offset, &value, sizeof value); }

void write(Stream *stream, const void *data, u32 size) {
    s32 result;
    Stream_Write(&result, stream, data, (s32)size, nullptr);
}

// The word behind each of the piece's +0xb0 pointers from +0xb4.
void write_words(Stream *stream, const u8 *piece) {
    for (u32 k = 0; k < at<u32>(piece, 0xb0); ++k) write(stream, at<const u16 *>(piece, 0xb4 + 4 * k), 2);
}

// Each set bit of flags in from[] gives the bit of its index.
u32 remap(u32 flags, const u32 *from, u32 count) {
    u32 out = 0;
    for (u32 i = 0; i < count; ++i)
        if (flags & from[i]) out |= 1u << i;
    return out;
}

}  // namespace

extern "C" {

u32 __cdecl Coasters_Save(Stream *stream) {
    const u8 *first = *reinterpret_cast<const u8 *const *>(0x00790fe0);
    u32 links = 0;
    for (const u8 *link = first; link; link = at<const u8 *>(link, 0x18)) ++links;
    write(stream, &links, 4);
    write(stream, reinterpret_cast<const void *>(0x00790b04), 4);
    write(stream, reinterpret_cast<const void *>(0x00790b00), 4);
    write(stream, reinterpret_cast<const void *>(0x00791ce0), 4);

    for (const u8 *link = first; link; link = at<const u8 *>(link, 0x18)) {
        u16 pieces = 0;
        for (const u8 *p = at<const u8 *>(link, 0x54); p; p = at<const u8 *>(p, 0x64))
            if (!(at<u8>(at<const u8 *>(p, 0x100), 4) & 0x10)) ++pieces;
        static const u32 kLinkFlags[] = {0x20, 0x40, 0x80, 0x100, 0x200, 0x400, 0x800};
        u8 record[0x22];
        set<u32>(record, 0, (at<u8>(link, 0x3c) & 3) | remap(at<u32>(link, 0), kLinkFlags, 7) << 2);
        set<u32>(record, 4, at<u32>(link, 8));
        set<u16>(record, 8, at<u16>(link, 0xc));
        set<u16>(record, 0xa, at<u16>(link, 0x10));
        set<u16>(record, 0xc, at<u16>(link, 0x14));
        set<u16>(record, 0xe, at<u16>(link, 0x1c));
        set<u16>(record, 0x10, at<u16>(link, 0x20));
        set<u32>(record, 0x12, at<u32>(link, 0x2c));
        set<u16>(record, 0x16, pieces);
        set<u16>(record, 0x18, at<u16>(link, 0xf4));
        set<u16>(record, 0x1a, at<u16>(link, 0xf0));
        set<u32>(record, 0x1c, at<u32>(link, 0x140));
        write(stream, record, 0x20);

        for (const u8 *p = at<const u8 *>(link, 0x54); p; p = at<const u8 *>(p, 0x64)) {
            if (at<u32>(p, 0) == 2) {
                write(stream, &at<const u32>(p, 0xb0), 4);
                write_words(stream, p);
            }
            const u8 *part = at<const u8 *>(p, 0x100);
            if (at<u8>(part, 4) & 0x10) continue;
            static const u32 kPieceFlags[] = {1, 2, 4, 0x80, 0x40, 0x100, 0x10000, 0x20000, 0x40000, 0x80000,
                                              0x200000, 0x400000};
            set<u32>(record, 0, remap(at<u32>(p, 8), kPieceFlags, 12));
            set<u32>(record, 4, at<u32>(p, 0x70));
            set<u16>(record, 8, at<u16>(part, 0xc));
            set<u16>(record, 0xa, at<u16>(part, 0x10));
            set<u8>(record, 0xc, at<u8>(p, 4));
            set<u32>(record, 0xd, at<u32>(p, 0xe8));
            set<u32>(record, 0x11, at<u32>(p, 0xd8));
            set<u32>(record, 0x15, at<u32>(p, 0xdc));
            set<u32>(record, 0x19, at<u32>(p, 0xe0));
            set<u32>(record, 0x1d, at<u32>(p, 0xf8));
            set<u8>(record, 0x21, at<u8>(p, 0xb0));
            write(stream, record, 0x22);
            write_words(stream, p);
        }

        const u8 *car = at<const u8 *>(link, 0x128);
        for (u32 j = 0; j < at<u32>(at<const u8 *>(link, 4), 0x2e8); ++j, car += 0x80) {
            set<u32>(record, 0, at<u32>(car, 0));
            set<u8>(record, 4, at<u8>(car, 0xc));
            set<u32>(record, 5, at<u32>(car, 0x74));
            set<u32>(record, 9, at<u32>(car, 0x78));
            set<u32>(record, 0xd, at<u32>(car, 0x38));
            set<u32>(record, 0x11, at<u32>(car, 0x64));
            set<u32>(record, 0x15, at<u32>(car, 0x68));
            set<u16>(record, 0x19, at<u16>(car, 0x10));
            write(stream, record, 0x1b);
            const u8 *seat = at<const u8 *>(car, 0x28);
            for (u32 m = 0; m < at<u32>(at<const u8 *>(car, 4), 4); ++m, seat += 0x60) {
                set<u32>(record, 0, at<u32>(seat, 0));
                set<u8>(record, 4, at<u8>(seat, 0x30));
                set<u8>(record, 5, at<u8>(seat, 0x34));
                set<u8>(record, 6, at<u8>(seat, 0x44));
                set<u32>(record, 7, at<u32>(seat, 0xc));
                write(stream, record, 0xb);
                write(stream, at<const void *>(seat, 0x28), at<u32>(seat, 0x30) << 2);
                write(stream, at<const void *>(seat, 0x2c), at<u32>(seat, 0x34) << 2);
            }
        }
    }
    return 1;
}
}
