#include "chatavatar.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/stdtree.h"
#include "../core/stdwstring.h"
#include "../sprite/script.h"
#include "../ui/window7cc8bc.h"
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

void __fastcall ChatAvatar_Enter(u8 *avatar, void *, u32 x, u32 y) {
    Person_PlaceScript(at<u8 *>(avatar, 4), nullptr, 0, 0.0f, 0, 0, 3);
    SpriteScript_SetRegF(at<s32>(at<u8 *>(avatar, 4), 0xc), 8, 0x3fc00000);   // 1.5f
    moveTo(avatar, x, y, 1);
}

u32 __fastcall ChatAvatar_SayPosition(u8 *avatar) {
    const u8 *const thing = at<u8 *>(avatar, 4);
    return movePlayer(avatar, at<s16>(thing, 4), at<s16>(thing, 6), 2);
}

void __fastcall ChatAvatar_MoveTo3(u8 *avatar, void *, u32 x, u32 y) { moveTo(avatar, x, y, 3); }

u32 __fastcall ChatAvatar_MovePlayer1(u8 *avatar, void *, u32 x, u32 y) { return movePlayer(avatar, (s16)x, (s16)y, 1); }

void __fastcall ChatAvatar_GotoRoom(u8 *avatar) {
    const u16 *text = at<const u16 *>(avatar, 0xf00);
    if (!text) text = reinterpret_cast<const u16 *>(0x006fd6c4);
    StdWString name;  // (its allocator byte is an uninitialised stack byte in the original)
    name.allocator = 0;
    name.ptr = nullptr;
    name.len = 0;
    name.res = 0;
    const u32 n = Crt().wcslen(text);
    if (StdWString_Grow(&name, nullptr, n, 1)) {
        for (u32 k = 0; k < n; ++k) name.ptr[k] = text[k];
        name.ptr[n] = 0;
        name.len = n;
    }
    using Method = u32(__fastcall *)(void *, void *, StdWString *, void *);
    (*reinterpret_cast<const Method *>((uintptr_t)0x006fd490))(at<void *>(avatar, 0), nullptr, &name, avatar);
    if (name.ptr) {  // _Tidy(true), inlined
        u8 &refs = *reinterpret_cast<u8 *>(reinterpret_cast<uintptr_t>(name.ptr) - 1);
        if (refs != 0 && refs != 0xff)
            --refs;
        else
            GameHeap_Free(reinterpret_cast<u8 *>(name.ptr) - 2);
    }
}

void __fastcall ChatAvatar_AddName(u8 *avatar, void *, const StdWString *name, u32) {
    const u16 *text = name->ptr;
    if (!text) text = reinterpret_cast<const u16 *>(0x006fd6c4);
    Window7cc8bc_ListAddStd(text, 0);
    u8 *result[2];
    StdMapD_Insert(avatar + 0xe80, nullptr, result, name);
}

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
