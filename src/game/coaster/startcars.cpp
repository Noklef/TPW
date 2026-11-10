#include "startcars.h"

#include <cstdint>
#include "coastersload.h"
#include "../core/staticobjects.h"

namespace {

AddrCalls g_calls;

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// The object of a part of the holder at this index of the table 0x7a4610.
const u8 *partObject(u32 index, s32 part) {
    const u8 *const holder = global<const u8 *>(0x007a4610 + 4 * index);
    return at<const u8 *>(at<const u8 *>(at<const u8 *>(at<const u8 *>(holder, 8), 0x28), 4), 0x14 * part + 4);
}

}  // namespace

void StartCars_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" void __cdecl Coaster_AddStartCars(u32 keepFrontEnd) {
    at<u32>(gCoastersload_790fec, 0x3c) |= 1;
    u8 *const coaster = gCoastersload_790fec;
    const u8 *const type = at<const u8 *>(coaster, 4);
    const u32 cars = *type & 0x20 ? 1 : 2;
    u32 position[3];   // (a part not found leaves it as it was: at first what the stack holds, sic)
    for (u32 i = 0; i < cars; ++i) {
        if (*type & 0x20) {
            const u8 *const model = at<const u8 *>(coaster, 0x54);
            const s32 part = at<s32>(at<const u8 *>(at<const u8 *>(model, 0x100), 0x18), 0x2c);
            if (part != -1) {
                if (const u8 *const object = partObject(at<u32>(model, 0x70), part))
                    g_calls.Cd(0x00435710, object, position);   // Object_GetPosition
                else
                    position[0] = position[1] = position[2] = 0;
            }
        } else {
            const s32 part = (s32)g_calls.Cd(0x0044b220, global<u32>(0x007a4610 + 4 * at<u32>(coaster, 0x10)), 0x400000u, i + 1);   // Model_FindPartByKey4
            if (part != -1) {
                if (const u8 *const object = partObject(at<u32>(coaster, 0x10), part)) {
                    position[0] = at<u32>(object, 0x30);
                    position[1] = at<u32>(object, 0x34);
                    position[2] = at<u32>(object, 0x38);
                } else {
                    position[0] = position[1] = position[2] = 0;
                }
            }
        }
        const u8 *const link = reinterpret_cast<const u8 *>((uintptr_t)g_calls.Cd(0x00432c10, 0u, 0u, 0x13u, 0u, 0u, position));   // CoasterLink_AddEntry
        const u32 flags = !i && (*type & 0x20) ? 0x144 : 4;
        g_calls.Cd(0x00434060, link, at<u32>(link, 0x1c), at<u32>(link, 0x24), at<u32>(link, 0x20), flags, 0u, 0u, 0u, 0u);   // CoasterLink_AddCar
    }
    at<u32>(gCoastersload_790fec, 0) |= 0x1001;
    if (!global<u32>((u32)(uintptr_t)&gActionRec_785a08 + 0x24))
        g_calls.Cd(0x00442880, 0x10u);   // CoasterEdit_SetTools
    if (!keepFrontEnd)
        g_calls.Cd(0x00498780);   // FrontEnd_EnsureMode
    g_calls.Cd(0x00435730, reinterpret_cast<u32 &>(gCoastersload_790fec), 2u);   // Coaster_SetState
}
