// See parkmapwindow.h.
#include "parkmapwindow.h"

#include <cmath>
#include <cstdint>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/gameowner.h"
#include "../core/language.h"
#include "../core/stdlist.h"
#include "../core/string.h"
#include "../core/vcall.h"
#include "../files/charsets.h"
#include "../frontend/helpbg.h"
#include "../gfx/surface.h"
#include "../gfx/texcache.h"
#include "../math/x87.h"
#include "../render/backdrop.h"
#include "../render/renderer.h"
#include "../render/textparts.h"
#include "../terrain/land.h"
#include "../ui/focus.h"
#include "../ui/radiogroup.h"
#include "../ui/thingpanel.h"
#include "../ui/window.h"
#include "../ui/window705a50.h"
#include "../ui/windowscript.h"
#include "advisor.h"
#include "parkmap.h"
#include "signtext.h"
#include "staff.h"
#include "../frontend/lobbysky.h"
#include "../ui/panel7b9940.h"

namespace {

template <typename T>
T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(p) + offset);
}
template <typename T>
T &global(u32 address) {
    return *reinterpret_cast<T *>((uintptr_t)address);
}

// A function of the game by its address.
struct Game {
    u32 address;
    template <typename F>
    operator F() const {
        return reinterpret_cast<F>((uintptr_t)address);
    }
};

const ParkMapWindowEnv kPorts = {&ParkMap_Scroll, &ParkMap_LoadPicture, &ParkMap_Draw, &Backdrop_UncacheRect, &Staff_GetName, &Charset_FormatWide,
                                 &SignText_Format, &Window_FindChild, &ParkMap_ApplyView, &ThingPanel_Show, &Window_CallDefault, &ParkMap_LayOut,
                                 Game{0x0056b450}, Game{0x0056b340}, Game{0x0056b210}, &SurfaceInfo_Free, Game{0x005763f0}, Game{0x00409300},
                                 Game{0x00662420}, Game{0x00658d9f}, Game{0x00486b40}, Game{0x00485b70}, Game{0x004862a0}, Game{0x00492530}};
const ParkMapWindowEnv *g_env = &kPorts;

const u32 kBackdrop = 0x0087aee8;
const u32 kThings = 0x007cfb90;  // (20 bytes a slot)
const u32 kText = 0x007ba058;

s32 Ftol(double x) { return (s32)(long long)x; }

// The picture reloaded with the filters as they are, redrawn and uncached whole.
void Reload(u8 *map) {
    g_env->loadPicture(map, nullptr, 1);
    g_env->draw(map);
    g_env->uncacheRect(reinterpret_cast<u8 *>((uintptr_t)kBackdrop), nullptr, 0, 0, at<s32>(map, 0x48) - 1, at<s32>(map, 0x4c) - 1);
}

void Flip(u8 *map, u32 bit) {
    at<u32>(map, 0x18) ^= bit;
    Reload(map);
}

void Scroll(u8 *map, u32 direction) { g_env->scroll(map, nullptr, direction, 1.0f); }

void Zoom(u8 *map, float step) {
    const X87Reg z = X87Reg(at<float>(map, 0x5c)) - step;
    const float low = global<float>(0x00703004), high = global<float>(0x0070302c);
    at<float>(map, 0x5c) = !(z.v >= low) ? low : z.v > high ? high : (float)z;
    g_env->layOut(map);
}

