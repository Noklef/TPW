#include "sfman.h"

#include <cstdint>
#include <cstring>

#include "crt.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// The API table's slots (cdecl function pointers).
template <typename F> F api(const u8 *self, u32 offset) { return at<F>(at<u8 *>(self, 8), offset); }

u32 ShortMsg(u32 handle, u32 message) {   // midiOutShortMsg
    using F = u32(__stdcall *)(u32, u32);
    return (*reinterpret_cast<const F *>((uintptr_t)0x006fd3ccu))(handle, message);
}

template <typename F> F Import(u32 slot) { return *reinterpret_cast<const F *>((uintptr_t)slot); }

u32 *Answer(u32 *out, u32 value) {
    *out = value;
    return out;
}

}  // namespace

extern "C" {

u32 __stdcall SfMan_PackMsg(u32 status, u32 data1, u32 data2) {
    return ((data2 & 0x7f) << 8 | (data1 & 0x7f)) << 8 | status | 0x82;
}

u32 __fastcall SfMan_UnloadBank(u8 *self) {
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

u32 *__fastcall SfMan_Sysex(u8 *self, void *, u32 *out, u32 value, const u32 *message) {
    using Header = u32(__stdcall *)(u32, u32 *, u32);   // midiOutPrepareHeader, midiOutLongMsg
    const u32 handle = at<u32>(self, 0x10);
    if (!handle || !message) return Answer(out, 0xffffffff);
    u8 data[0x14] = {0xf0, 0, 0x20, 0x21, 0x5f, 0, static_cast<u8>(message[0])};
    const u8 *const bytes = reinterpret_cast<const u8 *>(message);
    const u32 type = message[0];
    u32 length;
    if (type == 0x7f) {
        data[7] = 0xf7;
        length = 8;
    } else if (type == 0) {
        for (u32 i = 0; i < 4; ++i) data[7 + i] = static_cast<u8>(value >> 8 * i & 0x7f);
        data[0xb] = 0xf7;
        length = 0xc;
    } else if (type == 4) {
        data[7] = 0;
        data[8] = bytes[8];
        data[9] = 0;
        data[0xa] = bytes[0xa];
        for (u32 i = 0; i < 4; ++i) data[0xb + i] = static_cast<u8>(value >> 8 * i & 0x7f);
        data[0xf] = bytes[0xc];
        data[0x10] = 0;
        data[0x11] = bytes[0xe];
        data[0x12] = 0xf7;
        length = 0x13;
    } else if (type == 2) {
        data[7] = bytes[4];
        for (u32 i = 0; i < 4; ++i) data[8 + i] = static_cast<u8>(value >> 8 * i & 0x7f);
        data[0xc] = 0xf7;
        length = 0xd;
    } else {
        return Answer(out, 0xffffffff);
    }
    u32 header[0x10] = {};
    header[0] = static_cast<u32>(reinterpret_cast<uintptr_t>(data));
    header[1] = length;
    u32 result = 0;
    if (Import<Header>(0x006fd3b8u)(handle, header, 0x40)) result = 0xffffffff;
    if (Import<Header>(0x006fd3d0u)(at<u32>(self, 0x10), header, 0x40)) result = 0xffffffff;
    return Answer(out, result);
}

u32 *__fastcall SfMan_OpenOut(u8 *self, void *, u32 *out) {
    if (at<u32>(self, 0x10)) return Answer(out, 0);
    const u32 count = Import<u32(__stdcall *)()>(0x006fd3c8u)();   // midiOutGetNumDevs
    if (!static_cast<u16>(count)) return Answer(out, 0xffffffff);
    for (u32 i = 0; static_cast<u16>(i) < static_cast<u16>(count); ++i) {
        u8 caps[0x34] = {};
        Import<u32(__stdcall *)(u32, u8 *, u32)>(0x006fd3c4u)(i & 0xffff, caps, 0x34);   // midiOutGetDevCapsA
        if (Crt().strncmp(reinterpret_cast<const char *>(caps + 8), "SoundFont", 9)) continue;
        if (Import<u32(__stdcall *)(u32 *, u32, u32, u32, u32)>(0x006fd3c0u)(&at<u32>(self, 0x10), i & 0xffff, 0, 0, 0)) continue;   // midiOutOpen
        u32 message[4] = {0x7f, 0, 0, 0};
        SfMan_Sysex(self, nullptr, out, 0, message);
        return out;
    }
    return Answer(out, 0xffffffff);
}

u32 *__fastcall SfMan_GetName(u8 *self, void *, u32 *out, u32 index, u8 *sink) {
    if (!at<u8 *>(self, 8)) return Answer(out, 0xffffffff);
    char name[0x40];
    u16 pair[2] = {1, static_cast<u16>(index)};
    u32 buffer[2] = {0x40, static_cast<u32>(reinterpret_cast<uintptr_t>(name))};
    if (api<s32(__cdecl *)(u16, u16 *, u32 *)>(self, 0x40)(at<u16>(self, 0xc), pair, buffer)) return Answer(out, 0xffffffff);
    vcall<void>(sink, 0x14, name);
    return Answer(out, 0);
}

u32 __fastcall SfMan_CountPresets(u8 *self) {
    if (!at<u8 *>(self, 8)) return 0;
    const char *const empty = reinterpret_cast<const char *>((uintptr_t)0x00786de4u);
    for (u16 i = 0;;) {
        char name[0x40];
        std::memcpy(name, empty, std::strlen(empty) + 1);   // (no bound, sic)
        u16 pair[2] = {1, i};
        u32 buffer[2] = {0x40, static_cast<u32>(reinterpret_cast<uintptr_t>(name))};
        if (api<s32(__cdecl *)(u16, u16 *, u32 *)>(self, 0x40)(at<u16>(self, 0xc), pair, buffer)) return 0;
        if (!std::strcmp(name, empty)) return at<u32>(self, 0x18);
        const u32 count = ++at<u32>(self, 0x18);
        if (++i >= 0x80) return count;
    }
}

u32 *__fastcall SfMan_LoadBank(u8 *self, void *, u32 *out, u32 path, u32) {
    if (!(at<u8>(self, 0x14) & 1) || !at<u8 *>(self, 8)) return Answer(out, 0xffffffff);
    u16 pair[2] = {1, 0};
    u32 file[2] = {0x80, path};
    if (api<s32(__cdecl *)(u16, u16 *, u32 *)>(self, 0x2c)(at<u16>(self, 0xc), pair, file)) return Answer(out, 0xffffffff);
    return Answer(out, SfMan_CountPresets(self) ? 0 : 0xffffffff);
}

}  // extern "C"
