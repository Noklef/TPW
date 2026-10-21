#include "refcounted.h"

#include <cstdint>

#include "vcall.h"

namespace {

template <typename F> F Import(u32 slot) { return *reinterpret_cast<const F *>((uintptr_t)slot); }
using Interlocked = s32(__stdcall *)(void *);

}  // namespace

s32 __fastcall Class703770_AddRef(u8 *object) { return Import<Interlocked>(0x006fd0c0)(object + 4); }

s32 __fastcall Class703770_Release(u8 *object) {
    if (Import<Interlocked>(0x006fd12c)(object + 4)) return *reinterpret_cast<volatile s32 *>(object + 4);
    if (object) vcall<void>(object, 8, 1u);
    return 0;
}
