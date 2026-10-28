#include "analyser.h"

#include <cstdint>
#include <cstring>

#include "../core/filetime.h"
#include "../core/circbuf.h"
#include "../core/funnytime.h"
#include "../core/accessors.h"
#include "../core/indexed.h"
#include "../core/trivial.h"
#include "../core/vcall.h"
#include "../core/savegame.h"
#include "../files/wrapsources.h"
#include "../math/x87.h"
#include "mapcell.h"
#include "messagecentre.h"
#include "object.h"
#include "objectmsg.h"
#include "objectdefs.h"
#include "thingmsg.h"
#include "things.h"
#include "thingscan.h"
#include "world.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// The funny time now (FunnyTime_Now of the world's clock, [0x80239c] + 0x2a0, no fields) to the analyser's 8 bytes at offset.
void stamp(u8 *analyser, u32 offset) {
    FileTime now;
    const FileTime *t = FunnyTime_Now(*reinterpret_cast<u8 *const *>(0x0080239c) + 0x2a0, nullptr, &now, nullptr, nullptr, nullptr, nullptr,
                                      nullptr, nullptr, nullptr, nullptr);
    at<FileTime>(analyser, offset) = *t;
}

const SaveSpec kVal[] = {{0x4, 4, 0x00759a48}, {0x8, 1, 0x00759a3c}};   // mVal, mWorkedOut
}  // namespace

