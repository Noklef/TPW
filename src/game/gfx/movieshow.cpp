// A movie's frame shown: see movieshow.h.
#include "movieshow.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T game(u32 address) { return reinterpret_cast<T>((uintptr_t)address); }

using Decode = void(__fastcall *)(u8 *, const u8 *, u8 *, u32, u32);
using Step = void(__fastcall *)(u8 *);
using OnData = void(__fastcall *)(u8 *, const u8 *);

const MovieShowEnv kGameEnv = {game<OnData>(0x00670890), game<Step>(0x006711a0), game<Step>(0x006719e0), game<u32(__fastcall *)(u8 *)>(0x00671fa0), game<void(__cdecl *)(s32)>(0x00671000),
                               game<Decode>(0x00671540), game<Decode>(0x00671230), game<void(__cdecl *)(s32 *, s32)>(0x006710c0), game<Decode>(0x00671940), game<Decode>(0x00671800),
                               game<Decode>(0x006718a0), game<OnData>(0x006777d0), game<Step>(0x00677930), game<OnData>(0x00677310), game<void(__fastcall *)(u8 *, u8 *, u32, u32)>(0x006779e0),
                               game<void(__fastcall *)(u8 *, const u8 *, u8 *, u32, u32)>(0x006771a0)};
const MovieShowEnv *g_env = &kGameEnv;

u8 *settingsOf(u8 *player) { return at<u8 *>(player, 0); }

template <typename... A> u32 com(void *object, u32 slot, A... args) {
    using Method = u32(__stdcall *)(void *, A...);
    return (*reinterpret_cast<Method *>(*reinterpret_cast<u8 **>(object) + slot))(object, args...);
}

// The place in the target of the view at +0x14, +0x18 (its pixels' bytes), and the pitch doubled by the flags 0x30.
u8 *viewPlace(u8 *player, u8 *target, u32 &pitch, u32 pixelBytes, u32 flags) {
    u8 *const place = target + at<u32>(player, 0x18) * pitch + at<u32>(player, 0x14) * pixelBytes;
    if (flags & 0x30) pitch += pitch;
    return place;
}

// The target's unlock: a failure counted.
void unlock(u8 *player, u32 surface, u32 count) {
    if (com(at<void *>(settingsOf(player), surface), 0x80, 0u)) ++at<u32>(player, count);
}

}  // namespace

void MovieShow_SetEnv(const MovieShowEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" void __fastcall MovieShow_Frame(u8 *player, u8 *entry) {
    const u8 *const data = at<const u8 *>(entry, 0);
    at<u32>(player, 0x3a8) = at<u32>(entry, 0xc);
    u32 desc[0x1b] = {0x6c};
    u8 *target = nullptr;
    u32 second = 0;
    u32 pitch = 0;
    g_env->readByKind(player, data);
    g_env->placeView(player);
    u8 *s = settingsOf(player);
    if (at<u32>(s, 4) & 0x800) {
        pitch = at<u32>(s, 0x64) * 2;
        target = at<u8 *>(s, 0x4c);
        second = at<u32>(s, 0x50);
        if (at<u32>(s, 4) & 0x40) pitch *= 2;
    } else {
        g_env->createForMode(player);
        if (g_env->restore(player)) return;
        s = settingsOf(player);
        u32 surface = 0;
        switch (at<u32>(player, 0x4c)) {
            case 1:
                if (at<u32>(s, 4) & 0x400) surface = (at<u32>(s, 4) & 0x8000000) ? 0x18 : 0x14;
                else surface = 0x1c;
                break;
            case 2:
                if (at<u32>(player, 0x54)) {
                    surface = 0x20;
                } else {
                    target = at<u8 *>(player, 0x648);
                    second = at<u32>(player, 0x648);
                    pitch = at<u32>(s, 0x64) * 2;
                }
                break;
            case 4:
                surface = 0x1c;
                break;
            default:
                pitch = static_cast<u32>(reinterpret_cast<uintptr_t>(data));   // (sic)
                break;
        }
        if (surface) {
            if (com(at<void *>(s, surface), 0x64, 0u, desc, 0x801u, 0u)) return;
            target = reinterpret_cast<u8 *>((uintptr_t)desc[9]);
            second = desc[9];
            pitch = desc[4];
        }
    }
    const u32 kind = at<u32>(player, 0x3a8);
    switch (kind) {
        case 1:
            g_env->allocate(player, data);
            if (at<u32>(settingsOf(player), 0x48) != 8) g_env->buildPixels(player);
            g_env->draw(player, target, second, pitch);
            break;
        case 2:
            g_env->decode(player, data);
            g_env->draw(player, target, second, pitch);
            break;
        case 3: {
            g_env->tgqQuality(at<s32>(player, 0x48));
            const u32 flags = at<u32>(settingsOf(player), 4);
            if (flags & 0x40) {
                u8 *const place = viewPlace(player, target, pitch, 4, flags);
                g_env->tgq32(player, data + 8, place, second, pitch);
            } else {
                u8 *const place = viewPlace(player, target, pitch, 2, flags);
                g_env->tgq16(player, data + 8, place, second, pitch);
            }
            break;
        }
        case 4: {
            g_env->tqiQuality(&at<s32>(player, 0x408), at<s32>(player, 0x48));
            if (at<u32>(player, 0x4c) != 1) {
                g_env->tqiYuy2(player, data + 8, target, second, pitch);
                break;
            }
            s = settingsOf(player);
            const u32 flags = at<u32>(s, 4);
            if (!(flags & 0x40) && at<u32>(s, 0x48) <= 0x10) {
                u8 *const place = (flags & 0x400) ? viewPlace(player, target, pitch, 2, flags) : target;
                g_env->tqi16(player, data + 8, place, second, pitch);
            } else {
                u8 *const place = (flags & 0x400) ? viewPlace(player, target, pitch, 4, flags) : target;
                g_env->tqi32(player, data + 8, place, second, pitch);
            }
            break;
        }
        case 5:
        case 6:
        case 7:
            g_env->madVideo(player, data, target, pitch, kind);
            break;
        default:
            break;
    }
    s = settingsOf(player);
    const u32 flags = at<u32>(s, 4);
    if (flags & 0x800) return;
    switch (at<u32>(player, 0x4c)) {
        case 1:
            if (flags & 0x400) {
                if (flags & 0x8000000) unlock(player, 0x18, 0x30);
                else unlock(player, 0x14, 0x2c);
            } else {
                unlock(player, 0x1c, 0x38);
            }
            break;
        case 2:
            if (at<u32>(player, 0x54)) unlock(player, 0x20, 0x34);
            break;
        case 4:
            unlock(player, 0x1c, 0x38);
            break;
        default:
            break;
    }
}
