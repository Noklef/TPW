#include "rideresource.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#include "../core/trivial.h"
#include "../files/gamepaths.h"
#include "../files/quickload.h"
#include "../world/objectdefs.h"
#include "faces.h"
#include "hoardingmesh.h"
#include "parts.h"
#include "ridebase.h"
#include "ridemodel.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const RideResourceEnv kPorts = {reinterpret_cast<decltype(RideResourceEnv::modelPath)>((uintptr_t)0x0046e650),
                                Quickload_LoadRide,
                                RideModel_Load,
                                RideBase_Prepare,
                                Face_MarkTextures,
                                HoardingMesh_Build,
                                Model_MarkFlag80Faces,
                                reinterpret_cast<decltype(RideResourceEnv::heightMap)>((uintptr_t)0x00451640),
                                GamePaths_FontsFolder,
                                ObjectDefs_FindAt410,
                                reinterpret_cast<decltype(RideResourceEnv::sign)>((uintptr_t)0x00467a80)};
const RideResourceEnv *g_env = &kPorts;

const u32 kFlags[][2] = {{0x10, 2},       {0x20, 0x10},     {0x40, 0x20},      {2, 0x40},          {0x100, 1},         {0x200, 0x100},    {4, 0x80},
                         {0x4000, 0x400}, {0x10000, 0x800}, {0x40000, 0x1000}, {0x100000, 0x2000}, {0x200000, 0x4000}, {0x400000, 0x8000}, {0x1000000, 0x20000}};

bool HasExtension(const char *dot) { return dot && dot[1] != '\\'; }

// Part i of the model: of 0xa0 bytes in the list at +0x70 up to the count +0x44, then of 0x58 bytes at +0x74.
u8 *Part(const u8 *inner, u32 i) {
    const u32 split = at<u16>(inner, 0x44);
    return i < split ? at<u8 *>(inner, 0x70) + i * 0xa0 : at<u8 *>(inner, 0x74) + (i - split) * 0x58;
}

void MarkParts(const u8 *model, u32 flags) {
    const u8 *const holder = at<u8 *>(model, 8);
    const u8 *const inner = at<u8 *>(holder, 4);
    const u8 *const first = at<u8 *>(holder, 0x2c);
    u32 count = first ? at<u16>(first, 0x58) : 0;
    const u8 *const extras = at<u8 *>(holder, 0x28);
    const u8 *entry = at<u8 *>(inner, 0x7c);
    u8 *extra = extras ? at<u8 *>(extras, 4) : nullptr;
    for (u32 i = 0; i < at<u16>(inner, 0x48); ++i, entry += 0x14) {
        const u8 *const part = Part(inner, i + at<u16>(inner, 0x46));
        const u32 kind = at<u32>(entry, 0);
        if (kind & 2) count += at<u16>(part, 0x58);
        if (extra) {
            if (kind & 0x580f00 || flags & 0x400000) at<u32>(extra, 0) |= 8;
            extra += 0x14;
        }
    }
    if (count > gRideresource_79f7d0) gRideresource_79f7d0 = count;
}

void FinishParts(const u8 *model) {
    u8 *const inner = at<u8 *>(at<u8 *>(model, 8), 4);
    for (u32 i = 0; i < at<u16>(inner, 0x42); ++i) {
        u8 *const part = Part(inner, i);
        if (at<u32>(part, 0) & 0x200) at<u32>(part, 0) |= 0x80000010;
        if (at<u32>(part, 0) & 0x20040) at<u32>(inner, 0x30) |= 0x80;
        if (!(at<u32>(part, 0) & 0x200)) {
            u8 *texture = at<u8 *>(part, 0x6c);
            for (u32 j = 0; j < at<u16>(part, 0x5a); ++j, texture += 0x10) {
                const u8 *const data = at<u8 *>(texture, 0);
                if (data && at<u8>(data, 0) & 0x80) at<u8>(texture, 0xc) |= 2;
            }
        }
        const u8 *const chain = at<u8 *>(part, 0xc);
        bool all = true;
        for (const u8 *link = chain; link; link = at<u8 *>(link, 8))
            if (!(at<u8>(link, 1) & 2) || at<u32>(link, 0xc)) {
                all = false;
                break;
            }
        if (all && chain) at<u32>(part, 0) |= 0x20;
    }
    u8 *a = at<u8 *>(inner, 0x50);
    const u8 *b = at<u8 *>(inner, 0x54);
    for (u32 n = at<u16>(inner, 0x36); n; --n, a += 8, b += 0x10) {
        if (at<u8>(a, 0) & 0xc0) at<u32>(inner, 0x30) |= 0x1000;
        if (at<u8>(b, 0) & 1) at<u8>(a, 0) |= 0x40;
    }
}

}  // namespace

