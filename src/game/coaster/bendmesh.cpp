#include "bendmesh.h"

#include <cstdint>

#include "../math/vector.h"
#include "../math/x87.h"
#include "../model/partstate.h"
#include "helpers.h"
#include "nodeangles.h"
#include "../world/rides.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const BendMeshEnv kPortEnv = {&Coaster_NodeAngles, &PartStates_Update};
const BendMeshEnv *g_env = &kPortEnv;

const u32 kFrame = 0xa0;
const u32 kExtraFrame = 0x58;

u8 *RideOf(const u8 *car) { return gRides_7a4610[at<u32>(car, 0xc)]; }

// The table's value at the place: the entries at w and w + 1 mixed by f and g.
X87Reg Mixed(const float *table, s32 w, float f, float g) { return (X87Reg(table[w]) * g) + (X87Reg(table[w + 1]) * f); }

void BendFrame(const u8 *car, const u8 *holder, const float *out1, const float *out2, const float *out3, u32 count, float spacing) {
    const u8 *model = at<u8 *>(at<u8 *>(holder, 8), 4);
    const u32 number = at<u32>(at<u8 *>(car, 4), 0x20);
    const u32 first = at<u16>(model, 0x44);
    const u8 *frame = number < first ? at<u8 *>(model, 0x70) + number * kFrame : at<u8 *>(model, 0x74) + (number - first) * kExtraFrame;
    if (at<u32>(frame, 0) & 0x200) return;

    const s32 index = (s32)(frame - at<u8 *>(model, 0x70)) / (s32)kFrame;
    float *dst = at<float *>(frame, 0x60);
    const float *src = at<float *>(at<u8 *>(at<u8 *>(model, 0x9c), 0x70) + index * kFrame, 0x60);
    const float total = (float)X87_MulInt(count, spacing);
    const u32 vertices = at<u16>(frame, 0x58);
    u32 i = 0;
    for (u32 v = 0; v < vertices; ++v) {
        float vec[3];
        vec[2] = X87_Through(src[8 + i]);
        const X87Reg t = (X87Reg(total) + vec[2]) / spacing;
        vec[0] = X87_Through(src[i]);
        vec[1] = X87_Through(src[4 + i]);
        s32 w;
        const float f = (float)Float_SplitWhole((float)t, &w);
        const float g = (float)(X87Reg(1.0f) - f);   // 0x6fdf28
        // The original's tables are its stack: a w outside the filled entries reads what the stack holds there. The bench keeps w inside.
        const X87Reg third = Mixed(out3, w, f, g);
        const X87Reg second = Mixed(out2, w, f, g);
        vec[1] = (float)(third + vec[1]);
        Vec3_RotateX(vec, (float)second);
        Vec3_RotateY(vec, (float)Mixed(out1, w, f, g));
        dst[i] = X87_Through(vec[0]);
        dst[4 + i] = X87_Through(vec[1]);
        dst[8 + i] = X87_Through(vec[2]);
        if (++i == 4) {
            i = 0;
            src += 12;
            dst += 12;
        }
    }
}

}  // namespace

void BendMesh_SetEnv(const BendMeshEnv *env) { g_env = env ? env : &kPortEnv; }

extern "C" void __cdecl Coaster_BendMesh(const u8 *path, const u8 *car) {
    if (!at<u32>(car, 0xc)) return;
    float out1[0x80], out2[0x80], out3[0x80];
    u32 count;
    g_env->nodeAngles(path, car, out1, out2, out3, 0x80, at<float>(car, 0x40), &count);
    const float spacing = at<float>(path, 0xac);
    const u8 *holder = RideOf(car);
    if (holder) BendFrame(car, holder, out1, out2, out3, count, spacing);
    // sic: a ride with no holder faults here, reading +8 of null.
    g_env->partStates(at<u8 *>(RideOf(car), 8));
}
