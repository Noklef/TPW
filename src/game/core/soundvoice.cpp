#include "soundvoice.h"

#include <cstdint>

#include "fieldops.h"
#include "gameheap.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __fastcall SoundVoice_EnsureParams(u8 *voice) {
    if (at<u8 *>(voice, 0x4c)) return;
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x20));
    if (block) {
        for (u32 offset = 8; offset <= 0x18; offset += 4) at<u32>(block, offset) = 0;
        block[0] = 0;
        at<u32>(block, 4) = 0;
    }
    at<u8 *>(voice, 0x4c) = block;
}

u32 *__fastcall SoundVoice_SetPending(u8 *voice, void *, u32 *out, u32 value) {
    if (!at<u32>(voice, 0x24)) {
        at<u16>(voice, 0x48) = static_cast<u16>(value);
        at<u32>(voice, 0x30) |= 0x8000;
        *out = 0xffffffff;
        return out;
    }
    u32 record[0x27];   // (the fields not named here are left unset, sic)
    static const u8 kCleared[] = {0, 1, 3, 4, 5, 0xb, 0xc, 0xe, 0xf, 0x10, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x22};
    for (const u8 i : kCleared) record[i] = 0;
    record[0x20] = 0x3e800000;   // 0.25f
    record[0x21] = 0x3f000000;   // 0.5f
    record[0x23] = 0x3f800000;   // 1.0f
    SetThenFlag28_14_6c5130(reinterpret_cast<u8 *>(record), nullptr, value);
    u32 functor[3] = {0x0070a850, at<u32>(voice, 0x24), static_cast<u32>(reinterpret_cast<uintptr_t>(record))};
    vcall<void>(global<u8 *>(0x00fb1f24), 0x10, out, functor);
    return out;
}

}  // extern "C"
