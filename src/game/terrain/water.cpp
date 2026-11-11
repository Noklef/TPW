#include "water.h"

#include <cstdint>
#include <cstring>

#include "../gfx/texture.h"
#include "../math/x87.h"
#include "../render/batch.h"
#include "../render/clipcodes.h"
#include "../render/clippoly.h"
#include "../render/transformer.h"
#include "gridquad.h"
#include "landscape.h"
#include "../render/rendererframe.h"
#include "ground.h"
#include "../render/d3dsetup.h"
#include "../core/staticobjects.h"

using gridquad::FacingClipped;
using gridquad::FacingOpen;
using gridquad::FacingOpenFirst;
using gridquad::Fan;
using gridquad::KeepTriangle;

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const TerrainWaterEnv kPortEnv = {&Batch_Begin, &Batch_Close, &Transformer_Transform, &ClipPoly_Vertices};
const TerrainWaterEnv *g_env = &kPortEnv;

u8 *LandscapeObject() { return reinterpret_cast<u8 *>((uintptr_t)&gLandscape_8c1828); }

const u32 kPoint = 0x10; // a point's bytes: x, y, z, w

void KeepPoint(u8 *self, const u8 *point) {
    at<u32>(self, 0x295c) = at<u32>(point, 0);
    at<u32>(self, 0x2960) = at<u32>(point, 4);
    at<u32>(self, 0x2964) = at<u32>(point, 8);
}

// A batch kept from before: its vertices moved by the point's change.
void MoveKept(u8 *self) {
    const u8 *const batch = at<u8 *>(self, 0x2958);
    const u32 stride = gGround_7014b4;
    const u32 count = at<u32>(batch, 0x20);
    u8 *vertex = at<u8 *>(batch, 0x28);
    const u8 *const point = Landscape_Point(LandscapeObject());
    const float z = (float)(X87Reg(at<float>(point, 8)) - at<float>(self, 0x2964));
    const float x = (float)(X87Reg(at<float>(point, 0)) - at<float>(self, 0x295c));
    KeepPoint(self, point);
    for (u32 n = count; n != 0; --n, vertex += stride) {
        at<float>(vertex, 0x1c) = (float)(X87Reg(x) + at<float>(vertex, 0x1c));
        at<float>(vertex, 0x18) = (float)(X87Reg(z) + at<float>(vertex, 0x18));
    }
}

// The points of a row into their columns of the frame's arrays, transformed and clip coded.
void FillRow(u8 *self, s32 row, u8 *points, u8 *codes) {
    const s32 first = at<s8>(self, row + 0x28);
    const s32 last = at<s8>(self, row + 0xa8);
    if (first == 0x7f || last == -1) return;
    const u8 *const heights = at<u32>(self, 0x12970) ? at<u8 *>(self, 0x12970) : at<u8 *>(at<u8 *>(self, 0), 0x28);
    const float lift = global<float>((u32)(uintptr_t)&gView_8bcbc8 + 0x924);
    const float z = (float)(X87Reg((double)row) * at<float>(at<u8 *>(self, 0), 0x14) + at<float>(at<u8 *>(self, 0), 0xc));
    const u32 base = at<u32>(self, 8) * (u32)row;
    u8 *const start = points + (u32)first * kPoint;
    for (s32 column = first; column <= last; ++column) {
        const u8 *const grid = at<u8 *>(self, 0);
        u8 *const to = points + (u32)column * kPoint;
        at<float>(to, 0) = (float)(X87Reg((double)column) * at<float>(grid, 0x10) + at<float>(grid, 8));
        at<float>(to, 8) = z;
        at<float>(to, 4) = (float)(X87Reg(at<float>(heights, (base + (u32)column) * 4)) + lift);
    }
    const u32 count = (u32)(last - first + 1);
    g_env->transform(global<void *>(0x0087b088), nullptr, (u32)(uintptr_t)start, (u32)(uintptr_t)start, kPoint, kPoint, 0, count);
    u32 result;
    ClipCodes_Run(global<u32>((u32)(uintptr_t)&gD3dsetup_879a20), 0, reinterpret_cast<u8 *>(&result), codes + first, start, count, kPoint, 0xff);
}

} // namespace

void TerrainWater_SetEnv(const TerrainWaterEnv *env) { g_env = env ? env : &kPortEnv; }

