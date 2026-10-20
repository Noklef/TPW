// The asynchronous pointer (0x5fb8e0-0x5fbdd9, thiscall unless noted; vtable 0x703644): a
// thread ("Asynchronous Pointer Thread") that keeps the software cursor (cursor.h) drawn on the
// display's back surface every 66 ms, a semaphore keeping it and the game's own calls apart.
//
// The display is the game's object at 0x7b4ac0 (vtable 0x6fef78), seen here only through its
// virtual methods: the back surface (+4), the front surface (+8), its bounds (+0xc) and whether it
// is flipping (+0x10; always 0 in this game). Around a flip, BeginFlip takes the lock and draws
// the cursor afresh onto the front surface; EndFlip puts back what it covered and lets go.
//
// The originals' quirks are kept: Attach ignores whether it got the lock (it asks to wait
// forever); BeginFlip returns holding the lock for EndFlip; SetPaused answers 0x4ffe0002 when
// already in the state asked for; the thread procedure ends the thread itself (_endthreadex).
#pragma once

#include <cstddef>

#include "../../../re/types.h"
#include "../core/semaphore.h"
#include "../math/rect.h"
#include "cursor.h"
#include "surface.h"

struct AsyncPointer {
    u32 vtable;         // 0x703644: the deleting destructor only
    void *display;      // +4: 0x7b4ac0
    Semaphore lock;     // +8
    void *thread;       // +0xc
    u8 stop;            // +0x10: tells the thread to finish
    u8 paused;          // +0x11: no thread (SetPaused)
    u8 park;            // +0x12: what BeginFlip and EndFlip hand the cursor (never set here)
    u8 pad13;
    u32 nextTick;       // +0x14: when the thread draws next (ms)
    u32 interval;       // +0x18: 0x42
    Cursor *cursor;     // +0x1c
    const s32 *pos;     // +0x20: the mouse position, read on each draw
};
static_assert(offsetof(AsyncPointer, nextTick) == 0x14, "AsyncPointer::nextTick is at +0x14");
static_assert(sizeof(AsyncPointer) == 0x24, "AsyncPointer is 0x24 bytes");

struct AsyncPointerEnv {
    u32 vtable;                                // 0x703644
    void(__cdecl *free)(void *);               // 0x67acb0
    u32(__stdcall *threadProc)(void *);        // 0x5fba00, what Start hands _beginthreadex
    void *(__cdecl *beginThread)(void *, u32, u32(__stdcall *)(void *), void *, u32, u32 *);  // 0x67f780
    void(__cdecl *endThread)(u32);             // 0x67f8b0
    s32(__stdcall *const *getExitCodeThread)(void *, u32 *);  // 0x6fd0b8
    u32(__stdcall *const *waitForSingleObject)(void *, u32);  // 0x6fd128
    s32(__stdcall *const *closeHandle)(void *);               // 0x6fd13c
    void(__stdcall *const *outputDebugString)(const char *);  // 0x6fd1e0
    const char *started;                       // 0x7758a4
    const char *finished;                      // 0x775878
};
// nullptr restores the game's.
void AsyncPointer_SetEnv(const AsyncPointerEnv *e);
const AsyncPointerEnv *AsyncPointer_GetEnv();

extern "C" {
// 0x5fb8e0 (display): paused and stopped, no cursor, a 66 ms interval.
AsyncPointer *__fastcall AsyncPointer_Ctor(AsyncPointer *p, void *edx, void *display);
AsyncPointer *__fastcall AsyncPointer_DeletingDtor(AsyncPointer *p, void *edx, u32 flags);  // 0x5fb920
void __fastcall AsyncPointer_Dtor(AsyncPointer *p);     // 0x5fb940
void __fastcall AsyncPointer_Detach(AsyncPointer *p);   // 0x5fb990: Cursor_Discard; Stop
void __fastcall AsyncPointer_Stop(AsyncPointer *p);     // 0x5fb9b0: waits for the thread, closes it
u32 __stdcall AsyncPointer_ThreadProc(void *p);         // 0x5fba00
// 0x5fba10 (now): draws onto the back surface when due, unless paused or the lock is busy.
void __fastcall AsyncPointer_Tick(AsyncPointer *p, void *edx, u32 now);
u32 __fastcall AsyncPointer_Run(AsyncPointer *p);       // 0x5fba70: the thread's loop
u8 __fastcall AsyncPointer_Lock(AsyncPointer *p, void *edx, u8 forever);  // 0x5fbad0: kept held
void __fastcall AsyncPointer_Unlock(AsyncPointer *p);  // 0x5fbb70
// 0x5fbbd0 (&out, &pos): the cursor singleton, clipped to the display's bounds; 0.
s32 *__fastcall AsyncPointer_Attach(AsyncPointer *p, void *edx, s32 *out, const s32 *pos);
s32 *__fastcall AsyncPointer_Start(AsyncPointer *p, void *edx, s32 *out);  // 0x5fbc30: -1 without a thread
s32 *__fastcall AsyncPointer_SetPaused(AsyncPointer *p, void *edx, s32 *out, u8 paused);  // 0x5fbc80
// 0x5fbcf0 (&out, image, &rect, &hotspot): Cursor_SetImage under the lock, then a Tick.
s32 *__fastcall AsyncPointer_SetImage(AsyncPointer *p, void *edx, s32 *out, Surface *image, const Rect *rect,
                                      const s32 *hot);
void __fastcall AsyncPointer_BeginFlip(AsyncPointer *p);  // 0x5fbd40
void __fastcall AsyncPointer_EndFlip(AsyncPointer *p);    // 0x5fbd90

// The 0x6fefc8 class's virtual slots 2 and 3 (its constructor and destructors: core/initializers; names unconfirmed), on the game's asynchronous
// pointer [0x7b4b84]. The object: +4 a shown byte, +5 / +6 the hot spot (signed bytes), +0xc a frame, +0x14 a Surface whose frames are
// 32 x 32 side by side. 0x47ebc0 (thiscall) shows the frame: SetImage(the Surface, frame * 32, 0 to (frame + 1) * 32, 32, the hot spot),
// shown = 1. 0x47ec30 (thiscall) takes it away: SetImage(no image, a zero rectangle and hot spot), shown = 0.
void __fastcall PointerFrames_Show(u8 *self);
void __fastcall PointerFrames_Hide(u8 *self);
}
