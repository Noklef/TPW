#include "small31.h"

#include <cstdint>

#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

void Dtor(u8 *object) {
    at<u32>(object, 0) = 0x00703780;
    if (u8 *member = at<u8 *>(object, 8))
        vcall<u32>(member, 0);
    at<u32>(object, 0) = 0x00703770;
}

}  // namespace

extern "C" {

void __fastcall Class703780_DtorA(u8 *object) { Dtor(object); }
void __fastcall Class703780_DtorB(u8 *object) { Dtor(object); }
void __fastcall Class703780_DtorC(u8 *object) { Dtor(object); }

}  // extern "C"
