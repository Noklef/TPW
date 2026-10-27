#include "renderer.h"

#include <cstdint>
#include <cstring>
#include <initializer_list>

#include "../core/buffers.h"
#include "../core/gameheap.h"
#include "../core/initializers.h"
#include "../gfx/wavelet.h"
#include "../core/vcall.h"
#include "../core/vobj3.h"
#include "../render/engineerrors.h"
#include "../gfx/array3d.h"
#include "../gfx/spriteframe.h"
#include "../math/x87.h"
#include "../core/liveobjects.h"
#include "../render/spritequeue.h"
#include "../ui/guidialog.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u32 &SceneFlags() { return *reinterpret_cast<u32 *>(0x008bd508); }

// The rectangle's words (left, top, right, bottom at +4..+0xa of the record at the info's +0x14) to the part's +0x18..+0x1e.
void copy_rect(u8 *part, const u8 *info) {
    const u8 *const rect = at<const u8 *>(info, 0x14);
    for (u32 k = 0; k < 8; k += 2) at<u16>(part, 0x18 + k) = at<u16>(rect, 4 + k);
}

X87Reg f32(u32 bits) { return X87Reg((double)reinterpret_cast<const float &>(bits)); }

// A node {key, next} goes on the front of the free list.
void Free(u8 *renderer, u8 *node) {
    at<u32>(node, 4) = at<u32>(renderer, 4);
    at<u32>(renderer, 4) = (u32)(uintptr_t)node;
    at<u32>(renderer, 0xc)--;
}

void FreeList(u8 *renderer, u8 *node) {
    while (node) {
        u8 *next = reinterpret_cast<u8 *>(at<uintptr_t>(node, 4));
        Free(renderer, node);
        node = next;
    }
}

// The first node with the key leaves the list at head; whether there was one.
bool Remove(u8 *renderer, u32 *head, u32 key) {
    u8 *prev = nullptr;
    for (u8 *node = reinterpret_cast<u8 *>(*head); node; node = reinterpret_cast<u8 *>(at<uintptr_t>(node, 4))) {
        if (at<u32>(node, 0) == key) {
            if (prev)
                at<u32>(prev, 4) = at<u32>(node, 4);
            else
                *head = at<u32>(node, 4);
            Free(renderer, node);
            return true;
        }
        prev = node;
    }
    SceneFlags() |= 0x80000000;
    return false;
}

u32 *Slot(u8 *renderer, u32 tag) { return &at<u32>(renderer, 0x10 + ((s32)tag >> 4) * 4); }

// Four arrays from base + 0x2c in steps of 0x10: Array3D_Free on each, the block of the last freed and cleared, then BufferC_FreeAndClear on the first three from the third back.
void ClearGroup(u8 *base) {
    Array3D_Free(base + 0x2c);
    Array3D_Free(base + 0x3c);
    Array3D_Free(base + 0x4c);
    Array3D_Free(base + 0x5c);
    u8 *const last = base + 0x5c;
    if (at<void *>(last, 0xc)) GameHeap_Free(at<void *>(last, 0xc));
    at<u32>(last, 0) = 0;
    at<u32>(last, 4) = 0;
    at<u32>(last, 8) = 0;
    at<u32>(last, 0xc) = 0;
    BufferC_FreeAndClear(base + 0x4c);
    BufferC_FreeAndClear(base + 0x3c);
    BufferC_FreeAndClear(base + 0x2c);
}

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __cdecl Renderer_Create() {
    if (global<u32>(0x0087a6d0))
        return 0;
    u8 *const renderer = static_cast<u8 *>(GameHeap_Alloc(0x7c));
    if (renderer) {
        for (const u32 offset : {0x0u, 0x4u, 0x8u, 0xcu, 0x3cu, 0x38u, 0x70u, 0x30u, 0x10u, 0x34u, 0x78u})
            at<u32>(renderer, offset) = 0;
        at<u32>(renderer, 0x40) = 0x30;
        at<u32>(renderer, 0x74) = 0xffffffff;
        for (u32 offset = 0x14; offset < 0x30; offset += 4)
            at<u32>(renderer, offset) = 0;
        at<u32>(renderer, 0) = TrackedGlobal_Alloc(0x24000);
        if (at<u32>(renderer, 0)) {
            for (u32 i = 0; i < 0x24000; i += 8) {
                at<u32>(reinterpret_cast<u8 *>((uintptr_t)at<u32>(renderer, 0)), i + 4) = at<u32>(renderer, 4);
                at<u32>(renderer, 4) = at<u32>(renderer, 0) + i;
            }
            at<u32>(renderer, 8) = 0x4800;
            at<u32>(renderer, 0xc) = 0;
        }
        // (a renderer without its pool is kept and counts as made, sic)
    }
    global<u8 *>(0x0087a6d0) = renderer;
    if (!renderer) {
        EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 3, 0xb0754000);
        return 0;
    }
    return 1;
}

