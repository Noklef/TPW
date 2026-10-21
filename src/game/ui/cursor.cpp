#include "cursor.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/initializers.h"
#include "../core/vcall.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const GameCursorsEnv kGameEnv = {
    reinterpret_cast<u8 *(__cdecl *)(const char *)>((uintptr_t)0x004895b0),
    reinterpret_cast<u8 *(__cdecl *)(const char *)>((uintptr_t)0x00489670),
};
const GameCursorsEnv *g_env = &kGameEnv;

// A cursor file (the game's string of its name) and its slot.
struct CursorFile {
    u32 name;
    u16 slot;
};

const CursorFile kAniCursors[] = {
    {0x0074ff4c, 0x0},   // c_norm.ani
    {0x0074ff40, 0x1},   // c_busy.ani
    {0x0074ff34, 0x3},   // c_path.ani
    {0x0074ff28, 0x4},   // c_queue.ani
    {0x0074ff1c, 0x5},   // c_link.ani
    {0x0074ff10, 0x7},   // c_place.ani
    {0x0074ff00, 0x8},   // c_noplace.cur
    {0x0074fef4, 0x2},   // c_pick.ani
    {0x0074fee8, 0x9},   // c_carry.ani
    {0x0074fedc, 0xa},   // c_erase.ani
    {0x0074fed0, 0xb},   // c_pylon.ani
    {0x0074fec4, 0x13},   // c_cam.ani
    {0x0074feb8, 0x14},   // c_cash.ani
    {0x0074feac, 0x6},   // c_track.ani
    {0x0074fe9c, 0xc},   // c_height.ani
    {0x0074fe8c, 0xd},   // c_rotate.ani
    {0x0074fe80, 0xe},   // c_pivot.ani
    {0x0074fe70, 0xf},   // c_swapdown.ani
    {0x0074fe60, 0x10},   // c_swapup.ani
    {0x0074fe54, 0x12},   // c_end.ani
    {0x0074fe48, 0x11},   // c_move.ani
    {0x0074fe38, 0x15},   // c_crosshair.ani
};

const CursorFile kTgaCursors[] = {
    {0x00750054, 0x0},   // CNor.tga
    {0x00750048, 0x1},   // Cbus.tga
    {0x0075003c, 0x3},   // Cpat.tga
    {0x00750030, 0x4},   // Cque.tga
    {0x00750024, 0x5},   // Clin.tga
    {0x00750018, 0x7},   // Cpla.tga
    {0x0075000c, 0x8},   // Cnog.tga
    {0x00750000, 0x2},   // Cpic.tga
    {0x0074fff4, 0x9},   // Ccar.tga
    {0x0074ffe8, 0xa},   // Cera.tga
    {0x0074ffdc, 0xb},   // Cpyl.tga
    {0x0074ffd0, 0x13},   // Ccam.tga
    {0x0074ffc4, 0x14},   // Cash.tga
    {0x0074ffb8, 0x6},   // Ctra.tga
    {0x0074ffac, 0xc},   // Chei.tga
    {0x0074ffa0, 0xd},   // Crot.tga
    {0x0074ff94, 0xe},   // Cpiv.tga
    {0x0074ff88, 0xf},   // Csdn.tga
    {0x0074ff7c, 0x10},   // Csup.tga
    {0x0074ff70, 0x12},   // CEnd.tga
    {0x0074ff64, 0x11},   // CMov.tga
    {0x0074ff58, 0x15},   // CCro.tga
};

}  // namespace

void GameCursors_SetEnv(const GameCursorsEnv *e) { g_env = e ? e : &kGameEnv; }
const GameCursorsEnv *GameCursors_GetEnv() { return g_env; }

