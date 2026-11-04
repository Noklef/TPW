// The rides read back from a save: see ridesload.h.
#include "ridesload.h"

#include <cstdint>

#include "../files/file.h"

namespace {

AddrCalls g_calls;

template <typename T>
T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}
template <typename T>
T &g(u32 address) {
    return *reinterpret_cast<T *>((uintptr_t)address);
}

// A file's bits to the bits they set: {file, set}.
struct BitMap {
    u32 from, to;
};
const BitMap kRideBits[] = {{1, 0x20}, {2, 0x40}, {4, 0x80}, {8, 0x100}, {0x10, 0x200}, {0x20, 0x400}, {0x40, 0x800}, {0x80, 0x40000}, {0x100, 0x8000000}};
const BitMap kPartBits[] = {{1, 0x10}, {2, 0x20}, {4, 0x8000}, {8, 0x10000}, {0x20, 0x40000}, {0x40, 0x80000}, {0x80, 0x200000}, {0x100, 0x400000}, {0x800, 0x100000}, {0x200, 0x20000000}, {0x400, 0x80000000}};

template <u32 N>
void SetBits(u32 &to, u32 from, const BitMap (&map)[N]) {
    for (u32 i = 0; i < N; ++i)
        if (from & map[i].from) to |= map[i].to;
}

// A game's bits back to the file's.
template <u32 N>
u32 FileBits(u32 from, const BitMap (&map)[N]) {
    u32 bits = 0;
    for (u32 i = 0; i < N; ++i)
        if (from & map[i].to) bits |= map[i].from;
    return bits;
}

// A ride's record, after its tag: past what it wrote.
u8 *SaveRecord(u8 *cursor, u8 *ride) {
    at<u32>(cursor, 0) = at<u32>(ride, 0xf8);
    at<u32>(cursor, 4) = at<u32>(ride, 0xe8);
    at<u32>(cursor, 8) = at<u32>(ride, 0xec);
    at<u32>(cursor, 0xc) = at<u32>(ride, 0xf0);
    at<u32>(cursor, 0x10) = at<u32>(ride, 0xf4);
    at<u32>(cursor, 0x14) = at<u32>(ride, 0xe4);
    at<u32>(cursor, 0x18) = at<u32>(ride, 0xd4);
    at<u32>(cursor, 0x1c) = FileBits(at<u32>(ride, 4), kRideBits);
    at<u16>(cursor, 0x20) = at<u16>(ride, 0xc);
    at<u32>(cursor, 0x22) = at<u32>(ride, 0xb8);
    cursor += 0x26;
    u8 *const held = at<u8 *>(ride, 8);
    u8 *const model = at<u8 *>(held, 4);
    at<u32>(cursor, 0) = at<u32>(held, 0x10);
    at<u16>(cursor, 4) = at<u16>(model, 0x42);
    at<u16>(cursor, 6) = at<u16>(model, 0x48);
    u8 *const motions = at<u8 *>(held, 0x28);
    if (motions) {
        at<u32>(cursor, 8) = at<u32>(motions, 0);
        at<u32>(cursor, 0xc) = at<u32>(motions, 8);
        cursor += 0x10;
        const u8 *vertex = at<u8 *>(motions, 4);
        for (u32 i = 0; i < at<u16>(model, 0x48); ++i, cursor += 8, vertex += 0x14) {
            at<u32>(cursor, 0) = at<const u32>(vertex, 0);
            at<u32>(cursor, 4) = at<const u32>(vertex, 0xc);
        }
    } else {
        cursor += 0x10;   // sic: the size counted the vertices all the same
    }
    for (u32 i = 0; i < at<u16>(model, 0x42); ++i, cursor += 4) {
        const u32 first = at<u16>(model, 0x44);
        const u32 part = i < first ? at<u32>(at<u8 *>(model, 0x70), 0xa0 * i) : at<u32>(at<u8 *>(model, 0x74), 0x58 * (i - first));
        at<u32>(cursor, 0) = FileBits(part, kPartBits);
    }
    u8 *layer = at<u8 *>(ride, 0x10);
    for (u32 i = 0; i < at<u16>(ride, 0xe); ++i, cursor += 0x2c, layer += 0x38) {
        g_calls.Cd(0x00464580, layer, ride);
        at<u32>(cursor, 0) = at<u32>(layer, 0);
        at<u32>(cursor, 4) = at<u32>(layer, 4);
        at<u32>(cursor, 8) = at<u32>(layer, 8);
        at<u32>(cursor, 0xc) = at<u32>(layer, 0x10);
        at<u32>(cursor, 0x10) = at<u32>(layer, 0x14);
        at<u32>(cursor, 0x14) = at<u32>(layer, 0x18);
        at<u32>(cursor, 0x18) = at<u32>(layer, 0xc);
        for (u32 k = 0; k < 4; ++k)
            at<u32>(cursor, 0x1c + 4 * k) = at<u32>(layer, 0x24 + 4 * k);
    }
    return cursor;
}