u32 __cdecl Renderer_Destroy() {
    u8 *renderer = global<u8 *>(0x0087a6d0);
    if (!renderer)
        return 0;
    if (const u32 frame = at<u32>(renderer, 0x10)) {
        TrackedGlobal_Free(frame);
        at<u32>(renderer, 0x10) = 0;
        renderer = global<u8 *>(0x0087a6d0);
    }
    if (renderer) {
        if (const u32 pool = at<u32>(renderer, 0)) {
            TrackedGlobal_Free(pool);
            at<u32>(renderer, 8) = 0;
            at<u32>(renderer, 0xc) = 0;
        }
        if (const u32 frame = at<u32>(renderer, 0x10)) {
            TrackedGlobal_Free(frame);
            at<u32>(renderer, 0x10) = 0;
        }
        GameHeap_Free(renderer);
    }
    global<u32>(0x0087a6d0) = 0;
    return 1;
}

u32 __fastcall Renderer_MakeFrameBuffer(u8 *renderer, void *, s32 width, s32 height) {
    if (const u32 frame = at<u32>(renderer, 0x10)) {
        TrackedGlobal_Free(frame);
        at<u32>(renderer, 0x10) = 0;
    }
    at<s32>(renderer, 0x38) = width;
    const u32 size = (u32)width * (u32)height << 2;
    at<s32>(renderer, 0x3c) = height;
    at<u32>(renderer, 0x10) = TrackedGlobal_Alloc(size);
    u8 *const frame = reinterpret_cast<u8 *>((uintptr_t)at<u32>(renderer, 0x10));
    if (!frame) {
        EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 3, 0xb0754000);
        return 0;
    }
    for (u32 i = 0; i < size; ++i)
        frame[i] = 0;
    const u32 state = (u32)(uintptr_t)(renderer + 0x48);
    at<u32>(renderer, 0x50) = 0xbf800000;   // -1.0f
    at<u32>(renderer, 0x4c) = 0xbf800000;
    at<u32>(renderer, 0x48) = 0x1f31;
    at<u32>(renderer, 0x58) = 0x40000000;   // 2.0f
    at<u32>(renderer, 0x54) = 0x40000000;
    at<u32>(renderer, 0x5c) = global<u32>(0x0087afe8);
    const s32 count = at<s32>(renderer, 0x70);
    if (count < 4) {
        s32 i = 0;
        while (i < count && at<u32>(renderer, 0x60 + 4 * i) != state)
            ++i;
        if (i == count) {
            at<u32>(renderer, 0x60 + 4 * count) = state;
            at<s32>(renderer, 0x70) += 1;
        }
    }
    at<u32>(renderer, 0x70) = 1;   // (whatever the stack held, sic)
    return 1;
}

void __fastcall Renderer_SetRestoreCallback(u8 *renderer, void *, u32 fn) { at<u32>(renderer, 0x78) = fn; }

