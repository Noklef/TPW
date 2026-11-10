// The asynchronous pointer: see pointer.h.
#include "pointer.h"

#include <cstdint>

#include "../core/vcall.h"
#include "../files/file.h"
#include "imageops.h"
#include "../render/screen.h"

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const AsyncPointerEnv kGameEnv = {
    (u32)(uintptr_t)AsyncPointer_vtable,
    at<void(__cdecl *)(void *)>(0x0067acb0),
    at<u32(__stdcall *)(void *)>(0x005fba00),
    at<void *(__cdecl *)(void *, u32, u32(__stdcall *)(void *), void *, u32, u32 *)>(0x0067f780),
    at<void(__cdecl *)(u32)>(0x0067f8b0),
    at<s32(__stdcall *const *)(void *, u32 *)>(0x006fd0b8),
    at<u32(__stdcall *const *)(void *, u32)>(0x006fd128),
    at<s32(__stdcall *const *)(void *)>(0x006fd13c),
    at<void(__stdcall *const *)(const char *)>(0x006fd1e0),
    at<const char *>(0x007758a4),
    at<const char *>(0x00775878),
};
const AsyncPointerEnv *g_env = &kGameEnv;

const u32 kStillActive = 0x103;
const s32 kAlready = 0x4ffe0002;

// The display's virtual methods.
Surface *backSurface(void *d) { return vcall<Surface *>(d, 4); }
Surface *frontSurface(void *d) { return vcall<Surface *>(d, 8); }
Rect *bounds(void *d, Rect *out) { return vcall<Rect *>(d, 0xc, out); }
u8 flipping(void *d) { return vcall<u8>(d, 0x10); }

}  // namespace

void AsyncPointer_SetEnv(const AsyncPointerEnv *e) { g_env = e ? e : &kGameEnv; }
const AsyncPointerEnv *AsyncPointer_GetEnv() { return g_env; }

namespace {
AddrCalls g_calls;
struct ScreenEvent {   // (what the event queue is posted: the byte's upper bytes and the rest for kind 6 stale, as in the original)
    u32 kind, a, b;
    u8 c, pad[3];
    u32 d;
};
}
void AsyncPointer_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

AsyncPointer *__fastcall AsyncPointer_Ctor(AsyncPointer *p, void *, void *display) {
    p->display = display;
    Semaphore_Ctor(&p->lock);
    p->thread = nullptr;
    p->park = 0;
    p->nextTick = 0;
    p->cursor = nullptr;
    p->stop = 1;
    p->paused = 1;
    p->interval = 0x42;
    p->vtable = g_env->vtable;
    return p;
}

AsyncPointer *__fastcall AsyncPointer_DeletingDtor(AsyncPointer *p, void *, u32 flags) {
    AsyncPointer_Dtor(p);
    if (flags & 1) g_env->free(p);
    return p;
}

void __fastcall AsyncPointer_Dtor(AsyncPointer *p) {
    p->vtable = g_env->vtable;
    AsyncPointer_Detach(p);
    Semaphore_Dtor(&p->lock);
}

void __fastcall AsyncPointer_Detach(AsyncPointer *p) {
    Cursor *const c = p->cursor;
    p->cursor = nullptr;
    if (c) Cursor_Discard(c);
    AsyncPointer_Stop(p);
}

void __fastcall AsyncPointer_Stop(AsyncPointer *p) {
    if (!p->thread) return;
    p->stop = 1;
    u32 code;
    if ((*g_env->getExitCodeThread)(p->thread, &code) && code == kStillActive)
        (*g_env->waitForSingleObject)(p->thread, 0xffffffff);
    (*g_env->closeHandle)(p->thread);
    p->thread = nullptr;
}

u32 __stdcall AsyncPointer_ThreadProc(void *p) { return AsyncPointer_Run(static_cast<AsyncPointer *>(p)); }

void __fastcall AsyncPointer_Tick(AsyncPointer *p, void *, u32 now) {
    if (!p->cursor || p->paused) return;
    if (static_cast<s32>(now - p->nextTick) <= 0) return;
    Surface *const back = backSurface(p->display);
    if (!back || !AsyncPointer_Lock(p, nullptr, 0)) return;
    Cursor_Draw(p->cursor, nullptr, p->pos, back);
    p->nextTick = p->interval + now;
    AsyncPointer_Unlock(p);
}

