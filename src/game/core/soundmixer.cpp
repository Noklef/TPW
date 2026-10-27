#include "soundmixer.h"

#include <cstdint>

#include "../files/file.h"
#include "fieldops.h"
#include "gameheap.h"
#include "selectors.h"
#include "soundvoice.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename F> F Import(u32 slot) { return *reinterpret_cast<const F *>((uintptr_t)slot); }

using OnObject = u32(__stdcall *)(void *);

void Lock(u8 *mixer) { Import<OnObject>(0x006fd140)(mixer + 4); }     // EnterCriticalSection
void Unlock(u8 *mixer) { Import<OnObject>(0x006fd130)(mixer + 4); }   // LeaveCriticalSection

}  // namespace

extern "C" {

void __fastcall SoundMixer_Update(u8 *mixer) {
    SoundMixer_ClearLevels(mixer);
    u8 *voice = at<u8 *>(mixer, 0x24);
    const u32 now = Time_GetMs();
    while (voice) {
        u8 *const next = at<u8 *>(voice, 0x34);
        if (voice[0x30] & 0x10 || Obj_PastEnd(voice, nullptr, now)) {
            SoundMixer_Retire(mixer, nullptr, voice);
        } else if (at<u32>(voice, 0x30) & 0x100) {
            u32 out;
            u32 functor[2] = {0x0070a2d0, at<u32>(voice, 0x24)};
            vcall<void>(voice, 0x18, &out, functor);
        } else if (!(voice[0x30] & 0x40) && SoundVoice_CategoryEnabled(voice)) {
            if (mixer[0x50] && at<u32>(voice, 0x30) & 0x400) vcall<void>(voice, 0x2c);
            if (SoundVoice_UpdateAudible(voice)) Top_Insert(mixer, nullptr, voice);
        }
        voice = next;
    }
    for (u32 i = 0; at<u8 **>(mixer, 0x44)[i]; ++i) {
        u32 out;
        vcall<void>(at<u8 **>(mixer, 0x44)[i], 0x14, &out, now);
    }
}

void __fastcall SoundMixer_Dtor(u8 *mixer) {
    at<u32>(mixer, 0) = 0x0070a2cc;
    mixer[0x35] = 1;
    Import<u32(__stdcall *)(void *, u32)>(0x006fd200)(mixer + 0x38, 1);   // InterlockedExchange
    while (!*reinterpret_cast<volatile u8 *>(mixer + 0x34)) {
    }
    for (u8 *voice = at<u8 *>(mixer, 0x24); voice; voice = at<u8 *>(voice, 0x34)) vcall<void>(voice, 0xc);
    while (u8 *const page = at<u8 *>(mixer, 0x1c)) {
        at<u8 *>(mixer, 0x1c) = at<u8 *>(page, 8);
        GameHeap_Free(page);
    }
    GameHeap_Free(at<void *>(mixer, 0x44));
    GameHeap_Free(at<void *>(mixer, 0x40));
    if (u8 *const object = at<u8 *>(mixer, 0x3c)) vcall<void>(object, 8, 3u);
    Import<OnObject>(0x006fd0d0)(mixer + 4);   // DeleteCriticalSection
}

u8 *__fastcall SoundMixer_DeletingDtor(u8 *mixer, void *, u32 flags) {
    SoundMixer_Dtor(mixer);
    if (flags & 1) GameHeap_Free(mixer);
    return mixer;
}

u32 __fastcall SoundMixer_TouchFlagged(u8 *mixer) {
    u32 count = 0;
    for (u8 *voice = at<u8 *>(mixer, 0x24); voice; voice = at<u8 *>(voice, 0x34)) {
        if (!(at<u32>(voice, 0x30) & 0x400)) continue;
        Import<OnObject>(0x006fd0c0)(&count);   // InterlockedIncrement
        vcall<void>(voice, 0x2c);
    }
    return count;
}

void __fastcall SoundMixer_Retire(u8 *mixer, void *, u8 *voice) {
    Lock(mixer);
    vcall<void>(voice, 0xc);
    Chain24_MoveTo20(mixer, nullptr, voice);
    Unlock(mixer);
}

void __fastcall SoundMixer_ForEachActive(u8 *mixer, void *, u32 arg) {
    const u32 value = arg;   // (the slot 6 calls write their out through the argument's own slot, sic; the test keeps the value)
    for (u8 *voice = at<u8 *>(mixer, 0x24); voice; voice = at<u8 *>(voice, 0x34)) {
        if (value) {
            vcall<void>(voice, 0x30, value);
            continue;
        }
        u32 functor[2] = {0x0070a2d0, at<u32>(voice, 0x24)};
        vcall<void>(voice, 0x18, &arg, functor);
    }
}

u8 *__fastcall SoundMixer_FindVoice(u8 *mixer, void *, u32 id) {
    if (!id) return nullptr;
    Lock(mixer);
    u8 *const voice = Pages100_EntryAt(mixer, nullptr, id);
    if (voice && at<u32>(voice, 0x2c) == id) {
        Unlock(mixer);
        return voice;
    }
    Unlock(mixer);
    return nullptr;
}

u32 __fastcall SoundMixer_StampVoice(u8 *mixer, void *, u8 *voice) {
    const u32 id = at<u32>(voice, 0x2c);
    const u32 stamped = (Obj_Inc30(mixer) << 16) + id;
    at<u32>(voice, 0x2c) = stamped;
    return stamped;
}

s32 *__cdecl VoicePage_New(s32 *status, u8 **out, u32 count) {
    if (out) {
        const u32 size = count * 0x50;
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(size + 0xc));
        if (block) {
            at<u32>(block, 4) = size;
            at<u32>(block, 8) = 0;
            at<u32>(block, 0) = 0x0070a2d8;
            *out = block;
            *status = 0;
            return status;
        }
    }
    *status = -1;
    return status;
}

