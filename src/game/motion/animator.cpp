// Animator_ResetLayers, the layer helpers, Layer_Play, Animator_Play and
// Motion_UpdateClocks: reimplementation of 0x4726d0 and 0x472bc0-0x473440.
#include "animator.h"

#include "../math/x87.h"
#include "track.h"

namespace {

template <typename T> T field(const void *p, u32 offset) {
    return *reinterpret_cast<const T *>(static_cast<const u8 *>(p) + offset);
}
template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>(static_cast<u8 *>(p) + offset); }

const float kMsPerFrame = 33.333332f;   // 0x6fec08, 0x6fec10
const float kFramesPerMs = 0.03f;       // 0x6fec0c

u32 &flags_of(u8 *node) { return at<u32>(node, 0); }
void show(u8 *node) {
    if (!(flags_of(node) & 0x80000000)) flags_of(node) &= ~0x10u;
}
void hide(u8 *node) {
    if (!(flags_of(node) & 0x80000000)) flags_of(node) |= 0x10;
}

const u8 *entry_block(const u8 *animator, u32 bank, u32 entry) {
    return field<const u8 *>(field<const u8 *>(animator, 0x20 + 12 * bank), 4 + 8 * entry);
}
const u8 *bank_block(const u8 *animator, const u8 *layer) {
    return entry_block(animator, field<u32>(layer, 4), field<u32>(layer, 8));
}
bool loaded(const u8 *animator, u32 bank) { return bank != 0xc && (animator[0x18 + 12 * bank] & 3); }
bool bank_loaded(const u8 *animator, const u8 *layer) { return loaded(animator, field<u32>(layer, 4)); }
bool has_entry(const u8 *animator, u32 bank, u32 entry) {
    return loaded(animator, bank) && entry < field<u32>(animator, 0x1c + 12 * bank);
}

// The entry's length in ms: frames * 33.333332, __ftol'd.
u32 motion_ms(const u8 *animator, u32 bank, u32 entry) {
    const u8 *block = entry_block(animator, bank, entry);
    return (u32)(long long)X87_MulInt(field<u32>(block, 8) - field<u32>(block, 4), kMsPerFrame);
}

// The block's frame count, as fild loads it and fst stores it.
float block_length(const u8 *block) { return (float)(double)(field<u32>(block, 8) - field<u32>(block, 4)); }

}  // namespace

void Animator_ResetLayersWith(u32 motionFlags, u32 *trackFlags, u8 *animator) {
    const u8 *rest = field<const u8 *>(field<const u8 *>(field<const u8 *>(animator, 0), 8), 4);
    u8 *target = field<u8 *>(field<const u8 *>(animator, 8), 4);
    const u32 animator_flags = field<u32>(animator, 4);
    u32 flags_now = motionFlags;
    if (animator_flags & 8) {
        for (u32 i = 0; i < field<u16>(target, 0x42); ++i) flags_of(Model_Node(target, i)) &= ~0x10u;
    } else if (animator_flags & 0x100000) {
        flags_now |= 1;
    }
    const u32 cleared = (target[0x30] & 4) ? 0x800000 : 0xf40000;
    for (u32 i = 0; i < field<u16>(target, 0x42); ++i) trackFlags[i] = 0;
    const u32 layers = field<u16>(animator, 0xe);
    u8 *const first = field<u8 *>(animator, 0x10);
    for (u8 *layer = first; layer != first + 0x38 * layers; layer += 0x38) {
        if (layer[0] & 0x20) continue;
        if (const u8 *block = field<const u8 *>(layer, 0x34)) {
            const u16 *listed = field<const u16 *>(block, 0x38);
            for (u32 n = field<u16>(block, 0x1a); n != 0; --n) show(Model_Node(target, *listed++));
            const u8 *tracks = field<const u8 *>(block, 0x2c);
            for (u32 k = 0; k < field<u16>(block, 0x12); ++k) {
                const u16 index = field<u16>(tracks + 0x40 * k, 0x14);
                trackFlags[index] = field<u32>(tracks + 0x40 * k, 4);
                u8 *node = Model_Node(target, index);
                flags_of(node) &= ~cleared;
                show(node);
            }
        }
        if (!bank_loaded(animator, layer)) continue;
        if ((flags_now & 1) && !(target[0x30] & 4)) continue;
        if (const u8 *next = bank_block(animator, layer)) {
            const u8 *tracks = field<const u8 *>(next, 0x2c);
            for (u32 k = 0; k < field<u16>(next, 0x12); ++k)
                trackFlags[field<u16>(tracks + 0x40 * k, 0x14)] &= ~field<u32>(tracks + 0x40 * k, 4);
        }
    }
    bool materials_done = false;
    for (u8 *layer = first; layer != first + 0x38 * layers; layer += 0x38) {
        if (!bank_loaded(animator, layer)) continue;
        const u8 *next = bank_block(animator, layer);
        const u8 *block = field<const u8 *>(layer, 0x34);
        if (!(layer[0] & 0x20) && block) {
            if (!(flags_now & 1) || (target[0x30] & 4)) {
                const u8 *tracks = field<const u8 *>(block, 0x2c);
                for (u32 k = 0; k < field<u16>(block, 0x12); ++k) {
                    const u16 index = field<u16>(tracks + 0x40 * k, 0x14);
                    Motion_ResetNode(Model_Node(target, index), Model_Node(rest, index), trackFlags[index]);
                }
            }
            if ((block[0] & 2) && !(motionFlags & 8) && !materials_done) {
                u8 *material = field<u8 *>(target, 0x50);
                for (u32 n = field<u16>(target, 0x36); n != 0; --n, material += 8) {
                    if ((material[0] & 0x40) && at<u16>(material, 2) != 0) {
                        material[1] |= 4;
                        at<u16>(material, 2) = 0;
                    }
                }
                materials_done = true;
            }
        }
        if (layer[0] & 4) layer[0] |= 0x10;
        Motion_MarkNodes(next, field<const u8 *>(animator, 8));
    }
}

