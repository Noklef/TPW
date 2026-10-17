#include "accessors.h"

#include <cstdint>

#include "gameheap.h"
#include "string.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __fastcall Obj_Get24(const u8 *object) { return at<u32>(object, 0x24); }

u32 __fastcall Obj_Get2c(const u8 *object) { return at<u32>(object, 0x2c); }

u32 __fastcall Obj_Get30(const u8 *object) { return at<u32>(object, 0x30); }

u32 __fastcall Obj_SetFirst(u8 *object, void *, u32 value) {
    at<u32>(object, 0) = value;
    return value;
}

u32 __fastcall Obj_Get188(const u8 *object) { return at<u32>(object, 0x188); }

u32 __fastcall Obj_Set188(u8 *object, void *, u32 value) {
    at<u32>(object, 0x188) = value;
    return value;
}

u32 __fastcall Obj_Get194(const u8 *object) { return at<u32>(object, 0x194); }

u32 __fastcall Obj_Set194(u8 *object, void *, u32 value) {
    at<u32>(object, 0x194) = value;
    return value;
}

u8 __fastcall Obj_GetByte18c(const u8 *object) { return at<u8>(object, 0x18c); }

void __fastcall Obj_Inc2b8(u8 *object) { at<u32>(object, 0x2b8)++; }

void __fastcall Vtbl6fd818_Set(u8 *object) { at<u32>(object, 0) = 0x006fd818; }

u32 __fastcall Obj_SetByte7(u8 *object, void *, u32, u32 value) {
    at<u8>(object, 7) = (u8)value;
    return value;
}

void __stdcall Stub_Ret16(u32, u32, u32, u32) {}

void __stdcall Stub_Ret4b(u32) {}

u32 __cdecl Global7ca1cc_Get() { return global<u32>(0x007ca1cc); }

u32 __cdecl Global7cb2e8_Get() { return global<u32>(0x007cb2e8); }

u32 __cdecl Global7c2650_Get() { return global<u32>(0x007c2650); }

u32 __cdecl Global7c2654_Get() { return global<u32>(0x007c2654); }

u32 __cdecl Global7c2654_Set(u32 value) {
    global<u32>(0x007c2654) = value;
    return value;
}

u32 __cdecl Global7c2650_Set(u32 value) {
    global<u32>(0x007c2650) = value;
    return value;
}

u32 __cdecl Global8186ac_Get() { return global<u32>(0x008186ac); }

u32 __cdecl Global816d48_Set(u32 value) {
    global<u32>(0x00816d48) = value;
    return value;
}

u32 __cdecl Global816d48_Get() { return global<u32>(0x00816d48); }

u32 __cdecl Global80239c_Get() { return global<u32>(0x0080239c); }

u32 __cdecl Global802ba8_SetMasked(u32 value) {
    global<u32>(0x00802ba8) = value & 0x3ff;
    return value & 0x3ff;
}

void __cdecl Global802bc0_Set1() { global<u32>(0x00802bc0) = 1; }

void __cdecl Global871ee0_Clear() { global<u32>(0x00871ee0) = 0; }

u8 __cdecl Global7c24d0_GetByte() { return global<u8>(0x007c24d0); }

u32 __cdecl Table74fb50_Get(u32 i) { return reinterpret_cast<u32 *>(0x0074fb50)[i]; }

u32 __cdecl Counter7cdb98_Inc() { return ++global<u32>(0x007cdb98); }

u32 __cdecl Nibble_Scale(u32 v) {
    const u8 low = (u8)v;
    if (low == 0)
        return v;
    return low < 0x10 ? (u32)low << 4 : (u32)low >> 4;
}

u32 __cdecl Byte_ShiftLog2(u32 a, u32 b) {
    u8 reduced = (u8)a;
    u32 shift = 0;
    if (reduced > 1) {
        do {
            reduced >>= 1;
            shift++;
        } while (reduced > 1);
    }
    const u32 scaled = (b & 0xff) << shift;
    return (s32)scaled > 0x80 ? scaled >> 8 : scaled;
}

u32 __fastcall MeshPlus60_Ready(u8 *sub) {
    if (!(at<u32>(sub, (u32)-0x10) & 0x10000000)) {
        at<u32>(sub, (u32)-8) = global<u32>(0x00879dec);
        global<u32>(0x00879dec) = (u32)(uintptr_t)(sub - 0x60);
    }
    return 1;
}

