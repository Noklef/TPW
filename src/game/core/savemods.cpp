#include "savemods.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

void Write(Stream *stream, const void *buffer, u32 count) {
    s32 result;
    Stream_Write(&result, stream, buffer, (s32)count, nullptr);
}

bool Write_Ok(Stream *stream, u32 address, u32 count) {
    s32 result;
    return *Stream_Write(&result, stream, reinterpret_cast<const void *>((uintptr_t)address), (s32)count, nullptr) >= 0;
}

void WriteValue(Stream *stream, u32 value) { Write(stream, &value, 4); }

}  // namespace

extern "C" {

u32 __cdecl Camera_Save(Stream *stream) {
    return Write_Ok(stream, 0x007909ec, 4) && Write_Ok(stream, 0x00790a38, 4) && Write_Ok(stream, 0x00790ab0, 4) && Write_Ok(stream, 0x007908f0, 0xc) &&
           Write_Ok(stream, 0x00790ad0, 0xc) && Write_Ok(stream, 0x00790a9c, 4);
}

u32 __cdecl Particles_Save(Stream *stream, s32 full) {
    WriteValue(stream, 0x5054434c);
    Write(stream, &global<u32>(0x00816d40), 4);
    if (global<u32>(0x00816d40) != 0) {
        WriteValue(stream, 0x9e68);
        Write(stream, &global<u8>(0x0080ced0), 0x9e68);
        WriteValue(stream, 0x8b60);
        Write(stream, &global<u8>(0x00804370), 0x8b60);
        const u32 size = full == 0 ? 0 : global<u32>(0x0080cedc) * 0x34;
        WriteValue(stream, size);
        if (size != 0)
            Write(stream, global<void *>(0x0080cef8), size);
    }
    return 0;
}

u32 __cdecl FlyingRides_Save(Stream *stream) {
    WriteValue(stream, 0x5f594c46);
    Write(stream, &global<u32>(0x008791ec), 4);
    if (global<u32>(0x008791ec) != 0) {
        WriteValue(stream, 400);
        Write(stream, &global<u8>(0x008791f8), 400);
        for (u32 address = 0x007660b8; address <= 0x007660c4; address += 4)
            Write(stream, &global<u32>(address), 4);
        const u32 pad = 0x5f444150;
        for (int i = 0; i < 5; ++i)
            Write(stream, &pad, 4);
        WriteValue(stream, 0x122c);
        for (u32 address = 0x008791fc; (s32)address < 0x879388; address += 4) {
            if (u8 *block = global<u8 *>(address))
                Write(stream, block, 0x122c);
        }
    }
    return 0;
}

u32 __cdecl RSSE_Save(Stream *stream) {
    WriteValue(stream, 0x45535352);
    WriteValue(stream, 0x14);
    Write(stream, &global<u8>(0x008791a0), 0x14);
    if (global<u32>(0x008791a0) == 0)
        return 0;
    const u32 pad = 0x5f444150;
    for (int i = 0; i < 5; ++i)
        Write(stream, &pad, 4);
    u32 count = 0;
    for (const u8 *ctx = global<const u8 *>(0x008791b0); ctx; ctx = at<const u8 *>(ctx, 0))
        ++count;
    WriteValue(stream, count);
    WriteValue(stream, 0xf4);
    for (const u8 *ctx = global<const u8 *>(0x008791b0); ctx; ctx = at<const u8 *>(ctx, 0)) {
        Write(stream, ctx, 0xf4);
        u32 size = at<u32>(ctx, 0x50) << 2;
        WriteValue(stream, size);
        Write(stream, at<const void *>(ctx, 0x18), size);
        size = at<u32>(ctx, 0x54) << 2;
        WriteValue(stream, size);
        if (size != 0)
            Write(stream, at<const void *>(ctx, 0x20), size);
        size = at<u32>(ctx, 0x8c) << 2;
        WriteValue(stream, size);
        if (size != 0)
            Write(stream, at<const void *>(ctx, 0x1c), size);
        size = at<u32>(ctx, 0x90);
        WriteValue(stream, size);
        if (size != 0)
            Write(stream, at<const void *>(ctx, 0x34), size);
        size = at<u32>(ctx, 0x58) << 3;
        WriteValue(stream, size);
        if (size != 0)
            Write(stream, at<const void *>(ctx, 0x24), size);
        size = at<u32>(ctx, 0x64) << 4;
        WriteValue(stream, size);
        if (size != 0)
            Write(stream, at<const void *>(ctx, 0x28), size);
        size = at<u32>(ctx, 0x7c);
        WriteValue(stream, size);
        if (size != 0)
            Write(stream, at<const void *>(ctx, 0x2c), size << 5);
        size = at<u32>(ctx, 0x4c) << 2;
        WriteValue(stream, size);
        if (size != 0)
            Write(stream, at<const void *>(ctx, 0x30), size);
        const char *name = at<const char *>(ctx, 0x38);
        u32 length = 0;
        while (name[length])
            ++length;
        size = length + 1;
        WriteValue(stream, size);
        Write(stream, name, size);
        WriteValue(stream, 0x204a424f);
        count = 0;
        for (const u8 *object = at<const u8 *>(ctx, 0xb0); object; object = at<const u8 *>(object, 0))
            ++count;
        WriteValue(stream, count);
        WriteValue(stream, 0x1c);
        for (const u8 *object = at<const u8 *>(ctx, 0xb0); object; object = at<const u8 *>(object, 0))
            Write(stream, object, 0x1c);
    }
    return 0;
}

}  // extern "C"
