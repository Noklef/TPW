// The advisor's speech from frame to frame: see advisorspeech.h.
#include "advisorspeech.h"

#include <cstdint>
#include <cstring>

#include "../core/crt.h"
#include "../files/file.h"
#include "advisorspeech.h"
#include "advisor.h"
#include "../core/soundset.h"

namespace {

AddrCalls g_calls;

const u32 kClock = 0x00785970;

template <typename T>
T &global(u32 address) {
    return *reinterpret_cast<T *>((uintptr_t)address);
}

AdvisorState &Adv() { return gAdvisor_State_f79660; }
u32 &Speaking() { return Adv().speaking; }
u32 &Voice() { return Adv().voice; }
s32 &Slot() { return Adv().slot; }
s32 &MouthOpen() { return Adv().mouthOpen; }
s32 &Shape() { return Adv().shape; }
s32 &ShownShape() { return Adv().shownShape; }
s32 *LipTimes() { return reinterpret_cast<s32 *>((uintptr_t)Adv().lipTimes); }
// (past the queue's 20 the index runs on into the block, as the original's)
u32 &Queued(u32 index) { return global<u32>((u32)Adv().queue + index * 4); }
u32 &QueueIndex() { return Adv().queueIndex; }
// An animation's length (1-based).
s32 AnimLength(u32 anim) { return global<s32>((u32)Adv().animLengths - 4 + anim * 4); }
// The lip times given up: freed unless they are the static ones, which are marked free.
void FreeLipTimes() {
    const u32 times = Adv().lipTimes;
    if (times) {
        if (times != (u32)gAdvisor_LipBuffer_f797d0) {
            g_calls.Cd(0x0067ad90, times);   // CRT_free
        } else {
            gAdvisor_f79bb8 = 0;
        }
    }
}

// A shape's first face in the head's model.
u32 &Face(u32 holder, s32 shape) {
    const u32 face = global<u32>((u32)Adv().mouthParts - 4 + (u32)(shape + Slot() * 5) * 4);
    const u32 model = global<u32>(global<u32>(holder + 8) + 4);
    return global<u32>(global<u32>(model + 0x70) + face * 0xa0);
}

}  // namespace

void AdvisorSpeech_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

void __cdecl Advisor_UpdateSpeech() {
    AdvisorState &a = Adv();
    s32 now = (s32)g_calls.Th(0x00402d80, kClock);   // Clocks_RealNow
    if (g_calls.Th(0x00402df0, kClock)) {
        if (!a.paused) {
            a.paused = 1;
            a.pauseStart = now;
        } else {
            now = a.pauseStart;
        }
    } else if (a.paused) {
        a.paused = 0;
        a.pausedTotal += now - a.pauseStart;
    }
    now -= a.pausedTotal;

    if (!Speaking()) {
        s32 *const times = LipTimes();
        if (times) {
            if (reinterpret_cast<u8 *>(times) != gAdvisor_LipBuffer_f797d0) {
                g_calls.Cd(0x0067ad90, times);   // CRT_free
            } else {
                gAdvisor_f79bb8 = 0;
            }
            a.lipTimes = 0;
            a.lipBytes = 0;
            a.lipSync = 0;
        }
        return;
    }

    if (!Voice() || !(Voice() = g_calls.Cd(0x0051c270, Voice(), 0u, 0u, 0u))) {   // Sound_SetPosition
        if (a.endTime - now <= 0) {
            Speaking() = 0;
            g_calls.Cd(0x0051e6f0, 0u);   // Sound_Set803acc
            g_calls.Cd(0x005ed5a0, 0u, 0u, 0u);   // ParticlePool_Resize
        }
    }

    const s32 sampleAt = a.sampleAt;
    if (sampleAt && sampleAt - now <= 0) {
        const u32 owner = a.theme ? gSoundCategories.themeSpeech : gSoundCategories.speech;
        Voice() = g_calls.Cd(0x0051bfc0, 0u, owner, a.sound, 0u, 0u, 0u);   // Sound_PlayAt
        a.sampleAt = 0;
        MouthOpen() = 1;
    }

    const s32 endAt = a.endAt;
    if (endAt && endAt - now <= 0) {
        g_calls.Cd(0x00521e60, 0x57u, 0x15f90u, 0u, 0x1b58u);   // Particle_Spawn
        g_calls.Cd(0x0051bfc0, 0u, gSoundCategories.ui, 0xc6u, 0u, 0u, 0u);
        g_calls.Cd(0x004b9340);   // Dialog7cc4b4_Update
        a.endAt = 0;
    }

    if (!a.highlight && (Queued(QueueIndex()) & 0xff80ffff)) {
        const u32 holder = g_calls.Cd(0x00429fe0, Slot());   // ModelSlot_Holder
        s32 layerState, entry;
        g_calls.Cd(0x00473ff0, holder, &layerState, &entry, 0u);   // Layer_GetQueued
        if (layerState == 0xc) {
            g_calls.Cd(0x004732a0, holder, 5u, (Queued(QueueIndex()) & 0xff80ffff) - 1, 0u, 0x3f800000u, 0u);   // Animator_Play
            const u32 index = QueueIndex();
            a.flags = Queued(index) & 0x7f0000;
            a.pending = 1;
            a.anim = (Queued(index) & 0xff80ffff) - 1;
            QueueIndex() = index + 1;
        }
    }

    u32 lipSync = a.lipSync;
    if (lipSync && now > a.lipAt) {
        MouthOpen() = MouthOpen() == 0;
        s32 wait = -1;
        s32 *const times = LipTimes();
        if (times) {
            const s32 time = times[a.lipIndex++];
            if (time != -1) wait = time / 1000;
        }
        if (wait == -1) {
            a.lipAt = -1;
            lipSync = 0;
            MouthOpen() = 0;
            a.lipSync = 0;
        } else {
            a.lipAt = wait + a.startTime;
        }
    }

    if (MouthOpen() && lipSync) {
        if (now > a.shapeAt) {
            Shape() = Crt().rand() % a.shapeCount + 1;
            a.shapeAt = now + 100;
        }
    } else {
        Shape() = 1;
    }

    if (ShownShape() != Shape()) {
        const u32 holder = g_calls.Cd(0x00429fe0, Slot());
        if (ShownShape() > 0) Face(holder, ShownShape()) |= 0x10;
        if (Shape() && !(a.flags & 0x10000)) Face(holder, Shape()) &= ~0x10u;
        ShownShape() = Shape();
    }
}

