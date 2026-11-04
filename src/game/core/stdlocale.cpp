#include "stdlocale.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *facet_of(const u8 *imp, u32 id) { return id < at<u32>(imp, 0xc) ? at<u8 **>(imp, 8)[id] : nullptr; }

}  // namespace

extern "C" {

u8 *__fastcall Locale_Getfacet(u8 **self, void *, u32 id, u32 transparent) {
    const u8 *const imp = *self;
    u8 *const facet = facet_of(imp, id);
    if (facet || !static_cast<u8>(transparent) || !at<u8>(imp, 0x14)) return facet;
    return facet_of(global<u8 *>(0x00fb1dd0), id);
}

u8 __fastcall Locale_Iscloc(u8 **self) {
    const u8 *const imp = *self;
    const u8 *const classic = global<u8 *>(0x00fb1dcc);
    if (imp == classic) return 1;
    return at<u8>(imp, 0x14) && global<u8 *>(0x00fb1dd0) == classic ? 1 : 0;
}

}  // extern "C"
