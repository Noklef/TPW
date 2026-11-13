#include "inputsystem.h"

#include <cstdint>

#include "accessors.h"
#include "app.h"
#include "events.h"
#include "factories.h"
#include "gameheap.h"
#include "initializers.h"
#include "misc.h"
#include "stdlist.h"
#include "vcall.h"
#include "vdtor.h"
#include "../gfx/screensurfaces.h"
#include "../input/devicefilter.h"
#include "imports.h"
#include "addrcalls.h"

namespace {

template <typename T>
T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(p) + offset);
}

template <typename T>
T &global(uintptr_t address) {
    return *reinterpret_cast<T *>(address);
}

template <typename F>
F fn(uintptr_t address) {
    return GameAddress<F>((u32)address);   // (the port in tpw.exe: core/addrcalls.h)
}

using Env = InputSystemEnv;
const InputSystemEnv kGameEnv = {fn<Env::Ctor>(0x631870), fn<Env::Ctor>(0x633360), fn<Env::Ctor>(0x633900), fn<Env::Ctor>(0x634220), fn<Env::Ctor>(0x6345d0), fn<Env::Ctor>(0x634ee0), &Class703ca0_Ctor,
                                 fn<Env::Plain>(0x631fc0), fn<Env::Plain>(0x632020), fn<Env::Plain>(0x6323e0), fn<Env::Plain>(0x6336f0), fn<Env::Plain>(0x633160), fn<Env::Plain>(0x633920), fn<Env::Plain>(0x634310),
                                 fn<Env::Arg>(0x633250), fn<Env::Arg>(0x632350),
                                 fn<Env::Out>(0x6326e0), fn<Env::Out>(0x633b90), fn<Env::Out>(0x633340), fn<Env::Out>(0x633ea0), fn<Env::Out>(0x6345a0), fn<Env::Out>(0x634a30),
                                 fn<Env::OutArg>(0x6328b0), fn<Env::OutArg>(0x6328d0), fn<Env::OutArg>(0x6335b0), fn<Env::OutArg>(0x633560), fn<Env::OutArg>(0x6341d0), fn<Env::OutArg>(0x633d10),
                                 fn<void(__fastcall *)(u8 *, void *, u32, u8 *)>(0x633700), &EventQueue_Post, &App_PumpMessages};
const InputSystemEnv *g_env = &kGameEnv;

constexpr u32 kNullObject = 0x80004003, kNoMemory = 0xcffe0100, kOpenAlready = 0xcffe0102, kNothingNew = 0x4ffe0002;
const uintptr_t kSystem = 0xfa6e40;

u8 *system() { return reinterpret_cast<u8 *>(kSystem); }

void release(u8 *object) {
    if (object) vcall<void>(object, 0);
}

u8 *freed(u8 *object, u32 flags) {
    if (flags & 1) GameHeap_Free(object);
    return object;
}

u8 *alloc(u32 size) { return static_cast<u8 *>(GameHeap_Alloc(size)); }

// The object's init the result: not failed, the object set, else its slot 2 (1).
u32 *finish(u32 *out, u8 **object, u8 *made, InputSystemEnv::Out init) {
    u32 result;
    init(made, nullptr, &result);
    if (static_cast<s32>(result) >= 0)
        *object = made;
    else
        vcall<u8 *>(made, 8, 1u);
    *out = result;
    return out;
}

u32 *createInit(u32 *out, u8 **object, u32 size, InputSystemEnv::Ctor ctor, InputSystemEnv::Out init) {
    if (!object) {
        *out = kNullObject;
        return out;
    }
    u8 *const block = alloc(size);
    u8 *const made = block ? ctor(block) : nullptr;
    if (!made) {
        *out = kNoMemory;
        return out;
    }
    return finish(out, object, made, init);
}

u32 *createPlain(u32 *out, u8 **object, u32 size, InputSystemEnv::Ctor ctor) {
    if (!object) {
        *out = kNullObject;
        return out;
    }
    u8 *const block = alloc(size);
    u8 *const made = block ? ctor(block) : nullptr;
    if (!made) {
        *out = kNoMemory;
        return out;
    }
    *object = made;
    *out = 0;
    return out;
}

