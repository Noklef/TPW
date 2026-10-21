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

}  // extern "C"