u32 __fastcall SoundMixer_ClearLevels(u8 *mixer) {
    u32 *const levels = at<u32 *>(mixer, 0x44);
    for (u32 i = 0; i < 0x1f; ++i) levels[i] = 0;
    at<u32>(mixer, 0x48) = 0;
    return 0;
}

u32 *__fastcall SoundMixer_SetVoiceParams(u8 *mixer, void *, u32 *out, u32 id, const u32 *params) {
    u8 *const voice = SoundMixer_FindVoice(mixer, nullptr, id);
    if (!voice) {
        *out = 0xffffffff;
        return out;
    }
    SoundVoice_EnsureParams(voice);
    u32 *const to = reinterpret_cast<u32 *>(at<u8 *>(voice, 0x4c) + 8);
    for (u32 i = 0; i < 4; ++i) to[i] = params[i];
    at<u32>(voice, 0x30) |= 0x1000;
    *out = 0;
    return out;
}

u32 *__fastcall SoundMixer_SetVoicePending(u8 *mixer, void *, u32 *out, u32 id, u32 value) {
    u8 *const voice = SoundMixer_FindVoice(mixer, nullptr, id);
    if (!voice) {
        *out = 0xffffffff;
        return out;
    }
    SoundVoice_SetPending(voice, nullptr, out, value);
    return out;
}

u32 *__fastcall SoundMixer_SendVoice(u8 *mixer, void *, u32 *out, u32 id, u32 value) {
    u8 *const voice = SoundMixer_FindVoice(mixer, nullptr, id);
    if (!voice) {
        *out = 0xffffffff;
        return out;
    }
    vcall<void>(voice, 0x30, value);
    *out = 0;
    return out;
}

u32 __fastcall SoundMixer_CallVoice10(u8 *mixer, void *, u32 id, u32 a, u32 b) {
    u8 *const voice = SoundMixer_FindVoice(mixer, nullptr, id);
    if (!voice) return 0;
    vcall<void>(voice, 0x28, a, b);
    return id;
}

u32 __fastcall SoundMixer_CallVoice9(u8 *mixer, void *, u32 id, u32 value) {
    u8 *const voice = SoundMixer_FindVoice(mixer, nullptr, id);
    if (!voice) return 0;
    u32 local = id;   // (the out: the id's own stack slot, sic)
    const s32 *const status = vcall<const s32 *>(voice, 0x24, &local, value);
    return *status >= 0 ? id : 0;
}

u32 *__fastcall SoundMixer_AddPage(u8 *mixer, void *, u32 *out) {
    u8 *page;
    s32 status;
    if (*VoicePage_New(&status, &page, 100) < 0) {
        *out = 0xffffffffu;
        return out;
    }
    if (u8 *last = at<u8 *>(mixer, 0x1c)) {
        while (at<u8 *>(last, 8)) last = at<u8 *>(last, 8);
        at<u8 *>(last, 8) = page;
    } else {
        at<u8 *>(mixer, 0x1c) = page;
    }
    u8 *voice = page + 0xc;
    for (int i = 0; i < 100; ++i, voice += 0x50) {
        at<u32>(voice, 0x2c) = at<u32>(mixer, 0x2c)++;
        at<u32>(voice, 0x30) = 0;
        Chain20_PushFront(mixer, nullptr, voice);
    }
    ++at<u32>(mixer, 0x28);
    *out = 0;
    return out;
}

u32 __fastcall SoundMixer_AddVoice(u8 *mixer, void *, u8 *owner, u8 *definition, u8 *info) {
    const u32 count = at<u32>(mixer, 0x4c);
    u32 slot = 0;
    if (count) {
        const u32 *const slots = at<const u32 *>(mixer, 0x40);
        while (slots[slot] && ++slot < count) {
        }
    }
    if (slot >= count) return 0;
    u8 *const memory = at<u8 *>(mixer, 0x3c) + slot * 0x50;
    u8 *const voice = memory ? Voice70a4f0_Ctor(memory) : nullptr;
    u32 *const out = reinterpret_cast<u32 *>(&info);   // (sic: the info argument's own slot)
    if (static_cast<s32>(*vcall<u32 *>(voice, 0x10, out, owner, definition, info + 0xc, at<u32>(info, 0x18))) < 0) return 0;
    at<u8 **>(mixer, 0x40)[slot] = voice;
    return 1;
}

}  // extern "C"
