#include "statsgraph.h"

#include <cstdint>

#include "../core/circbuf.h"
#include "../ui/dialogs.h"
#include "../ui/guirect.h"
#include "../ui/window.h"
#include "../world/world.h"
#include "gauge.h"
#include "../ui/dialog7cb398.h"
#include "../ui/dialog7ca2e8.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

constexpr u32 kLines[] = {0x1f100, 0x1f350, 0x1fc90, 0x1fee0, 0x20130, 0x20380, 0x1f5a0, 0x1f7f0};

using Scan = void(__cdecl *)(const u8 *, s32 *, u32);

s32 shown(Scan scan) {
    s32 result = 0;
    const u8 *const record = reinterpret_cast<const u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(global<u8 *>(0x007cf83c)));
    for (u32 k = 0; k < 8; ++k)
        if (reinterpret_cast<u8 &>(gDialog7ca2e8_751fa0) & (1u << k)) scan(record + kLines[k], &result, global<u32>(0x007ca2ec));
    return result;
}

const StatsGraphEnv kGameEnv = {&StatsGraph_Lines, &StatsGraph7cb318_Lines, &Window7ca2e8_ClearButtons,
                                reinterpret_cast<void(__fastcall *)(u8 *, void *, u32 *, s32, u32, u32, u32)>(&Gauge_DrawPlot)};
const StatsGraphEnv *g_env = &kGameEnv;

void SetMarksXOfKind(u8 *gauge, u32 kind) {
    if (kind == 0) Gauge_SetMarksX(gauge, nullptr, 0, 0xb, 0xc);
    else if (kind == 1) Gauge_SetMarksX(gauge, nullptr, 0, 0x23, 0x24);
    else if (kind == 2) Gauge_SetMarksX(gauge, nullptr, 0, 0x8f, 0x90);
}

// A message 0x101's id's kind: -1 for another id.
s32 KindOfRadio(u32 index) { return index == 0 ? 2 : index == 1 ? 0 : index == 2 ? 1 : -1; }

u8 *Graph1() { return reinterpret_cast<u8 *>((uintptr_t)0x007ca250); }
u8 *Graph2() { return reinterpret_cast<u8 *>((uintptr_t)0x007cb318); }

void SetMarksY1() {
    const s32 high = StatsGraph_ShownMax();
    Gauge_SetMarksY(Graph1(), nullptr, (u32)StatsGraph_ShownMin(), (u32)high, 10);
}

u32 plotWindow(u32 kind) { return kind == 0 ? 0xc : kind == 1 ? 0x24 : kind == 2 ? 0x90 : 0; }

// The samples into the points; sample(i) gives the point of the sample i, first the one before any.
template <typename Sample> void plot(u8 *gauge, const u8 *line, const u8 *colour, s32 step, u32 kind, u32 first, Sample sample) {
    const s32 head = *reinterpret_cast<const s32 *>(line + 0x244), capacity = *reinterpret_cast<const s32 *>(line + 0x248);
    const bool wrapped = line[0x24c] != 0;
    const s32 window = (s32)plotWindow(kind), count = wrapped ? capacity : head + 1, last = window < count ? window : count;
    if (last < 0) return;
    u32 *const points = reinterpret_cast<u32 *>((uintptr_t)0x007b4bf8);
    u32 value = first;
    s32 back = last * step;
    for (s32 i = 0; i <= last; ++i, back -= step) {
        if (back < capacity) {
            if (back <= head) value = sample(head - back);
            else if (wrapped) value = sample(head + capacity - back);
        }
        points[i] = value;
    }
    g_env->plot(gauge, nullptr, points, last, plotWindow(kind), *reinterpret_cast<const u32 *>(colour), colour[4]);
}

}  // namespace

