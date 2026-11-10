#include "advisor.h"

#include <cstdint>

#include "../core/addrcalls.h"
#include "../core/crt.h"
#include "../core/gameclock.h"
#include "../core/indexed.h"
#include "../core/keymap.h"
#include "../core/savegame.h"
#include "../core/soundplay.h"
#include "../core/soundset.h"
#include "../core/timesig.h"
#include "../core/selectors.h"
#include "../model/parts.h"
#include "../model/slots.h"
#include "../motion/animator.h"
#include "../render/particles.h"
#include "advisorscore.h"
#include "messagecentre.h"
#include "objectdefs.h"
#include "things.h"
#include "world.h"
#include "advisor.h"
#include "../core/staticobjects.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// A message's 6 dwords copied (forward, as REP MOVSD).
void copy_message(u8 *to, const u8 *from) {
    for (u32 i = 0; i < 0x18; i += 4)
        at<u32>(to, i) = at<u32>(from, i);
}

// An advisor message (b -1, flag 0, no score; its bytes +2, +3 and +0x17 never written, sic) for id posted.
void post(u8 *self, u32 id, u16 type, u32 a) {
    u8 message[0x18];
    AdvisorMsg_Init(message, nullptr, id, type, a, 0xffffffffu, 0, -1);
    Advisor_Post(self, nullptr, message);
}

u32 event_code(u8 *event) {
    using Code = u32(__fastcall *)(u8 *, void *);
    return reinterpret_cast<Code>((uintptr_t)at<u32>(at<u8 *>(event, 0), 0))(event, nullptr);
}

AddrCalls g_calls;

// The speech's model, particles and text ended, and the voice's pause undone: the part Advisor_StopSpeech and Advisor_EndSpeech share.
void end_speech_parts() {
    Sound_SetVoicePending(global<u32>(0x00f79680), 0);
    if (global<u32>(0x00f797c4) && global<u8>((u32)(uintptr_t)&gOptions_78d8d8 + 0x34) && global<u32>(0x00f79680)) {
        Sound_VoiceFunctor3(global<u32>(0x00f79680));
        global<u32>(0x00f797c4) = 0;
    }
    global<u32>(0x00f7966c) = 0;
    Sound_Set803acc(0);
    ParticlePool_Resize(0, 0, 0);
    ModelSlot_KillAdvisor(global<u32>(0x00f79674));
    if (u8 *const text = global<u8 *>(0x00f797a0)) {
        if (text != reinterpret_cast<u8 *>(0x00f797d0))
            Crt().free(text);
        else
            global<u32>(0x00f79bb8) = 0;
        global<u32>(0x00f797a0) = 0;
        global<u32>(0x00f79794) = 0;
        global<u32>(0x00f7979c) = 0;
    }
}

// The next line of a slot's message when the slot names none: by the definition's kind (DefField2c), from the last line played
// (+0xe4 of the id's history record), n lines (DefField28) in all. Kind 0 wraps, 1 stays on the last, 2 just counts on, 3 is random (n 0 divides by zero, sic).
// A kind above 3, and kind 0 at the last line, leave the original with its uninitialised local (the slot of the out dword): 0 here, which the later bound check gives for nearly every value it can hold.
s32 next_line(const u8 *self, u32 scoring, u32 id) {
    const u32 n = g_calls.Th(0x0059c2b0, scoring, id);
    const u32 kind = g_calls.Th(0x0059c490, scoring, id);
    const s32 &last = at<s32>(self, 0xe4 + (id << 4));
    s32 next = 0;
    switch (kind) {
    case 0:
        if (last != (s32)(n - 1)) next = last + 1;
        break;
    case 1:
        next = last >= (s32)(n - 1) ? last : last + 1;
        break;
    case 2:
        next = last + 1;
        break;
    case 3:
        next = (s32)(g_calls.Th(0x00516330, global<u32>(0x0080239c)) % n);
        break;
    default:
        break;
    }
    if (next < 0) return 0;
    return next < (s32)g_calls.Th(0x0059c2b0, scoring, id) ? next : 0;
}

}  // namespace

void Advisor_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u32 __cdecl Table768fb8_Lookup(u32 id) {
    for (u32 entry = 0x00768fb8; global<u32>(entry) != 0x270f; entry += 0x20) {
        if (global<u32>(entry) == id)
            return global<u32>(entry + 0x1c);
    }
    return 0x17f;
}

