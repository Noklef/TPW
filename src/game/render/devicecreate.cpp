#include "devicecreate.h"

#include <cstdint>
#include <initializer_list>

#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "batch.h"
#include "../core/staticobjects.h"

namespace {

AddrCalls g_calls;

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
u32 &global(u32 address) { return *reinterpret_cast<u32 *>((uintptr_t)address); }
u32 address_of(const void *p) { return (u32)(uintptr_t)p; }

const u32 kD3d = 0x00879da8, kSoft = 0x00879dac, kVirtualBase = 0x008bd52c;

// The table's second dword: the virtual base's place in the object.
u32 base_offset(const u8 *object) { return at<u32>(at<const u8 *>(object, 0), 4); }

void push_error(u32 code) { g_calls.Th(0x0056af20, 0x00879ad0, 3u, code); }

void note_virtual_base(u32 object) {
    global(kVirtualBase) = object ? object + base_offset(reinterpret_cast<const u8 *>((uintptr_t)object)) : 0;
}

// A constructor with a virtual base: its tables set through the table's own offset.
void set_tables(u8 *object, u32 table, u32 baseTable, u32 place) {
    at<u32>(object, 0) = table;
    at<u32>(object, place) = 0x00701650;
    at<u32>(object, base_offset(object)) = baseTable;
    const u32 offset = base_offset(object);
    at<u32>(object, offset - 4) = offset - place;
}

u8 *new_soft() {
    u8 *const object = static_cast<u8 *>(GameHeap_Alloc(0x18));
    if (object) {
        set_tables(object, 0x007016a8, (u32)(uintptr_t)Class701688_vtable, 0x14);
        at<u32>(object, 4) = 0;
        at<u32>(object, 8) = 0;
    }
    global(kSoft) = address_of(object);
    return object;
}

// The raster interface of the DirectDraw object's +0x14: its first slot (QueryInterface) with 0x707210.
u32 raster_interface() {
    const u32 surface = at<u32>(reinterpret_cast<const void *>((uintptr_t)global(0x00879a24)), 0x14);
    u32 result;
    comcall<u32>(reinterpret_cast<void *>((uintptr_t)surface), 0, 0x00707210u, &result);
    return result;
}

}  // namespace

void DeviceCreate_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u32 __cdecl D3dSetup_Create(u32 kind) {
    if (global(kD3d))
        return 0;
    u8 *const object = static_cast<u8 *>(GameHeap_Alloc(0x22c));
    if (object) {
        set_tables(object, 0x00701670, (u32)(uintptr_t)Class701630_vtable, 0x228);
        const u32 *const guid = reinterpret_cast<const u32 *>(0x00707550);
        for (u32 i = 0; i < 4; ++i)
            at<u32>(object, 0x18 + 4 * i) = guid[i];
        at<u32>(object, 0x220) = 0;
        at<u32>(object, 4) = 0;
        at<u32>(object, 8) = 0;
        at<u32>(object, 0xc) = 0;
        at<u32>(object, 0x10) = 0;
    }
    global(kD3d) = address_of(object);
    if (!object) {
        push_error(0xb0754000);
        return 0;
    }
    const u32 opened = g_calls.Th(0x00573080, address_of(object), kind == 1 ? 0x10u : 0u);
    if (opened) {
        note_virtual_base(global(kD3d));
        return 1;
    }
    GameHeap_Free(reinterpret_cast<void *>((uintptr_t)global(kD3d)));
    global(kD3d) = 0;
    push_error(0xb0754005);
    return 0;
}

u32 __cdecl SoftSetup_Create(u32 kind) {
    if (global(kSoft))
        return 0;
    u8 *const object = new_soft();
    if (!object) {
        push_error(0xb0754000);
        return 0;
    }
    if (g_calls.Th(0x00573b90, address_of(object), kind)) {
        note_virtual_base(global(kSoft));
        return 1;
    }
    GameHeap_Free(reinterpret_cast<void *>((uintptr_t)global(kSoft)));   // sic: [0x879dac] left set
    push_error(0xb0754010);
    return 0;
}

u32 __cdecl SoftSetup_CreateSquare() {
    if (global(kSoft))
        return 0;
    u8 *const object = new_soft();
    if (!object) {
        push_error(0xb0754000);
        return 0;
    }
    const u32 raster = raster_interface();
    const u32 side = global((u32)(uintptr_t)&gView_8bcbc8 + 0x8e4);
    at<u32>(object, 8) = g_calls.Cd(0x00678e70, raster, 0u, side, side);
    const u32 now = global((u32)(uintptr_t)&gView_8bcbc8 + 0x8e4);
    global((u32)&gSoftSetup_fbe014) = 0;
    global((u32)&gSoftSetup_fbe018) = 0;
    global((u32)&gDevicecreate_fbe034) = 0;
    global((u32)&gSoftSetup_fbe01c) = now - 1;
    global((u32)&gSoftSetup_fbe020) = now - 1;
    global(0x00fbe024) = at<u32>(object, 8);
    global(0x00fbe02c) = now * 4;
    at<u32>(object, 0xc) = (now * now) << 2;
    return 1;
}

u32 __cdecl SoftSetup_Destroy() {
    u8 *const object = reinterpret_cast<u8 *>((uintptr_t)global(kSoft));
    if (!object)
        return 0;
    for (const u32 offset : {4u, 8u})
        if (const u32 block = at<u32>(object, offset)) {
            g_calls.Cd(0x00579b80, block);
            at<u32>(object, offset) = 0;
        }
    g_calls.Cd(0x00679270);
    GameHeap_Free(reinterpret_cast<void *>((uintptr_t)global(kSoft)));
    global(kVirtualBase) = 0;
    global(kSoft) = 0;
    return 1;
}

u32 __fastcall SoftSetup_Open(u8 *self, void *, u32 kind) {
    if (global(0x008bd508) & 0x2000000)
        g_calls.Cd(0x005645e0, "\n\nInitializing Software renderer");
    const u32 raster = raster_interface();
    const u32 width = global((u32)(uintptr_t)&gView_8bcbc8 + 0x8), height = global((u32)(uintptr_t)&gView_8bcbc8 + 0xc);
    at<u32>(self, 8) = g_calls.Cd(0x00678e70, raster, kind ? address_of(self + 4) : 0u, width, height);
    at<u32>(self, 0xc) = (global((u32)(uintptr_t)&gView_8bcbc8 + 0x8) * global((u32)(uintptr_t)&gView_8bcbc8 + 0xc)) << 2;
    const u32 w = global((u32)(uintptr_t)&gView_8bcbc8 + 0x8);
    global((u32)&gSoftSetup_fbe014) = 0;
    global((u32)&gSoftSetup_fbe01c) = w - 1;
    global((u32)&gSoftSetup_fbe018) = 0;
    global((u32)&gSoftSetup_fbe020) = global((u32)(uintptr_t)&gView_8bcbc8 + 0xc) - 1;
    global((u32)&gDevicecreate_fbe034) = 0;
    global(0x00fbe024) = at<u32>(self, 8);
    global(0x00fbe02c) = w * 4;
    global((u32)&gDevicecreate_fbe028) = at<u32>(self, 4);
    global((u32)&gDevicecreate_fbe030) = w * 2;
    if (global(0x008bd508) & 0x2000000)
        g_calls.Cd(0x005645e0, "Software renderer initialized succesfully");   // sic
    return 1;
}
}