u8 *__fastcall Plus28_IfKind2(u8 *sub) {
    return (at<u32>(sub, (u32)-0x10) & 0xf) == 2 ? sub - 0x28 : nullptr;
}

u32 __cdecl Globals7c264c_Load() {
    global<u32>(0x007c264c) = global<u32>(0x007b05a8);
    global<u32>(0x007c2650) = global<u32>(0x007b05ac);
    global<u32>(0x007c2654) = global<u32>(0x007b05b0);
    return global<u32>(0x007b05b0);
}

u8 *__fastcall Class492d80_Ctor(u8 *object) {
    at<u32>(object, 0x10) = 0;
    at<u32>(object, 0) = 0x00492d80;
    at<u32>(object, 4) = 0;
    at<u32>(object, 8) = 0xffffffff;
    at<u32>(object, 0xc) = 0;
    return object;
}

s32 __cdecl Global7cb28c_Select(s32 which, u32 v) {
    switch (which) {
    case 1:
        global<u32>(0x007cb28c) = v;
        return 0;
    case 2:
        global<u32>(0x007cb2a0) = v;
        return 0;
    case 3:
        global<u32>(0x007cb290) = v;
        return (s32)v;
    default:
        return which - 3;
    }
}

u32 __fastcall Obj_Get64Plus10(const u8 *object) { return at<u32>(object, 0x64) + 0x10; }

u32 __fastcall Obj_Get3c(const u8 *object) { return at<u32>(object, 0x3c); }

u32 __fastcall Obj_Take(u8 *object, void *, const u8 *source) {
    at<u32>(object, 0x14) = 0;
    at<u16>(object, 0xe) = at<u16>(source, 0xc);
    const u32 value = at<u32>(source, 0x10);
    at<u32>(object, 0x10) = value;
    at<u8>(object, 0xc) = 3;
    return value;
}

u32 __fastcall Obj_Set10(u8 *object, void *, u32 value) {
    at<u32>(object, 0x10) = value;
    return value;
}

const u8 *__fastcall Obj_TakeWords(u8 *object, void *, const u8 *source) {
    at<u32>(object, 0x14) = at<u32>(source, 0x20);
    at<u8>(object, 0xd) = at<u8>(source, 0x10);
    at<u16>(object, 0xe) = (u16)(at<u8>(object, 0x14) & 8);
    at<u8>(object, 0xc) = 1;
    at<u32>(object, 0x10) = ((u32)at<u16>(source, 0x18) << 16) | at<u16>(source, 0x14);
    return source;
}

u32 __fastcall Obj_Copy4At34(u8 *object, void *, const u32 *source) {
    if (!source)
        return 0;
    for (u32 i = 0; i < 4; i++)
        at<u32>(object, 0x34 + 4 * i) = source[i];
    return source[3];
}

u32 __fastcall Obj_GetWordPair(const u8 *object, void *, u32 *a, u32 *b) {
    if (at<u16>(object, 0x44) == 0 || !a || !b)
        return 0;
    *a = at<u16>(object, 0xa8);
    *b = at<u16>(object, 0xaa);
    return at<u32>(object, 0xa0);
}

u32 __fastcall Obj_GetBefore14c(const u8 *object) { return at<u32>(object, (u32)-0x14c); }

u32 __fastcall Obj_Set14_18_10(u8 *object, void *, u32 a, u32 b, u32 c) {
    at<u32>(object, 0x10) = c;
    at<u32>(object, 0x14) = a;
    at<u32>(object, 0x18) = b;
    return b;
}

u32 __fastcall Obj_SetColour8b4(u8 *object, void *, u32 colour) {
    at<u32>(object, 0x8b4) = colour;
    at<float>(object, 0x8b8) = (float)(colour & 0xff000000);
    at<float>(object, 0x8bc) = (float)(colour & 0xff0000);
    at<float>(object, 0x8c0) = (float)(colour & 0xff00);
    at<float>(object, 0x8c4) = (float)(colour & 0xff);
    return colour & 0xff;
}

u8 *__fastcall Obj_Init4f1520(u8 *object, void *, u32 a) {
    at<u32>(object, 0) = a;
    at<u32>(object, 8) = global<u32>(0x007099f0);
    at<u32>(object, 0xc) = global<u32>(0x007099f4);
    at<u32>(object, 0x10) = global<u32>(0x00fb34b0);
    return object;
}

