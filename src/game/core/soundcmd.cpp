#include "soundcmd.h"

#include <cstdint>

#include "accessors.h"
#include "fieldops.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u32 *Forward(u8 *object, u32 offset, u32 *out, u32 value) {
    vcall<void>(object, offset, out, value);
    return out;
}

u32 *Functor(u8 *target, u32 *out, u32 vtable) {
    u32 functor[2];   // (the second dword left unset, sic)
    functor[0] = vtable;
    vcall<void>(at<u8 *>(target, 0x24), 0x10, out, functor);
    return out;
}

u32 *SetByte(u8 *target, u32 *out, u32 index, u32 value) {
    Bytes44_Set(at<u8 *>(target, 0x24), nullptr, out, index, value);
    return out;
}

}  // namespace

extern "C" {

u32 *__fastcall SoundCmd700b2c_Run(const u8 *command, void *, u32 *out, u8 *target) { return Forward(target, 0x20, out, at<u32>(command, 4)); }

u32 *__fastcall SoundCmd700b34_Run(const u8 *command, void *, u32 *out, u8 *target) {
    const u8 *const value = command + 4;
    u32 bits;
    __asm {
        mov eax, value
        fld dword ptr [eax]
        fstp bits
    }
    Set2c_6b9220(at<u8 *>(target, 0x24), nullptr, bits);
    *out = 0;
    return out;
}

u32 *__fastcall SoundCmd700b48_Run(const u8 *command, void *, u32 *out, u8 *target) { return Forward(target, 0x1c, out, at<u32>(command, 4)); }

u32 *__fastcall SoundCmd700b50_Run(const u8 *command, void *, u32 *out, u8 *target) {
    return Forward(at<u8 *>(target, 0x24), 0x24, out, at<u32>(command, 4));
}

u32 *__fastcall SoundCmd700b64_Run(const u8 *, void *, u32 *out, u8 *target) { return Functor(target, out, 0x0070a268); }

u32 *__fastcall SoundCmd700b68_Run(const u8 *, void *, u32 *out, u8 *target) { return Functor(target, out, 0x0070a270); }

u32 *__fastcall SoundCmd700b70_Run(const u8 *command, void *, u32 *out, u8 *target) { return SetByte(target, out, 3, at<u32>(command, 4)); }

u32 *__fastcall SoundCmd700b74_Run(const u8 *command, void *, u32 *out, u8 *target) { return SetByte(target, out, 1, at<u32>(command, 4)); }

u32 *__fastcall SoundCmd700b78_Run(const u8 *command, void *, u32 *out, u8 *target) { return SetByte(target, out, 2, at<u32>(command, 4)); }

u32 *__fastcall SoundCmd700ba8_Run(const u8 *command, void *, u32 *out, u8 *target) { return Forward(target, 0x18, out, at<u32>(command, 4)); }

}  // extern "C"
