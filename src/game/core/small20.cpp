#include "small20.h"

extern "C" {

void __stdcall CRT_VectorCtorIterator(u8 *array, u32 size, s32 count, void(__fastcall *ctor)(void *), void *) {
    u8 *element = array;
    for (s32 i = 0; i < count; ++i) {
        ctor(element);
        element += size;
    }
}

void __stdcall CRT_VectorDtorIterator(u8 *array, u32 size, s32 count, void(__fastcall *dtor)(void *)) {
    u8 *element = array + count * size;
    for (s32 remaining = count - 1; remaining >= 0; --remaining) {
        element -= size;
        dtor(element);
    }
}

}  // extern "C"