void __fastcall Renderer_SetStateFlags(u8 *renderer, void *, u32 flags) { at<u32>(renderer, 0x48) = flags; }

u32 __fastcall Renderer_StateFlags(const u8 *renderer) { return at<u32>(renderer, 0x48); }

u32 __fastcall Renderer_RemoveFromSlot(u8 *renderer, void *, u32 key, u32 tag) {
    if (!(tag & 0x70))
        return 0;
    return Remove(renderer, Slot(renderer, tag), key);
}

u32 __fastcall Renderer_ClearSlot(u8 *renderer, void *, u32 tag) {
    if (!(tag & 0x70))
        return 0;
    u32 *slot = Slot(renderer, tag);
    FreeList(renderer, reinterpret_cast<u8 *>(*slot));
    *slot = 0;
    SceneFlags() |= 0x80000000;
    return 1;
}

u32 __fastcall Renderer_RemoveInRect(u8 *renderer, void *, s32 x0, s32 y0, s32 x1, s32 y1, u32 key) {
    if (!at<u32>(renderer, 0x10))
        return 1;
    const s32 width = at<s32>(renderer, 0x38), height = at<s32>(renderer, 0x3c);
    if (x0 >= width || x1 < 0 || y0 >= height || y1 < 0)
        return 0;
    if (x0 < 0)
        x0 = 0;
    if (x1 >= width)
        x1 = width - 1;
    if (y0 < 0)
        y0 = 0;
    if (y1 >= height)
        y1 = height - 1;
    u32 all = 1;
    for (s32 y = y0; y <= y1; y++)
        for (s32 x = x0; x <= x1; x++)
            all &= Remove(renderer, reinterpret_cast<u32 *>(at<uintptr_t>(renderer, 0x10) + (y * width + x) * 4), key);
    return all;
}

u32 __fastcall Renderer_ClearAll(u8 *renderer) {
    const s32 width = at<s32>(renderer, 0x38), height = at<s32>(renderer, 0x3c);
    if (width > 0 && height > 0) {
        for (s32 y = 0; y < height; y++) {
            for (s32 x = 0; x < width; x++) {
                u32 *cell = reinterpret_cast<u32 *>(at<uintptr_t>(renderer, 0x10) + (y * at<s32>(renderer, 0x38) + x) * 4);
                FreeList(renderer, reinterpret_cast<u8 *>(*cell));
                *cell = 0;
                SceneFlags() |= 0x80000000;
            }
        }
    }
    for (u32 offset = 0x14; offset < 0x30; offset += 4) {
        FreeList(renderer, reinterpret_cast<u8 *>(at<uintptr_t>(renderer, offset)));
        at<u32>(renderer, offset) = 0;
        SceneFlags() |= 0x80000000;
    }
    return 1;
}

u32 __fastcall Grid_AddObject(u8 *grid, void *, s32 x0, s32 y0, s32 x1, s32 y1, u8 *object, u32) {
    u32 result = 1;
    const s32 width = at<s32>(grid, 0x38);
    const s32 height = at<s32>(grid, 0x3c);
    if (x0 >= width || x1 < 0 || y0 >= height || y1 < 0) return 0;
    if (x0 < 0) x0 = 0;
    if (x1 >= width) x1 = width - 1;
    if (y0 < 0) y0 = 0;
    if (y1 >= height) y1 = height - 1;
    for (s32 x = x0; x <= x1; ++x) {
        for (s32 y = y0; y <= y1; ++y)
            result &= Pool_AddSorted(grid, nullptr, reinterpret_cast<u8 **>(at<u8 *>(grid, 0x10) + (y * at<s32>(grid, 0x38) + x) * 4), object);
    }
    return result;
}

