// A virtual call on one of the game's objects: __thiscall through its vtable, offset bytes
// in. __fastcall with an unused EDX passes `this` and the stack arguments as __thiscall does.
#pragma once

#include "../../../re/types.h"

template <typename R, typename T, typename... A>
R vcall(T *self, u32 offset, A... args) {
    using F = R(__fastcall *)(T *, void *, A...);
    void *const *const vtable = *reinterpret_cast<void *const *const *>(self);
    return reinterpret_cast<F>(vtable[offset / 4])(self, nullptr, args...);
}

// A COM call (DirectX): __stdcall through the vtable, `this` the first stack argument.
template <typename R, typename T, typename... A>
R comcall(T *self, u32 offset, A... args) {
    using F = R(__stdcall *)(T *, A...);
    void *const *const vtable = *reinterpret_cast<void *const *const *>(self);
    return reinterpret_cast<F>(vtable[offset / 4])(self, args...);
}
