#include "pathdrag.h"

#include <cstdint>

namespace {

const PathDragEnv kGameEnv = {reinterpret_cast<void(__fastcall *)(u8 *, void *, u32, u32)>((uintptr_t)0x005367a0),
                              reinterpret_cast<void(__cdecl *)(u32, s32, s32, s32 *, s32 *)>((uintptr_t)0x00536100)};
const PathDragEnv *g_env = &kGameEnv;

}  // namespace

extern "C" {

void PathDrag_SetEnv(const PathDragEnv *env) { g_env = env ? env : &kGameEnv; }
}
