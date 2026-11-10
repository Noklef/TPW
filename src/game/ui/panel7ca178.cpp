#include "panel7ca178.h"

#include <cstdint>

#include "../core/vcall.h"
#include "../math/x87.h"
#include "../world/advisor.h"
#include "../world/object.h"
#include "../world/objectmsg.h"
#include "../world/objectstatus.h"
#include "dialogs.h"
#include "panelparts.h"
#include "window.h"
#include "window705a50.h"
#include "panel7cc218.h"

using namespace panelparts;

namespace {
u8 *window(u8 *self) { return *reinterpret_cast<u8 **>(self + 0x14); }
u8 *root(u8 *self) { return *reinterpret_cast<u8 **>(self + 0x10); }
template <typename T> T at(const u8 *p, u32 offset) { return *reinterpret_cast<const T *>(p + offset); }

const Panel7ca178StatsEnv kStatsGameEnv = {Object_Status, Object_CanOpen, Object_ScrapRefund, Window705a50_SetState};
const Panel7ca178StatsEnv *g_stats = &kStatsGameEnv;

// The newest 30 of the thing's ring (the data +0x1ac: at most the head + 1 unless wrapped), from the double [0x6ff738] (0.0).
double ringSum(const u8 *thing) {
    const bool wrapped = at<u8>(thing, 0x22c) != 0;
    s32 count = wrapped ? at<s32>(thing, 0x228) : at<s32>(thing, 0x224) + 1;
    if (count >= 0x1e) count = 0x1e;
    double sum = gPanel7ca178_6ff738;
    for (s32 i = 0; i < count; ++i) {
        const s32 used = at<s32>(thing, 0x228), head = at<s32>(thing, 0x224);
        if (i >= used) continue;
        if (i <= head) sum = X87_Add((double)(long long)at<u32>(thing, (u32)(head - i) * 4 + 0x1ac), sum);
        else if (wrapped) sum = X87_Add((double)(long long)at<u32>(thing, (u32)(head - i + used) * 4 + 0x1ac), sum);
    }
    return sum;
}
}  // namespace

void Panel7ca178_SetStatsEnv(const Panel7ca178StatsEnv *env) { g_stats = env ? env : &kStatsGameEnv; }

extern "C" {

void __fastcall Panel7ca178_ShowStats(u8 *self) {
    u8 *const thing = gPanel7cc218_7c2658;
    Object_GetDef(thing);
    if (window(self)) {
        if (u8 *const part = Window_FindChild(window(self), nullptr, 0x15bbd))
            vcall<void>(part, 0x1c, (u32)(long long)ringSum(thing));
        if (u8 *const bar = Window_FindChild(window(self), nullptr, 0x15bbb)) {
            const u32 value = (u8)(u32)(long long)at<float>(thing, 0x44);   // (__ftol's low byte)
            vcall<void>(bar, 0x1c, (u32)((s32)(value << 10) / 100));
        }
        u8 *const refund = Window_FindChild(window(self), nullptr, 0x15bc0);   // (not checked)
        vcall<void>(refund, 0x1c, g_stats->refund(thing));
    }
    u8 *const open = Window_FindChild(root(self), nullptr, 0x15bc4);
    if (!g_stats->canOpen(thing)) {
        vcall<void>(open, 0x14, 0u);
    } else {
        vcall<void>(open, 0x14, 1u);
        g_stats->setState(open, nullptr, Is68_Zero_4e0440(thing) ? 1 : 0, 1);
    }
    u8 *const state = Window_FindChild(root(self), nullptr, 0x15bb4);
    const u32 status = g_stats->status(thing, 2);
    g_stats->setState(state, nullptr, status >= 0xb && status <= 0xf ? 1 : 0, 1);
    if (status != at<u32>(self, 8)) {
        *reinterpret_cast<u32 *>(self + 8) = status;
        vcall<void>(self, 0x24);
    }
}

void __fastcall Panel7ca178_Dress(u8 *self) {
    if (!window(self)) return;
    text(window(self), 0x15bbe, 0x3c);
    text(window(self), 0x15bbc, 0x3d);
    text(window(self), 0x15bbf, 0x3f);
    part(window(self), TextPart6ff350_Ctor, 0x15bbd, 0);
    bar(window(self), kRideStatBar, 0x15bbb, 0x32);
    part(window(self), TextPart6ff3b0_Ctor, 0x15bc0, 0);
}

void __fastcall Panel7ca178_DressTip(u8 *self) { panelparts::tip(self); }

void __fastcall Panel7ca178_Show(u8 *self, void *, u8 *thing) {
    *reinterpret_cast<u8 **>((uintptr_t)0x007ca19c) = thing;
    WindowAt10_Create(self, nullptr, reinterpret_cast<u8 *>((uintptr_t)0x00751190), 0x00497640, 0x15bb8, 0x15bba, 0x15bb9);
    const u8 value = (u8)(u32)(long long)*reinterpret_cast<const float *>(thing + 0x44);   // (__ftol's low byte)
    Advisor_PostId(value < 0x19 ? 0xd5 : 0x12b);
}
}
