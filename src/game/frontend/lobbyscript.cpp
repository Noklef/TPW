// The lobby scene's script: see lobbyscript.h.
#include "lobbyscript.h"

#include <cstring>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/string.h"
#include "../core/vcall.h"
#include "../math/x87.h"

namespace {

AddrCalls g_calls;

template <typename T>
T &G(u32 address) {
    return *reinterpret_cast<T *>((uintptr_t)address);
}
u32 Ptr(const void *p) { return (u32)reinterpret_cast<uintptr_t>(p); }

void SkipPast(const char *&cursor, char c) {
    while (*cursor != c) ++cursor;
    ++cursor;
}

s32 IntToken(const char *&cursor, u32 delimiter) {
    String token;
    String_ReadToken(&token, &cursor, delimiter);
    const s32 value = Crt().atoi(token.data);
    String_Dtor(&token);
    return value;
}

u32 FloatBits(float value) {
    u32 bits;
    std::memcpy(&bits, &value, 4);
    return bits;
}

u32 FloatToken(const char *&cursor, u32 delimiter) {
    String token;
    String_ReadToken(&token, &cursor, delimiter);
    const float value = (float)Crt().atof(token.data);
    String_Dtor(&token);
    return FloatBits(value);
}

u32 Through(u32 address) { return X87_FloatThrough(G<u32>(address)); }

void Island(u8 *self, const char *&cursor, const char *name, u32 a2, s32 index, u32 a4, u32 a5) {
    SkipPast(cursor, '(');
    s32 at = IntToken(cursor, ',');
    if (index >= 0) at = index;
    String first, second, third;
    WString text;
    SkipPast(cursor, '"');
    String_ReadToken(&first, &cursor, '"');
    SkipPast(cursor, '"');
    String_ReadToken(&second, &cursor, '"');
    SkipPast(cursor, '"');
    String_ReadToken(&third, &cursor, '"');
    SkipPast(cursor, '"');
    WString_ReadToken(&text, &cursor, '"');
    SkipPast(cursor, ',');
    const u32 degrees = FloatToken(cursor, ',');
    u32 point[3];
    point[0] = Through(G<u32>(G<u32>(0x00f82884) + 0x10) + 0x1c + 4 * (u32)at);
    point[1] = FloatToken(cursor, ')');
    point[2] = Through(G<u32>(G<u32>(0x00f82884) + 0x10) + 0x44 + 4 * (u32)at);
    u32 object = Ptr(GameHeap_Alloc(0x74));
    if (object) {
        object = g_calls.Th(0x005dfa60, object, Ptr(self), (u32)at, name, first.data, second.data, third.data, degrees, point, text.data, a2, a4, a5);   // Class702ca8_Ctor
    }
    g_calls.Th(0x005dfca0, object);   // Class702ca8_AcquireModels
    WString_Dtor(&text);
    String_Dtor(&third);
    String_Dtor(&second);
    String_Dtor(&first);
}

// (the two triples on the stack as in the original: the bench's one hash takes both)
struct FlyingPoints {
    u32 start[3];
    u32 between[9];
    u32 end[3];
};

void FlyingMesh(u8 *self, const char *&cursor) {
    SkipPast(cursor, '(');
    String first, second;
    SkipPast(cursor, '"');
    String_ReadToken(&first, &cursor, '"');
    SkipPast(cursor, '"');
    String_ReadToken(&second, &cursor, '"');
    SkipPast(cursor, ',');
    const s32 count = IntToken(cursor, ',');
    const u32 from = *reinterpret_cast<u32 *>(self + 0x10) + 0x3c;
    FlyingPoints points;
    points.start[0] = G<u32>(from);
    const float y = (float)(X87Reg(G<const float>(from + 4)) - X87Reg(gLobbyscript_702d40));
    points.start[1] = FloatBits(y);
    points.start[2] = G<u32>(from + 8);
    points.end[0] = FloatToken(cursor, ',');
    points.end[1] = FloatToken(cursor, ',');
    points.end[2] = FloatToken(cursor, ',');
    const u32 last = FloatToken(cursor, ')');
    u32 object = Ptr(GameHeap_Alloc(0x5c));
    if (object) object = g_calls.Th(0x005d9db0, object, (u32)count, first.data, second.data, points.start, points.end, last, 0x43fa0000u);   // Class702af0_Ctor
    g_calls.Th(0x005da120, object);   // Class702af0_AcquireModels
    String_Dtor(&second);
    String_Dtor(&first);
}

}  // namespace

extern "C" {

void LobbyScript_SetEnv(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

void __fastcall LobbyScript_Load(u8 *self, void *, const char *name, u32 a2, s32 index, u32 a4, u32 a5) {
    String path;
    String_CtorCopy(&path, nullptr, reinterpret_cast<const StringView *>((uintptr_t)0x00f855b8));
    vcall<void>(&path, 0x20, name);
    vcall<void>(&path, 0x20, reinterpret_cast<const char *>((uintptr_t)0x00747948));
    const u32 stream = g_calls.Cd(0x005c4d50, path.data, 0x0074c8ecu);   // VFS_Open
    g_calls.Cd(0x005c4fa0, stream, 0u, 2u);                               // Stream_SeekFailed
    const u32 size = g_calls.Cd(0x005c5000, stream);                      // Object_CallSlot14
    const u32 block = g_calls.Cd(0x005f5660, size);                       // Mem_GlobalAlloc
    g_calls.Cd(0x005c4fa0, stream, 0u, 0u);
    g_calls.Cd(0x005c4f60, block, 1u, size, stream);                      // Object_CallSlot8Product
    g_calls.Cd(0x005c4f80, stream);                                       // Object_FinishAndDelete
    char *const text = reinterpret_cast<char *>((uintptr_t)block);
    const char *const end = text + size;
    for (char *c = text; c < end; ++c)
        if (*c == '\n') *c = 0;
    const char *cursor = text;
    while (cursor < end) {
        const u32 world = *reinterpret_cast<u32 *>(self + 0x10);
        if (!Crt().strncmp(gLobbyScript_Load_774d78, cursor, 7)) {
            Island(self, cursor, name, a2, index, a4, a5);
        } else if (!Crt().strncmp(gLobbyScript_Load_774d6c, cursor, 0xb)) {
            FlyingMesh(self, cursor);
        } else if (!Crt().strncmp(gLobbyScript_Load_774d60, cursor, 0xa)) {
            SkipPast(cursor, '(');
            G<s32>(world + 0x5c) = IntToken(cursor, ')');
        } else if (!Crt().strncmp(gLobbyScript_Load_774d58, cursor, 6)) {
            SkipPast(cursor, '(');
            G<s32>(world + 0x60) = IntToken(cursor, ')');
        } else if (!Crt().strncmp(gLobbyScript_Load_774d4c, cursor, 0xa)) {
            SkipPast(cursor, '(');
            const s32 red = IntToken(cursor, ',');
            const s32 green = IntToken(cursor, ',');
            const s32 blue = IntToken(cursor, ')');
            G<s32>(world + 0x68) = green << 16;
            G<s32>(world + 0x6c) = blue << 16;
            G<s32>(world + 0x64) = red << 16;
        }
        while (*cursor) ++cursor;
        ++cursor;
    }
    g_calls.Cd(0x005f5680, block);   // Mem_GlobalFree
    String_Dtor(&path);
}
}