extern "C" {

u8 *__fastcall AnalyserCell_Ctor(u8 *cell) {
    for (u32 t = 0x10; t < 0x7c; t += 0xc) {
        at<u32>(cell, t) = 0;
        at<u32>(cell, t + 4) = t < 0x28 ? 0xc479c000 : 0xfffffc19;  // -999.0f, -999
        at<u8>(cell, t + 8) = 0;
    }
    return cell;
}

void __fastcall ParkAnalyser_Record(u8 *analyser, void *, u32 ring, u32 value) {
    u8 *const r = analyser + 0x216fc + (ring << 6);
    const s32 size = at<s32>(r, 0x38);
    const s32 cursor = at<s32>(r, 0x34) + 1;
    at<s32>(r, 0x34) = cursor;
    if (cursor >= size) {
        at<s32>(r, 0x34) = 0;
        at<u8>(r, 0x3c) = 1;
    }
    at<u8>(r, (u32)at<s32>(r, 0x34) + 1) = (u8)value;
}

u8 __fastcall ParkAnalyser_RingFull(const u8 *analyser, void *, u32 ring) {
    const u8 *const r = analyser + 0x216fc + (ring << 6);
    if (at<u8>(r, 0x3c)) return at<s32>(r, 0x38) >= 0x32;
    return at<s32>(r, 0x34) + 1 >= 0x32;
}

u8 __fastcall ParkAnalyser_RingAverage(const u8 *analyser, void *, u32 ring) {
    const u8 *const r = analyser + 0x216fc + (ring << 6);
    const bool wrapped = at<u8>(r, 0x3c) != 0;
    s32 n = 0x32;
    const s32 held = wrapped ? at<s32>(r, 0x38) : at<s32>(r, 0x34) + 1;
    if (held < n) n = held;
    if (n <= 0) return 0;
    double sum = 0.0;   // (the double [0x700320]; byte sums and the quotient below are exact enough that x87's precision cannot matter)
    for (s32 i = 0; i < n; ++i) {
        const s32 size = at<s32>(r, 0x38), cursor = at<s32>(r, 0x34);
        if (i >= size) continue;
        if (i <= cursor)
            sum += r[(u32)cursor - (u32)i + 1u];   // (32-bit address arithmetic, as the original's)
        else if (wrapped)
            sum += r[(u32)cursor - (u32)i + (u32)size + 1u];
    }
    return (u8)(long long)(sum / n);   // (__ftol's low byte)
}

void __fastcall ParkAnalyser_Tally(u8 *analyser, void *, u32 flag) {
    ++at<u32>(analyser, 0x21bfc);
    ++at<u32>(analyser, 0x21c04);
    if (flag == 0) ++at<u32>(analyser, 0x21c00);
}

void __fastcall ParkAnalyser_Count(u8 *analyser, void *, s32 value) {
    ++at<u32>(analyser, 0x21c08);
    if (value > at<s32>(analyser, 0x21c40)) at<s32>(analyser, 0x21c40) = value;
    alignas(4) u8 msg[0x14];
    Object_PostMessage(ThingMsg1a_Ctor(msg, nullptr, 0xb, 1, 0, 0));
}

u16 *__fastcall ParkAnalyser_FindCutOff(u8 *, void *, u16 *out, u32 any) {
    void *const world = *reinterpret_cast<void *const *>(0x0080239c);
    u16 handle;
    World_FirstThing(world, nullptr, &handle);
    while (handle) {
        u8 *const thing = *reinterpret_cast<u8 *const *>(0x007cfb90 + handle * 20u);
        if (thing[2] == 3 && (thing[0x32] & 4) && (thing[0x32] & 8) && !Object_EntranceOnPath(thing) &&
            ((u8)any || (uintptr_t)Global8186ac_Get() != (uintptr_t)thing)) {
            *out = handle;
            return out;
        }
        u16 next;
        handle = *World_NextThing(world, nullptr, &next, thing);
    }
    *out = 0;
    return out;
}

u32 __fastcall ParkAnalyser_Tallied(const u8 *analyser, void *, u32 flag) {
    return (u8)flag ? at<u32>(analyser, 0x21c00) : at<u32>(analyser, 0x21bfc);
}

u32 __fastcall ParkAnalyser_Total(const u8 *analyser) { return at<u32>(analyser, 0x21c08); }

u32 __fastcall ParkAnalyser_TotalSince(const u8 *analyser, void *, s32 back) {
    const s32 size = at<s32>(analyser, 0x20a68);
    if (back >= size) return 0;
    const s32 cursor = at<s32>(analyser, 0x20a64);
    u32 slot;
    if (back <= cursor) {
        slot = (u32)cursor - (u32)back;
    } else if (at<u8>(analyser, 0x20a6c)) {
        slot = (u32)cursor - (u32)back + (u32)size;
    } else {
        return 0;
    }
    const u32 then = at<u32>(analyser, 0x20824 + slot * 4);
    return at<u32>(analyser, 0x21c08) - then;
}

u8 *__fastcall ParkAnalyser_Cell(u8 *analyser, void *, u32 pos) {
    const u32 i = (pos & 0xffff) - 1;
    const s32 scale = at<s32>(analyser, 0xc);
    const u32 row = (u32)((s32)(i >> 7) / scale), column = (u32)((s32)(i & 0x7f) / scale);
    return analyser + 0x20 + (row * at<u32>(analyser, 0x10) + column) * 0x7c;
}

void __fastcall ParkAnalyser_StampTime21c30(u8 *analyser) { stamp(analyser, 0x21c30); }

void __fastcall ParkAnalyser_StampTime21c38(u8 *analyser) { stamp(analyser, 0x21c38); }

void __fastcall ParkAnalyser_RecordDays(u8 *analyser, void *, u32 ticks) {
    FileTime span;
    const FileTime *t = FunnyTime_SpanOf(**reinterpret_cast<u8 *const *const *>(0x007cf6ec) + 0x2a0, nullptr, &span, ticks);
    const long long value = (long long)(((unsigned long long)t->high << 32) | t->low);
    const u32 days = (u32)(value / 864000000000LL);   // (_alldiv; the quotient's low dword)
    const s32 size = at<s32>(analyser, 0x216d0);
    const s32 cursor = at<s32>(analyser, 0x216cc) + 1;
    at<s32>(analyser, 0x216cc) = cursor;
    if (cursor >= size) {
        at<s32>(analyser, 0x216cc) = 0;
        at<u8>(analyser, 0x216d4) = 1;
    }
    at<u32>(analyser, 0x21604 + (u32)at<s32>(analyser, 0x216cc) * 4u) = days;
}

u32 __fastcall ParkAnalyser_InitCells(u8 *analyser) {
    MessageCentre_AddThing(MessageCentre_Instance(), nullptr, analyser, 0xc);
    MessageCentre_AddThing(MessageCentre_Instance(), nullptr, analyser, 0x11);
    MessageCentre_AddThing(MessageCentre_Instance(), nullptr, analyser, 0x13);
    const s32 count = (s32)(at<u32>(analyser, 0x14) * at<u32>(analyser, 0x10));
    for (s32 i = 0; i < count; ++i) {
        const s32 columns = at<s32>(analyser, 0x10);
        const u32 size = at<u32>(analyser, 0xc);
        const s32 column = i % columns, row = i / columns;
        const u32 right = (u32)(column + 1) * size, bottom = (u32)(row + 1) * size;
        const u32 w = (s32)right > at<s32>(analyser, 0x18) ? at<u32>(analyser, 0x18) - right + size : size;
        const u32 h = (s32)bottom > at<s32>(analyser, 0x1c) ? at<u32>(analyser, 0x1c) - bottom + size : size;
        u8 *const cell = analyser + 0x20 + i * 0x7c;
        at<u32>(cell, 4) = (u32)row * size;
        at<u32>(cell, 8) = w;
        at<u32>(cell, 0) = (u32)column * size;
        at<u32>(cell, 0xc) = h;
    }
    return 1;
}

u8 *__fastcall ParkAnalyser_Ctor(u8 *analyser) {
    Thing_Construct(analyser, nullptr, 1, 0xd);  // (the cell's upper half the original's stack bytes: Thing_Construct takes 16 bits)
    for (u32 i = 0; i < 0x400; ++i) AnalyserCell_Ctor(analyser + 0x20 + i * 0x7c);
    for (u32 i = 0; i < 4; ++i) CircBufU16_3_Ctor(analyser + 0x1f068 + i * 0x14);
    for (u32 i = 0; i < 6; ++i) {  // trackers: value 0, best -999, flag 0
        at<u32>(analyser, 0x1f0b8 + i * 12) = 0;
        at<s32>(analyser, 0x1f0bc + i * 12) = -999;
        at<u8>(analyser, 0x1f0c0 + i * 12) = 0;
    }
    // inlined CircBufU32 constructors: value +0 = 0, cursor -1, size, wrapped 0
    auto ring32 = [analyser](u32 offset, u32 size) {
        at<u32>(analyser, offset) = 0;
        at<s32>(analyser, offset + size * 4 + 4) = -1;
        at<u32>(analyser, offset + size * 4 + 8) = size;
        at<u8>(analyser, offset + size * 4 + 0xc) = 0;
    };
    for (u32 i = 0; i < 16; ++i) ring32(0x1f100 + i * 0x250, 0x90);
    ring32(0x21600, 50);
    ring32(0x216d8, 5);
    for (u32 i = 0; i < 20; ++i) CircBufU8_50_Ctor(analyser + 0x216fc + i * 0x40);
    at<u32>(analyser, 0xc) = 4;  // the cell size, the grid's columns and rows, the map's width and height
    at<u32>(analyser, 0x10) = 0x20;
    at<u32>(analyser, 0x14) = 0x20;
    at<u32>(analyser, 0x18) = 0x80;
    at<u32>(analyser, 0x1c) = 0x80;
    for (u32 offset = 0x21bfc; offset < 0x21c0c; offset += 4) at<u32>(analyser, offset) = 0;  // the tallies and the running total
    for (u32 offset = 0x21c10; offset < 0x21c40; offset += 8) stamp(analyser, offset);
    at<u32>(analyser, 0x21c40) = 0;
    at<u32>(analyser, 0x21c44) = 0;
    at<u32>(analyser, 0x21c48) = 0;
    ParkAnalyser_InitCells(analyser);
    return analyser;
}

u32 __fastcall AnalyserVal_Serialize(u8 *val, void *, Stream *stream, u32 save, s32 version, u32) {
    if (version < 0 || version > 1) return 0;
    return Save_Fields(stream, save, val, kVal, 2) ? 0 : 1;
}

u32 __fastcall AnalyserCell_Serialize(u8 *cell, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    static const SaveSpec kHead[] = {{0x0, 4, 0x00759aec}, {0x4, 4, 0x00759ae4}, {0x8, 4, 0x00759adc}, {0xc, 4, 0x00759ad4}};
    if (version < 0 || version > 1) return 0;
    if (save != 0) {
        if (!Save_Fields(stream, save, cell, kHead, 4)) return 1;
        for (u32 at = 0x10; at < 0x58; at += 0xc) {   // the first six values inline (as Save_WriteU32 / U8), the last three as themselves
            if (Save_WriteU32(stream, cell + at + 4, reinterpret_cast<const char *>(0x00759a48), 0)) return 1;
            if (Save_WriteU8(stream, cell + at + 8, reinterpret_cast<const char *>(0x00759a3c), 0)) return 1;
        }
        for (u32 at = 0x58; at < 0x7c; at += 0xc)
            if (AnalyserVal_Serialize(cell + at, nullptr, stream, save, version, unused)) return 1;
        return 0;
    }
    if (!Save_Fields(stream, 0, cell, kHead, 1)) return 1;
    for (u32 i = 1; i < 4; ++i)
        if (Save_ReadU32(stream, cell + kHead[i].at, reinterpret_cast<const char *>(kHead[i].name), 0)) return 1;
    for (u32 at = 0x10; at < 0x7c; at += 0xc)
        if (AnalyserVal_Serialize(cell + at, nullptr, stream, 0, version, unused)) return 1;
    return 0;
}

u32 __fastcall ParkAnalyser_Serialize(u8 *analyser, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    static const SaveSpec kHead[] = {{0xc, 4, 0x00759ac0}, {0x10, 4, 0x00759ab4}, {0x14, 4, 0x00759aa8}, {0x18, 4, 0x00759a9c},
                                     {0x1c, 4, 0x00759a90}};   // mMapSquaresPerCell, mGridSizeX / Y, mMapSizeX / Y
    static const SaveSpec kTail[] = {{0x0, 4, 0x00759a28}, {0x4, 4, 0x00759a04}, {0x8, 4, 0x007599ec}, {0xc, 4, 0x007599d8},
                                     {0x14, 8, 0x007599cc}, {0x1c, 8, 0x007599c0}, {0x24, 8, 0x007599b4}, {0x2c, 8, 0x007599a4},
                                     {0x34, 8, 0x00759990}, {0x3c, 8, 0x00759980}, {0x44, 4, 0x0075996c}, {0x48, 4, 0x0075995c},
                                     {0x4c, 4, 0x0075994c}};   // from +0x21bfc: mMisbehavingKids ..., mLastStrike ...
    if (Thing_Serialize(analyser, nullptr, stream, save, version, unused)) return 1;
    if (version < 0 || version > 1) return 0;
    // (saving, the original inlines the CircBufU16s, the values and the first nine CircBufU32s as their serializers write them)
    if (save != 0) {
        if (!Save_Fields(stream, save, analyser, kHead, 5)) return 1;
    } else {
        for (u32 i = 0; i < 5; ++i)
            if (Save_ReadU32(stream, analyser + kHead[i].at, reinterpret_cast<const char *>(kHead[i].name), 0)) return 1;
    }
    for (u32 i = 0; i < 0x400; ++i)
        if (AnalyserCell_Serialize(analyser + 0x20 + i * 0x7c, nullptr, stream, save, version, unused)) return 1;
    for (u32 i = 0; i < 4; ++i)
        if (CircBufU16_Serialize(analyser + 0x1f068 + i * 0x14, nullptr, stream, save, version, unused)) return 1;
    for (u32 i = 0; i < 6; ++i)
        if (AnalyserVal_Serialize(analyser + 0x1f0b8 + i * 0xc, nullptr, stream, save, version, unused)) return 1;
    for (u32 i = 0; i < 16; ++i)
        if (CircBufU32_144_Serialize(analyser + 0x1f100 + i * 0x250, nullptr, stream, save, version, unused)) return 1;
    if (CircBufU32_50_Serialize(analyser + 0x21600, nullptr, stream, save, version, unused)) return 1;
    if (CircBufU32_5_Serialize(analyser + 0x216d8, nullptr, stream, save, version, unused)) return 1;
    for (u32 i = 0; i < 0x14; ++i)
        if (CircBufU8_Serialize(analyser + 0x216fc + i * 0x40, nullptr, stream, save, version, unused)) return 1;
    u8 *const tail = analyser + 0x21bfc;
    for (u32 i = 0; i < 13; ++i) {
        const SaveSpec &f = kTail[i];
        const char *const name = reinterpret_cast<const char *>(f.name);
        u32 failed;
        if (save != 0)
            failed = f.size == 8 ? Save_Write8Bytes(stream, tail + f.at, name, 0) : Save_WriteU32(stream, tail + f.at, name, 0);
        else if (f.size == 8)
            failed = Save_Read8Bytes(stream, tail + f.at, name, 0);
        else   // sic: mMisbehavingKidsEver (+0x21c04) is written to the stream when loading, not read
            failed = i == 2 ? Save_WriteU32(stream, tail + f.at, name, 0) : Save_ReadU32(stream, tail + f.at, name, 0);
        if (failed) return 1;
    }
    return 0;
}

s32 __fastcall ParkAnalyser_AveragePersonStat19c(void *, void *, u32 cell, u32 radius) {
    const u32 id = (cell & 0xffff) - 1;
    const u32 column = id & 0x7f, row = id >> 7;
    const s32 left = (s32)(column - radius), top = (s32)(row - radius);
    const s32 right = column + radius > 0x7f ? 0x7f : (s32)(column + radius);   // (unsigned)
    const s32 bottom = row + radius > 0x7f ? 0x7f : (s32)(row + radius);
    s32 sum = 0, count = 0;
    for (s32 x = left; x <= right; ++x)
        for (s32 y = top; y <= bottom; ++y) {
            const u16 id_xy = (u16)(((u32)y << 7) + (u32)x + 1);
            u16 handle = at<u16>(*reinterpret_cast<u8 *const *>(0x008023a0), id_xy * 0x44u - 0x20u);
            while (handle) {
                const u8 *thing = static_cast<const u8 *>(reinterpret_cast<ThingSlot *>(0x007cfb90)[handle].thing);
                if (at<u8>(thing, 2) == 1) {
                    ++count;
                    sum += (u8)(long long)at<float>(thing, 0x19c);   // (__ftol's low byte)
                }
                handle = at<u16>(thing, 0xa);
            }
        }
    return count ? sum / count : 0;
}

u32 __fastcall AnalyserCell_CountBusy(u8 *cell, void *, u32 limit) {
    if (!cell[0x6c] || limit < Game1da70c_MinusAt0Quarters(reinterpret_cast<const u32 *>(cell + 0x64))) {
        u32 count = 0;
        for (s32 x = at<s32>(cell, 0); x < at<s32>(cell, 0) + at<s32>(cell, 8); ++x)
            for (s32 y = at<s32>(cell, 4); y < at<s32>(cell, 4) + at<s32>(cell, 0xc); ++y) {
                const u16 id = (u16)((y << 7) + x + 1);
                const u8 *const m = *reinterpret_cast<u8 *const *>(0x008023a0) + id * 0x44u - 0x44u;
                count += MapCell_IsType1(m) || MapCell_IsType3Or9(m) || MapCell_IsType9(m) || MapCell_IsTypeA(m) || Zero_5d60c0() ||
                                 MapCell_IsType4(m)
                             ? 1
                             : 0;
            }
        at<u32>(cell, 0x68) = count;
        cell[0x6c] = 1;
        Out_SetFromGame1da70c(reinterpret_cast<u32 *>(cell + 0x64));
    }
    return at<u32>(cell, 0x68);
}

u32 __fastcall ParkAnalyser_CountBusy(u8 *analyser, void *, u32 limit) {
    u32 sum = 0;
    for (u32 i = 0x400; i-- > 0;) sum += AnalyserCell_CountBusy(analyser + 0x20 + 0x7c * i, nullptr, limit);
    return sum;
}

u32 __fastcall AnalyserCell_CountType1Or3Or9(u8 *cell, void *, u32 limit) {
    if (!cell[0x54] || limit < Game1da70c_MinusAt0Quarters(reinterpret_cast<const u32 *>(cell + 0x4c))) {
        u32 count = 0;
        for (s32 x = at<s32>(cell, 0); x < at<s32>(cell, 0) + at<s32>(cell, 8); ++x)
            for (s32 y = at<s32>(cell, 4); y < at<s32>(cell, 4) + at<s32>(cell, 0xc); ++y) {
                const u16 id = (u16)((y << 7) + x + 1);
                const u8 *const m = *reinterpret_cast<u8 *const *>(0x008023a0) + id * 0x44u - 0x44u;
                count += MapCell_IsType1(m) || MapCell_IsType3Or9(m) ? 1 : 0;
            }
        at<u32>(cell, 0x50) = count;
        cell[0x54] = 1;
        Out_SetFromGame1da70c(reinterpret_cast<u32 *>(cell + 0x4c));
    }
    return at<u32>(cell, 0x50);
}

u32 __fastcall ParkAnalyser_CountType1Or3Or9(u8 *analyser, void *, u32 limit) {
    u32 sum = 0;
    for (u32 i = 0x400; i-- > 0;) sum += AnalyserCell_CountType1Or3Or9(analyser + 0x20 + 0x7c * i, nullptr, limit);
    return sum;
}

u32 __fastcall AnalyserCell_CountWith2c(u8 *cell, void *, u32 limit) {
    if (!cell[0x60] || limit < Game1da70c_MinusAt0Quarters(reinterpret_cast<const u32 *>(cell + 0x58))) {
        u32 count = 0;
        const s32 x0 = at<s32>(cell, 0), x1 = x0 + at<s32>(cell, 8);
        const s32 y0 = at<s32>(cell, 4), y1 = y0 + at<s32>(cell, 0xc);   // (the bounds and the map read once)
        const u8 *const map = *reinterpret_cast<u8 *const *>(0x008023a0);
        for (s32 x = x0; x < x1; ++x)
            for (s32 y = y0; y < y1; ++y) {
                const u16 id = (u16)((y << 7) + x + 1);
                count += at<u32>(map, id * 0x44u - 0x18u) != 0 ? 1 : 0;
            }
        at<u32>(cell, 0x5c) = count;
        cell[0x60] = 1;
        Out_SetFromGame1da70c(reinterpret_cast<u32 *>(cell + 0x58));
    }
    return at<u32>(cell, 0x5c);
}

u32 __fastcall ParkAnalyser_CountWith2c(u8 *analyser, void *, u32 limit) {
    u32 sum = 0;
    for (u32 i = 0x400; i-- > 0;) sum += AnalyserCell_CountWith2c(analyser + 0x20 + 0x7c * i, nullptr, limit);
    return sum;
}

void __fastcall ParkAnalyser_Sample(u8 *analyser) {
    u8 *const world = **reinterpret_cast<u8 *const *const *>(0x007cf6ec);
    const u32 position = Source_Field0c(reinterpret_cast<const TrackedSource *>((uintptr_t)GameTable7cfb90_Get1da726_5195d0(world)));
    CircBufU32_144_Push(analyser + 0x1f100, nullptr, position);
    // (the first four through 0x4ce290, the rest inlined: each the dword at its +0 pushed, then cleared)
    static const u32 kAccumulators[] = {0x1f5a0u, 0x1f7f0u, 0x1fa40u, 0x1fc90u, 0x1fee0u, 0x20130u, 0x20380u, 0x20cc0u};
    for (const u32 ring : kAccumulators) {
        CircBufU32_144_Push(analyser + ring, nullptr, at<u32>(analyser, ring));
        at<u32>(analyser, ring) = 0;
    }
    CircBufU32_144_Push(analyser + 0x205d0, nullptr, ParkAnalyser_CountKind1OnPlainCell(analyser, nullptr, 0));
    CircBufU32_144_Push(analyser + 0x20820, nullptr, at<u32>(analyser, 0x21c08));
    CircBufU32_144_Push(analyser + 0x20a70, nullptr, ParkAnalyser_CountDef4ac0(analyser, nullptr, 0));
    const float average = static_cast<float>(Things_AveragePersonStat19c(analyser, nullptr, 0));   // (fstp m32)
    u32 bits;
    std::memcpy(&bits, &average, 4);
    CircBufU32_144_Push(analyser + 0x20f10, nullptr, bits);
    CircBufU32_144_Push(analyser + 0x1f350, nullptr, ParkAnalyser_ParkScore(analyser, nullptr, 0));
    at<u32>(analyser, 0x21bfc) = 0;
    at<u32>(analyser, 0x21c00) = 0;
    double sum;
    const s32 count = CircBufU32_50_Sum(analyser + 0x21600, nullptr, &sum, 0x32);
    CircBufU32_144_Push(analyser + 0x21160, nullptr, count > 0 ? (u32)(long long)(X87Reg(sum) / X87Reg((double)count)).v : 0);
    CircBufU32_144_Push(analyser + 0x213b0, nullptr, (u32)Things_FacilityScore(analyser));
}

void __fastcall ParkAnalyser_HandleMessage(u8 *analyser, void *, u8 *message) {
    switch (vcall<u32>(message, 0)) {
    case 0xc:
        ParkAnalyser_Sample(analyser);
        return;
    case 0x11: {   // (a u16 ring of the definition's +0x4ac: +8 cursor, +0xc size, +0x10 wrapped, the values from +2)
        const u16 id = at<u16>(message, 8);
        u8 *const r = analyser + (at<u32>(ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, id), 0x4ac) + 0x18d2) * 0x14;
        const s32 size = at<s32>(r, 0xc);
        const s32 cursor = at<s32>(r, 8) + 1;
        at<s32>(r, 8) = cursor;
        if (cursor >= size) {
            at<s32>(r, 8) = 0;
            at<u8>(r, 0x10) = 1;
        }
        at<u16>(r, 2 + 2 * at<s32>(r, 8)) = id;
        return;
    }
    case 0x13:
        if (at<u32>(message, 8) == 3) stamp(analyser, 0x21c28);
        return;
    case 0x17:
        if (at<u32>(message, 0xc) == 2) stamp(analyser, 0x21c10);
        return;
    }
}

