// The online dialog's chat rooms' head counts: see roomcounts.h.
#include "roomcounts.h"

#include <cstdint>
#include <cstring>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/selectors.h"
#include "../core/stdstring.h"
#include "../core/string.h"
#include "../core/vcall.h"
#include "../ui/window.h"
#include "itemlist.h"
#include "session.h"
#include "chatevents.h"

namespace {

template <typename T>
T &global(u32 address) {
    return *reinterpret_cast<T *>((uintptr_t)address);
}
template <typename T>
const T &field(const u8 *object, u32 offset) {
    return *reinterpret_cast<const T *>(object + offset);
}

s32 *counts() { return &reinterpret_cast<s32 &>(gRoomcounts_7cdac8); }
s32 *ids() { return &global<s32>(0x007cdae0); }
u8 *&chat() { return global<u8 *>(0x007cdb14); }
s32 &answers() { return global<s32>(0x007cdb18); }
u32 &ready() { return global<u32>(0x007cdb1c); }

const char *Text(const char *s) { return s ? s : reinterpret_cast<const char *>((uintptr_t)0x006fe5b0); }

// The room of a string's id: its index, or 5.
u32 RoomOf(const StdString &id) {
    const s32 wanted = Crt().atoi(Text(id.ptr));
    u32 room = 0;
    while (room < 5 && ids()[room] != wanted) ++room;
    return room;
}

// ~basic_string() as the compiler inlines it: no _Tidy.
void Release(char *ptr) {
    if (!ptr) return;
    u8 *const block = reinterpret_cast<u8 *>(ptr) - 1;
    if (*block == 0 || *block == 0xff) GameHeap_Free(block);
    else --*block;
}

u8 *NewChat() {
    typedef u8 *(__fastcall * Ctor)(u8 *, void *);
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0xc));
    return memory ? (*reinterpret_cast<const Ctor *>((uintptr_t)0x006fd474))(memory, nullptr) : nullptr;
}

void AskRoom(s32 id) {
    typedef u32(__fastcall * RoomInfo)(u8 *, void *, const StdString *, void *);
    String number;
    String_Ctor(&number);
    String_Format(&number, gChatevents_759920, id);
    StdString text;
    text.ptr = nullptr;
    text.len = 0;
    text.res = 0;
    StdString_AssignChars(&text, nullptr, number.data, static_cast<u32>(std::strlen(number.data)));
    (*reinterpret_cast<const RoomInfo *>((uintptr_t)0x006fd478))(chat(), nullptr, &text, nullptr);
    Release(text.ptr);
    String_Dtor(&number);
}

void StartRound() {
    const u8 *const city = field<const u8 *>(Online_Instance(), 0xf4);
    if (!field<u32>(city, 0x28)) return;
    const s32 rooms = field<s32>(field<const u8 *>(city, 0x2c), 0xc);
    if (rooms <= 0) return;
    global<u32>(0x007cdb10) = 0;
    const s32 count = rooms < 5 ? rooms : 5;
    for (s32 i = 0; i < count; ++i) {
        const u8 *const room = Owner2c_At(field<u8 *>(Online_Instance(), 0xf4), nullptr, i);
        counts()[i] = 0;
        ids()[i] = field<s32>(room, 8);
    }
    chat() = NewChat();
    if (!chat()) chat() = NewChat();
    typedef u32(__fastcall * Connect)(u8 *, void *, const char *, s32, u32, u8 *);
    const char *const host = Text(global<const char *>(0x007a1504));
    const s32 port = global<s16>(0x007a1552);
    u8 *const context = Global7cc53c_Pick7cc4f0();
    // (the callback by its address in the game, which RoomCounts_ChatEvent's hook answers)
    if (!(*reinterpret_cast<const Connect *>((uintptr_t)0x006fd498))(chat(), nullptr, host, port, 0x004c4920, context)) {
        typedef void(__stdcall * Debug)(const char *);
        (*reinterpret_cast<const Debug *>((uintptr_t)0x006fd1e0))(gRoomcounts_759924);
        return;
    }
    ready() = 0;
    answers() = 0;
    for (s32 i = 0; i < count; ++i) AskRoom(ids()[i]);
}

}  // namespace

extern "C" {

void __stdcall RoomCounts_ChatEvent(s32 type, u32, const u8 *arguments) {
    if (type < -5) return;
    if (type <= -4) {
        ready() = 1;
        return;
    }
    if (type != 0x64) return;
    const u8 *const first = field<const u8 *>(arguments, 4);
    const s32 kind = field<s32>(first, 0x20);
    if (kind == 7) {
        StdString id;
        StdString_CopyInto(reinterpret_cast<const StdString *>(first + 0x6c), nullptr, &id);
        const s32 count = field<s32>(field<const u8 *>(arguments, 4), 0xb0);
        const u32 room = RoomOf(id);
        if (room < 5) counts()[room] = count;
        StdString_Tidy(&id, nullptr, 1);
    } else if (kind == 0x2c) {
        const s32 size = static_cast<s32>(field<const u8 *>(arguments, 8) - first) / 0x24;
        if (static_cast<u32>(size) > 1) {
            StdString id;
            StdString_CopyInto(reinterpret_cast<const StdString *>(first + 0x24), nullptr, &id);
            const u32 room = RoomOf(id);
            if (room < 5) counts()[room] = 0;
            Release(id.ptr);
        }
    }
    if (++answers() >= 5) ready() = 1;
}

void __cdecl RoomCounts_Update() {
    if (global<u32>(0x007cdb10)) StartRound();
    if (!ready()) return;
    typedef u32(__fastcall * ChatRelease)(u8 *);
    (*reinterpret_cast<const ChatRelease *>((uintptr_t)0x006fd47c))(chat());
    for (u32 i = 0; i < 5; ++i) {
        u8 *const dialog = global<u8 *>(0x007cdb0c);
        const s32 count = counts()[i];
        if (!dialog) continue;
        vcall<void>(Window_FindChild(Window_FindChild(dialog, nullptr, 0x16007), nullptr, 0x16008 + i), 0x1c, count);
    }
    ready() = 0;
}
}