u32 __fastcall AsyncPointer_Run(AsyncPointer *p) {
    (*g_env->outputDebugString)(g_env->started);
    p->nextTick = Time_GetMs();
    while (!p->stop) {
        AsyncPointer_Tick(p, nullptr, Time_GetMs());
        Sys_Sleep(0);
    }
    (*g_env->outputDebugString)(g_env->finished);
    g_env->endThread(0);
    return 0;
}

u8 __fastcall AsyncPointer_Lock(AsyncPointer *p, void *, u8 forever) {
    SemaphoreLock l = {p->lock.handle, 0, 0};
    u8 got = 0;
    if (SemaphoreLock_Acquire(&l, nullptr, forever)) {
        l.kept = 1;  // stays held until Unlock
        got = 1;
    }
    SemaphoreLock_Dtor(&l);
    return got;
}

void __fastcall AsyncPointer_Unlock(AsyncPointer *p) {
    SemaphoreLock l;
    SemaphoreLock_Ctor(&l, nullptr, &p->lock, 1);
    SemaphoreLock_Release(&l);
    SemaphoreLock_Dtor(&l);
}

s32 *__fastcall AsyncPointer_Attach(AsyncPointer *p, void *, s32 *out, const s32 *pos) {
    AsyncPointer_Lock(p, nullptr, 1);
    p->cursor = Cursor_Get();
    p->pos = pos;
    Rect r;
    p->cursor->clip = *bounds(p->display, &r);
    AsyncPointer_Unlock(p);
    *out = 0;
    return out;
}

s32 *__fastcall AsyncPointer_Start(AsyncPointer *p, void *, s32 *out) {
    AsyncPointer_Stop(p);
    p->stop = 0;
    u32 id;
    p->thread = g_env->beginThread(nullptr, 0, g_env->threadProc, p, 0, &id);
    *out = p->thread ? 0 : -1;
    return out;
}

s32 *__fastcall AsyncPointer_SetPaused(AsyncPointer *p, void *, s32 *out, u8 paused) {
    s32 r = kAlready;
    if (paused) {
        if (!p->paused) {
            AsyncPointer_Stop(p);
            p->paused = 1;
            r = 0;
        }
    } else if (p->paused) {
        s32 started;
        r = *AsyncPointer_Start(p, nullptr, &started);
        if (r >= 0) p->paused = 0;
    }
    *out = r;
    return out;
}

s32 *__fastcall AsyncPointer_SetImage(AsyncPointer *p, void *, s32 *out, Surface *image, const Rect *rect,
                                      const s32 *hot) {
    AsyncPointer_Lock(p, nullptr, 1);
    s32 set;
    const s32 r = *Cursor_SetImage(p->cursor, nullptr, &set, image, rect, hot);
    AsyncPointer_Unlock(p);
    AsyncPointer_Tick(p, nullptr, Time_GetMs());
    *out = r;
    return out;
}

void __fastcall AsyncPointer_BeginFlip(AsyncPointer *p) {
    AsyncPointer_Lock(p, nullptr, 1);
    void *const d = p->display;
    const u8 flip = flipping(d);
    Surface *const front = frontSurface(d);
    Surface *const back = backSurface(d);
    Cursor_Update(p->cursor, nullptr, p->pos, reinterpret_cast<std::uintptr_t>(back), front, p->park, flip);
}

void __fastcall AsyncPointer_EndFlip(AsyncPointer *p) {
    void *const d = p->display;
    const u8 flip = flipping(d);
    Surface *const front = frontSurface(d);
    Surface *const back = backSurface(d);
    Cursor_Restore(p->cursor, nullptr, back, front, p->park, flip);
    p->nextTick = Time_GetMs() + p->interval;
    AsyncPointer_Unlock(p);
}

void __fastcall PointerFrames_Show(u8 *self) {
    const s32 hot[2] = {static_cast<s8>(self[5]), static_cast<s8>(self[6])};
    const s32 frame = *reinterpret_cast<s32 *>(self + 0xc);
    const Rect rect = {frame << 5, 0, (frame + 1) << 5, 0x20};
    s32 out;
    AsyncPointer_SetImage(*at<AsyncPointer **>(0x007b4b84), nullptr, &out, reinterpret_cast<Surface *>(self + 0x14), &rect, hot);
    self[4] = 1;
}

