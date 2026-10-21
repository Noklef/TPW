#include "analyser.h"

#include <cstdint>

#include "../core/filetime.h"
#include "../core/circbuf.h"
#include "../core/funnytime.h"
#include "../core/savegame.h"
#include "messagecentre.h"
#include "things.h"

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

void __fastcall ParkAnalyser_Tally(u8 *analyser, void *, u32 flag) {
    ++at<u32>(analyser, 0x21bfc);
    ++at<u32>(analyser, 0x21c04);
    if (flag == 0) ++at<u32>(analyser, 0x21c00);
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
}
