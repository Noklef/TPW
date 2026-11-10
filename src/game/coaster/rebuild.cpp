#include "rebuild.h"

#include <cstdint>

#include "../math/matrix.h"
#include "../math/x87.h"
#include "../model/holder.h"
#include "carpose.h"
#include "contacts.h"
#include "keys.h"
#include "lists.h"
#include "nodes.h"
#include "orient.h"
#include "testrun.h"
#include "trackmesh.h"
#include "coastersupdate.h"
#include "coastersload.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// CoasterSearch_Reset's stack arguments past the first are unused.
const CoasterRebuildEnv kPorts = {&Coaster_PoseCars,
                                  &Track_RefreshOrientations,
                                  &Coaster_BuildNodes,
                                  &Coaster_TestRun,
                                  &Contacts_UpdateBounds,
                                  reinterpret_cast<void(__cdecl *)(u8 *)>((uintptr_t)0x00439ab0),
                                  &Owner_DisposeChildren,
                                  &TrackMesh_PlaceSections,
                                  reinterpret_cast<void(__cdecl *)(u8 *, u8 *, u32)>(&CoasterSearch_Reset),
                                  &Keys_MergeSamples,
                                  &TrackMesh_EmitPiece,
                                  &CoasterMesh_Finish};
const CoasterRebuildEnv *g_env = &kPorts;

float *const kKeysOut = reinterpret_cast<float *>((uintptr_t)0x00792418);

void CopyPartFlags(const u8 *type) {
    u32 offset = 0;
    for (u32 k = 0; k < at<u16>(at<u8 *>(type, 0x1c), 4); k++, offset += 0x1c)
        at<u32>(global<u8 *>(0x007925dc), offset) = (at<u32>(at<u8 *>(at<u8 *>(type, 0x1c), 0x10), k * 0x10) >> 2) & 1;
}

u8 *LastNode(const u8 *piece, u32 which) {
    const u8 *const list = at<const u8 *>(piece, 0x90);
    return at<u8 *>(list, which) + at<u32>(list, 0) * 0x58 - 0x58;
}

void LinkNodes(const u8 *coaster) {
    u32 number = 0;
    for (u8 *piece = at<u8 *>(coaster, 0x54); piece; piece = at<u8 *>(piece, 0x64)) {
        const u8 *const list = at<const u8 *>(piece, 0x90);
        for (u32 i = 0; i < at<u32>(at<const u8 *>(piece, 0x90), 0); i++) {
            u8 *const a = at<u8 *>(list, 4) + i * 0x58, *const b = at<u8 *>(list, 8) + i * 0x58;
            if (i) {
                at<u8 *>(a, 0x14) = a - 0x58;
                at<u8 *>(b, 0x14) = b - 0x58;
            } else if (const u8 *const last = at<const u8 *>(piece, 0x68)) {
                at<u8 *>(a, 0x14) = LastNode(last, 4);
                at<u8 *>(b, 0x14) = LastNode(last, 8);
            } else {
                at<u8 *>(a, 0x14) = nullptr;
                at<u8 *>(b, 0x14) = nullptr;
            }
            if (i < at<u32>(list, 0) - 1) {
                at<u8 *>(a, 0x18) = a + 0x58;
                at<u8 *>(b, 0x18) = b + 0x58;
            } else if (const u8 *const next = at<const u8 *>(piece, 0x64)) {
                at<u8 *>(a, 0x18) = at<u8 *>(at<u8 *>(next, 0x90), 4);
                at<u8 *>(b, 0x18) = at<u8 *>(at<u8 *>(next, 0x90), 8);
            } else {
                at<u8 *>(a, 0x18) = nullptr;
                at<u8 *>(b, 0x18) = nullptr;
            }
            at<u32>(a, 0) = number;
            at<u32>(b, 0) = number;
            number++;
        }
    }
}

X87Reg DistanceRatio(const u8 *piece, const float *node, const float *end) {
    const float *const place = reinterpret_cast<const float *>(piece + 0x4c);
    const float toNode = (float)X87Reg(Vec3_Distance(place, node));
    return toNode / X87Reg(Vec3_Distance(place, end));
}

