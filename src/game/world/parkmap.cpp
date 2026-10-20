// The park map's squares classified: see parkmap.h.
#include "parkmap.h"

#include <cstdint>
#include <cstring>

#include "../core/accessors.h"
#include "../core/ptrlist.h"
#include "../math/x87.h"
#include "../render/backdrop.h"
#include "../ui/window.h"
#include "camera.h"
#include "mapcell.h"
#include "object.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *cell44(u32 id) { return global<u8 *>(0x007cf83c) + 0x2d8 + (id & 0xffff) * 0x44 - 0x44; }
u8 *thing(u32 handle) { return global<u8 *>(0x007cfb90 + (handle & 0xffff) * 20); }

// The owner's things: the first that tells a kind sets the entry; 1 when one did.
void classify_things(u8 *entry, u8 *cell, u16 owner) {
    for (u16 handle = at<u16>(cell44(owner), 0x24); handle != 0; handle = at<u16>(thing(handle), 0xa)) {
        u8 *const t = thing(handle);
        if (t[2] != 3) continue;
        u32 kind;
        switch (at<u32>(Object_GetDef(t), 0x4ac)) {
        case 0:
            kind = !MapCell_IsType4(cell) && (MapHandle_IsTypeBDOr10Or12Or19(reinterpret_cast<const u16 *>(cell)) || MapCell_IsType15(cell)) ? 8 : 1;
            break;
        case 1: kind = 2; break;
        case 2: kind = 3; break;
        case 3: {
            const u8 flags = t[0x32];
            kind = flags & 0x41 ? 5 : flags & 0x10 ? 6 : flags & 2 ? 7 : 4;
            break;
        }
        default: continue;
        }
        at<u32>(entry, 0) = kind;
        at<u16>(entry, 4) = handle;
        return;
    }
}

}  // namespace