extern "C" void __cdecl Animator_ResetLayers(u8 *animator) {
    Animator_ResetLayersWith(*reinterpret_cast<const u32 *>(0x007b4980), *reinterpret_cast<u32 **>(0x007aecd4),
                             animator);
}

void Layer_StartWith(u32 time, u32 realTime, u8 *layer, const u8 *block, s32 restart, float frame) {
    if (!block) {
        at<float>(layer, 0x1c) = 0;
        at<float>(layer, 0x20) = 0;
        at<u32>(layer, 0x10) = 0;
        return;
    }
    const u32 frames = field<u32>(block, 8) - field<u32>(block, 4);
    at<float>(layer, 0x1c) = block_length(block);
    if (restart) {
        const u32 now = (layer[0] & 0x40) ? realTime : time;
        at<u32>(layer, 0x10) = now;
        at<u32>(layer, 0x18) = now;
        at<u32>(layer, 0x14) = now;
        return;
    }
    if ((double)frame > (double)frames) frame = block_length(block);  // fcomp of the unrounded count
    const double ms = X87_Round(X87_Round((double)frame * kMsPerFrame) / field<float>(layer, 0xc));
    const u32 back = (u32)(long long)ms;  // __ftol
    at<u32>(layer, 0x10) = field<u32>(layer, 0x14) - back;
    at<u32>(layer, 0x18) = field<u32>(layer, 0x14) - back;
}

extern "C" void __cdecl Layer_Start(u8 *layer, const u8 *block, s32 restart, float frame) {
    Layer_StartWith(*reinterpret_cast<const u32 *>(0x007b496c), *reinterpret_cast<const u32 *>(0x007b4974), layer,
                    block, restart, frame);
}

extern "C" void __cdecl Layer_Advance(u8 *layer, const u32 *now) {
    if (now) {
        at<u32>(layer, 0x14) = *now;
        at<u32>(layer, 0x18) = *now;
    }
    const u32 elapsed = field<u32>(layer, 0x14) - field<u32>(layer, 0x10);
    at<float>(layer, 0x20) = (float)X87_Round(X87_MulInt(elapsed, field<float>(layer, 0xc)) * kFramesPerMs);
}

extern "C" void __cdecl Layer_SetBlock(u8 *layer, const u8 *block) {
    if (!block) {
        at<float>(layer, 0x1c) = 0;
        at<float>(layer, 0x20) = 0;
        at<u32>(layer, 0x10) = 0;
        return;
    }
    at<float>(layer, 0x1c) = block_length(block);
    const u32 elapsed = field<u32>(layer, 0x14) - field<u32>(layer, 0x10);
    at<float>(layer, 0x20) = (float)X87_Round(X87_MulInt(elapsed, kFramesPerMs) / field<float>(layer, 0xc));
}

extern "C" s32 __cdecl Animator_HasMotion(u8 *const *animator, u32 bank, u32 entry) {
    if (bank == 0xc || !((*animator)[0x18 + 12 * bank] & 3)) return 0;
    return entry < field<u32>(*animator, 0x1c + 12 * bank) ? 1 : 0;
}

extern "C" double __cdecl Layer_TimeLeft(const u8 *layer) {
    const double left = X87_Round((double)field<float>(layer, 0x1c) - (double)field<float>(layer, 0x20));
    return X87_Round(left * kMsPerFrame);
}

