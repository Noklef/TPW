#include "scriptref.h"

#include <cstdint>

#include "../core/savegame.h"
#include "../math/x87.h"
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

void __fastcall ScriptRef_SetPeriodFor(u8 *ref, void *, float speed) {
    const s32 period = (s32)(long long)(X87Reg(62.0f) / X87Reg(speed)).v;   // (0x7003a0, __ftol)
    *reinterpret_cast<s32 *>(ref + 8) = period < 0 ? 0 : period > 0xfa ? 0xfa : period;
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

u32 __fastcall SpriteAnim_Serialize(u8 *ref, void *, Stream *stream, u32 save, s32 version, u32) {
    if (version < 0 || version > 1)
        return 0;
    if (!Save_Field(stream, save, ref, 4, 0x0075a4c0) || !Save_Field(stream, save, ref + 4, 4, 0x0075a4b4) ||
        !Save_Field(stream, save, ref + 8, 4, 0x0075a49c))
        return 1;
    return 0;
}

void __fastcall Obj_KillSlot(u8 *object) {
    SpriteScript_Kill((s32)at<u32>(object, 0));
    at<u32>(object, 0) = 0;
}
}