void Close(u8 *map) {
    while (at<u32>(at<u8 *>(map, 0x20080), 0xc)) {
        u8 *const object = List_PopHeadObject(at<u8 *>(map, 0x20080));
        if (object) vcall<void>(object, 0, 1u);
    }
    u8 *const list = at<u8 *>(map, 0x20080);
    if (list) vcall<void>(list, 0, 1u);
    at<u32>(map, 0x20080) = 0;
    u8 *const images = reinterpret_cast<u8 *>((uintptr_t)0x00879ce0);
    for (u32 i = 0; i < 6; ++i) g_env->removeImage(images, nullptr, at<u32>(map, 0x20068 + 4 * i));
    g_env->release(images);
    g_env->releaseAll(images);
    g_env->surfaceFree(reinterpret_cast<SurfaceInfo *>(map + 0x2c));
    gLobbysky_8bd47c = at<u32>(map, 8);
    g_env->setFlags(global<u8 *>(0x0087a6d0), nullptr, at<u32>(map, 0xc));
    global<u32>(0x00786ba4) = 1;
    g_env->restore(reinterpret_cast<u8 *>((uintptr_t)0x00786b68));
    g_env->restartTimers();
    at<u32>(map, 4) = 0;
    g_env->send(gPanel7b9940_7cb2ac, 6, 1, 0);
    g_env->cancelAdvisor(0x130);
    g_env->focusSet(nullptr);
    g_env->focusRestore();
    g_env->helpPlace(-1, -1);
}

void EdgeScroll(u8 *map) {
    const float low = global<float>(0x00703010), one = global<float>(0x00703004);
    const float nearEdge = global<float>(0x0070301c), farEdge = global<float>(0x00703024), rate = global<float>(0x00703020);
    X87Reg x = X87Reg((double)at<s32>(map, 0x10)) * global<float>(0x00703014);
    if (x.v < low) x = low;
    else if (x.v > one) x = one;
    float y = (float)(X87Reg((double)at<s32>(map, 0x14)) * global<float>(0x00703018));
    if (y < low) y = 0.0f;
    else if (y > one) y = 1.0f;
    if (x.v < nearEdge) {
        g_env->scroll(map, nullptr, 0, (float)((X87Reg(nearEdge) - x) * rate));
    } else if (x.v > farEdge) {
        g_env->scroll(map, nullptr, 1, (float)((x - farEdge) * rate));
    }
    if (y < nearEdge) {
        g_env->scroll(map, nullptr, 2, (float)((X87Reg(nearEdge) - y) * rate));
    } else if (y > farEdge) {
        g_env->scroll(map, nullptr, 3, (float)((X87Reg(y) - farEdge) * rate));
    }
}

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
    if (previous != now && (ItemIsPerson(previous) || ItemIsPerson(now))) Reload(map);
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

namespace {
const ParkMapSpritesEnv kSpritePorts = {Game{0x00516130}, Game{0x00516160}, Game{0x0056b0c0}, Game{0x0056b2f0}, Game{0x0056b380}, Game{0x005f3940}};
const ParkMapSpritesEnv *g_sprites = &kSpritePorts;
}  // namespace

void ParkMapSprites_SetEnv(const ParkMapSpritesEnv *env) { g_sprites = env ? env : &kSpritePorts; }

namespace {
const ParkMapOpenEnv kOpenPorts = {&Global7c2534_HasSource, &GameOwner_Pause, &Fn_662411, Game{0x005996d0}, &Renderer_StateFlags, &Renderer_SetStateFlags, &TexCache_ReleaseItems,
                                   &Window7c24c8_Close, &Window_SendMessage, &WindowScript_Build, &Window7c24c8_Set, &Window_SetFocus, &Window_FindChild, &TextPart_Ctor, &Window_SetImpl,
                                   &Window_NotifyC00, &Window_Notify3000, &Language_Font, &Window_Set4c, &Window_Notify40000, &Window_NotifyImpl100, &Window_SetDepth,
                                   &Window705a50_SetState, &RadioGroup_Select, &Window705a50_SetPair, &Window_SetHandler, &Terrain_GridSize, &ParkMap_Classify,
                                   &ParkMap_PrepareSprites, &ParkMap_LoadPicture, &Backdrop_Create, &Backdrop_Clear5c, &ParkMap_Draw, &Backdrop_UncacheRect,
                                   &Advisor_PostId, &HelpBg_Place, &ParkMap_HandleWindow, &ParkMap_HandleMouse};
const ParkMapOpenEnv *g_open = &kOpenPorts;

const u32 kOpenMap = 0x00f86db8;

// One coordinate of the origin as ParkMap_Open keeps it: below the floor [0x703010] the floor, past limit (compared exactly) the limit rounded to a float.
float open_origin(float v, s32 limit) {
    const float floor = global<float>(0x00703010);
    if (v < floor) return X87_Through(floor);
    return (double)v <= (double)limit ? v : (float)limit;
}
}  // namespace

