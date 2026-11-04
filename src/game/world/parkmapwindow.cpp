// See parkmapwindow.h.
#include "parkmapwindow.h"

#include <cmath>
#include <cstdint>

#include "../core/accessors.h"
#include "../core/string.h"
#include "../core/vcall.h"
#include "../files/charsets.h"
#include "../math/x87.h"
#include "../render/backdrop.h"
#include "../ui/thingpanel.h"
#include "../ui/window.h"
#include "parkmap.h"
#include "signtext.h"
#include "staff.h"

namespace {

template <typename T>
T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(p) + offset);
}
template <typename T>
T &global(u32 address) {
    return *reinterpret_cast<T *>((uintptr_t)address);
}

const ParkMapWindowEnv kPorts = {&ParkMap_Scroll, &ParkMap_LoadPicture, &ParkMap_Draw, &Backdrop_UncacheRect, &Staff_GetName, &Charset_FormatWide,
                                 &SignText_Format, &Window_FindChild, &ParkMap_ApplyView, &ThingPanel_Show, &Window_CallDefault};
const ParkMapWindowEnv *g_env = &kPorts;

const u32 kBackdrop = 0x0087aee8;
const u32 kThings = 0x007cfb90;  // (20 bytes a slot)
const u32 kText = 0x007ba058;

s32 Ftol(double x) { return (s32)(long long)x; }

bool IsPerson(u8 kind) { return kind == 4 || kind == 5 || kind == 6 || kind == 8 || kind == 7; }

bool ItemIsPerson(const u8 *item) {
    if (!item) return false;
    const u8 *thing = at<u8 *>(item, 0x18);
    return thing && IsPerson(at<u8>(thing, 2));
}

// The name of what is under the mouse, in the child 0x991 of the window's parent.
void ShowName(const u8 *map, u8 *window) {
    u16 *const text = reinterpret_cast<u16 *>((uintptr_t)kText);
    WString name;
    WString_CtorStr(&name, nullptr, reinterpret_cast<const u16 *>((uintptr_t)0x0078a374));
    const u8 *item = at<u8 *>(map, 0x20060);
    u8 *thing = item ? at<u8 *>(item, 0x18) : nullptr;
    if (thing) {
        const u8 kind = at<u8>(thing, 2);
        if (IsPerson(kind)) {
            vcall<void>(&name, 0x14, g_env->staffName(thing));
        } else if (kind == 1) {
            u8 *const charset = global<u8 *>(0x0078ba94);
            const u32 owner = at<u32>(thing, 0x1d8);
            if (!owner) {
                g_env->formatWide(charset, text, 0x3fff, 0x1c9);
            } else {
                const u32 argument[5] = {0x006ff170, 0x5641524d, 0x18, owner, 0};
                g_env->formatWide(charset, text, 0x3fff, 0x1c8, argument);
            }
            vcall<void>(&name, 0x14, text);
        }
    } else {
        const u16 id = at<u16>(map, 0x20064);
        if (id) {
            g_env->signFormat(global<u8 *>(kThings + id * 20u), nullptr, text, 0x3fff);
            vcall<void>(&name, 0x14, text);
        }
    }
    u8 *const child = g_env->findChild(reinterpret_cast<u8 *>((uintptr_t)Fn_65d55a(window)), nullptr, 0x991);
    vcall<void>(child, 0x24, name.data);
    WString_Dtor(&name);
}