extern "C" void __cdecl Motion_MarkNodes(const u8 *block, const u8 *holder) {
    const u8 *model = field<const u8 *>(holder, 4);
    const u8 *track = field<const u8 *>(block, 0x2c);
    for (u32 n = field<u16>(block, 0x12); n != 0; --n, track += 0x40) {
        u32 &node = flags_of(field<u8 *>(track, 0x3c));
        const u32 flags = field<u32>(track, 4);
        node |= 0x40000;
        if (flags & 0x1000) node |= 0x200000;
        if (flags & 0x10000) node |= 0x400000;
        if (flags & 0x289) node |= 0x100000;
    }
    const u16 *listed = field<const u16 *>(block, 0x38);
    if (!listed) return;
    for (u32 k = 0; k < field<u16>(block, 0x1a); ++k) {
        if (!(holder[0] & 4)) {
            hide(Model_Node(model, listed[k]));
            continue;
        }
        for (u32 i = 0; i < field<u16>(model, 0x42); ++i) {
            u8 *node = Model_Node(model, i);
            if (field<u16>(node, 0x50) == listed[k]) {
                hide(node);
                break;
            }
        }
    }
}

extern "C" void __cdecl Animator_CountLayers(u8 *animator) {
    u8 *model = field<u8 *>(field<const u8 *>(animator, 8), 4);
    at<u32>(model, 0x30) &= ~0x80000u;
    u32 playing = 0;
    const u8 *layer = field<const u8 *>(animator, 0x10);
    for (u32 n = field<u16>(animator, 0xe); n != 0; --n, layer += 0x38) playing += field<u32>(layer, 4) < 0xc;
    at<u32>(animator, 0x14) = playing;
    if (playing) at<u32>(model, 0x30) |= 0x80000;
}

extern "C" void __cdecl Animator_SetPlaying(u8 *animator) {
    at<u32>(animator, 0x14) = 1;
    at<u32>(field<u8 *>(field<const u8 *>(animator, 8), 4), 0x30) |= 0x80000;
}

u32 Layer_PlayWith(const AnimatorEnv &env, u8 *animator, u32 bank, u32 entry, u8 *layer, u32 mode, float speed) {
    u32 &flags = at<u32>(layer, 0);
    flags = (mode & 1) ? flags | 1 : flags & ~1u;
    flags = (mode & 4) ? flags | 0x20 : flags & ~0x20u;
    const bool stop = bank == 0xd || bank == 0xe;
    const u32 old_bank = field<u32>(layer, 4);
    s32 restart = 0;
    float frame = 0;
    const u8 *old_block = nullptr;
    if (old_bank == 0xc) {
        restart = 1;
    } else {
        if (stop) {
            flags |= bank == 0xd ? 2 : 0x14;
        } else if (has_entry(animator, bank, entry)) {
            // past the end of the last motion: the overrun carries into the new one
            const float at_frame = field<float>(layer, 0x20), length = field<float>(layer, 0x1c);
            if (at_frame > length) frame = at_frame - length;
            else restart = 1;
            flags &= ~6u;
        }
        old_block = bank_block(animator, layer);
    }
    bool played = false;
    if (!stop) {
        const bool reset = old_bank != 0xc && !(mode & 0xc);
        if (!loaded(animator, bank)) {
            if (reset) Motion_ResetChannelsWith(env.motionFlags, env.trackFlags, animator, old_block, nullptr);
            at<u32>(layer, 4) = 0xc;
        } else {
            const u8 *block = entry_block(animator, bank, entry);
            if (reset) Motion_ResetChannelsWith(env.motionFlags, env.trackFlags, animator, old_block, block);
            at<u32>(layer, 8) = entry;
            at<u32>(layer, 4) = bank;
            at<float>(layer, 0xc) = speed;
            Layer_StartWith(env.time, env.realTime, layer, block, restart, frame);
            if (!(field<u32>(animator, 4) & 0x8000)) {
                const u32 elapsed = field<u32>(layer, 0x14) - field<u32>(layer, 0x10);
                at<float>(layer, 0x20) =
                    (float)X87_Round(X87_MulInt(elapsed, field<float>(layer, 0xc)) * kFramesPerMs);
            }
            played = true;
            if (!(mode & 8)) Motion_MarkNodes(block, field<const u8 *>(animator, 8));
        }
    }
    if (!played) {
        if (!stop) {
            at<u32>(layer, 0x10) = at<u32>(layer, 0x14) = at<u32>(layer, 0x18) = 0;
            at<float>(layer, 0xc) = 1;
            at<u32>(layer, 0x20) = 0;
        } else {
            const u32 clock = (flags & 0x40) ? env.realTime : env.time;
            at<u32>(layer, 0x10) = clock;
            if (flags & 2) {
                at<u32>(layer, 0x14) = clock;
                at<u32>(layer, 0x20) = 0;
            } else {  // the time goes ahead by the length (-33.333332 ms a frame, 0x6fec18)
                const double ms = X87_Round(X87_Round((double)field<float>(layer, 0x1c) * -kMsPerFrame) /
                                            field<float>(layer, 0xc));
                at<u32>(layer, 0x14) = clock - (u32)(long long)ms;
                at<u32>(layer, 0x20) = field<u32>(layer, 0x1c);  // the frame becomes the length
            }
            at<u32>(layer, 0x18) = at<u32>(layer, 0x10);
        }
    }
    if (!(field<u32>(animator, 4) & 0x1000000)) Animator_CountLayers(animator);
    const u32 now_bank = field<u32>(layer, 4), now_entry = field<u32>(layer, 8);
    return has_entry(animator, now_bank, now_entry) ? motion_ms(animator, now_bank, now_entry) : 0;
}

