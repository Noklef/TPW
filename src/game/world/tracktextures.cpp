// The track pieces' texture tables and the map's reset: see tracktextures.h.
#include "tracktextures.h"

#include <cstdint>
#include "focuscell.h"
#include "queuepieces.h"
#include "tracktextures.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
u32 pieces(u32 offset) { return (u32)(uintptr_t)gPieceTables + offset; }
template <typename F> F game(u32 address) { return reinterpret_cast<F>((uintptr_t)address); }

const TrackTexturesEnv kGameEnv = {
    game<u32(__cdecl *)()>(0x0040ef40),
    game<s32(__cdecl *)(char *, u32, u32)>(0x0067a650),
    game<u32(__cdecl *)(u32, const char *, u32)>(0x00461a60),
    game<u32(__cdecl *)(const char *, u32, u32 *, u32 *, u32, u32, u32, u32, u32, u32, u32)>(0x004629d0),
    game<void(__fastcall *)(u8 *, void *, u32)>(0x00536490),
    game<void(__fastcall *)(u8 *)>(0x0053af00),
    game<void(__cdecl *)(u32)>(0x00527940),
    game<void(__cdecl *)()>(0x005323f0),
};
const TrackTexturesEnv *g_env = &kGameEnv;

// The tables after the queue's: [slot + 4] the name; none: the slot before's handle; (`dash`) a name starting '-': 0.
void load_table(u32 dimensions, u32 format, u32 first, u32 end, u32 first_id, u32 flags, bool dash) {
    global<u32>(dimensions) = 2;
    global<u32>(dimensions + 4) = 2;
    char folder[0x104];
    g_env->format(folder, format, g_env->basePath());
    u32 open = 0;
    u32 id = first_id;
    for (u32 slot = first; slot < end; slot += 0xc, ++id) {
        const u32 name = global<u32>(slot + 4);
        if (!name) {
            global<u32>(slot) = global<u32>(slot - 0xc);
        } else if (dash && global<u8>(name) == '-') {
            global<u32>(slot) = 0;
        } else {
            if (!open) open = g_env->openFolder(name, folder, open);
            global<u32>(slot) = g_env->load(folder, name, &global<u32>(dimensions), &global<u32>(dimensions), flags, id, 1, 0, 0, 0, 0);
        }
    }
}

}  // namespace

void TrackTextures_SetEnv(const TrackTexturesEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl TrackTextures_LoadQueue() {
    global<u32>(0x00816d68) = 1;
    global<u32>(0x00816d6c) = 1;
    char folder[0x104];
    g_env->format(folder, 0x0076394c, g_env->basePath());
    u32 open = 0;
    u32 id = 0x4268;
    for (u32 slot = pieces(4); slot < pieces(0x64); slot += 0xc, ++id) {
        const u32 name = global<u32>(slot);
        if (!open) open = g_env->openFolder(name, folder, open);
        const u32 flags = global<u8>(slot + 5) ? 0x112423 : 0x12423;
        global<u32>(slot - 4) = g_env->load(folder, name, &global<u32>(0x00816d68), &global<u32>(0x00816d68), flags, id, 1, 0, 0, 0, 0);
    }
}

void __cdecl TrackTextures_LoadGoKarts() { load_table(0x008179f8, 0x00763958, pieces(0x60), pieces(0x150), 0x42cc, 0x10422, true); }

void __cdecl TrackTextures_LoadWaterRide() { load_table(0x008179f8, 0x0076396c, pieces(0x150), pieces(0x27c), 0x4330, 0x10422, true); }

void __cdecl TrackTextures_LoadHoarding() { load_table(0x008179f8, 0x00763980, pieces(0x280), pieces(0x3d0), 0x4394, 0x422, false); }

void __cdecl Track_ResetMap(s32 columns, s32 rows) {
    global<s32>(0x0081ede4) = -1;
    global<s32>(0x0081ede8) = -1;
    global<u8>(0x0081b4cc) = 0;
    global<u8>(0x0081af7c) = 0;
    gTracktextures_81d740 = columns;
    global<s32>(0x0081b1b0) = rows;
    global<u32>(0x0081af4c) = 0;
    global<u32>(0x0081d7a4) = 0;
    global<u32>(0x0081b134) = 0;
    global<u32>(0x0081ae2c) = 0;
    global<u32>(0x0081af64) = 0;
    gTracktextures_8186ec = 0;
    for (s32 row = 0; row < rows; ++row) {
        const u32 base = ((u32)row << 7) + 1;
        for (s32 column = 0; column < columns; ++column) {
            const u32 cell = ((u32)column + base) & 0xffff;
            g_env->cellReset(global<u8 *>(0x007cf83c) + cell * 0x44 + 0x294, nullptr, 1);
            g_env->trackReset(global<u8 *>(0x007cf83c) + (cell + 0x6cde) * 0x28);
        }
    }
    g_env->setList(0);
    global<u32>(0x00816d50) = 0;
    global<u32>(0x00816d58) = 0;
    global<u32>(0x00816d60) = 0;
    global<u32>(0x00816d5c) = 0;
    global<u32>(0x00816d4c) = 0;
    global<u32>(0x0081b0cc) = 1;
    global<u32>(0x0081ecac) = 0;
    global<u32>(0x0081d7a8) = 0;
    global<u32>(0x0081b46c) = 0;
    TrackTextures_LoadQueue();
    TrackTextures_LoadGoKarts();
    TrackTextures_LoadWaterRide();
    TrackTextures_LoadHoarding();
    global<u32>(0x00820a8c) = 0;
    reinterpret_cast<u32 &>(gFocuscell_8186ac) = 0;
    global<u16>(0x00818698) = 0;
    g_env->layEntrance();
    global<u32>(0x0081b604) = 0;
}
}
