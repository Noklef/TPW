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
#include "messagecentre.h"
#include "random.h"
#include "thingmsg.h"
#include "things.h"
#include "world.h"
#include "../core/staticobjects.h"

namespace {

// A world random number (World_Rand on [0x7cf83c]) in 0..1: its low 30 bits times 2^-30 (fild qword; fmul float).
X87Reg unit() { return X87_MulInt((u32)World_Rand(*&gWorldPtr) & 0x3fffffff, 9.31322574615478515625e-10f); }

u32 bits(float f) {
    u32 b;
    std::memcpy(&b, &f, 4);
    return b;
}

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *world() { return gWorldPtr; }
u8 *fx() { return global<u8 *>(0x008bcbac); }

// The year's quarter's row of the table 0x7854c4: base, spread, percent.
const u32 *intensityRow() { return &global<const u32>((u32)(uintptr_t)&gBalanceLevel + 0x484) + 3 * FunnyTime_Quarter(world() + 0x2a0); }

// The funny time now past t.
bool nowPast(const FileTime *t) {
    FileTime now;
    FunnyTime_Now(world() + 0x2a0, nullptr, &now, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    return FileTime_Greater(&now, nullptr, t) != 0;
}

// a - b, or 0 when that is negative (by the sign of the wrapped result).
u32 atLeast0(u32 a, u32 b) { return (s32)(a - b) < 0 ? 0 : a - b; }

const WeatherEnv kGameEnv = {&MessageCentre_Post, &WeatherFx_ReloadTextures,
                             reinterpret_cast<void(__fastcall *)(u8 *, void *, u32, u32, u32, u32, u32, u32, u32, u32)>((uintptr_t)0x00580320), &Sound_SetAmbientLevel};
const WeatherEnv *g_env = &kGameEnv;

}  // namespace

extern "C" {

void Weather_SetEnv(const WeatherEnv *env) { g_env = env ? env : &kGameEnv; }

u32 __fastcall Weather_NextIntensity(u8 *) {
    if ((u32)World_Rand(world()) % 100 < intensityRow()[2])
        return (u32)World_Rand(world()) % 100;
    if ((s32)intensityRow()[1] > 0) {
        const u32 spread = intensityRow()[1] * 2;
        const u32 offset = (u32)World_Rand(world()) % spread - intensityRow()[1];
        return offset + intensityRow()[0];
    }
    return intensityRow()[0];
}

u8 *__fastcall Weather_Construct(u8 *weather) {
    Thing_Construct(weather, nullptr, 1, 0xf);
    for (u32 offset = 0x10; offset <= 0x24; offset += 4) at<u32>(weather, offset) = 0;
    weather[0x2c] = 0;
    MessageCentre_AddThing(MessageCentre_Instance(), nullptr, weather, 1);
    MessageCentre_AddThing(MessageCentre_Instance(), nullptr, weather, 2);
    at<u32>(weather, 0xc) = Weather_NextIntensity(weather);
    weather[0x64] = 0;
    at<u32>(weather, 0x34) = 0;
    at<u32>(weather, 0x30) = 0;
    weather[0x38] = 0;
    at<u32>(weather, 0x44) = 0;
    at<u32>(weather, 0x3c) = 0;
    at<u32>(weather, 0x40) = 0;
    at<u32>(weather, 0x54) = 0;
    at<u32>(weather, 0x5c) = 0x42b40000;
    at<u32>(weather, 0x60) = 0x3f000000;
    g_env->reloadTextures(fx());
    return weather;
}

void __fastcall Weather_Update(u8 *weather) {
    u8 msg[0x10];
    if (weather[0x2c] && nowPast(reinterpret_cast<const FileTime *>(weather + 0x20))) {
        ThingMsg01_Ctor(msg, nullptr, at<u32>(weather, 0x28));
        g_env->post(MessageCentre_Instance(), nullptr, msg);
        weather[0x2c] = 0;
    }
    if (!weather[0x2c]) {
        FileTime span, due;
        s32 result;
        FileTime_FromSpan(&span, nullptr, &result, atLeast0(global<u32>((u32)(uintptr_t)&gBalanceLevel + 0x4bc), global<u32>((u32)(uintptr_t)&gBalanceLevel + 0x4b8)), 0, 0, 0, 0);
        FileTime_Add(reinterpret_cast<const FileTime *>(weather + 0x10), nullptr, &due, &span);
        if (nowPast(&due)) {
            const u32 value = Weather_NextIntensity(weather);
            ThingMsg02_Ctor(msg, nullptr, global<u32>((u32)(uintptr_t)&gBalanceLevel + 0x4b8), value);
            g_env->post(MessageCentre_Instance(), nullptr, msg);
        }
    }
    const s32 target = at<s32>(weather, 0x34);
    if (target >= 0) {
        const s32 drops = at<s32>(weather, 0x30);
        const s32 apart = (s32)((u32)drops - (u32)target);
        const s32 gap = apart < 0 ? (s32)(0u - (u32)apart) : apart;
        const s32 step = global<s32>((u32)(uintptr_t)&gBalanceLevel + 0x4c0);
        if (gap >= step) {
            const s32 now = (s32)(drops < target ? (u32)drops + (u32)step : (u32)drops - (u32)step);
            at<s32>(weather, 0x30) = now;
            const s32 most = global<s32>((u32)(uintptr_t)&gBalanceLevel + 0x4f0);
            const s32 floor = now < 0 ? 0 : now;
            WeatherFx_SetDropCount(fx(), nullptr, floor < most ? floor : most);
            g_env->ambientLevel((s32)((u32)now * 100) / global<s32>((u32)(uintptr_t)&gBalanceLevel + 0x4f0));
        } else if (target == 0) {
            Weather_SetMode(weather, nullptr, 2);
            g_env->ambientLevel(0);
        }
    }
    if (weather[0x38]) {
        const s32 wait = (s32)(at<u32>(weather, 0x3c) - global<u32>((u32)(uintptr_t)&gBalanceLevel + 0x4dc));
        at<s32>(weather, 0x3c) = wait;
        if (wait < 0) {
            u32 start[3], end[3];
            Weather_RandomBolt(weather, nullptr, reinterpret_cast<float *>(start), reinterpret_cast<float *>(end), at<u32>(weather, 0x44));
            g_env->strike(fx(), nullptr, start[0], start[1], start[2], end[0], end[1], end[2], global<u32>((u32)(uintptr_t)&gBalanceLevel + 0x4e4), 1);
            if (at<s32>(weather, 0x44) <= 0) {
                const u32 keep = atLeast0(global<u32>((u32)(uintptr_t)&gBalanceLevel + 0x4e8), at<u32>(weather, 0x40));
                at<u32>(weather, 0x44) = (u32)World_Rand(world()) % global<u32>((u32)(uintptr_t)&gBalanceLevel + 0x4ec) + keep;
                std::memcpy(weather + 0x48, end, 12);
            }
            ++at<u32>(weather, 0x40);
            at<u32>(weather, 0x3c) = (u32)World_Rand(world()) % global<u32>((u32)(uintptr_t)&gBalanceLevel + 0x4e0);
        }
    } else {
        at<u32>(weather, 0x40) = 0;
    }
    if (--at<s32>(weather, 0x44) == 0) {
        const auto ftol = [](float f) { return (s32)(long long)f; };
        const s32 x = ftol(at<float>(weather, 0x48)), y = ftol(at<float>(weather, 0x4c)), z = ftol(at<float>(weather, 0x50));
        Sound_PlayAt(0, gSoundCategories.ambient, 0x20, x, y, z);
    }
}

u32 __fastcall Weather_Serialize(u8 *weather, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    const SaveSpec kHead[] = {{0xc, 4, (u32)gText_Weather_Serialize_76239c}, {0x10, 8, (u32)gText_Weather_Serialize_76238c}, {0x18, 8, (u32)gText_Weather_Serialize_76237c}};
    const SaveSpec kTail[] = {{0x30, 4, (u32)gText_Weather_Serialize_76236c}, {0x34, 4, (u32)gText_Weather_Serialize_76235c}, {0x38, 1, (u32)gText_Weather_Serialize_762350}, {0x3c, 4, (u32)gText_Weather_Serialize_76233c},
                              {0x40, 4, (u32)gText_Weather_Serialize_762324}, {0x44, 4, (u32)gText_Weather_Serialize_762310}, {0x48, 12, (u32)gText_Weather_Serialize_7622f8}, {0x54, 4, (u32)gText_Weather_Serialize_7622e4},
                              {0x58, 4, (u32)gText_Weather_Serialize_7622dc}, {0x5c, 4, (u32)gText_Weather_Serialize_7622d0}, {0x60, 4, (u32)gText_Weather_Serialize_7622c4}, {0x64, 1, (u32)gText_Weather_Serialize_7622b8}};
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
        if (!sound) sound = Sound_PlayAt(0, gSoundCategories.ambient, 0x21, 0, 0, 0);
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
    u8 *const world = *&gWorldPtr;
    FileTime time;
    const FileTime *now = FunnyTime_Now(world + 0x2a0, nullptr, &time, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    *reinterpret_cast<FileTime *>(weather + 0x10) = *now;
    const s32 *range = reinterpret_cast<const s32 *>((u32)(uintptr_t)&gBalanceLevel + 0x4c4);   // (three [low, high] pairs)
    bool a = v >= range[0] && v <= range[1];
    bool b = v >= range[2] && v <= range[3];
    weather[0x38] = v >= range[4] && v <= range[5];
    if (a && b) {
        if (World_Rand(*&gWorldPtr) & 1) {
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
        const s32 most = *reinterpret_cast<const s32 *>((u32)(uintptr_t)&gBalanceLevel + 0x4f0);
        const s32 drops = (s32)((u32)(100 - share) * (u32)most) / 100;
        *reinterpret_cast<s32 *>(weather + 0x34) = drops >= most ? most : drops;
        WeatherFx_SetFallSpeed(fx, nullptr, a ? 0x3f4ccccd : 0x40200000);   // 0.8, 2.5
    } else {
        *reinterpret_cast<u32 *>(weather + 0x34) = 0;
    }
    // The wind's strength: (100 - v) * 0.01 cut to 0..1.
    const X87Reg strength = X87Reg((double)(100 - v)) * gWeather_SetIntensity_700a58;
    u32 &strengthBits = *reinterpret_cast<u32 *>(weather + 0x60);
    if (strength.v < 0.0) {
        strengthBits = 0;
    } else if (strength.v > 1.0) {
        strengthBits = gWeather_SetIntensity_700a3c;
    } else {
        strengthBits = bits((float)strength);
    }
    // The wind's angle: moved by a random 0-89 degrees less 45, then brought into 0..360 (a NaN or an infinity there loops for ever, sic).
    float &angle = *reinterpret_cast<float *>(weather + 0x5c);
    const float old = angle;
    const u32 turn = (u32)World_Rand(*&gWorldPtr) % 90;
    X87Reg x = X87Reg((double)turn) + (X87Reg(X87_Through(old)) - gWeather_SetIntensity_700a5c);
    angle = (float)x;
    if (x.v < 0.0 || x.v != x.v) {
        do {
            x = X87Reg(X87_Through(angle)) - gWeather_SetIntensity_700a60;
            angle = (float)x;
        } while (x.v < 0.0 || x.v != x.v);
    }
    if (X87Reg(X87_Through(angle)).v > 360.0) {
        do {
            x = X87Reg(X87_Through(angle)) - gWeather_SetIntensity_700a64;
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
    u8 *const clock = *&gWorldPtr + 0x2a0;
    FileTime span, until, now;
    s32 status;   // (the original's out: the message's own stack slot)
    FileTime_FromSpan(&span, nullptr, &status, days, 0, 0, 0, 0);
    FileTime_Add(FunnyTime_Now(clock, nullptr, &now, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr), nullptr, &until, &span);
    *reinterpret_cast<FileTime *>(weather + 0x20) = until;
    *reinterpret_cast<s32 *>(weather + 0x28) = value;
    weather[0x2c] = 1;
    *reinterpret_cast<FileTime *>(weather + 0x18) = *FunnyTime_Now(clock, nullptr, &now, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
}

u32 __fastcall Weather_Detach(u8 *weather) {
    Thing_Detach(weather);
    return WeatherFx_FreeTextures(*reinterpret_cast<u8 **>((uintptr_t)0x008bcbac));
}
}