extern "C" {

u8 *__fastcall CursorTable_Get(const u8 *table, void *, u32 index) {
    const u32 i = index & 0xffff;
    if (i < at<u16>(table, 4)) return at<u8 **>(table, 0)[i];
    return nullptr;
}

void __fastcall CursorTable_Clear(u8 *table) {
    if (at<u8 **>(table, 0)) {
        for (s32 i = 0; i < (s32)at<u16>(table, 4); ++i) {
            if (u8 *const cursor = at<u8 **>(table, 0)[i]) vcall<void>(cursor, 0, 1u);
        }
        GameHeap_Free(at<u8 **>(table, 0));
    }
    at<u16>(table, 4) = 0;
}

u16 __fastcall CursorTable_Init(u8 *table, void *, u32 count) {
    at<u8 **>(table, 0) = static_cast<u8 **>(GameHeap_Alloc((count & 0xffff) << 2));
    if (!at<u8 **>(table, 0)) return 0;
    for (s32 i = 0; i < (s32)(count & 0xffff); ++i) at<u8 **>(table, 0)[i] = nullptr;
    at<u16>(table, 4) = (u16)count;
    return 1;
}

u16 __fastcall CursorTable_Set(u8 *table, void *, u32 index, u8 *cursor) {
    const u32 i = index & 0xffff;
    if (i >= at<u16>(table, 4) || !cursor || at<u8 **>(table, 0)[i]) return 0;
    at<u8 **>(table, 0)[i] = cursor;
    return 1;
}

void __cdecl Gui_SetCursorTable(u8 *table) {
    if (!table || table == global<u8 *>(0x00faa604)) return;
    if (global<u8 *>(0x00faa608)) vcall<void>(global<u8 *>(0x00faa608), 0x10, 0u);
    global<u8 *>(0x00faa604) = table;
    u8 *const cursor = CursorTable_Get(global<u8 *>(0x00faa604), nullptr, global<u16>(0x0077c484));
    if (cursor) vcall<void>(cursor, 0x10, 1u);
    global<u8 *>(0x00faa608) = cursor;
}

void __cdecl Gui_SetCursor(u32 id) {
    if (!global<u8 *>(0x00faa604)) return;
    if ((s16)id == global<s16>(0x0077c484)) return;
    if ((s16)id == -1) id = global<u16>(0x00faa600);
    u8 had = 0;
    if (u8 *const old = CursorTable_Get(global<u8 *>(0x00faa604), nullptr, global<u16>(0x0077c484))) {
        vcall<void>(old, 0x10, 0u);
        had = Fn_66190a(old);
    }
    u8 *c = CursorTable_Get(global<u8 *>(0x00faa604), nullptr, (u16)id);
    if (!c) c = CursorTable_Get(global<u8 *>(0x00faa604), nullptr, global<u16>(0x00faa600));
    global<u8 *>(0x00faa608) = c;
    global<u16>(0x0077c484) = (u16)id;
    if (!c) return;
    vcall<void>(c, 0x10, 1u);
    if (had) vcall<void>(c, 0x8);
}

void __fastcall Window_SetCursor(u8 *window, void *, u32 id) {
    at<u16>(window, 0x50) = (u16)id;
    if (window == global<u8 *>(0x00faa5e0)) Gui_SetCursor(at<u16>(window, 0x50));
}

void __fastcall Window_ApplyCursor(const u8 *window) { Gui_SetCursor(at<u16>(window, 0x50)); }

void __cdecl GameCursor_Set(u32 id) {
    u8 *const window = global<u8 *>(0x007cb2ac);
    if (!window) return;
    Window_SetCursor(window, nullptr, id);
    global<s32>(0x007cb2d8) = (s16)id;
}

void __fastcall Obj7b51f0_SetCursor(void *, void *, u32 id) {
    if (global<u8 *>(0x007cb2ac)) {
        GameCursor_Set(id);
        return;
    }
    if (u8 *const window = global<u8 *>(0x007cc8c0)) Window_SetCursor(window, nullptr, id);
    if (u8 *const window = global<u8 *>(0x007cc8bc)) Window_SetCursor(window, nullptr, id);
}

void __cdecl GameCursor_Update() {
    const auto pick = [](u32 otherwise) -> u32 {
        if (global<u32>(0x00816d60)) return 5;
        if (global<u32>(0x00816d4c)) return 0x12;
        return otherwise;
    };
    switch (global<u32>(0x0081ae2c)) {
    case 0x01: GameCursor_Set(pick(3)); break;
    case 0x03: GameCursor_Set(pick(4)); break;
    case 0x0b:
    case 0x10: GameCursor_Set(pick(6)); break;
    case 0x15: GameCursor_Set(global<u32>(0x00816d4c) ? 0x12 : 0xb); break;
    case 0x16: GameCursor_Set(global<u32>(0x00816d4c) ? 0x12 : 0x11); break;
    case 0x04:
    case 0x3b: GameCursor_Set(7); break;
    case 0x39: GameCursor_Set(3); break;
    default: break;
    }
    if (Global816d48_Get()) GameCursor_Set(8);
    if (global<u32>(0x00816d5c)) GameCursor_Set(0x14);
}

void __cdecl GameCursors_LoadAni() {
    u8 *const table = reinterpret_cast<u8 *>((uintptr_t)0x007b9e88);
    for (const CursorFile &f : kAniCursors) {
        if (u8 *const cursor = g_env->loadAni(reinterpret_cast<const char *>((uintptr_t)f.name))) CursorTable_Set(table, nullptr, f.slot, cursor);
    }
}

void __cdecl GameCursors_LoadTga() {
    u8 *const table = reinterpret_cast<u8 *>((uintptr_t)0x007b9ab0);
    for (const CursorFile &f : kTgaCursors) {
        u8 *const cursor = g_env->loadTga(reinterpret_cast<const char *>((uintptr_t)f.name));
        if (!cursor) continue;
        CursorTable_Set(table, nullptr, f.slot, cursor);
        if (f.slot == 9) Fn_6618c7(cursor, nullptr, 10, 0x40);
        else if (f.slot == 0x15) Fn_6618c7(cursor, nullptr, 0x10, 0x10);
    }
}

}
