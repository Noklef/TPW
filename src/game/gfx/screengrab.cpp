#include "screengrab.h"

#include <cstdint>
#include <cstring>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "../files/filestream.h"
#include "../files/handles.h"
#include "../render/d3dresult.h"
#include "../render/engineerrors.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

AddrCalls g_calls;

}  // namespace

void ScreenGrab_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u32 __fastcall ScreenGrab_ToRgb24(u8 *grab, void *) {
    const s32 depth = global<s32>(0x008bcbe4);
    if (depth == 0x10) {
        const u32 out = TrackedGlobal_Alloc(at<u32>(grab, 8) * 3 + 1);
        at<u32>(grab, 4) = out;
        if (!out) {
            TrackedGlobal_Free(at<u32>(grab, 0));
            at<u32>(grab, 0) = 0;
            EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 2, 0xb0754000);
            return 0;
        }
        const u32 maskA = global<u32>(0x008bcc54), maskB = global<u32>(0x008bcc6c), maskC = global<u32>(0x008bcc60);
        const u32 shiftA = (0x18 - global<u32>(0x008bcc58) - global<u32>(0x008bcc5c)) & 31;
        const u32 shiftC = (0x10 - global<u32>(0x008bcc64) - global<u32>(0x008bcc68)) & 31;
        const u32 shiftB = (8 - global<u32>(0x008bcc70) - global<u32>(0x008bcc74)) & 31;
        const u16 *pixels = at<const u16 *>(grab, 0);
        for (s32 i = 0; i < at<s32>(grab, 8); ++i) {
            const u32 p = pixels[i];
            *reinterpret_cast<u32 *>((uintptr_t)at<u32>(grab, 4) + 3 * i) = (p & maskB) << shiftB | (p & maskA) << shiftA | (p & maskC) << shiftC;
        }
        TrackedGlobal_Free(at<u32>(grab, 0));
        at<u32>(grab, 0) = 0;
        return 1;
    }
    if (depth == 0x20) {
        for (s32 i = 0; i < at<s32>(grab, 8); ++i) {
            at<u8 *>(grab, 0)[3 * i] = at<u8 *>(grab, 0)[4 * i];
            at<u8 *>(grab, 0)[3 * i + 1] = at<u8 *>(grab, 0)[4 * i + 1];
            at<u8 *>(grab, 0)[3 * i + 2] = at<u8 *>(grab, 0)[4 * i + 2];
        }
    } else if (depth != 0x18) {
        return 1;
    }
    const u32 pixels = at<u32>(grab, 0);
    at<u32>(grab, 0) = 0;
    at<u32>(grab, 4) = pixels;
    return 1;
}

u32 __fastcall ScreenGrab_TakeFront(u8 *grab, void *) {
    const u32 count = global<u32>(0x008bcbd0) * global<u32>(0x008bcbd4);
    at<u32>(grab, 8) = count;
    const u32 pixels = TrackedGlobal_Alloc(global<u32>(0x008bcbe4) * count);   // (the depth in bits: eight times the room, sic)
    at<u32>(grab, 0) = pixels;
    if (!pixels) {
        EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 2, 0xb0754000);
        return 0;
    }
    u8 *const front = at<u8 *>(global<u8 *>(0x00879a24), 0x14);
    u32 desc[0x1f] = {0x7c};
    d3dresult::keep(comcall<u32>(front, 0x64, nullptr, desc, 1u, nullptr));
    if (d3dresult::result()) return 0;
    const u32 bytes = (u32)(global<s32>(0x008bcbe4) >> 3);
    const u8 *from = reinterpret_cast<const u8 *>((uintptr_t)desc[9]);
    u8 *to = reinterpret_cast<u8 *>((uintptr_t)pixels);
    for (s32 y = 0; y < global<s32>(0x008bcbd4); ++y) {
        const u32 row = bytes * global<u32>(0x008bcbd0);
        memcpy(to, from, row);
        from += (s32)desc[4];
        to += row;
    }
    d3dresult::keep(comcall<u32>(front, 0x80, nullptr));
    return d3dresult::result() == 0;
}

u32 __fastcall ScreenGrab_SetName(u8 *grab, void *, const char *name) {
    if (name) {
        u32 size = 0;  // repne scasb, then rep movsd / movsb of the name and its 0
        while (name[size++] != 0) {
        }
        u8 *const out = grab + 0xc;
        for (u32 i = 0; i < size; i++) out[i] = (u8)name[i];
        at<u8 *>(grab, 0x8c) = out;
    }
    return 1;
}

void __cdecl ScreenGrab_NextName() {
    StreamBase stream;
    char name[0x14];
    s32 result;
    StreamBase_Ctor(&stream);
    stream.vtable = reinterpret_cast<void *const *>((u32)(uintptr_t)FileStream_vtable);
    do {
        StreamBase_Close(&stream);
        Crt().sprintf(name, gScreengrab_7650dc, ++gScreenGrab_Count_879084);   // "Scr%05ld.tga"
    } while (*FileStream_OpenPath(&stream, nullptr, &result, name, 0x80000001) >= 0);
    StreamBase_Close(&stream);
    ScreenGrab_SetName(reinterpret_cast<u8 *>(0x008798d0), nullptr, name);
    StreamBase_Dtor(&stream);
}

u32 __fastcall ScreenGrab_WriteTga(u8 *grab, void *, const char *path) {
    u8 header[0x12] = {0, 0, 2};
    at<u16>(header, 0xc) = global<u16>(0x008bcbd0);
    at<u16>(header, 0xe) = global<u16>(0x008bcbd4);
    header[0x10] = 0x18;
    const u32 file = g_calls.Cd(0x0067b8a0, path, 0x00766128u);
    if (!file) {
        g_calls.Th(0x0056af20, 0x00879ad0u, 2u, 0xb0754002u);
        return 0;
    }
    g_calls.Cd(0x0067b6d0, header, 1u, 0x12u, file);
    for (s32 row = 0; row < global<s32>(0x008bcbd4); ++row) {
        const s32 width = global<s32>(0x008bcbd0);
        g_calls.Cd(0x0067b6d0, at<u8 *>(grab, 4) + (global<s32>(0x008bcbd4) - row - 1) * width * 3, 3u, width, file);
    }
    g_calls.Cd(0x0067b620, file);
    return 1;
}

u32 __fastcall ScreenGrab_Save(u8 *grab) {
    if (at<u32>(grab, 0x8c)) {
        const u32 self = (u32)(uintptr_t)grab;
        if (!g_calls.Th(0x00561610, self)) return 0;
        if (!g_calls.Th(0x00561460, self)) return 0;
        if (!ScreenGrab_WriteTga(grab, nullptr, at<const char *>(grab, 0x8c))) return 0;
        g_calls.Cd(0x00579b80, at<u32>(grab, 4));
        at<u32>(grab, 4) = 0;
        at<u32>(grab, 0x8c) = 0;
    }
    return 1;
}
}
