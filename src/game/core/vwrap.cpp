#include "vwrap.h"

#include <cstdint>

#include "../render/device.h"
#include "../render/engineerrors.h"
#include "addrcalls.h"
#include "gameheap.h"
#include "liveobjects.h"
#include "misc.h"
#include "vcall.h"
#include "../ui/dialog7ca1e0.h"
#include "../ui/msgtags.h"
#include "../ui/thingbrowser.h"
#include "../ui/panel7cc218.h"
#include "../gfx/texcache.h"
#include "../render/spritegroups.h"
#include "vtables.h"
#include "../render/d3dsetup.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

AddrCalls g_calls;

}  // namespace

void VWrap_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" void __cdecl Pointer_BeginFlip() {
    const u32 pointer = global<u32>(0x007b4b84);
    if (pointer) g_calls.Th(0x005fbd40, pointer);   // AsyncPointer_BeginFlip
}

extern "C" {

u32 __fastcall Global8bba48_ClipAndTouch(u8 *self, void *) {
    typedef u32(__cdecl * Entry)(u32);
    typedef void(__cdecl * Box)(u32, s32 *, s32 *);
    typedef void(__cdecl * Give)(u32);
    for (u32 i = 0; i < 0x78; i++) {
        s32 count = (s32)reinterpret_cast<Entry>(at<u32>(self, 0u - 8))(i);
        if (count == 0) continue;
        s32 v[6];
        reinterpret_cast<Box>(at<u32>(self, 0u - 0x20))(i, &v[0], &v[3]);
        u8 code = 0x7c;
        if (v[0] != 0 || v[1] != 0 || v[2] != 0 || v[3] != 0 || v[4] != 0 || v[5] != 0) {
            float out[6];
            for (s32 j = 0; j < 6; j++) out[j] = (float)((double)v[j] * gSpritegroups_701b24);
            code = (u8)g_calls.Th(0x00561a30, global<u32>((u32)(uintptr_t)&gD3dsetup_879a20), (u32)(uintptr_t)out, global<u32>(0x0087b088));
        }
        at<u8>(self, i - 0x98) = code;
        const u32 records = reinterpret_cast<Entry>(at<u32>(self, 0u - 0x10))(i);
        if (records == 0) continue;
        for (u32 record = records + 0x10; count > 0; count--, record += 0x1c) {
            const u32 item = global<u32>(record);
            if (item != 0xffffffffu) g_calls.Th(0x0058c9f0, gTexcache_8c4b44, global<u32>(item), 0x3f000000u);
        }
        reinterpret_cast<Give>(at<u32>(self, 0u - 0xc))(records);
    }
    return 1;
}

u32 __fastcall Obj_CallSlot18(u8 *object, void *, u32 a) { return vcall<u32>(object, 0x18, a); }

u32 __fastcall Obj_PassToSlot10(u8 *object, void *, u8 *target) { return vcall<u32>(target, 0x10, object); }

u32 __fastcall Obj_RaiseIfSlot24Zero(u8 *object) {
    const u32 result = vcall<u32>(object, 0x24);
    if (result == 0)
        at<u32>(object, 4) = 1;
    return result;
}

u32 __fastcall Obj_ClearIfSlot24Set(u8 *object) {
    const u32 result = vcall<u32>(object, 0x24);
    if (result != 0)
        at<u32>(object, 4) = 0;
    return result;
}

u32 __stdcall Global7b05d8_CallSlot14(u32 a, u32 b) { return vcall<u32>(global<u8 *>(0x007b05d8), 0x14, a, b); }

void __cdecl Global7ca1e0_Release() {
    if (u8 *object = gDialog7ca1e0_7ca1e0)
        vcall<u32>(object, 8, 1u);
}

void __cdecl Global7ca1c8_Release() {
    if (u8 *object = global<u8 *>(0x007ca1c8))
        vcall<u32>(object, 8, 1u);
}

void __cdecl Global87b1d8_Delete() {
    if (u8 *object = global<u8 *>(0x0087b1d8)) {
        vcall<u32>(object, 0, 1u);
        global<u32>(0x0087b1d8) = 0;
    }
}

u32 __fastcall Obj_CallSub14Slot18(u8 *object, void *, u32 a) { return vcall<u32>(object + 0x14, 0x18, a); }

u32 __fastcall Obj_CallSub28Slot18(u8 *object, void *, u32 a) { return vcall<u32>(object + 0x28, 0x18, a); }

u32 __fastcall Obj_ForwardToMember14(u8 *object, void *, u8 *a) {
    if (u8 *member = at<u8 *>(object, 8))
        return vcall<u32>(member, 0x14, a);
    at<u32>(a, 8) = 0;
    return (u32)(uintptr_t)a;
}

u32 __fastcall Obj_ForwardToMember18(u8 *object, void *, u8 *a) {
    if (u8 *member = at<u8 *>(object, 8))
        return vcall<u32>(member, 0x18, a);
    at<u32>(a, 8) = 0;
    return (u32)(uintptr_t)a;
}

u32 __fastcall Obj_ForwardToMember10(u8 *object, void *, u8 *a) {
    if (u8 *member = at<u8 *>(object, 8))
        return vcall<u32>(member, 0x10, a);
    at<u32>(a, 8) = 0;
    return (u32)(uintptr_t)a;
}

u32 __cdecl Object_CallSlot14(u8 *object) { return vcall<u32>(object, 0x14); }

u32 __cdecl Object_FinishAndDelete(u8 *object) {
    vcall<u32>(object, 4);
    if (object)
        vcall<u32>(object, 0, 1u);
    return 0;
}

u32 __cdecl Object_CallSlot8Product(u32 a, u32 b, u32 c, u8 *object) { return vcall<u32>(object, 8, a, c * b); }

u32 __fastcall Obj_CallSlot10IfSized(u8 *object, void *, u8 *a) {
    if (at<s32>(a, 0x20) > 0 && at<s32>(a, 0x24) > 0)
        return vcall<u32>(object, 0x10, a);
    return 1;
}

u32 __fastcall Obj_ApplyIfShared(u8 *object, void *, u8 *a) {
    if (at<u32>(object, 0xc) & at<u32>(a, 4)) {
        if (!vcall<u32>(object, 0x10, a))
            return 0;
        at<u32>(a, 0) |= 0x10;
    }
    return 1;
}

u32 __fastcall Obj_CallSlot2cIfGlobal(u8 *object, void *, u32 a) {
    if (a == reinterpret_cast<u32 &>(gPanel7cc218_7c2658))
        return vcall<u32>(object, 0x2c);
    return a;
}

u32 __fastcall Obj_CallSlot2cIfGlobalIf10(u8 *object, void *, u32 a) {
    if (!at<u32>(object, 0x10)) return 0;
    return Obj_CallSlot2cIfGlobal(object, nullptr, a);
}

u32 __cdecl Global879acc_Delete() {
    if (u8 *object = global<u8 *>(0x00879acc)) {
        vcall<u32>(object, 0, 1u);
        global<u32>(0x00879acc) = 0;
        return 1;
    }
    return 0;
}

u32 __cdecl Global879acc_Create() {
    if (global<u32>(0x00879acc)) return 0;
    u8 *object = static_cast<u8 *>(GameHeap_Alloc(0x38));
    u8 *base = nullptr;
    if (object) {
        at<u32>(object, 0) = 0x00701410;
        Class701260_Ctor(object + 8);
        const s32 offset = at<s32>(at<u8 *>(object, 0), 4);
        at<u32>(object, offset) = (u32)(uintptr_t)Class7013c8_vtable;
        const s32 again = at<s32>(at<u8 *>(object, 0), 4);
        at<s32>(object, again - 4) = again - 8;
        at<u32>(object, at<s32>(at<u8 *>(object, 0), 4) + 0xc) = 0x1000;
        base = object + at<s32>(at<u8 *>(object, 0), 4);
    }
    global<u8 *>(0x00879acc) = base;
    if (!base) {
        EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 3, 0xb0754000);
        return 0;
    }
    vcall<u32>(base, 4);
    return 1;
}

