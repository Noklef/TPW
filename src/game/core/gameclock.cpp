#include "gameclock.h"

#include <cstdint>

#include "../files/file.h"
#include "../math/x87.h"
#include "thunks.h"

namespace {

u32 &u32_at(u8 *p, u32 offset) { return *reinterpret_cast<u32 *>(p + offset); }
double &double_at(u8 *p, u32 offset) { return *reinterpret_cast<double *>(p + offset); }

}  // namespace

extern "C" {

u32 __fastcall GameClock_Save24(u8 *clock, void *, Stream *stream) {
    s32 result;
    return *Stream_Write(&result, stream, clock + 0x24, 4, nullptr) >= 0;
}

u32 __fastcall GameClock_Load24(u8 *clock, void *, Stream *stream) {
    s32 result;
    return *Stream_Read(&result, stream, clock + 0x24, 4, nullptr) >= 0;
}

u32 __fastcall GameClock_Save4c(u8 *clock, void *, Stream *stream) {
    s32 result;
    return *Stream_Write(&result, stream, clock + 0x4c, 4, nullptr) >= 0;
}

u32 __fastcall GameClock_Load4c(u8 *clock, void *, Stream *stream) {
    s32 result;
    return *Stream_Read(&result, stream, clock + 0x4c, 4, nullptr) >= 0;
}

u32 __fastcall Clock_Save(u8 *clock, void *, Stream *stream) {
    const u32 ok = GameClock_Save4c(clock, nullptr, stream);
    return ok ? GameClock_Save24(clock + 0x50, nullptr, stream) : ok;
}

u32 __fastcall Clock_Load(u8 *clock, void *, Stream *stream) {
    const u32 ok = GameClock_Load4c(clock, nullptr, stream);
    return ok ? GameClock_Load24(clock + 0x50, nullptr, stream) : ok;
}

double __fastcall GameClock_Double20(const u8 *clock) { return *reinterpret_cast<const double *>(clock + 0x20); }

u32 __fastcall RealClock_Read(u8 *clock) {
    const u32 ms = Time_GetMs();
    const u32 delta = ms - u32_at(clock, 0);
    u32_at(clock, 0) = ms;
    double &run = double_at(clock, 8);
    run = X87_Add((double)delta, run);  // fild qword (delta, 0); fadd
    return (u32)(long long)run;         // __ftol: 64-bit truncation, low half
}

void __fastcall RealClock_Pause(u8 *clock) {
    if (u32_at(clock, 0x18)) return;
    u32_at(clock, 0x10) = RealClock_Read(clock);
    u32_at(clock, 0x18) = 1;
}

void __fastcall RealClock_Resume(u8 *clock) {
    if (!u32_at(clock, 0x18)) return;
    const u32 now = RealClock_Read(clock);
    u32_at(clock, 0x18) = 0;
    u32_at(clock, 0x14) += now - u32_at(clock, 0x10);
}

void __fastcall RealClock_TogglePause(u8 *clock) {
    if (u32_at(clock, 0x18))
        RealClock_Resume(clock);
    else
        RealClock_Pause(clock);
}

u32 __fastcall RealClock_Elapsed(u8 *clock) {
    return u32_at(clock, 0x18) ? u32_at(clock, 0x10) - u32_at(clock, 0x14) : RealClock_Read(clock) - u32_at(clock, 0x14);
}

u32 __fastcall RealClock_Now(u8 *clock) { return RealClock_Elapsed(clock) + u32_at(clock, 0x20); }

void __fastcall RealClock_RestoreNow(u8 *clock) { u32_at(clock, 0x20) = u32_at(clock, 0x24) - RealClock_Elapsed(clock); }

u32 __fastcall GameClock_Read(u8 *clock) {
    const u32 ms = Time_GetMs();
    const u32 delta = ms - u32_at(clock, 0x10);
    u32_at(clock, 0x10) = ms;
    double &run = double_at(clock, 0x18);
    run = X87_Add(X87_MulUIntDouble(delta, double_at(clock, 0x20)), run);  // fild qword (delta, 0); fmul; fadd
    return (u32)(long long)run;                                           // __ftol: 64-bit truncation, low half
}

void __fastcall GameClock_Pause(u8 *clock) {
    if (u32_at(clock, 0x30)) return;
    u32_at(clock, 0x28) = GameClock_Read(clock);
    u32_at(clock, 0x30) = 1;
}

void __fastcall GameClock_Resume(u8 *clock) {
    if (!u32_at(clock, 0x30)) return;
    const u32 now = GameClock_Read(clock);
    u32_at(clock, 0x30) = 0;
    u32_at(clock, 0x2c) += now - u32_at(clock, 0x28);
}

void __fastcall GameClock_TogglePause(u8 *clock) {
    if (u32_at(clock, 0x30))
        GameClock_Resume(clock);
    else
        GameClock_Pause(clock);
}

u32 __fastcall GameClock_Elapsed(u8 *clock) {
    return u32_at(clock, 0x30) ? u32_at(clock, 0x28) - u32_at(clock, 0x2c) : GameClock_Read(clock) - u32_at(clock, 0x2c);
}

void __fastcall GameClock_Freeze(u8 *clock, void *, u32 fps) {
    if (u32_at(clock, 0x38)) return;
    const u32 time = GameClock_Elapsed(clock) + u32_at(clock, 0x44);
    u32_at(clock, 0x38) = 1;
    u32_at(clock, 0x3c) = time;
    u32_at(clock, 0x40) = 1000u / fps;  // (DIV: 0 faults, sic)
}

void __fastcall GameClock_Unfreeze(u8 *clock) {
    if (!u32_at(clock, 0x38)) return;
    const u32 elapsed = GameClock_Elapsed(clock);
    u32_at(clock, 0x38) = 0;
    u32_at(clock, 0x44) = u32_at(clock, 0x3c) - elapsed;
}

u32 __fastcall GameClock_Time(u8 *clock) { return u32_at(clock, 0x38) ? u32_at(clock, 0x3c) : GameClock_Elapsed(clock) + u32_at(clock, 0x44); }

u32 __fastcall GameClock_Now(u8 *clock) { return GameClock_Time(clock) + u32_at(clock, 0x48); }

void __fastcall GameClock_RestoreNow(u8 *clock) { u32_at(clock, 0x48) = u32_at(clock, 0x4c) - GameClock_Time(clock); }

u32 __fastcall Clocks_Now(u8 *clock) { return GameClock_Now(clock); }

u32 __fastcall Clocks_RealNow(u8 *clock) { return RealClock_Now(clock + 0x50); }

void __fastcall Clocks_Pause(u8 *clock) {
    GameClock_Pause(clock);
    RealClock_Pause(clock + 0x50);
}

void __fastcall Clocks_Resume(u8 *clock) {
    GameClock_Resume(clock);
    RealClock_Resume(clock + 0x50);
}

void __fastcall Clocks_TogglePause(u8 *clock) {
    GameClock_TogglePause(clock);
    RealClock_TogglePause(clock + 0x50);
}

void __fastcall Clocks_SaveNow(u8 *clock) {
    u32_at(clock, 0x4c) = GameClock_Now(clock);
    u32_at(clock, 0x74) = RealClock_Now(clock + 0x50);
}

void __fastcall Clocks_RestoreNow(u8 *clock) {
    GameClock_RestoreNow(clock);
    RealClock_RestoreNow(clock + 0x50);
}

void __cdecl GameClock_InitUnusedOne() { *reinterpret_cast<float *>(0x00785a04) = 1.0f; }

void __cdecl GameClock_InitUnusedEpsilon() { *reinterpret_cast<float *>(0x00785ddc) = 1.0f / 8388608.0f; }

u32 __cdecl Clocks_RealNowLessPauses() {
    u8 *const clocks = reinterpret_cast<u8 *>(0x00785970);
    u32 &paused = *reinterpret_cast<u32 *>(0x00f7977c);
    u32 &pauseStart = *reinterpret_cast<u32 *>(0x00f79780);
    u32 &pausedFor = *reinterpret_cast<u32 *>(0x00f79784);
    const u32 now = Clocks_RealNow(clocks);
    if (Wrap_Obj_Get30_402df0(clocks) != 0) {
        if (paused == 0) {
            paused = 1;
            pauseStart = now;
        }
        return pauseStart - pausedFor;
    }
    if (paused != 0) {
        paused = 0;
        pausedFor += now - pauseStart;
    }
    return now - pausedFor;
}
}
