#include "options.h"

#include <cstdint>

#include "../render/mode.h"
#include "accessors.h"
#include "../files/file.h"
#include "gameheap.h"
#include "savegame.h"
#include "soundset.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const char *name(u32 address) { return reinterpret_cast<const char *>(static_cast<uintptr_t>(address)); }

// The save game's field names of the switches +0x34..+0x3a (AdvisorOn .. IsometricOn).
const u32 kFlagNames[7] = {0x0074c7e4, 0x0074c7d8, 0x0074c7cc, 0x0074c7bc, 0x0074c7b0, 0x0074c7a4, 0x0074c798};

// count bytes written / read in full, without an error.
bool write(Stream *stream, const void *p, s32 count) {
    s32 result, done = 0;
    return *Stream_Write(&result, stream, p, count, &done) >= 0 && done == count;
}

bool read(Stream *stream, void *p, s32 count) {
    s32 result, done = 0;
    return *Stream_Read(&result, stream, p, count, &done) >= 0 && done == count;
}

void copy(u8 *dst, const u8 *src) {
    for (u32 i = 0; i < 0x3c; i += 4) at<u32>(dst, i) = at<u32>(src, i);
}

void set_changed(u8 *options, u32 offset, u32 value) {
    if (at<u32>(options, offset) != value) at<u32>(options, offset) = value;
}

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __fastcall Options_Revert(u8 *options) {
    copy(options, options + 0x3c);
    at<u32>(options, 0x94) = 0;
    at<u32>(options, 0x98) &= ~7u;
}

void __fastcall Options_Keep(u8 *options) {
    copy(options + 0x3c, options);
    if (at<s32>(options, 0x94) < 0) at<s32>(options, 0x94) = 0;
    at<u32>(options, 0x98) &= ~7u;
}

void __fastcall Options_Set30(u8 *options, void *, u32 value) { at<u32>(options, 0x30) = value; }
void __fastcall Options_Set4(u8 *options, void *, u32 value) { set_changed(options, 4, value); }
void __fastcall Options_Set8(u8 *options, void *, u32 value) { set_changed(options, 8, value); }
void __fastcall Options_SetC(u8 *options, void *, u32 value) { set_changed(options, 0xc, value); }

void __fastcall Setting_SetMode(u8 *object, void *, u32 mode) {
    at<u32>(object, 0) = mode;
    u32 &flags = global<u32>(0x007a1a8c);
    if (mode == 0)
        flags = (flags & 0xfcffffff) | 0x800000;
    else if (mode == 1)
        flags = (flags & 0xfd7fffff) | 0x1000000;
    else if (mode == 2)
        flags = (flags & 0xfe7fffff) | 0x2000000;
}

u8 *__fastcall Setting_ReadMode(u8 *object) {
    u32 &flags = global<u32>(0x007a1a8c);
    if (flags & 0x2000000) {
        at<u32>(object, 0) = 2;
        flags = (flags & 0xfe7fffff) | 0x2000000;
    } else if (flags & 0x1000000) {
        at<u32>(object, 0) = 1;
        flags = (flags & 0xfd7fffff) | 0x1000000;
    } else {
        at<u32>(object, 0) = 0;
        flags = (flags & 0xfcffffff) | 0x800000;
    }
    return object;
}

u32 __cdecl Graphics_Tier() {
    if (RenderMode_Size() == 0x4000) {
        const u32 depth = RenderMode_Depth();
        if (depth == 8 || depth == 0x10) return 2;
        return depth == 0x20 ? 1 : 0;
    }
    return *reinterpret_cast<u32 *>(0x007858d0) == 0 ? 1 : 2;  // (a jump table: 0 -> 1, 1..3 -> 2; above 3 -> 2)
}

u32 __cdecl Graphics_Level() {
    if (RenderMode_Size() == 0x4000) {
        const u32 depth = RenderMode_Depth();
        if (depth == 8) return 8;
        if (depth == 0x10) return 6;
        return depth == 0x20 ? 4 : 0;
    }
    switch (*reinterpret_cast<u32 *>(0x007858d0)) {
    case 0: return 2;
    case 1: return 4;
    case 2: return 6;
    default: return 8;
    }
}

void __cdecl Options_ApplySound() {
    const u8 *const o = reinterpret_cast<const u8 *>((uintptr_t)0x0078d8d8);
    if (o[0x18]) {
        Sound_Set803adc(1);
        Sound_Set803ac0(at<u32>(o, 0x1c));
    } else {
        Sound_Set803adc(0);
    }
    if (o[0x20]) {
        Sound_Set803ae0(1);
        Sound_Set803ac4(at<u32>(o, 0x24));
    } else {
        Sound_Set803ae0(0);
    }
    if (o[0x10]) {
        Sound_Set803ad8(1);
        Sound_Set803abc(at<u32>(o, 0x14));
    } else {
        Sound_Set803ad8(0);
    }
    Global802ba8_SetMasked(o[0x28] ? (u32)((s32)(at<u32>(o, 0x2c) * 1023u) / 100) : 0);
}