u32 *closeDevice(u8 *o, u32 offset, u32 *out) {
    if (at<u8 *>(o, offset)) {
        u32 result;
        g_env->f6328d0(o + 0x10, nullptr, &result, at<u32>(o, offset));
        u8 *const device = at<u8 *>(o, offset);
        if (device) {
            at<u32>(o, offset) = 0;
            vcall<void>(device, 0);
        }
    }
    *out = 0;
    return out;
}

}  // namespace

void InputSystem_SetEnv(const InputSystemEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u8 *__fastcall InputSystem_DeletingDtor(u8 *o, void *, u32 flags) {
    InputSystem_Dtor(o);
    return freed(o, flags);
}

void __fastcall InputSystem_Dtor(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)Class703710_vtable;
    if (u8 *const part = at<u8 *>(o, 0x80)) vcall<u8 *>(part, 4, 1u);
    at<u32>(o, 0x84) = (u32)(uintptr_t)SurfaceSource_vtable;
    release(at<u8 *>(o, 0x78));
    release(at<u8 *>(o, 0x74));
    release(at<u8 *>(o, 0x70));
    release(at<u8 *>(o, 0x6c));
    InputBase_Dtor(o);
}

u32 __fastcall InputSystem_Update(u8 *o, void *, u32 a, u32 limit, u32 c) {
    u8 *const state = at<u8 *>(o, 0x6c);
    at<u32>(state, 0xc) = a;
    at<u32>(state, 0x10) = c;
    g_env->f631fc0(at<u8 *>(o, 0x68));
    u32 count = vcall<u32>(at<u8 *>(o, 0x68), 0x1c);
    if (limit && limit < count) count = limit;
    u32 local;
    for (u32 i = count; i; --i)
        vcall<void>(at<u8 *>(o, 0x68), 0x20, &local, at<u32>(o, 0x70));
    g_env->f632020(at<u8 *>(o, 0x68));
    return count;
}

u32 *__fastcall InputSystem_Init(u8 *o, void *, u32 *out) {
    u32 local;
    s32 result = static_cast<s32>(*InputBase_Init(o, nullptr, &local));
    if (result >= 0) result = static_cast<s32>(*Class703750_Create(&local, &at<u8 *>(o, 0x6c)));
    if (result >= 0) result = static_cast<s32>(*InputSystem_CreateDevice70(&local, &at<u8 *>(o, 0x70)));
    if (result >= 0) result = static_cast<s32>(*InputSystem_CreateDevice74(&local, &at<u8 *>(o, 0x74)));
    if (result >= 0) result = static_cast<s32>(*Class7037c0_Create(&local, &at<u8 *>(o, 0x78)));
    if (result < 0) {
        *out = static_cast<u32>(result);
        return out;
    }
    g_env->f633250(at<u8 *>(o, 0x74), nullptr, at<u32>(o, 0x60));
    const u32 added = *g_env->f6328b0(o + 0x10, nullptr, &local, at<u32>(o, 0x74));
    if (static_cast<s32>(added) >= 0) g_env->f632350(at<u8 *>(o, 0x70), nullptr, at<u32>(o, 0x6c));
    *out = added;
    return out;
}

u32 *__fastcall InputSystem_OpenDevice5c(u8 *o, void *, u32 *out) {
    u32 local;
    if (!at<u32>(o, 0x6c)) {
        const u32 result = *vcall<u32 *>(o, 8, &local);
        if (static_cast<s32>(result) < 0) {
            *out = result;
            return out;
        }
    }
    const u32 result = *InputBase_OpenDevice5c(o, nullptr, &local);
    g_env->f6335b0(at<u8 *>(o, 0x70), nullptr, &local, at<u32>(o, 0x5c));
    *out = result;
    return out;
}

u32 *__fastcall InputSystem_OpenDevice58(u8 *o, void *, u32 *out) {
    u32 local;
    if (!at<u32>(o, 0x6c)) {
        const u32 result = *vcall<u32 *>(o, 8, &local);
        if (static_cast<s32>(result) < 0) {
            *out = result;
            return out;
        }
    }
    const u32 result = *InputBase_OpenDevice58(o, nullptr, &local);
    g_env->f633560(at<u8 *>(o, 0x70), nullptr, &local, at<u32>(o, 0x58));
    *out = result;
    return out;
}

u8 *__fastcall Class703744_DeletingDtor(u8 *o, void *, u32 flags) {
    g_env->f6336f0(o);
    return freed(o, flags);
}

u32 *__cdecl InputSystem_CreateDevice70(u32 *out, u8 **object) { return createPlain(out, object, 0x124, g_env->c633360); }