u32 __cdecl Global879ac0_Create() {
    if (global<u32>(0x00879ac0)) return 0;
    u8 *object = static_cast<u8 *>(GameHeap_Alloc(0x5c));
    u8 *base = nullptr;
    if (object) {
        Class701378_Ctor(object, nullptr, 1);
        base = object + at<s32>(at<u8 *>(object, 0), 4);
    }
    global<u8 *>(0x00879ac0) = base;
    if (!base) {
        EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 3, 0xb0754000);
        return 0;
    }
    vcall<u32>(base, 4);
    return 1;
}

u32 __cdecl Global879ac0_Delete() {
    if (u8 *object = global<u8 *>(0x00879ac0)) {
        vcall<u32>(object, 0, 1u);
        global<u32>(0x00879ac0) = 0;
        return 1;
    }
    return 0;
}

void __cdecl Global7b4b84_Delete() {
    if (u8 *object = global<u8 *>(0x007b4b84))
        vcall<u32>(object, 0, 1u);
}

void __cdecl Globals7cc2fc_Release() {
    if (u8 *object = gThingbrowser_7cc2fc)
        vcall<u32>(object, 8, 1u);
    if (u8 *object = gThingbrowser_7cc300)
        vcall<u32>(object, 8, 1u);
    if (u8 *object = gThingbrowser_7cc304)
        vcall<u32>(object, 8, 1u);
}

