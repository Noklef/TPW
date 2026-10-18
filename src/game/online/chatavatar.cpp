#include "chatavatar.h"

#include <cstdint>

#include "../world/person.h"
#include "../world/personscripts.h"
#include "../world/things.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// Chat::MovePlayer(int x, int y, int mode, void *context) (__thiscall on the Chat) through its import slot.
u32 movePlayer(u8 *avatar, s32 x, s32 y, s32 mode) {
    using Method = u32(__fastcall *)(void *, void *, s32, s32, s32, void *);
    return (*reinterpret_cast<const Method *>((uintptr_t)0x006fd4cc))(at<void *>(avatar, 0), nullptr, x, y, mode, avatar);
}

// The byte +0xeb1 cleared, then Chat::NotAFK(void *context) (__thiscall on the Chat) through its import slot with the avatar.
void notAfk(u8 *avatar) {
    at<u8>(avatar, 0xeb1) = 0;
    using Method = u32(__fastcall *)(void *, void *, void *);
    (*reinterpret_cast<const Method *>((uintptr_t)0x006fd4bc))(at<void *>(avatar, 0), nullptr, avatar);
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

void __fastcall ChatAvatar_Arrive(u8 *avatar, void *, u32 x, u32 y) {
    Person_ReleaseScripts(at<u8 *>(avatar, 4));
    u16 handle;
    const u16 got = *Thing_GetHandle(at<u8 *>(avatar, 4), nullptr, &handle);
    placeThing(slotThing(got), x, y);
    movePlayer(avatar, (s16)x, (s16)y, 1);
}

u32 __fastcall ChatAvatar_SayPosition(u8 *avatar) {
    const u8 *const thing = at<u8 *>(avatar, 4);
    return movePlayer(avatar, at<s16>(thing, 4), at<s16>(thing, 6), 2);
}

void __fastcall ChatAvatar_MoveTo3(u8 *avatar, void *, u32 x, u32 y) { moveTo(avatar, x, y, 3); }

u8 __fastcall ChatAvatar_PlaceAtCell(u8 *self, void *, u32 handle, u32 cell) {
    const u32 x = ((u32)(uintptr_t)self & 0xffff0000) | ((cell - 1) & 0x7f) << 8 | 0x80;
    const u32 y = (cell & 0xffff0000) | (((cell - 1) >> 7) & 0xff) << 8 | 0x80;
    return placeThing(slotThing(handle), x, y) ? 1 : 0;
}

u8 __fastcall ChatAvatar_MoveToCell(u8 *avatar, void *, u32 cell) {
    u16 handle;
    const u16 got = *Thing_GetHandle(at<u8 *>(avatar, 4), nullptr, &handle);
    const u32 column = (cell - 1) & 0x7f;
    const u32 row = (((cell & 0xffff) - 1) >> 7) & 0xff;
    // (x's high word is uninitialised stack in the original: only its low word is used)
    if (!placeThing(slotThing(got), column << 8 | 0x80, (cell & 0xffff0000) | row << 8 | 0x80))
        return 0;
    movePlayer(avatar, (s16)(u16)(column << 8 | 0x80), (s16)(u16)(row << 8 | 0x80), 1);
    notAfk(avatar);
    return 1;
}

u8 __fastcall ChatPlayer_NameIs(const u8 *thing, void *, const u8 *name) {
    const u16 *a = at<const u16 *>(name, 4);
    if (!a)
        a = reinterpret_cast<const u16 *>((uintptr_t)0x006fd6c4);
    const u16 *b = reinterpret_cast<const u16 *>(thing + 0x19c);
    for (;; ++a, ++b) {
        if (*a != *b)
            return 0;
        if (!*a)
            return 1;
    }
}

u8 *__stdcall ChatPlayers_FindByName(const u8 *name) {
    u16 out;
    u16 handle = *World_FirstThing(at<void *>(reinterpret_cast<void *>((uintptr_t)0x007cf83c), 0), nullptr, &out);
    while (handle) {
        u8 *const thing = slotThing(handle);
        if (thing[2] == 0x12 && ChatPlayer_NameIs(thing, nullptr, name))
            return thing;
        handle = *World_NextThing(at<void *>(reinterpret_cast<void *>((uintptr_t)0x007cf83c), 0), nullptr, &out, thing);
    }
    return nullptr;
}

u8 __fastcall ChatAvatar_GoToCell(u8 *avatar, void *, u32 cell) {
    notAfk(avatar);
    u16 handle;
    const u16 got = *Thing_GetHandle(at<u8 *>(avatar, 4), nullptr, &handle);
    u8 *const thing = slotThing(got);
    if (thing[2] != 0x12 || !Sprite_SetDestinationCellMarked(thing, nullptr, cell))
        return 0;
    const u32 index = (cell & 0xffff) - 1;
    movePlayer(avatar, (s32)((index & 0x7f) << 8), (s32)((index >> 7) << 8), 0);
    return 1;
}
}