u32 *__cdecl InputSystem_CreateDevice74(u32 *out, u8 **object) {
    if (!object) {
        *out = kNullObject;
        return out;
    }
    u8 *const made = alloc(0x38);
    if (!made) {
        *out = kNoMemory;
        return out;
    }
    Class703c18_Ctor(made);
    u8 *const part = made + 0x10, *const list = part + 4;
    const u8 listAllocator = 0, blockAllocator = 0;   // (stack bytes in the original; nothing reads what they set)
    Sentinel_Ctor(list, nullptr, &listAllocator);
    at<u32>(part, 0) = (u32)(uintptr_t)Class7037b8_vtable;
    u32 entry = 0;
    if (u8 *const block = alloc(0x14)) {
        at<u32>(block, 8) = 0;
        at<u32>(block, 0) = (u32)(uintptr_t)Class7037b4_vtable;
        entry = static_cast<u32>(reinterpret_cast<uintptr_t>(block));
    }
    StdListDword_PushBack(list, nullptr, &entry);
    u32 head;
    Fn_5ce530(list, nullptr, static_cast<u32>(reinterpret_cast<uintptr_t>(&head)));
    at<u32>(part, 0x10) = head;
    Fn_4f0150(made + 0x24, nullptr, static_cast<u32>(reinterpret_cast<uintptr_t>(&blockAllocator)));
    g_env->c633900(made + 0x34);
    at<u32>(made, 0) = (u32)(uintptr_t)Class7037a0_vtable;
    at<u32>(part, 0) = ((u32)(uintptr_t)Class703780_vtable + 0x1c);
    return finish(out, object, made, g_env->f633340);
}

u8 *__fastcall InputDevice74_DeletingDtor(u8 *o, void *, u32 flags) {
    InputDevice74_Dtor(o);
    return freed(o, flags);
}

u8 *__fastcall InputDevice74_DeletingDtorThunk(u8 *part, void *, u32 flags) { return InputDevice74_DeletingDtor(part - 0x10, nullptr, flags); }

void __fastcall InputDevice74_Dtor(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)Class7037a0_vtable;
    at<u32>(o, 0x10) = ((u32)(uintptr_t)Class703780_vtable + 0x1c);
    g_env->f633920(o + 0x34);
    GameHeap_Free(at<void *>(o, 0x28));
    at<u32>(o, 0x28) = 0;
    at<u32>(o, 0x2c) = 0;
    at<u32>(o, 0x30) = 0;
    Class7037b8_Dtor(o + 0x10);
    g_env->f633160(o);
}

u8 *__fastcall InputBase_DeletingDtor(u8 *o, void *, u32 flags) {
    InputBase_Dtor(o);
    return freed(o, flags);
}

void __fastcall InputBase_Dtor(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)Class7037e8_vtable;
    u32 local;
    InputBase_CloseDevice5c(o, nullptr, &local);
    InputBase_CloseDevice58(o, nullptr, &local);
    release(at<u8 *>(o, 0x68));
    release(at<u8 *>(o, 0x64));
    release(at<u8 *>(o, 0x60));
    release(at<u8 *>(o, 0x5c));
    release(at<u8 *>(o, 0x58));
    g_env->f6323e0(o + 0x10);
    g_env->f6336f0(o + 4);
    at<u32>(o, 0) = (u32)(uintptr_t)KeyFilter_vtable_703748;
}

u32 *__fastcall InputBase_Init(u8 *o, void *, u32 *out) {
    u32 local;
    s32 result = static_cast<s32>(*g_env->f6326e0(o + 0x10, nullptr, &local));
    if (result >= 0) result = static_cast<s32>(*InputBase_CreateDevice60(&local, &at<u8 *>(o, 0x60)));
    if (result >= 0) result = static_cast<s32>(*InputBase_CreateDevice64(&local, &at<u8 *>(o, 0x64)));
    if (result >= 0) result = static_cast<s32>(*InputBase_CreateChain68(&local, &at<u8 *>(o, 0x68)));
    if (result >= 0) {
        g_env->f632350(at<u8 *>(o, 0x64), nullptr, at<u32>(o, 0x68));
        g_env->f632350(at<u8 *>(o, 0x60), nullptr, at<u32>(o, 0x64));
        g_env->f633700(o + 4, nullptr, 0x7037e0, o);
    }
    *out = static_cast<u32>(result);
    return out;
}

