#include "weather.h"

#include <cstring>

#include "../core/savegame.h"
#include "../math/x87.h"
#include "forecast.h"
#include "random.h"
#include "things.h"

namespace {

// A world random number (World_Rand on [0x7cf83c]) in 0..1: its low 30 bits times 2^-30 (fild qword; fmul float).
X87Reg unit() { return X87_MulInt((u32)World_Rand(*reinterpret_cast<u8 *const *>(0x007cf83c)) & 0x3fffffff, 9.31322574615478515625e-10f); }

u32 bits(float f) {
    u32 b;
    std::memcpy(&b, &f, 4);
    return b;
}

}  // namespace

extern "C" {

u32 __fastcall Weather_Serialize(u8 *weather, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    static const SaveSpec kHead[] = {{0xc, 4, 0x0076239c}, {0x10, 8, 0x0076238c}, {0x18, 8, 0x0076237c}};
    static const SaveSpec kTail[] = {{0x30, 4, 0x0076236c}, {0x34, 4, 0x0076235c}, {0x38, 1, 0x00762350}, {0x3c, 4, 0x0076233c}, {0x40, 4, 0x00762324},
                                     {0x44, 4, 0x00762310}, {0x48, 12, 0x007622f8}, {0x54, 4, 0x007622e4}, {0x58, 4, 0x007622dc}, {0x5c, 4, 0x007622d0},
                                     {0x60, 4, 0x007622c4}, {0x64, 1, 0x007622b8}};
    if (Thing_Serialize(weather, nullptr, stream, save, version, unused))
        return 1;
    if (version < 0 || version > 1)
        return 0;
    if (!Save_Fields(stream, save, weather, kHead, 3) || Forecast_Serialize(weather + 0x20, nullptr, stream, save, version, unused))
        return 1;
    return Save_Fields(stream, save, weather, kTail, 12) ? 0 : 1;
}

void __fastcall Weather_RandomBolt(u8 *, void *, float *start, float *end, u32) {
    const float maxX = (float)X87_MulInt(*reinterpret_cast<const u32 *>(0x007a0314), 10.0f);   // (the map's width and height in cells)
    const float maxZ = (float)X87_MulInt(*reinterpret_cast<const u32 *>(0x007a0374), 10.0f);
    const float minX = 0.0f, minZ = 0.0f;
    X87Reg v = unit() * (X87Reg(maxX) - X87Reg(minX));
    reinterpret_cast<u32 *>(start)[1] = 0;
    start[0] = (float)(v + X87Reg(minX));
    v = unit() * (X87Reg(maxZ) - X87Reg(minZ));
    start[2] = (float)(v + X87Reg(minZ));
    v = unit() * X87Reg(100.0f);
    reinterpret_cast<u32 *>(end)[1] = 0x43960000;   // 300.0
    end[0] = (float)(v + X87Reg(start[0]) - X87Reg(50.0f));
    v = unit() * X87Reg(100.0f);
    end[2] = (float)(v + X87Reg(start[2]) - X87Reg(50.0f));
    if (start[0] > maxX) reinterpret_cast<u32 *>(start)[0] = bits(maxX);
    if (!(start[0] >= 1.0f)) reinterpret_cast<u32 *>(start)[0] = 0x3f800000;
    if (start[2] > maxZ) reinterpret_cast<u32 *>(start)[2] = bits(maxZ);
    if (!(start[2] >= 1.0f)) reinterpret_cast<u32 *>(start)[2] = 0x3f800000;
}
}