u8 *__stdcall Obj_InitMinus1A(u8 *object, u32, u32, u32) {
    Obj_Init4f1520(object, nullptr, 0xffffffffu);
    return object;
}

u8 *__stdcall Obj_InitMinus1B(u8 *object, u32, u32, u32, u32, u32, u32, u32) {
    Obj_Init4f1520(object, nullptr, 0xffffffffu);
    return object;
}

u32 __cdecl Obj_Read30_34_38(const u8 *source, u32 *a, u32 *b, u32 *c) {
    *a = at<u32>(source, 0x30);
    *b = at<u32>(source, 0x34);
    return *c = at<u32>(source, 0x38);
}

u8 *__fastcall Obj_AddrOf4(u8 *object) { return object + 4; }

u32 __fastcall Obj_GetCOr6fd6c4(const u8 *object) {
    const u32 value = at<u32>(object, 0xc);
    return value ? value : 0x006fd6c4;
}

void __fastcall Obj_Set118(u8 *object, void *, u32 value) { at<u32>(object, 0x118) = value; }

u32 __fastcall WaveletCodec_GetRefCount(const u8 *o) {
    return at<u32>(o, (u32)-76);
}

u8 * __fastcall BalanceFile_GetValues(u8 *o) {
    return o + 0x8;
}

u32 __fastcall StreamBase_GetSource(const u8 *o) {
    return at<u32>(o, 0x4);
}

u8 __fastcall GetBytea_405560(const u8 *o) {
    return at<u8>(o, 0xa);
}

u32 __fastcall Get0_4056e0(const u8 *o) {
    return at<u32>(o, 0x0);
}

u32 __fastcall Get4f0_4145e0(const u8 *o) {
    return at<u32>(o, 0x4f0);
}

u32 __fastcall Get34_41ce80(const u8 *o) {
    return at<u32>(o, 0x34);
}

u32 __fastcall Get38_41ce90(const u8 *o) {
    return at<u32>(o, 0x38);
}

u32 __fastcall Set4_4217f0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x4) = v0;
    return v0;
}

u32 __fastcall Set4c_48f690(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x4c) = v0;
    return v0;
}

u32 __fastcall Get64_4e03d0(const u8 *o) {
    return at<u32>(o, 0x64);
}

u8 __fastcall GetByte190_4e21b0(const u8 *o) {
    return at<u8>(o, 0x190);
}

u32 __fastcall Set198_4e21e0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x198) = v0;
    return v0;
}

u8 __fastcall GetByte198_4e21f0(const u8 *o) {
    return at<u8>(o, 0x198);
}

u32 __fastcall Get1da710_51a280(const u8 *o) {
    return at<u32>(o, 0x1da710);
}

u32 __fastcall Get8_51c050(const u8 *o) {
    return at<u32>(o, 0x8);
}

u32 __fastcall Getm10_564ee0(const u8 *o) {
    return at<u32>(o, (u32)-16);
}

u8 * __fastcall Partc4_56dd60(u8 *o) {
    return o + 0xc4;
}

u32 __fastcall Getc_59b540(const u8 *o) {
    return at<u32>(o, 0xc);
}

u32 __fastcall Get4_5dc650(const u8 *o) {
    return at<u32>(o, 0x4);
}

void __fastcall Set70_To0_5dc850(u8 *o) {
    at<u32>(o, 0x70) = 0x0;
}

void __fastcall Set14_To1_5e26f0(u8 *o) {
    at<u32>(o, 0x14) = 0x1;
}

void __fastcall Set4_To1_5e3bb0(u8 *o) {
    at<u32>(o, 0x4) = 0x1;
}

void __fastcall Set4_To0_5e3bc0(u8 *o) {
    at<u32>(o, 0x4) = 0x0;
}

u8 __fastcall GetBytec_695af0(const u8 *o) {
    return at<u8>(o, 0xc);
}

u8 __fastcall GetByted_695b00(const u8 *o) {
    return at<u8>(o, 0xd);
}

u16 __fastcall GetWordc_69c8e0(const u8 *o) {
    return at<u16>(o, 0xc);
}

u16 __fastcall GetWorde_69c8f0(const u8 *o) {
    return at<u16>(o, 0xe);
}

