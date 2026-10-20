#include "analyser.h"

#include <cstdint>

#include "../core/filetime.h"
#include "../core/funnytime.h"
#include "messagecentre.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// The funny time now (FunnyTime_Now of the world's clock, [0x80239c] + 0x2a0, no fields) to the analyser's 8 bytes at offset.
void stamp(u8 *analyser, u32 offset) {
    FileTime now;
    const FileTime *t = FunnyTime_Now(*reinterpret_cast<u8 *const *>(0x0080239c) + 0x2a0, nullptr, &now, nullptr, nullptr, nullptr, nullptr,
                                      nullptr, nullptr, nullptr, nullptr);
    at<FileTime>(analyser, offset) = *t;
}

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
}
