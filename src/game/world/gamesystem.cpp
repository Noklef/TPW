#include "gamesystem.h"

#include <cstdint>

#include "../files/file.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

bool write(Stream *stream, const void *data) {
    s32 result;
    return *Stream_Write(&result, stream, data, 4, nullptr) >= 0;
}

bool read(Stream *stream, void *data) {
    s32 result;
    return *Stream_Read(&result, stream, data, 4, nullptr) >= 0;
}

}  // namespace

extern "C" {

u32 __cdecl GameSystem_Save(Stream *stream) {
    if (!write(stream, &global<u32>(0x00878c74)) || !write(stream, &global<u32>(0x0087879c)) || !write(stream, &global<u32>(0x00878a1c)) ||
        !write(stream, &global<u32>(0x00877d34)) || !write(stream, &global<u32>(0x008786c0)))
        return 0;
    const s32 unset = 0;  // sic: the original writes an uninitialised local here
    const s32 first = global<s32>(0x00878c74) - global<s32>(0x008786bc);
    const s32 second = global<s32>(0x0087879c) - global<s32>(0x008786bc);
    const s32 third = global<s32>(0x00878a1c) - global<s32>(0x008786bc);
    return write(stream, &unset) && write(stream, &first) && write(stream, &second) && write(stream, &third);
}

u32 __cdecl GameSystem_Load(Stream *stream) {
    s32 unset = 0, first = 0, second = 0, third = 0;  // (stack garbage in the original when the stream runs short: not reproducible)
    if (!read(stream, &global<u32>(0x00878c74)) || !read(stream, &global<u32>(0x0087879c)) || !read(stream, &global<u32>(0x00878a1c)) ||
        !read(stream, &global<u32>(0x00877d34)) || !read(stream, &global<u32>(0x008786c0)))
        return 0;
    if (!read(stream, &unset) || !read(stream, &first) || !read(stream, &second) || !read(stream, &third))
        return 0;
    global<s32>(0x00878c74) = first + global<s32>(0x008786bc);
    global<s32>(0x0087879c) = second + global<s32>(0x008786bc);
    global<s32>(0x00878a1c) = third + global<s32>(0x008786bc);
    return 1;
}
}
