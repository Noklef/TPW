// A park's coasters read: see coastersload.h.
#include "coastersload.h"

#include <cstring>

#include "../files/file.h"
#include "../math/x87.h"
#include "../world/ridelist.h"
#include "registry.h"
#include "../world/rides.h"

namespace {

AddrCalls g_calls;

template <typename T>
T &G(u32 address) {
    return *reinterpret_cast<T *>((uintptr_t)address);
}
u32 &D(u32 base, u32 offset) { return G<u32>(base + offset); }

u32 Ride(u32 index) { return (u32)(uintptr_t)gRides_7a4610[index]; }

void Read(void *stream, u32 buffer, u32 count) {
    s32 result;
    Stream_Read(&result, static_cast<Stream *>(stream), reinterpret_cast<void *>((uintptr_t)buffer), (s32)count, nullptr);
}
u32 Ptr(const void *p) { return (u32)reinterpret_cast<uintptr_t>(p); }

template <typename T>
T From(const u8 *bytes, u32 offset) {
    T value;
    std::memcpy(&value, bytes + offset, sizeof(T));
    return value;
}

void ReadWords(void *stream, u32 piece) {   // a piece's words, as dwords
    for (u32 i = 0; i < D(piece, 0xb0); ++i) {
        u16 word;
        Read(stream, Ptr(&word), 2);
        D(piece, 0xb4 + 4 * i) = word;
    }
}

// Each other piece. The first's and these with the flags 0x40, 0x80 at first and second.
void ReadPiece(void *stream, u32 &piece, u32 &first, u32 &second) {
    u8 p[0x22];
    Read(stream, Ptr(p), 0x22);
    const u32 flags = From<u32>(p, 0);
    if (p[0] & 8) g_calls.Cd(0x00442cd0);   // CoasterLink_BeginEnd
    u32 placed;
    g_calls.Cd(0x00435020, (u32)From<u16>(p, 8), (u32)From<u16>(p, 0xa), 1u, &placed, &piece, From<u32>(p, 0xc), From<u32>(p, 4));   // CoasterEdit_PlaceCar
    if (!placed) return;
    g_calls.Cd(0x00444150, From<u32>(p, 0xd), D(piece, 0), 0u, 1u);    // Car_SetLengthAt
    g_calls.Cd(0x004441d0, From<u32>(p, 0x19), D(piece, 0), 0u, 1u);   // Car_SetE0At
    if (p[0] & 0x10)
        g_calls.Cd(0x00444270, From<u32>(p, 0x1d), 0u, D(piece, 0), 0u, 1u);   // Car_SetAnglesAt
    else
        g_calls.Cd(0x00444270, From<u32>(p, 0x11), From<u32>(p, 0x15), D(piece, 0), 0u, 1u);
    u32 &bits = D(piece, 8);
    if (p[0] & 1) bits |= 1;
    if (p[0] & 2) bits |= 2;
    if (p[0] & 4) bits |= 4;
    if (flags & 0x100) bits |= 0x40000;
    if (flags & 0x200) bits |= 0x80000;
    if (p[0] & 0x40) {
        bits |= 0x10000;
        first = piece;
    }
    if (p[0] & 0x80) {
        bits |= 0x20000;
        second = piece;
    }
    if (flags & 0x400)
        bits |= 0x200000;
    else
        bits &= 0xffdfffff;
    if (flags & 0x800) bits |= 0x400000;
    D(piece, 0xb0) = p[0x21];
    ReadWords(stream, piece);
}

void PlaceTrains(u32 coaster) {
    for (u32 train = D(coaster, 0x4c); train; train = D(train, 0x28)) {
        if (G<u8>(train + 4) & 1) continue;
        if (!G<u32>(0x00790afc)) {
            for (u32 i = 0; i < D(train, 0x30); ++i) {
                const u32 count = D(train, 0x30);
                const u32 car = D(train, 0x34 + 4 * i);
                u32 bits = D(car, 8);
                if (count > 1 && i < count - 1)
                    bits |= 0x100;
                else
                    bits &= 0xfffffeff;
                D(car, 8) = bits;
                const u32 model = D(car, 0x100);
                if (!(bits & 0x2000) && (bits & 0x100) && (G<u8>(D(model, 0x18)) & 8)) {
                    D(car, 0x90) = car + 0x84;
                    D(car, 0x74) = D(model, 0x18) + 0x44;
                } else {
                    D(car, 0x90) = car + 0x78;
                    D(car, 0x74) = D(model, 0x18) + 0x30;
                }
            }
        }
        D(coaster, 0) |= 0x1001;
        g_calls.Cd(0x00432df0, D(train, 0x18), train, 0u, 1u);   // Train_PlaceCars
    }
}

void AddEntries() {   // flag 1
    const u32 coaster = reinterpret_cast<u32 &>(gCoastersload_790fec);
    D(coaster, 0x3c) |= 1;
    const u32 type = D(coaster, 4);
    const u32 entries = (G<u8>(type) & 0x20) ? 1 : 2;
    u32 position[3];
    u32 part;
    for (u32 i = 0; i < entries; ++i) {
        if (G<u8>(type) & 0x20) {
            const u32 car = D(coaster, 0x54);
            part = D(D(D(car, 0x100), 0x18), 0x2c);
            if (part != 0xffffffff) {
                const u32 holder = D(D(D(Ride(D(car, 0x70)), 8), 0x28), 4);
                const u32 object = D(holder, part * 20 + 4);
                if (object) {
                    g_calls.Cd(0x00435710, object, position);   // Object_GetPosition
                } else {
                    position[2] = 0;
                    position[1] = 0;
                    position[0] = 0;
                }
            }
        } else {
            part = g_calls.Cd(0x0044b220, Ride(D(coaster, 0x10)), 0x400000u, i + 1);   // Model_FindPartByKey4
            if (part != 0xffffffff) g_calls.Cd(0x004386d0, D(coaster, 0x10), &part, position);   // Ride_EntryPosition
        }
        const u32 link = g_calls.Cd(0x00432c10, 0u, 0u, 0x13u, 0u, 0u, position);   // CoasterLink_AddEntry
        const u32 how = (i == 0 && (G<u8>(type) & 0x20)) ? 0x144 : 4;
        g_calls.Cd(0x00434060, link, D(link, 0x1c), D(link, 0x24), D(link, 0x20), how, 0u, 0u, 0u, 0u);   // CoasterLink_AddCar
    }
    D(reinterpret_cast<u32 &>(gCoastersload_790fec), 0) |= 0x1001;
    if (!G<u32>(0x00785a2c)) g_calls.Cd(0x00442880, 0x10u);   // CoasterEdit_SetTools
    g_calls.Cd(0x00435730, reinterpret_cast<u32 &>(gCoastersload_790fec), 2u);   // Coaster_SetState
}

void ReadSeat(void *stream, u32 seat) {
    u8 q[0xb];
    Read(stream, Ptr(q), 0xb);
    D(seat, 0) = From<u32>(q, 0);
    D(seat, 0x30) = q[4];
    D(seat, 0x34) = q[5];
    D(seat, 0x44) = q[6];
    D(seat, 0xc) = From<u32>(q, 7);
    const u32 kind = D(seat, 4);
    if (D(kind, 0xc)) {
        g_calls.Cd(0x00463ab0, D(seat, 0xc), D(kind, 0xc), 0x101u, 0u, 0u, 0u, 0u, 0u, 0u);   // Ride_Claim
        if (!(q[0] & 1)) {
            const u32 entry = Ride(D(seat, 0xc));
            if (G<u8>(entry + 5) & 0x40)
                g_calls.Cd(0x0045be80, D(entry, 8), D(entry, 0xc0), D(entry, 0xc4));   // Obj_RemoveFromRect
            else
                g_calls.Cd(0x0045bdd0, D(entry, 8));   // Holder_Sleep
        }
        const u32 entry = Ride(D(seat, 0xc));
        if (entry) g_calls.Cd(0x00472f40, entry);   // Animator_SetPlaying
    }
    if (D(seat, 0x28) && D(seat, 0x2c)) {
        Read(stream, D(seat, 0x28), D(seat, 0x30) << 2);
        Read(stream, D(seat, 0x2c), D(seat, 0x34) << 2);
    }
}

void ReadRecords(void *stream, u32 coaster) {
    u32 record = D(coaster, 0x128);
    for (u32 i = 0; i < D(D(coaster, 4), 0x2e8); ++i, record += 0x80) {
        u8 r[0x1b];
        Read(stream, Ptr(r), 0x1b);
        u32 seat = D(record, 0x28);
        D(record, 0) = From<u32>(r, 0);
        D(record, 0xc) = r[4];
        D(record, 0x74) = From<u32>(r, 5);
        D(record, 0x38) = From<u32>(r, 0xd);
        D(record, 0x64) = From<u32>(r, 0x11);
        D(record, 0x68) = From<u32>(r, 0x15);
        D(record, 0x10) = From<u16>(r, 0x19);
        D(record, 0x78) = From<u32>(r, 9);
        D(record, 0) |= 4;
        for (u32 j = 0; j < D(D(record, 4), 4); ++j, seat += 0x60) ReadSeat(stream, seat);
    }
}

void ReadCoaster(void *stream) {
    u8 h[0x20];
    G<u32>(0x00790afc) = 1;
    Read(stream, Ptr(h), 0x20);
    const u32 flags = From<u32>(h, 0);
    const u32 id = From<u16>(h, 0xa);
    u32 coaster = reinterpret_cast<u32 &>(gRidelist_790fe0);
    while (coaster && D(coaster, 0x10) != id) coaster = D(coaster, 0x18);
    if (coaster) {
        reinterpret_cast<u32 &>(gCoastersload_790fec) = coaster;
        reinterpret_cast<u32 &>(gCoastersload_790fe4) = coaster + 0x3c;
        g_calls.Cd(0x00436d10, D(coaster, 4), coaster, 1u);   // CoasterLink_SetUpCars
        const X87Reg rate(G<const float>(D(D(D(Ride(id), 8), 4), 0x78) + 0x44));
        g_calls.Cd(0x00436ac0, coaster, (u32)From<u16>(h, 0xe), (u32)From<u16>(h, 0x10), (float)rate, 1u);   // CoasterLink_AddFixedPieces
        D(coaster, 0x14) = g_calls.Cd(0x00468760, (u32)&gCoasterIdUsed_790b04, (u32)&gCoasterIdHoles_790b00, (u32)&gCoasterIdNext_791ce0, 0xffu, (u32)&gCoasters_790bd0, coaster, (u32)From<u16>(h, 0xc));   // IdTable_Add
        D(coaster, 0) |= 2;
    }
    D(coaster, 0) |= 0x4000;
    u32 first = 0, second = 0;
    if (h[0] & 2) {
        const u32 car = D(coaster, 0x54);
        first = D(D(car, 0x64), 0x64);
        second = D(coaster, 0x58);
    }
    u32 piece = D(D(D(coaster, 0x54), 0x64), 0x64);
    Read(stream, piece + 0xb0, 4);
    ReadWords(stream, piece);
    for (u32 k = 0; k < From<u16>(h, 0x16); ++k) ReadPiece(stream, piece, first, second);
    G<u32>(0x00790afc) = 0;
    PlaceTrains(coaster);
    if (h[0] & 1) AddEntries();
    for (u32 car = D(coaster, 0x54); car; car = D(car, 0x64))
        for (u32 i = 0; i < D(car, 0xb0); ++i) D(car, 0xb4 + 4 * i) = g_calls.Cd(0x0043e520, coaster + 0x3c, D(car, 0xb4 + 4 * i));   // Track_PieceAt
    if (h[0] & 2) {
        D(coaster, 0x60) = first;
        D(coaster, 0x3c) |= 2;
        D(coaster, 0x64) = second;
        if (D(first, 0) == 2) D(first, 8) |= 0x50000;
    }
    if (h[0] & 4) D(coaster, 0) |= 0x20;
    if (h[0] & 8) D(coaster, 0) |= 0x40;
    if (h[0] & 0x10) D(coaster, 0) |= 0x80;
    if (h[0] & 0x20) D(coaster, 0) |= 0x100;
    if (h[0] & 0x40) D(coaster, 0) |= 0x200;
    if (h[0] & 0x80) D(coaster, 0) |= 0x400;
    if (flags & 0x100) D(coaster, 0) |= 0x800;
    D(coaster, 8) = From<u32>(h, 4);
    D(coaster, 0x2c) = From<u32>(h, 0x12);
    D(coaster, 0xf4) = From<u16>(h, 0x18);
    D(coaster, 0xf0) = From<u16>(h, 0x1a);
    D(coaster, 0x140) = From<u32>(h, 0x1c);
    ReadRecords(stream, coaster);
    g_calls.Cd(0x0043ba90, coaster, 0u, D(D(coaster, 4), 0x2f4), 0u);   // Coaster_SetupCars
    g_calls.Cd(0x004426a0);   // CoasterEdit_EndAll
    const u32 state = D(coaster, 0x3c);
    if ((state & 2) || !(state & 1)) g_calls.Cd(0x00441840, coaster, 1u);   // Coaster_FlashCar
}

}  // namespace

extern "C" {

void CoastersLoad_SetEnv(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

u32 __cdecl Coasters_Load(void *stream) {
    u32 count;
    Read(stream, Ptr(&count), 4);
    Read(stream, (u32)&gCoasterIdUsed_790b04, 4);
    Read(stream, (u32)&gCoasterIdHoles_790b00, 4);
    Read(stream, (u32)&gCoasterIdNext_791ce0, 4);
    for (u32 i = 0; i < count; ++i) {
        ReadCoaster(stream);
        reinterpret_cast<u32 &>(gCoastersload_790fec) = 0;
    }
    return 1;
}
}
