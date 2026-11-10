#include "ground.h"

#include <cstdint>
#include <cstring>

#include "../core/seqs.h"
#include "../gfx/texset.h"
#include "../gfx/texture.h"
#include "../core/vwrap.h"
#include "procedural.h"
#include "../math/x87.h"
#include "../render/batch.h"
#include "../render/clipcodes.h"
#include "../render/clippoly.h"
#include "../render/drawbuffer.h"
#include "../render/fixedverts.h"
#include "../render/meshlist.h"
#include "../render/transformer.h"
#include "gridquad.h"
#include "../render/renderer.h"
#include "../gfx/texcache.h"
#include "../core/staticobjects.h"

using gridquad::Fan;
using gridquad::FacingClipped;
using gridquad::FacingOpen;
using gridquad::FacingOpenFirst;
using gridquad::KeepTriangle;

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const TerrainGroundEnv kPortEnv = {&ListNodes_Drain, &Meshes_SetView, &TerrainTex_LightGrid, &Bands_CollectTextures, &Batch_Begin, &DrawBatch_Open, &Transformer_Transform, &ClipPoly_Vertices,
                                   &Batch_Fog, &Batch_TransformVertices, &FixedVerts_PackRounded, &Batch_Close};
const TerrainGroundEnv *g_env = &kPortEnv;

const u32 kPoint = 0x10;  // a point's bytes: x, y, z, w

// A record of vertices for the sort: its texture's id, first vertex, the texture's +0x44 and the two counts.
struct Run {
    u32 id;
    s32 start;
    u32 extra;
    s16 first, second;
};

// The points of a row into their columns of the frame's arrays, transformed and clip coded.
void FillRow(u8 *self, s32 row, u8 *points, u8 *codes) {
    const s32 first = at<s8>(self, row + 0x28);
    const s32 last = at<s8>(self, row + 0xa8);
    if (first >= 0x7f) return;
    const u8 *const grid = at<u8 *>(self, 0);
    const u8 *const heights = at<u8 *>(grid, 0x28);
    const float z = (float)(X87Reg((double)row) * at<float>(grid, 0x14) + at<float>(grid, 0xc));
    const u32 base = at<u32>(self, 8) * (u32)row;
    u8 *const start = points + (u32)first * kPoint;
    for (s32 column = first; column <= last; ++column) {
        u8 *const to = points + (u32)column * kPoint;
        at<float>(to, 0) = (float)(X87Reg((double)column) * at<float>(at<u8 *>(self, 0), 0x10) + at<float>(at<u8 *>(self, 0), 8));
        at<u32>(to, 4) = at<u32>(heights, (base + (u32)column) * 4);
        at<float>(to, 8) = z;
    }
    const u32 count = (u32)(last - first + 1);
    g_env->transform(global<void *>(0x0087b088), nullptr, (u32)(uintptr_t)start, (u32)(uintptr_t)start, kPoint, kPoint, 0, count);
    u32 result;
    ClipCodes_Run(global<u32>(0x00879a20), 0, reinterpret_cast<u8 *>(&result), codes + first, start, count, kPoint, 0xff);
}

u32 CellColour(const u8 *self, s32 row, s32 column, bool halved) {
    const u32 colour = at<u32>(self, 0x2968 + ((u32)row * 0x80 + (u32)column) * 4);
    return halved ? colour >> 1 & 0x7f7f7f : colour;
}