u32 Animator_PlayWith(const AnimatorEnv &env, u8 *animator, u32 bank, u32 entry, u32 mode, float speed,
                      u32 index) {
    u8 *layer = field<u8 *>(animator, 0x10) + 0x38 * index;
    u8 *holder = field<u8 *>(animator, 8);
    if ((field<u32>(animator, 4) & 8) && !(holder[0] & 1) && bank != 0xc) env.wake(holder, field<u32>(animator, 0xc8), 1);
    const bool busy = has_entry(animator, field<u32>(layer, 4), field<u32>(layer, 8)) &&
                      !(field<float>(layer, 0x20) > field<float>(layer, 0x1c)) && !(layer[0] & 6) && bank != 0xd &&
                      bank != 0xe;
    if (busy && !(mode & 2)) {
        at<u32>(layer, 0x2c) = mode;
        at<u32>(layer, 0x24) = bank;
        at<u32>(layer, 0x28) = entry;
        at<float>(layer, 0x30) = speed;
        const u32 left = (u32)(long long)Layer_TimeLeft(layer);
        return left + (has_entry(animator, bank, entry) ? motion_ms(animator, bank, entry) : 1000);
    }
    if (mode & 2) {
        at<u32>(layer, 0x24) = 0xc;
        at<u32>(layer, 0x28) = 0;
        at<u32>(layer, 0x30) = 0;
    }
    const u32 ms = Layer_PlayWith(env, animator, bank, entry, layer, mode, speed);
    return ms ? ms : 1000;
}

namespace {
AnimatorEnv game_env() {
    return {*reinterpret_cast<const u32 *>(0x007b4980), *reinterpret_cast<u32 **>(0x007aecd4),
            *reinterpret_cast<const u32 *>(0x007b496c), *reinterpret_cast<const u32 *>(0x007b4974),
            reinterpret_cast<void(__cdecl *)(u8 *, u32, s32)>(0x0045bd30)};
}
}  // namespace

extern "C" u32 __cdecl Layer_Play(u8 *animator, u32 bank, u32 entry, u8 *layer, u32 mode, float speed) {
    return Layer_PlayWith(game_env(), animator, bank, entry, layer, mode, speed);
}

extern "C" u32 __cdecl Animator_Play(u8 *animator, u32 bank, u32 entry, u32 mode, float speed, u32 index) {
    return Animator_PlayWith(game_env(), animator, bank, entry, mode, speed, index);
}

extern "C" s32 __cdecl Layer_ClearQueue(u8 *animator, u32 index) {
    u32 &queued = at<u32>(field<u8 *>(animator, 0x10) + 0x38 * index, 0x24);
    if (queued == 0xc) return 0;
    queued = 0xc;
    return 1;
}

void Motion_UpdateClocksWith(u32 *time, float *step, u32 now) {
    const u32 before = *time;
    *time = now;
    *step = (float)X87_MulInt(now - before, 0.001f);  // 0x6fec1c
}

extern "C" void __cdecl Motion_UpdateClocks() {
    using Clock = u32(__fastcall *)(void *self, void *unused);  // thiscall without arguments
    void *const clocks = reinterpret_cast<void *>(0x00785970);
    Motion_UpdateClocksWith(reinterpret_cast<u32 *>(0x007b496c), reinterpret_cast<float *>(0x007b497c),
                            reinterpret_cast<Clock>(0x00402d70)(clocks, nullptr));
    *reinterpret_cast<u32 *>(0x007b4974) = reinterpret_cast<Clock>(0x00402d80)(clocks, nullptr);
}
