// The vehicle that brings the visitors: see arrivalvehicle.h.
#include "arrivalvehicle.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/string.h"
#include "objectdefs.h"
#include "ridelist.h"
#include "things.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const ArrivalVehicleEnv kGameEnv = {
    reinterpret_cast<u8 *(__fastcall *)(u8 *, void *, u32, u32, s32, u32)>((uintptr_t)0x004db090),
    Rides_FindById,
    Rides_SetElement,
};
const ArrivalVehicleEnv *g_env = &kGameEnv;

void set_element_of_thing(u32 handle, s32 index) {
    const u8 *const thing = global<u8 *>(0x007cfb90 + 20 * handle);
    g_env->setElement(g_env->findRide(at<s32>(thing, 0x24)), index, 1);
}

struct Kind {
    u32 name;
    u32 word;
    u32 bit;
};
const Kind kKinds[3] = {{0x00802370, 0x1da72c, 1}, {0x00801d58, 0x1da72e, 2}, {0x007cf3f0, 0x1da730, 4}};

}  // namespace

void ArrivalVehicle_SetEnv(const ArrivalVehicleEnv *env) { g_env = env ? env : &kGameEnv; }

u16 *__fastcall Game_ArrivalVehicle(u8 *game, void *, u16 *out, u32 which) {
    u16 &current = at<u16>(game, 0x1da72a);
    if (current != 0) {
        set_element_of_thing(current, 0);
        *out = current;
        return out;
    }
    WString name;
    WString_Ctor(&name);
    u8 *const defs = reinterpret_cast<u8 *>((uintptr_t)0x007890a0);
    ObjectDefs_First(defs);   // (its answer unused, sic)
    u16 *slot = nullptr;
    u32 tried = 0;
    for (;;) {
        const Kind *kind = nullptr;
        if (tried == 0) {
            u32 index;
            if (which == 0) {
                u32 &random = at<u32>(game, 0x1da708);
                random = random * 0x19660d + 0x3c6ef35f;
                random = random >> 13 | random << 19;
                index = ((s32)random < 0 ? 0u - random : random) % 3;
            } else {
                index = which - 1;
            }
            if (index < 3) kind = &kKinds[index];
        } else {
            for (s32 i = 2; i >= 0 && !kind; --i)
                if (!(tried & kKinds[i].bit)) kind = &kKinds[i];
            if (!kind) break;
        }
        if (kind) {
            WString copy;
            WString_CtorCopy(&copy, nullptr, reinterpret_cast<const WString *>((uintptr_t)kind->name));
            WString_AssignNarrow(&name, nullptr, reinterpret_cast<const String *>(&copy));
            WString_Dtor(&copy);
            slot = &at<u16>(game, kind->word);
            tried += kind->bit;
        }
        if (*slot != 0) {
            set_element_of_thing(*slot, 0);
            current = *slot;
            break;
        }
        u8 *def = ObjectDefs_First(defs);
        while (def && Crt().wcsicmp(reinterpret_cast<const u16 *>(def + 0x390), name.data) != 0) def = ObjectDefs_Next(defs);
        if (!def) continue;
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x450));
        u8 *const object = block ? g_env->construct(block, nullptr, 1, at<u16>(def, 0), 0, 0) : nullptr;
        u16 handle;
        *slot = *Thing_GetHandle(object, nullptr, &handle);
        g_env->setElement(g_env->findRide(at<s32>(object, 0x24)), 1, 1);
        current = *slot;
        break;
    }
    WString_Dtor(&name);
    *out = current;
    return out;
}
