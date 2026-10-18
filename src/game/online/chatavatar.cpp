#include "chatavatar.h"

#include <cstdint>

#include "../world/things.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// Chat::MovePlayer(int x, int y, int mode, void *context) (__thiscall on the Chat) through its import slot.
u32 movePlayer(u8 *avatar, s32 x, s32 y, s32 mode) {
    using Method = u32(__fastcall *)(void *, void *, s32, s32, s32, void *);
    return (*reinterpret_cast<const Method *>((uintptr_t)0x006fd4cc))(at<void *>(avatar, 0), nullptr, x, y, mode, avatar);
}

// The thing in the thing table's slot (0x7cfb90, 20 bytes each) for the handle's low word. (The debug check of the table left out.)
u8 *slotThing(u32 handle) { return *reinterpret_cast<u8 *const *>((uintptr_t)(0x007cfb90 + 20 * (handle & 0xffff))); }

// The avatar's thing's slot thing placed at (x, y) when it is of kind 0x12. Returns whether it was.
bool placeThing(u8 *thing, u32 x, u32 y) {
    if (thing[2] != 0x12)
        return false;
    Thing_ResetPlaceAt(thing, nullptr, x, y);
    return true;
}

void moveTo(u8 *avatar, u32 x, u32 y, s32 mode) {
    movePlayer(avatar, (s16)x, (s16)y, mode);
    u16 handle;
    const u16 got = *Thing_GetHandle(at<u8 *>(avatar, 4), nullptr, &handle);
    placeThing(slotThing(got), x, y);
}

}  // namespace

extern "C" {

void __fastcall ChatAvatar_MoveTo(u8 *avatar, void *, u32 x, u32 y) { moveTo(avatar, x, y, 0); }

void __fastcall ChatAvatar_MoveTo3(u8 *avatar, void *, u32 x, u32 y) { moveTo(avatar, x, y, 3); }

u8 __fastcall ChatAvatar_PlaceAtCell(u8 *self, void *, u32 handle, u32 cell) {
    const u32 x = ((u32)(uintptr_t)self & 0xffff0000) | ((cell - 1) & 0x7f) << 8 | 0x80;
    const u32 y = (cell & 0xffff0000) | (((cell - 1) >> 7) & 0xff) << 8 | 0x80;
    return placeThing(slotThing(handle), x, y) ? 1 : 0;
}
}
