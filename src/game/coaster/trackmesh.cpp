#include "trackmesh.h"

#include <cmath>
#include <cstdint>
#include <cstring>

#include "../math/x87.h"
#include "lists.h"
#include "spline.h"
#include "xsection.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *track() { return global<u8 *>(0x00790fe4); }
TrackOrientation *orientation() { return reinterpret_cast<TrackOrientation *>(track() + 0x3c); }
const u8 *coasterNodes() { return at<u8 *>(global<u8 *>(0x00792218), 0x9c); }

float *const kMax = reinterpret_cast<float *>((uintptr_t)0x007925b8);
float *const kMin = reinterpret_cast<float *>((uintptr_t)0x007925a8);

// The vertex counts moved on by a section's points.
void advance(const u8 *section) {
    const s32 points = at<s32>(section, 0);
    global<s32>(0x0079c600) += points;
    global<s32>(0x007923e0) -= points;
    global<s32>(0x007925c4) += points;
    global<s32>(0x007925d4) -= points;
}

void emit(float blend, float yaw, float pitch, float roll, const Vec3 *at_, u32 separate, const u8 *record) {
    CrossSection_Emit(blend, yaw, pitch, roll, &at_->x, kMax, kMin, separate, at<u8 *>(record, 0xc),
                      reinterpret_cast<const float *>(record));
}

}  // namespace

