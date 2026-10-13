// Motion_Apply, Track_Apply, Motion_CopyVertices and Motion_ResetChannels:
// reimplementation of 0x4721f0, 0x471860, 0x472270 and 0x472310.
#include "track.h"

#include <cmath>
#include <cstring>

#include "../math/matrix.h"
#include "curve.h"
#include "keys.h"
#include "quat.h"
#include "uvanim.h"
#include "vertexanim.h"

namespace {

template <typename T> T field(const void *p, u32 offset) {
    return *reinterpret_cast<const T *>(static_cast<const u8 *>(p) + offset);
}

// x - n with one rounding, as fisub / fsubr of an integer. n is always x's own __ftol
// result (or 0 out of range), so the double difference is exact.
float minus_whole(float x, double n) { return (float)((double)x - n); }

// The node's bounding box (+0x78: min x, y, z, max x, y, z) grows to hold v. A NaN v (or
// min) replaces the min: fcom's C0 is set when unordered.
void grow(u32 *node, int axis, float v) {
    float *box = reinterpret_cast<float *>(node + 0x1e);
    if (!(v >= box[axis])) box[axis] = v;
    else if (v > box[3 + axis]) box[3 + axis] = v;
}

void apply_vertices(const void *state, const u8 *model, u32 *node, const u8 *track) {
    const float frame = field<float>(state, 0x20);
    const u32 whole = (u32)(long long)frame;
    const u8 *block = field<const u8 *>(track, 0x28);
    const u32 *remap = field<const u32 *>(block, 0);
    const float *vertices = field<const float *>(block, 8);
    const u32 base = field<u32>(block, 4) * whole;
    const float t = minus_whole(frame, whole);  // fild qword: the whole part as unsigned
    node[0] |= 0x10000;
    const bool add = model[0x30] & 4;
    float *positions = field<float *>(node, 0x60);
    for (s32 e = 0; e < (s32)field<u16>(node, 0x58); ++e) {
        // Both ends of the lerp are the same key: the original never reads the next frame.
        const float *a = vertices + 3 * (base + (remap ? remap[e] : (u32)e));
        const float w = 1.0f - t;  // 0x6febd8
        const float x = a[0] * w + a[0] * t, y = a[1] * w + a[1] * t, z = a[2] * w + a[2] * t;
        float *dst = positions + 12 * ((u32)e >> 2) + (e & 3);
        if (add) {
            dst[0] = x + dst[0];
            dst[4] = dst[4] + y;
            dst[8] = dst[8] + z;
        } else {
            dst[0] = x, dst[4] = y, dst[8] = z;
        }
        grow(node, 0, dst[0]);
        grow(node, 1, dst[4]);
        grow(node, 2, dst[8]);
    }
}

// The rotation key's ease: t through an 8-byte table of bytes / 255 (0x6febec), 0 to 1
// in nine steps (0x6febe4 is 8.999995).
float ease(const u8 *table, float t) {
    const float scaled = t * 8.999995f;
    const s32 n = (s32)(long long)scaled;
    const float f = minus_whole(scaled, n);
    const float k = 0.003921568859368563f;
    float lo, hi;
    if (n == 0) {
        lo = 0.0f, hi = (float)table[0] * k;
    } else if ((u32)n < 8) {
        lo = (float)table[n - 1] * k, hi = (float)table[n] * k;
    } else {
        lo = (float)table[7] * k, hi = 1.0f;
    }
    return hi * f + (1.0f - f) * lo;
}

void apply_rotation(const MotionEnv &env, const void *state, const u8 *model, u32 *node, const u8 *track) {
    const RotationKey *keys = field<const RotationKey *>(track, 0x1c);
    const u16 count = field<u16>(track, 0x10);
    u32 i, j;
    float t;
    if (!Motion_FindKeys(state, reinterpret_cast<const u16 *>(keys), count, 2, 1, &i, &j, &t)) return;
    *env.rotationKeyCount = count;
    if ((u16)keys[i].ease != 0xffff) t = ease(field<const u8 *>(track, 0x34) + 8 * (u16)keys[i].ease, t);
    float m[9];
    Motion_RotationAtWith(env.sines, keys, *env.rotationKeyCount, *env.motionFlags, i, t, m);
    float *matrix = reinterpret_cast<float *>(node + 4);
    if (model[0x30] & 4) Matrix_PreMul33(m, matrix);
    else Matrix_SetAxes(matrix, m, m + 3, m + 6);
}

void apply_scale(const void *state, u32 *node, const u8 *track) {
    const u8 *keys = field<const u8 *>(track, 0x20);
    u32 i, j;
    float t;
    if (!Motion_FindKeys(state, reinterpret_cast<const u16 *>(keys), field<u16>(track, 0x12), 4, 0, &i, &j, &t))
        return;
    const float *a = reinterpret_cast<const float *>(keys + 16 * i + 4);
    const float *b = reinterpret_cast<const float *>(keys + 16 * j + 4);
    const float w = 1.0f - t;
    float s[3];
    for (int k = 0; k < 3; ++k) s[k] = b[k] * t + a[k] * w;
    Matrix_SetAxisLengths(reinterpret_cast<float *>(node + 4), &s[0], &s[1], &s[2]);
}

// Where on which path the node is: false when it is on none at this frame.
bool find_path(const void *state, const u8 *model, const u32 *node, const u8 *track, const u8 **path, s32 *index,
               float *t) {
    if (field<u32>(track, 4) & 1) {
        *path = field<const u8 *>(track, 0x18);
        u32 i, j;
        if (!Motion_FindKeys(state, field<const u16 *>(*path, 0xc), field<u16>(*path, 6), 1, 0, &i, &j, t))
            return false;
        *index = (**path & 2) ? (s32)(i * 3 + 1) : (s32)i;
        return true;
    }
    const float frame = field<float>(state, 0x20);
    const u32 *range = field<const u32 *>(track, 0x24);
    const float start = (float)range[0];
    if (!(frame >= start)) return false;
    if (!((float)((double)range[1] + start) >= frame)) return false;
    *path = field<const u8 *>(model, 0xac) + 16 * field<u16>(field<const u8 *>(node, 4), 0x52);
    const u32 flags = field<u32>(*path, 0);
    float percent;
    Path_PercentAt(reinterpret_cast<const float *>(range[3]), range[1], frame - start, &percent, flags & 1);
    percent = (float)std::fmod((double)(percent - -1000.0f), 100.0);  // _CIfmod (0x67b24a)
    u32 n = field<u16>(*path, 4);
    if (flags & 2) n /= 3;
    const float segments = (flags & 1) ? (float)n : (float)n - 1.0f;
    const float at = segments * percent * 0.0099999998f;  // 0x6fec00
    const u32 whole = (u32)(long long)at;
    *t = minus_whole(at, whole);
    *index = (flags & 2) ? (s32)(whole * 3 + 1) : (s32)whole;
    return true;
}

void apply_path(const MotionEnv &env, const void *state, const u8 *model, u32 *node, const u8 *track) {
    const u8 *path;
    s32 index;
    float t;
    if (!find_path(state, model, node, track, &path, &index, &t)) return;
    const u32 count = field<u16>(path, 4);
    *env.curvePointCount = count;
    const Vec3 *points = field<const Vec3 *>(path, 8);
    const u32 flags = field<u32>(path, 0);
    Vec3 p;
    if (flags & 2) Curve_BezierAt(points, count, index, t, &p);
    else if (flags & 8) Curve_LinearAt(points, index, t, &p);
    else Curve_CatmullRomAt(points, count, index, t, &p);
    float *position = reinterpret_cast<float *>(node + 0x10);
    if (model[0x30] & 4) {
        position[0] = position[0] + p.x;
        position[1] = position[1] + p.y;
        position[2] = position[2] + p.z;
    } else {
        position[0] = p.x, position[1] = p.y, position[2] = p.z;
    }
    if (!(field<u32>(track, 4) & 0x400)) return;
    Vec3 d;
    if (flags & 2) Curve_BezierTangentAt(points, count, index, t, &d);
    else if (flags & 8) Curve_LinearTangentAt(points, index, t, &d);
    else Curve_CatmullRomAt(points, count, index, t - -0.1f, &d);  // 0x6fec04: a point ahead, not a tangent
    const float length = std::sqrt((d.x * d.x + d.y * d.y) + d.z * d.z);
    const float dir[3] = {d.x / length, d.y / length, d.z / length};
    float *matrix = reinterpret_cast<float *>(node + 4);
    if (!(field<u32>(track, 4) & 8)) {
        matrix[4] = 0.0f, matrix[5] = 1.0f, matrix[6] = 0.0f;
    }
    Matrix_LookAlong(matrix, dir);
}

}  // namespace