static u32 g_activatedWhileClosed;   // (not the original's: see InputBase_OnEvent; the devices are acquired only after this value reaches them)

u32 *__fastcall InputBase_OpenDevice5c(u8 *o, void *, u32 *out) {
    if (at<u32>(o, 0x5c)) {
        *out = kOpenAlready;
        return out;
    }
    u32 local;
    u32 result = *InputBase_CreateDevice5c(&local, &at<u8 *>(o, 0x5c));
    if (static_cast<s32>(result) >= 0) result = *g_env->f6328b0(o + 0x10, nullptr, &local, at<u32>(o, 0x5c));
    if (static_cast<s32>(result) < 0) {
        *out = result;
        return out;
    }
    g_env->f633250(at<u8 *>(o, 0x5c), nullptr, at<u32>(o, 0x60));
    g_env->f633b90(at<u8 *>(o, 0x5c) + 0x10, nullptr, &local);
    if (g_activatedWhileClosed && at<u32>(o, 0x58)) {   // (the second device to open, Gui_Startup's order)
        InputBase_SetDeviceValue(o, nullptr, &local, g_activatedWhileClosed);
        g_activatedWhileClosed = 0;
    }
    *out = 0;
    return out;
}

u32 *__fastcall InputBase_CloseDevice5c(u8 *o, void *, u32 *out) { return closeDevice(o, 0x5c, out); }

u32 *__fastcall InputBase_CloseDevice58(u8 *o, void *, u32 *out) { return closeDevice(o, 0x58, out); }

u32 *__fastcall InputBase_OpenDevice58(u8 *o, void *, u32 *out) {
    if (at<u32>(o, 0x58)) {
        *out = kOpenAlready;
        return out;
    }
    u32 local;
    u32 result = *InputBase_CreateDevice58(&local, &at<u8 *>(o, 0x58));
    if (static_cast<s32>(result) >= 0) result = *g_env->f6328b0(o + 0x10, nullptr, &local, at<u32>(o, 0x58));
    if (static_cast<s32>(result) < 0) {
        *out = result;
        return out;
    }
    g_env->f633250(at<u8 *>(o, 0x58), nullptr, at<u32>(o, 0x60));
    g_env->f6341d0(at<u8 *>(o, 0x60), nullptr, &local, at<u32>(o, 0x58));
    g_env->f633b90(at<u8 *>(o, 0x58) + 0x10, nullptr, &local);
    *out = 0;
    return out;
}

u32 *__fastcall InputBase_SetDeviceValue(u8 *o, void *, u32 *out, u32 value) {
    Obj_Set10(at<u8 *>(o, 0x58) + 0x10, nullptr, value);
    Obj_Set10(at<u8 *>(o, 0x5c) + 0x10, nullptr, value);
    InputBase_PollAll(o, nullptr, out);
    return out;
}

void __fastcall InputBase_OnEvent(u8 *o, void *, u32, const u32 *event) {
    u32 local;
    switch (event[0]) {
    case 4:
        global<void(__stdcall *)(const char *)>(IMPORT(Kernel32_OutputDebugStringA))(gInputsystem_7780d0);
        global<u32(__stdcall *)(u32)>(IMPORT(User32_ShowCursor))(event[1] ? 0 : 1);
        // (Not the original's: it writes through the null devices when the window is activated before Gui_Startup opens them, or after they
        // close. Here the activation waits for the devices.)
        if (at<u32>(o, 0x58) && at<u32>(o, 0x5c)) {
            if (!at<u32>(o, 0x54) && event[1]) InputBase_SetDeviceValue(o, nullptr, &local, event[1]);
        } else {
            g_activatedWhileClosed = event[1];
        }
        return;
    case 6:
        at<u32>(o, 0x54) = 1;
        return;
    case 7:
        if (event[4]) {
            if (u8 *const device = at<u8 *>(o, 0x58)) g_env->f633d10(device + 0x10, nullptr, &local, global<u32>(0x703c78) | global<u32>(0x703c70));
            if (u8 *const device = at<u8 *>(o, 0x5c)) g_env->f633d10(device + 0x10, nullptr, &local, global<u32>(0x703c78) | global<u32>(0x703c70));
        } else {
            if (u8 *const device = at<u8 *>(o, 0x58)) g_env->f633d10(device + 0x10, nullptr, &local, global<u32>(0x703c74) | global<u32>(0x703c70));
            if (u8 *const device = at<u8 *>(o, 0x5c)) g_env->f633d10(device + 0x10, nullptr, &local, global<u32>(0x703c74) | global<u32>(0x703c6c));
        }
        InputBase_PollAll(o, nullptr, &local);
        at<u32>(o, 0x54) = 0;
        return;
    default:
        return;
    }
}