void RideResource_SetEnv(const RideResourceEnv *env) { g_env = env ? env : &kPorts; }

extern "C" u32 __cdecl Ride_LoadResource(const char *folder, const char *name, const u8 *cells, const u8 *grid, u32 flags, u32 id, u32 layers, const u8 *def, u32 signArg, u32 width, u32 height) {
    if (!width) width = cells ? at<u32>(cells, 0) : 0;
    if (!height) height = cells ? at<u32>(cells, 4) : 0;
    u32 loadFlags = 0;
    for (const auto &pair : kFlags)
        if (flags & pair[0]) loadFlags |= pair[1];
    if (!layers) layers = 1;
    char path[0x104], where[0x104], modelName[0x104], text[0x104];
    std::strcpy(path, name);
    if (!HasExtension(std::strrchr(path, '.'))) std::strcat(path, ".md2");
    g_env->modelPath(where);
    u32 answer = g_env->quickload(path, folder, where);
    u8 *model = nullptr;   // (set by the load unless it fails early: the original then uses the stack's value, sic)
    if (answer == 1) {
        answer = g_env->load(name, loadFlags, &model, modelName, layers);
        at<u32>(model, 4) |= 1;
        if (answer == 1 && flags & 0x20000) {
            std::sprintf(text, "p%s", name);
            g_env->load(text, 0, &at<u8 *>(model, 0xd0), nullptr, answer);
        }
    }
    if (answer != 1) return 0;
    at<u32>(model, 0xf8) = id;
    at<u32>(model, 0xf0) = width;
    at<u32>(model, 0xf4) = height;
    if (flags & 1) g_env->prepare(model, global<const u8 *>(0x007a0854), cells, width, height);
    if (flags & 0x2000) {
        const u8 *const holder = at<u8 *>(model, 8);
        const u8 *const face = at<u8 *>(holder, 0x2c);
        g_env->markTextures((u32)(uintptr_t)holder, face ? face : at<u8 *>(at<u8 *>(holder, 4), 0x78), 1);
        if (const u8 *const other = at<u8 *>(model, 0xd0)) {
            const u8 *const otherHolder = at<u8 *>(other, 8);
            g_env->markTextures((u32)(uintptr_t)otherHolder, at<u8 *>(at<u8 *>(otherHolder, 4), 0x78), 1);
        }
    }
    MarkParts(model, flags);
    if (flags & 8 && grid) g_env->hoarding(at<const char *>(Obj_Self(reinterpret_cast<u8 *>(0x00786b68)), 8), model, grid);
    if (flags & 0x80) g_env->markFaces(reinterpret_cast<const u8 *const *>(model + 8));
    if (flags & 0x400) {
        char *const dot = std::strrchr(modelName, '.');
        if (HasExtension(dot))
            std::strcpy(dot, ".hmp");
        else
            std::strcat(modelName, ".hmp");
        g_env->heightMap(model, width, height, modelName, cells, flags & 0x800);
        if (u8 *const other = at<u8 *>(model, 0xd0)) at<u32>(other, 0xcc) = at<u32>(model, 0xcc);
    }
    if (g_env->fontsFolder(text) == 1) {
        if (def)
            g_env->sign(model, folder, name, text, def, signArg);
        else
            g_env->sign(model, folder, name, text, g_env->findDef(reinterpret_cast<u8 *>(0x007890a0), nullptr, static_cast<u16>(id)), 0);
    }
    FinishParts(model);
    const u32 slot = ++global<u32>(0x007a439c);
    global<u8 *>(0x007ae6a8 + slot * 4) = model;
    return slot;
}
