#include "vwrap.h"

#include <cstdint>

#include "../render/device.h"
#include "../render/engineerrors.h"
#include "gameheap.h"
#include "liveobjects.h"
#include "misc.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

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
    if (u8 *object = global<u8 *>(0x007ca1e0))
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
    if (a == global<u32>(0x007c2658))
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
        at<u32>(object, offset) = 0x007013c8;
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
    if (u8 *object = global<u8 *>(0x007cc2fc))
        vcall<u32>(object, 8, 1u);
    if (u8 *object = global<u8 *>(0x007cc300))
        vcall<u32>(object, 8, 1u);
    if (u8 *object = global<u8 *>(0x007cc304))
        vcall<u32>(object, 8, 1u);
}

void __cdecl Table7c26b0_ReleaseIdle() {
    for (u32 record = 0x007c26b0; (s32)record < 0x007c3b00; record += 0x208) {
        if (global<u16>(record + 4) == 0 && global<u8 *>(record)) {
            vcall<u32>(global<u8 *>(record), 8, 1u);
            global<u32>(record) = 0;
        }
    }
}

u32 __cdecl Global8bba48_Create() {
    if (global<u32>(0x008bba48)) return 0;
    u8 *object = static_cast<u8 *>(GameHeap_Alloc(0xa8));
    if (object) {
        at<u32>(object, 0) = 0x00701b40;
        at<u32>(object, 0xa4) = 0x006fdce8;
        at<u32>(object, at<s32>(at<u8 *>(object, 0), 4)) = 0x00701b30;
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
    global<u8 *>(0x008bba48) = object;
    if (!object) {
        EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 3, 0xb0754000);
        return 0;
    }
    return 1;
}

u32 __cdecl Global8bba48_DeleteVBase() {
    if (u8 *object = global<u8 *>(0x008bba48)) {
        u8 *base = object + at<u32>(at<u8 *>(object, 0), 4);
        vcall<u32>(base, 0, 1u);
        global<u32>(0x008bba48) = 0;
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

u32 __fastcall Obj_CallMember20Slot14(u8 *object, void *, u32 a) { return vcall<u32>(at<u8 *>(object, 0x20), 0x14, a); }

void __fastcall Obj_CallSlot14IfGlobalThing(u8 *object, void *, u32 handle) {
    if (!at<u32>(object, 0x10)) return;
    const u8 *const thing = global<const u8 *>(0x007cfb90 + 20 * (handle & 0xffff));
    if (thing && thing == global<const u8 *>(0x007c2658))
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