// A GDI LOGFONT at +0 of block (height, width, escapement, orientation 0; weight 400; not italic, underlined or struck out; charset
// 1, output precision 4, clipping 1, quality 4, pitch 0; the face name the string at 0x774d80).
static void DefaultFont(u8 *block) {
    at<u32>(block, 0) = 0;
    at<u32>(block, 4) = 0;
    at<u32>(block, 8) = 0;
    at<u32>(block, 0xc) = 0;
    at<u32>(block, 0x10) = 0x190;
    const u8 bytes[8] = {0, 0, 0, 1, 4, 1, 4, 0};
    std::memcpy(block + 0x14, bytes, 8);
    std::strcpy(reinterpret_cast<char *>(block + 0x1c), reinterpret_cast<const char *>(0x00774d80));
}

// One of the two groups of four arrays (+0x2c, +0x3c, +0x4c, +0x5c): the first two cleared directly or by Init_5ec210 (first),
// +0 = 3, +4 = 10.0f, +8 = +0x14 = 1.0f, then Array3Ds_Free4.
static void SetupGroup(u8 *group, bool first) {
    for (u32 off = 0x2c; off < (first ? 0x4c : 0x3c); off += 4)
        at<u32>(group, off) = 0;
    for (u32 off = first ? 0x4c : 0x3c; off <= 0x5c; off += 0x10)
        Init_5ec210(group + off);
    at<u32>(group, 0) = 3;
    at<u32>(group, 4) = 0x41200000;
    at<u32>(group, 8) = 0x3f800000;
    at<u32>(group, 0x14) = 0x3f800000;
    Array3Ds_Free4(group);
}

u8 *__fastcall Renderer_Construct(u8 *object) {
    for (u32 off = 0x18; off <= 0x34; off += 4)
        at<u32>(object, off) = 0;
    DefaultFont(object + 0x48);
    at<u32>(object, 0x40) = 100;
    at<u32>(object, 0x44) = 0;
    DefaultFont(object + 0x1d8);
    at<u32>(object, 0x1d0) = 100;
    at<u32>(object, 0x1d4) = 0;
    SetupGroup(object + 0x358, true);
    SetupGroup(object + 0x3c4, false);
    Fn_5ebef0(object + 0x430);
    for (u32 off = 0x444; off < 0x448; ++off)
        at<u8>(object, off) = 0;
    for (u32 off = 0x448; off <= 0x454; off += 4)
        at<u32>(object, off) = 0;
    at<u8 *>(object, 0x14) = WaveletCodec_Create();
    return object;
}

void __fastcall Renderer_Teardown(u8 *object) {
    vcall<u32>(at<u8 *>(object, 0x14), 0x18);
    ClearGroup(object + 0x3c4);
    ClearGroup(object + 0x358);
    static const u32 offsets[2] = {0x28u, 0x18u};
    for (u32 offset : offsets) {
        u8 *const block = object + offset;
        if (at<void *>(block, 0xc)) GameHeap_Free(at<void *>(block, 0xc));
        at<u32>(block, 0) = 0;
        at<u32>(block, 4) = 0;
        at<u32>(block, 8) = 0;
        at<u32>(block, 0xc) = 0;
    }
}

u32 __fastcall Renderer_Add(u8 *renderer, void *, u8 *object, u32 tag) {
    if (tag & 0x70)
        return Pool_AddSorted(renderer, nullptr, reinterpret_cast<u8 **>(renderer + 0x10 + ((s32)tag >> 4) * 4), object);
    if (!(tag & 1) || vcall<s32>(object, 8) != 2)
        return 0;
    const u8 *const record = vcall<u8 *>(object, 0xc);
    if (!record)
        return 0;
    // The rectangle is packed in the record's word at +0xa8, a byte each: x0 high, then x1, y0, y1.
    const u32 packed = at<u32>(record, 0xa8);
    const s32 width = at<s32>(renderer, 0x38), height = at<s32>(renderer, 0x3c);
    const s32 x0 = (s32)(packed >> 24), y0 = (s32)((packed >> 8) & 0xff);
    s32 x1 = (s32)((packed >> 16) & 0xff), y1 = (s32)(packed & 0xff);
    if (x0 >= width || y0 >= height)
        return 0;
    if (x1 >= width)
        x1 = width - 1;
    if (y1 >= height)
        y1 = height - 1;
    u32 all = 1;
    for (s32 x = x0; x <= x1; x++)
        for (s32 y = y0; y <= y1; y++)
            all &= Pool_AddSorted(renderer, nullptr, reinterpret_cast<u8 **>(at<uintptr_t>(renderer, 0x10) + (y * at<s32>(renderer, 0x38) + x) * 4), object);
    return all;
}