// The batches' indices, a batch for each run of an id. 0 or 1 to return; the batches' vertices are all in the first.
u32 DrawRuns(u8 *self, u8 *batch, Run *runs, s32 records, s32 vertices, u32 flags) {
    Bands_SortRecords(self, nullptr, reinterpret_cast<u8 *>(runs), 0, records - 1);
    for (s32 i = 0; i < records;) {
        s32 count = 1;
        for (s32 j = i + 1; j < records && runs[i].id == runs[j].id; ++j) ++count;
        if (batch) at<u32>(batch, 8) = runs[i].id;
        else if (!(batch = g_env->open(global<u8 *>(0x00879ab0), nullptr, 1, 4, 0x1c4, flags, runs[i].id))) return 1;
        at<u32>(batch, 0x34) = runs[i].extra;
        u16 *indices = at<u16 *>(batch, 0x2c);
        for (s32 k = 0; k < count; ++k, ++i) {
            indices = Fan(indices, runs[i].start, runs[i].first);
            indices = Fan(indices, runs[i].start + runs[i].first, runs[i].second);
        }
        at<u32>(batch, 0) = (at<u32>(batch, 0) & ~0x10u) | 0x100;
        at<s32>(batch, 0x20) = vertices;
        at<s32>(batch, 0x24) = (s32)((u32)(uintptr_t)indices - at<u32>(batch, 0x2c)) >> 1;
        const float depth = (float)X87Reg((double)(s32)((u32)vertices * 2000000u));
        if (!g_env->close(global<u8 *>(0x00879ab0), nullptr, depth)) return 0;
        batch = nullptr;
    }
    return 1;
}

const u8 *CellSet(const u8 *self, const u8 *cell) { return at<const u8 *>(at<const u8 *>(self, 4), 8 * (u32)at<u16>(cell, 2) + 4); }

AddrCalls g_prepareCalls;

}  // namespace

void TerrainGround_SetEnv(const TerrainGroundEnv *env) { g_env = env ? env : &kPortEnv; }
void TerrainGround_SetPrepareCalls(const AddrCall *table, u32 count) { g_prepareCalls.Set(table, count); }

extern "C" u32 __fastcall TerrainGround_PrepareTextures(u8 *self) {
    const u32 flags = global<u32>(0x8bd508);
    if (!(flags & 0xc0000000)) return 1;
    if (!(flags & 0x100)) {
        const u8 *const block = global<const u8 *>(0x879d54);
        memcpy(self + 0x128, block + 0xc4, 0x108);
        memcpy(self + 0x20, block + 0x1cc, 0x108);
    }
    const u32 format = gTerrainGround_PrepareTextures_7014c8[global<u32>((u32)(uintptr_t)&gTexSettings_8bd550 + 0x4)];
    if (at<s32>(self, 0x128) > at<s32>(self, 0x12c)) return 1;
    if ((global<u8>((u32)(uintptr_t)&gTexSettings_8bd550) & 4) && !at<u32>(self, 0x230)) global<u32>((u32)(uintptr_t)&gTexSettings_8bd550) &= ~4u;

    u32 records[0x9c4][5];   // the original's frame. Each: texture, its width and height, format, the cell's first word (only the first and last without a texture)
    s32 taken = 0;
    for (s32 y = at<s32>(self, 0x128); y < at<s32>(self, 0x12c); ++y) {
        for (s32 x = at<s8>(self, 0x130 + y); x < at<s8>(self, 0x1b0 + y); ++x) {
            const u8 *const map = at<const u8 *>(self, 0);
            u8 *const cell = at<u8 *>(map, 0x2c) + 4 * (u32)(at<s32>(map, 0x18) * y + x);
            if (at<u8>(cell, 0) & 1) continue;
            if ((global<u8>((u32)(uintptr_t)&gTexSettings_8bd550) & 4) && x >= at<s32>(self, 0x234) && x <= at<s32>(self, 0x23c) && y >= at<s32>(self, 0x238) && y <= at<s32>(self, 0x240)) {
                if (taken >= 0x9c4) continue;
                at<u32>(self, 0x248 + taken * 4) = ((y - at<s32>(self, 0x238)) << 8) + x - at<s32>(self, 0x234);
                u32 *const record = records[taken];
                record[4] = at<u16>(cell, 0);
                at<u16>(cell, 0) &= 0xfeff;
                if (at<u8>(cell, 0) & 0x80) {
                    const u8 *const texture = reinterpret_cast<const u8 *>((uintptr_t)TexSet_Texture(CellSet(self, cell), nullptr, 0));
                    record[0] = (u32)(uintptr_t)texture;
                    record[1] = at<s16>(texture, 0x10);
                    record[2] = at<s16>(texture, 0x12);
                    record[3] = format;
                } else {
                    record[0] = 0;
                }
                ++taken;
            } else if (const u8 *const set = CellSet(self, cell)) {
                u8 *const kept = reinterpret_cast<u8 *>((uintptr_t)TexSet_Texture(set, nullptr, 0));
                if (kept) Texture_SetKeep(kept);
                g_prepareCalls.Th(0x0058c9f0, gTexcache_8c4b44, kept, 0x3f000000u);   // TexCache_TouchItem
            }
        }
    }
    if ((global<u8>((u32)(uintptr_t)&gTexSettings_8bd550) & 4) && taken > 0) g_prepareCalls.Th(0x00572440, at<u32>(self, 0x230), self + 0x248, records, taken);   // TerrainTexCache_Update
    at<u32>(self, 0x1296c) = taken;
    return 1;
}

