// The screen's window and DirectDraw object: see screen.h.
#include "screen.h"

#include <cstdint>
#include <cstring>

#include "../core/app.h"
#include "../core/vcall.h"

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const ScreenEnv kGameEnv = {
    at<ScreenState *>(0x00fa7948),
    at<s32 *>(0x00fa9f3c),
    at<List *>(0x00fa77d0),
    at<char *>(0x00fa77e0),
    at<u8 *>(0x00fa78e8),
    at<FpuControl *>(0x00fa78ec),
    at<Surface *>(0x00fa78f8),
    at<Surface *>(0x00fa7730),
    at<Surface *>(0x00fa7780),
    at<void *>(0x00fa7cb0),
    at<void(__fastcall *)(void *, void *, u32, const u32 *)>(0x005fbf80),
    at<void *(__cdecl *)(u32)>(0x0067acc0),
    at<void(__cdecl *)(void *)>(0x0067acb0),
    at<const char *>(0x00775840),
    at<const char *>(0x0077584c),
    at<void *>(0x005fa540),
    at<void *(__stdcall *const *)(u32, const char *, const char *, u32, s32, s32, s32, s32, void *, void *, void *,
                                  void *)>(0x006fd398),
    at<s32(__stdcall *const *)(s32)>(0x006fd338),
    at<s32(__stdcall *const *)(void *)>(0x006fd33c),
    at<void *(__stdcall *const *)(void *, const char *)>(0x006fd364),
    at<void *(__stdcall *const *)(s32)>(0x006fd054),
    at<u16(__stdcall *const *)(const void *)>(0x006fd394),
    at<s32(__stdcall *const *)(void *, u32, u32, s32)>(0x006fd340),
    at<void *(__stdcall *const *)(void *)>(0x006fd334),
    at<s32(__stdcall *const *)(const void *, void **, void *)>(0x006fd018),
};
const ScreenEnv *g_env = &kGameEnv;

const u32 kExclusive = 0x55;  // DDSCL_FULLSCREEN | NOWINDOWCHANGES | EXCLUSIVE | ALLOWMODEX
const u32 kNormal = 8;        // DDSCL_NORMAL

void post(u32 type, u32 value) {
    const u32 event[2] = {type, value};
    g_env->post(g_env->queue, nullptr, 0, event);
}

void release(Surface *s) {
    if (s->surface) {
        comcall<u32>(s->surface, 8);
        s->surface = nullptr;
    }
}

}  // namespace

void Screen_SetEnv(const ScreenEnv *e) { g_env = e ? e : &kGameEnv; }
const ScreenEnv *Screen_GetEnv() { return g_env; }