extern "C" u32 __fastcall TerrainWater_Draw(u8 *self) {
    if (at<u32>(self, 0x12970) == 0) return 1;
    if (!(global<u32>(0x008bd508) & 0xc0000000) && at<u8 *>(self, 0x2958) != nullptr) {
        MoveKept(self);
        return 1;
    }
    const u8 *const texture = reinterpret_cast<const u8 *>((uintptr_t)Landscape_List(LandscapeObject()));
    if (texture == nullptr) return 1;
    const u32 stride = gGround_7014b4;
    const u32 batchFlags = global<u32>((u32)(uintptr_t)&gView_8bcbc8) & 0x2000 ? 0x60 : 0x20;
    u32 flags = (Texture_FormatBits(texture) | 0x800) & ~0x400u;
    if (global<u32>((u32)(uintptr_t)&gView_8bcbc8) & 0x2000) flags |= 0x100000;
    const u32 extra = at<u32>(texture, 8) & 0x1000 ? at<u32>(texture, 0x1c) : 0;
    u8 *const batch = g_env->begin(global<u8 *>(0x00879ab0), nullptr, 1, 4, 0x1c4, flags, extra);
    at<u8 *>(self, 0x2958) = batch;
    if (batch == nullptr) return 1;

    u16 *indices = at<u16 *>(batch, 0x2c);
    s32 row = at<s32>(self, 0x128);
    s32 vertices = 0;
    const u8 *const point = Landscape_Point(LandscapeObject());
    const u32 vertexLimit = at<u32>(global<u8 *>(0x00879ab0), 0x28) - (stride << 4);
    const u32 indexLimit = at<u32>(global<u8 *>(0x00879ab0), 0x2c) - 0x30;
    KeepPoint(self, point);
    at<u32>(batch, 0) = batchFlags;

    u8 thisRow[0x800], lastRow[0x800], codes[0x80], lastCodes[0x80], quad[0xe0], source[0x2a0];
    FillRow(self, row, thisRow, codes);
    const float scaleU = at<float>(texture, 0x20), scaleV = at<float>(texture, 0x24);
    const float offsetU = at<float>(texture, 0x28), offsetV = at<float>(texture, 0x2c);

    bool room = true;
    while (room && row < at<s32>(self, 0x12c)) {
        std::memcpy(lastRow, thisRow, sizeof lastRow);
        std::memcpy(lastCodes, codes, sizeof lastCodes);
        const s32 next = row + 1;
        FillRow(self, next, thisRow, codes);
        for (s32 column = at<s8>(self, row + 0x130); column < at<s8>(self, row + 0x1b0); ++column) {
            const u32 offset = (u32)vertices * stride;
            u8 *const batchVertex = at<u8 *>(batch, 0x28) + offset;
            if (vertexLimit < (u32)(uintptr_t)batchVertex || indexLimit < (u32)(uintptr_t)indices) {
                room = false;
                break;
            }
            const u8 *const grid = at<u8 *>(self, 0);
            const u8 cell = at<u8>(at<u8 *>(grid, 0x2c), (at<u32>(grid, 0x18) * (u32)row + (u32)column) * 4);
            const u8 a = lastCodes[column], b = lastCodes[column + 1], c = codes[column], d = codes[column + 1];
            if (a & b & c & d) continue;

            const float wrapped = (float)(X87Reg((double)column) * gTerrainWater_Draw_70153c);
            const X87Reg across = X87Reg((double)row) * gTerrainWater_Draw_701540;
            const X87Reg u0 = (X87Reg(at<float>(point, 0)) - wrapped) * gTerrainWater_Draw_768318;
            const float u1 = (float)(((X87Reg(at<float>(point, 0)) - gTerrainWater_Draw_70153c) - wrapped) * gTerrainWater_Draw_768318);
            const float v0 = (float)((X87Reg(at<float>(point, 8)) - across) * gTerrainWater_Draw_76831c);
            const float v1 = (float)(((X87Reg(at<float>(point, 8)) - gTerrainWater_Draw_70153c) - across) * gTerrainWater_Draw_76831c);
            const u32 colour = global<u32>((u32)(uintptr_t)&gView_8bcbc8 + 0x8f4);
            const u32 lastColour = row ? colour : colour & 0xffffff, nextColour = next ? colour : colour & 0xffffff;

            struct Corner {
                const u8 *from;
                u32 colour;
                X87Reg u;
                float v;
            };
            const Corner corners[4] = {{lastRow + (u32)column * kPoint, lastColour, u0, v1},
                                       {thisRow + (u32)column * kPoint, nextColour, X87Reg((float)u0), v0},
                                       {thisRow + (u32)(column + 1) * kPoint, nextColour, X87Reg(u1), v0},
                                       {lastRow + (u32)(column + 1) * kPoint, lastColour, X87Reg(u1), v1}};
            const u32 clipFlags = (u32)(a | b | c | d);
            u8 *const out = clipFlags ? quad : batchVertex;
            const u32 start = cell & 2 ? 0 : 1;
            for (u32 k = 0; k < 4; ++k) {
                const Corner &corner = corners[(start + k) & 3];
                u8 *const to = out + k * stride;
                std::memcpy(to, corner.from, kPoint);
                at<u32>(to, 0x10) = corner.colour;
                at<float>(to, 0x18) = (float)(X87Reg(scaleU) * corner.u + offsetU);
                at<float>(to, 0x1c) = (float)(X87Reg(scaleV) * corner.v + offsetV);
            }
            const u8 *const v1st = out, *const v2nd = out + stride, *const v3rd = out + 2 * stride, *const v4th = out + 3 * stride;
            s32 base = vertices, first = 0, second = 0;
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
                const s32 sum = (s32)(countA + countB);
                if (sum > 0) {
                    vertices += sum;
                    first = (s16)countA;
                    second = (s16)countB;
                }
            } else {
                const bool front = FacingOpenFirst(v1st, v2nd, v3rd);
                KeepTriangle(v1st, v3rd, v4th);
                const bool back = FacingOpen();
                if (front && back) {
                    vertices += 4;
                    first = 4;
                } else if (front || back) {
                    vertices += 3;
                    first = 3;
                    if (back) std::memmove(batchVertex + stride, batchVertex + 2 * stride, 2 * stride);
                }
            }
            indices = Fan(indices, base, first);
            if (second != 0) indices = Fan(indices, first + base, second);
        }
        row = next;
    }

    at<s32>(batch, 0x20) = vertices;
    at<s32>(batch, 0x24) = (s32)((u32)(uintptr_t)indices - at<u32>(batch, 0x2c)) >> 1;
    if (at<u32>(batch, 0x20) == 0 || at<u32>(batch, 0x24) == 0) {
        at<u8 *>(self, 0x2958) = nullptr;
        return 1;
    }
    const float depth = (float)X87Reg((double)(s32)((u32)vertices * 1900000u));
    return g_env->close(global<u8 *>(0x00879ab0), nullptr, depth) != 0 ? 1 : 0;
}