void ParkMapOpen_SetEnv(const ParkMapOpenEnv *env) { g_open = env ? env : &kOpenPorts; }

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

s32 __fastcall ParkMap_HandleWindow(u8 *map, void *, u8 *window, u32 message, u32 a, u32 b) {
    switch (message) {
        case 0x14:
            Close(map);
            break;
        case 0x15:
            g_env->layOut(map);
            break;
        case 0x1e:
            if (!global<u8>(0x0078d910)) EdgeScroll(map);
            break;
        case 0x100:
            switch (a) {
                case 0xffffffff:
                    g_env->applyView(map);
                    return 0;
                case 0x982:
                    Flip(map, 8);
                    break;
                case 0x983:
                    Flip(map, 0x20);
                    break;
                case 0x985:
                    Zoom(map, global<float>(0x00703030));
                    break;
                case 0x986:
                    Zoom(map, global<float>(0x00703028));
                    break;
                case 0x987:
                    Flip(map, 1);
                    break;
                case 0x989:
                    Flip(map, 0x10);
                    break;
                case 0x993:
                    Flip(map, 2);
                    break;
                case 0x994:
                    Flip(map, 4);
                    break;
                default:
                    break;
            }
            break;
        case 0x101: {
            const bool pressed = b == 1;
            switch (a) {
                case 0x98c:
                    at<u32>(map, 0x1c) = pressed ? 3 : 0;
                    Reload(map);
                    break;
                case 0x98d:
                    at<u32>(map, 0x1c) = pressed ? 4 : 0;
                    Reload(map);
                    break;
                case 0x98e:
                    at<u32>(map, 0x1c) = pressed ? 1 : 0;
                    Reload(map);
                    break;
                case 0x98f:
                    at<u32>(map, 0x1c) = pressed ? 2 : 0;
                    Reload(map);
                    break;
                case 0x997:
                    at<u32>(map, 0x20) = pressed ? 2 : 0;
                    Reload(map);
                    break;
                case 0x998:
                    at<u32>(map, 0x20) = pressed ? 1 : 0;
                    Reload(map);
                    break;
                case 0x999:
                    at<u32>(map, 0x20) = pressed ? 3 : 0;
                    Reload(map);
                    break;
                default:
                    break;
            }
            break;
        }
        case 0x10001:
        case 0x10003:
            at<u32>(map, 0x10) = a >> 16;
            at<u32>(map, 0x14) = a & 0xffff;
            break;
        case 0x10008:
        case 0x10009:
            switch (a) {
                case 0x981:
                    Scroll(map, 3);
                    break;
                case 0x984:
                    Scroll(map, 0);
                    break;
                case 0x988:
                    Scroll(map, 1);
                    break;
                case 0x98a:
                    Scroll(map, 2);
                    break;
                default:
                    break;
            }
            break;
        case 0x1000a:
            switch (a) {
                case 0x2500:
                    Scroll(map, 0);
                    break;
                case 0x2600:
                    Scroll(map, 2);
                    break;
                case 0x2700:
                    Scroll(map, 1);
                    break;
                case 0x2800:
                    Scroll(map, 3);
                    break;
                case 0x6b00:
                    Zoom(map, global<float>(0x00703028));
                    break;
                case 0x6d00:
                    Zoom(map, global<float>(0x00703030));
                    break;
                default:
                    break;
            }
            break;
        case 0x1000b:
            if (a == 0x1b && !(b & 0xff0000)) g_env->send(window, 4, 0, 0);
            return 0;
        default:
            break;
    }
    return g_env->callDefault(window, nullptr, message, a, b);
}

