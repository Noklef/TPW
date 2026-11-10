// The objects' lists dialog's rows: see dialog7ca150rows.h.
#include "dialog7ca150rows.h"

#include <cstdint>

#include "../core/circbuf.h"
#include "../world/object.h"
#include "../world/objectdefs.h"
#include "../world/world.h"
#include "dialog7ca210.h"
#include "window7059f0.h"
#include "dialog7ca150.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(u8 *p, u32 offset) { return *reinterpret_cast<T *>(p + offset); }

const Dialog7ca150RowsEnv kGameEnv = {&Object_GetDef, &Object_Rating, &Object_Field48AsInt, &Object_HistoryDifference, &Obj_Diff180_184, &Object_HistoryAverage, &ObjectControls_GetWord18, &Window7059f0_FindItemData, &Window7059f0_AddItem};
const Dialog7ca150RowsEnv *g_env = &kGameEnv;

u8 *listOfDialog() { return gDialog7ca150_7ca14c; }

// The row's first texts: the thing's name (the figures' definition asked first) and the newest-30 sum of the figures' ring +0x1a8.
void nameAndSum(u32 *texts, u8 *thing, u8 *figures) {
    g_env->def(figures);
    texts[0] = (u32)(uintptr_t)Thing_NameBuffer7ba058(thing);
    double sum;
    CircBufU32_30_Sum(figures + 0x1a8, nullptr, &sum, 0x1e);
    texts[1] = (u32)(long long)sum;   // __ftol, the low half
}

s32 percent(u32 byte) { return static_cast<s32>((byte & 0xff) << 10) / 100; }
s32 signedPercent(u32 value) { return static_cast<s32>(value << 10) / 100; }

}  // namespace

void Dialog7ca150_SetRowsEnv(const Dialog7ca150RowsEnv *env) { g_env = env ? env : &kGameEnv; }

s32 __cdecl Dialog7ca150_AddRow(u8 *thing, u8 *list, u8 *figures) {
    u8 *const object = figures ? figures : thing;
    u32 texts[5];
    nameAndSum(texts, thing, object);
    texts[2] = static_cast<u32>(percent(g_env->rating(object, nullptr, 0)));
    texts[3] = static_cast<u32>(percent((u32)(long long)at<float>(object, 0x44)));
    texts[4] = static_cast<u32>(percent(g_env->field48(object)));
    s32 parent = -1;
    if (figures) {
        parent = g_env->find(list, nullptr, (u32)(uintptr_t)figures);
        if (parent < 0) return parent;
    }
    return g_env->add(list, nullptr, texts, (u32)(uintptr_t)thing, parent, 0);
}

u32 __cdecl Dialog7ca150_AddPlainRow(u8 *thing) {
    Dialog7ca150_AddRow(thing, listOfDialog(), nullptr);
    return 1;
}

u32 __cdecl Dialog7ca150_AddLinkedRow(u8 *thing) {
    const u16 link = at<u16>(thing, 0x30);
    u8 *const figures = link ? global<u8 *>(0x007cfb90 + link * 0x14) : nullptr;
    Dialog7ca150_AddRow(thing, listOfDialog(), figures);
    return 1;
}

u32 __cdecl Dialog7ca150_AddHistoryRow(u8 *thing) {
    u8 *const list = listOfDialog();
    u32 texts[5];
    nameAndSum(texts, thing, thing);
    texts[2] = g_env->difference(thing);
    texts[3] = g_env->diff180(thing);
    texts[4] = static_cast<u32>(signedPercent(g_env->average(thing)));
    g_env->add(list, nullptr, texts, (u32)(uintptr_t)thing, -1, 0);
    return 1;
}

u32 __cdecl Dialog7ca150_AddRatedHistoryRow(u8 *thing) {
    u8 *const list = listOfDialog();
    u32 texts[6];
    nameAndSum(texts, thing, thing);
    texts[2] = static_cast<u32>(percent(g_env->rating(thing, nullptr, 0)));
    texts[3] = g_env->difference(thing);
    texts[4] = g_env->diff180(thing);
    texts[5] = static_cast<u32>(signedPercent(g_env->average(thing)));
    g_env->add(list, nullptr, texts, (u32)(uintptr_t)thing, -1, 0);
    return 1;
}

u32 __cdecl Dialog7ca150_AddControlRow(u8 *thing) {
    u8 *const list = listOfDialog();
    g_env->def(thing);
    u32 texts[2];
    texts[0] = (u32)(uintptr_t)Thing_NameBuffer7ba058(thing);
    GameTable7cfb90_Get1da720_519510(global<u8 *>(0x007cf83c));   // (its answer only the ECX of the stdcall)
    texts[1] = g_env->control(at<u16>(thing, 0xe));
    g_env->add(list, nullptr, texts, (u32)(uintptr_t)thing, -1, 0);
    return 1;
}