s32 Track_ApplyWith(const MotionEnv &env, const void *state, u8 *model, u32 *node, const u8 *track) {
    const float frame = field<float>(state, 0x20);
    if (frame > field<float>(state, 0x1c)) return 0;
    const u32 flags = field<u32>(track, 4);

    if (flags & 0x20000) {  // visibility: the last key at or before the frame
        const u32 whole = (u32)(long long)frame;
        const s16 *key = field<const s16 *>(track, 0x30) + field<u16>(track, 0x16) - 1;
        for (u32 n = field<u16>(track, 0x16); n != 0; --n, --key) {
            s32 at = *key;
            if (at < 0) at = -at;
            if ((u32)(s32)(s16)at <= whole) {
                node[0] = *key > 0 ? node[0] & ~0x10u : node[0] | 0x10;
                break;
            }
        }
    }
    if (flags & 0x1000) {
        if (flags & 0x4000) {
            apply_vertices(state, model, node, track);
        } else {
            Motion_UnpackVertices(node, field<const u8 *>(track, 0x28), frame, 0, model[0x30] & 4);
            *reinterpret_cast<u32 *>(model + 0x30) |= 0x40000;
        }
    }
    if (flags & 8) apply_rotation(env, state, model, node, track);
    if (flags & 0x80) apply_scale(state, node, track);
    const u8 *object = field<const u8 *>(node, 4);
    if (((flags & 0x200) && object && (*object & 8)) || (flags & 1)) apply_path(env, state, model, node, track);
    if (((!(*env.motionFlags & 0x10) && !(node[0] & 0x4000)) || (model[0x30] & 4)) && (flags & 0x10000))
        Motion_ApplyUVs(state, track, model[0x30] & 4);
    return 1;
}