extern "C" {

void __cdecl Screen_SetGuid(const void *guid) { g_env->state->guid = guid; }

void *__cdecl Screen_GetDirectDraw(s32 addRef) {
    ScreenState *s = g_env->state;
    if (addRef && s->directDraw) comcall<u32>(s->directDraw, 4);
    return s->directDraw;
}

void __cdecl Screen_Close() {
    const ScreenEnv *e = g_env;
    if (*e->open) {
        *e->open = 0;
        Screen_ReleaseSurfaces();
        Screen_ReleaseDirectDraw();
        if (e->state->window) (*e->destroyWindow)(e->state->window);
        e->state->window = nullptr;
    }
    while (ListNode *n = List_First(e->modes)) {
        n->prev->next = n->next;
        n->next->prev = n->prev;
        e->free(n);
    }
}

void __cdecl Screen_ReleaseSurfaces() {
    const ScreenEnv *e = g_env;
    ScreenState *s = e->state;
    *e->lockFlag = 0;
    FpuControl_Restore(e->fpu);
    s->active = 0;
    s->unknown18 = 0;
    if (!e->primary->surface) return;
    post(5, 0);
    if (s->directDraw) comcall<s32>(s->directDraw, 0x50, s->coopWindow, kNormal);  // SetCooperativeLevel
    release(e->surfaceA);
    release(e->surfaceB);
    release(e->primary);
    s->unknown0c = 0;
    if (s->unknown10) {
        comcall<u32>(s->unknown10, 8);
        s->unknown10 = nullptr;
    }
}

void __cdecl Screen_ReleaseDirectDraw() {
    ScreenState *s = g_env->state;
    if (!s->directDraw) return;
    comcall<s32>(s->directDraw, 0x4c);  // RestoreDisplayMode
    comcall<u32>(s->directDraw, 8);
    s->directDraw = nullptr;
    Surface_SetDirectDraw(nullptr);
}

s32 *__cdecl Screen_Open(s32 *out, const void *guid, s32 exclusive, void *window, void *listWindow) {
    const ScreenEnv *e = g_env;
    if (!*e->open) {
        App_SetWindow(window);
        if (!window) {
            Screen_RegisterClass();
            void *const instance = App_GetInstance();
            const s32 height = (*e->getSystemMetrics)(1);  // SM_CYSCREEN
            const s32 width = (*e->getSystemMetrics)(0);   // SM_CXSCREEN
            // WS_EX_TOPMOST; WS_POPUP | WS_SYSMENU
            void *const made = (*e->createWindowEx)(8, e->className, App_GetName(), 0x80080000, 0, 0, width, height,
                                                    nullptr, nullptr, instance, nullptr);
            e->state->window = made;
            if (!made) {
                *out = -1;
                return out;
            }
            App_SetWindow(made);
        }
        s32 listed;
        if (*Screen_ListModes(&listed, exclusive, listWindow, guid) < 0) {
            *out = -1;
            return out;
        }
        Screen_SetGuid(guid);
        *e->open = 1;
    }
    *out = 0;
    return out;
}

s32 *__cdecl Screen_CreateDirectDraw(s32 *out, const void *guid, void **directDraw, void *) {
    if ((*g_env->directDrawCreate)(guid, directDraw, nullptr)) {
        *out = -1;
        return out;
    }
    if (g_env->state->window) App_SetWindow(g_env->state->window);
    *out = 0;
    return out;
}

s32 *__cdecl Screen_ListModes(s32 *out, s32 exclusive, void *window, const void *guid) {
    const ScreenEnv *e = g_env;
    void *dd = nullptr;
    u32 unused = 0;
    s32 created;
    if (*Screen_CreateDirectDraw(&created, guid, &dd, &unused) < 0) {
        *out = -1;
        return out;
    }
    List_Init(e->modes);
    auto enumerate = [&] {
        comcall<s32>(dd, 0x20, 0u, static_cast<void *>(nullptr), e->modes, &Screen_AddMode);  // EnumDisplayModes
    };
    if (!exclusive) {
        enumerate();
    } else {
        void *w = window;
        if (!w) {
            Screen_RegisterClass();
            void *const instance = App_GetInstance();
            w = (*e->createWindowEx)(0, e->className, e->listTitle, 0x80000000, 0, 0, 0x500, 0x400, nullptr, nullptr,
                                     instance, nullptr);  // WS_POPUP
            if (!w) {
                *out = -1;
                return out;
            }
        }
        if (comcall<s32>(dd, 0x50, w, kExclusive)) {
            comcall<u32>(dd, 8);
            *out = -1;
            return out;
        }
        enumerate();
        comcall<s32>(dd, 0x50, w, kNormal);
        if (!window) (*e->destroyWindow)(w);
    }
    comcall<u32>(dd, 8);
    *out = 0;
    return out;
}

s32 __stdcall Screen_AddMode(const u8 *desc, List *modes) {
    ScreenMode *m = static_cast<ScreenMode *>(g_env->alloc(sizeof(ScreenMode)));
    if (!m) return 0;
    std::memcpy(&m->width, desc + 0xc, 4);
    std::memcpy(&m->height, desc + 8, 4);
    std::memcpy(&m->bitCount, desc + 0x54, 4);
    List_InsertAfter(modes, modes->tailPred, &m->link);
    return 1;
}

void __cdecl Screen_RegisterClass() {
    const ScreenEnv *e = g_env;
    if (e->state->classRegistered) return;
    u32 wc[10] = {};  // WNDCLASSA
    e->state->classRegistered = 1;
    wc[0] = 3;        // CS_HREDRAW | CS_VREDRAW
    wc[1] = static_cast<u32>(reinterpret_cast<std::uintptr_t>(e->wndProc));
    wc[4] = static_cast<u32>(reinterpret_cast<std::uintptr_t>(App_GetInstance()));
    if (App_GetIconId()) {
        const u16 id = App_GetIconId();
        wc[5] = static_cast<u32>(reinterpret_cast<std::uintptr_t>(
            (*e->loadIcon)(App_GetInstance(), reinterpret_cast<const char *>(static_cast<std::uintptr_t>(id)))));
    }
    wc[7] = static_cast<u32>(reinterpret_cast<std::uintptr_t>((*e->getStockObject)(4)));  // BLACK_BRUSH
    std::memcpy(e->className, e->classSource, std::strlen(e->classSource) + 1);  // sprintf, no conversions
    wc[9] = static_cast<u32>(reinterpret_cast<std::uintptr_t>(e->className));
    (*e->registerClass)(wc);
}

s32 __stdcall Screen_WndProc(void *window, u32 message, u32 wParam, s32 lParam) {
    switch (message) {
    case 6:  // WM_ACTIVATE
        if (window == App_GetWindow()) {
            const s32 active = wParam == 1 || wParam == 2;  // WA_ACTIVE, WA_CLICKACTIVE
            Screen_SetActive(active);
            post(4, static_cast<u32>(active));
        }
        break;
    case 0x10:  // WM_CLOSE
        App_SetQuit(1);
        return 0;
    case 0x1c:  // WM_ACTIVATEAPP
        App_SetActive(wParam != 0);
        break;
    case 0x112:  // WM_SYSCOMMAND
        if (wParam == 0xf100) return 0;  // SC_KEYMENU
        break;
    }
    if (App_GetWndProc()) return App_GetWndProc()(window, message, wParam, lParam);
    return (*g_env->defWindowProc)(window, message, wParam, lParam);
}

void __cdecl Screen_SetActive(s32 active) {
    g_env->state->active = active;
    (*g_env->setCursor)(nullptr);
}

}  // extern "C"
