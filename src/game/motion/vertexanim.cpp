// Packed vertex animation: reimplementation of 0x4714a0, 0x470e90 and 0x4711d0.
#include "vertexanim.h"

namespace {

const float *offsets(const void *block) { return reinterpret_cast<const float *>(static_cast<const u8 *>(block) + 0x14); }
const float *scales(const void *block) { return reinterpret_cast<const float *>(static_cast<const u8 *>(block) + 0x20); }

s32 component(u32 key, int axis) {  // sign-extended 10-bit field
    return (s32)(key << (22 - 10 * axis)) >> 22;
}

// (a * scale + offset) * (1 - t) + (b * scale + offset) * t for one axis.
float lerp(const void *block, u32 a, u32 b, int axis, float t, float w) {
    const float s = scales(block)[axis], o = offsets(block)[axis];
    return ((float)component(a, axis) * s + o) * w + ((float)component(b, axis) * s + o) * t;
}

// Advance the group's key while the next key's frame is before the frame (a NaN frame
// never stops it, as in the original: fcomp's C0 is set when unordered) and return t.
float advance(PackedGroup *g, float frame) {
    while (!((float)g->frames[g->key + 1] >= frame)) ++g->key;
    const float f0 = (float)g->frames[g->key];
    return (frame - f0) / ((float)g->frames[g->key + 1] - f0);
}

// Raw keys: one per vertex, no lerp.
void unpack_raw(const void *block, float *positions, const PackedGroup *g, s32 add) {
    for (u32 e = 0; e < g->count; ++e) {
        const u16 slot = g->slots[e];
        float *dst = positions + 12 * (slot >> 2) + (slot & 3);
        for (int axis = 0; axis < 3; ++axis) {
            const float v = (float)component(g->keys[e], axis) * scales(block)[axis] + offsets(block)[axis];
            dst[4 * axis] = add ? v + dst[4 * axis] : v;
        }
    }
}

}  // namespace

extern "C" void __cdecl Motion_LerpPackedVertices(const void *block, float *positions, const PackedGroup *group,
                                                  float t, s32 add) {
    const u32 n = group->count;
    const u32 *a = group->keys + group->key * n, *b = a + n;
    const float w = 1.0f - t;  // 0x6febd8
    for (u32 e = 0; e < n; ++e) {
        const u16 slot = group->slots[e];
        float *dst = positions + 12 * (slot >> 2) + (slot & 3);
        for (int axis = 0; axis < 3; ++axis) {
            const float v = lerp(block, a[e], b[e], axis, t, w);
            dst[4 * axis] = add ? v + dst[4 * axis] : v;
        }
    }
}

extern "C" void __cdecl Motion_LerpPackedBox(const void *block, float *box, const PackedGroup *group, float t,
                                             s32 add) {
    const u32 n = group->count;
    const u32 *a = group->keys + group->key * n, *b = a + n;
    const float w = 1.0f - t;
    const float *s = scales(block);
    for (int axis = 0; axis < 3; ++axis) {  // min corner, pushed out by scale + 0.25 (0x6febdc)
        const float v = (lerp(block, a[0], b[0], axis, t, w) - s[axis]) - 0.25f;
        box[axis] = add ? v + box[axis] : v;
    }
    // max corner (0x6febe0 is -0.25); the add path sums in a different order for x
    const float x = lerp(block, a[1], b[1], 0, t, w), y = lerp(block, a[1], b[1], 1, t, w),
                z = lerp(block, a[1], b[1], 2, t, w);
    if (add) {
        box[3] = ((x + s[0]) + box[3]) - -0.25f;
        box[4] = ((y + box[4]) + s[1]) - -0.25f;
        box[5] = ((z + box[5]) + s[2]) - -0.25f;
    } else {
        box[3] = (x + s[0]) - -0.25f;
        box[4] = (y + s[1]) - -0.25f;
        box[5] = (z + s[2]) - -0.25f;
    }
}

extern "C" void __cdecl Motion_UnpackVertices(u32 *node, const u8 *block, float frame, u32, s32 add) {
    // The original also takes modf(frame) and __ftol's the whole part into a local it never reads.
    PackedGroup *g = *reinterpret_cast<PackedGroup *const *>(block + 0xc);
    u32 left = *reinterpret_cast<const u16 *>(block + 2);
    float *box = reinterpret_cast<float *>(node + 0x1e);
    auto positions = [node] { return *reinterpret_cast<float **>(node + 0x18); };
    if (!(node[0] & 0x800000)) {
        for (u32 i = 0; i < left; ++i) g[i].key = 0;
        Motion_LerpPackedBox(block, box, g, advance(g, frame), add);
        ++g, --left;
        if (block[0] & 2) {
            unpack_raw(block, positions(), g, add);
            ++g, --left;
        }
        if (!add) node[0] |= 0x800000;
    } else {
        Motion_LerpPackedBox(block, box, g, advance(g, frame), add);
        ++g, --left;
        if (block[0] & 2) ++g, --left;
    }
    for (; left != 0; --left, ++g) Motion_LerpPackedVertices(block, positions(), g, advance(g, frame), add);
    node[0] |= 0x10000;
}
