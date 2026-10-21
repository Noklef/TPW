#include "sfman.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// The API table's slots (cdecl function pointers).
template <typename F> F api(const u8 *self, u32 offset) { return at<F>(at<u8 *>(self, 8), offset); }

u32 ShortMsg(u32 handle, u32 message) {   // midiOutShortMsg
    using F = u32(__stdcall *)(u32, u32);
    return (*reinterpret_cast<const F *>((uintptr_t)0x006fd3ccu))(handle, message);
}

u32 *Answer(u32 *out, u32 value) {
    *out = value;
    return out;
}

}  // namespace

extern "C" {

u32 __stdcall SfMan_PackMsg(u32 status, u32 data1, u32 data2) {
    return ((data2 & 0x7f) << 8 | (data1 & 0x7f)) << 8 | status | 0x82;
}

u32 __fastcall SfMan_SetLoad(u8 *self) {
    if (!at<u8 *>(self, 8)) return 0;
    u16 pair[2] = {1, 0};
    return api<s32(__cdecl *)(u16, u16 *)>(self, 0x38)(at<u16>(self, 0xc), pair) == 0 ? 1 : 0;
}

u32 __fastcall SfMan_NotesOff(u8 *self) {
    const u32 handle = at<u32>(self, 0x10);
    if (!handle) return 0;
    return ShortMsg(handle, SfMan_PackMsg(0xb0, 0x7b, 0));
}

u32 *__fastcall SfMan_Send(u8 *self, void *, u32 *out, u32 program, u32 flags, u32 data2, u32 data1) {
    if (!at<u32>(self, 0x10) || program >= at<u32>(self, 0x18)) return Answer(out, 0xffffffff);
    if (ShortMsg(at<u32>(self, 0x10), SfMan_PackMsg(0xb0, 0, 1))) return Answer(out, 0xffffffff);
    if (ShortMsg(at<u32>(self, 0x10), SfMan_PackMsg(0xc0, program, 0))) return Answer(out, 0xffffffff);
    u32 status, first;
    const u8 bits = static_cast<u8>(flags);
    if (bits & 1) {
        status = 0x90, first = data1;
    } else if (bits & 2) {
        status = 0x80, first = data1;
    } else if (bits & 4) {
        status = 0xb0, first = 0xa;
    } else if (bits & 0x10) {
        status = 0xb0, first = 0xb;
    } else if (bits & 8) {
        status = 0xb0, first = 0x40;
    } else if (bits & 0x20) {
        status = 0xb0, first = 0x5b;
    } else {
        status = first = static_cast<u32>(reinterpret_cast<uintptr_t>(out));   // sic
    }
    if (ShortMsg(at<u32>(self, 0x10), SfMan_PackMsg(status, first, data2))) {
        SfMan_NotesOff(self);
        ShortMsg(at<u32>(self, 0x10), SfMan_PackMsg(status, first, data2));
    }
    return Answer(out, 0);
}

}  // extern "C"
