// Animator_ResetLayers and the layer helpers: reimplementation of 0x4726d0 and
// 0x472bc0-0x472f40.
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

const u8 *bank_block(const u8 *animator, const u8 *layer) {
    const u8 *entries = field<const u8 *>(animator, 0x20 + 12 * field<u32>(layer, 4));
    return field<const u8 *>(entries, 4 + 8 * field<u32>(layer, 8));
}
bool bank_loaded(const u8 *animator, const u8 *layer) {
    const u32 bank = field<u32>(layer, 4);
    return bank != 0xc && (animator[0x18 + 12 * bank] & 3);
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
