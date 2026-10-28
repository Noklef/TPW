#include "trackrides.h"

#include <cstdint>

#include "../model/holder.h"
#include "../model/parts.h"
#include "../model/partstate.h"
#include "../motion/animator.h"
#include "../core/soundplay.h"
#include "../math/x87.h"
#include "../render/particles.h"
#include "../core/soundset.h"
#include "collobj.h"
#include "queuepieces.h"
#include "ridelist.h"
#include "trackpools.h"
#include "things.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

bool loaded() { return global<u32>(0x00877b58) == 0x4a454647; }

s32 Ftol(X87Reg x) { return (s32)(long long)x.v; }

// Particle_Spawn(0x13, or effect) at 1024 times the position +0x40..+0x48 of the root (+0x78) of the model (+4) of the instance's holder
// (+8): z through __ftol first.
void SeatEffect(const u8 *instance, s32 effect = 0x13) {
    const u8 *const root = at<const u8 *>(at<const u8 *>(at<const u8 *>(instance, 8), 4), 0x78);
    const s32 z = Ftol(X87Reg(at<float>(root, 0x48)) * 1024.0f);
    const s32 y = Ftol(X87Reg(at<float>(root, 0x44)) * 1024.0f);
    const s32 x = Ftol(X87Reg(at<float>(root, 0x40)) * 1024.0f);
    Particle_Spawn(effect, x, y, z);
}

// The ride a handle indexes, or 0; whether the handle also names it (its index and serial).
u8 *RideOf(u32 handle) {
    return loaded() ? reinterpret_cast<u8 *>(global<uintptr_t>(0x00877b60) + (handle & 0xff) * 0xd0) : nullptr;
}

u32 NameOf(const u8 *ride) {
    if (!loaded())
        return 0;
    return (u32)((s32)((uintptr_t)ride - global<uintptr_t>(0x00877b60)) / 0xd0) | at<u32>(ride, 0) << 8;
}

// A track section's collision node (CollNode_Alloc(x, y, size)) put at the head of its list (+0x1c; the node's +0x14 the next, +0x18 the
// section). Returns it (0 for none).
u8 *SectionNode(u8 *section, u32 x, u32 y, u32 size) {
    u8 *const node = CollNode_Alloc(x, y, size);
    if (node) {
        at<u8 *>(node, 0x14) = at<u8 *>(section, 0x1c);
        at<u8 *>(section, 0x1c) = node;
        at<u8 *>(node, 0x18) = section;
    }
    return node;
}

// That node's flags (+0) cleared of clear and given set (no node: a write at address 0, faulting as the original does).
u8 *Flagged(u8 *node, u32 clear, u32 set) {
    at<u32>(node, 0) = (at<u32>(node, 0) & ~clear) | set;
    return node;
}

// A car's position dword (+0x34 or +0x38) as the sounds take it: fild; fmul 0.0033036 (0x700f20); __ftol.
s32 SoundPlace(const u8 *car, u32 offset) { return Ftol(X87_MulIntDouble(at<s32>(car, offset), global<float>(0x00700f20))); }

// The first thing in use (newest first) that is an object (+2 is 3) of the ride (its dword +0x28 the name): its handle, else 0
// (TrakRide_FindThing's search, inlined, through Thing_ByHandle).
u16 RideObject(u32 ride) {
    void *const world = global<void *>(0x007cf83c);
    u16 handle;
    World_FirstThing(world, nullptr, &handle);
    while (handle != 0) {
        const u8 *const thing = Thing_ByHandle(handle);
        if (thing[2] == 3 && at<u32>(thing, 0x28) == ride) return *Thing_GetHandle(thing, nullptr, &handle);
        handle = *World_NextThing(world, nullptr, &handle, thing);
    }
    return 0;
}

}  // namespace

