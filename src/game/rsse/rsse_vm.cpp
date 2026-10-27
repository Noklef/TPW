#include "rsse.h"
#include <cstring>

#include "../coaster/ridebounds.h"
#include "../core/gameclock.h"
#include "../math/sintable.h"
#include "../math/x87.h"
#include "../model/heightat.h"
#include "../model/partstate.h"
#include "../model/parts.h"
#include "../model/scratch.h"
#include "../render/particles.h"
#include "../world/personscripts.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

s32 Ftol(X87Reg x) { return (s32)(long long)x.v; }

// A walk's time: 100 times the distance from p1 to p2 (its x87 sums, fsqrt, __ftol), 100 for none.
u32 WalkTime(const float *p1, const float *p2) {
    const X87Reg dx = X87Reg(p1[0]) - p2[0];
    const X87Reg dy = X87Reg(p1[1]) - p2[1];
    const X87Reg dz = X87Reg(p1[2]) - p2[2];
    const X87Reg squares = dx * (float)dx + dy * dy + dz * (float)dz;
    const u32 time = (u32)Ftol(X87_Sqrt(squares)) * 100;
    return time == 0 ? 100 : time;
}

// The octant (C's % 8, signed) of __ftol(0.5 - (6.28318 - (angle - 1.570795)) * -1.27324...) (the doubles 0x700fe8..0x701000).
s32 Octant(double angle) {
    const X87Reg turn = X87_SubR(X87_SubR(X87Reg(angle) - global<double>(0x00700fe8), global<double>(0x00700ff0)) * global<double>(0x00700ff8),
                                 global<double>(0x00701000));
    return Ftol(turn) % 8;
}

// Particle_Spawn(0x13) at 1024 times the place (each __ftol'd: z, y, then x).
void Puff(const float *place) {
    const s32 z = (s32)((u32)Ftol(X87Reg(place[2])) << 10);
    const s32 y = (s32)((u32)Ftol(X87Reg(place[1])) << 10);
    const s32 x = (s32)((u32)Ftol(X87Reg(place[0])) << 10);
    Particle_Spawn(0x13, x, y, z);
}

u8 *Ride(const u8 *vm) { return global<u8 *>(0x007a4610 + 4 * at<u32>(vm, 0xc8)); }

// A leg's progress in thousandths: (now * 1000 - start * 1000) / (end - start), unsigned (the start +8 read before the clock).
u32 Progress(u8 *slot) {
    const u32 start = at<u32>(slot, 8);
    const u32 now = Clocks_Now(reinterpret_cast<u8 *>(0x00785970));
    return (now * 1000 - start * 1000) / (at<u32>(slot, 0xc) - start);
}

}  // namespace

