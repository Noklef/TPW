#include "soundset.h"

#include <cstdint>
#include <cstring>

#include "class6fd9d8.h"
#include "gameheap.h"
#include "soundmanager.h"
#include "string.h"
#include "vcall.h"
#include "../math/x87.h"

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

namespace {

bool On() { return global<u32>(0x00802bc8) && global<u32>(0x00802bd4); }

void Apply(u32 *out, u32 *functor) { SoundManager_Apply(global<u8 *>(0x00802bcc), nullptr, out, reinterpret_cast<u8 *>(functor)); }

}  // namespace

void __cdecl Sound_VoiceFunctor3(u32 id) {
    if (!On()) return;
    u32 functor[2] = {0x00700ba0, id};
    Apply(&id, functor);
}

void __cdecl Sound_SetVoicePending(u32 id, u32 value) {
    if (!On()) return;
    u32 functor[3] = {0x00700ba4, value, id};
    Apply(&value, functor);
}

void __cdecl Sound_Forward6(u32 value) {
    if (!On()) return;
    u32 functor[2] = {0x00700ba8, value};
    Apply(&value, functor);
}

void __cdecl Sound_Stop(u32 id) {
    if (!On()) return;
    u32 functor[3] = {0x00700bac, id, 0x3c};
    Apply(&id, functor);
}

u32 __cdecl Sound_AddOccluder(u32 x, u32, u32 y, u32 radius, u32 z) {
    if (!On()) return 0;
    u32 functor[6] = {0x00700bc0, x, y, z, radius, (u32)(uintptr_t)&x};
    Apply(&y, functor);
    return x;
}

void __cdecl Sound_Post700b5c() {
    if (!On()) return;
    u32 local[2];   // (the functor its vtable; the out unset)
    local[0] = 0x00700b5c;
    Apply(&local[1], local);
}

void __cdecl Sound_Post700b60() {
    if (!On()) return;
    u32 local[3];   // (the out unset; the functor {0x700b60, 0x5a})
    local[1] = 0x00700b60;
    local[2] = 0x5a;
    Apply(&local[0], &local[1]);
}

void __cdecl Sound_Post700b64() {
    if (!On()) return;
    u32 local[2];
    local[0] = 0x00700b64;
    Apply(&local[1], local);
}

void __cdecl Sound_Post700b68() {
    if (!On()) return;
    u32 local[2];
    local[0] = 0x00700b68;
    Apply(&local[1], local);
}

void __cdecl Sound_ApplySettings() {
    if (!On()) return;
    u32 local[3];   // (local[0] the out, local[1-2] the command; the last command's {local[0]} with the out local[1])
    if (global<u32>(0x00803ac8)) {
        auto post = [&](u32 vtable, u32 value) {
            local[1] = vtable;
            local[2] = value;
            Apply(&local[0], &local[1]);
        };
        const s32 scale = global<s32>(0x00785914);
        auto scaled = [&](u32 value) { return (u32)((s32)(value * (u32)scale) / 100); };
        if (global<u32>(0x00803acc) && global<u32>(0x00803ae0) && global<s32>(0x00803ac4) > 0) {
            post(0x00700b88, global<u16>(0x00803ad0) ? 0 : scaled(global<u32>(0x00803ac0)));
            post(0x00700b84, scaled(global<u32>(0x00803abc)));
        } else {
            post(0x00700b88, global<u16>(0x00803ad0) ? 0 : global<u32>(0x00803ac0));
            post(0x00700b84, global<u32>(0x00803abc));
        }
        post(0x00700b80, global<u32>(0x00803ab8));
        post(0x00700b7c, global<u32>(0x00803ac4));
        post(0x00700b78, global<u32>(0x00803ae0));
        post(0x00700b74, global<u32>(0x00803adc));
        SoundCmd_Ctor(reinterpret_cast<u8 *>(&local[1]), nullptr, global<u32>(0x00803ad8));
        local[1] = 0x00700b70;
        Apply(&local[0], &local[1]);
        global<u32>(0x00803ac8) = 0;
    }
    local[0] = 0x00700b6c;
    Apply(&local[1], &local[0]);
}

void __cdecl Sound_SetCameraOcclusion(s32 level) {
    if (!On()) return;
    u32 functor[3] = {0x00700bc8, 0, 0};
    const float &radius954 = global<float>(0x00785954), &radius95c = global<float>(0x0078595c), &radius964 = global<float>(0x00785964);
    // Between two (level, radius) points: (level - low) * (high radius - low radius) / (high - low) + low radius; the level's slot
    // then holds the divisor (sic).
    auto between = [&](s32 low, s32 high, const float &lowRadius, const float &highRadius) {
        const s32 span = (s32)((u32)high - (u32)low);
        const X87Reg x = X87Reg((double)(s32)((u32)level - (u32)low)) * (X87Reg(highRadius) - lowRadius) / X87Reg((double)span) + lowRadius;
        level = span;
        const float radius = (float)x;
        functor[1] = 1;
        std::memcpy(&functor[2], &radius, 4);
    };
    if (global<u8>(0x00790ab0) & 0x16) {
        const s32 high = global<s32>(0x00785968), mid = global<s32>(0x00785960), low = global<s32>(0x00785958);
        if (level > high) {
            functor[1] = 1;
            functor[2] = X87_FloatThrough(global<u32>(0x00785964));
        } else if (level > mid) {
            between(mid, high, radius95c, radius964);
        } else if (level > low) {
            between(low, mid, radius954, radius95c);
        } else if (level > global<s32>(0x0078590c)) {
            functor[1] = 1;
            functor[2] = X87_FloatThrough(global<u32>(0x00785954));
        }
    }
    Apply(reinterpret_cast<u32 *>(&level), functor);
}

void __cdecl Sound_RemoveOccluder(u32 node) {
    if (!On()) return;
    u32 out;   // (unset: the run writes it)
    u32 functor[2] = {0x00700bc4, (u32)(uintptr_t)&node};
    Apply(&out, functor);
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

void __cdecl Sound_RegisterGlobalLobbySfx() {
    u8 *found = nullptr;
    if (find_folder(reinterpret_cast<const void *>(0x008030d8), &found))
        global<u32>(0x00803a48) = Sound_QueryFound(text(0x00762bcc), found);   // "cat_globallobbysfx"
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
