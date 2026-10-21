#include "trackmesh.h"

#include <cmath>
#include <cstdint>
#include <cstring>

#include "../math/x87.h"
#include "helpers.h"
#include "lists.h"
#include "nodes.h"
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

// The spline value at a node index and fraction as 0x4402d0 inlines it: CoasterNodes_SplineAt's, but the spline node's +0 read
// unsigned (fild of it and a zero dword).
float splineValue(float at_) {
    const u8 *const nodes = at<u8 *>(global<u8 *>(0x00792218), 0x9c);
    s32 whole;
    Float_SplitWhole(at_, &whole);
    const u8 *node = at<u8 *>(nodes, whole * 0x38 + 0x2c);
    for (const u8 *next = at<u8 *>(node, 0x18); next && !(at<float>(next, 8) >= at_); next = at<u8 *>(node, 0x18)) node = next;
    const u8 *const next = at<u8 *>(node, 0x18);
    const X87Reg fraction =
        next ? (X87Reg(at_) - X87Reg(at<float>(node, 8))) / (X87Reg(at<float>(next, 8)) - X87Reg(at<float>(node, 8))) : X87Reg(0.0);
    return (float)(X87Reg((double)at<u32>(node, 0)) + fraction);
}

float splineAt(float at_) { return (float)CoasterNodes_SplineAt(at<u8 *>(global<u8 *>(0x00792218), 0x9c), at_); }

u32 bits(float f) {
    u32 b;
    std::memcpy(&b, &f, 4);
    return b;
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

void __cdecl TrackMesh_PlaceSections() {
    u8 *const track = global<u8 *>(0x00790fe4);
    for (u32 o = 0x38; o <= 0x54; o += 4) at<u32>(track, o) = 0;
    at<u32>(track, 0x58) = 0x7f7fffff;
    TrackOrientation *const orient = orientation();
    u8 *const coaster = global<u8 *>(0x00792218);
    u8 *const build = at<u8 *>(coaster, 4);
    global<u32>(0x00792600) = 0;
    global<u32>(0x00792604) = 0;
    const u32 first = at<u32>(at<u8 *>(build, 0x78), 0x20);
    global<u32>(0x007925ec) = 0x4b189680;   // 1e7
    global<u32>(0x0079260c) = first;
    global<u32>(0x007925e8) = 0x4b189680;
    global<u32>(0x007925f4) = at<u32>(build, 0x80);
    global<u32>(0x007925f0) = 0;
    global<u32>(0x007923f4) = 0x4b189680;
    global<u32>(0x007923f0) = 0x4b189680;
    global<u32>(0x007923f8) = 0;
    global<u32>(0x007923fc) = at<u32>(build, 0x84);

    u8 *cur = reinterpret_cast<u8 *>((uintptr_t)0x00792600);
    u8 *open = nullptr;
    u32 blending = 0, previous = first;
    float p = 0.0f;
    if (!Coaster_IsPastEnd(coaster, 0.0f)) {
        do {
            Vec3 point;
            Coaster_FrameAt(coaster, p, &point, orient);
            orient->roll = (float)std::fmod((double)orient->roll, 3.1415927410125732);   // _CIfmod (0x6fe158)
            const u32 zone = Coaster_FindZone(build, bits(p), reinterpret_cast<const float *>(orient), 0.0f);
            if (zone != previous) {
                u32 tag = zone;
                if (build[0x74] & 1) {
                    u8 *found = nullptr;
                    const u8 *box = at<u8 *>(build, 0x78);
                    const u32 boxes = at<u32>(build, 0x7c);
                    for (u32 i = 0; i < boxes; ++i, box += 0x2c) {
                        if (at<u32>(box, 0x24) == previous && at<u32>(box, 0x28) == zone) {
                            found = at<u8 *>(box, 0x20);
                            break;
                        }
                    }
                    at<float>(cur, 4) = p;
                    at<float>(cur, 0) = splineAt(p);
                    cur += 0x10;
                    if (found) {
                        if (at<float>(found, 0x14) > 0.0f) {
                            tag = (u32)(uintptr_t)found;
                            if (blending) {
                                at<float>(open, 4) = p;
                                at<float>(open, 0) = splineValue(p);
                            }
                            const float ahead = (float)(X87Reg(p) + X87Reg(at<float>(found, 0x14)));
                            at<float>(cur, 4) = ahead;
                            blending = 1;
                            at<u32>(cur, 8) = 0x3f800000;
                            at<float>(cur, 0) = splineValue(ahead);
                            open = cur;
                        } else {
                            at<u8 *>(cur, 0xc) = found;
                            const X87Reg ahead = X87Reg(p) + X87Reg(at<float>(found, 0x14));
                            if (open && !(ahead.v >= (double)at<float>(open, 4))) {
                                if (blending) {
                                    const X87Reg before = X87Reg(std::fabs(at<float>(at<u8 *>(open, 0xc), 0x14)));
                                    const X87Reg length = X87Reg(std::fabs(at<float>(found, 0x14)));
                                    X87Reg share = X87_SubR(X87Reg(at<float>(open, 4)) - before, X87Reg(p)) / (length + before);
                                    if (!(share.v > 0.0)) share = X87Reg(0.0);
                                    if (!(share.v < 1.0)) share = X87Reg(1.0);
                                    at<float>(open, 8) = (float)share;
                                    at<float>(cur, 8) = (float)share;
                                    const float start = (float)X87_SubR(before * (X87Reg(1.0f) - share), X87Reg(at<float>(open, 4)));
                                    at<float>(open, 4) = start;
                                    at<float>(open, 0) = splineAt(start);
                                    at<float>(cur, 4) = p;
                                    at<float>(cur, 0) = splineValue(p);
                                } else {
                                    at<float>(cur, 4) = p;
                                    at<u32>(cur, 8) = 0x3f800000;
                                    at<float>(cur, 0) = splineValue(p);
                                }
                            } else {
                                const float end = (float)(X87Reg(p) + X87Reg(at<float>(found, 0x14)));
                                at<float>(cur, -0xc) = end;
                                at<float>(cur, -0x10) = splineValue(end);
                                at<float>(cur, 4) = p;
                                at<float>(cur, 0) = splineAt(p);
                                at<u32>(cur, 8) = 0x3f800000;
                            }
                            cur += 0x10;
                            blending = 0;
                            open = cur;
                        }
                    }
                } else {
                    at<float>(cur, 4) = p;
                    at<float>(cur, 0) = splineValue(p);
                    cur += 0x10;
                }
                previous = zone;
                at<u32>(cur, 0xc) = tag;
            }
            if (blending && p >= at<float>(open, 4)) {
                at<float>(cur, 4) = p;
                const float value = splineValue(p);
                cur += 0x10;
                blending = 0;
                at<float>(cur, -0x10) = value;
                at<u32>(cur, 0xc) = zone;
            }
            p = (float)(X87Reg(p) - X87Reg(-0.25f));   // 0x6fe160
        } while (!Coaster_IsPastEnd(coaster, p));
    }
    at<u32>(cur, 0) = 0x47c35000;   // 1e5
    at<float>(cur, 4) = (float)(X87Reg(p) - X87Reg(-0.25f) - X87Reg(-1.0f));   // 0x6fe160, 0x6fe140
}
}