u32 __fastcall ParkAnalyser_ParkScore(u8 *analyser, void *, u32 limit) {
    if (!analyser[0x1f0f0] || limit < Game1da70c_MinusAt0Quarters(reinterpret_cast<const u32 *>(analyser + 0x1f0e8))) {
        s32 score = 0;
        u8 *const cells = *reinterpret_cast<u8 *const *>(0x0080239c) + 0x2d8;
        for (s32 x = 0x7f; x >= 0; --x)
            for (s32 y = 0x7f; y >= 0; --y) {
                const u16 id = (u16)((y << 7) + x + 1);
                u8 *const cell = cells + id * 0x44u - 0x44u;
                if (MapCell_HasFlags(cell, nullptr, 0x40)) continue;
                if (MapCell_IsType2(cell)) score += 1;
                if (MapCell_IsType1(cell)) score += 3;
                if (MapCell_IsType3Or9(cell)) score += 5;
                if (MapCell_IsType9(cell)) score += 1;
                if (MapCell_IsTypeA(cell)) score += 1;
                if (Zero_5d60c0()) score += 10;
                if (MapCell_IsType0(cell)) score += 2;
                if (MapHandle_IsTypeBDOr10Or12Or19(reinterpret_cast<const u16 *>(cell))) score += 0x32;
            }
        u16 handle = at<u16>(*reinterpret_cast<u8 *const *>(0x0080239c), 0x1da746);
        while (handle) {
            u8 *const thing = static_cast<u8 *>(reinterpret_cast<ThingSlot *>(0x007cfb90)[handle].thing);
            s32 value = 0;
            if (!Is19c_Eq4_4e0380(thing)) {
                s32 sum = 0;
                u32 i = 0;
                do {   // (the original's JC after a TEST never jumps: at least once)
                    const u8 *const def = ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(thing, 0xe));
                    sum += at<s32>(def, 0x1b8 + i * 0x40);
                    ++i;
                } while (i <= thing[0x50]);
                value = (s32)(((u32)(long long)at<float>(thing, 0x44) & 0xff) * (u32)sum) / 100;   // (__ftol's low byte)
            }
            score += value;
            handle = at<u16>(thing, 0xc);
        }
        at<s32>(analyser, 0x1f0ec) = score;
        analyser[0x1f0f0] = 1;
        Out_SetFromGame1da70c(reinterpret_cast<u32 *>(analyser + 0x1f0e8));
    }
    return at<u32>(analyser, 0x1f0ec);
}

u32 __fastcall ParkAnalyser_CountAtTicketBooths(u8 *analyser) {
    s32 count = 0;
    for (u32 b = 0; b <= 1; ++b) {
        u8 *const world = *reinterpret_cast<u8 *const *>(0x0080239c);
        u16 pos;
        const u32 id = *Map_TicketBoothPos(world + 0x2d8, nullptr, &pos, b);
        u16 handle = at<u16>(world, id * 0x44 + 0x2b8);
        while (handle) {
            const u8 *thing = static_cast<const u8 *>(reinterpret_cast<ThingSlot *>(0x007cfb90)[handle].thing);
            if (at<u8>(thing, 2) == 1 && at<u32>(thing, 0x220) == 3) ++count;
            handle = at<u16>(thing, 0xa);
        }
    }
    at<s32>(analyser, 0x1f0d4) = count;
    analyser[0x1f0d8] = 1;
    Out_SetFromGame1da70c(reinterpret_cast<u32 *>(analyser + 0x1f0d0));
    return at<u32>(analyser, 0x1f0d4);
}
}
