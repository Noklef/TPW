#include "overlay.h"

#include <cstdint>
#include <cstring>

#include "../core/gameheap.h"
#include "../math/matrix.h"
#include "../math/x87.h"
#include "../model/holder.h"
#include "../model/objmodel.h"
#include "../sprite/screenmatrix.h"
#include "../ui/guidialog.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// Flag 0x10's rectangle: the four words at desc [+0x14] + 4 to +0x18..+0x1e, copied to +0x24..+0x2a.
void CopyRect(u8 *overlay, const u8 *desc) {
    const u8 *const rect = at<const u8 *>(desc, 0x14);
    for (u32 k = 0; k < 8; k += 2) at<u16>(overlay, 0x18 + k) = at<u16>(rect, 4 + k);
    for (u32 k = 0; k < 8; k += 2) at<u16>(overlay, 0x24 + k) = at<u16>(overlay, 0x18 + k);
}

// The node's quad (+0x60) stretched to the overlay's rectangle (see Overlay_ApplyResized): count vertices, x moved too when wide.
void Stretch(const u8 *overlay, u8 *node, u32 count, bool wide) {
    float low[3], high[3];
    std::memcpy(low, node + 0x78, sizeof low);
    std::memcpy(high, node + 0x84, sizeof high);
    const X87Reg a = X87Reg((double)(at<s16>(overlay, 0x1c) - at<s16>(overlay, 0x18))) - (X87Reg(high[0]) - X87Reg(low[0]));
    const X87Reg dy = X87Reg((double)(at<s16>(overlay, 0x1e) - at<s16>(overlay, 0x1a))) - (X87Reg(high[1]) - X87Reg(low[1]));
    const X87Reg b(-dy.v);   // (fchs)
    for (u32 i = 0; i < count; ++i) {
        float *const v = reinterpret_cast<float *>(at<u8 *>(node, 0x60) + (i >> 2) * 0x30 + (i & 3) * 4);
        const float x = v[0], y = v[4], z = v[8];
        v[0] = wide && x > 0.0f ? (float)(X87Reg(x) + a) : X87_Through(x);   // (0x6fedec: 0)
        v[4] = y >= 0.0f ? X87_Through(y) : (float)(X87Reg(y) + b);
        v[8] = X87_Through(z);
    }
    std::memcpy(node + 0x78, &low[0], 4);
    at<float>(node, 0x7c) = (float)(b + X87Reg(low[1]));
    std::memcpy(node + 0x80, &low[2], 4);
    if (wide)
        at<float>(node, 0x84) = (float)(a + X87Reg(high[0]));
    else
        std::memcpy(node + 0x84, &high[0], 4);
    std::memcpy(node + 0x88, &high[1], 8);
}

}  // namespace

