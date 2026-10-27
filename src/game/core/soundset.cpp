#include "soundset.h"

#include <cstdint>
#include <cstring>

#include "class6fd9d8.h"
#include "gameheap.h"
#include "soundmanager.h"
#include "string.h"
#include "vcall.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

void set(u32 address, u32 value) {
    global<u32>(0x00803ac8) = 1;
    global<u32>(address) = value;
}

// The named objects registry's (0x786b9c) folder of the key's name; false unless NamedList_Find answers 1.
bool find_folder(const void *key, u8 **found) {
    return NamedList_Find(global<u8 **>(0x00786b9c), nullptr, static_cast<const u8 *>(key), found) == 1;
}

const char *text(u32 address) { return reinterpret_cast<const char *>((uintptr_t)address); }
const StringView *view(u32 address) { return reinterpret_cast<const StringView *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __cdecl Sound_Set803ab8(u32 value) { set(0x00803ab8, value); }
void __cdecl Sound_Set803ac0(u32 value) { set(0x00803ac0, value); }
void __cdecl Sound_Set803abc(u32 value) { set(0x00803abc, value); }
void __cdecl Sound_Set803ac4(u32 value) { set(0x00803ac4, value); }
void __cdecl Sound_Set803adc(u32 value) { set(0x00803adc, value); }
void __cdecl Sound_Set803ad8(u32 value) { set(0x00803ad8, value); }
void __cdecl Sound_Set803ae0(u32 value) { set(0x00803ae0, value); }
void __cdecl Sound_Set803acc(u32 value) { set(0x00803acc, value); }

void __cdecl Sound_Set803ad0(u32 value) {
    global<u32>(0x00803ac8) = 1;
    global<u16>(0x00803ad0) = (u16)value;
}

void __cdecl Sound_Set803ad2(u32 value) { global<u16>(0x00803ad2) = (u16)value; }

u32 __cdecl Sound_IsOn() { return global<u32>(0x00802bc8); }

u8 *__fastcall SoundCmd_Ctor(u8 *cmd, void *, u32 value) {
    *reinterpret_cast<u32 *>(cmd + 4) = value;
    *reinterpret_cast<u32 *>(cmd) = 0x00700b58;
    return cmd;
}

u32 __cdecl Sound_Query(u32 a, u32 b) {
    if (!global<u32>(0x00802bc8) || !global<u32>(0x00802bd4)) return 0;
    void *const manager = global<void *>(0x00802bcc);
    u32 first = a, out = b;
    vcall<void>(manager, 4, &first, a, b, &out);
    return out;
}

u32 __cdecl Sound_SetParam(u32 sound, u32 parameter, u32 value) {
    if (!global<u32>(0x00802bc8) || !global<u32>(0x00802bd4)) return 0;
    return SoundManager_Call3(global<u8 *>(0x00802bcc), nullptr, sound, parameter, value);
}

u32 __cdecl Sound_SetPosition(u32 sound, s32 x, s32 y, s32 z) {
    if (!global<u32>(0x00802bc8) || !global<u32>(0x00802bd4)) return 0;
    float position[3] = {(float)x, (float)y, (float)z};
    return SoundManager_Call5(global<u8 *>(0x00802bcc), nullptr, sound, (u32)(uintptr_t)position);
}

u32 __cdecl Sound_QueryFound(const char *name, u8 *folders) {
    String found, key;
    String_Ctor(&found);
    String_CtorStr(&key, nullptr, name);
    const bool hit = Class6fd9d8_FindFirstMatch(folders, nullptr, reinterpret_cast<const StringView *>(&key), (u32)(uintptr_t)&found) == 1;
    String_Dtor(&key);
    u32 result = 0;
    if (hit && global<u32>(0x00802bc8) && global<u32>(0x00802bd4)) {
        u8 *first = folders;
        u32 out = (u32)(uintptr_t)name;
        vcall<void>(global<void *>(0x00802bcc), 4, &first, found.data, name, &out);
        result = out;
    }
    String_Dtor(&found);
    return result;
}

void __cdecl Sound_Shutdown() {
    if (global<u32>(0x00802bc8) && global<u32>(0x00802bd4)) {
        u32 result;
        vcall<void>(global<void *>(0x00802bcc), 0, &result);
    }
    global<u32>(0x00802bd4) = 0;
}

s32 __cdecl Sound_RegisterLobbyCategories(const char *name) {
    String key;
    String_CtorCopy(&key, nullptr, view(0x00803768));
    String_AppendStr(&key, nullptr, name);
    u8 *found = nullptr;
    if (!find_folder(&key, &found)) {
        String_Dtor(&key);
        return -1;
    }
    const u32 music = Sound_QueryFound(text(0x00762c40), found);   // "cat_locallobbymusic"
    const u32 sfx = Sound_QueryFound(text(0x00762c2c), found);     // "cat_locallobbysfx"
    const u32 count = ++global<u32>(0x00803a50);
    u8 *const pairs = reinterpret_cast<u8 *>((uintptr_t)Sound_Alloc(count * 8));
    if (global<u8 *>(0x00803a4c)) {
        memcpy(pairs, global<u8 *>(0x00803a4c), global<u32>(0x00803a50) * 8 - 8);   // (rep movsd / movsb)
        Sound_Free(global<u8 *>(0x00803a4c));
    }
    global<u8 *>(0x00803a4c) = pairs;
    *reinterpret_cast<u32 *>(pairs + global<u32>(0x00803a50) * 8 - 4) = music;
    *reinterpret_cast<u32 *>(global<u8 *>(0x00803a4c) + global<u32>(0x00803a50) * 8 - 8) = sfx;
    const s32 index = global<s32>(0x00803a50) - 1;
    String_Dtor(&key);
    return index;
}

void __cdecl Sound_RegisterGlobalCategories() {
    u8 *found = nullptr;
    if (find_folder(reinterpret_cast<const void *>(0x008030d8), &found)) {
        global<u32>(0x00803a20) = Sound_QueryFound(text(0x00762c8c), found);   // "cat_ambient"
        global<u32>(0x00803a28) = Sound_QueryFound(text(0x00762c80), found);   // "cat_rides"
        global<u32>(0x00803a2c) = Sound_QueryFound(text(0x00762c78), found);   // "cat_ui"
        global<u32>(0x00803a24) = Sound_QueryFound(text(0x00762c6c), found);   // "cat_kids"
        global<u32>(0x00803a30) = Sound_QueryFound(text(0x00762c60), found);   // "cat_staff"
    }
    if (find_folder(reinterpret_cast<const void *>(0x00803558), &found))
        global<u32>(0x00803a34) = Sound_QueryFound(text(0x00762c54), found);   // "cat_speech"
}

void __cdecl Sound_RegisterThemeCategories(const StringView *theme) {
    String key;
    String_CtorCopy(&key, nullptr, view(0x00803768));
    u8 *found = nullptr;
    String_Append(&key, nullptr, theme);
    if (find_folder(&key, &found)) {
        global<u32>(0x00803a38) = Sound_QueryFound(text(0x00762c8c), found);   // "cat_ambient"
        global<u32>(0x00803a3c) = Sound_QueryFound(text(0x00762c80), found);   // "cat_rides"
    }
    String_AssignWide(&key, nullptr, reinterpret_cast<const WString *>(0x008039c0));   // (slot 6: its data as chars, length bytes)
    String_Append(&key, nullptr, theme);
    if (find_folder(&key, &found)) global<u32>(0x00803a40) = Sound_QueryFound(text(0x00762c54), found);   // "cat_speech"
    String_AssignWide(&key, nullptr, reinterpret_cast<const WString *>(0x00802e68));
    String_Append(&key, nullptr, theme);
    if (find_folder(&key, &found)) global<u32>(0x00803a44) = Sound_QueryFound(text(0x00762e68), found);   // "cat_music"
    String_Dtor(&key);
}
}
