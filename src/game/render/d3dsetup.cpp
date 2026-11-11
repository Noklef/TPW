// The Direct3D setup: see d3dsetup.h.
#include "d3dsetup.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "../core/vobj3.h"
#include "../math/bits.h"
#include "../math/x87.h"
#include "d3dresult.h"
#include "d3dviewport.h"
#include "../frontend/lobbysky.h"
#include "ddopen.h"
#include "../core/staticobjects.h"

using namespace d3dresult;

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const u32 kVirtualBase = 0x228;

u32 debugFlags() { return global<u32>(0x008bd508); }

template <typename... A> void logged(u32 format, A... args) {
    if (!(debugFlags() & 0x1000000)) return;
    char text[0x100];
    Crt().sprintf(text, reinterpret_cast<const char *>((uintptr_t)format), args...);
    EngineDebug_Log(text);
}

void noted(u32 text) {
    if (debugFlags() & 0x2000000) EngineDebug_LogFallback(reinterpret_cast<const char *>((uintptr_t)text));
}

void CopyFormat(u32 *to, const u32 *from) {
    for (u32 i = 0; i < 8; ++i)
        to[i] = from[i];
}

void ReleaseAt(u8 *self, u32 offset) {
    if (u8 *const object = at<u8 *>(self, offset)) {
        comcall<u32>(object, 8);
        at<u32>(self, offset) = 0;
    }
}

void CloseAll(u8 *self) {
    Obj_DeleteAttached(global<u8 *>((u32)(uintptr_t)&gD3dsetup_879a20 + 0x4));
    ReleaseAt(self, 0x10);
    ReleaseAt(self, 0xc);
    ReleaseAt(self, 8);
    ReleaseAt(self, 4);
}

void ZeroDwords(u32 *to, u32 count) {
    for (u32 i = 0; i < count; ++i)
        to[i] = 0;
}

bool Build(u8 *self, u32 flags) {
    u8 *const draw = at<u8 *>(global<u8 *>((u32)(uintptr_t)&gD3dsetup_879a20 + 0x4), 8);
    if (at<u32>(global<u8 *>((u32)(uintptr_t)&gD3dsetup_879a20 + 0x4), 0x428) == 0x121a && at<u32>(global<u8 *>((u32)(uintptr_t)&gD3dsetup_879a20 + 0x4), 0x42c) == 1) {
        noted((u32)&gText_D3dsetup_7689d0);
        return false;
    }
    logged((u32)&gText_D3dsetup_7689b0);
    noted((u32)&gText_D3dsetup_768998);
    keep(comcall<u32>(draw, 0, 0x00707520u, self + 4));
    if (result()) return false;
    if (flags & 0x10) {
        logged((u32)&gText_D3dsetup_768988);
        u32 format[8];
        ZeroDwords(format, 8);
        format[1] = 0x400;
        keep(comcall<u32>(at<u8 *>(self, 4), 0x28, self + 0x18, 0x00573660u, format));
        if (result() || format[0] != 0x20) return false;
        u32 description[0x1f];
        ZeroDwords(description, 0x1f);
        description[0] = 0x7c;
        description[1] = 0x1007;
        description[2] = global<u32>((u32)(uintptr_t)&gView_8bcbc8 + 0xc);
        description[3] = global<u32>((u32)(uintptr_t)&gView_8bcbc8 + 0x8);
        CopyFormat(description + 0x12, format);
        description[0x1a] = 0x24000;
        logged((u32)&gText_D3dsetup_76896c, format[3]);
        u32 surface;
        keep(comcall<u32>(draw, 0x18, description, &surface, 0u));
        if (result()) return false;
        Obj_AddAttached(global<u8 *>((u32)(uintptr_t)&gD3dsetup_879a20 + 0x4), nullptr, surface);
        at<u32>(self, 0x220) |= 0x10;
    } else {
        logged((u32)&gText_D3dsetup_768958);
        at<u32>(self, 0x220) &= ~0x10u;
    }
    keep(comcall<u32>(at<u8 *>(self, 4), 0x20, self + 0x18, at<u32>(global<u8 *>((u32)(uintptr_t)&gD3dsetup_879a20 + 0x4), 0x14), self + 8, 0u));
    if (result()) return false;
    ZeroDwords(&at<u32>(self, 0x28), 0x3f);
    ZeroDwords(&at<u32>(self, 0x124), 0x3f);
    at<u32>(self, 0x28) = 0xfc;
    at<u32>(self, 0x124) = 0xfc;
    keep(comcall<u32>(at<u8 *>(self, 8), 0xc, self + 0x28, self + 0x124));
    if (result()) return false;
    if (!D3dSetup_CreateViewport(self)) return false;
    keep(comcall<u32>(at<u8 *>(self, 8), 0x20, 0x00573060u, self));
    if (result()) return false;
    if (!global<u32>((u32)(uintptr_t)&gTexSettings_8bd550 + 0x48)) {
        noted((u32)&gText_D3dsetup_768910);
        return false;
    }
    keep(comcall<u32>(at<u8 *>(self, 4), 0x14, self + 0x10, 0u));
    if (result()) return false;
    u32 material[0x14];
    ZeroDwords(material, 0x14);
    material[0] = 0x50;
    for (u32 i = 4; i < 9; ++i)
        material[i] = 0x3f800000;
    material[0x13] = 1;
    keep(comcall<u32>(at<u8 *>(self, 0x10), 0xc, material));
    if (result()) return false;
    keep(comcall<u32>(at<u8 *>(self, 0x10), 0x14, at<u8 *>(self, 8), self + 0x14));
    if (result()) return false;
    keep(comcall<u32>(at<u8 *>(self, 0xc), 0x20, at<u32>(self, 0x14)));
    if (result()) return false;
    return vcall<u32>(self + at<u32>(at<u8 *>(self, 0), 4), 0) != 0;
}

}  // namespace

