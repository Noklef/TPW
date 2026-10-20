#include "circbuf.h"

#include <cstdint>

#include "../math/x87.h"
#include "savegame.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

// The sample `back` entries behind the cursor of a buffer of 144, if there is one (a negative index is read as the
// code does, from beside the cursor).
static bool Sample(const u8 *buffer, s32 back, s32 &value) {
    const s32 count = at<s32>(buffer, 0x248), head = at<s32>(buffer, 0x244);
    if (back >= count)
        return false;
    if (back <= head) {
        value = at<s32>(buffer, 4 + 4 * (u32)(head - back));
        return true;
    }
    if (at<u8>(buffer, 0x24c) == 0)
        return false;
    value = at<s32>(buffer, 4 + 4 * (u32)(head - back + count));
    return true;
}

// The same for a buffer of floats.
static bool SampleF(const u8 *buffer, s32 back, float &value) {
    const s32 count = at<s32>(buffer, 0x248), head = at<s32>(buffer, 0x244);
    if (back >= count)
        return false;
    if (back <= head) {
        value = at<float>(buffer, 4 + 4 * (u32)(head - back));
        return true;
    }
    if (at<u8>(buffer, 0x24c) == 0)
        return false;
    value = at<float>(buffer, 4 + 4 * (u32)(head - back + count));
    return true;
}

static s32 Window(u32 kind) { return kind == 0 ? 0xc : kind == 1 ? 0x24 : kind == 2 ? 0x90 : 0; }