// The slopes. Returns what was left in the last-piece slot.
const float *Slopes(u8 *piece, const float *slot) {
    for (; piece; piece = at<u8 *>(piece, 0x64)) {
        const u32 bank = !(at<u8>(at<u8 *>(piece, 0x100), 4) & 1) && !(at<u32>(piece, 8) & 0x2000) ? at<u32>(at<u8 *>(piece, 0x74), 0) : 0;
        const u8 *const last = at<const u8 *>(piece, 0x68), *const next = at<const u8 *>(piece, 0x64);
        slot = last ? reinterpret_cast<const float *>(last + 0x4c) : nullptr;
        const float *const ahead = next ? reinterpret_cast<const float *>(next + 0x4c) : nullptr;
        const u8 *const list = at<const u8 *>(piece, 0x90);
        u8 *const a = at<u8 *>(list, 4), *const b = at<u8 *>(list, 8);
        for (u32 j = 0; j < at<u32>(at<const u8 *>(piece, 0x90), 0); j++) {
            const float *const node = reinterpret_cast<const float *>(a + j * 0x58 + 0x40);
            X87Reg slope = 0.0f;
            if (j < bank) {
                if (slot) {
                    const double ratio = DistanceRatio(piece, node, slot).v;
                    slope = -ratio;
                }
            } else if (j > bank && ahead) {
                slope = DistanceRatio(piece, node, ahead);
                if (!(slope.v > 0.0))
                    slope = 0.0f;
                if (slope.v >= 1.0)   // (a NaN cannot get here)
                    slope = 1.0f;
            }
            at<float>(a, j * 0x58 + 0xc) = (float)slope;
            at<float>(b, j * 0x58 + 0xc) = (float)slope;
        }
    }
    return slot;
}

void AddToRun(u32 amount) { global<float>(0x0079240c) = (float)(X87Reg((double)amount) + global<float>(0x0079240c)); }

}  // namespace

void CoasterRebuild_SetEnv(const CoasterRebuildEnv *env) { g_env = env ? env : &kPorts; }

extern "C" void __cdecl Coaster_Rebuild(u8 *coaster) {
    gCoastersupdate_792218 = coaster;
    u8 *const type = at<u8 *>(coaster, 4);
    if (type != global<u8 *>(0x007923ec)) {
        CopyPartFlags(type);
        global<u8 *>(0x007923ec) = type;
    }
    g_env->pose();
    LinkNodes(coaster);
    const float *const slot = Slopes(at<u8 *>(coaster, 0x54), reinterpret_cast<const float *>(coaster));

    g_env->refresh();
    g_env->buildNodes(gCoastersupdate_792218);
    g_env->testRun(gCoastersupdate_792218);
    g_env->updateBounds(gCoastersupdate_792218);
    g_env->f439ab0(gCoastersupdate_792218);
    g_env->dispose(gCoastersupdate_792218);
    g_env->placeSections();

    u8 *const track = gCoastersload_790fe4;
    u8 *piece = at<u8 *>(track, 0x18);
    global<u32>(0x00792414) = 0x00792600;
    global<u32>(0x0079240c) = 0;
    global<u32>(0x007923e4) = 0;
    at<u32>(track, 0x34) = 0;
    global<u32>(0x007925f8) = 0;
    at<u32>(track, 0x38) = 0;
    g_env->searchReset(track + 0x3c, track, 0);

    bool emitting = true;
    u32 restart = (u32)(uintptr_t)slot;
    u8 *next = piece ? at<u8 *>(piece, 0x64) : nullptr;
    while (piece && next) {
        const u32 flags = at<u32>(piece, 8);
        if (flags & 0x10000) {
            emitting = false;
        } else if (flags & 0x20000) {
            emitting = true;
            restart = 1;
        }
        const u8 *const list = at<const u8 *>(piece, 0x90);
        const u32 count = at<u32>(list, 0);
        if (count > 1) {
            if (emitting) {
                const u32 samples = g_env->merge(piece, piece, global<const float *>(0x00792414), kKeysOut, (float)(double)(count - 1));
                g_env->emit(piece, piece, (s32)samples, kKeysOut, restart);
                restart = 0;
            } else {
                at<u32>(gCoastersload_790fe4, 0x34) = at<u32>(at<u8 *>(list, 4), count * 0x58 - 0x50);
            }
            AddToRun(at<u32>(at<const u8 *>(piece, 0x90), 0) - 1);
        }
        if (emitting) {
            const u32 samples = g_env->merge(piece, next, global<const float *>(0x00792414), kKeysOut, 1.0f);
            g_env->emit(piece, next, (s32)samples, kKeysOut, restart);
            restart = 0;
        } else {
            at<u32>(gCoastersload_790fe4, 0x34) = at<u32>(at<u8 *>(at<u8 *>(next, 0x90), 4), 8);
        }
        global<float>(0x0079240c) = (float)(X87Reg(global<float>(0x0079240c)) - -1.0f);
        piece = next;
        next = at<u8 *>(next, 0x64);
    }

    g_env->finish(1);
    at<u32>(gCoastersupdate_792218, 0) &= ~0x4000u;
}
