#include "weather.h"

#include <cstring>

#include "../core/accessors.h"
#include "../core/filetime.h"
#include "../core/funnytime.h"
#include "../core/savegame.h"
#include "../core/soundplay.h"
#include "../core/soundset.h"
#include "../core/vcall.h"
#include "../math/x87.h"
#include "../render/weatherfx.h"
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

void __fastcall Weather_SetMode(u8 *weather, void *, s32 mode) {
    u8 *const fx = *reinterpret_cast<u8 *const *>(0x008bcbac);
    u32 &sound = *reinterpret_cast<u32 *>(weather + 0x54);
    if (mode == 0) {
        Obj_SetFirst(fx, nullptr, 1);
        if (!sound) sound = Sound_PlayAt(0, *reinterpret_cast<const u32 *>(0x00803a20), 0x21, 0, 0, 0);
        return;
    }
    if (mode == 1) {
        Obj_SetFirst(fx, nullptr, 2);
    } else if (mode == 2) {
        *reinterpret_cast<u32 *>(weather + 0x30) = 0;
        WeatherFx_SetDropCount(fx, nullptr, 0);
    } else {
        return;
    }
    if (sound) {
        Sound_Stop(sound);
        sound = 0;
    }
}

void __fastcall Weather_SetIntensity(u8 *weather, void *, s32 value) {
    const s32 floor1 = value < 1 ? 1 : value;
    const s32 v = floor1 >= 100 ? 100 : value < 1 ? 1 : value;
    *reinterpret_cast<s32 *>(weather + 0xc) = v;
    u8 *const world = *reinterpret_cast<u8 *const *>(0x007cf83c);
    FileTime time;
    const FileTime *now = FunnyTime_Now(world + 0x2a0, nullptr, &time, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    *reinterpret_cast<FileTime *>(weather + 0x10) = *now;
    const s32 *range = reinterpret_cast<const s32 *>(0x00785504);   // (three [low, high] pairs)
    bool a = v >= range[0] && v <= range[1];
    bool b = v >= range[2] && v <= range[3];
    weather[0x38] = v >= range[4] && v <= range[5];
    if (a && b) {
        if (World_Rand(*reinterpret_cast<u8 *const *>(0x007cf83c)) & 1) {
            a = false;
        } else {
            b = false;
        }
    }
    u8 *const fx = *reinterpret_cast<u8 *const *>(0x008bcbac);
    if (a || b) {
        const s32 low = a ? range[0] : range[2], span = (s32)((u32)(a ? range[1] : range[3]) - (u32)low);
        const s32 share = span ? (s32)(((u32)v - (u32)low) * 100) / span : 100;
        Weather_SetMode(weather, nullptr, a ? 1 : 0);
        const s32 most = *reinterpret_cast<const s32 *>(0x00785530);
        const s32 drops = (s32)((u32)(100 - share) * (u32)most) / 100;
        *reinterpret_cast<s32 *>(weather + 0x34) = drops >= most ? most : drops;
        WeatherFx_SetFallSpeed(fx, nullptr, a ? 0x3f4ccccd : 0x40200000);   // 0.8, 2.5
    } else {
        *reinterpret_cast<u32 *>(weather + 0x34) = 0;
    }
    // The wind's strength: (100 - v) * 0.01 cut to 0..1.
    const X87Reg strength = X87Reg((double)(100 - v)) * *reinterpret_cast<const float *>(0x00700a58);
    u32 &strengthBits = *reinterpret_cast<u32 *>(weather + 0x60);
    if (strength.v < 0.0) {
        strengthBits = 0;
    } else if (strength.v > 1.0) {
        strengthBits = *reinterpret_cast<const u32 *>(0x00700a3c);
    } else {
        strengthBits = bits((float)strength);
    }
    // The wind's angle: moved by a random 0-89 degrees less 45, then brought into 0..360 (a NaN or an infinity there loops for ever, sic).
    float &angle = *reinterpret_cast<float *>(weather + 0x5c);
    const float old = angle;
    const u32 turn = (u32)World_Rand(*reinterpret_cast<u8 *const *>(0x007cf83c)) % 90;
    X87Reg x = X87Reg((double)turn) + (X87Reg(X87_Through(old)) - *reinterpret_cast<const float *>(0x00700a5c));
    angle = (float)x;
    if (x.v < 0.0 || x.v != x.v) {
        do {
            x = X87Reg(X87_Through(angle)) - *reinterpret_cast<const float *>(0x00700a60);
            angle = (float)x;
        } while (x.v < 0.0 || x.v != x.v);
    }
    if (X87Reg(X87_Through(angle)).v > 360.0) {
        do {
            x = X87Reg(X87_Through(angle)) - *reinterpret_cast<const float *>(0x00700a64);
            angle = (float)x;
        } while (x.v > 360.0);
    }
    // (The debug print of the angle being out of 0..360 is left out.)
    reinterpret_cast<void(__fastcall *)(u8 *, void *, u32, u32)>(&WeatherFx_SetWind)(fx, nullptr, *reinterpret_cast<const u32 *>(weather + 0x5c),
                                                                                   strengthBits);
}

void __fastcall Weather_HandleMessage(u8 *weather, void *, u8 *message) {
    const s32 kind = vcall<s32>(message, 0);
    if (kind == 1) {
        if (!weather[0x64]) Weather_SetIntensity(weather, nullptr, *reinterpret_cast<s32 *>(message + 8));
        return;
    }
    if (kind != 2) return;
    const s32 days = *reinterpret_cast<s32 *>(message + 0xc), value = *reinterpret_cast<s32 *>(message + 8);
    u8 *const clock = *reinterpret_cast<u8 *const *>(0x007cf83c) + 0x2a0;
    FileTime span, until, now;
    s32 status;   // (the original's out: the message's own stack slot)
    FileTime_FromSpan(&span, nullptr, &status, days, 0, 0, 0, 0);
    FileTime_Add(FunnyTime_Now(clock, nullptr, &now, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr), nullptr, &until, &span);
    *reinterpret_cast<FileTime *>(weather + 0x20) = until;
    *reinterpret_cast<s32 *>(weather + 0x28) = value;
    weather[0x2c] = 1;
    *reinterpret_cast<FileTime *>(weather + 0x18) = *FunnyTime_Now(clock, nullptr, &now, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
}
}