void __fastcall PointerFrames_Hide(u8 *self) {
    const s32 hot[2] = {0, 0};
    const Rect rect = {0, 0, 0, 0};
    s32 out = 0;
    AsyncPointer_SetImage(*at<AsyncPointer **>(0x007b4b84), nullptr, &out, nullptr, &rect, hot);
    self[4] = 0;
}

u32 __cdecl AsyncPointer_Make() {
    u32 &pointer = *reinterpret_cast<u32 *>((uintptr_t)0x007b4b84);
    const u32 &height = gScreen_7a1968, &width = gScreen_7a1930;
    ScreenEvent event;
    event.kind = 6;
    event.a = 0;
    g_calls.Th(0x005fbf80, 0x00fa7cb0u, 0u, &event);
    event.kind = 7;
    event.a = height;
    event.b = width;
    event.c = 0x10;
    event.d = 0;
    g_calls.Th(0x005fbf80, 0x00fa7cb0u, 0u, &event);
    g_calls.Th(0x0047e460, 0x007b4ac0u);
    s32 clip[4] = {0, 0, (s32)height - 1, (s32)width - 1};
    g_calls.Cd(0x005f5650, clip);
    pointer = 0;
    const u32 block = g_calls.Cd(0x0067acc0, 0x24u);
    pointer = block ? g_calls.Th(0x005fb8e0, block, 0x007b4ac0u) : 0;
    if (!pointer) return 0;
    const u32 position = g_calls.Cd(0x005f5640);
    u8 reply[8];
    g_calls.Th(0x005fbbd0, pointer, reply, position);
    const u8 *card = *reinterpret_cast<const u8 **>((uintptr_t)0x00879a24);
    const bool paused = (*reinterpret_cast<const u32 *>(card + 0x428) == 0x121a && *reinterpret_cast<const u32 *>(card + 0x42c) < 5) || *reinterpret_cast<const u32 *>((uintptr_t)0x0078826c);
    g_calls.Th(0x005fbc80, pointer, reply, paused ? 1u : 0u);
    reply[0] = 0xff;
    reply[1] = 0;
    reply[2] = 0xff;
    reply[3] = 0xff;
    reply[4] = (u8)g_calls.Cd(0x005f73a0, 0x00fa71f8u, 0xffu, 0u, 0xffu);
    g_calls.Th(0x005fb7b0, *reinterpret_cast<u32 *>((uintptr_t)pointer + 0x1c), reply);
    *reinterpret_cast<u32 *>((uintptr_t)0x008bd510) = 0x0047e9b0;
    *reinterpret_cast<u32 *>((uintptr_t)0x008bd514) = 0x0047e9c0;
    *reinterpret_cast<u32 *>((uintptr_t)0x007b4b88) = 1;
    return 1;
}

u32 __fastcall PointerFrames_LoadFile(u8 *self, void *, const char *name) {
    const u32 loaded = Surface_LoadTga(name, self + 0x14, reinterpret_cast<u32 *>(self + 8));
    *reinterpret_cast<u32 *>(self + 0x64) = loaded;
    return loaded;
}

void __fastcall PointerFrames_Step(u8 *self, void *, s32 now) {
    s32 *const frame = reinterpret_cast<s32 *>(self + 0xc);
    s32 *const last = reinterpret_cast<s32 *>(self + 0x10);
    if (!self[4] || !*at<AsyncPointer **>(0x007b4b84) || (s32)((u32)now - (u32)*last) <= 300) return;
    if (++*frame >= *reinterpret_cast<s32 *>(self + 8)) *frame = 0;
    const s32 hot[2] = {static_cast<s8>(self[5]), static_cast<s8>(self[6])};
    const Rect rect = {*frame << 5, 0, (*frame + 1) << 5, 0x20};
    s32 out;   // (the original's: the now argument's slot)
    AsyncPointer_SetImage(*at<AsyncPointer **>(0x007b4b84), nullptr, &out, reinterpret_cast<Surface *>(self + 0x14), &rect, hot);
    *last = now;
}

}  // extern "C"
