#include "soundcmd.h"

#include <cstdint>
#include <cstring>

#include "accessors.h"
#include "fieldops.h"
#include "initializers.h"
#include "soundchannel.h"
#include "soundmixer.h"
#include "soundobject.h"
#include "vcall.h"
#include "../math/x87.h"

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

// The channel's rate modifier of the index (SoundChannel_SetRateModifier on target's +0x24): the command's value times 0.01 ([0x70a280];
// fild qword of the zero-extended value, fmul float, fstp float).
u32 *SetRate(const u8 *command, u32 *out, u8 *target, u32 index) {
    const float modifier = (float)X87_MulInt(at<u32>(command, 4), *reinterpret_cast<const float *>(0x0070a280));
    u32 bits;
    std::memcpy(&bits, &modifier, 4);
    SoundChannel_SetRateModifier(at<u8 *>(target, 0x24), nullptr, out, index, bits);
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

u32 *__fastcall SoundCmd700b80_Run(const u8 *command, void *, u32 *out, u8 *target) { return SetRate(command, out, target, 0); }

u32 *__fastcall SoundCmd700b88_Run(const u8 *command, void *, u32 *out, u8 *target) { return SetRate(command, out, target, 1); }

u32 *__fastcall SoundCmd700b7c_Run(const u8 *command, void *, u32 *out, u8 *target) { return SetRate(command, out, target, 2); }

u32 *__fastcall SoundCmd700b84_Run(const u8 *command, void *, u32 *out, u8 *target) { return SetRate(command, out, target, 3); }

u32 *__fastcall SoundCmd700ba8_Run(const u8 *command, void *, u32 *out, u8 *target) { return Forward(target, 0x18, out, at<u32>(command, 4)); }

u32 *__fastcall SoundCmd700b5c_Run(const u8 *, void *, u32 *out, u8 *target) { return SoundObject_Broadcast(target, nullptr, out); }

u32 *__fastcall SoundCmd700b6c_Run(const u8 *, void *, u32 *out, u8 *target) {
    u8 *const object = target;
    u32 functor = 0x0070a260;
    SoundChannel_Apply(at<u8 *>(object, 0x24), nullptr, reinterpret_cast<u32 *>(&target), reinterpret_cast<u8 *>(&functor));
    SoundMixer_Update(at<u8 *>(object, 0x20));
    *out = 0;
    return out;
}

u32 *__fastcall SoundCmd700b60_Run(const u8 *command, void *, u32 *out, u8 *target) {
    return SoundObject_BroadcastValue(target, nullptr, out, at<u32>(command, 4));
}

u32 *__fastcall SoundCmd700bbc_Run(const u8 *command, void *, u32 *out, u8 *target) {
    const u32 id = at<u32>(command, 4);
    if (!id) {
        *out = 0xffffffff;
        return out;
    }
    const u32 params[4] = {at<u32>(command, 8), at<u32>(command, 0xc), at<u32>(command, 0x10), at<u32>(command, 0x14)};
    return SoundMixer_SetVoiceParams(at<u8 *>(target, 0x20), nullptr, out, id, params);
}

u32 *__fastcall SoundCmd700ba4_Run(const u8 *command, void *, u32 *out, u8 *target) {
    return SoundMixer_SetVoicePending(at<u8 *>(target, 0x20), nullptr, out, at<u32>(command, 8), at<u32>(command, 4));
}

u32 *__fastcall SoundCmd700bac_Run(const u8 *command, void *, u32 *out, u8 *target) {
    return SoundObject_SendVoice(target, nullptr, out, at<u32>(command, 4), at<u32>(command, 8));
}

u32 *__fastcall SoundCmd700bb8_Run(const u8 *command, void *, u32 *out, u8 *target) {
    return SoundObject_VoiceInfo(target, nullptr, out, at<u32>(command, 8), at<u32 *>(command, 4));
}

u32 *__fastcall SoundCmd700b4c_Run(const u8 *command, void *, u32 *out, u8 *target) {
    u8 record[0x70];
    Init_6b9c50(record);
    SetThenFlag68_4c_6b9d30(record, nullptr, at<u32>(command, 4));
    u32 functor[2] = {0x0070a278, (u32)(uintptr_t)record};
    SoundChannel_Apply(at<u8 *>(target, 0x24), nullptr, out, reinterpret_cast<u8 *>(functor));
    return out;
}

u32 *__fastcall SoundCmd700b3c_Run(const u8 *command, void *, u32 *out, u8 *target) {
    SoundObject_SetVoices(target, nullptr, out, at<u32>(command, 4));
    return out;
}

u32 *__fastcall SoundCmd700b54_Run(const u8 *command, void *, u32 *out, u8 *target) {
    SoundChannel_SetMode(at<u8 *>(target, 0x24), nullptr, out, at<u32>(command, 4), at<u32>(command, 8), at<u32>(command, 0xc));
    return out;
}

u32 *__fastcall SoundCmd700bc0_Run(const u8 *command, void *, u32 *out, u8 *target) {
    SoundObject_AddOccluder(target, nullptr, out, at<u32>(command, 4), at<u32>(command, 8), at<u32>(command, 0xc), at<u32>(command, 0x10),
                            at<u8 **>(command, 0x14));
    return out;
}

u32 *__fastcall SoundCmd700b30_Run(const u8 *command, void *, u32 *out, u8 *target) {
    SoundObject_ApplyEaxRecord(target, nullptr, out, at<u32>(command, 4));
    return out;
}

u32 *__fastcall SoundCmd700bc4_Run(const u8 *command, void *, u32 *out, u8 *target) {
    SoundObject_RemoveOccluder(target, nullptr, out, at<u8 **>(command, 4));
    return out;
}

u32 *__fastcall SoundCmd700bc8_Run(const u8 *command, void *, u32 *out, u8 *target) {
    SoundChannel_SwitchOcclusion(at<u8 *>(target, 0x24), nullptr, out, at<u32>(command, 4), at<u32>(command, 8));
    return out;
}

u32 *__fastcall SoundCmd700b44_Run(const u8 *command, void *, u32 *out, u8 *target) {
    // (the original pushes EAX with only AL set: the upper bytes are the caller's, and only the low byte is read below)
    SoundChannel_SetHidden(at<u8 *>(target, 0x24), nullptr, at<u32>(command, 4) != 0);
    *out = 0;
    return out;
}

u32 *__fastcall SoundCmd700ba0_Run(const u8 *command, void *, u32 *out, u8 *target) {
    SoundObject_SendVoiceFunctor3(target, nullptr, out, at<u32>(command, 4));
    return out;
}

u32 *__fastcall SoundCmd700bb0_Run(const u8 *command, void *, u32 *out, u8 *target) {
    SoundObject_Save(target, nullptr, out, at<u8 *>(command, 4));
    return out;
}

u32 *__fastcall SoundCmd700bb4_Run(const u8 *command, void *, u32 *out, u8 *target) {
    SoundObject_Load(target, nullptr, out, at<u8 *>(command, 4), at<u8>(command, 8));
    return out;
}

u32 *__fastcall SoundCmd700b9c_Run(const u8 *command, void *, u32 *out, u8 *target) {
    SoundObject_SendVoiceFunctor2(target, nullptr, out, at<u32>(command, 4));
    return out;
}

}  // extern "C"