extern "C" u32 __fastcall TerrainGround_Draw(u8 *self) {
    if (at<s32>(self, 0x128) > at<s32>(self, 0x12c) || !(global<u32>(0x008bd508) & 0xc0000000)) return 1;
    const u32 stride = gGround_7014b4;
    global<u32>(0x00879dec) = 0;
    g_env->drain(at<u8 *>(gRenderer_87a6d0, 0x2c));
    g_env->setView(gMeshlist_87a248, nullptr, reinterpret_cast<const u32 *>((uintptr_t)0x0087b090), 0);
    g_env->light(self);
    static const float kUvs[4][2] = {{0, 0}, {0, 1}, {1, 1}, {1, 0}};
    // (The frame's arrays, as big as the original's.)
    u32 ids[0x400];
    s32 idCount;
    static_assert(sizeof(Run) == 0x10, "the sort's records");
    u32 textures[0x4000];
    Run runs[0x4000];
    g_env->collect(self, nullptr, ids, &idCount, textures);
    u8 *const object = global<u8 *>(0x00879ab0);
    u8 *const batch = g_env->begin(object, nullptr, 1, 4, 0x1c4, global<u32>((u32)(uintptr_t)&gView_8bcbc8) & 0x2000 ? 0x101256 : 0x1256, ids[0]);
    if (!batch) return 1;

    u8 thisRow[0x800], lastRow[0x800], codes[0x80], lastCodes[0x80], quad[0xe0], source[0x2a0];
    s32 row = at<s32>(self, 0x128), vertices = 0, records = 0;
    u32 nextTexture = 0;
    FillRow(self, row, thisRow, codes);
    while (row < at<s32>(self, 0x12c)) {
        std::memcpy(lastRow, thisRow, sizeof lastRow);
        std::memcpy(lastCodes, codes, sizeof lastCodes);
        const s32 next = row + 1;
        FillRow(self, next, thisRow, codes);
        for (s32 column = at<s8>(self, row + 0x130); column < at<s8>(self, row + 0x1b0); ++column) {
            const u8 *const grid = at<u8 *>(self, 0);
            const u16 *const cell = reinterpret_cast<const u16 *>(at<u8 *>(grid, 0x2c) + (at<u32>(grid, 0x18) * (u32)row + (u32)column) * 4);
            if (*reinterpret_cast<const u8 *>(cell) & 1) continue;
            const u32 number = nextTexture++;
            const u8 a = lastCodes[column], b = lastCodes[column + 1], c = codes[column], d = codes[column + 1];
            if (a & b & c & d) continue;
            u32 turned = 0x03020100;
            Bytes4_Permute(reinterpret_cast<u8 *>(&turned), cell);
            const u8 *const places = reinterpret_cast<const u8 *>(&turned);
            const u8 *const texture = reinterpret_cast<const u8 *>((uintptr_t)textures[number]);
            const u32 id = texture && (at<u32>(texture, 8) & 0x1000) ? at<u32>(texture, 0x1c) : 0;
            const float scaleU = texture ? at<float>(texture, 0x20) : 0.0f, scaleV = texture ? at<float>(texture, 0x24) : 0.0f;
            const X87Reg offsetU = X87Reg(texture ? at<float>(texture, 0x28) : gTerrainGround_Draw_701520);
            const X87Reg offsetV = X87Reg(texture ? at<float>(texture, 0x2c) : gTerrainGround_Draw_701520);
            const u32 extra = texture ? at<u32>(texture, 0x44) : 0xffffffff;
            const bool halved = !(global<u32>((u32)(uintptr_t)&gView_8bcbc8) & 0x800) && (reinterpret_cast<const u8 *>(cell)[1] & 4);

            struct Corner {
                const u8 *from;
                u32 colour;
                u8 place;
            };
            const Corner corners[4] = {{lastRow + (u32)column * kPoint, CellColour(self, row, column, halved), places[0]},
                                       {thisRow + (u32)column * kPoint, CellColour(self, next, column, halved), places[1]},
                                       {thisRow + (u32)(column + 1) * kPoint, CellColour(self, next, column + 1, halved), places[2]},
                                       {lastRow + (u32)(column + 1) * kPoint, CellColour(self, row, column + 1, halved), places[3]}};
            const u32 clipFlags = (u32)(a | b | c | d);
            const u32 offset = (u32)vertices * stride;
            u8 *const out = clipFlags ? quad : at<u8 *>(batch, 0x28) + offset;
            const u32 startCorner = *reinterpret_cast<const u8 *>(cell) & 2 ? 0 : 1;
            for (u32 k = 0; k < 4; ++k) {
                const Corner &corner = corners[(startCorner + k) & 3];
                u8 *const to = out + k * stride;
                std::memcpy(to, corner.from, kPoint);
                at<u32>(to, 0x10) = corner.colour;
                at<float>(to, 0x18) = (float)(X87Reg(kUvs[corner.place][0]) * scaleU + offsetU);
                at<float>(to, 0x1c) = (float)(X87Reg(kUvs[corner.place][1]) * scaleV + offsetV);
            }
            const u8 *const v1st = out, *const v2nd = out + stride, *const v3rd = out + 2 * stride, *const v4th = out + 3 * stride;
            Run &run = runs[records];
            if (clipFlags) {
                u32 countA = 0, countB = 0;
                KeepTriangle(v1st, v2nd, v3rd);
                if (FacingClipped()) {
                    std::memcpy(source, v1st, stride);
                    std::memcpy(source + stride, v2nd, stride);
                    std::memcpy(source + 2 * stride, v3rd, stride);
                    countA = 3;
                    g_env->clip((s32)clipFlags, at<u8 *>(batch, 0x28) + offset, source, &countA, (s32)stride);
                }
                KeepTriangle(v1st, v3rd, v4th);
                if (FacingClipped()) {
                    std::memcpy(source, v1st, stride);
                    std::memcpy(source + stride, v3rd, stride);
                    std::memcpy(source + 2 * stride, v4th, stride);
                    countB = 3;
                    g_env->clip((s32)clipFlags, at<u8 *>(batch, 0x28) + (countA + (u32)vertices) * stride, source, &countB, (s32)stride);
                }
                if ((s32)(countA + countB) <= 0) continue;
                run = {id, vertices, extra, (s16)countA, (s16)countB};
                vertices += (s32)(countA + countB);
                ++records;
            } else {
                const bool front = FacingOpenFirst(v1st, v2nd, v3rd);
                KeepTriangle(v1st, v3rd, v4th);
                const bool back = FacingOpen();
                if (!front && !back) continue;
                const s32 kept = front && back ? 4 : 3;
                run = {id, vertices, extra, (s16)kept, 0};
                vertices += kept;
                ++records;
                if (!front) std::memmove(out + stride, out + 2 * stride, 2 * stride);
            }
        }
        row = next;
    }

    at<s32>(batch, 0x20) = vertices;
    if (global<u32>((u32)(uintptr_t)&gView_8bcbc8) & 0x2000) g_env->fog(object, nullptr);
    const u32 flags = at<u32>(batch, 4);
    g_env->transformVertices(global<u8 *>(0x00879ab0), nullptr, 0);
    if (global<u32>(0x008bd524) == 4) g_env->pack(at<u8 *>(batch, 0x28), at<u32>(batch, 0x20), (s32)stride);
    return DrawRuns(self, batch, runs, records, vertices, flags);
}
