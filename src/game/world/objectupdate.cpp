// An object's tick: see objectupdate.h.
#include "objectupdate.h"

#include <cstdint>

#include "../math/x87.h"
#include "mapcell.h"
#include "object.h"
#include "objectwear.h"
#include "person.h"
#include "ridelist.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(u8 *p, u32 offset) { return *reinterpret_cast<T *>(p + offset); }

const ObjectUpdateEnv kGameEnv = {Rides_FindById, Rides_GetElement, Rides_SetElement, Cell_MakeHandle, Map_SpreadRemove,
                                  Object_WearTick, Visitor_IsState11To14, Object_BeginCycle, Object_DismissQueueHead, Object_ArriveWaiting};
const ObjectUpdateEnv *g_env = &kGameEnv;

u8 percent(float value) { return (u8)(long long)value; }

}  // namespace

void ObjectUpdate_SetEnv(const ObjectUpdateEnv *env) { g_env = env ? env : &kGameEnv; }

void __fastcall Object_Update(u8 *object) {
    if (at<u8>(object, 0x32) & 0x80 && g_env->element(g_env->ride(at<s32>(object, 0x24)), 0) == 1) {
        u32 cell = (u32)(uintptr_t)object;  // sic: the local is the pushed ECX; the handle writes its low word only
        g_env->makeHandle(reinterpret_cast<u16 *>(&cell), at<u8>(object, 5), at<u8>(object, 7));
        g_env->spreadRemove(global<u8 *>(0x008023a0), nullptr, 7, cell);
        g_env->setElement(g_env->ride(at<s32>(object, 0x24)), 0, 2);
    }
    const u32 state = at<u32>(object, 0x19c);
    if (state == 3 || state == 4) return;
    if (state != 1 && at<u32>(object, 0x68) && !(at<u8>(global<u8 *>(0x0080239c), 0x1da70c) & 7)) {
        const u8 *const record = g_env->ride(at<s32>(object, 0x24));
        g_env->wearTick(object);
        if ((!percent(at<float>(object, 0x48)) || !percent(at<float>(object, 0x44))) && !(at<u8>(object, 0x32) & 1) && !g_env->element(record, 4)) {
            if (percent(at<float>(object, 0x48))) {
                const X87Reg value = X87Reg(at<float>(object, 0x48)) - 5.0f;
                float &field = at<float>(object, 0x48);
                if (value.v > 100.0) field = 100.0f;
                else if (!(value.v >= 0.0)) field = 0.0f;
                else field = (float)value;
            }
            g_env->setElement(g_env->ride(at<s32>(object, 0x24)), 4, 1);
        }
    }
    if (at<u8>(object, 0x32) & 1 && percent(at<float>(object, 0x44)) < 25) g_env->setElement(g_env->ride(at<s32>(object, 0x24)), 8, 1);
    const u16 handle = at<u16>(object, 0x3c);
    if (!handle) return;
    const u8 *const thing = global<u8 *>(0x007cfb90 + handle * 0x14);
    if (thing && g_env->visitorIs(thing)) return;
    at<u16>(object, 0x3c) = 0;
}

void __fastcall Object_PostUpdate(u8 *object) {
    switch (at<u32>(object, 0x19c)) {
    case 0:
        g_env->beginCycle(object);
        break;
    case 1:
    case 2:
    case 4:
        g_env->dismissHead(object);
        g_env->arriveWaiting(object);
        break;
    default:
        break;
    }
}
