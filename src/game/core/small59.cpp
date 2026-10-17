#include "small59.h"

#include <cstdint>

#include "../files/handles.h"
#include "small35.h"
#include "string.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

void BaseFinish(u8 *object) {
    at<u32>(object, 0) = 0x006fda58;
    Handle_Dtor(reinterpret_cast<Handle *>(object + 0x40));
    Class6fd9d8_Dtor(object);
}

void Derived(u8 *object, u32 vtable) {
    at<u32>(object, 0) = vtable;
    Handle_Dtor(reinterpret_cast<Handle *>(object + 0x48));
    BaseFinish(object);
}

}  // namespace

extern "C" {

void __fastcall Class6fda58_DtorA(u8 *object) { BaseFinish(object); }
void __fastcall Class6fda58_DtorB(u8 *object) { BaseFinish(object); }
void __fastcall Class6fda58_DtorC(u8 *object) { BaseFinish(object); }

void __fastcall Class6fda88_FullDtor(u8 *object) { Derived(object, 0x006fda88); }
void __fastcall Class6fdac8_FullDtor(u8 *object) { Derived(object, 0x006fdac8); }

void __fastcall Class6fdaa4_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x006fdaa4;
    Handle_Dtor(reinterpret_cast<Handle *>(object + 0x2c));
    String_Dtor(reinterpret_cast<String *>(object + 0x18));
    String_Dtor(reinterpret_cast<String *>(object + 4));
}

}  // extern "C"