extern "C" {

s32 __cdecl TrakChunk_Write(u8 *chunk, const void *buffer, s32 count) {
    s32 result, done;
    if (*Stream_Write(&result, at<Stream *>(chunk, 4), buffer, count, &done) < 0) return 0;
    at<s32>(chunk, 0x10) += count;
    return done;
}

void *__cdecl TrakMem_Alloc(u32 size) { return size ? (*Files_GetEnv()->globalAlloc)(0x40, size) : nullptr; }  // (GMEM_ZEROINIT)

u8 *__cdecl TrakChunk_Begin(u32 version, Stream *stream) {
    u8 *chunk = static_cast<u8 *>((*Files_GetEnv()->globalAlloc)(0x40, 0x18));
    if (!chunk)
        return nullptr;
    at<u32>(chunk, 0) = Stream_Tell(stream);
    at<u32>(chunk, 0xc) = version;
    at<Stream *>(chunk, 4) = stream;
    at<u32>(chunk, 8) = 0;
    at<u32>(chunk, 0x10) = 0;
    at<u32>(chunk, 0x14) = 0;
    s32 result, done;
    if (*Stream_Write(&result, stream, chunk + 0xc, 0xc, &done) >= 0)
        at<u32>(chunk, 0x10) += 0xc;
    return chunk;
}

u32 __cdecl TrakChunk_End(u8 *chunk) {
    Stream *stream = at<Stream *>(chunk, 4);
    const u32 end = Stream_Tell(stream);
    at<u32>(chunk, 0x14) = end - at<u32>(chunk, 0);
    Stream_Seek(at<Stream *>(chunk, 4), (s32)at<u32>(chunk, 0), 0);
    s32 result, done;
    if (*Stream_Write(&result, at<Stream *>(chunk, 4), chunk + 0xc, 0xc, &done) >= 0)
        at<u32>(chunk, 0x10) += 0xc;
    Stream_Seek(at<Stream *>(chunk, 4), (s32)end, 0);
    (*Files_GetEnv()->globalFree)(chunk);
    return at<u32>(chunk, 0x14);  // (sic: after the free)
}

u32 __cdecl TrakChunk_Close(u8 *chunk) {
    Stream_Seek(at<Stream *>(chunk, 4), (s32)(at<u32>(chunk, 0) + at<u32>(chunk, 0x14)), 0);
    const u32 total = at<u32>(chunk, 0x14);
    (*Files_GetEnv()->globalFree)(chunk);
    return total;
}

s32 *__cdecl TrakStream_Read(s32 *result, Stream *stream, void *buffer, s32 count, s32 *done) {
    Stream_Read(result, stream, buffer, count, done);
    return result;
}

u8 *__cdecl TrakPool_FirstFree() {
    u8 *record = global<u8 *>(0x00877b68);
    if (!record) return nullptr;
    for (u32 i = 0; i < 0x100; ++i, record += 0xac) {
        if (!(record[0] & 1)) return record;
    }
    return nullptr;
}

u32 __cdecl TrakRide_Field5c(u32 handle) {
    const u8 *ride = RideOf(handle);
    return handle == NameOf(ride) ? at<u32>(ride, 0x5c) : 0;
}

u32 __cdecl TrakRide_PopNode(u32 handle) {
    u8 *ride = RideOf(handle);
    if (!loaded() || handle != NameOf(ride))
        return 0;
    u8 *node = at<u8 *>(ride, 0xc8);
    if (!node)
        return 0;
    at<u32>(ride, 0xc8) = at<u32>(node, 0x10);
    at<u32>(node, 0x10) = global<u32>(0x00877b8c);
    global<u32>(0x00877b8c) = (u32)(uintptr_t)node;
    return at<u32>(node, 0);
}

u32 __cdecl TrakChunk_Read(u8 *chunk, u8 *buffer, u32 count) {
    u32 n = at<u32>(chunk, 0x10) - at<u32>(chunk, 8);
    if (count < n)
        n = count;
    s32 result, done;
    Stream_Read(&result, at<Stream *>(chunk, 4), buffer, n, &done);
    if (result < 0)
        return 0;  // (the debug output is gone)
    at<u32>(chunk, 8) += done;
    for (u32 i = done; i < count && (u32)done < count; ++i)
        buffer[i] = 0;
    return done;
}

u32 __cdecl TrakChunk_ReadDword(u8 *chunk) {
    u32 value = (u32)(uintptr_t)chunk;  // (the slot the read goes to starts as the chunk pointer)
    u32 n = at<u32>(chunk, 0x10) - at<u32>(chunk, 8);
    if (4 < n)
        n = 4;
    s32 result, done;
    Stream_Read(&result, at<Stream *>(chunk, 4), &value, n, &done);
    if (result < 0)
        return 0;
    at<u32>(chunk, 8) += done;
    return done == 4 ? value : 0;
}

namespace {

s32 ShiftDown8(s32 v) { return (s32)((u32)v + ((u32)(v >> 31) & 0xffu)) >> 8; }

// A piece (a buoy) put at the end of the ride's chain (+0xa8, counted at +0xac, the next at +0x14); its ride (+0x10) is set.
void AppendPiece(u8 *ride, u8 *piece) {
    at<u8 *>(piece, 0x10) = ride;
    u8 *tail = at<u8 *>(ride, 0xa8);
    at<s32>(ride, 0xac) += 1;
    if (!tail) {
        at<u8 *>(ride, 0xa8) = piece;
    } else {
        while (u8 *next = at<u8 *>(tail, 0x14))
            tail = next;
        at<u8 *>(tail, 0x14) = piece;
    }
}

u32 NameOfRide(const u8 *ride) { return NameOf(ride); }

// The end piece of the pieces along a line or of a single piece: flag orientation of the ride (+0xb0) turns the offset 0xc00 along x or y and mirrors it.
void EndOffsets(const u8 *ride, s32 &dx, s32 &dy) {
    if ((at<u8>(ride, 0xb0) & 1) == 0) {
        dx = -0xc00;
        dy = 0;
    } else {
        dx = 0;
        dy = -0xc00;
    }
    if (at<u32>(ride, 0xb0) & 2u)
        dx = -dx;
    if (at<u32>(ride, 0xb0) & 4u)
        dy = -dy;
}

}  // namespace

void __cdecl TrakRide_Fixed(u32 handle) {
    u8 *ride = RideOf(handle);
    if (handle != NameOf(ride))
        return;
    const u32 was = at<u32>(ride, 0x54);
    at<u32>(ride, 0x54) = 0;
    if (was != 2)
        return;
    const s32 kind = at<s32>(ride, 0);
    if (!(kind > -15 && (kind < -2 || kind == -1)))
        return;
    u8 *record = global<u8 *>(0x00877b68);
    u32 left = global<u32>(0x00877b6c);
    while (left != 0) {
        if ((at<u8>(record, 0) & 1) && (--left, at<u8 *>(record, 0x9c) == ride) && at<u32>(record, 0x2c) != 0xffffffffu) {
            ParticleSystem_End(at<u32>(record, 0x2c));
            at<u32>(record, 0x2c) = 0xffffffffu;
            if (at<s32>(at<u8 *>(record, 0x9c), 0) == -1 && (at<u32>(record, 0) & 0x4000)) {
                at<u32>(record, 0x10) = 5;
                if (at<u32>(record, 8) != 0)
                    Layer_ClearQueue(reinterpret_cast<u8 *const *>(0x007a4610)[at<u32>(record, 8)], 0);
                if (at<u32>(record, 8) != 0)
                    Animator_Play(reinterpret_cast<u8 *const *>(0x007a4610)[at<u32>(record, 8)], 5, 0, 1, 1.0f, 0);
            }
        }
        record += 0xac;
    }
}

void __cdecl TrakRide_SetPerformance(u32 handle, s32 percent) {
    u8 *ride = RideOf(handle);
    if (handle != NameOf(ride))
        return;
    if (percent < 0)
        percent = 0;
    else if (percent > 99)
        percent = 100;
    at<s32>(ride, 0x1c) = percent;
    for (u32 i = 0; i < 4; ++i) {
        const s32 from = at<s32>(ride, 0x30 + 4 * i), to = at<s32>(ride, 0x40 + 4 * i);
        at<s32>(ride, 0x20 + 4 * i) = (to - from) * percent / 100 + from;
    }
}

s32 __cdecl TrakRide_CountPieces(u32 handle, s32 *others, s32 *longestRun, s32 *kind11, s32 *free) {
    u8 *ride = RideOf(handle);
    if (handle != NameOf(ride))
        return 0;
    s32 total = 0, run = 0, kind11Count = 0, longest = 0, freeCount = 0, otherCount = 0;
    for (s32 pass = 2; pass != 0; --pass) {
        for (u8 *piece = at<u8 *>(ride, 0xbc); piece; piece = at<u8 *>(piece, 0x24)) {
            switch (at<u32>(piece, 4) & 0xffff) {
            case 11:
                ++run;
                ++kind11Count;
                break;
            case 12:
            case 13:
                if (at<s32>(piece, 8) == 0)
                    ++freeCount;
                ++run;
                break;
            case 9:
            case 10:
                ++run;
                break;
            default:
                if (longest < run)
                    longest = run;
                run = 0;
                ++otherCount;
            }
            ++total;
        }
    }
    if (others)
        *others = otherCount / 2;
    if (longestRun)
        *longestRun = longest;
    if (kind11)
        *kind11 = kind11Count / 2;
    if (free)
        *free = freeCount;
    return total / 2;
}

u32 __cdecl TrakRide_AddNextCar(u32 handle, u32 value) {
    u8 *ride = RideOf(handle);
    if (handle != NameOf(ride) || at<u32>(ride, 0x50) == 0)
        return 0;
    u8 *node = global<u8 *>(0x00877b8c);
    if (!node)
        return 0;
    global<u32>(0x00877b8c) = at<u32>(node, 0x10);
    at<u32>(node, 0x10) = at<u32>(ride, 0xc4);
    at<u8 *>(ride, 0xc4) = node;
    at<u32>(node, 0) = value;
    at<u32>(node, 4) = handle;
    at<u32>(node, 8) = 0xffffffffu;
    return 1;
}

void __cdecl TrakRide_Open(u32 handle) {
    u8 *ride = RideOf(handle);
    if (handle != NameOf(ride))
        return;
    const s32 kind = at<s32>(ride, 0);
    u32 state = 1;
    switch (kind) {
    case -13: case -12: case -8: case -5:
        state = 2;
        [[fallthrough]];
    case -10: case -9: case -7: case -4: {
        if (at<u32>(ride, 0x50) != 0)
            return;
        u8 *head = at<u8 *>(ride, 0xa8);
        u8 *piece = head;
        while ((at<u8>(piece, 0) & 4) == 0)
            piece = at<u8 *>(piece, 0x14);
        do {
            piece = at<u8 *>(piece, 0x14);
            if (!piece)
                piece = head;
        } while (at<u8>(piece, 0) & 2);
        at<u8 *>(ride, 0xa4) = piece;
        break;
    }
    default:
        break;
    }
    at<u32>(ride, 0x50) = state;
    TrakRide_SetPerformance(NameOf(ride), at<u32>(ride, 0x1c));
}

u32 __cdecl TrakRide_Create(s32 kind, s32 x, s32 y, u32 orientation, u32 handle, u32 value) {
    if (global<u32>(0x00877b58) != 0x4a454647)
        return 0;
    u8 *ride;
    if (handle != 0) {
        ride = global<u8 *>(0x00877b60) + (handle & 0xff) * 0xd0;
        kind = (s32)handle >> 8;
    } else {
        ride = global<u8 *>(0x00877b60);
        if (ride) {
            s32 i = 0;
            while (at<u32>(ride, 0) != 0) {
                ++i;
                ride += 0xd0;
                if (i >= 0x40) {
                    ride = nullptr;
                    break;
                }
            }
        }
    }
    if (!ride)
        return NameOfRide(ride);  // (faults: the original reads the first dword of the missing ride)
    const u32 *from = reinterpret_cast<const u32 *>((uintptr_t)(0x00764178 - (u32)(kind + 1) * 0xd0));
    for (u32 i = 0; i < 0x34; ++i)
        at<u32>(ride, 4 * i) = from[i];
    global<u32>(0x00877b64) += 1;
    at<u32>(ride, 0xb0) = orientation;
    at<u32>(ride, 0x5c) = 0;
    at<u32>(ride, 0x10) = value;
    at<u32>(ride, 0xac) = 0;
    at<u32>(ride, 0xa8) = 0;
    at<u32>(ride, 0xc0) = 0;
    at<u32>(ride, 0xbc) = 0;
    at<s32>(ride, 0xb4) = x;
    at<s32>(ride, 0xb8) = y;
    at<u32>(ride, 0xcc) = 0;
    at<u32>(ride, 0x58) = 0;
    const s32 own = at<s32>(ride, 0);
    switch (own) {
    case -14: case -11: case -6: case -3: case -1: {
        u8 *node;
        s32 d;
        if (own == -11) {
            node = CollNode_Alloc(x + 0x600, y + 0x600, 0x1100);
            d = 0xb00;
        } else if (own == -3 || own != -1) {
            node = CollNode_Alloc(x + 0x600, y + 0x600, 0x1600);
            d = 0x1000;
        } else {
            node = CollNode_Alloc(x + 0x600, y + 0x600, 0x1200);
            d = 0xc00;
        }
        at<u8 *>(node, 0x14) = at<u8 *>(ride, 0xc0);
        at<u8 *>(ride, 0xc0) = node;
        x += 0x600;
        y += 0x600;
        s32 px = 0;
        at<s32>(ride, 0x90) = x;
        at<s32>(ride, 0x74) = x;
        at<s32>(ride, 0x94) = y;
        at<s32>(ride, 0x78) = y;
        for (int n = 8; n != 0; --n) {
            if (u8 *piece = Buoy_Alloc(px + x, d + y))
                AppendPiece(ride, piece);
            const s32 cosine = global<s32>(0x00877458), sine = global<s32>(0x00877658);
            const s32 oldx = px;
            px = ShiftDown8((s32)((u32)sine * (u32)px + (u32)cosine * (u32)d));
            d = ShiftDown8((s32)((u32)sine * (u32)d - (u32)oldx * (u32)cosine));
        }
        at<u32>(ride, 0x50) = 0;
        break;
    }
    case -13: case -12: case -8: case -5: {
        at<s32>(ride, 0x74) = x;
        at<s32>(ride, 0x78) = y;
        s32 dx, dy;
        EndOffsets(ride, dx, dy);
        u8 *piece = Buoy_Alloc(dx + x, dy + y);
        if (piece)
            AppendPiece(ride, piece);
        at<u32>(piece, 0) |= 4;
        at<u32>(ride, 0x50) = 0;
        at<s32>(ride, 0x90) = dx / 3 + at<s32>(ride, 0x74);
        at<s32>(ride, 0x94) = dy / 3 + at<s32>(ride, 0x78);
        break;
    }
    case -10: case -9: case -7: case -4: {
        at<s32>(ride, 0x90) = x;
        at<s32>(ride, 0x74) = x;
        at<s32>(ride, 0x94) = y;
        at<s32>(ride, 0x78) = y;
        for (s32 i = 0; i < at<s32>(ride, 0x64); ++i) {
            s32 dx = ((at<s32>(ride, 0x64) - i) - 2) * 0x400;
            const u32 flags = at<u32>(ride, 0xb0);
            s32 across = (i & 1) * -0x800 + 0x400;
            s32 dy = across;
            if (flags & 1) {
                dy = dx;
                dx = across;
            }
            if (flags & 2)
                dx = -dx;
            if (flags & 4)
                dy = -dy;
            u8 *piece = Buoy_Alloc(dx + x, dy + y);
            if (piece)
                AppendPiece(ride, piece);
            at<u32>(piece, 0) |= 2;
        }
        s32 dx, dy;
        EndOffsets(ride, dx, dy);
        u8 *piece = Buoy_Alloc(dx + x, dy + y);
        if (piece)
            AppendPiece(ride, piece);
        at<u32>(piece, 0) |= 4;
        at<u32>(ride, 0x50) = 0;
        break;
    }
    case -2: {
        CollNode_Alloc(0xb400, 0x9600, 0x1e00);
        u8 *node = CollNode_Alloc(0xb400, 0x9600, 0x1400);
        at<u32>(node, 0) |= 0x40;
        CollNode_Alloc(0x9000, 0x7200, 0x2600);
        const s32 starts[2][4] = {{0xb400, 0x9600, 0, -0x1800}, {0x9000, 0x7200, 0, 0x2000}};
        for (int group = 0; group < 2; ++group) {
            s32 px = starts[group][2], py = starts[group][3];
            for (int n = 7; n != 0; --n) {
                if (u8 *piece = Buoy_Alloc(px + starts[group][0], py + starts[group][1]))
                    AppendPiece(ride, piece);
                const s32 cosine = global<s32>(0x00877458), sine = global<s32>(0x00877a58);
                const s32 oldx = px;
                px = ShiftDown8((s32)((u32)sine * (u32)py + (u32)cosine * (u32)px));
                py = ShiftDown8((s32)((u32)cosine * (u32)py - (u32)oldx * (u32)sine));
            }
        }
        at<u32>(ride, 0x50) = 2;
        return NameOfRide(ride);
    }
    default:
        return NameOfRide(ride);
    }
    TrakRide_SetPerformance(NameOfRide(ride), at<u32>(ride, 0x1c));
    return NameOfRide(ride);
}

void __cdecl TrakRide_Close(u32 handle) {
    u8 *ride = RideOf(handle);
    if (handle != NameOf(ride) || at<u32>(ride, 0x50) == 0)
        return;
    at<u32>(ride, 0x50) = 0;
    u8 *node = at<u8 *>(ride, 0xc8);
    if (!node) {
        at<u32>(ride, 0xc8) = at<u32>(ride, 0xc4);
    } else {
        while (u8 *next = at<u8 *>(node, 0x10))
            node = next;
        at<u32>(node, 0x10) = at<u32>(ride, 0xc4);
    }
    at<u32>(ride, 0xc4) = 0;
    const s32 kind = at<s32>(ride, 0);
    if (kind != -14 && kind != -11 && kind != -6 && kind != -3 && kind != -1)
        return;
    u8 *record = global<u8 *>(0x00877b68);
    u32 left = global<u32>(0x00877b6c);
    if (!record || !left)
        return;
    do {
        if (at<u8>(record, 0) & 1) {
            --left;
            if (at<u8 *>(record, 0x9c) == ride) {
                at<u32>(record, 0x88) = 0;
                at<u32>(record, 0) = (at<u32>(record, 0) & 0xffffbfa1u) | 0x10020u;
            }
        }
        record += 0xac;
    } while (left != 0);
}

void __cdecl TrakRide_MarkWornOut(u32 handle) {
    u8 *ride = RideOf(handle);
    if (handle == NameOf(ride))
        at<u32>(ride, 0x54) = 1;
}

void __cdecl TrakRide_SetField4(u32 handle, u32 value) {
    u8 *ride = RideOf(handle);
    if (handle == NameOf(ride))
        at<u32>(ride, 4) = value;
}

void __cdecl TrakRide_PushNode(u32 handle, u32 value) {
    u8 *ride = RideOf(handle);
    if (handle != NameOf(ride))
        return;
    u8 *node = global<u8 *>(0x00877b8c);
    if (!node)
        return;
    global<u32>(0x00877b8c) = at<u32>(node, 0x10);
    at<u32>(node, 0x10) = at<u32>(ride, 0xc8);
    at<u8 *>(ride, 0xc8) = node;
    at<u32>(node, 4) = handle;
    at<u32>(node, 0) = value;
}

u32 __cdecl TrakRide_LinkedValue(u32 handle) {
    const uintptr_t rides = global<uintptr_t>(0x00877b60);
    const uintptr_t ride = loaded() ? rides + (handle & 0xff) * 0xd0 : 0;
    // The handle of that ride: its index (from the pointer, as the original divides it back
    // out) and serial.
    const u32 named = loaded() ? (u32)((s32)(ride - rides) / 0xd0) | at<u32>((const void *)ride, 0) << 8 : 0;
    if (handle != named) return 0;
    const u8 *link = at<const u8 *>((const void *)ride, 0x58);
    if (!link || !(at<u32>(link, 0) & 0x400000)) return 0;
    return at<u32>(link, 8);
}

void __cdecl TrakRide_SetAsleep(u8 *ride, s32 asleep) {
    u8 *const *const instances = reinterpret_cast<u8 *const *>(0x007a4610);
    if (asleep && !(at<u32>(ride, 0) & 0x200)) {
        at<u32>(ride, 0) |= 0x200;
        u8 *inst = instances[at<u32>(ride, 8)];
        if (at<u32>(inst, 4) & 0x4000)
            Obj_RemoveFromRect(at<u8 *>(inst, 8), at<u32>(inst, 0xc0), at<u32>(inst, 0xc4));
        else
            Holder_Sleep(at<u8 *>(inst, 8));
        if ((at<u32>(ride, 0) & 0x2002000) && at<u32>(ride, 0xc)) {
            inst = instances[at<u32>(ride, 0xc)];
            if (at<u32>(inst, 4) & 0x4000)
                Obj_RemoveFromRect(at<u8 *>(inst, 8), at<u32>(inst, 0xc0), at<u32>(inst, 0xc4));
            else
                Holder_Sleep(at<u8 *>(inst, 8));
        }
        for (u8 *node = at<u8 *>(ride, 0x30); node; node = at<u8 *>(node, 0x10))
            if (at<s32>(node, 8) != -1) PartStates_Release(at<u8 *>(instances[at<u32>(ride, 8)], 8), at<u32>(node, 8));
        return;
    }
    if (asleep || !(at<u32>(ride, 0) & 0x200))
        return;
    at<u32>(ride, 0) &= ~0x200u;
    u8 *inst = instances[at<u32>(ride, 8)];
    if (at<u32>(inst, 4) & 0x4000)
        Holder_AddToGrid(at<u8 *>(inst, 8), at<u32>(inst, 0xc0), at<u32>(inst, 0xc4));
    else
        Holder_Wake(at<u8 *>(inst, 8));
    if ((at<u32>(ride, 0) & 0x2002000) && at<u32>(ride, 0xc)) {
        inst = instances[at<u32>(ride, 0xc)];
        if (at<u32>(inst, 4) & 0x4000)
            Holder_AddToGrid(at<u8 *>(inst, 8), at<u32>(inst, 0xc0), at<u32>(inst, 0xc4));
        else
            Holder_Wake(at<u8 *>(inst, 8));
    }
    for (u8 *node = at<u8 *>(ride, 0x30); node; node = at<u8 *>(node, 0x10))
        if (at<s32>(node, 8) != -1) PartStates_Activate(at<u8 *>(instances[at<u32>(ride, 8)], 8), at<u32>(node, 8), 0);
}

s32 __cdecl TrakRide_AddSection(u32 handle, s32 x, s32 y, u32 piece, u32 a, u32 b, u32 c) {
    u8 *const ride = RideOf(handle);
    if (!loaded() || NameOf(ride) != handle)
        return 0;
    for (const u8 *other = at<const u8 *>(ride, 0xbc); other; other = at<const u8 *>(other, 0x24))
        if (at<s32>(other, 0x14) == x && at<s32>(other, 0x18) == y)
            return 0;   // (after "Duplicate track section - IGNORED!" to the empty Debug_Printf)
    u8 *section = global<u8 *>(0x00877b80);
    if (!section)
        return 0;
    u32 i = 0;
    for (; i < 0x400 && (at<u8>(section, 0) & 1); ++i)
        section += 0x28;
    if (i == 0x400)
        return 0;
    at<u32>(section, 0) = 1;
    global<u32>(0x00877b84) += 1;
    at<u32>(section, 8) = a;
    at<u32>(section, 0xc) = b;
    at<u32>(section, 0x10) = c;
    at<u32>(section, 4) = piece;
    at<s32>(section, 0x14) = x;
    at<s32>(section, 0x18) = y;
    at<u8 *>(section, 0x20) = ride;
    at<u32>(section, 0x24) = 0;   // (+0x1c, the node list, is left as it was: sic)
    if (u8 *last = at<u8 *>(ride, 0xbc)) {
        while (at<u8 *>(last, 0x24))
            last = at<u8 *>(last, 0x24);
        at<u8 *>(last, 0x24) = section;
    } else {
        at<u8 *>(ride, 0xbc) = section;
    }
    const u32 X = (u32)x, Y = (u32)y, width = at<u32>(ride, 0xc) - at<u32>(ride, 8);
    const u32 wide = width + 0xc00, narrow = 0xc00 - width;
    u8 *first = nullptr;
    u32 ox = Y + 0x1800, oy = Y + 0x1800;   // the buoy at (x + ox, y + 0x1800 - oy); these for the other kinds (sic)
    const u32 kind = piece & 0xffff;
    switch (kind) {
    case 1:
    case 5:
        first = Flagged(SectionNode(section, X + 0x1800, Y, wide), 0x14, 0);
        Flagged(SectionNode(section, X + 0x1800, Y, narrow), 0x14, 0x40);
        ox = oy = 0xf84;
        break;
    case 2:
    case 6:
        first = Flagged(SectionNode(section, X, Y, wide), 0x12, 0);
        Flagged(SectionNode(section, X, Y, narrow), 0x12, 0x40);
        ox = 0x87c;
        oy = 0xf84;
        break;
    case 3:
    case 7:
        first = Flagged(SectionNode(section, X, Y + 0x1800, wide), 0xa, 0);
        Flagged(SectionNode(section, X, Y + 0x1800, narrow), 0xa, 0x40);
        ox = oy = 0x87c;
        break;
    case 4:
    case 8:
        first = Flagged(SectionNode(section, X + 0x1800, Y + 0x1800, wide), 0xc, 0);
        Flagged(SectionNode(section, X + 0x1800, Y + 0x1800, narrow), 0xc, 0x40);
        ox = 0xf84;
        oy = 0x87c;
        break;
    case 9:
    case 12:
        first = Flagged(SectionNode(section, X + 0xc00, Y + 0xc00, 0xc20), 0, kind == 12 ? 0x120 : 0x20);
        at<u32>(first, 0x10) = width;
        ox = oy = 0xc00;
        break;
    case 10:
    case 13:
        first = Flagged(SectionNode(section, X + 0xc00, Y + 0xc00, width), 0, kind == 13 ? 0x120 : 0x20);
        at<u32>(first, 0x10) = 0xc20;
        ox = oy = 0xc00;
        break;
    case 11: {
        const s32 reach = (s32)(long long)X87_Sqrt(X87Reg((double)(s32)(width * width + 0x900000))).v;
        first = SectionNode(section, X + 0xc00, Y + 0xc00, (u32)reach);
        const s32 type = at<s32>(ride, 0);
        if (type == -5 || type == -8 || type == -12 || type == -13)
            Flagged(first, 0, 0x80);
        Flagged(SectionNode(section, X + 0x1800, Y, narrow), 0x14, 0x40);
        Flagged(SectionNode(section, X, Y, narrow), 0x12, 0x40);
        Flagged(SectionNode(section, X, Y + 0x1800, narrow), 0xa, 0x40);
        Flagged(SectionNode(section, X + 0x1800, Y + 0x1800, narrow), 0xc, 0x40);
        ox = oy = 0xc00;
        break;
    }
    default:
        break;
    }
    if (!(piece & 0x10000)) {
        if (u8 *const buoy = Buoy_Alloc(X + ox, Y + 0x1800 - oy)) {
            at<u8 *>(buoy, 0x10) = ride;
            at<u32>(ride, 0xac) += 1;
            if (u8 *last = at<u8 *>(ride, 0xa8)) {
                while (at<u8 *>(last, 0x14))
                    last = at<u8 *>(last, 0x14);
                at<u8 *>(last, 0x14) = buoy;
            } else {
                at<u8 *>(ride, 0xa8) = buoy;
            }
            at<u8 *>(buoy, 0x18) = first;
            at<u8 *>(first, 0x1c) = buoy;   // (no first node, as for the other kinds: a write at address 0x1c)
        }
    }
    return 1;
}

u16 *__cdecl TrakRide_FindThing(u16 *out, u32 ride) {
    void *const world = global<void *>(0x007cf83c);
    u16 handle;
    World_FirstThing(world, nullptr, &handle);
    while (handle != 0) {
        const u8 *const thing = global<const u8 *>(0x007cfb90 + handle * 20u);
        if (thing[2] == 3 && at<u32>(thing, 0x28) == ride) {
            *out = *Thing_GetHandle(thing, nullptr, &handle);
            return out;
        }
        handle = *World_NextThing(world, nullptr, &handle, thing);
    }
    *out = 0;
    return out;
}

s32 __cdecl TrakRide_Unload(u8 *ride, u32 effect) {
    u8 *const *const instances = reinterpret_cast<u8 *const *>(0x007a4610);
    for (u8 *node; (node = at<u8 *>(ride, 0x30)) != nullptr;) {
        at<u32>(ride, 0x30) = at<u32>(node, 0x10);
        at<u32>(node, 0x10) = at<u32>(at<u8 *>(ride, 0x9c), 0xc8);
        at<u8 *>(at<u8 *>(ride, 0x9c), 0xc8) = node;
        --at<s32>(at<u8 *>(ride, 0x9c), 0x60);
        if (at<s32>(node, 8) == -1) continue;
        PartStates_Release(at<u8 *>(instances[at<u32>(ride, 8)], 8), at<u32>(node, 8));
        at<s32>(node, 8) = -1;
        if (effect) SeatEffect(instances[at<u32>(ride, 8)]);
    }
    return at<s32>(at<u8 *>(ride, 0x9c), 0x60);
}

s32 __cdecl TrakCar_Remove(u8 *car) {
    u8 *const ride = at<u8 *>(car, 0x9c);
    TrakRide_Unload(car, 0);
    if (at<u32>(car, 0) & 0x400000u) {
        at<u32>(ride, 0x58) = 0;
        at<u32>(car, 0) &= ~0x400000u;
    }
    if (at<s32>(ride, 0x50) != 4) SeatEffect(reinterpret_cast<u8 *const *>(0x007a4610)[at<u32>(car, 8)], 0xf);
    QueuePiece_Remove(at<u32>(car, 8));   // (Ride_Destroy 0x463c80, through the queue pieces' env)
    if (at<u32>(car, 0) & 0x2002000u) {
        QueuePiece_Remove(at<u32>(car, 0xc));
        at<u32>(car, 0) &= 0xfdffdfffu;
    }
    const u32 sound = at<u32>(car, 0x20);
    if (sound && at<s32>(ride, 0x50) != 4) {
        const s32 kind = at<s32>(ride, 0);
        if (kind == -10 || kind == -9 || kind == -7 || kind == -4) {
            const u16 found = RideObject(NameOf(at<const u8 *>(car, 0x9c)));
            const s32 id = found ? at<s32>(Thing_ByHandle(found), 0x24) : 0;
            const u32 played = Sound_PlayRideElement(id, sound, global<u32>(0x00803a3c), 1, 0, 0, 0);
            at<u32>(car, 0x20) = played;
            if (played) {
                const s32 x = SoundPlace(car, 0x34), z = SoundPlace(car, 0x38);
                at<u32>(car, 0x20) = Sound_SetPosition(played, x, 0, z);
            }
            at<u32>(car, 0x20) = 0;
        } else {
            Sound_Stop(sound);
        }
    }
    if (at<s32>(car, 0x2c) != -1) {
        ParticleSystem_End(at<u32>(car, 0x2c));
        at<s32>(car, 0x2c) = -1;
    }
    at<u32>(car, 0) &= ~1u;
    --global<s32>(0x00877b6c);
    if (--at<s32>(ride, 0x5c) == 0) {
        const s32 kind = at<s32>(ride, 0);
        if (kind != -13 && kind != -12 && kind != -8 && kind != -5) at<s32>(ride, 0x50) = 1;
    }
    return at<s32>(ride, 0x5c);
}

void __cdecl TrakRide_Load(u8 *ride, u32 effect) {
    u8 *const *const instances = reinterpret_cast<u8 *const *>(0x007a4610);
    u32 seat = 1;
    for (u8 *node = at<u8 *>(ride, 0x30); node; node = at<u8 *>(node, 0x10)) {
        const u32 part = Model_FindPartByKey4(instances[at<u32>(ride, 8)], 0x80, seat);
        at<u32>(node, 0xc) = seat++;
        at<u32>(node, 8) = part;
        if (part != 0xffffffffu) {
            PartStates_Activate(at<u8 *>(instances[at<u32>(ride, 8)], 8), part, at<u32>(node, 0));
            if (effect) SeatEffect(instances[at<u32>(ride, 8)]);
        }
        ++at<s32>(at<u8 *>(ride, 0x9c), 0x60);
    }
}

void __cdecl TrakCar_PlaySounds(u8 *car) {
    const u32 next = at<u32>(car, 0x14);
    if (next != 0) {
        if (next > at<u32>(car, 0x18))
            at<u32>(car, 0x18) = next;
        at<u32>(car, 0x14) = 0;
    }
    const u32 state = at<u32>(car, 0x18);
    if (state - 1 > 4)
        return;
    if (state == 2) {
        const u32 seed = global<u32>(0x00877b90) * 0x343fd + 0x269ec3;
        global<u32>(0x00877b90) = seed;
        if (seed & 0x70000) {
            at<u32>(car, 0x18) = 0;
            return;
        }
    }
    const s32 x = SoundPlace(car, 0x34), z = SoundPlace(car, 0x38);
    if (state == 1 || state == 3 || state == 5) {
        Sound_PlayAt(0, global<u32>(0x00803a28), state == 1 ? 0x18 : 0xc, x, 0, z);
        at<u32>(car, 0x18) = 0;
        return;
    }
    const u32 ride = NameOf(at<const u8 *>(car, 0x9c));
    u16 found = 0;
    s32 id = 0;
    if (state == 2) {
        u16 handle;
        found = *TrakRide_FindThing(&handle, ride);
        if (found)
            id = at<s32>(global<u8 *>(0x007cfb90 + 20u * found), 0x24);
    } else {
        found = RideObject(ride);
        if (found) {
            (void)*reinterpret_cast<const volatile u32 *>(global<uintptr_t>(0x007cf6ec));   // (the world pointer, read for ECX)
            id = at<s32>(Thing_ByHandle(found), 0x24);
        }
    }
    Sound_PlayRideElement(id, 0, global<u32>(0x00803a3c), state == 2 ? 3 : 4, x, 0, z);
    at<u32>(car, 0x18) = 0;
}

namespace {

float FloatOf(u32 bits) {
    float f;
    std::memcpy(&f, &bits, 4);
    return f;
}

u32 BitsOf(float f) {
    u32 bits;
    std::memcpy(&bits, &f, 4);
    return bits;
}

u8 *Instance(u32 index) { return global<u8 *>(0x007a4610 + 4 * index); }

// The game's own rand (0x877b90 = * 0x343fd + 0x269ec3): the new seed.
u32 NextSeed() { return global<u32>(0x00877b90) = global<u32>(0x00877b90) * 0x343fd + 0x269ec3; }

bool FrameDue() { return (global<u8>(0x00877b5c) & 0x1f) == 0; }

// 256 - atan2(a, b) / pi * 256 truncated (__ftol): fild a; fild b; fpatan (unrounded); fmul 1 / pi [0x700eb8]; fmul 256 [0x700ec0];
// fsubr 256, as collobj's RideTurn.
s32 Heading(s32 a, s32 b) {
    const double turned = X87_MulDouble(X87_MulDouble(X87_Atan2((double)a, (double)b), global<double>(0x00700eb8)), global<double>(0x00700ec0));
    return (s32)(long long)X87_Add(global<double>(0x00700ec0), -turned);
}

// (heading - +0x54) & 0x1ff as -0xff..0x100, times speed (32-bit) over 1000 (signed, truncated).
s32 TurnBy(const u8 *car, s32 heading, s32 speed) {
    s32 d = (s32)((u32)(heading - at<s32>(car, 0x54)) & 0x1ff);
    if (d > 0x100) d -= 0x200;
    return (s32)((u32)d * (u32)speed) / 1000;
}

// The car's instance's holder's part state entries (20 bytes: flags, the part): [[[0x7a4610 + 4 * (+8)] + 8] + 0x28] + 4.
const u8 *PartEntry(const u8 *car, u32 index) {
    const u8 *const entries = at<const u8 *>(at<const u8 *>(at<const u8 *>(Instance(at<u32>(car, 8)), 8), 0x28), 4);
    return entries + index * 20u;
}

// The entry's part's position (+0x30 and +0x34 as dwords, +0x38 loaded), or 0, 0 and [0x700ed0] (0.0) with no part.
const u8 *PartPlace(const u8 *entry, u32 &x, u32 &y, X87Reg &z) {
    const u8 *const part = at<const u8 *>(entry, 4);
    if (part) {
        x = at<u32>(part, 0x30);
        y = at<u32>(part, 0x34);
        z = X87Reg(at<float>(part, 0x38));
    } else {
        z = X87Reg(global<float>(0x00700ed0));
        y = x = 0;
    }
    return part;
}

// Particle_SpawnWithVelocity(effect) at 1024 ([0x700ed4]) times the part's place, along its direction (+0x20..+0x28; negated through
// fld / fchs / fstp when the entry has flag 0x10) over its length (fsqrt of x x + y y + z z, stored as a float to the original's argument
// slot), each __ftol'd: z, y, x of the direction, then z, y, x of the place. No part: the direction (0, 0, 1) and the slot kept.
void SpawnAlong(const u8 *car, u32 index, s32 effect, u32 &slot) {
    const u8 *const entry = PartEntry(car, index);
    u32 px, py;
    X87Reg pz(0.0);
    PartPlace(entry, px, py, pz);
    X87Reg dz(0.0), dy(0.0);
    u32 dx;
    if (const u8 *const part = at<const u8 *>(entry, 4)) {
        u32 v[3];
        for (u32 k = 0; k < 3; k++) {
            v[k] = at<u32>(part, 0x20 + 4 * k);
            if (entry[0] & 0x10) v[k] = X87_FloatThrough(v[k]) ^ 0x80000000u;
        }
        const float x = FloatOf(v[0]), y = FloatOf(v[1]), z = FloatOf(v[2]);
        const X87Reg length = X87_Sqrt(X87Reg(x) * X87Reg(x) + X87Reg(y) * y + X87Reg(z) * z);
        const float stored = (float)length;
        slot = BitsOf(stored);
        dx = BitsOf((float)(X87Reg(x) / stored));
        dy = X87Reg(y) / stored;
        dz = X87Reg(z) / stored;
    } else {
        dz = X87Reg(global<float>(0x00700eb0));
        dy = X87Reg(global<float>(0x00700ed0));
        dx = 0;
    }
    const float scale = global<float>(0x00700ed4);
    const s32 vz = Ftol(dz * scale), vy = Ftol(dy * scale), vx = Ftol(X87Reg(FloatOf(dx)) * scale);
    const s32 z = Ftol(pz * scale), y = Ftol(X87Reg(FloatOf(py)) * scale), x = Ftol(X87Reg(FloatOf(px)) * scale);
    Particle_SpawnWithVelocity(effect, x, y, z, vx, vy, vz);
}

// The particle system +0x2c (not -1) put at 1024 times the place of part +0x24 (not -1): ParticleSystem_SetVectorA.
void SystemAtPart(const u8 *car) {
    const u32 system = at<u32>(car, 0x2c);
    if (system == 0xffffffff || at<s32>(car, 0x24) == -1)
        return;
    u32 px, py;
    X87Reg pz(0.0);
    PartPlace(PartEntry(car, at<u32>(car, 0x24)), px, py, pz);
    const float scale = global<float>(0x00700ed4);
    const s32 z = Ftol(pz * scale), y = Ftol(X87Reg(FloatOf(py)) * scale), x = Ftol(X87Reg(FloatOf(px)) * scale);
    ParticleSystem_SetVectorA(system, x, y, z);
}

// The car's sound +0x20 (when not 0) moved to x = __ftol(+0x34 * 0.0033036), y 0, z = __ftol(+0x38 * ...) and given parameter
// Rides_GetLinkedElement(the ride object's id +0x24 (none: 0), 10), value the speed +0x4c / 3; the object found by TrakRide_FindThing
// (its handle written to slot's low word) or, with walk, by the park's things walked (World_NextThing's handle written there).
void SoundAtCar(u8 *car, u32 &slot, bool walk) {
    const u32 sound = at<u32>(car, 0x20);
    if (!sound)
        return;
    const s32 x = Ftol(X87_MulIntDouble(at<s32>(car, 0x34), global<float>(0x00700f20)));
    const s32 z = Ftol(X87_MulIntDouble(at<s32>(car, 0x38), global<float>(0x00700f20)));
    at<u32>(car, 0x20) = Sound_SetPosition(sound, x, 0, z);
    const u32 ride = NameOf(at<const u8 *>(car, 0x9c));
    s32 id = 0;
    if (!walk) {
        if (const u16 found = *TrakRide_FindThing(reinterpret_cast<u16 *>(&slot), ride))
            id = at<s32>(global<u8 *>(0x007cfb90 + 20u * found), 0x24);
    } else {
        void *const world = global<void *>(0x007cf83c);
        u16 handle, found = 0;
        World_FirstThing(world, nullptr, &handle);
        while (handle != 0) {
            const u8 *const thing = Thing_ByHandle(handle);
            if (thing[2] == 3 && at<u32>(thing, 0x28) == ride) {
                u16 own;
                found = *Thing_GetHandle(thing, nullptr, &own);
                break;
            }
            handle = *World_NextThing(world, nullptr, reinterpret_cast<u16 *>(&slot), thing);
        }
        if (found) {
            (void)*reinterpret_cast<const volatile u32 *>(global<uintptr_t>(0x007cf6ec));   // (the world pointer, read for ECX)
            id = at<s32>(Thing_ByHandle(found), 0x24);
        }
    }
    const s32 value = at<s32>(car, 0x4c) / 3;
    const u32 element = Rides_GetLinkedElement(id, 10);
    at<u32>(car, 0x20) = Sound_SetParam(at<u32>(car, 0x20), element, (u32)value);
}

}  // namespace

void __cdecl TrakCar_Move(u8 *car) {
    const u8 *ride = at<const u8 *>(car, 0x9c);
    u32 slot = at<u32>(car, 0);   // (the original's argument slot, reused: later calls' out values land in it and are read back)
    s32 rate = at<s32>(ride, 0x24);
    if (slot & 0x80000)
        rate = (s32)((u32)rate * 7u) / 10;
    else if (at<s32>(ride, 0x54) == 2)
        rate = 1000;
    const s32 px = (s32)((u32)rate * at<u32>(car, 0x3c)), py = (s32)((u32)rate * at<u32>(car, 0x40));
    const s32 vx = (s32)((u32)px + ((u32)(px >> 31) & 0x3ff)) >> 10, vy = (s32)((u32)py + ((u32)(py >> 31) & 0x3ff)) >> 10;
    const s32 squares = (s32)((u32)vx * (u32)vx + (u32)vy * (u32)vy);
    at<s32>(car, 0x3c) = vx;
    at<s32>(car, 0x44) = vx;
    at<s32>(car, 0x40) = vy;
    at<s32>(car, 0x48) = vy;
    const s32 speed = Ftol(X87_Sqrt(X87Reg((double)squares)));
    at<s32>(car, 0x4c) = speed;
    at<u32>(car, 0x38) += (u32)vy;
    at<u32>(car, 0x34) += (u32)vx;
    const s32 kind = at<s32>(at<const u8 *>(car, 0x9c), 0);
    bool steer = true;   // (the default's heading, after the kinds of group C too)
    switch (kind) {
    case -14: case -11: case -6: case -3: case -1:
        ride = at<const u8 *>(car, 0x9c);
        if (at<s32>(ride, 0x50) == 2) {
            const s32 heading = kind == -1 ? at<s32>(car, 0x50) : Heading(vx, (s32)(0u - (u32)vy));
            at<s32>(car, 0x58) = heading;
            at<s32>(car, 0x5c) = TurnBy(car, heading, speed);
        } else {
            at<s32>(car, 0x5c) = 0;
        }
        if (at<s32>(at<const u8 *>(car, 0x9c), 0x50) == 2 && (slot & 0x4000) && FrameDue() && at<s32>(car, 0x28) != -1 &&
            ((NextSeed() >> 16) & 3) == 2 && at<s32>(at<const u8 *>(car, 0x9c), 0) != -1 && at<u32>(car, 0x14) == 0) {
            SpawnAlong(car, at<u32>(car, 0x28), 1, slot);
            at<u32>(car, 0x14) = 4;
        }
        SoundAtCar(car, slot, false);
        SystemAtPart(car);
        steer = false;
        break;
    case -13: case -12: case -8: case -5: {
        slot = at<u32>(car, 0x58);
        const s32 turn = (s32)slot;
        if ((turn < 0 ? (s32)(0u - (u32)turn) : turn) > 0x30)
            at<s32>(car, 0x58) = Ftol(X87Reg((double)turn) * global<float>(0x00700f24) * global<float>(0x00700f28));
        const s32 now = at<s32>(car, 0x58);
        const s32 eighth = (s32)((u32)now + ((u32)(now >> 31) & 0x7f)) >> 7;
        at<s32>(car, 0x5c) = eighth > 0x10 ? 0x10 : eighth < -0x10 ? -0x10 : eighth;
        if (at<s32>(at<const u8 *>(car, 0x9c), 0x50) == 2 && FrameDue() && ((NextSeed() >> 16) & 7) == 2)
            SeatEffect(Instance(at<u32>(car, 8)), 0x60);
        SoundAtCar(car, slot, false);
        steer = false;
        break;
    }
    case -10: case -9: case -7: case -4: {
        ride = at<const u8 *>(car, 0x9c);
        if (at<s32>(ride, 0x50) == 2 && FrameDue() && speed > 0xe6 && at<u32>(car, 0x14) == 0)
            at<u32>(car, 0x14) = 2;
        const u32 state = at<u32>(car, 0x10);
        if (speed > 0x28) {
            if (state == 0xc) {
                at<u32>(car, 0x10) = 5;
                if (at<u32>(car, 8)) Layer_ClearQueue(Instance(at<u32>(car, 8)), 0);
                if (at<u32>(car, 8)) Animator_Play(Instance(at<u32>(car, 8)), 5, 0, 1, 1.0f, 0);
            }
        } else if (state == 5 && !(slot & 0x400)) {
            at<u32>(car, 0x10) = 0xc;
            if (at<u32>(car, 8)) Layer_ClearQueue(Instance(at<u32>(car, 8)), 0);
            if (at<u32>(car, 8)) Animator_Play(Instance(at<u32>(car, 8)), 0xc, 0, 2, 1.0f, 0);
        }
        ride = at<const u8 *>(car, 0x9c);
        if (at<s32>(ride, 0x50) == 2 && at<s32>(ride, 0x54) == 1 && FrameDue() && at<s32>(car, 0x4c) > 0x50 && at<s32>(car, 0x24) != -1) {
            const u32 roll = (NextSeed() >> 16) & 7;
            if (roll <= 1) {
                SpawnAlong(car, at<u32>(car, 0x24), 1, slot);
            } else if (roll == 2 && at<u32>(car, 0x14) == 0) {
                SpawnAlong(car, at<u32>(car, 0x24), 10, slot);
                at<u32>(car, 0x14) = 1;
            }
        }
        SoundAtCar(car, slot, true);
        SystemAtPart(car);
        break;
    }
    default:
        break;
    }
    if (steer) {
        slot = at<u32>(car, 0x3c);
        slot = 0u - at<u32>(car, 0x40);
        const s32 heading = Heading(at<s32>(car, 0x3c), (s32)slot);
        at<s32>(car, 0x58) = heading;
        const s32 previous = at<s32>(car, 0x5c);
        at<s32>(car, 0x60) = previous;
        const s32 turn = TurnBy(car, heading, at<s32>(car, 0x4c));
        at<s32>(car, 0x5c) = turn;
        at<s32>(car, 0x60) = turn - at<s32>(car, 0x60);
    }
    at<u32>(car, 0x54) = (at<u32>(car, 0x54) + at<u32>(car, 0x5c)) & 0x1ff;
    u8 *const link = at<u8 *>(car, 0x98);
    if (!link)
        return;
    if (!(at<u32>(link, 0) & 0x100)) {
        if (at<u32>(car, 0) & 0x200)
            TrakRide_SetAsleep(car, 0);
        return;
    }
    u8 *const gate = at<u8 *>(link, 0x18);
    if (!gate || at<u32>(gate, 0x10) != 0xdac)
        return;
    const s32 apart = at<u32>(gate, 4) == 0xc ? (s32)(at<u32>(car, 0x34) - at<u32>(link, 4)) : (s32)(at<u32>(car, 0x38) - at<u32>(link, 8));
    const u32 flags = at<u32>(car, 0);
    u32 entry;
    if (at<u32>(gate, 8) == 0) {
        if (!(flags & 0x200) && apart < 0x4b0 && apart > -0x4b0) {
            at<u32>(car, 0) = apart < 0 ? flags | 0x800000 : flags & 0xff7fffff;
            TrakRide_SetAsleep(car, 1);
            if (at<u32>(gate, 0xc)) Layer_GetFlags(Instance(at<u32>(gate, 0xc)), &slot, &entry, 0);
            if (slot != 5) {
                if (at<u32>(gate, 0xc)) Layer_ClearQueue(Instance(at<u32>(gate, 0xc)), 0);
                if (at<u32>(gate, 0xc)) Animator_Play(Instance(at<u32>(gate, 0xc)), 5, 0, 0, 1.0f, 0);
                at<u32>(gate, 0) |= 2;
            }
        }
    } else if (flags & 0x200) {
        if ((flags & 0x800000) ? apart > 0x4b0 : apart < -0x4b0)
            TrakRide_SetAsleep(car, 0);
    }
    if (gate[0] & 2) {
        if (at<u32>(gate, 0xc)) Layer_GetQueued(Instance(at<u32>(gate, 0xc)), &slot, &entry, 0);
        if (slot == 0xc) {
            if (at<u32>(gate, 0xc)) Animator_Play(Instance(at<u32>(gate, 0xc)), 2, 0, 1, 1.0f, 0);
            at<u32>(gate, 0) &= ~2u;
        }
    }
}
}