void __fastcall ParkMap_PrepareSprites(u8 *map) {
    u8 *const world = global<u8 *>(0x007cf83c);
    s32 count = 0;
    u16 id;
    g_sprites->firstThing(world, nullptr, &id);
    while (id) {
        u8 *const thing = global<u8 *>(kThings + id * 20u);
        const u8 kind = at<u8>(thing, 2);
        if (kind == 1 || kind == 6 || kind == 7) {
            count += 2;
        } else if (kind == 4 || kind == 5) {
            count += 1;
        }
        u16 next;
        id = *g_sprites->nextThing(world, nullptr, &next, thing);
    }
    const s32 width = at<s32>(map, 0x24), height = at<s32>(map, 0x28);
    for (s32 y = 0; y < height; ++y) {
        for (s32 x = 0; x < width; ++x) {
            const u32 cell = (u32)((y << 7) + 1 + x) & 0xffff;
            if (at<u32>(world, cell * 0x44 + 0x2c0)) ++count;
        }
    }
    for (u16 chain = at<u16>(world, 0x1da746); chain;) {
        const u8 *const thing = global<u8 *>(kThings + chain * 20u);
        if (at<u8>(thing, 0x32) & 0x10) ++count;
        chain = at<u16>(thing, 0xc);
    }
    u8 *const images = reinterpret_cast<u8 *>((uintptr_t)0x00879ce0);
    g_sprites->arrayInit(images, nullptr, count + 6, 0);
    g_sprites->addToRenderer(images);
    for (u32 i = 0; i < 6; ++i) at<u32>(map, 0x20068 + 4 * i) = g_sprites->addImage(images, nullptr, global<u32>(0x00f86d48 + 0x14 * i));
    u32 *const list = static_cast<u32 *>(GameHeap_Alloc(0x10));
    if (list) {
        list[1] = 0;
        list[2] = 0;
        list[3] = 0;
        list[0] = 0x00703080;
    }
    at<u32 *>(map, 0x20080) = list;
    g_sprites->addSprites(map);
}

s32 __cdecl ParkMap_WindowHandler(u8 *window, u32 message, u32 a, u32 b) {
    return g_open->handleWindow(reinterpret_cast<u8 *>((uintptr_t)kOpenMap), nullptr, window, message, a, b);
}

s32 __cdecl ParkMap_MouseHandler(u8 *window, u32 message, u32 a, u32 b) {
    return g_open->handleMouse(reinterpret_cast<u8 *>((uintptr_t)kOpenMap), nullptr, window, message, a, b);
}

