#include "parkinfo.h"

#include <cstdint>

#include "savegame.h"

namespace {

const char *name(u32 address) { return reinterpret_cast<const char *>((uintptr_t)address); }

// One field through the stream: whether no error was reported and all of it went.
bool transfer(Stream *stream, u32 save, void *at, s32 size) {
    s32 result, done = 0;
    const s32 *const r = save ? Stream_Write(&result, stream, at, size, &done) : Stream_Read(&result, stream, at, size, &done);
    return *r >= 0 && done == size;
}

bool transferAll(Stream *stream, u32 save, u8 *at, s32 size, u32 count) {
    for (u32 i = 0; i < count; ++i)
        if (!transfer(stream, save, at + i * size, size)) return false;
    return true;
}

// u16s read by Save_ReadU16 (quiet): whether all came.
bool readU16s(Stream *stream, u8 *at, u32 count, u32 field) {
    for (u32 i = 0; i < count; ++i)
        if (Save_ReadU16(stream, at + i * 2, name(field), 1) != 0) return false;
    return true;
}

}  // namespace

extern "C" {

u32 __fastcall ParkInfo_Serialize(u8 *self, void *, Stream *stream, u32 save, s32 version, u32) {
    if (version < 0 || version > 1) return 0;
    if (!transferAll(stream, save, self, 2, 0x10) || !transferAll(stream, save, self + 0x20, 2, 0x80) ||
        !transferAll(stream, save, self + 0x120, 4, 3) || !transferAll(stream, save, self + 0x12c, 2, 0x10))
        return 1;
    if (save) {
        if (!transferAll(stream, save, self + 0x14c, 2, 0x10) || !transferAll(stream, save, self + 0x16c, 2, 0x32) ||
            !transferAll(stream, save, self + 0x1d0, 2, 0x32) || !transferAll(stream, save, self + 0x3ce, 2, 3))
            return 1;
        return 0;
    }
    if (!readU16s(stream, self + 0x14c, 0x10, 0x0074c2d4) || !readU16s(stream, self + 0x16c, 0x32, 0x0074c2c0) ||
        !readU16s(stream, self + 0x1d0, 0x32, 0x0074c2a8) || !readU16s(stream, self + 0x3ce, 1, 0x0074c290) ||
        !readU16s(stream, self + 0x3d0, 1, 0x0074c278) || !readU16s(stream, self + 0x3d2, 1, 0x0074c260))
        return 1;
    return 0;
}
}
