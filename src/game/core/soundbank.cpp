#include "soundbank.h"

#include <cstdint>

#include "string.h"
#include "vcall.h"

namespace {

// The opener's third argument: its +0x20 points to the text at +0x24 (empty), the rest left unset.
struct OpenInfo {
    u8 head[0x20];
    char *text;
    char buffer[0x104];
};

const char *Text(u32 address) { return reinterpret_cast<const char *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __fastcall SoundBank_OpenSdt(void *, void *, u8 *opener, String *path, u32 hdFirst) {
    OpenInfo info;
    info.text = info.buffer;
    info.buffer[0] = 0;
    String saved;
    String_CtorCopy(&saved, nullptr, reinterpret_cast<const StringView *>(path));
    const char *const hd = Text(0x00782de8), *const hw = Text(0x00782de0);   // "HD.sdt", "HW.sdt"
    vcall<void>(path, 0x20, hdFirst ? hd : hw);
    s32 status;
    if (*vcall<const s32 *>(opener, 4, &status, path->data, &info) < 0) {
        vcall<void>(path, 0x18, &saved);
        vcall<void>(path, 0x20, hdFirst ? hw : hd);
    }
    String_Dtor(&saved);
}

u32 *__fastcall SoundBank_Open(void *bank, void *, u32 *out, u8 *opener, String *path, u32, u32 hdFirst) {
    String copy;
    String_CtorCopy(&copy, nullptr, reinterpret_cast<const StringView *>(path));
    OpenInfo info;
    info.text = info.buffer;
    info.buffer[0] = 0;
    vcall<void>(path, 0x18, &copy);
    SoundBank_OpenSdt(bank, nullptr, opener, path, hdFirst);
    s32 status;
    *out = *vcall<const s32 *>(opener, 4, &status, path->data, &info) < 0 ? 0xffffffffu : 0;
    String_Dtor(&copy);
    return out;
}

}  // extern "C"