extern "C" {

void __cdecl StatsGraph_PlotU32(u8 *gauge, const u8 *line, const u8 *colour, s32 step, u32 kind) {
    plot(gauge, line, colour, step, kind, 0, [line](s32 i) { return *reinterpret_cast<const u32 *>(line + 4 + i * 4); });
}

void __cdecl StatsGraph_PlotFloat(u8 *gauge, const u8 *line, const u8 *colour, s32 step, u32 kind) {
    const auto truncated = [](float x) { return (u32)(long long)x; };
    plot(gauge, line, colour, step, kind, truncated(global<float>(0x006ff0a4)), [line, truncated](s32 i) { return truncated(*reinterpret_cast<const float *>(line + 4 + i * 4)); });
}

void __cdecl StatsGraph_Lines(u8 *gauge) {
    static const u32 kColours[8] = {0x007ca2b0, 0x007ca2c0, 0x007ca2c8, 0x007ca2d8, 0x007ca2a8, 0x007ca2e0, 0x007ca298, 0x007ca2b8};
    const u8 *const record = reinterpret_cast<const u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(global<u8 *>(0x007cf83c)));
    for (u32 k = 0; k < 8; ++k)
        if (reinterpret_cast<u8 &>(gDialog7ca2e8_751fa0) & (1u << k)) StatsGraph_PlotU32(gauge, record + kLines[k], reinterpret_cast<const u8 *>((uintptr_t)kColours[k]), 1, global<u32>(0x007ca2ec));
}

void __cdecl StatsGraph7cb318_Lines(u8 *gauge) {
    static const u32 kOffsets[5] = {0x20cc0, 0x20f10, 0x21160, 0x205d0, 0x213b0}, kColours[5] = {0x007cb368, 0x007cb380, 0x007cb308, 0x007cb390, 0x007cb370};
    const u8 *const record = reinterpret_cast<const u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(global<u8 *>(0x007cf83c)));
    for (u32 k = 0; k < 5; ++k)
        if (reinterpret_cast<u8 &>(gDialog7cb398_753c48) & (1u << k)) (k == 1 ? StatsGraph_PlotFloat : StatsGraph_PlotU32)(gauge, record + kOffsets[k], reinterpret_cast<const u8 *>((uintptr_t)kColours[k]), 1, global<u32>(0x007cb39c));
}
}

void StatsGraph_SetEnv(const StatsGraphEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __cdecl StatsGraph_ShownMax() { return shown(&CircBufU32_144_MaxInWindow); }

s32 __cdecl StatsGraph_ShownMin() { return shown(&CircBufU32_144_MinInWindow); }

s32 __cdecl StatsGraph7cb318_ShownMax() {
    constexpr u32 kOffsets[] = {0x20cc0, 0x20f10, 0x21160, 0x205d0, 0x213b0};
    s32 result = 0;
    const u8 *const record = reinterpret_cast<const u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(global<u8 *>(0x007cf83c)));
    for (u32 k = 0; k < 5; ++k)
        if (reinterpret_cast<u8 &>(gDialog7cb398_753c48) & (1u << k)) (k == 1 ? &CircBufF32_144_MaxInWindow : &CircBufU32_144_MaxInWindow)(record + kOffsets[k], &result, global<u32>(0x007cb39c));
    return result;
}

void __cdecl StatsGraph_Redraw() {
    u8 *const dialog = global<u8 *>(0x007ca2e8);
    if (!dialog) return;
    u8 *const gauge = reinterpret_cast<u8 *>((uintptr_t)0x007ca250);
    const s32 high = StatsGraph_ShownMax();
    Gauge_SetMarksY(gauge, nullptr, (u32)StatsGraph_ShownMin(), (u32)high, 10);
    Gauge_DrawInWindow(Window_FindChild(dialog, nullptr, 0x12272), gauge, g_env->lines49aa30);
}

