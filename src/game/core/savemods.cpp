#include "savemods.h"

#include <cstdint>
#include <cstring>

#include "../coaster/ridebounds.h"
#include "../math/x87.h"
#include "../rsse/rsse.h"
#include "../rsse/scriptkill.h"
#include "../world/camera.h"
#include "../world/flyingrides.h"
#include "soundset.h"

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

bool Read_Ok(Stream *stream, u32 address, u32 count) {
    s32 result;
    return *Stream_Read(&result, stream, reinterpret_cast<void *>((uintptr_t)address), (s32)count, nullptr) >= 0;
}

void Read(Stream *stream, void *buffer, u32 count) {
    s32 result;
    Stream_Read(&result, stream, buffer, (s32)count, nullptr);
}

// A block of size bytes (none for 0: the original prints, left out).
void *Block(u32 size) { return size ? Mem_GlobalAlloc(size) : nullptr; }

u32 bits(float f) {
    u32 b;
    std::memcpy(&b, &f, 4);
    return b;
}

}  // namespace

extern "C" {

u32 __cdecl Camera_Save(Stream *stream) {
    return Write_Ok(stream, 0x007909ec, 4) && Write_Ok(stream, 0x00790a38, 4) && Write_Ok(stream, 0x00790ab0, 4) && Write_Ok(stream, 0x007908f0, 0xc) &&
           Write_Ok(stream, 0x00790ad0, 0xc) && Write_Ok(stream, 0x00790a9c, 4);
}

u32 __cdecl Camera_Load(Stream *stream) {
    if (!(Read_Ok(stream, 0x007909ec, 4) && Read_Ok(stream, 0x00790a38, 4) && Read_Ok(stream, 0x00790ab0, 4) && Read_Ok(stream, 0x007908f0, 0xc) &&
          Read_Ok(stream, 0x00790ad0, 0xc) && Read_Ok(stream, 0x00790a9c, 4)))
        return 0;
    global<u32>(0x00790ab0) = 0;
    global<u32>(0x007909e0) = global<u32>(0x00790a38);
    if (global<u8>(0x0078d912))
        Camera_SnapYaw();
    return 1;
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

u32 __cdecl Particles_Load(Stream *stream) {
    if (global<u32>(0x00816d40) == 0)
        return 0;
    s32 result;
    u32 slot = (u32)(uintptr_t)stream;  // (the read goes to the same slot every time: a short read leaves the old value)
    Stream_Read(&result, stream, &slot, 4, nullptr);
    const u32 savedF4 = global<u32>(0x0080cef4), savedEc = global<u32>(0x0080ceec);
    if (slot != 0x5054434c)
        return 0;
    Stream_Read(&result, stream, &slot, 4, nullptr);
    if (slot == 0)
        return 0;
    if (global<void *>(0x0080cef8))
        Mem_GlobalFree(global<void *>(0x0080cef8));
    Stream_Read(&result, stream, &slot, 4, nullptr);
    if (slot != 0x9e68)
        return 1;
    Stream_Read(&result, stream, &global<u8>(0x0080ced0), 0x9e68, nullptr);
    Stream_Read(&result, stream, &slot, 4, nullptr);
    if (slot != 0x8b60)
        return 1;
    Stream_Read(&result, stream, &global<u8>(0x00804370), 0x8b60, nullptr);
    const u32 bytes = global<u32>(0x0080cedc) * 0x34;
    global<void *>(0x0080cef8) = bytes ? Mem_GlobalAlloc(bytes) : nullptr;
    u32 count = 0;
    Stream_Read(&result, stream, &count, 4, nullptr);
    if (count != 0) {
        Stream_Read(&result, stream, global<void *>(0x0080cef8), (s32)count, nullptr);
    } else {
        u8 *pool = global<u8 *>(0x0080cef8);
        for (u32 i = 0; i < bytes; ++i)
            pool[i] = 0;
        global<u32>(0x0080ced0) = 0;
        global<u16>(0x00816d24) = 0;
        at<u16>(global<u8 *>(0x0080cef8), 0x32) = 0xffff;
        at<u16>(global<u8 *>(0x0080cef8), 0x30) = 1;
        at<u16>(global<u8 *>(0x0080cef8), global<u32>(0x0080cedc) * 0x34 - 4) = 0xffff;
        at<u16>(global<u8 *>(0x0080cef8), global<u32>(0x0080cedc) * 0x34 - 2) = (u16)(global<u32>(0x0080cedc) - 2);
        if ((s32)global<u32>(0x0080cedc) - 1 > 1) {
            s32 i = 1;
            u32 offset = 0x34;
            do {
                at<u16>(global<u8 *>(0x0080cef8), offset + 0x32) = (u16)(i - 1);
                at<u16>(global<u8 *>(0x0080cef8), offset + 0x30) = (u16)(i + 1);
                ++i;
                offset += 0x34;
            } while (i < (s32)global<u32>(0x0080cedc) - 1);
        }
        for (s32 system = global<s16>(0x00816d1c); system >= 0;) {
            u8 *entry = reinterpret_cast<u8 *>((uintptr_t)(0x0080cefc + system * 0x140));
            at<u16>(entry, 8) = 0;
            at<u16>(entry, 6) = 0xffff;
            system = at<s16>(entry, 0xd0);
        }
    }
    global<u32>(0x0080ceec) = savedEc;
    global<u32>(0x0080cef4) = savedF4;
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

u32 __cdecl RSSE_Load(Stream *stream) {
    if (global<u32>(0x008791a0)) {
        for (RsseContext *ctx = global<RsseContext *>(0x008791b0); ctx; ctx = RSSE_FreeContext(ctx, 0)) {
        }
        global<u32>(0x008791b0) = 0;
        global<u32>(0x008791a0) = 0;
        // (a non-zero 0x8791b8 prints "Something malloced but not freed": Debug_Printf, left out)
    }
    RSSE_Init();   // (inlined in the original)
    // The original's stack slots: a read that falls short leaves them as they were (the bench's streams hold the counts).
    u32 value = 0, count = 0, size = 0, extra = 0, objects = 0;
    Read(stream, &value, 4);
    if (value != 0x45535352)
        return 1;   // ("Load Fail - Missing header": Debug_Printf, left out)
    Read(stream, &value, 4);
    Read(stream, &global<u8>(0x008791a0), value);
    global<u32>(0x008791b0) = 0;
    for (int i = 0; i < 5; ++i)
        Read(stream, &value, 4);
    Read(stream, &count, 4);
    Read(stream, &size, 4);
    for (s32 n = 0; n < (s32)count; ++n) {
        u8 *const rec = static_cast<u8 *>(Mem_GlobalAlloc(0xf4));
        std::memset(rec, 0, 0xf4);
        ++global<s32>(0x008791ac);
        at<u8 *>(rec, 0) = global<u8 *>(0x008791b0);
        at<u8 *>(rec, 4) = nullptr;
        if (global<u8 *>(0x008791b0))
            at<u8 *>(global<u8 *>(0x008791b0), 4) = rec;
        global<u8 *>(0x008791b0) = rec;
        u8 *const next = at<u8 *>(rec, 0), *const prev = at<u8 *>(rec, 4);
        Read(stream, rec, size);   // (the record as saved, its links kept)
        at<u8 *>(rec, 4) = prev;
        at<u8 *>(rec, 0) = next;
        // the code (+0x18, its dwords +0x50), always read (into none for 0 bytes)
        Read(stream, &value, 4);
        at<void *>(rec, 0x18) = Block(value);
        Read(stream, at<void *>(rec, 0x18), value);
        at<u32>(rec, 0x50) = value >> 2;
        // the stack (+0x20, dwords +0x54) and the variables (+0x1c, dwords +0x8c)
        Read(stream, &value, 4);
        at<void *>(rec, 0x20) = nullptr;
        if (value) {
            at<void *>(rec, 0x20) = Mem_GlobalAlloc(value);
            Read(stream, at<void *>(rec, 0x20), value);
        }
        at<u32>(rec, 0x54) = value >> 2;
        Read(stream, &value, 4);
        at<void *>(rec, 0x1c) = nullptr;
        if (value) {
            at<void *>(rec, 0x1c) = Mem_GlobalAlloc(value);
            Read(stream, at<void *>(rec, 0x1c), value);
        }
        at<u32>(rec, 0x8c) = value >> 2;
        // +0x34 (bytes +0x90), the limbo +0x24 (+0x58: bytes, then entries of 8), the riders +0x28 (+0x64: bytes, then entries of 0x10)
        Read(stream, &at<u32>(rec, 0x90), 4);
        at<void *>(rec, 0x34) = nullptr;
        if (const u32 bytes = at<u32>(rec, 0x90)) {
            at<void *>(rec, 0x34) = Mem_GlobalAlloc(bytes);
            Read(stream, at<void *>(rec, 0x34), at<u32>(rec, 0x90));
        }
        Read(stream, &at<u32>(rec, 0x58), 4);
        at<void *>(rec, 0x24) = nullptr;
        if (const u32 bytes = at<u32>(rec, 0x58)) {
            at<void *>(rec, 0x24) = Mem_GlobalAlloc(bytes);
            Read(stream, at<void *>(rec, 0x24), at<u32>(rec, 0x58));
            at<u32>(rec, 0x58) >>= 3;
        }
        Read(stream, &at<u32>(rec, 0x64), 4);
        if (const u32 bytes = at<u32>(rec, 0x64)) {
            at<void *>(rec, 0x28) = Mem_GlobalAlloc(bytes);
            Read(stream, at<void *>(rec, 0x28), at<u32>(rec, 0x64));
            at<u32>(rec, 0x64) >>= 4;
        } else {
            at<void *>(rec, 0x28) = nullptr;
        }
        // the walkers +0x2c (+0x7c of 0x20 bytes), the keyed parts' dwords +0x30 (+0x4c: bytes, then dwords) and the folder +0x38
        Read(stream, &at<u32>(rec, 0x7c), 4);
        at<void *>(rec, 0x2c) = nullptr;
        if (at<u32>(rec, 0x7c)) {
            at<void *>(rec, 0x2c) = Block(at<u32>(rec, 0x7c) << 5);
            Read(stream, at<void *>(rec, 0x2c), at<u32>(rec, 0x7c) << 5);
        }
        Read(stream, &at<u32>(rec, 0x4c), 4);
        at<void *>(rec, 0x30) = nullptr;
        if (const u32 bytes = at<u32>(rec, 0x4c)) {
            at<void *>(rec, 0x30) = Mem_GlobalAlloc(bytes);
            Read(stream, at<void *>(rec, 0x30), at<u32>(rec, 0x4c));
        }
        at<u32>(rec, 0x4c) >>= 2;
        Read(stream, &extra, 4);
        if (extra) {   // (none: +0x38 stays as the record had it)
            at<void *>(rec, 0x38) = Mem_GlobalAlloc(extra);
            Read(stream, at<void *>(rec, 0x38), extra);
        }
        // a sound occluder (+0xd4) made again for a ride's (+0xc8) box, as RSSE_LoadScriptFile's (else +0xd4 stays as saved)
        float box[6];
        if (at<u32>(rec, 0xd4) && at<u32>(rec, 0xc8) && Ride_Bounds(at<s32>(rec, 0xc8), box)) {
            const s32 owner = at<s32>(rec, 0xc8);
            const float half = global<float>(0x00700fd0);
            const float cx = static_cast<float>((X87Reg(box[0]) + box[3]) * half);
            const u8 *const instance = global<u8 *>(0x007a4610 + 4 * owner);
            const float y = X87_Through(at<float>(at<u8 *>(at<u8 *>(at<u8 *>(instance, 8), 4), 0x78), 0x44));
            const float cz = static_cast<float>((X87Reg(box[2]) + box[5]) * half);
            float sizes[6];
            if (owner && Ride_Bounds(owner, sizes)) {
                const float dx = static_cast<float>(X87Reg(sizes[3]) - sizes[0]);
                const float dy = static_cast<float>(X87Reg(sizes[4]) - sizes[1]);
                const float radius = static_cast<float>(((X87Reg(sizes[5]) - sizes[2]) + dx) * half);
                at<u32>(rec, 0xd4) = Sound_AddOccluder(bits(cx), bits(y), bits(cz), bits(radius), bits(dy));
            }
        }
        at<u16>(rec, 0xe6) = 0xffff;
        at<u8 *>(rec, 0xb0) = nullptr;
        // the object list: 'OBJ ', the count and the node size; each node (0x1c bytes) pushed at the head, its links kept
        Read(stream, &value, 4);
        if (value != 0x204a424f)
            return 1;   // ("Load Fail - Object list missing": Debug_Printf, left out)
        Read(stream, &objects, 4);
        Read(stream, &value, 4);
        for (s32 k = 0; k < (s32)objects; ++k) {
            u8 *const node = static_cast<u8 *>(Mem_GlobalAlloc(0x1c));
            at<u8 *>(node, 0) = at<u8 *>(rec, 0xb0);
            at<u8 *>(node, 4) = nullptr;
            if (at<u8 *>(rec, 0xb0))
                at<u8 *>(at<u8 *>(rec, 0xb0), 4) = node;
            at<u8 *>(rec, 0xb0) = node;
            ++global<s32>(0x008791b4);
            u8 *const link_prev = at<u8 *>(node, 4), *const link_next = at<u8 *>(node, 0);
            Read(stream, node, value);
            at<u8 *>(node, 0) = link_next;
            at<u8 *>(node, 4) = link_prev;
        }
    }
    return 0;
}

u32 __cdecl FlyingRides_Load(Stream *stream) {
    FlyingRides_Shutdown();   // (inlined in the original)
    // The original's stack slot: a read that falls short leaves it as it was (uninitialised before the magic).
    u32 value = 0;
    Read(stream, &value, 4);
    if (value != 0x5f594c46)
        return 1;   // ("Load Fail - Missing header": Debug_Printf, left out)
    Read(stream, &value, 4);
    if (!value)
        return 0;
    FlyingRides_Init();   // (inlined in the original: the flag is clear here)
    Read(stream, &value, 4);
    Read(stream, &global<u8>(0x008791f8), value);
    for (u32 address = 0x007660b8; address <= 0x007660c4; address += 4)
        Read(stream, &global<u32>(address), 4);
    for (int i = 0; i < 6; ++i)   // (five pads, then the record size)
        Read(stream, &value, 4);
    for (u32 address = 0x008791fc; address < 0x00879388; address += 4) {
        if (global<u32>(address)) {   // (the saved pointer: a record follows)
            global<void *>(address) = Mem_GlobalAlloc(value);
            Read(stream, global<void *>(address), value);
        }
    }
    return 0;
}

}  // extern "C"
