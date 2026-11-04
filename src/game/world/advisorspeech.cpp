// The advisor's speech from frame to frame: see advisorspeech.h.
#include "advisorspeech.h"

#include <cstdint>

#include "../core/crt.h"

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
        const u32 owner = global<u32>(0x00f79678) ? global<u32>(0x00803a40) : global<u32>(0x00803a34);
        Voice() = g_calls.Cd(0x0051bfc0, 0u, owner, global<u32>(0x00f7967c), 0u, 0u, 0u);   // Sound_PlayAt
        global<u32>(0x00f79694) = 0;
        MouthOpen() = 1;
    }

    const s32 endAt = global<s32>(0x00f79778);
    if (endAt && endAt - now <= 0) {
        g_calls.Cd(0x00521e60, 0x57u, 0x15f90u, 0u, 0x1b58u);   // Particle_Spawn
        g_calls.Cd(0x0051bfc0, 0u, global<u32>(0x00803a2c), 0xc6u, 0u, 0u, 0u);
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
}
