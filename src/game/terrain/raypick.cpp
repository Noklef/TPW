#include "raypick.h"

#include <cmath>
#include <cstdint>

#include "../math/matrix.h"
#include "../math/x87.h"
#include "../coaster/train.h"
#include "../model/worldverts.h"
#include "../motion/normals.h"
#include "land.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const float kFullCircle = 6.2821855545043945f;   // 0x6fe678
const float kOne = 1.0f;                          // 0x6fe620
const float kZero = 0.0f;                         // 0x6fe630
const float kTenth = 0.100000001f;                // 0x6fe664

const RayPickEnv kPortEnv = {&WorldVerts_TransformNode, &Terrain_SurfaceHeight, &Ride_Train, &Train_CarValue, &Model_UpdateNormals, &RayPick_Node};
const RayPickEnv *g_env = &kPortEnv;

// The game's acos (0x67b4e0) of a register, as in motion/quat.cpp. Past 1 (a dot of rounded unit vectors) it gives a NaN, so no hit, here as there; there the CRT's math error path also sets
// errno and raises a masked exception, not kept here.
double CrtAcos(X87Reg x) {
    const X87Reg r = X87_Sqrt((X87Reg(1.0) + x) * (X87Reg(1.0) - x));
    return X87_Atan2(r.v, x.v);
}

X87Reg Dot(const float *a, const float *b) { return ((X87Reg(a[1]) * b[1]) + (X87Reg(a[2]) * b[2])) + (X87Reg(b[0]) * a[0]); }

// Face against the ray: the point stored and true when it is in the face.
bool HitFace(const float *v1, const float *v2, const float *v3, const float *normalIn, const float *m) {
    float n[3] = {normalIn[0], normalIn[1], normalIn[2]};
    Matrix_TransformVector(m, n);
    const X87Reg length = X87_Sqrt(((X87Reg(n[0]) * X87Reg(n[0])) + (X87Reg(n[1]) * n[1])) + (X87Reg(n[2]) * n[2]));
    n[0] = (float)(X87Reg(n[0]) / length);
    n[1] = (float)(X87Reg(n[1]) / length);
    n[2] = (float)(X87Reg(n[2]) / length);

    const float *o = global<const float *>(0x7a1cb0);
    const float *d = global<const float *>(0x7a1cd8);
    const X87Reg p = ((X87Reg(o[0]) * n[0]) + (X87Reg(n[1]) * o[1])) + (X87Reg(n[2]) * o[2]);
    const X87Reg q = ((X87Reg(n[2]) * v1[2]) + (X87Reg(n[1]) * v1[1])) + (X87Reg(n[0]) * v1[0]);
    const X87Reg top = p + X87Reg(-q.v);
    const X87Reg under = ((X87Reg(d[0]) * n[0]) + (X87Reg(n[1]) * d[1])) + (X87Reg(n[2]) * d[2]);
    const X87Reg back(-(top / under).v);

    float h[3];
    h[0] = (float)((X87Reg(d[0]) * back) + o[0]);
    h[1] = (float)((back * d[1]) + o[1]);
    h[2] = (float)((back * d[2]) + o[2]);

    const X87Reg ax = X87Reg(v1[0]) - h[0];
    float a[3] = {(float)ax, (float)(X87Reg(v1[1]) - h[1]), (float)(X87Reg(v1[2]) - h[2])};
    float b[3], c[3];
    for (int i = 0; i < 3; ++i) {
        b[i] = (float)(X87Reg(v2[i]) - h[i]);
        c[i] = (float)(X87Reg(v3[i]) - h[i]);
    }
    const float lengthA = (float)X87_Sqrt(((ax * ax) + (X87Reg(a[1]) * a[1])) + (X87Reg(a[2]) * a[2]));
    const float lengthB = (float)X87_Sqrt(((X87Reg(b[0]) * b[0]) + (X87Reg(b[1]) * b[1])) + (X87Reg(b[2]) * b[2]));
    const float lengthC = (float)X87_Sqrt(((X87Reg(c[0]) * c[0]) + (X87Reg(c[1]) * c[1])) + (X87Reg(c[2]) * c[2]));
    for (int i = 0; i < 3; ++i) {
        a[i] = (float)(X87Reg(a[i]) / lengthA);
        b[i] = (float)(X87Reg(b[i]) / lengthB);
        c[i] = (float)(X87Reg(c[i]) / lengthC);
    }

    const float first = (float)CrtAcos(Dot(c, b));
    const double second = CrtAcos(((X87Reg(b[0]) * a[0]) + (X87Reg(b[1]) * a[1])) + (X87Reg(b[2]) * a[2]));
    const X87Reg sum = X87Reg(first) + X87Reg(second);
    const double third = CrtAcos(((X87Reg(c[2]) * a[2]) + (X87Reg(c[0]) * a[0])) + (X87Reg(c[1]) * a[1]));
    const X87Reg total = sum + X87Reg(third);
    if (!(total.v >= kFullCircle)) return false;

    float *point = reinterpret_cast<float *>((uintptr_t)0x7a1cc8);
    float *normal = reinterpret_cast<float *>((uintptr_t)0x7a1c80);
    for (int i = 0; i < 3; ++i) {
        point[i] = h[i];
        normal[i] = n[i];
    }
    return true;
}

}  // namespace