namespace {

// 0x545450's and 0x545610's start (inlined in both): the cars of the pool [0x877b68] (the used ones, as many as [0x877b6c] counts, read
// once) on ride (+0x9c) taken off (TrakCar_Remove), then the ride closed (TrakRide_Close). (Both originals check the handle against the
// ride a second time first, always alike.)
void RemoveCarsAndClose(u32 handle, const u8 *ride) {
    u8 *record = global<u8 *>(0x00877b68);
    if (record) {
        u32 left = global<u32>(0x00877b6c);
        while (left) {
            if (at<u8>(record, 0) & 1) {
                --left;
                if (at<const u8 *>(record, 0x9c) == ride) TrakCar_Remove(record);
            }
            record += 0xac;
        }
    }
    TrakRide_Close(handle);
}

}  // namespace

extern "C" {

void __cdecl TrakRide_Strip(u32 handle) {
    u8 *const ride = RideOf(handle);
    if (handle != NameOf(ride)) return;
    RemoveCarsAndClose(handle, ride);
    Collider_KeepA8(ride, 6);   // (inlined)
    Pool_FreeAll(ride);         // (inlined)
}

void __cdecl TrakRide_Destroy(u32 handle) {
    u8 *const ride = RideOf(handle);
    if (handle != NameOf(ride)) return;
    RemoveCarsAndClose(handle, ride);
    Collider_KeepA8(ride, 0);   // (inlined: every piece dropped)
    Pool_FreeAll(ride);         // (inlined)
    Collider_ReleaseC0(ride);   // (inlined)
    at<u32>(ride, 0) = 0;
    --global<s32>(0x00877b64);
}

void __cdecl TrakRides_DestroyAll() {
    if (global<u32>(0x00877b58) != 0x4a454647)
        return;
    u8 *ride = global<u8 *>(0x00877b60);
    for (u32 left = global<u32>(0x00877b64); left; ride += 0xd0) {   // (the used count read once: the walk ends with it)
        if (!at<u32>(ride, 0))
            continue;
        at<u32>(ride, 0x50) = 4;
        --left;
        TrakRide_Destroy(NameOf(ride));   // (inlined in the original)
    }
    // the free records (0x400 of 0x14 bytes from [0x877b88]) chained again by +0x10 from the first, the block re-read each step
    u8 *next = nullptr;
    for (s32 offset = 0x4fec; offset >= 0; offset -= 0x14) {
        at<u8 *>(global<u8 *>(0x00877b88) + offset, 0x10) = next;
        next = global<u8 *>(0x00877b88) + offset;
    }
    global<u8 *>(0x00877b8c) = global<u8 *>(0x00877b88);
}
}