s32 Motion_ApplyWith(const MotionEnv &env, const void *state, const void *holder, const u8 *block) {
    const u8 *track = field<const u8 *>(block, 0x2c);
    for (s32 k = 0; k < (s32)field<u16>(block, 0x12); ++k, track += 0x40)
        Track_ApplyWith(env, state, field<u8 *>(holder, 4), field<u32 *>(track, 0x3c), track);
    if ((block[0] & 2) && !(*env.motionFlags & 8)) Motion_ApplyMaterial(field<u8 *>(holder, 4), block, state);
    return 1;
}

namespace {

// A model's node by index: 0xa0-byte nodes first, then 0x58-byte ones.
u8 *model_node(const u8 *model, u32 index) {
    const u32 primary = field<u16>(model, 0x44);
    return index < primary ? field<u8 *>(model, 0x70) + 0xa0 * index
                           : field<u8 *>(model, 0x74) + 0x58 * (index - primary);
}

void show(u8 *node) {
    u32 &flags = *reinterpret_cast<u32 *>(node);
    if (!(flags & 0x80000000)) flags &= ~0x10u;
}

}  // namespace

void Motion_ResetChannelsWith(u32 motionFlags, u32 *trackFlags, const void *animator, const u8 *block,
                              const u8 *prev) {
    const u8 *rest = field<const u8 *>(field<const u8 *>(field<const u8 *>(animator, 0), 8), 4);
    const u8 *target = field<const u8 *>(field<const u8 *>(animator, 8), 4);
    const u32 animator_flags = field<u32>(animator, 4);
    u32 flags_now = motionFlags;  // a copy: the animator's 0x100000 sets 0x1 in it only
    if (animator_flags & 8) {
        for (u32 i = 0; i < field<u16>(target, 0x42); ++i) *reinterpret_cast<u32 *>(model_node(target, i)) &= ~0x10u;
    } else if (animator_flags & 0x100000) {
        flags_now |= 1;
    }
    const bool add = target[0x30] & 4;
    const u32 cleared = add ? 0x800000 : 0xf40000;
    const u16 *listed = field<const u16 *>(block, 0x38);
    for (u32 n = field<u16>(block, 0x1a); n != 0; --n) show(model_node(target, *listed++));
    for (u32 i = 0; i < field<u16>(target, 0x42); ++i) trackFlags[i] = 0;
    const u8 *tracks = field<const u8 *>(block, 0x2c);
    for (u32 k = 0; k < field<u16>(block, 0x12); ++k) {
        const u8 *track = tracks + 0x40 * k;
        const u16 index = field<u16>(track, 0x14);
        trackFlags[index] = field<u32>(track, 4);
        u8 *node = model_node(target, index);
        *reinterpret_cast<u32 *>(node) &= ~cleared;
        show(node);
    }
    if (!(flags_now & 1) || add) {
        if (prev) {
            const u8 *old = field<const u8 *>(prev, 0x2c);
            for (u32 k = 0; k < field<u16>(prev, 0x12); ++k)
                trackFlags[field<u16>(old + 0x40 * k, 0x14)] &= ~field<u32>(old + 0x40 * k, 4);
        }
        for (u32 k = 0; k < field<u16>(block, 0x12); ++k) {
            const u16 index = field<u16>(tracks + 0x40 * k, 0x14);
            u8 *to = model_node(target, index);
            const u8 *from = model_node(rest, index);
            const u32 flags = trackFlags[index];
            if (flags & 0x289) {
                for (u32 row = 0x10; row <= 0x40; row += 0x10) std::memmove(to + row, from + row, 12);
            }
            if (flags & 0x1000) {
                void *positions = field<void *>(to, 0x60);
                const void *source = field<const void *>(from, 0x60);
                const u32 blocks = ((u32)field<u16>(to, 0x58) + 3) >> 2;
                if (positions && source && blocks) std::memmove(positions, source, 48 * blocks);
                std::memmove(to + 0x78, from + 0x78, 24);
            }
            if (flags & 0x10000) {
                void *uvs = field<void *>(to, 0x68);
                const void *source = field<const void *>(from, 0x68);
                const u32 blocks = ((u32)field<u16>(to, 0x5e) + 3) >> 2;
                if (uvs && source && blocks) std::memmove(uvs, source, 32 * blocks);
            }
        }
    }
    if ((block[0] & 2) && !(motionFlags & 8)) {
        u8 *material = field<u8 *>(target, 0x50);
        for (u32 n = field<u16>(target, 0x36); n != 0; --n, material += 8) {
            if (material[0] & 0x40) {
                material[1] |= 4;
                material[2] = material[3] = 0;
            }
        }
    }
}