extern "C" {

void __cdecl CircBufU32_144_MaxInWindow(const u8 *buffer, s32 *out, u32 kind) {
    const s32 window = Window(kind), count = at<s32>(buffer, 0x248);
    s32 best = 0, bestAt = -1;
    for (s32 i = 0; count > 0 && i <= window; i++) {
        s32 value;
        if (!Sample(buffer, i, value))
            break;
        if (value > best) {
            best = value;
            bestAt = i;
        }
        if (i + 1 >= count)
            break;
    }
    s32 result = window;
    if (bestAt < count)
        Sample(buffer, bestAt, result);
    if (result > *out)
        *out = result;
}

void __cdecl CircBufF32_144_MaxInWindow(const u8 *buffer, s32 *out, u32 kind) {
    const s32 window = Window(kind), count = at<s32>(buffer, 0x248);
    float best = 0.0f;
    s32 bestAt = -1;
    for (s32 i = 0; count > 0 && i <= window; i++) {
        float value;
        if (!SampleF(buffer, i, value))
            break;
        if (value > best) {
            best = value;
            bestAt = i;
        }
        if (i + 1 >= count)
            break;
    }
    float result = best;
    if (bestAt < count)
        SampleF(buffer, bestAt, result);
    const s32 whole = (s32)(long long)X87Reg(result).v;  // __ftol
    if (whole > *out)
        *out = whole;
}

void __cdecl CircBufU32_144_MinInWindow(const u8 *buffer, s32 *out, u32 kind) {
    const s32 window = Window(kind), count = at<s32>(buffer, 0x248);
    s32 best = 0, bestAt = -1;
    for (s32 i = 0; count > 0 && i <= window; i++) {
        s32 value;
        if (!Sample(buffer, i, value))
            break;
        if (value < best) {
            best = value;
            bestAt = i;
        }
        if (i + 1 >= count)
            break;
    }
    if (bestAt == -1)
        return;
    s32 result = window;
    if (bestAt < count)
        Sample(buffer, bestAt, result);
    if (result < *out)
        *out = result;
}

u8 *__fastcall CircBufU16_3_Ctor(u8 *buf) {
    at<u16>(buf, 0) = 0;
    at<u32>(buf, 8) = 0xffffffff;
    at<u32>(buf, 0xc) = 3;
    at<u8>(buf, 0x10) = 0;
    return buf;
}

u8 *__fastcall CircBufU8_50_Ctor(u8 *buf) {
    at<u8>(buf, 0) = 0;
    at<u32>(buf, 0x34) = 0xffffffff;
    at<u32>(buf, 0x38) = 50;
    at<u8>(buf, 0x3c) = 0;
    return buf;
}

namespace {

u8 *u32_ctor(u8 *buf, u32 size) {
    at<u32>(buf, 0) = 0;
    at<u32>(buf, 4 + size * 4) = 0xffffffff;
    at<u32>(buf, 8 + size * 4) = size;
    at<u8>(buf, 0xc + size * 4) = 0;
    return buf;
}

}  // namespace

u8 *__fastcall CircBufU32_30_Ctor(u8 *buf) { return u32_ctor(buf, 30); }
u8 *__fastcall CircBufU32_144_Ctor(u8 *buf) { return u32_ctor(buf, 144); }
u8 *__fastcall CircBufU32_50_Ctor(u8 *buf) { return u32_ctor(buf, 50); }
u8 *__fastcall CircBufU32_5_Ctor(u8 *buf) { return u32_ctor(buf, 5); }

void __fastcall CircBufU32_144_Push(u8 *buf, void *, u32 value) {
    const s32 size = at<s32>(buf, 0x248);
    const s32 cursor = at<s32>(buf, 0x244) + 1;
    at<s32>(buf, 0x244) = cursor;
    if (cursor >= size) {
        at<s32>(buf, 0x244) = 0;
        at<u8>(buf, 0x24c) = 1;
    }
    at<u32>(buf, 4 + (u32)at<s32>(buf, 0x244) * 4) = value;
}

u32 __fastcall CircBufU32_30_Count(const u8 *buf) {
    return at<u8>(buf, 0x84) ? at<u32>(buf, 0x80) : at<u32>(buf, 0x7c) + 1;
}

u8 __fastcall CircBufU32_30_Get(const u8 *buf, void *, u32 *out, s32 back) {
    const s32 size = at<s32>(buf, 0x80);
    if (back >= size) return 0;
    const s32 cursor = at<s32>(buf, 0x7c);
    if (back <= cursor) {
        *out = at<u32>(buf, 4 + ((u32)cursor - (u32)back) * 4);
        return 1;
    }
    if (!at<u8>(buf, 0x84)) return 0;
    *out = at<u32>(buf, 4 + ((u32)cursor - (u32)back + (u32)size) * 4);
    return 1;
}

void __fastcall CircBufU32_30_Sum(const u8 *buf, void *, double *sum, s32 count) {
    const s32 limit = at<u8>(buf, 0x84) ? at<s32>(buf, 0x80) : at<s32>(buf, 0x7c) + 1;
    const s32 n = count > limit ? limit : count;
    *sum = 0;
    for (s32 i = 0; i < n; ++i) {
        u32 value;
        if (CircBufU32_30_Get(buf, nullptr, &value, i)) *sum = X87_Add((double)value, *sum);
    }
}

// A CircBufU32's serializer for its cursor's offset (4 + 4 * its size).
static u32 SerializeU32(u8 *buf, Stream *stream, u32 save, s32 version, u16 cursor) {
    const SaveSpec fields[] = {{cursor, 4, 0x00759a80}, {(u16)(cursor + 4), 4, 0x00759a74}, {(u16)(cursor + 8), 1, 0x00759a64}, {0x0, 4, 0x00759a5c}};
    if (version < 0 || version > 1)
        return 0;
    if (!Save_Fields(stream, save, buf, fields, 4))
        return 1;
    for (s32 i = 0; i < at<s32>(buf, cursor + 4); ++i)  // sic: not bounded by the buffer's size
        if (!Save_Field(stream, save, buf + 4 + i * 4, 4, 0x00759a50))
            return 1;
    return 0;
}

u32 __fastcall CircBufU32_30_Serialize(u8 *buf, void *, Stream *stream, u32 save, s32 version, u32) {
    return SerializeU32(buf, stream, save, version, 0x7c);
}

u32 __fastcall CircBufU32_144_Serialize(u8 *buf, void *, Stream *stream, u32 save, s32 version, u32) {
    return SerializeU32(buf, stream, save, version, 0x244);
}
}