u32 __fastcall Get1c_6a8cc0(const u8 *o) {
    return at<u32>(o, 0x1c);
}

u32 __fastcall Get88_6adf80(const u8 *o) {
    return at<u32>(o, 0x88);
}

u32 __fastcall Set2c_6b9220(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x2c) = v0;
    return v0;
}

u32 __fastcall Setc_6baa00(u8 *o, void *, u32 v0) {
    at<u32>(o, 0xc) = v0;
    return v0;
}

u32 __fastcall Set14_6bb190(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x14) = v0;
    return v0;
}

u32 __fastcall Get14_6bb1a0(const u8 *o) {
    return at<u32>(o, 0x14);
}

void __fastcall Set14_To0_6bb1e0(u8 *o) {
    at<u32>(o, 0x14) = 0x0;
}

u8 * __fastcall Part14_6c5590(u8 *o) {
    return o + 0x14;
}

u16 __fastcall Fn_4e0de0(u8 *o, void *, u32 v0) {
    at<u16>(o, 0x30) = (u16)v0;
    return (u16)v0;
}

u8 __cdecl Fn_4f6010() {
    return 0x0;
}

u16 __fastcall Fn_509ae0(u8 *o, void *, u32 v0) {
    at<u16>(o, 0x60) = (u16)v0;
    return (u16)v0;
}

u16 __fastcall Fn_50b080(u8 *o, void *, u32 v0) {
    at<u16>(o, 0x0) = (u16)v0;
    return (u16)v0;
}

void __cdecl Fn_52c2a0() {
    global<u8>(0x00818c54) = 0x0;
}

void __cdecl Fn_52fbd0() {
    global<u32>(0x00820a8c) = 0x0;
}

u32 __stdcall Fn_56b620(u32 v0) {
    return v0;
}

u32 __fastcall Fn_5ce530(u8 *o, void *, u32 v0) {
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x0) = at<u32>(at<u8 *>(o, 0x4), 0x0);
    return v0;
}

u32 __fastcall Fn_5cec50(u8 *o, void *, u32 v0, u32 v1) {
    return (u32)(uintptr_t)o;
}

u8 __cdecl Fn_5d1400() {
    return 0x1;
}

u32 __cdecl Fn_6590c3(u32 v0) {
    global<u32>(0x00faa5d4) = v0;
    return v0;
}

u32 __cdecl Fn_6590ea(u32 v0) {
    global<u32>(0x00faa5fc) = v0;
    return v0;
}

u32 __cdecl Fn_6593ff() {
    return global<u32>(0x0077c488);
}

u16 __cdecl Fn_6595a8() {
    return global<u16>(0x00faa5bc);
}