void __cdecl Table7c26b0_ReleaseIdle() {
    for (u32 record = (u32)(uintptr_t)&gMsgtags_7c26b0; (s32)record < reinterpret_cast<uintptr_t>(&gMsgtags_7c3b00); record += 0x208) {
        if (global<u16>(record + 4) == 0 && global<u8 *>(record)) {
            vcall<u32>(global<u8 *>(record), 8, 1u);
            global<u32>(record) = 0;
        }
    }
}

u32 __cdecl Global8bba48_Create() {
    if (gVwrap_8bba48) return 0;
    u8 *object = static_cast<u8 *>(GameHeap_Alloc(0xa8));
    if (object) {
        at<u32>(object, 0) = 0x00701b40;
        at<u32>(object, 0xa4) = (u32)(uintptr_t)Class6fdce8_vtable;
        at<u32>(object, at<s32>(at<u8 *>(object, 0), 4)) = (u32)(uintptr_t)Class701b30_vtable;
        const s32 offset = at<s32>(at<u8 *>(object, 0), 4);
        at<s32>(object, offset - 4) = offset - 0xa4;
        at<u32>(object, 4) = 0xffffffff;
        at<u32>(object, 8) = 0xffffffff;
        at<u32>(object, 0x88) = 0x0051faa0;
        at<u32>(object, 0x8c) = 0x0051fd20;
        at<u32>(object, 0x90) = 0x0051f370;
        at<u32>(object, 0x94) = 0x0051ef30;
        at<u32>(object, 0x98) = 0x0051f2f0;
        at<u32>(object, 0x9c) = 0x0051f240;
        at<u32>(object, 0x84) = 0x0051f280;
        LiveObjects_Add(object + at<s32>(at<u8 *>(object, 0), 4));
    }
    reinterpret_cast<u8 * &>(gVwrap_8bba48) = object;
    if (!object) {
        EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 3, 0xb0754000);
        return 0;
    }
    return 1;
}

u32 __cdecl Global8bba48_DeleteVBase() {
    if (u8 *object = reinterpret_cast<u8 * &>(gVwrap_8bba48)) {
        u8 *base = object + at<u32>(at<u8 *>(object, 0), 4);
        vcall<u32>(base, 0, 1u);
        gVwrap_8bba48 = 0;
        return 1;
    }
    return 0;
}

void __fastcall ListNodes_Drain(u8 *node) {
    while (node) {
        u8 *object = at<u8 *>(node, 0);
        if (vcall<u32>(object, 8) != 1)
            return;
        vcall<u32>(at<u8 *>(node, 0), 4);
        node = at<u8 *>(node, 4);
    }
}

s32 __fastcall Refcounted_Release(u8 *field) {
    const s32 count = at<s32>(field, (u32)-0x4c) - 1;
    at<s32>(field, (u32)-0x4c) = count;
    if (count <= 0) {
        u8 *owner = field - 0x54;
        if (owner) {
            u8 *held = at<u8 *>(owner, 4);
            u8 *base = owner + at<u32>(held, 4) + 4;
            vcall<u32>(base, 0x1c, 1u);
        }
    }
    return count;
}

void __cdecl Global7ca1bc_Release() {
    if (u8 *object = global<u8 *>(0x007ca1bc))
        vcall<u32>(object, 8, 1u);
}

void __cdecl Global8023d8_Release() {
    if (!global<u32>(0x008023d0)) return;
    if (global<u32>(0x008023d4)) comcall<u32>(global<u8 *>(0x008023d8), 0x80, 0u);
    comcall<u32>(global<u8 *>(0x008023d8), 8);
    global<u32>(0x008023d0) = 0;
}

u32 __fastcall Obj_CallMember20Slot14(u8 *object, void *, u32 a) { return vcall<u32>(at<u8 *>(object, 0x20), 0x14, a); }

void __fastcall Obj_CallSlot14IfGlobalThing(u8 *object, void *, u32 handle) {
    if (!at<u32>(object, 0x10)) return;
    const u8 *const thing = global<const u8 *>(0x007cfb90 + 20 * (handle & 0xffff));
    if (thing && thing == gPanel7cc218_7c2658)
        vcall<u32>(object, 0x14);
}

u32 __fastcall Obj_KeyMessageToSlots(u8 *object, void *, u32 message, u32 key, u32) {
    if (message == 0x10005) {
        if (key == 0)
            return vcall<u32>(object, 0x40);
        return key;  // (what EAX held)
    }
    if (message != 0x1000b)
        return message - 0x1000b;  // (what EAX held)
    if (key == 0xd)
        return vcall<u32>(object, 0x40);
    if (key == 0x2500)
        return vcall<u32>(object, 0x3c);
    if (key == 0x2700)
        return vcall<u32>(object, 0x38);
    return key;  // (what EAX held)
}

void __fastcall Obj7b51f0_PassThingTo7cc218(u8 *, void *, u32 handle) {
    Obj_CallSlot14IfGlobalThing(reinterpret_cast<u8 *>(0x007cc218), nullptr, handle);
}

}  // extern "C"
