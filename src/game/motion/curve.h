// Motion path curves: reimplementation of the position and tangent functions that
// Track_Apply (0x471860) calls for .md2 path tracks.
//
// The originals are x87 code and the game runs the FPU at single precision (Direct3D sets
// it; SetCooperativeLevel gets neither DDSCL_FPUSETUP nor DDSCL_FPUPRESERVE), where each x87
// operation rounds like a float operation. Each component below therefore evaluates exactly
// the expression tree the original computes (recovered with tools/x87sym.py; the compiler
// associated x, y and z differently), so results match bit for bit.
#pragma once

#include "../../../re/types.h"

extern "C" {
// Pure versions. t selects segment i = (int)t with fraction t - i. Bezier and Catmull-Rom
// wrap indices modulo count; Bezier segments start at points[base + 3i - 1], Catmull-Rom
// ones at points[base + i - 1] (4 points each), linear ones at points[base + i] (2 points).
void Curve_BezierAt(const Vec3 *points, u32 count, s32 base, float t, Vec3 *out);
void Curve_BezierTangentAt(const Vec3 *points, u32 count, s32 base, float t, Vec3 *out);
void Curve_LinearAt(const Vec3 *points, s32 base, float t, Vec3 *out);
void Curve_LinearTangentAt(const Vec3 *points, s32 base, float t, Vec3 *out);
void Curve_CatmullRomAt(const Vec3 *points, u32 count, s32 base, float t, Vec3 *out);

// Hook entries (cdecl, as the originals). points == null means the curve's own (curve +8;
// Curve_LinearTangent always uses those);
// count comes from the game's gCurvePointCount (0x7b49e8), which Track_Apply sets first.
void __cdecl Curve_Bezier(const void *curve, const Vec3 *points, s32 base, float t, Vec3 *out);          // 0x474840
void __cdecl Curve_BezierTangent(const void *curve, const Vec3 *points, s32 base, float t, Vec3 *out);   // 0x474a20
void __cdecl Curve_Linear(const void *curve, const Vec3 *points, s32 base, float t, Vec3 *out);          // 0x474bf0
void __cdecl Curve_LinearTangent(const void *curve, const Vec3 *points, s32 base, float t, Vec3 *out);   // 0x474c70
void __cdecl Curve_CatmullRom(const void *curve, const Vec3 *points, s32 base, float t, Vec3 *out);      // 0x474cc0
}