extern "C" {

u32 __fastcall D3dSetup_Open(u8 *self, void *, u32 flags) {
    if (Build(self, flags)) {
        u8 *const device = at<u8 *>(self, 8);
        if (at<u32>(self, 0x220) & 0x10) {
            comcall<u32>(device, 0x58, 7u, 1u);
            comcall<u32>(device, 0x58, 0x17u, 4u);
            comcall<u32>(device, 0x58, 0xeu, 1u);
        } else {
            comcall<u32>(device, 0x58, 7u, 0u);
            comcall<u32>(device, 0x58, 0xeu, 0u);
        }
        noted((u32)&gText_D3dSetup_Open_7688ec);
        return 1;
    }
    if (debugFlags() & 0x2000000) {
        char text[0x100];
        Crt().sprintf(text, reinterpret_cast<const char *>((uintptr_t)&gText_Ddopen_7661cc), result());
        EngineDebug_LogFallback(text);
    }
    CloseAll(self);
    return 0;
}

u32 __fastcall D3dSetup_Close(u8 *base) {
    CloseAll(base - kVirtualBase);
    return 1;
}

u32 __fastcall D3dSetup_Reopen(u8 *base) {
    return D3dSetup_Open(base - kVirtualBase, nullptr, at<u32>(base - 8, 0) & 0x10);
}

u32 __fastcall D3dSetup_Clear(u8 *base) {
    const u32 caps = global<u32>((u32)(uintptr_t)&gView_8bcbc8);
    const auto ftol = [](X87Reg x) { return (s32)(long long)x.v; };
    s32 rect[4];
    rect[0] = ftol(X87Reg(global<float>((u32)(uintptr_t)&gView_8bcbc8 + 0x30)) - global<float>((u32)(uintptr_t)&gView_8bcbc8 + 0x38));
    rect[1] = ftol(X87Reg(global<float>((u32)(uintptr_t)&gView_8bcbc8 + 0x34)) - global<float>((u32)(uintptr_t)&gView_8bcbc8 + 0x3c));
    rect[2] = rect[0] + global<s32>((u32)(uintptr_t)&gView_8bcbc8 + 0x20);
    rect[3] = rect[1] + global<s32>((u32)(uintptr_t)&gView_8bcbc8 + 0x24);
    u32 targets = (caps & 0x80000000u) ? 0 : 1;
    if (at<u32>(base - 8, 0) & 0x10) targets |= 2;
    const u32 colour = (caps & 0x4002000) ? global<u32>((u32)(uintptr_t)&gView_8bcbc8 + 0x8b4) : 0;
    keep(comcall<u32>(at<u8 *>(base - kVirtualBase, 0xc), 0x50, 1u, rect, targets, colour, 0x3f800000u, 0u));
    return pushed(2);
}

u32 __cdecl D3dSetup_Teardown() {
    u8 *const self = global<u8 *>(0x00879da8);
    if (!self) return 0;
    global<u32>(0x008bd52c) = 0;
    CloseAll(self);
    GameHeap_Free(global<u8 *>(0x00879da8));
    global<u32>(0x00879da8) = 0;
    return 1;
}

u32 __stdcall D3dSetup_PickFormat(u32, const u32 *format, u32) {
    const s16 *entry = reinterpret_cast<const s16 *>((uintptr_t)&gD3dSetup_PickFormat_7015f8);
    for (s32 i = 0; i < 4; ++i, entry += 4) {
        if ((s32)Bits_Count(format[4]) != entry[1] || (s32)Bits_Count(format[5]) != entry[2] || (s32)Bits_Count(format[6]) != entry[3] || (s32)Bits_Count(format[7]) != entry[0]) continue;
        CopyFormat(reinterpret_cast<u32 *>((uintptr_t)&gTexSettings_8bd550 + 0x8) + i * 8, format);
        logged((u32)&gText_D3dSetup_PickFormat_7688c4, i, (s32)entry[0], (s32)entry[1], (s32)entry[2], (s32)entry[3]);
    }
    return 1;
}

u32 __stdcall D3dSetup_OnFormat(const u32 *format, u32 context) { return D3dSetup_PickFormat(context, format, 0); }

u32 __stdcall D3dSetup_OnZFormat(const u32 *format, u32 *wanted) {
    if (!format || !wanted) return 0;
    if (format[1] != wanted[1]) return 1;
    CopyFormat(wanted, format);
    return format[3] != 0x10;
}
}