void __cdecl Advisor_Init() {
    for (u32 offset = 0; offset < 0x168; offset += 4)
        global<u32>(0x00f79660 + offset) = 0;
    global<s32>(0x00f79770) = 5;
    global<u32>(0x00f7978c) = 0xffffffff;
    for (u32 slot = 0; slot < 2; slot++) {
        const u8 *holder = ModelSlot_Holder(slot);
        if (!holder)
            continue;
        for (s32 j = 0; j < global<s32>(0x00f79770); j++)
            global<u32>(0x00f796a0 + 0x14 * slot + 4 * j) = Model_FindMouthPart(holder, 0, j + 1);
        global<u32>(0x00f79698 + 4 * slot) = Model_FindMouthPart(holder, 0, -1);
    }
    const u8 *holder = ModelSlot_Holder(1);
    if (!holder)
        holder = ModelSlot_Holder(0);
    if (!holder)
        return;
    for (u32 k = 0; k < 20; k++)
        global<u32>(0x00f796c8 + 4 * k) = Animator_MotionEndMs(holder, 5, k);
    global<u32>(0x00f796fc) = 0;
    ParticlePools_Reset();
}

u8 *__fastcall Advisor_Ctor(u8 *advisor) {
    Thing_Construct(advisor, nullptr, 1, 0xb);
    auto dword = [advisor](u32 offset) -> u32 & { return *reinterpret_cast<u32 *>(advisor + offset); };
    dword(0xc) = 0;
    dword(0x10) = 0;
    for (u32 k = 0; k < 8; ++k)
        advisor[0x28 + 0x18 * k] = 0;
    dword(0xd4) = 0;
    dword(0xdc) = 0;
    for (u32 k = 0; k < 0x15f; ++k) {
        const u32 at = 0xe0 + 0x10 * k;
        dword(at) = 0;
        dword(at + 4) = 0xffffffffu;
        advisor[at + 8] = 0;
        dword(at + 0xc) = 0;
    }
    advisor[0x16e4] = 0;
    static const s32 kSlots[] = {5, 0x11, 0x13, 0x17, 0xe, 8, 0x19};
    for (s32 slot : kSlots)
        MessageCentre_AddThing(MessageCentre_Instance(), nullptr, advisor, slot);
    dword(0x10) = 0;
    dword(0xd4) = 0;
    dword(0xdc) = 0;
    dword(0xd8) = 0;
    return advisor;
}

u8 __fastcall Advisor_MayPost(u8 *self, void *, u32 id, u8 urgent) {
    if (!global<u8>((u32)(uintptr_t)&gOptions_78d8d8 + 0x35)) {
        AdvisorScoring_Instance();
        if (Record_IsActive(id))
            return 0;
    }
    const u8 *const record = self + (id + 0xe) * 0x10;
    if (Obj_Get0Shr2(record)) {
        const u32 wait = Table76e300_GetObjectDword(AdvisorScoring_Instance(), nullptr, id);
        if (Game1da70c_MinusAt0Quarters(reinterpret_cast<const u32 *>(record)) < wait)
            return 0;
    }
    if (!urgent) {
        if (Table76e300_ObjectDwordSet(AdvisorScoring_Instance(), nullptr, id) && at<u8>(self, 0x10 * id + 0xe8))
            return 0;
        if (Table76e300_LimitCheck(AdvisorScoring_Instance(), nullptr, id, at<s32>(self, 0x10 * id + 0xec)))
            return 0;
    }
    s32 queued = 0;
    for (u32 i = 0; i < 8; ++i) {
        if (at<u8>(self, 0x28 + 0x18 * i) && at<u32>(self, 0x20 + 0x18 * i) == id)
            ++queued;
    }
    AdvisorScoring_Instance();
    if (queued >= (s32)Table76e300_GetByte(id))
        return 0;
    AdvisorScoring_Instance();
    if (!DefField2c_59c490(id)) {
        AdvisorScoring_Instance();
        if (at<u32>(self, 0x10 * id + 0xe4) == DefField28_59c2b0(id) - 1)
            return 0;
    }
    return 1;
}

