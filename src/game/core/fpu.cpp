// The x87 control word guard: see fpu.h.
#include "fpu.h"

#include <cstdint>

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const FpuEnv kGameEnv = {
    at<void(__cdecl *)(void *)>(0x0067acb0),
    at<void *const *>(0x007035d8),
    at<void *const *>(0x007035dc),
};
const FpuEnv *g_env = &kGameEnv;

}  // namespace

void Fpu_SetEnv(const FpuEnv *e) { g_env = e ? e : &kGameEnv; }
const FpuEnv *Fpu_GetEnv() { return g_env; }

extern "C" {

u32 __fastcall FpuControl_Merge(const FpuControl *f, void *, u32 cw) {
    return (cw & ~static_cast<u32>(f->changed[1] | f->changed[0])) | f->saved;
}

void __fastcall FpuControl_Restore(FpuControl *f) {
    if (f->changed[1] | f->changed[0]) {
        u16 cw;
        __asm fstcw cw
        u16 merged = static_cast<u16>(FpuControl_Merge(f, nullptr, cw));
        __asm fldcw merged
    }
    f->changed[1] = 0;
    f->changed[0] = 0;
}

void __fastcall FpuControl_Dtor(FpuControl *f) {
    f->vtable = const_cast<void **>(g_env->baseVtable);
    FpuControl_Restore(f);
}

FpuControl *__fastcall FpuControl_DeletingDtor(FpuControl *f, void *, u32 flags) {
    FpuControl_Dtor(f);
    if (flags & 1) g_env->free(f);
    return f;
}

FpuControl *__fastcall FpuControl_BaseDeletingDtor(FpuControl *f, void *, u32 flags) {
    FpuControl_Dtor(f);
    if (flags & 1) g_env->free(f);
    return f;
}

}  // extern "C"

extern "C" {

u32 __cdecl Fpu_SinglePrecision() {
    u16 cw;
    __asm fnstcw cw
    u16 old = cw;
    cw &= 0xfcff;
    __asm fldcw cw
    return old;
}

void __cdecl Fpu_LoadControl(u32 cw) {
    __asm fldcw word ptr cw
}

void __cdecl Fpu_LoadControlFullPrecision(u32 cw) {
    cw |= 0x300;
    __asm fldcw word ptr cw
}

u8 *__fastcall Class7035d8_Ctor(u8 *object) {
    FpuControl *control = reinterpret_cast<FpuControl *>(object + 4);
    control->vtable = reinterpret_cast<void *>(static_cast<std::uintptr_t>(0x007035d8));
    object[0] = 0;
    control->saved = 0;
    control->changed[0] = 0;
    control->changed[1] = 0;
    return object;
}

u8 *__cdecl Class7035d8_InitGlobal() { return Class7035d8_Ctor(reinterpret_cast<u8 *>(0x00fa78e8)); }

}  // extern "C"