void __fastcall Options_Defaults(u8 *options) {
    at<u32>(options, 0) = 1;
    at<u32>(options, 4) = 1;
    at<u32>(options, 8) = 1;
    at<u32>(options, 0xc) = 0;
    at<u32>(options, 0x14) = global<u32>(0x00785928);
    at<u32>(options, 0x1c) = global<u32>(0x0078592c);
    at<u32>(options, 0x24) = global<u32>(0x00785930);
    at<u32>(options, 0x2c) = global<u32>(0x00785934);
    if (global<u32>(0x00785924) != 0) {
        options[0x10] = 1;
        options[0x18] = 1;
        options[0x20] = 1;
        options[0x28] = 1;
    }
    options[0x34] = 1;
    at<u32>(options, 0x30) = global<u32>(0x0078591c);
    options[0x35] = 1;
    options[0x36] = 1;
    options[0x37] = 1;
    options[0x39] = 1;
    options[0x3a] = 1;
    options[0x38] = 0;
    copy(options + 0x3c, options);
    Options_ApplySound();
}

u32 __fastcall Options_SerializeSound(u8 *options, void *, Stream *stream, u32 saving, u32) {
    if (!saving) {
        if (Save_Read8Bytes(stream, options + 0x10, name(0x0074c818), 1) || Save_Read8Bytes(stream, options + 0x18, name(0x0074c80c), 1) ||
            Save_Read8Bytes(stream, options + 0x20, name(0x0074c7fc), 1) || Save_Read8Bytes(stream, options + 0x28, name(0x0074c7f0), 1))
            return 1;
        for (u32 k = 0; k < 7; k++)   // (AdvisorOn .. IsometricOn: +0x34..+0x3a)
            if (Save_ReadU8(stream, options + 0x34 + k, name(kFlagNames[k]), 1))
                return 1;
    } else {
        for (u32 off = 0x10; off < 0x30; off += 8)
            if (!write(stream, options + off, 8))
                return 1;
        for (u32 off = 0x34; off < 0x3b; off++)
            if (!write(stream, options + off, 1))
                return 1;
    }
    Options_ApplySound();
    return 0;
}

u32 __fastcall Options_SerializeDisplay(u8 *options, void *, Stream *stream, u32 saving, u32) {
    if (saving) {
        if (!write(stream, options, 4) || !write(stream, options + 4, 4) || !write(stream, options + 8, 4) || !write(stream, options + 0xc, 4) ||
            !write(stream, options + 0x28, 8) || !write(stream, options + 0x30, 4))
            return 1;
    } else {
        if (!read(stream, options, 4) || !read(stream, options + 4, 4) || !read(stream, options + 8, 4))
            return 1;
        if (Save_ReadU32(stream, options + 0xc, name(0x0074c834), 1) || Save_Read8Bytes(stream, options + 0x28, name(0x0074c7f0), 1) ||
            Save_ReadU32(stream, options + 0x30, name(0x0074c824), 1))
            return 1;
    }
    Options_ApplySound();
    return 0;
}

u32 __fastcall Options_SerializeMisc(u8 *options, void *, Stream *stream, u32 saving, u32) {
    if (saving) {
        for (u32 off = 0x78; off < 0x90; off += 4)
            if (!write(stream, options + off, 4))
                return 1;
        return 0;
    }
    if (!read(stream, options + 0x78, 4) || !read(stream, options + 0x7c, 4) || !read(stream, options + 0x80, 4))
        return 1;
    if (Save_ReadU32(stream, options + 0x84, name(0x0074c864), 1) || Save_ReadU32(stream, options + 0x88, name(0x0074c854), 1) ||
        Save_ReadU32(stream, options + 0x8c, name(0x0074c840), 1))
        return 1;
    return 0;
}

u32 __fastcall Options_SaveFile(u8 *options, void *, const u8 *path) {
    s32 result;
    Stream *stream;
    if (*FileStream_Open(&result, &stream, at<const char *>(path, 8), 0xc0000010) < 0)
        return 0;
    u32 version = 1;
    s32 done = 0;
    const bool ok = *Stream_Write(&result, stream, &version, 4, &done) >= 0 && done == 4 &&
                    Options_SerializeDisplay(options, nullptr, stream, 1, version) == 0;
    Stream_Close(&result, stream);
    return ok ? 1 : 0;
}

u32 __fastcall Options_LoadFile(u8 *options, void *, const u8 *path) {
    Options_Defaults(options);
    s32 result;
    Stream *stream;
    if (*FileStream_Open(&result, &stream, at<const char *>(path, 8), 0x80000001) < 0)
        return 0;
    u32 version;
    s32 done = 0;
    const bool ok = *Stream_Read(&result, stream, &version, 4, &done) >= 0 && done == 4 && version != 0 &&
                    Options_SerializeDisplay(options, nullptr, stream, 0, version) == 0;
    Stream_Close(&result, stream);
    return ok ? 1 : 0;
}

u32 __fastcall Options_LoadMiscFile(u8 *options, void *, const u8 *path) {
    at<u32>(options, 0x78) = 0;
    at<u32>(options, 0x7c) = 0;
    at<u32>(options, 0x80) = 0;
    at<u32>(options, 0x84) = 1;
    at<u32>(options, 0x88) = 0;
    at<u32>(options, 0x8c) = 0;
    at<u32>(options, 0x90) = 0;
    s32 result;
    Stream *stream;
    if (*FileStream_Open(&result, &stream, at<const char *>(path, 8), 0x80000001) < 0)
        return 0;
    u32 version;
    s32 done = 0;
    const bool ok = *Stream_Read(&result, stream, &version, 4, &done) >= 0 && done == 4 && version >= 2 &&
                    Options_SerializeMisc(options, nullptr, stream, 0, version) == 0;
    Stream_Close(&result, stream);
    return ok ? 1 : 0;
}
}
