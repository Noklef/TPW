#include "small47.h"

#include <cstdint>

#include "gameheap.h"
#include "small32.h"

namespace {

u8 *Finish(u8 *object, u32 flags) {
    if (flags & 1) GameHeap_Free(object);
    return object;
}

}  // namespace

extern "C" {

void __fastcall Class702ec0_Dtor(u8 *object) {
    *reinterpret_cast<u32 *>(object) = 0x00702ec0;
    Class702cb0_Dtor(object);
}

void __fastcall Class702dd0_Dtor(u8 *object) {
    *reinterpret_cast<u32 *>(object) = 0x00702dd0;
    Class702cb0_Dtor(object);
}

u8 *__fastcall Class702ec0_ScalarDtor(u8 *object, void *, u32 flags) {
    Class702ec0_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class702dd0_ScalarDtor(u8 *object, void *, u32 flags) {
    Class702dd0_Dtor(object);
    return Finish(object, flags);
}

}  // extern "C"