bool ReadFailed(Stream *stream, void *to, u32 count) {
    s32 result;
    return *Stream_Read(&result, stream, to, (s32)count, nullptr) < 0;
}

// The type looked for from the table's last entry: its index, 0 when none.
u32 FindType(u32 id) {
    for (u32 i = g<u32>(0x007a439c); i; --i)
        if (at<u32>(g<u8 *>(0x007ae6a8 + 4 * i), 0xf8) == id) return i;
    return 0;
}

// The next record's rest (after its tag): at the ride's end, past what it read.
const u8 *LoadRecord(const u8 *record, u32 serial) {
    const u8 *const counts = record + 0x26;
    const u8 *data = counts + 0x10;
    const u32 type = FindType(at<const u32>(record, 0));
    u32 desc[4];
    desc[0] = at<const u32>(record, 4);
    desc[1] = at<const u32>(record, 8);
    desc[2] = at<const u32>(counts, 0);
    const u32 sizeX = at<const u32>(record, 0xc), sizeY = at<const u32>(record, 0x10);
    const u32 definition = g_calls.Th(0x00412e90, 0x007890a0u, at<const u32>(record, 0));   // ObjectDefs_Find
    if (definition) {
        desc[3] = at<u32>(reinterpret_cast<const void *>((uintptr_t)definition), 0x18);
    } else if (sizeX && sizeY) {
        g<u32>(0x007a2ed8) = sizeX;
        g<u32>(0x007a2edc) = sizeY;
        desc[3] = 0x007a2ed8;
    } else {
        desc[3] = 0;
    }
    const u32 id = g_calls.Cd(0x00463060, type, at<const u32>(record, 0x14), desc, 0u, at<const u32>(record, 0x18), 0u, sizeX, sizeY, serial, 1u);   // Ride_Create
    u8 *const ride = g<u8 *>(0x007a4610 + 4 * id);
    u8 *const held = at<u8 *>(ride, 8);
    u8 *const model = at<u8 *>(held, 4);
    const bool same = at<const u16>(counts, 6) == at<u16>(model, 0x48) && at<const u16>(counts, 4) == at<u16>(model, 0x42);
    u32 flags = at<u32>(ride, 4) & 0xf7fff01f;
    SetBits(flags, at<const u32>(record, 0x1c), kRideBits);
    at<u32>(ride, 4) = flags;
    at<u16>(ride, 0xc) = at<const u16>(record, 0x20);
    at<u32>(ride, 0xb8) = at<const u32>(record, 0x22);
    g_calls.Cd(0x004547f0, ride);   // StripAnim_Refresh
    u8 *const vertices = at<u8 *>(held, 0x28);
    if (vertices && same) {
        at<u32>(vertices, 0) = at<const u32>(counts, 8);
        at<u32>(vertices, 8) = at<const u32>(counts, 0xc);
        u8 *to = at<u8 *>(vertices, 4);
        for (u32 i = 0; i < at<u16>(at<u8 *>(at<u8 *>(ride, 8), 4), 0x48); ++i, data += 8, to += 0x14) {
            at<u32>(to, 0) = at<const u32>(data, 0);
            at<u32>(to, 0xc) = at<const u32>(data, 4);
        }
    } else {
        data += 8 * at<const u16>(counts, 6);
    }
    if (same) {
        for (u32 i = 0; i < at<u16>(model, 0x42); ++i, data += 4) {
            const u32 first = at<u16>(model, 0x44);
            u32 &part = i < first ? at<u32>(at<u8 *>(model, 0x70), 0xa0 * i) : at<u32>(at<u8 *>(model, 0x74), 0x58 * (i - first));
            u32 value = part & 0x5f827fcf;
            part = value;
            SetBits(value, at<const u32>(data, 0), kPartBits);
            part = value;
        }
    } else {
        data += 4 * at<const u16>(counts, 4);
    }
    u8 *layer = at<u8 *>(ride, 0x10);
    u32 unset = 0;
    for (u32 i = 0; i < at<u16>(ride, 0xe); ++i, data += 0x2c, layer += 0x38) {
        at<u32>(layer, 0) = at<const u32>(data, 0);
        at<u32>(layer, 4) = at<const u32>(data, 4);
        at<u32>(layer, 8) = at<const u32>(data, 8);
        at<u32>(layer, 0x10) = at<const u32>(data, 0xc);
        at<u32>(layer, 0x14) = at<const u32>(data, 0x10);
        at<u32>(layer, 0x18) = at<const u32>(data, 0x14);
        at<u32>(layer, 0xc) = at<const u32>(data, 0x18);
        if (at<u8>(layer, 0) & 4) at<u32>(layer, 0) |= 0x10;
        for (u32 k = 0; k < 4; ++k)
            at<u32>(layer, 0x24 + 4 * k) = at<const u32>(data, 0x1c + 4 * k);
        const u32 bank = at<u32>(layer, 4), entry = at<u32>(layer, 8);
        u8 *self = ride;
        const u32 block = g_calls.Cd(0x00472d10, &self, bank, entry) ? at<u32>(at<u8 *>(self, 0x20 + 0xc * bank), 4 + 8 * entry) : 0;   // Animator_HasMotion
        if (block) g_calls.Cd(0x00472cb0, layer, block);   // Layer_SetBlock
        if (!at<u32>(layer, 4)) ++unset;
        g_calls.Cd(0x00464580, layer, ride);
    }
    if (!unset) g_calls.Cd(0x00473550, ride);
    layer = at<u8 *>(ride, 0x10);
    for (u32 i = 0; i < at<u16>(ride, 0xe); ++i, layer += 0x38)
        if (at<u32>(layer, 4) < 0xc) at<u32>(ride, 4) |= 0x400000;
    g_calls.Cd(at<u32>(ride, 4) & 0x1000000 ? 0x00472f40u : 0x00472ee0u, ride);
    return data;
}

}  // namespace