void RayPick_SetEnv(const RayPickEnv *env) { g_env = env ? env : &kPortEnv; }

extern "C" {

u32 __cdecl RayPick_Node(u8 *node, RayPickMatrix m) {
    Matrix_PreMul(reinterpret_cast<const float *>(node + 0x10), m.m);
    if (!(at<u32>(node, 0) & 0x8210)) {
        g_env->transformNode(node, m.m);
        const u16 *index = at<const u16 *>(node, 0x94);
        const u8 *group = at<const u8 *>(node, 0x6c);
        for (u32 g = at<u16>(node, 0x5a); g != 0; --g, group += 0x10) {
            const u8 *face = at<const u8 *>(node, 0x70) + at<u16>(group, 4) * 8u;
            for (u32 f = at<u16>(group, 6); f != 0; --f, face += 8) {
                const float *verts = global<const float *>(0x7aecd4);
                const float *v1 = verts + index[at<u16>(face, 2)] * 3u;
                const float *v2 = verts + index[at<u16>(face, 4)] * 3u;
                const float *v3 = verts + index[at<u16>(face, 6)] * 3u;
                const float *normal = at<const float *>(node, 0x64) + (at<u16>(face, 0) & 0x7fffu) * 3u;
                if (HitFace(v1, v2, v3, normal, m.m)) return 1;
            }
        }
    }
    u32 result = 0;
    if (!(at<u32>(node, 0) & 0x20)) {
        for (u8 *child = at<u8 *>(node, 0xc); child != nullptr && result == 0; child = at<u8 *>(child, 8)) result = RayPick_Node(child, m);
    }
    return result;
}

u8 *__cdecl RayPick_Column(s32 x, s32 y, u32 alongX, float atX, float atY, float atZ, float drop, const float *direction, const float *origin, float step) {
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= global<s32>(0x7a0314)) x = global<s32>(0x7a0314) - 1;
    if (y >= global<s32>(0x7a0374)) y = global<s32>(0x7a0374) - 1;
    u8 **const rides = reinterpret_cast<u8 **>((uintptr_t)0x7a4610);

    u32 slot = 0;
    float height = (float)g_env->surfaceHeight(x, y, &slot);
    if (slot == 0) return nullptr;
    u8 *ride = rides[slot];
    u32 cars;
    u8 *train = g_env->rideTrain(slot, &cars);
    const u32 count = train != nullptr ? cars : 1;

    for (u32 i = 0; i < count; ++i) {
        if (train != nullptr) {
            u32 id = g_env->carValue(train, count - i - 1);
            ride = rides[id];
            height = (float)g_env->surfaceHeight(x, y, &id);
        } else {
            u8 *model = at<u8 *>(at<u8 *>(ride, 8), 4);
            const u32 flags = at<u32>(model, 0x30);
            if ((flags & 0x40000) && (flags & 0x20000)) g_env->updateNormals(model);
        }

        const float along = alongX != 0 ? atX : atZ;
        double place;
        if (step > kZero) {
            double whole;
            place = std::modf((X87Reg(along) * kTenth).v, &whole);
        } else {
            place = X87_SubR(X87Reg(std::fmod((double)along, 10.0)) * kTenth, kOne).v;   // (fld 10.0, the double 0x6fe670; the CRT's fmod)
        }
        const X87Reg rest = X87Reg(kOne) - X87Reg(place);
        const X87Reg low = atY - (X87Reg(place) * drop);
        const float high = (float)((rest * drop) + atY);
        const double lowest = !(low.v >= high) ? low.v : (double)high;
        if (lowest > height) continue;
        const double highest = low.v > high ? low.v : (double)high;
        u8 *model = at<u8 *>(at<u8 *>(ride, 8), 4);
        if (!(highest >= at<float>(model, 0x84))) continue;

        global<const float *>(0x7a1cd8) = direction;
        global<const float *>(0x7a1cb0) = origin;
        RayPickMatrix m;
        Matrix_Identity(m.m);
        if (g_env->node(at<u8 *>(model, 0x78), m) != 0) {
            const u32 *point = reinterpret_cast<const u32 *>((uintptr_t)0x7a1cc8);
            const u32 *normal = reinterpret_cast<const u32 *>((uintptr_t)0x7a1c80);
            u32 *pointOut = reinterpret_cast<u32 *>((uintptr_t)0x7a1cb8);
            u32 *normalOut = reinterpret_cast<u32 *>((uintptr_t)0x7a1ca0);
            for (int k = 0; k < 3; ++k) {
                pointOut[k] = point[k];
                normalOut[k] = normal[k];
            }
            return ride;
        }
    }
    return nullptr;
}
}
