// The advisor's speech from frame to frame: see advisorspeech.h.
#include "advisorspeech.h"

#include <cstdint>
#include <cstring>

#include "../core/crt.h"
#include "../core/soundset.h"
#include "../files/file.h"
#include "advisorspeech.h"
#include "advisor.h"

namespace {

AddrCalls g_calls;

const u32 kClock = 0x00785970;

template <typename T>
T &global(u32 address) {
    return *reinterpret_cast<T *>((uintptr_t)address);
}

u32 &Speaking() { return global<u32>(0x00f7966c); }
u32 &Voice() { return global<u32>(0x00f79680); }
s32 &Slot() { return global<s32>(0x00f79674); }
s32 &MouthOpen() { return global<s32>(0x00f79670); }
s32 &Shape() { return global<s32>(0x00f79788); }
s32 &ShownShape() { return global<s32>(0x00f7978c); }
s32 *LipTimes() { return global<s32 *>(0x00f797a0); }
u32 &Queued(u32 index) { return global<u32>(0x00f79718 + index * 4); }
u32 &QueueIndex() { return global<u32>(0x00f79768); }
// An animation's length.
s32 AnimLength(u32 anim) { return global<s32>(0x00f796c4 + anim * 4); }
// The lip times given up: freed unless they are the static ones, which are marked free.
void FreeLipTimes() {
    const u32 times = global<u32>(0x00f797a0);
    if (times) {
        if (times != 0x00f797d0) {
            g_calls.Cd(0x0067ad90, times);   // CRT_free
        } else {
            global<u32>(0x00f79bb8) = 0;
        }
    }
}

// A shape's first face in the head's model.
u32 &Face(u32 holder, s32 shape) {
    const u32 face = global<u32>(0x00f7969c + (u32)(shape + Slot() * 5) * 4);
    const u32 model = global<u32>(global<u32>(holder + 8) + 4);
    return global<u32>(global<u32>(model + 0x70) + face * 0xa0);
}

}  // namespace

void AdvisorSpeech_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