u32 *__fastcall InputBase_PollAll(u8 *o, void *, u32 *out) {
    u32 local;
    u8 *const first = at<u8 *>(o, 0x58);
    InputBase_Poll(o, nullptr, &local, first ? first + 0x10 : nullptr);
    u8 *const second = at<u8 *>(o, 0x5c);
    InputBase_Poll(o, nullptr, &local, second ? second + 0x10 : nullptr);
    *out = kNothingNew;
    return out;
}

u32 *__fastcall InputBase_Poll(u8 *, void *, u32 *out, u8 *part) {
    if (!part) {
        *out = kNothingNew;
        return out;
    }
    u32 result;
    g_env->f633b90(part, nullptr, &result);
    if (static_cast<s32>(result) >= 0 && result != kNothingNew) {
        u32 event[5];
        event[0] = 0xb;
        event[1] = static_cast<u32>(reinterpret_cast<uintptr_t>(part));
        g_env->post(reinterpret_cast<EventQueue *>((uintptr_t)0xfa7cb0), nullptr, 5, event);
    }
    *out = result;
    return out;
}

u32 *__cdecl InputBase_CreateDevice60(u32 *out, u8 **object) { return createInit(out, object, 0x24, g_env->classCa0, g_env->f633ea0); }

u32 *__cdecl InputBase_CreateDevice64(u32 *out, u8 **object) {
    if (!object) {
        *out = kNullObject;
        return out;
    }
    u8 *const made = alloc(0x74);
    if (!made) {
        *out = kNoMemory;
        return out;
    }
    g_env->c634220(made);
    at<u32>(made, 0x6c) = 0;
    at<u32>(made, 0x70) = 0;
    at<u32>(made, 0x68) = (u32)(uintptr_t)InputSubs_vtable;
    at<u32>(made, 0) = (u32)(uintptr_t)InputChain64_vtable;
    at<u32>(made, 0x64) = 0x703808;
    return finish(out, object, made, g_env->f6345a0);
}

u8 *__fastcall InputChain64_DeletingDtor(u8 *o, void *, u32 flags) {
    InputChain64_Dtor(o);
    return freed(o, flags);
}

u8 *__fastcall InputChain64_DeletingDtorThunk(u8 *part, void *, u32 flags) { return InputChain64_DeletingDtor(part - 0x64, nullptr, flags); }

void __fastcall InputChain64_Dtor(u8 *o) {
    at<u32>(o, 0) = (u32)(uintptr_t)InputChain64_vtable;
    at<u32>(o, 0x64) = 0x703808;
    g_env->f6336f0(o + 0x68);
    at<u32>(o, 0x64) = (u32)(uintptr_t)KeyFilter_vtable_703748;
    g_env->f634310(o);
}

u32 *__cdecl InputBase_CreateChain68(u32 *out, u8 **object) { return createPlain(out, object, 0x44, g_env->c631870); }

u32 *__cdecl InputBase_CreateDevice5c(u32 *out, u8 **object) { return createInit(out, object, 0x5c, g_env->c6345d0, g_env->f634a30); }

u32 *__cdecl InputBase_CreateDevice58(u32 *out, u8 **object) { return createInit(out, object, 0x48, g_env->c634ee0, g_env->f634a30); }

u32 __cdecl Input_Update(u32 a, u32 limit, u32 c, u32 noPump) {
    if (!noPump) g_env->pump();
    return InputSystem_Update(system(), nullptr, a, limit, c);
}

u32 *__cdecl Input_OpenDevice58(u32 *out) {
    InputSystem_OpenDevice58(system(), nullptr, out);
    return out;
}

void __cdecl Input_CloseDevice58() {
    u32 local;
    InputBase_CloseDevice58(system(), nullptr, &local);
}

u32 *__cdecl Input_OpenDevice5c(u32 *out) {
    InputSystem_OpenDevice5c(system(), nullptr, out);
    return out;
}

void __cdecl Input_CloseDevice5c() {
    u32 local;
    InputBase_CloseDevice5c(system(), nullptr, &local);
}

}  // extern "C"
