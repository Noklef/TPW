// Functions with a frame prologue that do nothing or return a constant or a field (the C++ library code at 0x64fe7d .. 0x66d6ea that the game links in). Reimplementation of 0x64fe7d, 0x6503c1, 0x6503cb, 0x654a1d, 0x655e8d, 0x657def, 0x658490, 0x658aab, 0x658cc5, 0x6598dd, 0x6598ea, 0x659985, 0x659b49, 0x65ad33 and others. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x64fe7d: does nothing (frame style).
void __cdecl FrameStub_64fe7d();

// 0x6503c1: returns 0x989680.
u32 __cdecl FrameConst989680_6503c1();

// 0x6503cb: does nothing (frame style).
void __cdecl FrameStub_6503cb();

// 0x654a1d: does nothing (frame style).
void __cdecl FrameStub_654a1d();

// 0x655e8d: does nothing (frame style).
void __cdecl FrameStub_655e8d();

// 0x657def: returns the word 0.
u16 __stdcall FrameWordZero_657def(u32 v0);

// 0x658490: returns the word 0.
u16 __stdcall FrameWordZero_658490(u32 v0, u32 v1, u32 v2);

// 0x658aab: returns the word 0x1.
u16 __cdecl FrameWord1_658aab();

// 0x658cc5: returns 0.
u32 __cdecl FrameZero_658cc5();

// 0x6598dd: does nothing (frame style).
void __stdcall FrameStub_6598dd(u32 v0, u32 v1, u32 v2);

// 0x6598ea: does nothing (frame style).
void __stdcall FrameStub_6598ea(u32 v0, u32 v1, u32 v2, u32 v3);

// 0x659985: returns 0.
u32 __stdcall FrameZero_659985(u32 v0, u32 v1);

// 0x659b49: returns the word 0xffff.
u16 __cdecl FrameWordffff_659b49();

// 0x65ad33: returns 0x4.
u32 __cdecl FrameConst4_65ad33();

// 0x65bc60: returns 0.
u32 __cdecl FrameZero_65bc60();

// 0x65c2c8: returns 0x1.
u32 __cdecl FrameConst1_65c2c8();

// 0x65dd5f: does nothing (frame style).
void __stdcall FrameStub_65dd5f(u32 v0);

// 0x65dd6c: returns the word 0x1.
u16 __cdecl FrameWord1_65dd6c();

// 0x661596: returns 0.
u32 __cdecl FrameZero_661596();

// 0x6618ba: does nothing (frame style).
void __stdcall FrameStub_6618ba(u32 v0, u32 v1);

// 0x66191b: does nothing (frame style).
void __stdcall FrameStub_66191b(u32 v0, u32 v1);

// 0x661928: does nothing (frame style).
void __stdcall FrameStub_661928(u32 v0);

// 0x661b33: returns the word 0xffff.
u16 __cdecl FrameWordffff_661b33();

// 0x661c2b: does nothing (frame style).
void __cdecl FrameStub_661c2b();

// 0x661e60: does nothing (frame style).
void __stdcall FrameStub_661e60(u32 v0);

// 0x6628e4: returns 0x7.
u32 __cdecl FrameConst7_6628e4();

// 0x6662a7: returns 0x5.
u32 __cdecl FrameConst5_6662a7();

// 0x66825c: does nothing (frame style).
void __stdcall FrameStub_66825c(u32 v0, u32 v1);

// 0x668aad: returns 0x2.
u32 __cdecl FrameConst2_668aad();

// 0x668c89: returns the word 0.
u16 __stdcall FrameWordZero_668c89(u32 v0, u32 v1);

// 0x66a040: does nothing (frame style).
void __cdecl FrameStub_66a040();

// 0x66a04b: does nothing (frame style).
void __cdecl FrameStub_66a04b();

// 0x66a056: does nothing (frame style).
void __cdecl FrameStub_66a056();

// 0x66a127: does nothing (frame style).
void __cdecl FrameStub_66a127();

// 0x66a132: does nothing (frame style).
void __cdecl FrameStub_66a132();

// 0x66a13d: does nothing (frame style).
void __cdecl FrameStub_66a13d();

// 0x66a271: returns 0x8.
u32 __cdecl FrameConst8_66a271();

// 0x66a5b4: returns 0xc.
u32 __cdecl FrameConstc_66a5b4();

// 0x66ae73: returns 0x3.
u32 __cdecl FrameConst3_66ae73();

// 0x66bfc0: returns the word 0.
u16 __stdcall FrameWordZero_66bfc0(u32 v0, u32 v1);

// 0x66bfd0: does nothing (frame style).
void __stdcall FrameStub_66bfd0(u32 v0, u32 v1);

// 0x66bfdd: does nothing (frame style).
void __stdcall FrameStub_66bfdd(u32 v0, u32 v1);

// 0x66bfea: does nothing (frame style).
void __stdcall FrameStub_66bfea(u32 v0, u32 v1, u32 v2, u32 v3);

// 0x66c00b: returns 0.
u32 __stdcall FrameZero_66c00b(u32 v0);

// 0x66c156: does nothing (frame style).
void __stdcall FrameStub_66c156(u32 v0, u32 v1, u32 v2, u32 v3);

// 0x66c2d2: does nothing (frame style).
void __stdcall FrameStub_66c2d2(u32 v0, u32 v1, u32 v2, u32 v3);

// 0x66c744: does nothing (frame style).
void __stdcall FrameStub_66c744(u32 v0, u32 v1, u32 v2, u32 v3);

// 0x66c84a: returns 0xd.
u32 __cdecl FrameConstd_66c84a();

// 0x66cac3: returns 0xb.
u32 __cdecl FrameConstb_66cac3();

// 0x66d1e8: returns 0xa.
u32 __cdecl FrameConsta_66d1e8();

// 0x66d644: does nothing (frame style).
void __stdcall FrameStub_66d644(u32 v0);

// 0x66d6ea: does nothing (frame style).
void __cdecl FrameStub_66d6ea();

// 0x66d875: returns 0x9.
u32 __cdecl FrameConst9_66d875();

// 0x66da3d: returns 0x6.
u32 __cdecl FrameConst6_66da3d();
}  // extern "C"