void MouseMoved(u8 *map, u8 *window, u32 a) {
    const u32 width = global<u32>(0x007a1968), height = global<u32>(0x007a1930);
    const u8 *list = at<u8 *>(map, 0x20080);
    const float widthF = (float)(double)width, heightF = (float)(double)height;
    const u32 mouseX = a >> 16, mouseY = a & 0xffff;
    at<u32>(map, 0x10) = mouseX;
    u8 *const previous = at<u8 *>(map, 0x20060);
    at<u32>(map, 0x20060) = 0;
    at<u32>(map, 0x14) = mouseY;
    const float x = (float)(X87Reg((double)(s32)mouseX) * widthF * global<float>(0x00703014));
    const float y = (float)(X87Reg((double)(s32)(0x600 - mouseY)) * heightF * global<float>(0x00703018));
    float best = 100000000.0f;
    const float reach = global<float>(0x00703040);
    for (const u8 *node = at<u8 *>(list, 4); node;) {
        u8 *const item = at<u8 *>(node, 0);
        node = at<u8 *>(node, 4);
        if (!at<u32>(item, 0x2c)) continue;
        const X87Reg dx = X87Reg(x) - at<float>(item, 0xc);
        const X87Reg dy = X87Reg(y) - at<float>(item, 0x10);
        const double absX = std::fabs(dx.v);
        const float fx = (float)absX, fy = (float)std::fabs(dy.v);
        if (absX >= reach || fy >= reach) continue;  // (a NaN goes on, sic)
        const X87Reg distance = X87_Sqrt(X87Reg(fx) * fx + X87Reg(fy) * fy);
        if (!(distance.v >= best)) {
            best = (float)distance;
            at<u8 *>(map, 0x20060) = item;
        }
    }
    const u8 *const now = at<u8 *>(map, 0x20060);
    if (previous != now && (ItemIsPerson(previous) || ItemIsPerson(now))) {
        g_env->loadPicture(map, nullptr, 1);
        g_env->draw(map);
        g_env->uncacheRect(reinterpret_cast<u8 *>((uintptr_t)kBackdrop), nullptr, 0, 0, at<s32>(map, 0x48) - 1, at<s32>(map, 0x4c) - 1);
    }
    const float zoom = at<float>(map, 0x5c);
    const float half = global<float>(0x00703030);
    const X87Reg toY = X87_DivR(static_cast<double>(at<s32>(map, 0x28)), global<float>(0x00703048));
    const X87Reg toX = X87_DivR(static_cast<double>(at<s32>(map, 0x24)), global<float>(0x00703044));
    const X87Reg halfH = X87Reg(heightF) * half, halfW = X87Reg(widthF) * half;
    const X87Reg squareY = (X87Reg(y) - halfH) / (toY * zoom) + at<float>(map, 0x58);
    const X87Reg squareX = (X87Reg(x) - halfW) / (toX * zoom) + at<float>(map, 0x54);
    const s32 row = Ftol(squareY.v), column = Ftol(squareX.v);
    if (column >= 0 && column < 0x80 && row >= 0 && row < 0x80) {
        at<u16>(map, 0x20064) = at<u16>(map, ((u32)(row << 7) + column) * 8 + 0x64);
    } else {
        at<u16>(map, 0x20064) = 0;
    }
    ShowName(map, window);
}

void MouseClicked(u8 *map) {
    const u32 width = global<u32>(0x007a1968), height = global<u32>(0x007a1930);
    const float zoom = at<float>(map, 0x5c);
    const float half = global<float>(0x00703030);
    const X87Reg toX = X87_DivR(static_cast<double>(at<s32>(map, 0x24)), global<float>(0x00703044));
    const X87Reg toY = X87_DivR(static_cast<double>(at<s32>(map, 0x28)), global<float>(0x00703048));
    const float heightF = (float)(double)height;
    const X87Reg wide = X87Reg((double)at<s32>(map, 0x10)) * X87Reg((double)width);
    const X87Reg tall = X87Reg((double)(s32)(0x600 - at<u32>(map, 0x14))) * heightF;
    const float x = (float)(wide * global<float>(0x00703014));
    const float y = (float)(tall * global<float>(0x00703018));
    const X87Reg halfW = X87Reg((double)width) * half, halfH = X87Reg(heightF) * half;
    const X87Reg moveX = (X87Reg(x) - halfW) / (toX * zoom);
    const X87Reg moveY = (X87Reg(y) - halfH) / (toY * zoom);
    const X87Reg newX = moveX + at<float>(map, 0x54);
    const X87Reg newY = moveY + at<float>(map, 0x58);
    at<float>(map, 0x54) = (float)newX;
    at<float>(map, 0x58) = (float)newY;
    const u8 *item = at<u8 *>(map, 0x20060);
    u8 *thing = item ? at<u8 *>(item, 0x18) : nullptr;
    if (!thing) {
        const u16 id = at<u16>(map, 0x20064);
        if (id) thing = global<u8 *>(kThings + id * 20u);
    }
    g_env->applyView(map);
    if (thing) g_env->thingPanel(thing);
}

}  // namespace

void ParkMapWindow_SetEnv(const ParkMapWindowEnv *env) { g_env = env ? env : &kPorts; }

extern "C" {

s32 __fastcall ParkMap_HandleMouse(u8 *map, void *, u8 *window, u32 message, u32 a, u32 b) {
    switch (message) {
        case 0x1e:
            if (global<u8>(0x0078d910) && at<u32>(map, 0x20084)) {
                g_env->scroll(map, nullptr, 1, (float)(X87Reg((double)(s32)(at<u32>(map, 0x10) - 0x400)) * global<float>(0x00703034)));
                g_env->scroll(map, nullptr, 3, (float)(X87Reg((double)(s32)(at<u32>(map, 0x14) - 0x300)) * global<float>(0x00703038)));
            }
            break;
        case 0x10001:
        case 0x10003:
            MouseMoved(map, window, a);
            break;
        case 0x10004:
            if (a == 1) at<u32>(map, 0x20084) = 0;
            break;
        case 0x10005:
            if (a == 1) at<u32>(map, 0x20084) = a;
            break;
        case 0x10006:
            if (!a) {
                MouseClicked(map);
                return 0;
            }
            break;
        default:
            break;
    }
    return g_env->callDefault(window, nullptr, message, a, b);
}

}  // extern "C"
