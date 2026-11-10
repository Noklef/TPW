#include "vtables.h"

#include <cstdint>

#include "addrcalls.h"
#include "../render/device.h"
#include "../world/thingmsg.h"
#include "../input/devicefilter.h"
#include "refcounted.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

AddrCalls g_dtorCalls;

}  // namespace

void Vtables_SetDtorCalls(const AddrCall *table, u32 count) { g_dtorCalls.Set(table, count); }

extern "C" {

void __fastcall Vtbl703748_Set(u8 *object) { at<u32>(object, 0) = (u32)(uintptr_t)KeyFilter_vtable_703748; }

void __fastcall Vtbl703770_Set(u8 *object) { at<u32>(object, 0) = 0x00703770; }

u32 __fastcall VirtualBase_SetVtbl7099dc(u8 *object) {
    const u32 vtable = at<u32>(object, (u32)-8);
    at<u32>(object, at<u32>(reinterpret_cast<const u8 *>(vtable), 4) - 8) = 0x007099dc;
    return vtable;
}

u32 __fastcall VirtualBase_SetVtbl70995c(u8 *object) {
    const u32 vtable = at<u32>(object, (u32)-4);
    at<u32>(object, at<u32>(reinterpret_cast<const u8 *>(vtable), 4) - 4) = 0x0070995c;
    return vtable;
}

u32 __fastcall Vtbl7099ec_SetVirtual(u8 *sub) {
    const u32 vtable = at<u32>(sub, (u32)-4);
    at<u32>(sub, at<u32>(reinterpret_cast<const u8 *>(vtable), 4) - 4) = 0x007099ec;
    return vtable;
}

void __fastcall Vtbl6fd800_Set(u8 *object) { at<u32>(object, 0) = (u32)(uintptr_t)ThingMsg11_vtable; }

void __fastcall Vtbl6fdce8_At60_Set(u8 *object) { at<u32>(object, 0x60) = 0x006fdce8; }

void __fastcall Vtbl6fdce8_At50_Set(u8 *object) { at<u32>(object, 0x50) = 0x006fdce8; }

u32 __fastcall VirtualBase_SetVtbl70994c(u8 *object) {
    const u32 vtable = at<u32>(object, (u32)-8);
    at<u32>(object, at<u32>(reinterpret_cast<const u8 *>(vtable), 4) - 8) = 0x0070994c;
    return vtable;
}

void __fastcall Vtbl6fd6c8_Set_405520(u8 *o) {
    at<u32>(o, 0) = 0x006fd6c8;
}

void __fastcall Vtbl6fdce8_Set_429b50(u8 *o) {
    at<u32>(o, 0) = 0x006fdce8;
}

void __fastcall Vtbl6fd810_Set_4a8bf0(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)ThingMsg13_vtable;
}

void __fastcall Vtbl6fefb8_Set_4bc420(u8 *o) {
    at<u32>(o, 0) = 0x006fefb8;
}

u8 *__fastcall Vtbl6fefb8_Delete(u8 *object, void *, u32 flags) {
    Vtbl6fefb8_Set_4bc420(object);
    if (flags & 1) g_dtorCalls.Cd(0x0067acb0, (u32)(uintptr_t)object);
    return object;
}

void __fastcall Vtbl6fd838_Set_4c75f0(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)ThingMsg1a_vtable;
}

void __fastcall Vtbl6fd830_Set_4d19f0(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)ThingMsg19_vtable;
}

void __fastcall Vtbl6fd7b0_Set_4d5260(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)Class6fd7b0_vtable;
}

void __fastcall Vtbl6fd7c0_Set_4db7c0(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)ThingMsg06_vtable;
}

void __fastcall Vtbl6fd7e0_Set_4dd310(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)ThingMsg0a_vtable;
}

void __fastcall Vtbl6fd7c8_Set_4e11f0(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)ThingMsg07_vtable;
}

void __fastcall Vtbl6fd7d0_Set_4e1200(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)ThingMsg08_vtable;
}

void __fastcall Vtbl6fd7d8_Set_4e1210(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)ThingMsg09_vtable;
}

void __fastcall Vtbl700620_Set_4e5b90(u8 *o) {
    at<u32>(o, 0) = 0x00700620;
}

void __fastcall Vtbl6fd7e8_Set_4f8570(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)ThingMsg0b_vtable;
}

void __fastcall Vtbl6fd7f0_Set_4f8580(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)ThingMsg0c_vtable;
}

void __fastcall Vtbl6fd7f8_Set_4f8590(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)ThingMsg0d_vtable;
}

void __fastcall Vtbl6fd840_Set_4fb320(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)ThingMsg1c_vtable;
}

void __fastcall Vtbl7007a8_Set_4ff4e0(u8 *o) {
    at<u32>(o, 0) = 0x007007a8;
}

void __fastcall Vtbl6fd808_Set_503fe0(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)ThingMsg12_vtable;
}

void __fastcall Vtbl700868_Set_506190(u8 *o) {
    at<u32>(o, 0) = 0x00700868;
}

void __fastcall Vtbl6fd820_Set_509350(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)ThingMsg17_vtable;
}

void __fastcall Vtbl6fd848_Set_50b970(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)ThingMsg1b_vtable;
}

void __fastcall Vtbl6fd7a0_Set_512c30(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)ThingMsg02_vtable;
}

void __fastcall Vtbl6fd7a8_Set_512c40(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)ThingMsg01_vtable;
}

void __fastcall Vtbl701260_Set_565230(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)Class701260_vtable;
}

void __fastcall Vtbl7026e8_Set_591970(u8 *o) {
    at<u32>(o, 0) = 0x007026e8;
}

void __fastcall Vtbl6fd828_Set_59b860(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)ThingMsg18_vtable;
}

void __fastcall Vtbl709b44_Set_6a9180(u8 *o) {
    at<u32>(o, 0) = 0x00709b44;
}

void __fastcall Vtbl70a0a8_Set_6ad000(u8 *o) {
    at<u32>(o, 0) = 0x0070a0a8;
}

void __fastcall Vtbl70a078_Set_6add30(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)Class70a078_vtable;
}

void __fastcall Vtbl70a078_Set_6b4a50(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)Class70a078_vtable;
}

void __fastcall Vtbl70a258_Set_6b4a90(u8 *o) {
    at<u32>(o, 0) = 0x0070a258;
}

void __fastcall Vtbl70a778_Set_6c7090(u8 *o) {
    at<u32>(o, 0) = 0x0070a778;
}
}  // extern "C"
