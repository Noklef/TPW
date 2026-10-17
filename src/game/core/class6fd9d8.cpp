#include "class6fd9d8.h"

#include <cstdint>

#include "gameheap.h"
#include "string.h"
#include "vcall.h"
#include "../files/handles.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

namespace {

u8 *Finish(u8 *object, u32 flags) {
    if (flags & 1) GameHeap_Free(object);
    return object;
}

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

void __fastcall Class6fd9d8_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x006fd9d8;
    if (u8 *member = at<u8 *>(object, 0x3c))
        vcall<u32>(member, 0xc, 1u);
    u8 *list = at<u8 *>(object, 0x2c);
    at<u32>(object, 0x3c) = 0;
    if (list) {
        while (at<u32>(list, 0xc) > 0) {
            u8 *node = at<u8 *>(list, 4);
            u8 *held = nullptr;
            if (node) {
                u8 *next = at<u8 *>(node, 4);
                at<u8 *>(list, 4) = next;
                if (next)
                    at<u32>(next, 8) = 0;
                else
                    at<u32>(list, 8) = 0;
                held = at<u8 *>(node, 0);
                at<u32>(list, 0xc) = at<u32>(list, 0xc) - 1;
                GameHeap_Free(node);
            }
            if (held)
                vcall<u32>(held, 0x2c, 1u);
            list = at<u8 *>(object, 0x2c);
        }
        if (u8 *l = at<u8 *>(object, 0x2c))
            vcall<u32>(l, 0, 1u);
        at<u32>(object, 0x2c) = 0;
    }
    String_Dtor(reinterpret_cast<String *>(object + 0x18));
    String_Dtor(reinterpret_cast<String *>(object + 4));
}

void __fastcall Class6fda48_Dtor(u8 *object) {
    *reinterpret_cast<u32 *>(object) = 0x006fda48;
    Class6fd9d8_Dtor(object);
}

void __fastcall Class6fda68_Dtor(u8 *object) {
    *reinterpret_cast<u32 *>(object) = 0x006fda68;
    Class6fd9d8_Dtor(object);
}

void __fastcall Class6fdaa8_Dtor(u8 *object) {
    *reinterpret_cast<u32 *>(object) = 0x006fdaa8;
    Class6fd9d8_Dtor(object);
}

void __fastcall Class6fdab8_Dtor(u8 *object) {
    *reinterpret_cast<u32 *>(object) = 0x006fdab8;
    Class6fd9d8_Dtor(object);
}

void __fastcall Class6fdae8_Dtor(u8 *object) {
    *reinterpret_cast<u32 *>(object) = 0x006fdae8;
    Class6fd9d8_Dtor(object);
}

void __fastcall Class6fdaf8_Dtor(u8 *object) {
    *reinterpret_cast<u32 *>(object) = 0x006fdaf8;
    Class6fd9d8_Dtor(object);
}

u8 *__fastcall Class6fda48_ScalarDtor(u8 *object, void *, u32 flags) {
    Class6fda48_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class6fda68_ScalarDtor(u8 *object, void *, u32 flags) {
    Class6fda68_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class6fdaa8_ScalarDtor(u8 *object, void *, u32 flags) {
    Class6fdaa8_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class6fdab8_ScalarDtor(u8 *object, void *, u32 flags) {
    Class6fdab8_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class6fdae8_ScalarDtor(u8 *object, void *, u32 flags) {
    Class6fdae8_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class6fdaf8_ScalarDtor(u8 *object, void *, u32 flags) {
    Class6fdaf8_Dtor(object);
    return Finish(object, flags);
}

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

u8 *__fastcall Class6fda58_ScalarDtorB(u8 *object, void *, u32 flags) {
    Class6fda58_DtorB(object);
    return Finish(object, flags);
}

u8 *__fastcall Class6fda88_FullScalarDtor(u8 *object, void *, u32 flags) {
    Class6fda88_FullDtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class6fdaa4_ScalarDtor(u8 *object, void *, u32 flags) {
    Class6fdaa4_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class6fdac8_FullScalarDtor(u8 *object, void *, u32 flags) {
    Class6fdac8_FullDtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class6fda58_ScalarDtorC(u8 *object, void *, u32 flags) {
    Class6fda58_DtorC(object);
    return Finish(object, flags);
}
}  // extern "C"
