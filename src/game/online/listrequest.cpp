#include "listrequest.h"

#include <cstdint>

#include "../core/buffers.h"
#include "../core/dequeiter.h"
#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "session.h"
#include "sessionlogin.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const ListRequestEnv kGameEnv = {&CommandQueue_Post};
const ListRequestEnv *g_env = &kGameEnv;

using Callback = void(__cdecl *)(u32, u32, u32);
using Done = void(__fastcall *)(u8 *, void *, u32);

void PostError(u8 *list) {
    g_env->post(at<u32>(list, 0x18), 0);
    at<u32>(list, 0x18) = 0;
}

void Finish(u8 *list, u32 failed, u32 size, u32 destructor) {
    u32 posted = 0;
    if (at<u32>(list, 0x18)) {
        PostError(list);
        posted = 1;
    }
    const Callback callback = at<Callback>(list, 0x1c);
    const u32 user = at<u32>(list, 0x20);
    if (!failed && !posted && at<u32>(list, 0x14) && at<u32>(list, 0x30)) {
        while (at<u32>(at<u8 *>(list, 0x2c), 0xc)) {
            u8 *const owner = at<u8 *>(list, 0x2c);
            u8 *item = nullptr;
            if (u8 *const node = at<u8 *>(owner, 4)) {
                u8 *const next = at<u8 *>(node, 4);
                at<u8 *>(owner, 4) = next;
                if (next)
                    at<u32>(next, 8) = 0;
                else
                    at<u32>(owner, 8) = 0;
                --at<u32>(owner, 0xc);
                item = at<u8 *>(node, 0);
                GameHeap_Free(node);
            }
            if (item)
                vcall<void>(item, 0, 1u);
        }
        at<u32>(list, 0x24) = 0;
        at<u32>(list, 0x28) = 0;
        vcall<void>(list, 0xc, at<u32>(list, 0x30));
        at<u32>(list, 0x24) = at<u32>(at<u8 *>(list, 0x30), 8);
        Shared_Release(list, size, destructor);
        at<u32>(list, 0x28) = 1;
    }
    u32 result = 0;
    if (failed || posted || !at<u32>(list, 0x28)) {
        if (at<u32>(list, 8))
            Online_ResetAll(Online_Instance());
        result = 1;
    }
    if (callback)
        callback(result, user, at<u32>(list, 0x2c));
}

void Reset(u8 *list, u32 callback, u32 user, u32 size, u32 destructor) {
    at<u32>(list, 4) = 0xffffffff;
    at<u32>(list, 0x10) = 0;
    at<u32>(list, 0x14) = 0;
    at<u32>(list, 8) = 0;
    at<u32>(list, 0xc) = 0;
    at<u32>(list, 0x18) = 0;
    at<u32>(list, 0x1c) = callback;
    at<u32>(list, 0x20) = user;
    Shared_Release(list, size, destructor);
}

void Ask(u8 *list, Done done) {
    if (vcall<u32>(list, 4)) {
        at<u32>(list, 0x18) = 0x4e;
        done(list, nullptr, 1);
        return;
    }
    u8 *const city = Online_OpenCity(Online_Instance());
    if (!city) {
        at<u32>(list, 0x18) = 0x4f;
        done(list, nullptr, 1);
        return;
    }
    at<u32>(list, 0xc) = 1;
    at<u32>(list, 4) = vcall<u32>(list, 8, city, at<u32>(list, 0x24));
}

void Start(u8 *list, Done done, u32 login, u32 user) {
    if (Online_LoggedIn(Online_Instance())) {
        Ask(list, done);
        return;
    }
    at<u32>(list, 8) = 1;
    Online_OpenLoginDialog(Online_Instance(), nullptr, login, user);
}

// As Start with the session's own list asked (sic: resets went to this list).
void StartMember(u8 *list, Done done, u32 member, u32 login) {
    if (Online_LoggedIn(Online_Instance())) {
        Ask(at<u8 *>(Online_Instance(), member), done);
        return;
    }
    at<u32>(list, 8) = 1;
    Online_OpenLoginDialog(Online_Instance(), nullptr, login, 0);
}