u32 __cdecl Advisor_QueueRandom(s32 length) {
    u32 total = 0;
    s32 count = 0;
    while (length > 0 && count < 0x13) {
        const s32 pick = (s32)g_calls.Cd(0x0067b5c0) % 10;   // rand
        if (count == 0) {
            Queued(0) = 0xe;
        } else if (length < AnimLength(pick + 1)) {
            u32 fits = 0xa;
            for (u32 k = 0; k < 10; ++k) {
                if (length <= AnimLength(k + 1)) {
                    fits = k + 1;
                    break;
                }
            }
            Queued(count) = fits;
        } else {
            Queued(count) = pick + 1;
        }
        const s32 played = AnimLength(Queued(count));
        total += played;
        length -= played;
        ++count;
    }
    Queued(count) = 0xf;
    total += (u32)Adv().animLengths[14];
    ++count;
    Queued(count) = 0;
    QueueIndex() = 0;
    return total;
}

u32 __cdecl Advisor_QueueAnim(s32 index, s32 length) {
    u32 total = 0, count = 0;
    Adv().endAt = 0;
    const u32 *step = &global<u32>(0x0076dc18 + index * 0x50);
    for (;;) {
        const u32 anim = *step;
        Queued(count) = anim;
        if (!anim) break;
        if ((anim & 0x400000) == 0x400000) {
            Adv().endAt = (s32)(total + step[1]);
            step += 2;
            continue;
        }
        if (count == 0 && (anim & 0xff80ffff) != 0xe) Adv().sampleAt = 0;
        total += AnimLength(Queued(count) & 0xff80ffff);
        ++count;
        ++step;
    }
    if (length != 0 && total == 0) return Advisor_QueueRandom(length);
    QueueIndex() = 0;
    return total;
}