u8 __fastcall Advisor_Post(u8 *self, void *, const u8 *message) {
    const u32 id = at<u32>(message, 0xc);
    AdvisorScoring_Instance();
    if (DefField8_59c500(id) == 1)
        copy_message(self + 0x16d0, message);
    if (!Advisor_MayPost(self, nullptr, at<u32>(message, 0xc), at<u8>(message, 0x15)))
        return 0;
    for (u32 i = 0; i < 8; ++i) {
        if (!at<u8>(self, 0x28 + 0x18 * i)) {
            copy_message(self + 0x14 + 0x18 * i, message);
            return 1;
        }
    }
    s32 lowest = -1, score = 0;
    for (s32 i = 0; i < 8; ++i) {
        if (!at<u8>(self, 0x28 + 0x18 * i))
            continue;
        const s32 s = at<s32>(self, 0x24 + 0x18 * i);
        if (lowest < 0 || s < score) {
            lowest = i;
            score = s;
        }
    }
    if (lowest < 0)
        return 0;
    u8 *const slot = self + 0x14 + 0x18 * lowest;
    const s32 held = at<u8>(slot, 0x14) ? at<s32>(slot, 0x10) : -1;
    const s32 offered = at<u8>(message, 0x14) ? at<s32>(message, 0x10) : -1;
    if (held < offered)
        copy_message(slot, message);
    return 1;
}

u8 __fastcall Advisor_PostKindEvent(u8 *self, void *, const u8 *event) {
    const u32 n = at<u32>(event, 0xc);
    u32 id;
    switch (at<u32>(event, 8)) {
    case 0: id = n * 5 + 0x22; break;
    case 1: id = n * 5 + 0x21; break;
    case 2: id = n * 5 + 0x24; break;
    case 3: id = (n + 7) * 5; break;
    case 4: id = n * 5 + 0x25; break;
    default: id = (u32)(uintptr_t)event; break;   // (sic: the stack slot read there is the argument's)
    }
    u8 message[0x18];   // (its bytes +2, +3 and +0x17 never written, sic)
    AdvisorMsg_Init(message, nullptr, id, 0, 0, 0xffffffffu, 0, -1);
    return Advisor_Post(self, nullptr, message);
}

void __fastcall Advisor_HandleCode(u8 *self, void *, u32 code) {
    switch (code) {
    case 0:
        post(self, 0, 0, 0);
        if (AdvisorScore_ModeSetting(0x00fb2765) == 2) post(self, 0x143, 0, 0);
        break;
    case 2: post(self, 0x6a, 0, 0); break;
    case 3: post(self, 0x80, 0, 0); break;
    case 4: post(self, 0x81, 0, 0); break;
    case 10: {
        using Output = void(__stdcall *)(const char *);
        (*reinterpret_cast<const Output *>((uintptr_t)0x006fd1e0))(gText_Advisor_HandleCode_76e190);
        for (u32 i = 0; i < 0x15f; ++i) {
            u8 *const record = self + 0xe0 + 0x10 * i;
            at<u32>(record, 0xc) = 0;
            at<s32>(record, 4) = -1;
            at<u8>(record, 8) = 0;
        }
        break;
    }
    default: break;
    }
}

void __fastcall Advisor_HandleMessage(u8 *self, void *, u8 *event) {
    switch (event_code(event)) {
    case 5:
        if (at<u8>(event, 0x1c)) Advisor_Post(self, nullptr, event + 8);
        break;
    case 8: post(self, 0x102, at<u16>(event, 8), 0); break;
    case 14: {
        const u16 w = at<u16>(event, 8);
        const u8 *const entry = global<const u8 *>(0x007cfb90 + 20 * (u32)w);
        const u16 a = (u16)((at<u8>(entry, 7) << 7) + at<u8>(entry, 5) + 1);
        static const u16 kIds[] = {0x7d, 0x7e, 0x7f};
        const u32 kind = at<u32>(event, 0xc);
        if (kind < 3) post(self, kIds[kind], w, a);
        break;
    }
    case 17: {
        static const u16 kIds[] = {0x5d, 0x5f, 0x60, 0x61, 0x5e};
        const u32 kind = at<u32>(event, 0xc);
        if (kind < 5) post(self, kIds[kind], 0, at<u16>(event, 8));
        break;
    }
    case 19: Advisor_HandleCode(self, nullptr, at<u32>(event, 8)); break;
    case 23: Advisor_PostKindEvent(self, nullptr, event); break;
    case 25:
        if (at<u32>(event, 0xc) != 0x160) post(self, at<u32>(event, 0xc), 0, 0);
        break;
    default: event_code(event); break;   // (for Debug_Printf, empty)
    }
}