extern "C" {

void __fastcall ParkMap_Classify(u8 *map) {
    for (s32 y = 0; y < at<s32>(map, 0x28); ++y) {
        for (s32 x = 0; x < at<s32>(map, 0x24); ++x) {
            const s32 row = y * 0x80 + 0xc;
            u8 *const entry = x >= 0 && x < 0x80 && row >= 0xc && row < 0x400c ? map + (x + row) * 8 : nullptr;
            at<u32>(entry, 0) = 0;
            at<u16>(entry, 4) = 0;
            u8 *const cell = cell44((static_cast<u32>(y) << 7) + 1 + static_cast<u32>(x));
            const u16 *const handle = reinterpret_cast<const u16 *>(cell);
            if (MapHandle_ParentIsType19(handle)) {
                at<u32>(entry, 0) = 9;
            } else if (MapCell_IsType4(cell) || MapCell_IsType9(cell) || MapCell_IsTypeA(cell) || MapHandle_IsTypeBDOr10Or12Or19(handle) ||
                       MapCell_IsType15(cell)) {
                u16 owner;
                if (MapHandle_IsTypeBDOr10Or12Or19(handle)) {
                    const u8 *e = MapHandle_Entry28(handle);
                    if (MapCell_IsTypeCOr11(e)) e = global<u8 *>(0x007cf83c) + (static_cast<u32>(at<u16>(e, 0x10)) + 0x6cde) * 0x28;
                    owner = at<u16>(e, 0x10);
                } else {
                    owner = at<u16>(cell, 0x10);
                }
                classify_things(entry, cell, owner);
            } else if (MapCell_IsType1(cell)) {
                at<u32>(entry, 0) = 0xa;
            } else if (MapCell_IsType3Or9(cell)) {
                at<u32>(entry, 0) = 0xb;
            } else if (MapCell_HasFlags(cell, nullptr, 0x40)) {
                at<u32>(entry, 0) = 0xc;
            }
        }
    }
}

void __fastcall ParkMap_PlaceSprite(const u8 *map, void *, u8 *sprite) {
    const X87Reg width = static_cast<double>(global<u32>(0x007a1968)), height = static_cast<double>(global<u32>(0x007a1930));
    const X87Reg toX = X87_DivR(static_cast<double>(at<s32>(map, 0x24)), 480.0);
    const float toY = static_cast<float>(X87_DivR(static_cast<double>(at<s32>(map, 0x28)), 425.0));
    const float widthF = static_cast<float>(width), heightF = static_cast<float>(height);
    const X87Reg x = (X87Reg(at<float>(sprite, 4)) - at<float>(map, 0x54)) * toX * at<float>(map, 0x5c) + width * 0.5;
    at<float>(sprite, 0xc) = static_cast<float>(x);
    const X87Reg y = (X87Reg(at<float>(sprite, 8)) - at<float>(map, 0x58)) * toY * at<float>(map, 0x5c) + height * 0.5;
    const X87Reg left = X87Reg(at<float>(sprite, 0xc)) + at<float>(sprite, 0xc);
    const float u = static_cast<float>(left / widthF - 1.0);
    const float v = static_cast<float>((y + y) / heightF - 1.0);
    at<float>(sprite, 0x10) = static_cast<float>(y);
    const s32 layer = at<s32>(sprite, 0x14);
    const u32 depth = layer >= 0 && layer <= 2 ? 0x3f7d70a4 : 0x3f7ff972;  // 0.99 / 0.9999
    u32 ub, vb;
    std::memcpy(&ub, &u, 4);
    std::memcpy(&vb, &v, 4);
    PtrArray_SetVectorOf(reinterpret_cast<const u8 *>(0x00879ce0), nullptr, at<u8 *>(sprite, 0x28), ub, vb, depth);
    const float scale = static_cast<float>(X87Reg(at<float>(sprite, 0x24)) * at<float>(map, 0x5c) * 0.5);
    u8 *const frame = reinterpret_cast<u8 *>(static_cast<uintptr_t>(Fn_56b620(at<u32>(sprite, 0x28))));
    at<float>(frame, 0x2c) = static_cast<float>(X87Reg(at<u16>(at<u8 *>(frame, 0x28), 0xc)) * scale * 0.0625);
}

// One coordinate of the view origin kept to 0 to count - 1 (the limit as the integer the x87 holds). A zero keeps its sign (the
// compiler's max / min would not), a negative one or NaN gives 0.
static double clamp_view(float v, u32 count) {
    const double limit = (double)(s32)(count - 1);
    if (v == 0.0f)
        return limit >= 0.0 ? (double)v : limit;
    if (!(v > 0.0f))
        return 0.0;
    return (double)v <= limit ? (double)v : limit;
}

s32 __fastcall ParkMap_ApplyView(u8 *map) {
    const double x = clamp_view(at<float>(map, 0x54), at<u32>(map, 0x24));
    at<float>(map, 0x54) = (float)x;
    const double z = clamp_view(at<float>(map, 0x58), at<u32>(map, 0x28));
    at<float>(map, 0x58) = (float)z;
    const float fx = (float)(s32)((u32)(s32)(long long)x * 10u);  // __ftol
    const float fz = (float)(s32)((u32)(s32)(long long)z * 10u);
    Camera_SetView(reinterpret_cast<const u32 *>(&fx), reinterpret_cast<const u32 *>(&fz), nullptr, nullptr, nullptr);
    return Window_SendMessage(at<u8 *>(map, 4), 4, 0, 0);
}

void __fastcall ParkMap_LayOut(const u8 *map) {
    const auto bits = [](float f) {
        u32 b;
        std::memcpy(&b, &f, 4);
        return b;
    };
    const u32 width = global<u32>(0x007a1968), height = global<u32>(0x007a1930);
    const float zoom = at<float>(map, 0x5c);
    const X87Reg toX = X87_DivR(static_cast<double>(at<s32>(map, 0x24)), 480.0f);
    const float widthF = static_cast<float>(static_cast<double>(width));
    const X87Reg halfW = X87Reg(static_cast<double>(width)) / zoom * 0.5f;
    const X87Reg toY = X87_DivR(static_cast<double>(at<s32>(map, 0x28)), 425.0f);
    const float heightF = static_cast<float>(static_cast<double>(height));
    const X87Reg halfH = X87Reg(static_cast<double>(height)) / zoom * 0.5f;
    const X87Reg cx = toX * at<float>(map, 0x54) - -15.0f;
    const X87Reg cy = toY * at<float>(map, 0x58) - -42.0f;
    const float cxF = static_cast<float>(cx), halfWF = static_cast<float>(halfW);
    const X87Reg left = cx - halfWF;
    float l = static_cast<float>(left);
    const float cyF = static_cast<float>(cy);
    const float halfHF = static_cast<float>(halfH);
    float r = static_cast<float>(X87Reg(cxF) + halfWF);   // (from the stored centre, as the bottom)
    float t = static_cast<float>(cy - halfHF);
    float b = static_cast<float>(X87Reg(cyF) + halfHF);
    if (!(left.v >= 0.0))   // (the register before its store)
        l = 0.0f;
    if (!(t >= 0.0f))
        t = 0.0f;
    const double lastX = static_cast<double>(at<u32>(map, 0x48) - 1), lastY = static_cast<double>(at<u32>(map, 0x4c) - 1);
    if (!(r < lastX) && r == r)
        r = static_cast<float>(lastX);
    if (!(b < lastY) && b == b)
        b = static_cast<float>(lastY);
    const auto whole = [](float f) { return static_cast<float>(static_cast<s32>(static_cast<long long>(f))); };   // __ftol, then fild
    Backdrop_SetRect(reinterpret_cast<u8 *>(0x0087aee8), nullptr, bits(whole(l)), bits(whole(t)), bits(whole(r)), bits(whole(b)));
    const X87Reg halfWidth = X87Reg(widthF) * 0.5f;
    const float halfWidthF = static_cast<float>(halfWidth);
    const X87Reg halfHeight = X87Reg(heightF) * 0.5f;
    const X87Reg u0 = X87_SubR((X87Reg(cxF) - l) * zoom, halfWidthF);
    const X87Reg v0 = X87_SubR((X87Reg(cyF) - t) * zoom, halfHeight);
    const X87Reg u1 = (X87Reg(r) - cxF) * zoom + halfWidthF;
    const X87Reg v1 = (X87Reg(b) - cyF) * zoom + halfHeight;
    const auto edge = [](X87Reg v, float size) {   // 2v / size - 1, stored, then kept to -1 to 1 (a NaN to -1)
        const float e = static_cast<float>((v + v) / size - 1.0f);
        return !(e >= -1.0f) ? -1.0f : e > 1.0f ? 1.0f : e;
    };
    const float su0 = edge(u0, widthF), sv0 = edge(v0, heightF), su1 = edge(u1, widthF), sv1 = edge(v1, heightF);
    Backdrop_SetSourceRect(reinterpret_cast<u8 *>(0x0087aee8), nullptr, bits(su0), bits(sv0), bits(su1), bits(sv1));
    for (const u8 *node = at<const u8 *>(at<const u8 *>(map, 0x20080), 4); node;) {
        const u8 *const next = at<const u8 *>(node, 4);
        ParkMap_PlaceSprite(map, nullptr, at<u8 *>(node, 0));
        node = next;
    }
}

}  // extern "C"
