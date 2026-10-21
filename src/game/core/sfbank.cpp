#include "sfbank.h"

#include <cstdint>
#include <xmmintrin.h>

#include "../math/x87.h"
#include "gameheap.h"
#include "sfman.h"
#include "soundindex.h"
#include "string.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

s32 ftol(X87Reg x) { return (s32)(long long)x.v; }   // __ftol (0x67a830): truncated, its low dword

}  // namespace

extern "C" {

s32 __fastcall SoundResource_LengthMs(u8 *self, void *, u32 a) {
    u8 buffer[0x30];   // (not cleared, as the original's)
    u8 *format = buffer;
    u32 answer;
    const s32 *result = vcall<const s32 *>(self, 0x18, &answer, &format, a);
    if (*result < 0) return 0;
    X87Reg length = (double)at<s32>(format, 4);   // fild
    if (at<u8>(format, 0x1b) & 1) length = length * global<float>(0x00711ba0);
    if (at<u8>(format, 0x1a) == 0x10) length = length * global<float>(0x00711ba0);
    const X87Reg rate = (double)at<u16>(format, 0x18);
    const float ms = (float)((length / rate) * global<float>(0x00711ba4));   // fdivr st0, st1; fmul; fstp
    return _mm_cvtss_si32(_mm_set_ss(ms));   // fld; fistp
}

u32 *__fastcall SfBank_Load(u8 *self, void *, u32 *out, u32 path, u32) {
    if (at<u8>(self, 0x18) & 1) {
        *out = 0;
        return out;
    }
    if (u8 *const synth = at<u8 *>(self, 0x14)) {
        u32 answer;
        if ((s32)*SfMan_LoadBank(synth, nullptr, &answer, path, 0) >= 0) {
            at<u32>(self, 0x18) |= 1;
            *out = 0;
            return out;
        }
    }
    *out = 0xffffffff;
    return out;
}

void __fastcall SfBank_Unload(u8 *self) {
    if (u8 *const synth = at<u8 *>(self, 0x14)) {
        SfMan_UnloadBank(synth);
        at<u32>(self, 0x14) = 0;
        at<u32>(self, 0x18) = 0;
    }
}

u32 *__fastcall SfBank_Play(u8 *self, void *, u32 *out, const u8 *voice) {
    if (!at<u8 *>(self, 0x14)) {
        *out = 0xffffffff;
        return out;
    }
    u32 program = at<u32>(voice, 0) - 1;
    const u32 count = at<u32>(at<u8 *>(self, 0x14), 0x18);
    if (program > count) program %= count;
    u32 answer;   // (the original's: the voice argument's own slot)
    if (at<u8>(voice, 0x14) & 0x10) SfMan_Send(at<u8 *>(self, 0x14), nullptr, &answer, program, 4, at<u32>(voice, 0x20), 0x3c);
    if (at<u8>(voice, 0x14) & 2) SfMan_Send(at<u8 *>(self, 0x14), nullptr, &answer, program, 0x10, at<u32>(voice, 0x28), 0x3c);
    if (at<u32>(voice, 0x14) & 0x10000)
        SfMan_Send(at<u8 *>(self, 0x14), nullptr, &answer, program, 0x20, (u32)ftol(X87Reg(at<float>(voice, 0x5c)) * global<float>(0x00711be8)), 0x3c);
    String name;
    String_Ctor(&name);
    u32 note = 0x3c;
    SfMan_GetName(at<u8 *>(self, 0x14), nullptr, &answer, program, reinterpret_cast<u8 *>(&name));
    if (String_Find(&name, nullptr, reinterpret_cast<const char *>(0x00783b74), 0) != -1)        // "_1ol"
        note = 0x30;
    else if (String_Find(&name, nullptr, reinterpret_cast<const char *>(0x00783b6c), 0) != -1)   // "_2ol"
        note = 0x24;
    else if (String_Find(&name, nullptr, reinterpret_cast<const char *>(0x00783b64), 0) != -1)   // "_3ol"
        note = 0x18;
    SfMan_Send(at<u8 *>(self, 0x14), nullptr, &answer, program, 1, 0x7f, note);
    *out = 0;
    String_Dtor(&name);
    return out;
}

u8 *__fastcall SfBank_Ctor(u8 *self, void *, u8 *source, u8 *synth) {
    SoundResource_Ctor(self, nullptr, source);
    at<u32>(self, 0x18) = 0;
    at<u8 *>(self, 0x14) = synth;
    at<u32>(self, 0) = 0x00711ba8;
    return self;
}

void __fastcall SfBank_Dtor(u8 *self) {
    at<u32>(self, 0) = 0x00711ba8;
    SfBank_Unload(self);
    SoundResource_Dtor(self);
}

u8 *__fastcall SfBank_DeletingDtor(u8 *self, void *, u32 flags) {
    SfBank_Dtor(self);
    if (flags & 1) GameHeap_Free(self);
    return self;
}

}  // extern "C"
