#include "sharedglobals.h"

#include <cstdint>

#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

void release(u32 address) {
    u8 *const shared = global<u8 *>(address);
    const u32 count = at<u32>(shared, 4);
    if (count != 0 && count != 0xffffffffu) at<u32>(shared, 4) = count - 1;
    if (at<u32>(shared, 4) == 0) vcall<u32>(shared, 0, 1u);
    global<u32>(address) = 0;
}

}  // namespace

extern "C" {

void __cdecl Global7ced48_Release() { release(0x007ced48); }
void __cdecl Globalfb1dc0_Release() { release(0x00fb1dc0); }
void __cdecl Globalfb1dbc_Release() { release(0x00fb1dbc); }
void __cdecl Globalfb1dc4_Release() { release(0x00fb1dc4); }
void __cdecl Globalfb1e54_Release() { release(0x00fb1e54); }
void __cdecl Globalfb1e30_Release() { release(0x00fb1e30); }
void __cdecl Globalfb1e58_Release() { release(0x00fb1e58); }
void __cdecl Globalfb1e34_Release() { release(0x00fb1e34); }
void __cdecl Globalfb1e38_Release() { release(0x00fb1e38); }
void __cdecl Globalfb1e5c_Release() { release(0x00fb1e5c); }
void __cdecl Globalfb1e60_Release() { release(0x00fb1e60); }
void __cdecl Globalfb1e64_Release() { release(0x00fb1e64); }
void __cdecl Globalfb1e3c_Release() { release(0x00fb1e3c); }
void __cdecl Globalfb1e44_Release() { release(0x00fb1e44); }
void __cdecl Globalfb1e4c_Release() { release(0x00fb1e4c); }
void __cdecl Globalfb1ecc_Release() { release(0x00fb1ecc); }
void __cdecl Globalfb1eb4_Release() { release(0x00fb1eb4); }
void __cdecl Globalfb1eb8_Release() { release(0x00fb1eb8); }
void __cdecl Globalfb1ed0_Release() { release(0x00fb1ed0); }
void __cdecl Globalfb1ed4_Release() { release(0x00fb1ed4); }
void __cdecl Globalfb1ed8_Release() { release(0x00fb1ed8); }
void __cdecl Globalfb1ebc_Release() { release(0x00fb1ebc); }
void __cdecl Globalfb1ec4_Release() { release(0x00fb1ec4); }
}
