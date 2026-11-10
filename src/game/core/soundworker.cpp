#include "soundworker.h"

#include <cstdint>

#include "channelvoice.h"
#include "gameheap.h"
#include "methods.h"
#include "soundvoice.h"
#include "vcall.h"
#include "imports.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

using OnObject = void(__stdcall *)(void *);
OnObject import(u32 slot) { return global<OnObject>(slot); }
void Enter(void *section) { import(IMPORT(Kernel32_EnterCriticalSection))(section); }
void Leave(void *section) { import(IMPORT(Kernel32_LeaveCriticalSection))(section); }

const SoundWorkerEnv kGameEnv = {reinterpret_cast<u32(__cdecl *)(u32, u32, void *)>((uintptr_t)0x0067e050)};
const SoundWorkerEnv *g_env = &kGameEnv;

}  // namespace

extern "C" {

void SoundWorker_SetEnv(const SoundWorkerEnv *env) { g_env = env ? env : &kGameEnv; }

void __fastcall SoundBuffer_FillFromVoice(u8 *holder, void *, u8 *voice, u32 offset, u32 size) {
    u8 *buffer = at<u8 *>(holder, 0);
    if (!buffer) return;
    u8 *p1 = reinterpret_cast<u8 *>((uintptr_t)offset);
    u32 n1 = size, n2, p2;
    if (comcall<s32>(buffer, 0x2c, offset, size, &p1, &n1, &p2, &n2, 0u) < 0) return;
    const u32 value = at<u32>(voice, 0x2c) >> (global<u8>(0x00782db8) & 31);
    const bool done = ChannelVoice_Mix(voice, nullptr, p1, n1) == 0;
    buffer = at<u8 *>(holder, 0);
    comcall<u32>(buffer, 0x30, p1, n1, n1, 0u);
    if (done) vcall<void>(global<u8 *>(0x00fb2724), 0x28, &voice, value);
}

void __fastcall SoundWorker_RefillVoice(u8 *owner, void *, u32 position) {
    const u32 slot = position % at<u32>(owner, 4);
    u8 *const voice = at<u8 *>(at<u8 *>(owner, 0x34), 4) + slot * 0x8c;
    if (!voice) return;
    const u32 value = Obj_Get20Or2400(voice);
    const u32 offset = position < at<u32>(owner, 4) ? value : 0;
    Enter(owner + 0x38);
    SoundBuffer_FillFromVoice(at<u8 *>(owner, 0x28) + slot * 0x2c, nullptr, voice, offset, value);
    Leave(owner + 0x38);
}

void __fastcall ComList_ReleaseAll(u8 *list) {
    Enter(list + 0x10);
    u8 *node = at<u8 *>(list, 4);
    while (node) {
        u8 *const object = at<u8 *>(node, 0);
        u8 *const next = at<u8 *>(node, 4);
        comcall<u32>(object, 0x18);
        comcall<u32>(object, 8);
        if (node == at<u8 *>(list, 4)) at<u8 *>(list, 4) = at<u8 *>(node, 4);
        else at<u8 *>(at<u8 *>(node, 8), 4) = at<u8 *>(node, 4);
        if (node == at<u8 *>(list, 8)) at<u8 *>(list, 8) = at<u8 *>(node, 8);
        else at<u8 *>(at<u8 *>(node, 4), 8) = at<u8 *>(node, 8);
        --at<u32>(list, 0xc);
        GameHeap_Free(node);
        node = next;
    }
    Leave(list + 0x10);
}

u8 *__fastcall SoundWorker_Ctor(u8 *worker, void *, u32 count) {
    import(IMPORT(Kernel32_InitializeCriticalSection))(worker + 4);   // InitializeCriticalSection
    worker[0x34] = 0;
    for (u32 offset = 0x1c; offset <= 0x30; offset += 4) at<u32>(worker, offset) = 0;
    worker[0x35] = 0;
    at<u32>(worker, 0x38) = 0;
    Voices70a4f0_New(worker + 0x3c, nullptr, 30);
    at<void *>(worker, 0x40) = GameHeap_Alloc(0x78);
    at<void *>(worker, 0x44) = GameHeap_Alloc(0x7c);
    at<u32>(worker, 0x48) = 0;
    at<u32>(worker, 0x4c) = count < 30 ? count : 30;
    worker[0x50] = 1;
    at<u32>(worker, 0) = (u32)(uintptr_t)SoundWorker_vtable;
    for (u32 i = 0; i < 30; ++i) at<u32>(at<u8 *>(worker, 0x40), i * 4) = 0;
    g_env->beginThread(0x006b6040, 0, worker);
    return worker;
}

u8 *__fastcall SoundWorkerHost_Ctor(u8 *host) {
    for (u32 offset = 4; offset <= 0x18; offset += 4) at<u32>(host, offset) = 0;
    at<u32>(host, 0x1c) = 12;
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x54));
    at<u8 *>(host, 0x20) = block ? SoundWorker_Ctor(block, nullptr, at<u32>(host, 0x1c)) : nullptr;
    for (u32 offset = 0x24; offset <= 0x30; offset += 4) at<u32>(host, offset) = 0;
    at<u32>(host, 0) = (u32)(uintptr_t)SoundWorkerHost_vtable;
    return host;
}
}
