#include "pathdrag.h"

#include <cstdint>

#include "cellkind.h"
#include "cellmessage.h"
#include "mapcell.h"
#include "world.h"
#include "focuscell.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
u32 ident(s32 x, s32 y) { return ((u32)y * 128 + (u32)x + 1) & 0xffff; }

const PathDragEnv kGameEnv = {MapCell_Clear,
                              reinterpret_cast<void(__cdecl *)(u32, s32, s32, s32 *, s32 *)>(&Map_SendLine)};
const PathDragEnv *g_env = &kGameEnv;

}  // namespace

extern "C" {

void PathDrag_SetEnv(const PathDragEnv *env) { g_env = env ? env : &kGameEnv; }

void __cdecl PathDrag_Begin(s32 x, s32 y, u32 keep) {
    gFocusCell_DragY_763a04 = y;
    gFocusCell_DragX_763a00 = x;
    u8 *const cell = gWorldPtr + ident(x, y) * 0x44 + 0x294;
    u8 *const entry = MapHandle_Entry28(reinterpret_cast<const u16 *>(cell));
    gPathdrag_818c58 = at<u32>(entry, 4);
    at<u32>(entry, 4) = 0;
    if (!keep) g_env->cell5367a0(cell, nullptr, 0, 0);
    gPathdrag_818c54 = 1;
}

void __cdecl PathDrag_Finish() {
    s32 x = gFocusCell_DragX_763a00, y = gFocusCell_DragY_763a04;
    if (x == -1 || y == -1 || !gPathdrag_818c54) return;
    g_env->step536100(0x15, x, y, &x, &y);
    g_env->step536100(0x83, x, y, &x, &y);
    g_env->step536100(0x81, x, y, &x, &y);
    at<u32>(gWorldPtr, ident(x, y) * 0x28 + 0x1102b4) = gPathdrag_818c58;
    gPathdrag_818c54 = 0;
    gFocusCell_DragX_763a00 = -1;
    gFocusCell_DragY_763a04 = -1;
}
}
