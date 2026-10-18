#include "array3d.h"

#include <cstdint>
#include <cstring>

#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "../core/vwrap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *Pixels(const u8 *buffer) { return at<u8 *>(buffer, 0xc); }

// width * height as the code multiplies it, re-read from the header.
s32 Count(const u8 *buffer) { return at<s32>(buffer, 0) * at<s32>(buffer, 4); }

// size bytes from the stream (0x5c4f60 with a count of 1).
void take(void *to, u32 size, u8 *stream) { Object_CallSlot8Product((u32)(uintptr_t)to, size, 1, stream); }

// The codec's slot 3 (thiscall; width, height, flag, data, out, three dwords) on the array's width and height.
void decode(u8 *codec, const u8 *object, u32 flag, u8 *data, u8 *out, u32 a, u32 b, u32 c) {
    vcall<void>(codec, 0xc, at<u32>(object, 0), at<u32>(object, 4), flag, data, out, a, b, c);
}

}  // namespace

extern "C" {

u8 *__fastcall Array3D_Fill(u8 *buffer, void *, u32 a, u32 b, u32 c, u32 d) {
    u8 *pixel = Pixels(buffer);
    if (!pixel)
        return pixel;
    const u32 depth = at<u32>(buffer, 8);
    if (depth == 4) {
        if (Count(buffer) <= 0)
            return pixel;
        s32 i = 0;
        do {
            *pixel++ = (u8)d;
            *pixel++ = (u8)a;
            *pixel++ = (u8)b;
            *pixel++ = (u8)c;
            i++;
        } while (i < Count(buffer));
    } else if (depth == 1) {
        if (Count(buffer) <= 0)
            return pixel;
        s32 i = 0;
        do {
            *pixel++ = (u8)d;
            i++;
        } while (i < Count(buffer));
    }
    return pixel;
}

u8 *__fastcall Array3D_CopyTo(u8 *buffer, void *, u8 *dest) {
    const u8 *from = Pixels(buffer);
    if (!from)
        return buffer;
    const u32 bytes = at<u32>(buffer, 8) * at<u32>(buffer, 4) * at<u32>(buffer, 0);
    u8 *to = Pixels(dest);
    for (u32 i = 0; i < bytes / 4; i++, from += 4, to += 4)
        at<u32>(to, 0) = at<u32>(from, 0);
    for (u32 i = 0; i < (bytes & 3); i++)
        *to++ = *from++;
    return dest;
}

u8 *__fastcall Array3D_ReverseBytes(u8 *buffer) {
    u8 *pixel = Pixels(buffer);
    if (Count(buffer) > 0) {
        s32 i = 0;
        do {
            const u8 b0 = pixel[0], b1 = pixel[1], b2 = pixel[2], b3 = pixel[3];
            pixel[0] = b3;
            pixel[1] = b2;
            pixel[2] = b1;
            pixel[3] = b0;
            pixel += 4;
            i++;
        } while (i < Count(buffer));
    }
    return pixel;
}

u8 *__fastcall Array3D_MaxInto(u8 *buffer, void *, u8 *other) {
    const u8 *from = Pixels(buffer);
    u8 *to = Pixels(other);
    if (!from || at<s32>(buffer, 4) <= 0)
        return to;
    s32 y = 0;
    do {
        if (at<s32>(buffer, 0) > 0) {
            s32 x = 0;
            do {
                if (*to <= *from)
                    *to = *from;
                to++;
                from++;
                x++;
            } while (x < at<s32>(buffer, 0));
        }
        to += (at<s32>(other, 0) - at<s32>(buffer, 0)) * at<s32>(other, 8);
        y++;
    } while (y < at<s32>(buffer, 4));
    return to;
}

u8 __fastcall Array3D_Realloc(u8 *object, void *, u32 a, u32 b, u32 c) {
    if (at<void *>(object, 0xc))
        GameHeap_Free(at<void *>(object, 0xc));
    const u32 size = c * b * a;
    u8 *block = static_cast<u8 *>(GameHeap_Alloc(size));
    at<u8 *>(object, 0xc) = block;
    if (!block)
        return 0;
    std::memset(block, 0, size);
    at<u32>(object, 8) = c;
    at<u32>(object, 0) = a;
    at<u32>(object, 4) = b;
    return 1;
}

u8 *__fastcall Array3D_Init(u8 *object, void *, u32 a, u32 b, u32 c) {
    at<u32>(object, 8) = c;
    at<u32>(object, 0) = a;
    at<u32>(object, 4) = b;
    at<void *>(object, 0xc) = GameHeap_Alloc(c * b * a);
    return object;
}

void __fastcall Array3D_Free(u8 *object) {
    void *block = at<void *>(object, 0xc);
    at<u32>(object, 0) = 0;
    at<u32>(object, 4) = 0;
    at<u32>(object, 8) = 0;
    if (block) {
        GameHeap_Free(block);
        at<u32>(object, 0xc) = 0;
    }
}

void __fastcall ArrayHolder_Clear(u8 *object) {
    Array3D_Free(object + 0x2c);
    Array3D_Free(object + 0x3c);
    Array3D_Free(object + 0x4c);
    Array3D_Free(object + 0x5c);
    for (u32 offset = 0x5c; offset >= 0x2c; offset -= 0x10) {
        u8 *const array = object + offset;
        if (at<void *>(array, 0xc)) GameHeap_Free(at<void *>(array, 0xc));
        at<u32>(array, 0) = 0;
        at<u32>(array, 4) = 0;
        at<u32>(array, 8) = 0;
        at<u32>(array, 0xc) = 0;
    }
}

void __fastcall Array3Ds_Free4(u8 *object) {
    Array3D_Free(object + 0x2c);
    Array3D_Free(object + 0x3c);
    Array3D_Free(object + 0x4c);
    Array3D_Free(object + 0x5c);
}

u32 __fastcall Array3D_Read(u8 *object, void *, u8 *stream) {
    u32 a, b, c;
    Object_CallSlot8Product((u32)(uintptr_t)&a, 4, 1, stream);
    Object_CallSlot8Product((u32)(uintptr_t)&b, 4, 1, stream);
    Object_CallSlot8Product((u32)(uintptr_t)&c, 4, 1, stream);
    if (at<void *>(object, 0xc))
        GameHeap_Free(at<void *>(object, 0xc));
    const u32 size = c * b * a;
    u8 *block = static_cast<u8 *>(GameHeap_Alloc(size));
    at<u8 *>(object, 0xc) = block;
    if (block) {
        std::memset(block, 0, size);
        at<u32>(object, 0) = a;
        at<u32>(object, 4) = b;
        at<u32>(object, 8) = c;
    }
    const u32 read = Object_CallSlot8Product((u32)(uintptr_t)at<u8 *>(object, 0xc), c * b * a, 1, stream);  // sic: into a null block too
    return (read & ~0xffu) | 1;
}

u8 __fastcall Array3D_ReadCompressed(u8 *object, void *, u8 *stream, u8 *codec) {
    at<u32>(object, 0) = 0;
    at<u32>(object, 4) = 0;
    at<u32>(object, 8) = 0;
    if (at<void *>(object, 0xc))
        GameHeap_Free(at<void *>(object, 0xc));
    at<void *>(object, 0xc) = nullptr;
    u32 a, b, c;
    take(&a, 4, stream);
    take(&b, 4, stream);
    take(&c, 4, stream);
    if (at<void *>(object, 0xc))
        GameHeap_Free(at<void *>(object, 0xc));
    const u32 size = c * b * a;
    u8 *block = static_cast<u8 *>(GameHeap_Alloc(size));
    at<u8 *>(object, 0xc) = block;
    if (block) {
        std::memset(block, 0, size);
        at<u32>(object, 0) = a;
        at<u32>(object, 4) = b;
        at<u32>(object, 8) = c;
    }
    if (c == 1) {
        u32 total, length;
        take(&total, 4, stream);
        take(&length, 4, stream);
        u8 *data = static_cast<u8 *>(GameHeap_Alloc(length));
        if (!data)
            return 0;
        take(data, length, stream);
        decode(codec, object, 0, data, Pixels(object), total, 0, 0);
        GameHeap_Free(data);
        return 1;
    }
    if (c == 3) {
        u32 q0, q1, q2, length;
        take(&q0, 4, stream);
        take(&q1, 4, stream);
        take(&q2, 4, stream);
        take(&length, 4, stream);
        u8 *data = static_cast<u8 *>(GameHeap_Alloc(length));
        if (!data)
            return 0;
        take(data, length, stream);
        decode(codec, object, 1, data, Pixels(object), q0, q1, q2);
        GameHeap_Free(data);
        return 1;
    }
    if (c == 4) {
        u32 q0, q1, q2, total, length, length2;
        take(&q0, 4, stream);
        take(&q1, 4, stream);
        take(&q2, 4, stream);
        take(&total, 4, stream);
        take(&length, 4, stream);
        take(&length2, 4, stream);
        u8 *data = static_cast<u8 *>(GameHeap_Alloc(length));
        if (!data)
            return 0;
        u8 *data2 = static_cast<u8 *>(GameHeap_Alloc(length2));
        if (!data2)
            return 0;
        take(data, length, stream);
        take(data2, length2, stream);
        u8 *rgb = static_cast<u8 *>(GameHeap_Alloc(at<u32>(object, 0) * at<u32>(object, 4) * 3));
        decode(codec, object, 1, data, rgb, q0, q1, q2);
        u8 *alpha = static_cast<u8 *>(GameHeap_Alloc(at<u32>(object, 0) * at<u32>(object, 4)));
        decode(codec, object, 0, data2, alpha, total, 0, 0);
        u8 *out = Pixels(object);
        const u8 *from = rgb;
        for (s32 i = 0; i < Count(object); ++i) {
            out[0] = alpha[i];
            out[1] = from[0];
            out[2] = from[1];
            out[3] = from[2];
            out += 4;
            from += 3;
        }
        GameHeap_Free(alpha);
        GameHeap_Free(rgb);
        GameHeap_Free(data2);
        GameHeap_Free(data);
    }
    return 1;
}
}