extern "C" {

u32 __cdecl RSSE_FetchWord(RsseContext *ctx) {
    s32 pc = ctx->pc;
    if (pc >= 0 && pc < static_cast<s32>(ctx->code_len)) {
        ctx->pc = pc + 1;
        return ctx->code[pc];
    }
    // Original: Debug_Printf("RSSE: Tried to read outside script") (compiled out in retail).
    ctx->pc = RSSE_PC_DEAD;
    return 0;
}

s32 __cdecl RSSE_OperandValue(RsseContext *ctx, u32 word) {
    // The original tests the top *byte* (not nibble) and strips the tag with xor.
    // Variable indices are never range-checked.
    if ((word & 0xFF000000u) == RSSE_TAG_VAR)
        return ctx->vars[word ^ RSSE_TAG_VAR];
    return static_cast<s16>(word);
}

void __cdecl RSSE_Init() {
    if (global<u32>(0x008791a0))
        return;   // Original: Debug_Printf("RSSE: Tried to Initialise twice") (compiled out in retail).
    global<u32>(0x008791a0) = 1;
    global<u32>(0x008791a8) = 1;
    global<u32>(0x008791b0) = 0;
    global<u32>(0x008791ac) = 0;
    global<u32>(0x008791a4) = 0;
    global<u32>(0x00879190) = 0;
    global<u32>(0x0087919c) = 0;
    // The checksum runs over the names in the opcode table one pointer at a time (the table holds a mnemonic and an
    // operand string per opcode: every pointer is read, sic) and stops at the first pointer to an empty string.
    const u32 *entry = reinterpret_cast<const u32 *>(0x00765280);
    if (!*reinterpret_cast<const char *>(*entry))
        return;
    do {
        const char *const name = reinterpret_cast<const char *>(*entry);
        for (u32 i = 0; i < strlen(name); i++)
            global<u32>(0x00879190) += static_cast<s8>(name[i]);
        entry++;
    } while (*reinterpret_cast<const char *>(*entry));
}

const char *__cdecl RsseScript_EventName(const u8 *script, u32 index, u32 *group_out, u32 *entry_out, u32 *count_out) {
    static const u32 names[12] = {0x74d19c, 0x74d194, 0x74d18c, 0x74d184, 0x74d17c, 0x74d174, 0x74d170, 0x74d168, 0x74d160, 0x74d158, 0x74d150, 0x74d148};
    for (u32 group = 0; group < 12; group++) {
        const u8 *const record = script + 0x1c + group * 12;
        for (u32 entry = 0; entry < at<u32>(record, 0); entry++) {
            if (at<u32>(at<const u8 *>(script, 0xac), index * 4) != at<u32>(at<const u8 *>(record, 4), entry * 8 + 4))
                continue;
            if (group_out)
                *group_out = group;
            if (entry_out)
                *entry_out = entry;
            if (count_out)
                *count_out = at<u32>(record, 0);
            return reinterpret_cast<const char *>(names[group]);
        }
    }
    return reinterpret_cast<const char *>(0x74d6d4);
}

u32 __cdecl RSSE_StartTimer(RsseContext *ctx, u32 ms, u32 a, u32 b) {
    at<u32>(ctx, 0x80) = Clocks_Now(reinterpret_cast<u8 *>(0x00785970));
    at<u32>(ctx, 0x84) = ms;
    at<u16>(ctx, 0x88) = (u16)a;
    at<u16>(ctx, 0x8a) = (u16)b;
    return (ms & 0xffff0000u) | (u16)b;  // sic: EAX as the original leaves it (the opcode handler returns it)
}

u32 __cdecl Rsse_GetVector(const u8 *frame, u32 *x, u32 *y, u32 *z) {
    const u8 *record = at<const u8 *>(at<const u8 *>(at<const u8 *>(frame, 8), 4), 0x78) + 0x10;
    *x = at<u32>(record, 0x30);
    *y = at<u32>(record, 0x34);
    *z = at<u32>(record, 0x38);
    return *z;
}

u32 __cdecl Rsse_TakeFinished(u8 *vm) {
    const s32 count = at<s32>(vm, 0x7c);
    u8 *entries = at<u8 *>(vm, 0x2c);
    for (s32 i = 0; i < count; i++) {
        u8 *entry = entries + 0x20 * (u32)i;
        if (at<u16>(entry, 0x18) == 4) {
            const u32 value = at<u32>(entry, 0x10);
            at<u16>(entry, 0x18) = 0;
            at<u32>(at<u8 *>(vm, 0x2c), 0x20 * (u32)i + 0x10) = 0;
            return value;
        }
    }
    return 0;
}

s32 __cdecl Rsse_StartWalker(u8 *vm, u32 value, s32 from, s32 to, u32 a, u32 b, u32 kind, u32 last) {
    float p1[3], p2[3];
    Ride_PartPlace(vm, p1, nullptr, from, 0x800);
    Ride_PartPlace(vm, p2, nullptr, to, kind == 4 ? 0x80 : 0x800);
    const u32 time = WalkTime(p1, p2);
    const s32 count = at<s32>(vm, 0x7c);
    s32 i = 0;
    u8 *slot = at<u8 *>(vm, 0x2c);
    for (; i < count; ++i, slot += 0x20)
        if (at<u16>(slot, 0x18) == 0)
            break;
    if (i >= count)
        return i;
    at<u16>(slot, 0) = (u16)from;
    at<u16>(slot, 2) = (u16)to;
    at<u16>(slot, 4) = (u16)a;
    at<u16>(slot, 6) = (u16)b;
    at<u32>(slot, 0x10) = value;
    at<u16>(slot, 0x16) = (u16)kind;
    u8 *const clock = reinterpret_cast<u8 *>(0x00785970);
    at<u32>(slot, 8) = Clocks_Now(clock);
    at<u32>(slot, 0xc) = Clocks_Now(clock) + time;
    const s32 octant = Octant(X87_Atan2((X87Reg(p2[2]) - p1[2]).v, (X87Reg(p2[0]) - p1[0]).v));
    at<u16>(slot, 0x18) = 1;
    at<u16>(slot, 0x1a) = (u16)last;
    at<u16>(slot, 0x14) = (u16)octant;
    return octant;
}

void __cdecl Rsse_MoveWalker(u8 *vm, u8 *slot, s32 from, s32 to, s32 mode, s32 t) {
    float p1[3], p2[3];
    Ride_PartPlace(vm, p1, nullptr, from, mode == 1 ? 0x80 : 0x800);
    Ride_PartPlace(vm, p2, nullptr, to, mode == 2 ? 0x80 : 0x800);
    if (t > 1000)
        t = 1000;
    const X87Reg T = (double)t;
    const float &k1000 = global<float>(0x00701020), &k0001 = global<float>(0x00700fd4);
    // fld to; fsub from (m32); fmul st1; fld from; fmul -1000; fsubp; fmul 0.001
    const auto lerp = [&](int i) { return (float)(((X87Reg(p2[i]) - p1[i]) * T - X87Reg(p1[i]) * k1000) * k0001); };
    float x = lerp(0);
    float z = lerp(2);
    float y;
    std::memcpy(&y, &from, 4);   // (the stack argument the original stores y into)
    if (at<u8>(slot, 0x1a) & 1)
        y = lerp(1);
    else
        View_FloorAt(x, z, &y);
    u8 *const person = global<u8 *>(0x007cfb90 + 20 * (u32)at<u16>(slot, 0x10));
    const auto place = [&](s32 octant, u32 anim) {
        u32 xb, zb;
        std::memcpy(&xb, &x, 4);
        std::memcpy(&zb, &z, 4);
        Person_PlaceScript(person, nullptr, xb, y, zb, (u32)octant, anim);
    };
    if (at<u16>(slot, 0x18) != 2) {
        place(at<s16>(slot, 0x14), 1);
        return;
    }
    if (at<u32>(vm, 0x84) == 0) {
        if (at<u16>(slot, 0x16) == 3)
            place(at<s16>(slot, 0x14), 3);
        if (at<u16>(slot, 0x16) == 5)
            place(at<s16>(slot, 0x14), 8);
        if (at<u16>(slot, 0x16) == 6)
            place(at<s16>(slot, 0x14), 5);
        const u16 kind = at<u16>(slot, 0x16);
        if (kind == 4 || kind == 1 || kind == 2)
            Person_ReleaseScripts(person);
        return;
    }
    const u32 h = at<u32>(slot, 0x10);
    const float &k04 = global<float>(0x00701028), &k3 = global<float>(0x00701044), &k0033 = global<float>(0x00701040);
    const float a = (float)(k04 - X87Reg((double)(s32)(h & 7)) * global<float>(0x00701024));
    const float b = (float)(k04 - X87Reg((double)(s32)(h & 3)) * global<float>(0x0070102c));
    const float c = (float)(global<float>(0x00700fd0) - X87Reg((double)at<s16>(vm, 0xc0)) * global<float>(0x00700fcc));
    const float d = (float)(X87Reg((double)at<u16>(vm, 0x88)) * k0001);
    const float e = (float)(X87Reg((double)at<u16>(vm, 0x8a)) * k0001);
    u8 *const clock = reinterpret_cast<u8 *>(0x00785970);
    const float now = (float)(double)Clocks_Now(clock);   // (fild qword: the time unsigned)
    const s32 since = (s32)(Clocks_Now(clock) - at<u32>(vm, 0x80));
    const SinTable sines = SinTable_Game();
    const float &scale = global<float>(0x007b095c);
    const X87Reg angle = X87Reg(X87_MulIntDouble(since, global<float>(0x00701030))) / X87Reg((double)at<s32>(vm, 0x84)) * scale;
    const float s = SinTable_At(sines, (float)angle);
    const X87Reg g = X87Reg(a) * now * c;   // (fst: kept in the register for y, read back as the float for x)
    const float gf = (float)g;
    const float sy = SinTable_At(sines, (float)(g * global<float>(0x00701034) * scale));
    y = (float)((global<float>(0x0070103c) - X87Reg(sy) * global<float>(0x00701038)) * s * e + y);
    const float sx = SinTable_At(sines, (float)(X87Reg(gf) * k0033 * scale));
    x = (float)(X87Reg(sx) * b * s * k3 * d + x);
    const float sz = SinTable_At(sines, (float)((X87Reg(b) * now * c * k0033 - global<float>(0x00701048)) * scale));
    z = (float)(X87Reg(sz) * a * s * k3 * d + z);
    const u32 q = Clocks_Now(clock) / (u32)Ftol(X87Reg(b) * global<float>(0x0070104c));
    const s16 octant = (s16)(((s32)(s16)q + at<s16>(slot, 0x14)) % 8);
    at<s16>(slot, 0x14) = octant;
    place(octant, 2);
}

void __cdecl Rsse_WalkerBack(u8 *vm, u32 value) {
    const s32 count = at<s32>(vm, 0x7c);
    u8 *slot = at<u8 *>(vm, 0x2c);
    s32 i = 0;
    for (; i < count; ++i, slot += 0x20)
        if (at<u16>(slot, 0x18) != 0 && at<u32>(slot, 0x10) == value)
            break;
    if (i >= count)
        return;
    float p1[3], p2[3];
    Ride_PartPlace(vm, p1, nullptr, at<s16>(slot, 4), at<u16>(slot, 0x16) == 4 ? 0x80 : 0x800);
    Ride_PartPlace(vm, p2, nullptr, at<s16>(slot, 6), 0x800);
    const u32 time = WalkTime(p1, p2);
    at<u16>(slot, 0x14) = (u16)Octant(X87_Atan2((X87Reg(p2[2]) - p1[2]).v, (X87Reg(p2[0]) - p1[0]).v));
    u8 *const clock = reinterpret_cast<u8 *>(0x00785970);
    at<u32>(slot, 0xc) = Clocks_Now(clock) + time;
    at<u32>(slot, 8) = Clocks_Now(clock);
    if (at<u16>(slot, 0x16) == 2) {
        float place[3];
        Ride_PartPlace(vm, place, nullptr, at<s16>(slot, 4), 0x800);
        Puff(place);
    }
    if (at<u16>(slot, 0x16) == 4 && at<u16>(slot, 0x18) == 2) {
        const u32 part = Model_FindPartByKey4(Ride(vm), 0x80, (u32)(s32)at<s16>(slot, 2));
        if (part != 0xffffffffu)
            PartStates_Release(at<u8 *>(Ride(vm), 8), part);
    }
    at<u16>(slot, 0x18) = 3;
}

void __cdecl Rsse_UpdateWalkers(u8 *vm) {
    if (at<s32>(vm, 0x7c) == 0)
        return;
    Model_BeginView(at<u8 *>(Ride(vm), 8));
    u8 *const clock = reinterpret_cast<u8 *>(0x00785970);
    for (s32 i = 0; i < at<s32>(vm, 0x7c); ++i) {
        u8 *const slot = at<u8 *>(vm, 0x2c) + 0x20 * i;
        if (at<u16>(slot, 0x18) == 0)
            continue;
        switch (at<s16>(slot, 0x18)) {
        case 1: {   // walking out
            const u32 t = Progress(slot);
            Rsse_MoveWalker(vm, slot, at<s16>(slot, 0), at<s16>(slot, 2), at<u16>(slot, 0x16) == 4 ? 2 : 0, (s32)t);
            if (t < 1000)
                break;
            at<u16>(slot, 0x18) = 2;
            at<u32>(slot, 8) = Clocks_Now(clock);
            if (at<u16>(slot, 0x16) == 4) {
                const u32 part = Model_FindPartByKey4(Ride(vm), 0x80, (u32)(s32)at<s16>(slot, 2));
                if (part != 0xffffffffu)
                    PartStates_Activate(at<u8 *>(Ride(vm), 8), part, at<u32>(slot, 0x10));
            }
            if (at<u16>(slot, 0x16) == 2) {
                float place[3];
                Ride_PartPlace(vm, place, nullptr, at<s16>(slot, 2), 0x800);
                Puff(place);
            }
            break;
        }
        case 2: {   // there
            const u16 kind = at<u16>(slot, 0x16);
            if (kind == 4) {
                Rsse_MoveWalker(vm, slot, at<s16>(slot, 0), at<s16>(slot, 2), 2, 1);
            } else if (kind == 1 || kind == 2) {
                Rsse_MoveWalker(vm, slot, at<s16>(slot, 0), at<s16>(slot, 2), 0, 1);
            } else {
                float buffer[5];   // (the original's direction and place share a float: the place from buffer + 2, sic)
                Ride_PartPlace(vm, buffer + 2, buffer, at<s16>(slot, 2), 0x800);
                at<u16>(slot, 0x14) = (u16)Octant(X87_Atan2(buffer[2], buffer[0]));
                Rsse_MoveWalker(vm, slot, at<s16>(slot, 2), at<s16>(slot, 2), 0, 0);
            }
            break;
        }
        case 3: {   // walking back
            const u32 t = Progress(slot);
            Rsse_MoveWalker(vm, slot, at<s16>(slot, 4), at<s16>(slot, 6), at<u16>(slot, 0x16) == 4 ? 1 : 0, (s32)t);
            if (t >= 1000)
                at<u16>(slot, 0x18) = 4;
            break;
        }
        case 4:   // back
            Rsse_MoveWalker(vm, slot, at<s16>(slot, 6), at<s16>(slot, 6), 0, 0);
            break;
        }
    }
    if (at<u32>(vm, 0x84) != 0) {
        const u32 now = Clocks_Now(clock);
        if (at<u32>(vm, 0x80) + at<u32>(vm, 0x84) < now)
            at<u32>(vm, 0x84) = 0;
    }
}
}  // extern "C"