u32 __fastcall Fn_659ce0(u8 *o) {
    at<u16>(o, 0x10) = 0x1;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_65aa80(u8 *o, void *, u32 v0) {
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_65b850(const u8 *o) {
    return at<u32>(o, 0x148);
}

u32 __fastcall Fn_65bc70(const u8 *o) {
    return at<u32>(o, 0x90);
}

u32 __fastcall Fn_65d547(u8 *o, void *, u32 v0) {
    return at<u32>(o, 0x5c);
}

u32 __fastcall Fn_65d55a(const u8 *o) {
    return at<u32>(o, 0x60);
}

u32 __fastcall Fn_65d56b(const u8 *o) {
    return at<u32>(o, 0x54);
}

u32 __fastcall Fn_65d57c(const u8 *o) {
    return at<u32>(o, 0x48);
}

u32 __fastcall Fn_65d7bc(const u8 *o) {
    return at<u32>(o, 0x8c);
}

u32 __fastcall Fn_6617f0(u8 *o) {
    at<u16>(o, 0x12) = 0x1;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_661830(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x90) = v0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_661850(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x94) = v0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_6618e6(u8 *o) {
    at<u8>(o, 0x4) = 0x1;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_6618f8(u8 *o) {
    at<u8>(o, 0x4) = 0x0;
    return (u32)(uintptr_t)o;
}

u32 __cdecl Fn_661f9b(u32 v0) {
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x14) = 0x1;
    return v0;
}

u32 __fastcall Fn_662086(const u8 *o) {
    return at<u32>(o, 0xc);
}

u32 __fastcall Fn_662097(u8 *o) {
    at<u32>(o, 0x10) = 0x0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_6620cc(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x8) = v0;
    return (u32)(uintptr_t)o;
}

void __cdecl Fn_662411() {
    global<u32>(0x00faa638) = 0x1;
}

u32 __fastcall Fn_667801(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x140) = v0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_66781a(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x144) = v0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_668d84(const u8 *o) {
    return at<u32>(o, 0x148);
}

u32 __fastcall Fn_6693d0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x140) = v0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_6693f0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x13c) = v0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_669c30(const u8 *o) {
    return at<u32>(o, 0x10);
}

u32 __fastcall Fn_669f80(u8 *o) {
    at<u16>(o, 0x6) = 0x1;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_669f94(u8 *o) {
    at<u16>(o, 0x6) = 0x0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_66a002(u8 *o) {
    at<u16>(o, 0x4) = 0x1;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_66a281(const u8 *o) {
    return at<u32>(o, 0x138);
}

u32 __fastcall Fn_66a6c7(const u8 *o) {
    return at<u32>(o, 0x140);
}

u32 __fastcall Fn_66b469(const u8 *o) {
    return at<u32>(o, 0x140);
}

u32 __fastcall Fn_66b53e(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x148) = v0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_66c922(const u8 *o) {
    return at<u32>(o, 0x138);
}

u32 __fastcall Fn_66cc7c(const u8 *o) {
    return at<u32>(o, 0x158);
}

u32 __fastcall Fn_66d950(const u8 *o) {
    return at<u32>(o, 0x140);
}

u32 __fastcall Fn_66dad2(const u8 *o) {
    return at<u32>(o, 0x13c);
}

u8 __cdecl Fn_6a65e0() {
    return 0x0;
}

u32 __stdcall Fn_6c0200(u32 v0, u32 v1) {
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v1), 0x0) = 0x0;
    return 0x0;
}

u32 __cdecl Fn_6c3900(u32 v0) {
    return v0;
}

u32 __stdcall Fn_6d2780(u32 v0, u32 v1) {
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v1), 0x0) = 0x0;
    return v1;
}

u32 __fastcall Fn_409cd0(u8 *o, void *, u32 v0) {
    at<u16>(o, 0x0) = (u16)v0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_4d2a60(u8 *o, void *, u32 v0) {
    at<u16>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x0) = at<u16>(o, 0x10);
    return v0;
}

u32 __fastcall Fn_4e0df0(u8 *o, void *, u32 v0) {
    at<u16>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x0) = at<u16>(o, 0x30);
    return v0;
}

u16 __fastcall Fn_657ddd(const u8 *o) {
    return at<u16>(o, 0xa);
}

u8 __fastcall Fn_659710(const u8 *o) {
    return at<u8>(o, 0x4);
}

u16 __fastcall Fn_659730(const u8 *o) {
    return at<u16>(o, 0xe0);
}

u16 __fastcall Fn_659790(const u8 *o) {
    return at<u16>(o, 0x4);
}

u32 __fastcall Fn_659994(u8 *o, void *, u32 v0) {
    at<u16>(o, 0x8) = (u16)v0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_6599da(u8 *o, void *, u32 v0) {
    at<u16>(o, 0x32) = (u16)v0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_659a40(u8 *o, void *, u32 v0) {
    at<u16>(o, 0xe) = (u16)v0;
    return (u32)(uintptr_t)o;
}

u16 __fastcall Fn_65f330(const u8 *o) {
    return at<u16>(o, 0xe2);
}

u8 __fastcall Fn_66190a(const u8 *o) {
    return at<u8>(o, 0x4);
}

u32 __fastcall Fn_668564(u8 *o, void *, u32 v0) {
    at<u8>(o, 0x161) = (u8)v0;
    return (u32)(uintptr_t)o;
}

u8 __fastcall Fn_668800(const u8 *o) {
    return at<u8>(o, 0x161);
}

u16 __fastcall Fn_66b454(const u8 *o) {
    return at<u16>(o, 0x14c);
}

u32 __fastcall Fn_66b523(u8 *o, void *, u32 v0) {
    at<u16>(o, 0x14c) = (u16)v0;
    return (u32)(uintptr_t)o;
}

u8 __stdcall Fn_6d2770(u32 v0) {
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x0) = 0x0;
    return 0x0;
}
}