u8 __cdecl Advisor_PostId(u32 id) {
    u8 message[0x18];   // (its bytes +2, +3 and +0x17 never written, sic)
    AdvisorMsg_Init(message, nullptr, id, 0, 0, 0xffffffffu, 0, -1);
    u8 *const advisor = reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da71c_5194d0(global<u8 *>(0x007cf83c)));
    return Advisor_Post(advisor, nullptr, message);
}

u32 __fastcall Advisor_Serialize(u8 *self, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    if (Thing_Serialize(self, nullptr, stream, save, version, unused) != 0) return 1;
    if (version < 0 || version > 1) return 0;
    if (TimeSig_Serialize(self + 0xc, nullptr, stream, save, version, unused) != 0) return 1;
    if (!Save_Field(stream, save, self + 0x10, 4, (u32)&gText_Advisor_Serialize_76e184)) return 1;   // mSoundId
    for (u32 i = 0; i < 8; ++i)
        if (AdvisorMsg_Serialize(self + 0x14 + 0x18 * i, nullptr, stream, save, version, unused) != 0) return 1;
    if (!Save_Field(stream, save, self + 0xd4, 4, (u32)&gText_Advisor_Serialize_76e16c)) return 1;   // mNextAvailableQueuePos
    if (!Save_Field(stream, save, self + 0xd8, 4, (u32)&gText_Advisor_Serialize_76e158)) return 1;   // mLastActionStarted
    if (!Save_Field(stream, save, self + 0xdc, 4, (u32)&gText_Advisor_Serialize_76e144)) return 1;   // mLastActionDuration
    for (u32 i = 0; i < 0x15f; ++i)
        if (!Save_Field(stream, save, self + 0xe0 + 0x10 * i, 0x10, (u32)&gText_Advisor_Serialize_76e138)) return 1;   // mHistory[i]
    if (AdvisorMsg_Serialize(self + 0x16d0, nullptr, stream, save, version, unused) != 0) return 1;
    if (!Save_Field(stream, save, self + 0x16e8, 1, (u32)&gText_Advisor_Serialize_76e120)) return 1;   // mRequestedLastTutorial
    if (save != 0) return Save_Field(stream, save, self + 0x16ec, 4, (u32)&gText_Advisor_Serialize_76e110) ? 0 : 1;
    return Save_ReadU32(stream, self + 0x16ec, gText_Advisor_Serialize_76e110, 0) != 0 ? 1 : 0;
}

void __cdecl Advisor_PauseSpeech() {
    if (!global<u32>(0x00f7966c) || !global<u8>((u32)(uintptr_t)&gOptions_78d8d8 + 0x34) || !global<u32>(0x00f79680)) return;
    Sound_SendVoiceFunctor2(global<u32>(0x00f79680));
    global<u32>(0x00f797c4) = 1;
}

void __cdecl Advisor_ResumeSpeech() {
    if (!global<u32>(0x00f797c4) || !global<u8>((u32)(uintptr_t)&gOptions_78d8d8 + 0x34) || !global<u32>(0x00f79680)) return;
    Sound_VoiceFunctor3(global<u32>(0x00f79680));
    global<u32>(0x00f797c4) = 0;
}

void __cdecl Advisor_ClearSpeechText() {
    global<u32>(0x00f7966c) = 0;
    u8 *const text = global<u8 *>(0x00f797a0);
    if (!text) return;
    if (text != reinterpret_cast<u8 *>(0x00f797d0))
        Crt().free(text);
    else
        global<u32>(0x00f79bb8) = 0;
    global<u32>(0x00f797a0) = 0;
}

void __cdecl Advisor_EndSpeech(u32 stop) {
    if (!global<u32>(0x00f7966c)) return;
    end_speech_parts();
    if (!global<u32>(0x00f79680)) return;
    if (stop)
        Sound_Stop(global<u32>(0x00f79680));
    else
        Sound_Forward6(global<u32>(0x00f79680));
    global<u32>(0x00f79680) = 0;
}

