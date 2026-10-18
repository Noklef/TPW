// An animated object put down on the park grid: reimplementation of 0x467030 and 0x4671f0.
#include "placement.h"

#include "../math/matrix.h"
#include "../math/sintable.h"
#include "../math/x87.h"
#include "../model/holder.h"
#include "../terrain/land.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>(static_cast<u8 *>(p) + offset); }

// fild n (signed); fmul f: the exact product rounded once.
double mul_int(s32 n, float f) {
    if (n < 0 && f == f) return -X87_MulInt(0u - (u32)n, f);
    return X87_MulInt((u32)n, f);
}

const float kDegToRad = 0.0174532924f;    // 0x6fe774
const float kMinusHalfPi = -1.57079637f;  // 0x6fe778

}  // namespace

extern "C" {

void __cdecl Animator_PlaceOnGround(u8 *animator, s32 x, s32 y, const u8 *cells, s32 width, s32 depth, u32 flags) {
    if (at<u32>(animator, 4) & 0x4000)
        Obj_RemoveFromRect(at<u8 *>(animator, 8), at<u32>(animator, 0xc0), at<u32>(animator, 0xc4));
    else
        Holder_Sleep(at<u8 *>(animator, 8));
    const s32 turn = at<s32>(at<u8 *>(animator, 8), 0x10);
    s32 cornerX = x, cornerY = y, cellX = x, cellY = y;
    switch (turn) {
    case 90:
        cornerY = y + 1;
        cellY = cornerY - width;
        break;
    case 180:
        cornerX = x + 1;
        cornerY = y + 1;
        cellX = cornerX - width;
        cellY = cornerY - depth;
        break;
    case 270:
        cornerX = x + 1;
        cellX = cornerX - depth;
        break;
    }
    const float height = (float)Terrain_FootprintHeight(cornerX, cornerY, cells, width, depth, flags, turn);
    Animator_Place(animator, cornerX, cornerY, cellX, cellY, 0, reinterpret_cast<const u32 &>(height), flags, width, depth);
    at<s32>(animator, 0xe8) = x;
    at<s32>(animator, 0xec) = y;
    if (at<u32>(animator, 4) & 0x4000)
        Holder_AddToGrid(at<u8 *>(animator, 8), at<u32>(animator, 0xc0), at<u32>(animator, 0xc4));
    else
        Holder_Wake(at<u8 *>(animator, 8));
}

void __cdecl Animator_Place(u8 *animator, s32 x, s32 y, s32 cellX, s32 cellY, s32 angle, u32 height, u32 flags, s32 width, s32 depth) {
    if (flags & 2) {
        u8 *model = at<u8 *>(at<u8 *>(animator, 8), 4);
        const bool straight = angle == 0 || angle == 0xb4;
        const s32 right = (straight ? width : depth) + x - 1, bottom = (straight ? depth : width) + y - 1;
        at<u8>(model, 0x31) |= 4;
        at<u32>(model, 0xa8) = (u32)(x & 0xff) << 24 | (u32)(right & 0xff) << 16 | (u32)(y & 0xff) << 8 | (u32)(bottom & 0xff);
        at<u32>(animator, 0xc0) = ((u32)y & 0xffff) | (u32)x << 16;
        at<u32>(animator, 0xc4) = ((u32)bottom & 0xffff) | (u32)right << 16;
    }
    u8 *grid = at<u8 *>(*reinterpret_cast<u8 **>(0x007a0854), 0x6c);
    const volatile float *g = reinterpret_cast<const volatile float *>(grid);
    const float sizeX = g[4], originX = g[2], sizeY = g[5], originY = g[3];  // read whatever the flags
    const float posX = (float)(X87Reg(mul_int(cellX, sizeX)) + originX);
    const float posZ = (float)(X87Reg(mul_int(cellY, sizeY)) + originY);
    if (!(flags & 0x20)) return;
    at<s32>(at<u8 *>(animator, 8), 0x10) += angle;  // (fild; __ftol: the angle itself)
    at<s32>(at<u8 *>(animator, 8), 0x10) %= 0x168;
    if (at<s32>(at<u8 *>(animator, 8), 0x10) < 0) at<s32>(at<u8 *>(animator, 8), 0x10) += 0x168;
    const float radians = (float)mul_int(angle, kDegToRad);
    float *m = reinterpret_cast<float *>(at<u8 *>(at<u8 *>(at<u8 *>(animator, 8), 4), 0x78) + 0x10);
    const SinTable sines = SinTable_Game();
    const float s = SinTable_At(sines, (float)(X87Reg(sines.scale) * radians));
    const float c = SinTable_At(sines, (float)((X87Reg(radians) - kMinusHalfPi) * sines.scale));
    Matrix_RotateY(m, X87_Through(c), X87_Through(s));
    u8 *t = at<u8 *>(at<u8 *>(at<u8 *>(animator, 8), 4), 0x78) + 0x10;
    at<float>(t, 0x30) = posX;
    at<u32>(t, 0x34) = height;
    at<float>(t, 0x38) = posZ;
}
}
