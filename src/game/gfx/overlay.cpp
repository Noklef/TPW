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

}  // namespace

extern "C" {

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
}