u32 __fastcall Obj_ReleaseFromRenderer(u8 *object) {
    if (!(at<u32>(object, 0) & 1))
        return 1;
    at<u32>(object, 0) &= 0xfffffffe;
    u8 *renderer = global<u8 *>(0x0087a6d0);
    if (!renderer)
        return 0;
    u8 *part = at<u8 *>(object, 4);
    if (part)
        part += at<u32>(at<u8 *>(part, 0), 4);
    return Renderer_RemoveFromSlot(renderer, nullptr, (u32)(uintptr_t)part, 0x60) ? 1 : 0;
}

u32 __fastcall Obj_AddToRenderer(u8 *object) {
    if (at<u32>(object, 0) & 1)
        return 1;
    u8 *const renderer = global<u8 *>(0x0087a6d0);
    if (!renderer)
        return 0;
    u8 *part = nullptr;
    if (u8 *const sub = at<u8 *>(object, 4))
        part = sub + at<u32>(at<u8 *>(sub, 0), 4);
    if (!Renderer_Add(renderer, nullptr, part, 0x60))
        return 0;
    at<u32>(object, 0) |= 1;
    return 1;
}

void __fastcall Obj_SetRendererPart30(u8 *object, void *, u32 value) {
    if (at<u16>(object, 0x44) == 0)
        return;
    const u16 id = static_cast<u16>(value);
    if (at<u16>(object, 8) == id)
        return;
    u8 *part = object + 0xb4;
    if (id != 0) {
        // (the original has a branch for a null part pointer here, which cannot happen)
        part += at<u32>(at<u8 *>(part, 0), 4);
        Renderer_Add(global<u8 *>(0x0087a6d0), nullptr, part, 0x30);
    } else {
        part += at<u32>(at<u8 *>(part, 0), 4);
        Renderer_RemoveFromSlot(global<u8 *>(0x0087a6d0), nullptr, static_cast<u32>(reinterpret_cast<uintptr_t>(part)), 0x30);
    }
    at<u16>(object, 8) = id;
}

u32 __fastcall RendererPart_FreeTextures(u8 *object, void *, u32) {
    return SpriteImage_FreeTexturesIfSet(object + 0x9c);
}

u32 __fastcall RendererPart_FreeTextures2(u8 *object) {
    return SpriteImage_FreeTexturesIfSet(object + 0x9c);
}

void __fastcall RendererPart_AllocPixels(u8 *object, void *, u32) {
    if (at<u16>(object, 0x44)) return;
    SpriteImage_AllocPixels(object + 0x9c, nullptr, (u32)(s32)at<s16>(object, 0x3c), (u32)(s32)at<s16>(object, 0x3e));
    SpriteFrame_SetSize30(object + 0x54, nullptr, at<s16>(object, 0x3c), at<s16>(object, 0x3e));
    at<u16>(object, 0x44) = 1;
}