extern "C" {

void __cdecl TrackMesh_EmitPiece(u8 *piece, u8 *next, s32 count, const float *positions, u32 restart) {
    u8 *const build = at<u8 *>(global<u8 *>(0x00792218), 4);
    const u8 *record;
    u32 force = 0;
    const bool unbuilt = at<u32>(piece, 0xb0) == 0;
    if (unbuilt && !(at<u32>(piece, 8) & 0x1000000) && !(at<u32>(next, 8) & 0x802000)) {
        if ((at<u32>(piece, 8) & 0x800) && (build[0] & 0x40)) {
            if (global<u32>(0x007925f8) == 0) force = 1;
            record = reinterpret_cast<const u8 *>((uintptr_t)0x007923f0);
            global<u32>(0x007925f8) = 1;
        } else {
            if (global<u32>(0x007925f8) != 0) force = 1;
            record = global<u8 *>(0x00792414);
            global<u32>(0x007925f8) = 0;
        }
    } else {
        const u32 flags = at<u32>(piece, 8);
        u32 kind;
        if (flags & 0x1000000)
            kind = (flags & 0x4000) ? 2 : global<u32>(0x007925d0);
        else if (!unbuilt)
            kind = 2;
        else
            kind = (at<u32>(next, 8) & 0x4000) ? 2 : 3;
        const u16 list = (u16)(at<u32>(build, 0x20) + kind - 1);
        u8 *const section = at<u8 *>(build, 0x80);
        for (u32 i = 0; i < at<u32>(section, 4); ++i) at<u16>(at<u8 *>(section, 0xc), 12 * i + 4) = list;
        record = reinterpret_cast<const u8 *>((uintptr_t)0x007925e8);
        force = 1;
    }

    Vec3 prev, cur;
    u8 *spline = at<u8 *>(piece, 0x90);
    if (piece == next) {
        if (at<u8>(at<u8 *>(piece, 0x100), 4) & 1) return;
        std::memcpy(&prev, at<u8 *>(spline, 4) + 0x40, sizeof prev);
    } else {
        std::memcpy(&prev, at<u8 *>(spline, 4) + at<s32>(spline, 0) * 0x58 - 0x18, sizeof prev);
    }

    if (at<u32>(piece, 0) == 0 || restart != 0) {
        u8 *const nodes = at<u8 *>(spline, 4);
        const float blend = at<float>(nodes, 8);
        u8 *const following = at<u8 *>(next, 0x90);
        if (piece == next) {
            TrackSpline_Frame(coasterNodes(), track(), nodes, at<u8 *>(following, 4) + 0x58, at<u8 *>(spline, 8),
                              at<u8 *>(following, 8) + 0x58, 0.0f, &prev, orientation());
        } else {
            const s32 last = at<s32>(spline, 0) * 0x58 - 0x58;
            TrackSpline_Frame(coasterNodes(), track(), nodes + last, at<u8 *>(following, 4), at<u8 *>(spline, 8) + last,
                              at<u8 *>(following, 8), 0.0f, &prev, orientation());
        }
        const float yaw = at<float>(track(), 0x40), pitch = at<float>(track(), 0x44), roll = at<float>(track(), 0x48);
        CoasterMesh_Finish(0);
        emit(blend, yaw, pitch, roll, &prev, 1, record);
        advance(at<u8 *>(record, 0xc));
        force = 0;
    }

    const float *position = positions + 1;
    s32 left = count - 1;
    if (left <= 0) return;
    const float one = 1.0f;   // 0x6fe108
    do {
        const float yaw2 = at<float>(track(), 0x4c), pitch2 = at<float>(track(), 0x50), roll2 = at<float>(track(), 0x54);
        float blend;
        spline = at<u8 *>(piece, 0x90);
        u8 *const following = at<u8 *>(next, 0x90);
        if (piece == next) {
            double whole;
            const double fraction = std::modf((X87Reg(*position) - X87Reg(global<float>(0x0079240c))).v, &whole);   // CRT modf (0x67b280)
            s32 node = (s32)(long long)whole;   // __ftol
            float t = (float)fraction;
            if (node == at<s32>(spline, 0) - 1) {
                t = (float)(X87Reg(fraction) - X87Reg(-1.0f));   // 0x6fe140
                --node;
            }
            const u8 *from = at<u8 *>(spline, 4) + node * 0x58;
            const u8 *to = at<u8 *>(following, 4) + (node + 1) * 0x58;
            TrackSpline_Frame(coasterNodes(), track(), from, to, at<u8 *>(spline, 8) + node * 0x58,
                              at<u8 *>(following, 8) + (node + 1) * 0x58, t, &cur, orientation());
            blend = (float)(X87Reg(at<float>(from, 8)) * (X87Reg(one) - X87Reg(t)) + X87Reg(at<float>(to, 8)) * X87Reg(t));
        } else {
            const X87Reg t = X87Reg(*position) - X87Reg(global<float>(0x0079240c));
            const s32 last = at<s32>(spline, 0) * 0x58 - 0x58;
            const u8 *from = at<u8 *>(spline, 4) + last;
            const u8 *to = at<u8 *>(following, 4);
            TrackSpline_Frame(coasterNodes(), track(), from, to, at<u8 *>(spline, 8) + last, at<u8 *>(following, 8), (float)t, &cur,
                              orientation());
            const X87Reg t2 = X87Reg(*position) - X87Reg(global<float>(0x0079240c));
            blend = (float)(X87Reg(at<float>(from, 8)) * (X87Reg(one) - t2) + X87Reg(at<float>(to, 8)) * t2);
        }

        u32 moved = 0;
        if (!(at<float>(record, 0) >= *position)) {   // (below or unordered)
            moved = 1;
            do record += 0x10;
            while (!(at<float>(record, 0) >= *position));
        }
        const u8 *section = at<u8 *>(record, 0xc);
        const s32 points = at<s32>(section, 0);
        const u32 overflow = (s32)((u32)points * 2) > global<s32>(0x007923e0) || (s32)(at<u32>(section, 4) * 2) > global<s32>(0x00792410);
        if (overflow || moved || force) {
            const float carried = at<float>(track(), 0x34);
            if (overflow) CoasterMesh_Finish(0);
            emit(carried, yaw2, pitch2, roll2, &prev, overflow, record);
            advance(at<u8 *>(record, 0xc));
        }

        const X87Reg dx = X87Reg(prev.x) - X87Reg(cur.x);
        const X87Reg dy = X87Reg(prev.y) - X87Reg(cur.y);
        const X87Reg dz = X87Reg(prev.z) - X87Reg(cur.z);
        const float dyf = (float)dy, dzf = (float)dz;
        const X87Reg sum = dx * X87Reg(dx.v) + dy * X87Reg(dyf) + X87Reg(dzf) * X87Reg(dzf);
        at<float>(track(), 0x38) = (float)X87_SubR(X87_Sqrt(sum) * X87Reg(-0.166666672f), X87Reg(at<float>(track(), 0x38)));   // 0x6fe150
        emit(blend, at<float>(track(), 0x40), at<float>(track(), 0x44), at<float>(track(), 0x48), &cur, 0, record);

        section = at<u8 *>(record, 0xc);
        const s32 indices = at<s32>(section, 4);
        if (indices > 0) {
            const u8 *entry = at<u8 *>(section, 0xc);
            for (s32 k = indices; k != 0; --k, entry += 0xc) {
                u8 *const list = global<u8 *>(0x007925dc) + 0x1c * at<u16>(entry, 4);
                const u32 base = global<u32>(0x0079c600);
                const u32 a = at<u16>(entry, 6), b = at<u16>(entry, 8);
                const u32 i0 = a - points + base, i1 = a + base, i2 = b - points + base, i3 = b + base;
                u32 low = at<u32>(list, 0x14);
                if (low >= i0) low = i0;
                if (low >= i2) low = i2;
                at<u32>(list, 0x14) = low;
                u32 high = at<u32>(list, 0x18);
                if (high <= i1) high = i1;
                if (high <= i3) high = i3;
                at<u32>(list, 0x18) = high;
                u8 *item;
                if (global<u32>(0x007923e4) == 0) {
                    item = CoasterList_Append(list);
                    at<u16>(item, 2) = (u16)i0;
                    at<u16>(item, 4) = (u16)i1;
                    at<u16>(item, 6) = (u16)i2;
                    item = CoasterList_Append(list);
                    at<u16>(item, 2) = (u16)i3;
                    at<u16>(item, 4) = (u16)i2;
                    at<u16>(item, 6) = (u16)i1;
                } else {
                    item = CoasterList_Append(list);
                    at<u16>(item, 4) = (u16)i1;
                    at<u16>(item, 2) = (u16)i0;
                    at<u16>(item, 6) = (u16)i3;
                    item = CoasterList_Append(list);
                    at<u16>(item, 2) = (u16)i3;
                    at<u16>(item, 4) = (u16)i2;
                    at<u16>(item, 6) = (u16)i0;
                }
                global<u32>(0x007923e4) = global<u32>(0x007923e4) == 0;
            }
        }

        section = at<u8 *>(record, 0xc);
        const s32 placed = at<s32>(section, 0);
        const u32 listed = at<u32>(section, 4) * 2;
        global<s32>(0x0079c600) += placed;
        global<s32>(0x007923e0) -= placed;
        global<s32>(0x007925c4) += placed;
        global<s32>(0x007925d4) -= placed;
        global<u32>(0x007923e8) += listed;
        global<u32>(0x00792410) -= listed;
        at<float>(track(), 0x34) = blend;
        std::memcpy(&prev, &cur, sizeof prev);
        ++position;
    } while (--left != 0);
}
}