extern "C" {

u8 *__fastcall Overlay_Ctor(u8 *overlay, void *, u8 *holder) {
    GuiDialog_Ctor(overlay, nullptr, static_cast<u32>(reinterpret_cast<uintptr_t>(holder)));
    u8 *const h = at<u8 *>(overlay, 4);
    at<u32>(overlay, 0) = 0x006fee10u;
    if (h) {
        at<u32>(at<u8 *>(h, 4), 0x30) |= 0x200;   // (the model reread from the holder after each store, as the original)
        at<u16>(overlay, 0xe) = at<u16>(at<u8 *>(h, 4), 0x44);
        for (u8 *node = at<u8 *>(at<u8 *>(h, 4), 0x78); node; node = at<u8 *>(node, 0xc)) {
            const u32 flags = at<u32>(node, 0) | 0xc8010;
            at<u32>(node, 0) = flags;
            if (*reinterpret_cast<const volatile s32 *>(0x007a1968) == 0x280) at<u32>(node, 0) = flags | 0x8000000;
        }
        Holder_Create(h);
    }
    at<u32>(overlay, 0x7c) = 0;
    at<u32>(overlay, 0x80) = 0;
    return overlay;
}

const u8 *__fastcall Overlay_ShowFrame(u8 *overlay, void *, u32 flags, const u8 *desc) {
    const u8 *holder = at<const u8 *>(overlay, 4);
    if (!(flags & 1) && !(flags & 0x80000)) return at<u8 *>(overlay, 0x7c);
    if (flags & 0x80000) at<u16>(overlay, 0xa) = at<u16>(desc, 0xa);
    if (flags & 1) at<u16>(overlay, 0xc) = at<u16>(desc, 4);
    u16 n = (u16)(at<u16>(overlay, 0xc) + at<u16>(overlay, 0xa));
    u8 *node = at<u8 *>(at<const u8 *>(holder, 4), 0x78);
    for (; n != 0 && node; --n) node = at<u8 *>(node, 0xc);
    if (u8 *old = at<u8 *>(overlay, 0x7c)) at<u32>(old, 0) |= 0x8010;
    at<u8 *>(overlay, 0x7c) = node;
    if (node) at<u32>(node, 0) &= ~0x8010u;
    return node;
}

void __fastcall Overlay_Dtor(u8 *overlay) {
    at<u32>(overlay, 0) = 0x006fee10;
    if (u8 *holder = at<u8 *>(overlay, 4)) {
        if (at<u16>(overlay, 0x10) == 0)
            Holder_Dispose(holder);
        else
            Holder_Destroy(holder);
    }
    GuiDialog_Dtor(overlay);
}

u8 *__fastcall Overlay_ScalarDtor(u8 *overlay, void *, u32 flags) {
    Overlay_Dtor(overlay);
    if (flags & 1) GameHeap_Free(overlay);
    return overlay;
}

void __fastcall Overlay_Dtor_477290(u8 *overlay) { Overlay_Dtor(overlay); }

void __fastcall Overlay_Dtor_4775c0(u8 *overlay) { Overlay_Dtor(overlay); }

u8 *__fastcall Overlay_ScalarDtor_477270(u8 *overlay, void *, u32 flags) { return Overlay_ScalarDtor(overlay, nullptr, flags); }

u8 *__fastcall Overlay_ScalarDtor_4775a0(u8 *overlay, void *, u32 flags) { return Overlay_ScalarDtor(overlay, nullptr, flags); }

void __fastcall Overlay_Apply(u8 *overlay, void *, u32 flags, const u8 *desc) {
    u8 *const holder = at<u8 *>(overlay, 4);
    if (!holder || !desc || !flags) return;
    if (flags & 0x40) {
        at<u16>(overlay, 0x34) = at<u16>(desc, 0xc);
        at<u16>(overlay, 0x36) = at<u16>(desc, 0xe);
    }
    const u8 *const node = Overlay_ShowFrame(overlay, nullptr, flags, desc);
    if (!node) return;
    if (flags & 0x10) {
        const u8 *const rect = at<const u8 *>(desc, 0x14);
        for (u32 k = 0; k < 8; k += 2) at<u16>(overlay, 0x18 + k) = at<u16>(rect, 4 + k);
        for (u32 k = 0; k < 8; k += 2) at<u16>(overlay, 0x24 + k) = at<u16>(overlay, 0x18 + k);
    }
    if (flags & 4) at<u16>(overlay, 0x30) = at<u16>(desc, 8);
    float scale = at<float>(overlay, 0x74);
    bool changed = false;
    if (flags & 0x55) {
        float low[3], high[3], m[16];
        std::memcpy(low, node + 0x78, sizeof low);
        std::memcpy(high, node + 0x84, sizeof high);
        std::memcpy(m, node + 0x10, sizeof m);
        for (const u8 *parent = at<const u8 *>(node, 4); parent; parent = at<const u8 *>(parent, 4))
            Matrix_Mul(m, reinterpret_cast<const float *>(parent + 0x10));
        Matrix_TransformPoint(m, low);
        Matrix_TransformPoint(m, high);
        for (u32 k = 0; k < 2; ++k) {   // (below or unordered: swapped, the new low through the x87)
            if (!(high[k] >= low[k])) {
                const float was = low[k];
                low[k] = X87_Through(high[k]);
                high[k] = was;
            }
        }
        Sprite_ScreenMatrix(reinterpret_cast<float *>(overlay + 0x3c), at<s16>(overlay, 0x18), at<s16>(overlay, 0x1e),
                            (s16)(at<u16>(overlay, 0x1c) - at<u16>(overlay, 0x18)), (s16)(at<u16>(overlay, 0x1e) - at<u16>(overlay, 0x1a)),
                            low[0], low[1], (float)(X87Reg(high[0]) - X87Reg(low[0])), (float)(X87Reg(high[1]) - X87Reg(low[1])),
                            at<s16>(overlay, 0x34), at<s16>(overlay, 0x36), at<s16>(overlay, 0x30));
        const s32 rate = at<s32>(overlay, 0x80);
        if (rate) {
            X87Reg depth = X87Reg(high[2]) - X87Reg(low[2]);
            if (!(depth.v >= 0.0)) depth = X87Reg(-depth.v);   // (fchs: a NaN's sign too)
            at<float>(overlay, 0x64) = (float)(X87Reg((double)rate) / X87Reg(40.0f) / depth);   // 0x74f898
        }
        changed = true;
    }
    if (flags & 2) {
        const s16 height = at<s16>(desc, 6);
        at<s16>(overlay, 0x38) = height;
        changed = true;
        scale = (float)X87_SubR(X87Reg((double)height) / X87Reg(40.0f), X87Reg(1.0f));   // 1 - height / 40 (0x6fedd8, 0x74f898)
    }
    if (!changed) return;
    at<float>(overlay, 0x74) = scale;
    ObjModel_SetMatrices(at<u8 *>(holder, 0x1c), nullptr, 1, reinterpret_cast<const u32 *>(overlay + 0x3c));
    *reinterpret_cast<u32 *>((uintptr_t)0x008bd508) |= 0x80000000;
}

void __fastcall Overlay_SetShown(u8 *overlay, void *, u32 shown32) {
    const u16 shown = (u16)shown32;
    u8 *const holder = at<u8 *>(overlay, 4);
    if (!holder || shown == at<u16>(overlay, 8)) return;
    u32 *const node = at<u32 *>(overlay, 0x7c);
    if (shown) {
        if (node) *node &= 0xffff7fef;
        Holder_WakeWithMatrix(holder, reinterpret_cast<const u32 *>(overlay + 0x3c), 0);
    } else {
        if (node) *node |= 0x8010;
        Holder_Sleep(holder);
    }
    at<u16>(overlay, 8) = shown;
}

void __fastcall Overlay_ApplyResized(u8 *overlay, void *, u32 flags, const u8 *desc) {
    if (!desc) return;
    if (u8 *const node = const_cast<u8 *>(Overlay_ShowFrame(overlay, nullptr, flags, desc))) {
        if (flags & 0x10) CopyRect(overlay, desc);
        Stretch(overlay, node, 0x10, true);
    }
    Overlay_Apply(overlay, nullptr, flags, desc);
}

void __fastcall Overlay_ApplyResizedHeight(u8 *overlay, void *, u32 flags, const u8 *desc) {
    u8 *const node = at<u8 *>(at<u8 *>(at<u8 *>(overlay, 4), 4), 0x78);
    if (desc && (flags & 0x10) && node) {
        CopyRect(overlay, desc);
        Stretch(overlay, node, 8, false);
    }
    Overlay_Apply(overlay, nullptr, flags, desc);
}
}
