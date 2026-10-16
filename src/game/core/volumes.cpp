#include "volumes.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// Every share again, from the volumes and the master.
void Reshare(u8 *object) {
    u32 sum = 0;
    for (u32 i = 0; i < 5; i++)
        sum += at<u32>(object, 0x140c + 0x10 * i);
    for (u32 i = 0; i < 5; i++) {
        u32 &share = at<u32>(object, 0x1410 + 0x10 * i);
        if (sum == 0) {
            share = 0;
            continue;
        }
        const u32 percent = (at<u32>(object, 0x140c + 0x10 * i) * 100) / sum;
        const u32 scaled = percent * at<u32>(object, 0x1460);
        share = (u32)(((unsigned long long)scaled * 0x51eb851fu) >> 37);  // scaled / 100
    }
}

}  // namespace

extern "C" {

void __fastcall Volumes_SetMaster(u8 *object, void *, u32 volume) {
    at<u32>(object, 0x1460) = volume < 0x64 ? volume : 0x64;
    Reshare(object);
}

u32 __fastcall Volumes_SetChannel(u8 *object, void *, u32 channel, u32 volume) {
    at<u32>(object, 0x140c + (channel << 4)) = volume < 0x64 ? volume : 0x64;
    Reshare(object);
    return at<u32>(object, 0x140c + (channel << 4));
}
}