void __cdecl StatsGraph7cb318_Redraw() {
    u8 *const dialog = global<u8 *>(0x007cb398);
    if (!dialog) return;
    u8 *const gauge = reinterpret_cast<u8 *>((uintptr_t)0x007cb318);
    Gauge_SetMarksY(gauge, nullptr, 0, (u32)StatsGraph7cb318_ShownMax(), 10);
    Gauge_DrawInWindow(Window_FindChild(dialog, nullptr, 0x4a68bb), gauge, g_env->lines4a4750);
}

u32 __cdecl StatsGraph_Handler(u8 *window, u32 message, u32 a, u32 b) {
    u8 rect[12];
    GuiRect_Ctor(rect);
    bool draw = false;
    if (message == 0x15) {
        Window_GetSavedRect(window, nullptr, rect);
        const s32 high = StatsGraph_ShownMax();
        Gauge_Setup(Graph1(), rect, (u32)StatsGraph_ShownMin(), (u32)high, 10, 0, 0xb, 0xc);
        draw = true;
    } else if (message == 0x100) {
        if (a == 0xfffffffe) {
            Window_SendMessage(global<u8 *>(0x007ca2e8), 4, 0, 0);
            GuiRect_Dtor(rect);
            return 0;
        }
        static const u8 kBits[] = {1, 8, 4, 0x40, 2, 0x10, 0x20, 0x80};
        const u32 index = a - 0x12277;
        if (index < 8) {
            u32 &flags = gDialog7ca2e8_751fa0;
            const u8 bit = kBits[index];
            if (b == 1) {
                const u8 other = bit <= 2 ? 0xfc : 3;
                if (flags & other) {
                    g_env->clearButtons(other);
                    flags &= ~(u32)other;
                }
                flags |= bit;
            } else {
                flags &= ~(u32)bit;
            }
            SetMarksY1();
            draw = true;
        }
    } else if (message == 0x101) {
        const s32 kind = KindOfRadio(a - 0x12274);
        if (kind >= 0) global<u32>(0x007ca2ec) = (u32)kind;
        if (kind >= 0 && b == 1) {
            SetMarksXOfKind(Graph1(), (u32)kind);
            SetMarksY1();
            draw = true;
        }
    }
    if (draw) Gauge_DrawInWindow(window, Graph1(), g_env->lines49aa30);
    const u32 answer = (u32)Window_CallDefault(window, nullptr, message, a, b);
    GuiRect_Dtor(rect);
    return answer;
}

u32 __cdecl StatsGraph7cb318_Handler(u8 *window, u32 message, u32 a, u32 b) {
    u8 rect[12];
    GuiRect_Ctor(rect);
    bool draw = false;
    if (message == 0x15) {
        Window_GetSavedRect(window, nullptr, rect);
        Gauge_Setup(Graph2(), rect, 0, (u32)StatsGraph7cb318_ShownMax(), 10, 0, 0xb, 0xc);
        SetMarksXOfKind(Graph2(), global<u32>(0x007cb39c));
        draw = true;
    } else if (message == 0x100) {
        const u32 index = a - 0x4a68bc;
        if (index < 5) {
            u32 &flags = gDialog7cb398_753c48;
            if (b == 1) flags |= 1u << index;
            else flags &= ~(1u << index);
            Gauge_SetMarksY(Graph2(), nullptr, 0, (u32)StatsGraph7cb318_ShownMax(), 10);
            draw = true;
        }
    } else if (message == 0x101) {
        const s32 kind = KindOfRadio(a - 0x4a68c2);
        if (kind >= 0) global<u32>(0x007cb39c) = (u32)kind;
        if (kind >= 0 && b == 1) {
            SetMarksXOfKind(Graph2(), (u32)kind);
            Gauge_SetMarksY(Graph2(), nullptr, 0, (u32)StatsGraph7cb318_ShownMax(), 10);
            draw = true;
        }
    }
    if (draw) Gauge_DrawInWindow(window, Graph2(), g_env->lines4a4750);
    const u32 answer = (u32)Window_CallDefault(window, nullptr, message, a, b);
    GuiRect_Dtor(rect);
    return answer;
}
}