void __fastcall RendererPart_SetLayout(u8 *part, void *, u32 flags, const u8 *info) {
    if (!info || !flags) return;
    if (flags & 0x10) {
        if (!at<u16>(part, 0x44)) {
            copy_rect(part, info);
            const s16 gw = *reinterpret_cast<const s16 *>(0x00faa5c4), gh = *reinterpret_cast<const s16 *>(0x00faa5c0);
            at<s16>(part, 0x3c) = (s16)(static_cast<s32>((u32)(at<s16>(part, 0x1c) - at<s16>(part, 0x18)) * (u32)gw) / 2048);
            at<s16>(part, 0x3e) = (s16)(static_cast<s32>((u32)(at<s16>(part, 0x1e) - at<s16>(part, 0x1a)) * (u32)gh) / 1536);
            vcall<void>(part, 0x28, 0u);   // (RendererPart_AllocPixels in the part's own class)
        }
    } else if (!at<u16>(part, 0x44)) {
        return;
    }
    bool changed = false;
    if (flags & 0x10) {
        changed = true;
        copy_rect(part, info);
    }
    if (flags & 2) {
        at<s16>(part, 0x38) = at<s16>(info, 6);
        if (at<s16>(info, 6) <= 0) at<s16>(part, 0x38) = 1;
        changed = true;
    }
    if (flags & 4) {
        at<u16>(part, 0x30) = at<u16>(info, 8);
        changed = true;
    }
    if (!changed) return;
    const X87Reg one = f32(0x3f800000);
    const X87Reg x = X87Reg((double)at<s16>(part, 0x18)) * f32(0x3a800000) - one;      // / 1024 - 1
    const X87Reg y = X87_SubR(X87Reg((double)at<s16>(part, 0x1e)) * f32(0x3aaaaaab), one);   // 1 - / 768
    const X87Reg z = X87_SubR(X87Reg((double)at<s16>(part, 0x38)) * f32(0x3ccccccd), one);   // 1 - / 40
    if (!(flags & 0x12)) return;
    at<float>(part, 0x68) = (float)x.v;
    at<float>(part, 0x6c) = (float)y.v;
    at<float>(part, 0x70) = (float)z.v;
}

u8 *__fastcall RendererPart_Ctor(u8 *object, void *, u32 value) {
    GuiDialog_Ctor(object, nullptr, value);
    SpriteFrame_Init(object + 0x54);
    SpriteImage_Init(object + 0x9c);
    u8 *const set = object + 0xb4;   // (the sprite set's construction, inlined)
    at<u32>(set, 0) = 0x006ff318;
    at<u32>(set, 0x20) = 0x006fdce8;
    at<u32>(set, at<s32>(at<u8 *>(set, 0), 4)) = 0x006ff308;
    const s32 base = at<s32>(at<u8 *>(set, 0), 4);
    at<s32>(set, base - 4) = base - 0x20;
    at<u32>(set, 4) = 0;
    at<u32>(set, 8) = 0;
    at<u32>(set, 0x10) = 0;
    at<u32>(set, 0xc) = 0xffffffff;
    at<u32>(set, 0x14) = 0;
    at<u32>(set, 0x18) = 0;
    LiveObjects_Add(set ? set + at<s32>(at<u8 *>(set, 0), 4) : nullptr);
    at<u32>(object, 0) = 0x006ff2d8;
    at<u16>(object, 0x44) = 0;
    at<u32>(object, 0x98) = TrackedArray_Alloc(set, nullptr, 1);
    at<u32>(object, 0x64) = 0x30000;
    at<u32>(object, 0x80) = 0x3f800000;   // 1.0f
    at<u8 *>(object, 0x7c) = object + 0x9c;
    SpriteFrame_SetSize(object + 0x54, nullptr, 0, 0);
    at<u8 *>(at<u8 *>(object, 0x98), 0) = object + 0x54;   // (sic: a failed allocation faults)
    at<u32>(object, 0xb8) = 1;
    at<u32>(object, 0x4c) = 0;
    at<u32>(object, 0x50) = 0;
    return object;
}

void __fastcall RendererPart_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x006ff2d8;
    SpriteSet_Dtor(object + 0xb4);
    SpriteImage_Dtor(object + 0x9c);
    GuiDialog_Dtor(object);
}

u8 *__fastcall RendererPart_ScalarDtor(u8 *object, void *, u32 flags) {
    RendererPart_Dtor(object);
    if (flags & 1) GameHeap_Free(object);
    return object;
}
}