void __fastcall ParkMap_Open(u8 *map) {
    const ParkMapOpenEnv *const env = g_open;
    if (env->hasSource() != 0 || at<u32>(map, 4) != 0) return;
    env->pause(reinterpret_cast<u8 *>((uintptr_t)0x00786b68), nullptr, 0, 0);
    global<u32>(0x00786ba4) = 0;
    env->setFlag662411();
    env->f5996d0(1);
    at<u32>(map, 8) = gLobbysky_8bd47c;
    gLobbysky_8bd47c = 0;
    at<u32>(map, 0xc) = env->stateFlags(global<u8 *>(0x0087a6d0));
    env->setFlags(global<u8 *>(0x0087a6d0), nullptr, 0x2821);
    env->releaseItems(global<u8 *>(0x008c4b44), nullptr, 0);
    env->focusClose();
    env->send(gPanel7b9940_7cb2ac, 6, 0, 0);
    u8 *const window = env->build(global<u8 *>(0x00faa5d8), nullptr, reinterpret_cast<u8 *>((uintptr_t)0x00774da0), 0x005f0b00);
    env->focusSet(window);
    at<u8 *>(map, 4) = window;
    env->setFocus(window, nullptr, 0);

    u8 *const label = env->findChild(window, nullptr, 0x991);
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    env->setImpl(label, nullptr, memory ? env->textPart(memory, nullptr, 0) : nullptr);
    env->notifyC00(label, nullptr, 1, 1);
    env->notify3000(label, nullptr, 1, 1);
    env->set4c(label, nullptr, env->font(4));
    env->notify40000(label, nullptr, 0xff, 0xff, 0, 0xff);
    env->notifyImpl100(label, nullptr, 0);
    vcall<void>(env->findChild(window, nullptr, 0x990), 0x14, 0u);
    vcall<void>(env->findChild(window, nullptr, 0x995), 0x14, 0u);

    const u32 depth = (u32)(s32)at<s16>(env->findChild(window, nullptr, 0x990), 0xe0) + 2;
    static const struct { u32 id; u8 bit; } kFilters[] = {{0x982, 8}, {0x987, 1}, {0x989, 0x10}, {0x993, 2}, {0x994, 4}, {0x983, 0x20}};
    for (const auto &filter : kFilters) {
        u8 *const child = env->findChild(window, nullptr, filter.id);
        env->setDepth(child, nullptr, depth);
        if (at<u8>(map, 0x18) & filter.bit) env->setState(child, nullptr, 1, 1);
    }

    u8 *const views = env->findChild(window, nullptr, 0x98b);
    switch (at<u32>(map, 0x1c)) {
    case 1: env->select(views, nullptr, 0x98e); break;
    case 2: env->select(views, nullptr, 0x98f); break;
    case 3: env->select(views, nullptr, 0x98c); break;
    case 4: env->select(views, nullptr, 0x98d); break;
    default: break;
    }
    u8 *const modes = env->findChild(window, nullptr, 0x996);
    switch (at<u32>(map, 0x20)) {
    case 1: env->select(modes, nullptr, 0x998); break;
    case 2: env->select(modes, nullptr, 0x997); break;
    case 3: env->select(modes, nullptr, 0x999); break;
    default: break;
    }
    static const u32 kPairs[] = {0x98a, 0x984, 0x981, 0x988};
    for (const u32 id : kPairs) env->setPair(env->findChild(window, nullptr, id), nullptr, 0, 10);
    env->setHandler(env->findChild(window, nullptr, 0x992), nullptr, 0x005f0b20);

    at<u32>(map, 0x20060) = 0;
    at<u16>(map, 0x20064) = 0;
    at<u32>(map, 0x20084) = 0;
    env->gridSize(&at<u32>(map, 0x24), &at<u32>(map, 0x28));

    const float scale = global<float>(0x0070300c);
    s32 x = Ftol((X87Reg(global<float>(0x00790ab8)) * scale).v);  // __ftol
    s32 y = Ftol((X87Reg(global<float>(0x00790ac0)) * scale).v);
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= global<s32>(0x007a0314)) x = global<s32>(0x007a0314) - 1;
    if (y >= global<s32>(0x007a0374)) y = global<s32>(0x007a0374) - 1;
    at<float>(map, 0x54) = open_origin((float)x, at<s32>(map, 0x24));
    at<float>(map, 0x58) = open_origin((float)y, at<s32>(map, 0x28));

    env->classify(map);
    env->prepare(map);
    env->loadPicture(map, nullptr, 0);
    u8 *const backdrop = reinterpret_cast<u8 *>((uintptr_t)kBackdrop);
    env->backdropCreate(backdrop, nullptr, at<u8 *>(map, 0x44), at<s32>(map, 0x48), at<s32>(map, 0x4c), at<s32>(map, 0x50));
    env->backdropClear(backdrop);
    env->loadPicture(map, nullptr, 1);
    env->draw(map);
    env->uncacheRect(backdrop, nullptr, 0, 0, at<s32>(map, 0x48) - 1, at<s32>(map, 0x4c) - 1);
    env->advisor(0x130);
    env->helpPlace(0xffffffff, 10);
}

}  // extern "C"