namespace {
MotionEnv game_env() {
    return {reinterpret_cast<u32 *>(0x007b4988), reinterpret_cast<u32 *>(0x007b49e8),
            reinterpret_cast<const u32 *>(0x007b4980), SinTable_Game()};
}
}  // namespace

extern "C" s32 __cdecl Motion_Apply(const void *state, const void *holder, const u8 *block) {
    return block ? Motion_ApplyWith(game_env(), state, holder, block) : 1;
}

extern "C" void __cdecl Motion_CopyVertices(const void *from, const void *to, s32 all) {
    const u8 *src = field<const u8 *>(field<const u8 *>(from, 4), 0x70);
    const u8 *to_model = field<const u8 *>(to, 4);
    u8 *dst = field<u8 *>(to_model, 0x70);
    for (u32 n = field<u16>(to_model, 0x44); n != 0; --n, src += 0xa0, dst += 0xa0) {
        if (field<u16>(dst, 0x58) == 0) continue;
        if (!(field<u32>(dst, 0) & 0x200000) && !all) continue;
        void *positions = field<void *>(dst, 0x60);
        const void *source = field<const void *>(src, 0x60);
        const u32 blocks = ((u32)field<u16>(src, 0x58) + 3) >> 2;
        if (positions && source && blocks) std::memmove(positions, source, 48 * blocks);
        std::memmove(dst + 0x78, src + 0x78, 24);
    }
}

extern "C" void __cdecl Motion_ResetChannels(const void *animator, const u8 *block, const u8 *prev) {
    Motion_ResetChannelsWith(*reinterpret_cast<const u32 *>(0x007b4980), *reinterpret_cast<u32 **>(0x007aecd4),
                             animator, block, prev);
}

extern "C" s32 __cdecl Track_Apply(const void *state, u8 *model, u32, u32 *node, const u8 *track) {
    return Track_ApplyWith(game_env(), state, model, node, track);
}