void __cdecl Advisor_UpdateSpeech() {
    s32 now = (s32)g_calls.Th(0x00402d80, kClock);   // Clocks_RealNow
    if (g_calls.Th(0x00402df0, kClock)) {
        if (!global<u32>(0x00f7977c)) {
            global<u32>(0x00f7977c) = 1;
            global<s32>(0x00f79780) = now;
        } else {
            now = global<s32>(0x00f79780);
        }
    } else if (global<u32>(0x00f7977c)) {
        global<u32>(0x00f7977c) = 0;
        global<s32>(0x00f79784) += now - global<s32>(0x00f79780);
    }
    now -= global<s32>(0x00f79784);

    if (!Speaking()) {
        s32 *const times = LipTimes();
        if (times) {
            if ((u32)(uintptr_t)times != 0x00f797d0) {
                g_calls.Cd(0x0067ad90, times);   // CRT_free
            } else {
                global<u32>(0x00f79bb8) = 0;
            }
            global<u32>(0x00f797a0) = 0;
            global<u32>(0x00f79794) = 0;
            global<u32>(0x00f7979c) = 0;
        }
        return;
    }

    if (!Voice() || !(Voice() = g_calls.Cd(0x0051c270, Voice(), 0u, 0u, 0u))) {   // Sound_SetPosition
        if (global<s32>(0x00f79668) - now <= 0) {
            Speaking() = 0;
            g_calls.Cd(0x0051e6f0, 0u);   // Sound_Set803acc
            g_calls.Cd(0x005ed5a0, 0u, 0u, 0u);   // ParticlePool_Resize
        }
    }

    const s32 sampleAt = global<s32>(0x00f79694);
    if (sampleAt && sampleAt - now <= 0) {
        const u32 owner = global<u32>(0x00f79678) ? gSoundCategories.themeSpeech : gSoundCategories.speech;
        Voice() = g_calls.Cd(0x0051bfc0, 0u, owner, global<u32>(0x00f7967c), 0u, 0u, 0u);   // Sound_PlayAt
        global<u32>(0x00f79694) = 0;
        MouthOpen() = 1;
    }

    const s32 endAt = global<s32>(0x00f79778);
    if (endAt && endAt - now <= 0) {
        g_calls.Cd(0x00521e60, 0x57u, 0x15f90u, 0u, 0x1b58u);   // Particle_Spawn
        g_calls.Cd(0x0051bfc0, 0u, gSoundCategories.ui, 0xc6u, 0u, 0u, 0u);
        g_calls.Cd(0x004b9340);   // Dialog7cc4b4_Update
        global<u32>(0x00f79778) = 0;
    }

    if (!global<u32>(0x00f79690) && (Queued(QueueIndex()) & 0xff80ffff)) {
        const u32 holder = g_calls.Cd(0x00429fe0, Slot());   // ModelSlot_Holder
        s32 layerState, entry;
        g_calls.Cd(0x00473ff0, holder, &layerState, &entry, 0u);   // Layer_GetQueued
        if (layerState == 0xc) {
            g_calls.Cd(0x004732a0, holder, 5u, (Queued(QueueIndex()) & 0xff80ffff) - 1, 0u, 0x3f800000u, 0u);   // Animator_Play
            const u32 index = QueueIndex();
            global<u32>(0x00f79660) = Queued(index) & 0x7f0000;
            global<u32>(0x00f7968c) = 1;
            global<u32>(0x00f7976c) = (Queued(index) & 0xff80ffff) - 1;
            QueueIndex() = index + 1;
        }
    }

    u32 lipSync = global<u32>(0x00f7979c);
    if (lipSync && now > global<s32>(0x00f79774)) {
        MouthOpen() = MouthOpen() == 0;
        s32 wait = -1;
        s32 *const times = LipTimes();
        if (times) {
            const s32 time = times[global<u32>(0x00f79798)++];
            if (time != -1) wait = time / 1000;
        }
        if (wait == -1) {
            global<s32>(0x00f79774) = -1;
            lipSync = 0;
            MouthOpen() = 0;
            global<u32>(0x00f7979c) = 0;
        } else {
            global<s32>(0x00f79774) = wait + global<s32>(0x00f79664);
        }
    }

    if (MouthOpen() && lipSync) {
        if (now > global<s32>(0x00f79790)) {
            Shape() = Crt().rand() % global<s32>(0x00f79770) + 1;
            global<s32>(0x00f79790) = now + 100;
        }
    } else {
        Shape() = 1;
    }

    if (ShownShape() != Shape()) {
        const u32 holder = g_calls.Cd(0x00429fe0, Slot());
        if (ShownShape() > 0) Face(holder, ShownShape()) |= 0x10;
        if (Shape() && !(global<u32>(0x00f79660) & 0x10000)) Face(holder, Shape()) &= ~0x10u;
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
    total += global<u32>(0x00f79700);
    ++count;
    Queued(count) = 0;
    QueueIndex() = 0;
    return total;
}

u32 __cdecl Advisor_QueueAnim(s32 index, s32 length) {
    u32 total = 0, count = 0;
    global<u32>(0x00f79778) = 0;
    const u32 *step = &global<u32>(0x0076dc18 + index * 0x50);
    for (;;) {
        const u32 anim = *step;
        Queued(count) = anim;
        if (!anim) break;
        if ((anim & 0x400000) == 0x400000) {
            global<u32>(0x00f79778) = total + step[1];
            step += 2;
            continue;
        }
        if (count == 0 && (anim & 0xff80ffff) != 0xe) global<u32>(0x00f79694) = 0;
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
    global<u32>(0x00f7967c) = global<u32>(entry + 4);
    const u32 name = global<u32>(entry + 8);
    const s32 anim = global<s32>(entry + 0xc);
    const s32 where = global<s32>(entry + 0x10);
    global<s32>(0x00f79678) = where >> 16;
    global<u32>(0x00f79674) = (u32)where & 0xffff;
    global<u32>(0x00f79684) = global<u32>(entry + 0x14);
    global<u32>(0x00f79688) = global<u32>(entry + 0x18);

    s32 now = (s32)g_calls.Th(0x00402d80, kClock);   // Clocks_RealNow
    if (g_calls.Th(0x00402df0, kClock)) {
        if (!global<u32>(0x00f7977c)) {
            global<u32>(0x00f7977c) = 1;
            global<s32>(0x00f79780) = now;
        } else {
            now = global<s32>(0x00f79780);
        }
    } else if (global<u32>(0x00f7977c)) {
        global<u32>(0x00f7977c) = 0;
        global<s32>(0x00f79784) += now - global<s32>(0x00f79780);
    }
    global<s32>(0x00f79664) = now - global<s32>(0x00f79784) - 200;

    char file[100], path[240];
    g_calls.Cd(0x0067a650, file, (u32)&gText_Advisor_Say_76e0d8, name);   // sprintf ("\\Speech\\lips\\sp_...")
    strcpy(path, global<s32>(0x00f79678) ? reinterpret_cast<const char *>((uintptr_t)g_calls.Cd(0x0040ef40)) : static_cast<const char *>(gText_Advisor_Say_76e0cc));
    strcat(path, file);
    global<u32>(0x00f79670) = 0;
    global<u32>(0x00f7979c) = 0;
    global<s32>(0x00f79694) = global<s32>(0x00f79664) + 1000;

    if (g_calls.Cd(0x0044a280, path, 1u, 0u)) {   // Quickload_Locate
        const u32 located = g_calls.Cd(0x0044a280, path, 1u, 0u);
        const u32 stream = g_calls.Cd(0x005c4d50, located, 0x0074cfb8u);   // VFS_Open
        global<u32>(0x00f797c8) = stream;
        if (stream) {
            global<u32>(0x00f79794) = g_calls.Cd(0x005c5000, stream);   // Object_CallSlot14: its size
            FreeLipTimes();
            const s32 bytes = global<s32>(0x00f79794);
            u32 times;
            if (bytes < 1000 && !global<u32>(0x00f79bb8)) {
                global<u32>(0x00f79bb8) = 1;
                times = 0x00f797d0;
            } else {
                times = g_calls.Cd(0x0067dcd0, bytes);   // malloc
            }
            global<u32>(0x00f797a0) = times;
            s32 result;
            Stream_Read(&result, reinterpret_cast<Stream *>((uintptr_t)stream), reinterpret_cast<void *>((uintptr_t)times), bytes, nullptr);
            g_calls.Cd(0x005f5b80, &result, global<u32>(0x00f797c8));   // Stream_Close
            global<u32>(0x00f79798) = 0;
            global<u32>(0x00f7979c) = 1;
            global<u32>(0x00f79670) = 1;
        }
    }

    g_calls.Cd(0x0051e6f0, 1u);   // Sound_Set803acc
    const u32 owner = global<s32>(0x00f79678) ? gSoundCategories.themeSpeech : gSoundCategories.speech;
    Voice() = g_calls.Cd(0x0051bfc0, 0u, owner, global<u32>(0x00f7967c), 0u, 0u, 0u);   // Sound_PlayAt
    const u32 voiced = g_calls.Cd(0x0051c570, Voice()) + 0xc8;   // Sound_VoiceInfo: its length
    if (!voiced) return 0;
    const u32 played = anim == -1 ? Advisor_QueueRandom((s32)(voiced + 0x12c)) : Advisor_QueueAnim(anim - 0x10, (s32)(voiced + 0x12c));
    g_calls.Cd(0x005ed5a0, 0u, 0u, 0u);   // ParticlePool_Resize
    g_calls.Cd(0x00429e90, Queued(QueueIndex()) & 0xff80ffff, global<u32>(0x00f79674));   // ModelSlot_CallAdvisor
    const u32 first = QueueIndex();
    global<u32>(0x00f7976c) = (Queued(first) & 0xff80ffff) - 1;
    const u32 flags = Queued(first) & 0x7f0000;
    QueueIndex() = first + 1;
    global<u32>(0x00f79660) = flags;
    global<u32>(0x00f79690) = 1;
    s32 start = global<s32>(0x00f79664);
    if (global<u32>(0x00f79694)) {
        g_calls.Cd(0x0051c2c0, Voice());   // Sound_Forward6
        start = global<s32>(0x00f79664) + 1000;
        Voice() = 0;
        global<u32>(0x00f79670) = 0;
        global<s32>(0x00f79664) = start;
    }
    s32 lip = -1;
    const s32 *const times = LipTimes();
    if (times) {
        const u32 at = global<u32>(0x00f79798);
        const s32 time = times[at];
        global<u32>(0x00f79798) = at + 1;
        if (time != -1) lip = time / 1000;
    }
    global<s32>(0x00f79774) = lip + start;
    Speaking() = 1;
    global<u32>(0x00f79668) = played + start;
    if (global<u32>(0x00f79778)) global<s32>(0x00f79778) += start;
    return played + global<u32>(0x00f79700) + 1000;
}

void __cdecl Advisor_Load(Stream *stream) {
    u32 word = (u32)(uintptr_t)stream;   // (the argument's own slot: left as it was by a short read)
    s32 result;
    Stream_Read(&result, stream, &word, 4, nullptr);
    if (word != 0x5f564441) return;   // "ADV_"
    FreeLipTimes();
    global<u32>(0x00f797a0) = 0;
    Stream_Read(&result, stream, &word, 4, nullptr);
    Stream_Read(&result, stream, reinterpret_cast<void *>((uintptr_t)0x00f79660), (s32)word, nullptr);
    Stream_Read(&result, stream, reinterpret_cast<void *>((uintptr_t)0x00f79794), 4, nullptr);
    const s32 bytes = global<s32>(0x00f79794);
    if (!bytes) return;
    u32 times;
    if (bytes < 1000 && !global<u32>(0x00f79bb8)) {
        global<u32>(0x00f79bb8) = 1;
        times = 0x00f797d0;
    } else {
        times = g_calls.Cd(0x0067dcd0, bytes);   // malloc
    }
    const s32 count = global<s32>(0x00f79794);
    global<u32>(0x00f797a0) = times;
    Stream_Read(&result, stream, reinterpret_cast<void *>((uintptr_t)times), count, nullptr);
}
}
