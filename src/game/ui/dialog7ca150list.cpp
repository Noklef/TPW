// The dialog [0x7ca150]'s list's handler: see dialog7ca150list.h.
#include "dialog7ca150list.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/circbuf.h"
#include "../math/x87.h"
#include "../world/object.h"
#include "window.h"
#include "window7059f0.h"
#include "window7059f0texts.h"
#include "windowtimers.h"
#include "dialog7ca150.h"

namespace {

const Dialog7ca150ListEnv kGameEnv = {&Window_SetTimer, &Window_StartTimer, &Window7059f0_NthItemData, &Window7059f0_SetItemTexts, &Fn_4e0df0, &Object_Rating, &Object_Field48AsInt, &Object_HistoryDifference, &Obj_Diff180_184, &Object_HistoryAverage, &Window_CallDefault};
const Dialog7ca150ListEnv *g_env = &kGameEnv;

const u32 kOwners = 0x007cfb90;  // (20 bytes each)

s32 Percent(u32 value) { return static_cast<s32>(value << 10) / 100; }
u32 Truncated(double value) { return static_cast<u32>(static_cast<__int64>(value)); }

u32 Sum30(const u8 *object) {
    double sum;
    CircBufU32_30_Sum(object + 0x1a8, nullptr, &sum, 0x1e);
    return Truncated(sum);
}

u8 *ItemData(u8 *window, s32 row) {
    u32 data;
    g_env->nth(window, nullptr, row, &data);
    return reinterpret_cast<u8 *>((uintptr_t)data);
}

const u8 *OwnerOr(u8 *object) {
    u32 first, second;
    if (*reinterpret_cast<const u16 *>((uintptr_t)g_env->owner(object, nullptr, static_cast<u32>(reinterpret_cast<uintptr_t>(&first)))) == 0) return object;
    const u32 handle = *reinterpret_cast<const u16 *>((uintptr_t)g_env->owner(object, nullptr, static_cast<u32>(reinterpret_cast<uintptr_t>(&second))));
    return *reinterpret_cast<const u8 **>((uintptr_t)kOwners + handle * 20);
}

void Tab0(u8 *window, s32 row) {
    const u8 *const object = OwnerOr(ItemData(window, row));
    u32 texts[5];
    texts[0] = 0;
    texts[1] = Sum30(object);
    texts[2] = Percent(g_env->rating(object, nullptr, 0));
    texts[3] = Percent(Truncated(*reinterpret_cast<const float *>(object + 0x44)) & 0xff);
    texts[4] = Percent(g_env->field48(object) & 0xff);
    g_env->setTexts(window, nullptr, row, texts, 0x1e, row == 0 ? 1 : 0, 0xffffffff);
}

void Tab1(u8 *window, s32 row) {
    const u8 *const object = ItemData(window, row);
    const u8 *const buffer = object + 0x1a8;
    u32 texts[5];
    texts[0] = 0;
    s32 count = 0x1e;
    if (static_cast<s32>(CircBufU32_30_Count(buffer)) < 0x1e) count = static_cast<s32>(CircBufU32_30_Count(buffer));
    double sum = 0;
    for (s32 i = 0; i < count; ++i) {
        u32 value;
        if (CircBufU32_30_Get(buffer, nullptr, &value, i) == 1) sum = X87_Add(static_cast<double>(value), sum);
    }
    texts[1] = Truncated(sum);
    texts[2] = g_env->difference(object);
    texts[3] = g_env->diff180(object);
    texts[4] = Percent(g_env->average(object));
    g_env->setTexts(window, nullptr, row, texts, 0x1e, row == 0 ? 1 : 0, 0xffffffff);
}

void Tab2(u8 *window, s32 row) {
    const u8 *const object = ItemData(window, row);
    u32 texts[6];
    texts[0] = 0;
    texts[1] = Sum30(object);
    texts[2] = Percent(g_env->rating(object, nullptr, 0));
    texts[3] = g_env->difference(object);
    texts[4] = g_env->diff180(object);
    texts[5] = Percent(g_env->average(object));
    g_env->setTexts(window, nullptr, row, texts, 0x3e, row == 0 ? 1 : 0, 0xffffffff);
}

}  // namespace

void Dialog7ca150List_SetEnv(const Dialog7ca150ListEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __cdecl Dialog7ca150_ListHandler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x15) {
        if (static_cast<u16>(g_env->setTimer(window, nullptr, 0x80083, 0x7d0)) != 0) g_env->startTimer(window, nullptr, 0x80083);
    } else if (message == 0x10 && a == 0x80083) {
        const u32 tab = gDialog7ca150_7ca154;
        if (tab < 3 && window) {
            for (s32 row = *reinterpret_cast<const s32 *>(window + 0x150) - 1; row >= 0; --row) {
                if (tab == 0) Tab0(window, row);
                else if (tab == 1) Tab1(window, row);
                else Tab2(window, row);
            }
        }
    }
    return g_env->call(window, nullptr, message, a, b);
}
}