u32 __cdecl Advisor_Say(s32 id) {
    if (!global<u8>(0x0078d90c) || Speaking()) return 0;
    u32 entry = 0x00768fb8;
    while (global<s32>(entry) != id) {
        if (global<s32>(entry) == 0x270f) return 0;
        entry += 0x20;
    }
    if (id == 0x270f) return 0;
    AdvisorState &a = Adv();
    a.sound = global<u32>(entry + 4);
    const u32 name = global<u32>(entry + 8);
    const s32 anim = global<s32>(entry + 0xc);
    const s32 where = global<s32>(entry + 0x10);
    a.theme = where >> 16;
    a.slot = where & 0xffff;
    a.selection = global<s32>(entry + 0x14);
    a.selection2 = global<s32>(entry + 0x18);

    s32 now = (s32)g_calls.Th(0x00402d80, kClock);   // Clocks_RealNow
    if (g_calls.Th(0x00402df0, kClock)) {
        if (!a.paused) {
            a.paused = 1;
            a.pauseStart = now;
        } else {
            now = a.pauseStart;
        }
    } else if (a.paused) {
        a.paused = 0;
        a.pausedTotal += now - a.pauseStart;
    }
    a.startTime = now - a.pausedTotal - 200;

    char file[100], path[240];
    g_calls.Cd(0x0067a650, file, (u32)&gText_Advisor_Say_76e0d8, name);   // sprintf ("\\Speech\\lips\\sp_...")
    strcpy(path, a.theme ? reinterpret_cast<const char *>((uintptr_t)g_calls.Cd(0x0040ef40)) : static_cast<const char *>(gText_Advisor_Say_76e0cc));
    strcat(path, file);
    a.mouthOpen = 0;
    a.lipSync = 0;
    a.sampleAt = a.startTime + 1000;

    if (g_calls.Cd(0x0044a280, path, 1u, 0u)) {   // Quickload_Locate
        const u32 located = g_calls.Cd(0x0044a280, path, 1u, 0u);
        const u32 stream = g_calls.Cd(0x005c4d50, located, 0x0074cfb8u);   // VFS_Open
        gAdvisor_Stream_f797c8 = stream;
        if (stream) {
            a.lipBytes = g_calls.Cd(0x005c5000, stream);   // Object_CallSlot14: its size
            FreeLipTimes();
            const s32 bytes = (s32)a.lipBytes;
            u32 times;
            if (bytes < 1000 && !gAdvisor_f79bb8) {
                gAdvisor_f79bb8 = 1;
                times = (u32)gAdvisor_LipBuffer_f797d0;
            } else {
                times = g_calls.Cd(0x0067dcd0, bytes);   // malloc
            }
            a.lipTimes = times;
            s32 result;
            Stream_Read(&result, reinterpret_cast<Stream *>((uintptr_t)stream), reinterpret_cast<void *>((uintptr_t)times), bytes, nullptr);
            g_calls.Cd(0x005f5b80, &result, gAdvisor_Stream_f797c8);   // Stream_Close
            a.lipIndex = 0;
            a.lipSync = 1;
            a.mouthOpen = 1;
        }
    }

    g_calls.Cd(0x0051e6f0, 1u);   // Sound_Set803acc
    const u32 owner = a.theme ? gSoundCategories.themeSpeech : gSoundCategories.speech;
    Voice() = g_calls.Cd(0x0051bfc0, 0u, owner, a.sound, 0u, 0u, 0u);   // Sound_PlayAt
    const u32 voiced = g_calls.Cd(0x0051c570, Voice()) + 0xc8;   // Sound_VoiceInfo: its length
    if (!voiced) return 0;
    const u32 played = anim == -1 ? Advisor_QueueRandom((s32)(voiced + 0x12c)) : Advisor_QueueAnim(anim - 0x10, (s32)(voiced + 0x12c));
    g_calls.Cd(0x005ed5a0, 0u, 0u, 0u);   // ParticlePool_Resize
    g_calls.Cd(0x00429e90, Queued(QueueIndex()) & 0xff80ffff, (u32)a.slot);   // ModelSlot_CallAdvisor
    const u32 first = QueueIndex();
    a.anim = (Queued(first) & 0xff80ffff) - 1;
    const u32 flags = Queued(first) & 0x7f0000;
    QueueIndex() = first + 1;
    a.flags = flags;
    a.highlight = 1;
    s32 start = a.startTime;
    if (a.sampleAt) {
        g_calls.Cd(0x0051c2c0, Voice());   // Sound_Forward6
        start = a.startTime + 1000;
        Voice() = 0;
        a.mouthOpen = 0;
        a.startTime = start;
    }
    s32 lip = -1;
    const s32 *const times = LipTimes();
    if (times) {
        const u32 at = a.lipIndex;
        const s32 time = times[at];
        a.lipIndex = at + 1;
        if (time != -1) lip = time / 1000;
    }
    a.lipAt = lip + start;
    Speaking() = 1;
    a.endTime = (s32)(played + start);
    if (a.endAt) a.endAt += start;
    return played + (u32)a.animLengths[14] + 1000;
}

void __cdecl Advisor_Load(Stream *stream) {
    u32 word = (u32)(uintptr_t)stream;   // (the argument's own slot: left as it was by a short read)
    s32 result;
    Stream_Read(&result, stream, &word, 4, nullptr);
    if (word != 0x5f564441) return;   // "ADV_"
    AdvisorState &a = Adv();
    FreeLipTimes();
    a.lipTimes = 0;
    Stream_Read(&result, stream, &word, 4, nullptr);
    Stream_Read(&result, stream, &a, (s32)word, nullptr);
    Stream_Read(&result, stream, &a.lipBytes, 4, nullptr);
    const s32 bytes = (s32)a.lipBytes;
    if (!bytes) return;
    u32 times;
    if (bytes < 1000 && !gAdvisor_f79bb8) {
        gAdvisor_f79bb8 = 1;
        times = (u32)gAdvisor_LipBuffer_f797d0;
    } else {
        times = g_calls.Cd(0x0067dcd0, bytes);   // malloc
    }
    const s32 count = (s32)a.lipBytes;
    a.lipTimes = times;
    Stream_Read(&result, stream, reinterpret_cast<void *>((uintptr_t)times), count, nullptr);
}
}