void Login(u32 flag, u8 *list, Done done) {
    if (!flag) {
        Ask(list, done);
        return;
    }
    if (at<u32>(list, 0x18))
        PostError(list);
    const Callback callback = at<Callback>(list, 0x1c);
    const u32 user = at<u32>(list, 0x20);
    if (at<u32>(list, 8))
        Online_ResetAll(Online_Instance());
    if (callback)
        callback(1, user, at<u32>(list, 0x2c));
}

}  // namespace

void ListRequest_SetEnv(const ListRequestEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall ListRequest_Done30(u8 *list, void *, u32 failed) { Finish(list, failed, 0x30, 0x006fd500); }
void __fastcall ListRequest_Done66(u8 *list, void *, u32 failed) { Finish(list, failed, 0x66, 0x006fd504); }
void __fastcall ListRequest_Done10(u8 *list, void *, u32 failed) { Finish(list, failed, 0x10, 0x006fd50c); }
void __fastcall ListRequest_Done26(u8 *list, void *, u32 failed) { Finish(list, failed, 0x26, 0x006fd508); }
void __fastcall ListRequest_Done34(u8 *list, void *, u32 failed) { Finish(list, failed, 0x34, 0x006fd510); }

void __fastcall ListRequest_Start30(u8 *list, void *, u32 callback, u32 user) {
    Reset(list, callback, user, 0x30, 0x006fd500);
    Start(list, &ListRequest_Done30, 0x005bc710, reinterpret_cast<u32>(list));
}

void __fastcall ListRequest_Start66(u8 *list, void *, u32 value, u32 callback, u32 user) {
    at<u32>(list, 0x34) = value;
    Reset(list, callback, user, 0x66, 0x006fd504);
    Start(list, &ListRequest_Done66, 0x005bca70, reinterpret_cast<u32>(list));
}

void __fastcall ListRequest_StartText66(u8 *list, void *, u32 text, u32 callback, u32 user) {
    vcall<void>(list + 0x34, 0x14, text);
    Reset(list, callback, user, 0x66, 0x006fd504);
    Start(list, &ListRequest_Done66, 0x005bca70, reinterpret_cast<u32>(list));
}

void __fastcall ListRequest_Start10(u8 *list) {
    Reset(list, 0, 0, 0x10, 0x006fd50c);
    StartMember(list, &ListRequest_Done10, 0xf8, 0x005bcfb0);
}

void __fastcall ListRequest_Start26(u8 *list) {
    Reset(list, 0, 0, 0x26, 0x006fd508);
    StartMember(list, &ListRequest_Done26, 0xfc, 0x005bd3f0);
}

void __fastcall ListRequest_Start34(u8 *list) {
    Reset(list, 0, 0, 0x34, 0x006fd510);
    StartMember(list, &ListRequest_Done34, 0x100, 0x005bd800);
}

void __cdecl ListRequest_Login30(u32 flag, u8 *list) { Login(flag, list, &ListRequest_Done30); }
void __cdecl ListRequest_Login66(u32 flag, u8 *list) { Login(flag, list, &ListRequest_Done66); }
void __cdecl ListRequest_Login10(u32 flag) { Login(flag, at<u8 *>(Online_Instance(), 0xf8), &ListRequest_Done10); }
void __cdecl ListRequest_Login26(u32 flag) { Login(flag, at<u8 *>(Online_Instance(), 0xfc), &ListRequest_Done26); }
void __cdecl ListRequest_Login34(u32 flag) { Login(flag, at<u8 *>(Online_Instance(), 0x100), &ListRequest_Done34); }

u32 __cdecl Online_Has108NoFlag10() {
    if (!at<u32>(Online_Instance(), 0x108))
        return 0;
    return Online_UserFlags(Online_Instance()) & 0x10 ? 0 : 1;
}

}  // extern "C"
