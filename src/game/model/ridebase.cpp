#include "ridebase.h"

#include <cstdint>
#include <cstring>

#include "../core/globalmem.h"
#include "../math/matrix.h"
#include "basecells.h"
#include "faces.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const RideBaseEnv kPortEnv = {Frame_FitToGrid, RideBase_FillCells, GlobalMem_Alloc};
const RideBaseEnv *g_env = &kPortEnv;

// The original's frame, in its layout: a fit's cursor and its buffer are 0x60 apart.
struct Fit {
    u16 *cursor;
    u32 unused1[3];
    s32 bestCount;
    u32 unused2[3];
    GridFitMatrix matrix;
    u16 buffer[0x100];
};

bool Set(const u8 *cells, s32 offset) { return (at<u8>(cells, (u32)offset) & 0xf) != 0; }

void MarkEdges(const u8 *cells, s32 width, s32 height) {
    for (s32 y = 0; y < height; ++y) {
        for (s32 x = 0; x < width; ++x) {
            const s32 cell = y * 0xa0 + x * 8;
            const bool self = Set(cells, cell + 8);
            u32 edges = global<u32>(0x007afe20 + 4 * (y * 20 + x));
            if (self != (y < height - 1 && Set(cells, cell + 0xa8))) edges |= 4;
            if (self != (x > 0 && Set(cells, cell))) edges |= 2;
            if (self != (y > 0 && Set(cells, cell - 0x98))) edges |= 1;
            if (self != (x < width - 1 && Flags_AnyLow4(at<u32>(cells, (u32)(cell + 0x10))) != 0)) edges |= 8;
            global<u32>(0x007afe20 + 4 * (y * 20 + x)) = edges;
        }
    }
}

}  // namespace

void RideBase_SetEnv(const RideBaseEnv *env) { g_env = env ? env : &kPortEnv; }

void __cdecl RideBase_Prepare(const u8 *object, const u8 *owner, const u8 *cells, s32 width, s32 height) {
    u8 *holder = at<u8 *>(object, 8);
    const u8 *model = at<u8 *>(holder, 4);

    std::memset(reinterpret_cast<void *>((uintptr_t)0x007afe20), 0, 0x190 * 4);
    const bool edges = !(at<u8>(model, 0x30) & 0x80) && cells;
    global<s32>(0x007afe18) = edges ? width : 0;
    global<s32>(0x007b0460) = edges ? height : 0;
    if (edges) MarkEdges(cells, width, height);

    Fit fit;
    fit.bestCount = 0;
    s32 hits = 0;
    u8 *best = nullptr;
    const u8 *node = at<u8 *>(model, 0x78);

    if (owner && at<u8 *>(owner, 0x6c)) {
        Matrix_Identity(fit.matrix.m);
        fit.cursor = fit.buffer;
        g_env->fit(node, const_cast<u8 *>(node), &fit.cursor, fit.matrix, at<u8 *>(owner, 0x6c), 0, &hits, 0.0f, &best, &fit.bestCount,
                   (at<u32>(model, 0x30) >> 8) & 1, nullptr, 0);
        if (best) at<u32>(best, 0) |= 0x4000;
        at<u8 *>(holder, 0x2c) = best;
    }

    at<u16>(holder, 0xc) = (u16)hits;
    if (hits > 0) {
        void *copy = g_env->alloc(hits * 2, 1);
        at<void *>(holder, 0x14) = copy;
        if (copy) std::memcpy(copy, fit.buffer, (u32)hits * 2);
    }

    u8 *frame = at<u8 *>(model, 0x70);
    for (u32 left = at<u16>(model, 0x44); left != 0; --left, frame += 0xa0) {
        if (!(at<u8>(frame, 0) & 0x40) || frame == at<u8 *>(holder, 0x2c)) continue;
        Matrix_Identity(fit.matrix.m);
        hits = 0;
        fit.cursor = fit.buffer;
        g_env->fit(node, const_cast<u8 *>(node), &fit.cursor, fit.matrix, at<u8 *>(owner, 0x6c), 0, &hits, 0.0f, &best, &fit.bestCount, 0, frame, 1);
        if (hits > 0) {
            u8 *block = static_cast<u8 *>(g_env->alloc(hits * 2 + 8, 0));
            at<u8 *>(frame, 0x98) = block;
            at<u8 *>(block, 4) = block + 8;
            at<s32>(block, 0) = hits;
            std::memcpy(block + 8, fit.buffer, (u32)hits * 2);
        }
    }

    if (cells) {
        const u32 flags = at<u32>(at<u8 *>(holder, 4), 0x30);
        if ((flags & 8) && (at<u8 *>(holder, 0x2c) || (flags & 0x80))) g_env->fillCells(object, cells, width, height, flags & 0x80);
    }
}