extern "C" {

void RidesLoad_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

u32 __cdecl RideSystem_Load(Stream *stream) {
    u32 size;
    if (ReadFailed(stream, &size, 4))
        return 0;
    u8 *const block = reinterpret_cast<u8 *>((uintptr_t)g_calls.Cd(0x0045b9d0, size, 0u));   // GlobalMem_Alloc
    if (ReadFailed(stream, block, size))
        return 0;
    g<u32>(0x007a4084) = at<u32>(block, 0);
    g<u32>(0x007a3ebc) = at<u32>(block, 4);
    g<u32>(0x007aedf4) = at<u32>(block, 8);
    const u8 *data = block + 0xc;
    g_calls.Cd(0x00473440);   // Motion_UpdateClocks
    for (u32 i = 0; i < g<u32>(0x007a3ebc) + g<u32>(0x007a4084); ++i) {
        const u8 tag = *data++;
        if (tag == 1) data = LoadRecord(data, i + 1);
    }
    g_calls.Cd(0x0045ba00, block);   // GlobalMem_Free
    return 1;
}

u32 __cdecl RideSystem_Save(Stream *stream) {
    u32 size = 0xc;
    for (u32 i = 0; i < g<u32>(0x007a3ebc) + g<u32>(0x007a4084); ++i) {
        const u8 *const ride = g<u8 *>(0x007a4614 + 4 * i);
        ++size;
        if (ride) {
            const u8 *const model = at<u8 *>(at<u8 *>(ride, 8), 4);
            size += 0x36 + at<u16>(ride, 0xe) * 0x2c + at<u16>(model, 0x48) * 8 + at<u16>(model, 0x42) * 4;
        }
    }
    u8 *const block = reinterpret_cast<u8 *>((uintptr_t)g_calls.Cd(0x0045b9d0, size, 1u));   // GlobalMem_Alloc
    at<u32>(block, 0) = g<u32>(0x007a4084);
    at<u32>(block, 4) = g<u32>(0x007a3ebc);
    at<u32>(block, 8) = g<u32>(0x007aedf4);
    u8 *cursor = block + 0xc;
    for (u32 i = 0; i < g<u32>(0x007a3ebc) + g<u32>(0x007a4084); ++i) {
        u8 *const ride = g<u8 *>(0x007a4614 + 4 * i);
        *cursor++ = ride ? 1 : 0;
        if (ride) cursor = SaveRecord(cursor, ride);
    }
    s32 result;
    if (*Stream_Write(&result, stream, &size, 4, nullptr) < 0) return 0;
    if (*Stream_Write(&result, stream, block, (s32)size, nullptr) < 0) return 0;
    g_calls.Cd(0x0045ba00, block);   // GlobalMem_Free
    return 1;
}
}
