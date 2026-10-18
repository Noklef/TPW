#include "vdel.h"

#include <cstdint>

#include "../gfx/screensurfaces.h"
#include "buffers.h"
#include "gameheap.h"
#include "ptrlist.h"
#include "thunks.h"
#include "vdtor.h"
#include "vobj2.h"
#include "vtables.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *Finish(u8 *object, u32 flags) {
    if (flags & 1)
        GameHeap_Free(object);
    return object;
}

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *VBase_Deleter(u8 *object, u32 flags, u32 vtable) {
    u8 *base = object - 8;
    at<u32>(base, at<u32>(at<u8 *>(base, 0), 4)) = vtable;
    const u32 offset = at<u32>(at<u8 *>(base, 0), 4);
    at<u32>(base, offset - 4) = offset - 8;
    at<u32>(base, 8) = 0x00701940;
    if (flags & 1)
        GameHeap_Free(base);
    return base;
}

}  // namespace

extern "C" {

u8 *__fastcall Class703780_ScalarDtorA(u8 *object, void *, u32 flags) {
    Class703780_DtorA(object);
    return Finish(object, flags);
}

u8 *__fastcall Class703780_ScalarDtorB(u8 *object, void *, u32 flags) {
    Class703780_DtorB(object);
    return Finish(object, flags);
}

u8 *__fastcall Class703780_ScalarDtorC(u8 *object, void *, u32 flags) {
    Class703780_DtorC(object);
    return Finish(object, flags);
}

u8 *__fastcall Class70293c_ScalarDtor(u8 *object, void *, u32 flags) {
    Class70293c_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class702944_ScalarDtor(u8 *object, void *, u32 flags) {
    Class702944_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class702b08_ScalarDtor(u8 *object, void *, u32 flags) {
    Class702b08_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class702cb0_ScalarDtor(u8 *object, void *, u32 flags) {
    Class702cb0_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class702d78_ScalarDtor(u8 *object, void *, u32 flags) {
    Class702d78_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class702880_ScalarDtor(u8 *object, void *, u32 flags) {
    Class702880_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class702890_ScalarDtor(u8 *object, void *, u32 flags) {
    Class702890_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class7028d0_ScalarDtor(u8 *object, void *, u32 flags) {
    Class7028d0_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class7028f8_ScalarDtor(u8 *object, void *, u32 flags) {
    Class7028f8_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class702920_ScalarDtor(u8 *object, void *, u32 flags) {
    Class702920_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class702da0_ScalarDtor(u8 *object, void *, u32 flags) {
    Class702da0_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class702e70_ScalarDtor(u8 *object, void *, u32 flags) {
    Class702e70_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class702e20_ScalarDtor(u8 *object, void *, u32 flags) {
    Class702e20_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class7099a4_ScalarDtor(u8 *object, void *, u32 flags) {
    Class7099a4_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Counted2_ScalarDtor(u8 *object, void *, u32 flags) {
    Counted2_Clear(object);
    return Finish(object, flags);
}

u8 *__fastcall Class702938_ScalarDtor(u8 *object, void *, u32 flags) {
    Class702938_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class702940_ScalarDtor(u8 *object, void *, u32 flags) {
    Class702940_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class702948_ScalarDtor(u8 *object, void *, u32 flags) {
    Class702948_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class709914_ScalarDtor(u8 *object, void *, u32 flags) {
    Class709914_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class702a90_Ctor(u8 *object) {
    Class7029d0_Ctor(object);
    at<u32>(object, 0) = 0x00702a90;
    at<u32>(object, 0x20) = 0;
    return object;
}

u32 __fastcall Class702a90_Release(u8 *object) {
    at<u32>(object, 0) = 0x00702a90;
    return Class7029d0_Release(object);
}

u8 *__fastcall Class702a90_ScalarDtor(u8 *object, void *, u32 flags) {
    Class702a90_Release(object);
    return Finish(object, flags);
}

u8 *__fastcall Class702ec0_ScalarDtor(u8 *object, void *, u32 flags) {
    Class702ec0_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class702dd0_ScalarDtor(u8 *object, void *, u32 flags) {
    Class702dd0_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Deleter_Minus2c_569c80(u8 *object, void *, u32 flags) {
    Vtbl701260_Set_565230(object);
    if (flags & 1)
        GameHeap_Free(object - 0x2c);
    return object - 0x2c;
}

u8 *__fastcall Class703088_ScalarDtor(u8 *object, void *, u32 flags) {
    at<u32>(object, 0) = 0x00703088;
    u8 *const list = global<u8 *>(0x00fa6e38);
    u8 *node = at<u8 *>(list, 4);
    while (node && at<u8 *>(node, 0) != object)
        node = at<u8 *>(node, 4);
    if (node) {
        if (node == at<u8 *>(list, 4))
            at<u8 *>(list, 4) = at<u8 *>(node, 4);
        else
            at<u8 *>(at<u8 *>(node, 8), 4) = at<u8 *>(node, 4);
        if (node == at<u8 *>(list, 8))
            at<u8 *>(list, 8) = at<u8 *>(node, 8);
        else
            at<u8 *>(at<u8 *>(node, 4), 8) = at<u8 *>(node, 8);
        at<u32>(list, 0xc) = at<u32>(list, 0xc) - 1;
        GameHeap_Free(node);
    }
    PtrArray_RemoveAndFree(reinterpret_cast<u8 *>(0x00879ce0), nullptr, at<u32>(object, 0x28));
    if (flags & 1)
        GameHeap_Free(object);
    return object;
}

u8 *__fastcall Deleter_Minus60_42a090(u8 *object, void *, u32 flags) {
    at<u32>(object, 0) = 0x006fdce8;
    if (flags & 1)
        GameHeap_Free(object - 0x60);
    return object - 0x60;
}

u8 *__fastcall Deleter_Minus8_458970(u8 *object, void *, u32 flags) {
    at<u32>(object, 0x0) = 0x006fdce8;
    if (flags & 1)
        GameHeap_Free(object - 0x8);
    return object - 0x8;
}

u8 *__fastcall Deleter_Minus8_56a660(u8 *object, void *, u32 flags) {
    Vtbl701260_Set_565230(object);
    if (flags & 1)
        GameHeap_Free(object - 8);
    return object - 8;
}

u8 *__fastcall VBase_Deleter_7018d0(u8 *object, void *, u32 flags) { return VBase_Deleter(object, flags, 0x007018d0); }

u8 *__fastcall VBase_Deleter_701920(u8 *object, void *, u32 flags) { return VBase_Deleter(object, flags, 0x00701920); }

u8 *__fastcall VBase_Deleter_7018f8(u8 *object, void *, u32 flags) { return VBase_Deleter(object, flags, 0x007018f8); }

u8 * __fastcall ScalarDelete6fd7a0_40f270(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd7a0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd7a8_40f2b0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd7a8; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd7b0_40f2e0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd7b0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd7c0_40f360(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd7c0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd7c8_40f3b0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd7c8; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd7d0_40f400(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd7d0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd7d8_40f450(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd7d8; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd7e0_40f4a0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd7e0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd7e8_40f4e0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd7e8; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd7f0_40f520(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd7f0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd7f8_40f560(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd7f8; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd800_40f5c0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd800; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd808_40f610(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd808; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd810_40f660(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd810; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd818_40f6b0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd818; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd820_40f720(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd820; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd828_40f780(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd828; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd830_40f820(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd830; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd838_40f890(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd838; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd840_40f8e0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd840; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fd848_40f930(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fd848; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fdce8_429b60(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fdce8; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fef90_47e3a0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fef90; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete6fefb8_47e730(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x006fefb8; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete709b44_4f18b0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x00709b44; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete7007a8_4ff500(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x007007a8; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete700868_5061b0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x00700868; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete701178_564710(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x00701178; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete701260_565210(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x00701260; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete702198_58b310(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x00702198; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete7026e8_591980(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x007026e8; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete7029fc_5d5c30(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x007029fc; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete703748_617930(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x00703748; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDelete703770_617ae0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x00703770; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia407c30_407c70(u8 *o, void *, u32 v0) {
    ListDtor6fd708_407c30(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 *__fastcall ScalarDeleteList6fd708_409cf0(u8 *o, void *, u32 flags) {
    ListDtor6fd708_407c30(o);  // (inlined in the original)
    if (flags & 0x1) GameHeap_Free(o);
    return o;
}

u8 *__fastcall ScalarDeleteList6fdb10_421800(u8 *o, void *, u32 flags) {
    ListDtor6fdb10_421640(o);  // (inlined too)
    if (flags & 1) GameHeap_Free(o);
    return o;
}

u8 * __fastcall ScalarDeleteVia41c9a0_41c980(u8 *o, void *, u32 v0) {
    ListDtor6fd9e4_41c9a0(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia41eb50_41eb30(u8 *o, void *, u32 v0) {
    ListDtor6fdb08_41eb50(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia421640_421620(u8 *o, void *, u32 v0) {
    ListDtor6fdb10_421640(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia492060_492040(u8 *o, void *, u32 v0) {
    Wrap_Fn_669fee_492060(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia4e5b90_4e5bc0(u8 *o, void *, u32 v0) {
    Vtbl700620_Set_4e5b90(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5193d0_5193b0(u8 *o, void *, u32 v0) {
    ListDtor700aa0_5193d0(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5ac550_5ac530(u8 *o, void *, u32 v0) {
    ListDtor7027fc_5ac550(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5b58c0_5b5900(u8 *o, void *, u32 v0) {
    ListDtor7028a0_5b58c0(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5b9a80_5b9a40(u8 *o, void *, u32 v0) {
    ListDtor7028b0_5b9a80(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5b9ac0_5b9a60(u8 *o, void *, u32 v0) {
    ListDtor7028b4_5b9ac0(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5beae0_5bea80(u8 *o, void *, u32 v0) {
    ListDtor70294c_5beae0(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5beb20_5beaa0(u8 *o, void *, u32 v0) {
    ListDtor702950_5beb20(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5beb60_5beac0(u8 *o, void *, u32 v0) {
    ListDtor702954_5beb60(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5d5800_5d57e0(u8 *o, void *, u32 v0) {
    ListDtor7029f8_5d5800(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5d8610_5d85d0(u8 *o, void *, u32 v0) {
    ListDtor702a88_5d8610(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5d8650_5d85f0(u8 *o, void *, u32 v0) {
    ListDtor702a8c_5d8650(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5d9f10_5d9ef0(u8 *o, void *, u32 v0) {
    ListDtor702afc_5d9f10(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5da2b0_5da290(u8 *o, void *, u32 v0) {
    ListDtor702b28_5da2b0(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5dc6c0_5dc680(u8 *o, void *, u32 v0) {
    ListDtor702c0c_5dc6c0(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5dc700_5dc6a0(u8 *o, void *, u32 v0) {
    ListDtor702c10_5dc700(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5e0370_5e0350(u8 *o, void *, u32 v0) {
    ListDtor702d00_5e0370(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5e3cd0_5e3cb0(u8 *o, void *, u32 v0) {
    ListDtor702d98_5e3cd0(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5e40a0_5e4080(u8 *o, void *, u32 v0) {
    ListDtor702f10_5e40a0(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia5f3900_5f38e0(u8 *o, void *, u32 v0) {
    ListDtor703084_5f3900(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ScalarDeleteVia47e390_6175f0(u8 *o, void *, u32 v0) {
    SurfaceSource_Dtor(reinterpret_cast<u8 *>(o)); if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ListScalarDelete6fd9e4_41f6a0(u8 *o, void *, u32 v0) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x006fd9e4; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ListScalarDelete700a9c_51aeb0(u8 *o, void *, u32 v0) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00700a9c; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ListScalarDelete700aa0_51af00(u8 *o, void *, u32 v0) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00700aa0; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ListScalarDelete7028a0_5ba7c0(u8 *o, void *, u32 v0) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x007028a0; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ListScalarDelete7028b4_5ba810(u8 *o, void *, u32 v0) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x007028b4; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ListScalarDelete70294c_5beba0(u8 *o, void *, u32 v0) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x0070294c; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ListScalarDelete702950_5bebf0(u8 *o, void *, u32 v0) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702950; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ListScalarDelete702a88_5d9750(u8 *o, void *, u32 v0) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702a88; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ListScalarDelete702afc_5da4d0(u8 *o, void *, u32 v0) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702afc; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ListScalarDelete702c0c_5ddaf0(u8 *o, void *, u32 v0) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702c0c; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ListScalarDelete702f10_5e4320(u8 *o, void *, u32 v0) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00702f10; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 * __fastcall ListScalarDelete703084_5f46e0(u8 *o, void *, u32 v0) {
    const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x00703084; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } at<u32>(o, 0xc) = 0; if (v0 & 1) GameHeap_Free(o); return o;
}

u8 *__fastcall Class7037b8_ScalarDtor(u8 *object, void *, u32 flags) {
    Class7037b8_Dtor(object);
    if (flags & 1)
        GameHeap_Free(object);
    return object;
}
}  // extern "C"
