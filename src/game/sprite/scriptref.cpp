#include "scriptref.h"

#include <cstdint>

#include "script.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// Entry point number i of the table of script start codes.
u32 Entry(u32 i) { return reinterpret_cast<const u32 *>(0x0075a418)[i]; }

}  // namespace

extern "C" {

void __fastcall ScriptRef_Start(u8 *ref, void *, u32 anim, u32 folder, u32 sprite) {
    at<s32>(ref, 0) = SpriteScript_Start(Entry(anim), folder, sprite, 0, 0, 0, 0);
}

u32 __fastcall ScriptRef_IsAt(const u8 *ref, void *, u32 anim) {
    if (!SpriteScript_IsAt(at<s32>(ref, 0), Entry(anim)) && at<u32>(ref, 4) != anim)
        return 0;
    return 1;
}

void __fastcall ScriptRef_Apply(u8 *ref) {
    if (at<u32>(ref, 4)) {
        SpriteScript_Jump(at<s32>(ref, 0), Entry(at<u32>(ref, 4)));
        at<u32>(ref, 4) = 0;
    }
    if (at<u32>(ref, 8)) {
        SpriteScript_SetPeriod(at<s32>(ref, 0), at<u32>(ref, 8));
        at<u32>(ref, 8) = 0;
    }
}
}