void __fastcall Advisor_Update(u8 *self) {
    const u32 scoring = (u32)(uintptr_t)AdvisorScoring_Instance();
    const s32 &floor = at<s32>(reinterpret_cast<void *>((uintptr_t)scoring), 0x20);
    const u32 me = (u32)(uintptr_t)self;
    const u32 picked = g_calls.Th(0x0059c680, scoring, 1u);
    const s32 score = (s32)g_calls.Th(0x0059bf20, scoring, picked);
    if (score > floor) {
        u8 message[0x18];
        g_calls.Th(0x0059a940, me, g_calls.Th(0x0059b590, (u32)(uintptr_t)message, picked, 0u, 0u, 0xffffffffu, 0u, 0xffffffffu));
    }
    if (g_calls.Th(0x00402d80, (u32)(uintptr_t)&gObj_785970) < at<u32>(self, 0xd8) + at<u32>(self, 0xdc)) return;
    if (at<u8>(self, 0x16e8) == 1) {
        at<u8>(self, 0x16e8) = 0;
        return;
    }
    s32 index = -1, best = 0;
    for (s32 k = 0; k < 8; ++k) {
        const u8 *const slot = self + 0x14 + 0x18 * k;
        if (at<u8>(slot, 0x14) && (index < 0 || at<s32>(slot, 0x10) > best)) {
            index = k;
            best = at<s32>(slot, 0x10);
        }
    }
    if (index < 0 || best <= floor) {
        g_calls.Th(0x0041a990, me + 0xc);
        return;
    }
    u8 *const slot = self + 0x14 + 0x18 * index;
    s32 next = at<s32>(slot, 8);
    if (next < 0) next = next_line(self, scoring, at<u32>(slot, 0xc));
    u32 out;
    if (!g_calls.Th(at<u8>(slot, 0x16) ? 0x0059b870 : 0x0059b620, (u32)(uintptr_t)slot, &out, next)) return;
    const u32 length = out + 1000;
    at<u32>(self, 0xd8) = g_calls.Th(0x00402d80, (u32)(uintptr_t)&gObj_785970);
    at<u32>(self, 0xdc) = length;
    const u32 id = at<u32>(slot, 0xc);
    g_calls.Th(0x0041a960, me + 0xe0 + (id << 4));
    at<s32>(self, 0xe4 + (id << 4)) = next;
    at<u8>(self, 0xe8 + (id << 4)) = 1;
    at<u32>(self, 0x16ec) = id;
}

u8 __fastcall Advisor_PostHeld(u8 *self) {
    if (!at<u8>(self, 0x16e4)) return 0;
    const u8 *const scoring = AdvisorScoring_Instance();
    u8 *const held = self + 0x16d0;
    at<u32>(held, 0x10) = at<u32>(scoring, 0x32c);
    at<u8>(held, 0x16) = 1;
    at<u8>(self, 0x16e5) = 1;
    g_calls.Th(0x0059a940, (u32)(uintptr_t)self, (u32)(uintptr_t)held);
    return 1;
}

void __cdecl Advisor_StopSpeech() {
    if (!global<u32>(0x00f7966c)) return;
    const u32 had = global<u32>(0x00f79680);
    end_speech_parts();
    if (global<u32>(0x00f79680)) {
        Sound_Forward6(global<u32>(0x00f79680));
        global<u32>(0x00f79680) = 0;
    }
    if (!had) return;
    const s32 k = Crt().rand() % 3;   // (each tested in turn; the voice re-read, 0 by now)
    if (k == 0) Sound_PlayAt(global<u32>(0x00f79680), global<u32>(0x00803a34), 0x27f, 0, 0, 0);
    if (k == 1) Sound_PlayAt(global<u32>(0x00f79680), global<u32>(0x00803a34), 0x280, 0, 0, 0);
    if (k == 2) Sound_PlayAt(global<u32>(0x00f79680), global<u32>(0x00803a34), 0x281, 0, 0, 0);
}

u8 __fastcall Advisor_Cancel(u8 *self, void *, u32 id) {
    u8 found = 0;
    if (at<u8>(self, 0x16e4)) {
        AdvisorScoring_Instance();
        if (DefField8_59c500(id) == 1 && at<u32>(self, 0x16dc) == id) at<u8>(self, 0x16e4) = 0;
    }
    for (u32 i = 0; i < 8; ++i) {
        u8 *const slot = self + 0x14 + 0x18 * i;
        if (at<u8>(slot, 0x14) && at<u32>(slot, 0xc) == id) {
            at<u8>(slot, 0x14) = 0;
            found = 1;
        }
    }
    const u32 now = Clocks_RealNow(reinterpret_cast<u8 *>((u32)(uintptr_t)&gObj_785970));
    const u32 speaking = now < at<u32>(self, 0xd8) + at<u32>(self, 0xdc) ? at<u32>(self, 0x16ec) : 0x160;
    if (speaking == id) {
        found = 1;
        Advisor_StopSpeech();
        u8 *const record = self + (id << 4);
        at<u8>(record, 0xe8) = 0;
        ++at<u32>(record, 0xec);
        at<u32>(self, 0xdc) = 0;
    }
    return found;
}

