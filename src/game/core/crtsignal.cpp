#include "crtsignal.h"

#include <cstdint>

#include "crt.h"
#include "crtmessage.h"
#include "crtstartup.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

using Handler = void(__cdecl *)(s32 sig);
using FpeHandler = void(__cdecl *)(s32 sig, u32 code);

}  // namespace

extern "C" {

s32 __cdecl CRT_raise(s32 sig) {
    u32 *action;
    u8 *data = nullptr;
    bool locked = true;
    switch (sig) {
    case 2: action = &global<u32>(0x00fb1918); break;
    case 21: action = &global<u32>(0x00fb191c); break;
    case 22: action = &global<u32>(0x00fb1920); break;
    case 15: action = &global<u32>(0x00fb1924); break;
    case 4:
    case 8:
    case 11:
        data = CrtInner().getptd();
        action = CRT_siglookup(static_cast<u32>(sig), *reinterpret_cast<u32 **>(data + 0x50)) + 2;
        locked = false;
        break;
    default: return -1;
    }
    const u32 handler = *action;
    if (locked) CrtInner().lock(1);
    if (handler == 1) {
        if (locked) CrtInner().unlock(1);
        return 0;
    }
    if (!handler) {
        if (locked) CrtInner().unlock(1);
        CRT__exit(3);
    }
    const bool exception = sig == 8 || sig == 11 || sig == 4;
    u32 old_pointers = 0, old_code = 0;
    if (exception) {
        old_pointers = *reinterpret_cast<u32 *>(data + 0x54);
        *reinterpret_cast<u32 *>(data + 0x54) = 0;
        if (sig == 8) {
            old_code = *reinterpret_cast<u32 *>(data + 0x58);
            *reinterpret_cast<u32 *>(data + 0x58) = 0x8c;
        }
    }
    if (sig == 8) {
        for (s32 i = global<s32>(0x00781508); i < global<s32>(0x00781508) + global<s32>(0x0078150c); ++i) {
            (*reinterpret_cast<u32 **>(data + 0x50))[i * 3 + 2] = 0;
        }
    } else {
        *action = 0;
    }
    if (locked) CrtInner().unlock(1);
    if (sig == 8) reinterpret_cast<FpeHandler>((uintptr_t)handler)(sig, *reinterpret_cast<u32 *>(data + 0x58));
    else reinterpret_cast<Handler>((uintptr_t)handler)(sig);
    if (exception) {
        *reinterpret_cast<u32 *>(data + 0x54) = old_pointers;
        if (sig == 8) *reinterpret_cast<u32 *>(data + 0x58) = old_code;
    }
    return 0;
}

void __cdecl CRT_abort() {
    CRT_NMSG_WRITE(10);
    CRT_raise(22);
    CRT__exit(3);
}
}