void __cdecl Advisor_UpdateHighlight() {
    const u8 *const holder = ModelSlot_Holder(global<u32>(0x00f79674));
    if (holder == nullptr)
        return;
    const auto mark = [holder](u32 table) {
        const u32 part = global<u32>(table);
        const u8 *const inner = *reinterpret_cast<u8 *const *>(*reinterpret_cast<const u32 *>(holder + 8) + 4);
        *reinterpret_cast<u32 *>(*reinterpret_cast<const u32 *>(inner + 0x70) + part * 0xa0) |= 0x10;
    };
    for (s32 i = 0; i < global<s32>(0x00f79770); i++)
        mark(0x00f796a0 + ((u32)i + global<u32>(0x00f79674) * 5) * 4);
    global<u32>(0x00f7978c) = 0xffffffff;
    ModelSlot_ApplySelection(global<s32>(0x00f79684));
    if (global<s32>(0x00f79688) != 0)
        ModelSlot_ApplySelection(global<s32>(0x00f79688));
    if (global<u32>(0x00f79660) & 0x10000)
        mark(0x00f79698 + global<u32>(0x00f79674) * 4);
    const u32 flags = global<u32>(0x00f79660);
    const u32 arm = flags & 0x3e0000;
    if (arm == 0)
        return;
    if (global<u32>(0x00f7966c) == 0) {
        if (flags & 0x200000)
            ParticlePool_Resize(0, 0, 0);
        return;
    }
    const auto time = []() { return (float)global<s32>(0x00f796c8 + global<u32>(0x00f7976c) * 4); };
    const auto bits = [](u32 value) { return *reinterpret_cast<const float *>(&value); };
    switch (arm) {
    case 0x20000:
        if (ParticlePool_Resize(0, 1, 0x2c) == 0)
            return;
        ParticlePool_Move(0, 1, bits(0x3eae147b), bits(0x3f1eb852), time());
        KeyRecord_Push(0, 1, 0x3c23d70a, 0x3e4ccccd, 0x3f800000);
        break;
    case 0x40000:
        Sound_PlayAt(0, global<u32>(0x00803a2c), 0xc5, 0, 0, 0);
        ParticlePool_Resize(0, 2, 0x31);
        ParticlePool_Move(0, 0, 0.0f, 0.0f, time());
        KeyRecord_Push(0, 0, 0x3f19999a, 0x3ecccccd, 0x40000000);
        break;
    case 0x80000:
        ParticlePool_Resize(0, 3, 0x31);
        ParticlePool_Move(0, 0, 0.0f, 0.0f, time());
        KeyRecord_Push(0, 0, 0x3f19999a, 0x3ecccccd, 0x40400000);
        break;
    case 0x100000:
        ParticlePool_Resize(0, 0, 0x2c);
        ParticlePool_Resize(0, 2, 0x2c);
        ParticlePool_Move(0, 0, bits(0x3f4ccccd), bits(0xbf4ccccd), time());
        KeyRecord_Push(0, 0, 0, 0, 0x40000000);
        break;
    case 0x200000:
        ParticlePool_Resize(0, 0, 0);
        break;
    }
}

void __cdecl Advisor_FrameTick() {
    const u32 holder = g_calls.Cd(0x00429fe0, global<u32>(0x00f79674));   // ModelSlot_Holder
    if (global<u32>(0x00f7968c)) {
        s32 bank = 0, entry;
        g_calls.Cd(0x00473ff0, holder, &bank, &entry, 0u);   // Layer_GetQueued
        if (bank == 0xc) {
            g_calls.Cd(0x00598dc0);   // Advisor_UpdateHighlight
            global<u32>(0x00f7968c) = 0;
            global<u32>(0x00f79690) = 0;
        }
    }
    if (global<u32>(0x00f79690)) {
        g_calls.Cd(0x00598dc0);
        global<u32>(0x00f79690) = 0;
    }
    g_calls.Cd(0x005ed770);   // ParticlePools_Update
}
}
