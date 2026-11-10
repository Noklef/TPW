#include "vobj2.h"

#include <cstdint>
#include <cstring>

#include "crt.h"
#include "gameheap.h"
#include "string.h"
#include "vcall.h"
#include "vobj3.h"
#include "../gfx/texset.h"
#include "../gfx/texture.h"
#include "../math/geometry.h"
#include "../math/x87.h"
#include "../render/fixedverts.h"
#include "../render/softrender.h"
#include "../render/engineerrors.h"
#include "../gfx/texcache.h"
#include "../render/pointsprites.h"
#include "../render/batch.h"
#include "../frontend/viewcontrols.h"

namespace {

AddrCalls g_calls;

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

void Delete(u8 *object) { vcall<u32>(object, 0, 1u); }

void CountedDtor(u8 *object, u32 vtable) {
    at<u32>(object, 0) = vtable;
    u8 *shared = at<u8 *>(object, 0x34);
    if (!shared)
        return;
    const u32 count = at<u32>(shared, 4);
    if (count != 0 && count < 0xffffffffu)
        at<u32>(shared, 4) = count - 1;
    if (at<u32>(shared, 4) == 0)  // (the STL lock around it is left out)
        Delete(shared);
}

}  // namespace

extern "C" {

void __fastcall Obj_ReleaseTwoComMembers(u8 *object) {
    u8 *a = at<u8 *>(object, 0xac);
    at<u32>(object, 0xa8) = 0;
    at<u32>(object, 0xa4) = 0;
    if (a)
        comcall<u32>(a, 8);
    u8 *b = at<u8 *>(object, 0xb0);
    if (b)
        comcall<u32>(b, 8);
    at<u32>(object, 0xac) = 0;
    at<u32>(object, 0xb0) = 0;
}

void __fastcall Obj_DeleteFiveMembers(u8 *object) {
    for (u8 *node = at<u8 *>(at<u8 *>(object, 0x18), 4); node;) {
        u8 *next = at<u8 *>(node, 4);
        vcall<u32>(at<u8 *>(node, 0), 0x10);
        node = next;
    }
    u8 *member = at<u8 *>(object, 0x14);
    if (member)
        Delete(member);
    member = at<u8 *>(object, 0x10);
    at<u32>(object, 0x14) = 0;
    if (member)
        Delete(member);
    member = at<u8 *>(object, 0xc);
    at<u32>(object, 0x10) = 0;
    if (member)
        Delete(member);
    member = at<u8 *>(object, 8);
    at<u32>(object, 0xc) = 0;
    if (member)
        Delete(member);
    member = at<u8 *>(object, 4);
    at<u32>(object, 8) = 0;
    if (member)
        Delete(member);
    at<u32>(object, 4) = 0;
}

void __fastcall ListRef_DrainAndDelete(u8 **ref) {
    u8 *list = *ref;
    if (!list)
        return;
    while (at<u32>(list, 0xc) > 0) {
        u8 *node = at<u8 *>(list, 4);
        u8 *held = nullptr;
        if (node) {
            u8 *next = at<u8 *>(node, 4);
            at<u8 *>(list, 4) = next;
            if (next)
                at<u32>(next, 8) = 0;
            else
                at<u32>(list, 8) = 0;
            const u32 left = at<u32>(list, 0xc) - 1;
            held = at<u8 *>(node, 0);
            at<u32>(list, 0xc) = left;
            GameHeap_Free(node);
        }
        if (held)
            Delete(held);
        list = *ref;
    }
    if (*ref)
        Delete(*ref);
    *ref = nullptr;
}

u32 __fastcall ListRef_AnyHandles(u8 **ref, void *, u32 a, u32 b) {
    u8 *list = *ref;
    if (!list)
        return 0;
    for (u8 *node = at<u8 *>(list, 4); node;) {
        u8 *next = at<u8 *>(node, 4);
        if (vcall<u32>(at<u8 *>(node, 0), 8, a, b, 0x80000001u) == 1)
            return 1;
        node = next;
    }
    return 0;
}

void __fastcall Obj_SetNameAndWords(u8 *object, void *, u32 flag, u8 *text, const u16 *table) {
    at<u32>(object, 0x108) = flag;
    if (!flag)
        return;
    vcall<u32>(object + 0x10c, 0x14, text ? text : (u8 *)(uintptr_t)0x00786de4);
    for (u32 i = 0; i < 0x100; ++i)
        at<u16>(object, 0x120 + 2 * i) = table[i];
}

void __fastcall Class709914_Dtor(u8 *object) { CountedDtor(object, 0x00709914); }
void __fastcall Class7099a4_Dtor(u8 *object) { CountedDtor(object, 0x007099a4); }

u32 __fastcall Pointers_MarkAndCallSlot4(u8 *object) {
    for (s32 i = 0; i < at<s32>(object, 0x400); ++i) {
        u8 *thing = at<u8 *>(object, 4 * i);
        at<u32>(thing, 4) |= 0x10;
        thing = at<u8 *>(object, 4 * i);
        u8 *base = thing + at<u32>(at<u8 *>(thing, 0), 4);
        vcall<u32>(base, 4);
    }
    return 1;
}

u8 *__fastcall Class702198_DeletingDtor(u8 *field, void *, u32 flags) {
    u8 *object = field - 0x3c;
    u32 offset = at<u32>(at<u8 *>(object, 0), 4);
    at<u32>(object + offset, 0) = 0x007021f0;
    offset = at<u32>(at<u8 *>(object, 0), 4);  // (read again: the vtable pointer was just replaced when the virtual base is the object itself)
    at<u32>(object, offset - 4) = offset - 0x3c;
    offset = at<u32>(at<u8 *>(object, 0), 4);
    vcall<u32>(object + offset, 4);
    at<u32>(object, 0x3c) = 0x00702198;
    if (flags & 1)
        GameHeap_Free(object);
    return object;
}

void __fastcall Obj_PickByName(u8 *object, void *, u32 flag) {
    at<u32>(object, 0xc) = at<u32>(at<u8 *>(at<u8 *>(object, 8), 4), 0);
    if (flag) {
        vcall<u32>((u8 *)(uintptr_t)0x00f85480, 0x14, (u8 *)(uintptr_t)0x00786de4);
        return;
    }
    for (u8 *node = at<u8 *>(at<u8 *>(object, 8), 4); node;) {
        u8 *next = at<u8 *>(node, 4);
        u8 *entry = at<u8 *>(node, 0);
        if (Crt().stricmp(at<const char *>(entry, 0x30), global<const char *>(0x00f85488)) == 0)
            at<u8 *>(object, 0xc) = entry;
        node = next;
    }
}

void __fastcall Class702da0_Dtor(u8 *object) {
    at<u32>(object, 0) = (u32)(uintptr_t)Class702da0_vtable;
    u8 *node = at<u8 *>(object, 0xc);
    at<u32>(object, 8) = 0x00702f10;
    at<u32>(object, 0x10) = 0;
    at<u32>(object, 0xc) = 0;
    while (node) {
        u8 *next = at<u8 *>(node, 4);
        GameHeap_Free(node);
        node = next;
    }
    at<u32>(object, 0x14) = 0;
    at<u32>(object, 0) = (u32)(uintptr_t)Class702d48_vtable;
    vcall<u32>(global<u8 *>(0x00f85614), 0x1c, object);
}

u32 __fastcall Object_MakeBlock13c(u8 *object) {
    if (at<u32>(object, 0x13c))
        return 0;   // (one already: 0, sic)
    at<u32>(object, 0x13c) = TrackedGlobal_Alloc(0x38);
    if (at<u32>(object, 0x13c)) {
        u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x60));
        at<u8 *>(at<u8 *>(object, 0x13c), 0x18) = memory ? Texture_Ctor(memory, nullptr, 1) : nullptr;
        u8 *const block = at<u8 *>(object, 0x13c);
        if (u8 *const texture = at<u8 *>(block, 0x18)) {
            if (Texture_Alloc(texture, nullptr, global<u32>(0x008bd4b0), 0))
                return 1;
            if (u8 *const again = at<u8 *>(at<u8 *>(object, 0x13c), 0x18)) {
                Texture_Release(again);   // (through its jump 0x590a70)
                GameHeap_Free(again);   // (the new one)
            }
            TrackedGlobal_Free(at<u32>(object, 0x13c));
        } else {
            TrackedGlobal_Free(reinterpret_cast<u32>(block));
        }
        at<u32>(object, 0x13c) = 0;
    }
    EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 2, 0xb0754000);
    return 0;
}

u32 __fastcall Object_FreeBlock13c(u8 *object) {
    if (!at<u32>(object, 0x13c))
        return 1;
    if (u8 *const texture = at<u8 *>(at<u8 *>(object, 0x13c), 0x18)) {
        Texture_Release(texture);
        if (u8 *const again = at<u8 *>(at<u8 *>(object, 0x13c), 0x18)) {
            Texture_Release(again);  // (through its jump 0x590a70: a second time, sic)
            GameHeap_Free(again);
        }
    }
    TrackedGlobal_Free(at<u32>(object, 0x13c));
    at<u32>(object, 0x13c) = 0;
    return 1;
}

u32 __fastcall Object_ReleaseSavedPairs(u8 *object) {
    if ((at<u32>(object, 4) & 0x20000000) != 0 && at<s32>(object, 0x12c) > 0) {
        u8 *pairs = at<u8 *>(object, 0x124);
        for (s32 i = at<s32>(object, 0x12c) - 1; i >= 0; i--)
            *at<u32 *>(pairs, 4 + i * 8) = at<u32>(pairs, i * 8);
        if (at<u8 *>(object, 0x124)) {
            GameHeap_Free(at<u8 *>(object, 0x124));
            at<u32>(object, 0x124) = 0;
        }
        if (u8 *elements = at<u8 *>(object, 0x128)) {
            CRT_VectorDtorIterator(elements, 0x10, at<s32>(elements - 4, 0), reinterpret_cast<void(__fastcall *)(void *)>(0x00575bf0));
            GameHeap_Free(elements - 4);
            at<u32>(object, 0x128) = 0;
        }
        at<u32>(object, 4) = at<u32>(object, 4) & 0xdfffffff;
    }
    at<u32>(object, 0x12c) = 0;
    return 1;
}

u32 __fastcall Object_MakeSavedPairs130(u8 *object, void *, u32 *textures, s32 count) {
    u32 *const pairs = static_cast<u32 *>(GameHeap_Alloc(count * 8));
    if (pairs)
        for (s32 i = 0; i < count * 2; i++) pairs[i] = 0;
    at<u32 *>(object, 0x130) = pairs;
    u32 *const block = static_cast<u32 *>(GameHeap_Alloc(count * 0x10 + 4));
    u8 *sets = nullptr;
    if (block) {
        *block = static_cast<u32>(count);
        sets = reinterpret_cast<u8 *>(block + 1);
        CRT_VectorCtorIterator(sets, 0x10, count, reinterpret_cast<void(__fastcall *)(void *)>(&TexSet_Ctor), reinterpret_cast<void *>((uintptr_t)0x00575bf0));
    }
    at<u8 *>(object, 0x134) = sets;
    if (!pairs || !sets) {
        if (!pairs && sets) {
            g_calls.Cd(0x00579b80, static_cast<u32>(reinterpret_cast<uintptr_t>(sets)));
            at<u32>(object, 0x134) = 0;
        }
        if (pairs) {
            g_calls.Cd(0x00579b80, static_cast<u32>(reinterpret_cast<uintptr_t>(pairs)));
            at<u32>(object, 0x130) = 0;
        }
        return 0;
    }
    at<s32>(object, 0x138) = count;
    for (s32 i = 0; i < count; i++) {
        const u32 table = at<u32>(at<u8 *>(object, 8), 0x50);
        u32 *const slot = reinterpret_cast<u32 *>(static_cast<uintptr_t>(table + textures[i] * 8 + 4));
        const u32 set = static_cast<u32>(reinterpret_cast<uintptr_t>(at<u8 *>(object, 0x134) + i * 0x10));
        if (!g_calls.Th(0x00575ee0, set, *slot)) {
            g_calls.Th(0x0057b2e0, static_cast<u32>(reinterpret_cast<uintptr_t>(object)));
            return 0;
        }
        u32 *const pair = at<u32 *>(object, 0x130) + i * 2;
        pair[1] = static_cast<u32>(reinterpret_cast<uintptr_t>(slot));
        pair[0] = *slot;
        *slot = static_cast<u32>(reinterpret_cast<uintptr_t>(at<u8 *>(object, 0x134) + i * 0x10));
        u32 locked = 0;
        if (const u8 *const model = at<u8 *>(object, 8))
            if (const u32 now = at<u32>(reinterpret_cast<const void *>(static_cast<uintptr_t>(at<u32>(model, 0x50))), textures[i] * 8 + 4))
                if (const u32 texture = g_calls.Th(0x00575f90, now, 0u)) locked = g_calls.Th(0x00590d40, texture);
        textures[i] = locked;
    }
    at<u32>(object, 4) |= 0x40000000;
    return 1;
}

u32 __fastcall Object_ReleaseSavedPairs130(u8 *object) {
    if (!(at<u32>(object, 4) & 0x40000000) || at<s32>(object, 0x138) <= 0)
        return 1;   // (the count left as it is)
    for (s32 i = 0; i < at<s32>(object, 0x138); i++) {
        const u32 *const pair = reinterpret_cast<const u32 *>(at<u8 *>(object, 0x130) + i * 8);
        if (pair[0])
            *reinterpret_cast<u32 *>((uintptr_t)pair[1]) = pair[0];
    }
    at<u32>(object, 4) &= 0xbfffffff;
    if (at<u8 *>(object, 0x130)) {
        GameHeap_Free(at<u8 *>(object, 0x130));
        at<u32>(object, 0x130) = 0;
    }
    if (u8 *elements = at<u8 *>(object, 0x134)) {
        CRT_VectorDtorIterator(elements, 0x10, at<s32>(elements - 4, 0), reinterpret_cast<void(__fastcall *)(void *)>(0x00575bf0));
        GameHeap_Free(elements - 4);
        at<u32>(object, 0x134) = 0;
    }
    at<u32>(object, 0x138) = 0;
    return 1;
}

u32 __fastcall Obj_ReleaseSevenIfMode1(u8 *object) {
    if (global<u32>(0x008bd524) == 1)
        return Obj_ReleaseSevenComMembers(object);
    return 1;
}
u32 __fastcall Object_RenderToBlock13c(u8 *object, void *, u8 *mesh) {
    FixedVerts_Pack(at<u8 *>(mesh, 0x28), at<u32>(mesh, 0x20), 0x20);
    SoftRenderer_ClearBuffer(global<u8 *>(0x00879dac));
    vcall<void>(global<u8 *>(0x00879acc), 0x14, mesh);
    if (global<u32>(0x008bd554) != 2)
        return 1;
    const u32 *const pixels = at<const u32 *>(global<u8 *>(0x00879dac), 8);
    u16 *out = reinterpret_cast<u16 *>(Texture_Lock(at<u8 *>(at<u8 *>(object, 0x13c), 0x18)));
    s32 width = gBatch_8bd4ac, count = global<s32>(0x008bd4b0);
    const s32 side = width / count;
    if (count <= 0)
        return 1;
    s32 row = 0;
    for (s32 by = 0;;) {
        if (count > 0) {
            s32 column = 0;
            for (s32 bx = 0;;) {
                s32 sum = 0;
                if (side > 0) {
                    const u32 stride = (u32)width;
                    const u32 *line = pixels + ((u32)width * (u32)row + (u32)column);
                    for (s32 y = side; y != 0; y--, line += stride) {
                        for (s32 x = 0; x < side; x++)
                            sum += (s32)((line[x] >> 16) & 0xff);
                    }
                }
                const s32 mean = sum / (s32)((u32)side * (u32)side);
                const u32 high = (u32)mean & 0xf0;
                *out++ = (u16)(((high | 0xff00) << 4) | (u32)(mean >> 4) | high);
                count = global<s32>(0x008bd4b0);
                width = gBatch_8bd4ac;
                column += side;
                if (++bx >= count)
                    break;
            }
        }
        row += side;
        if (++by >= count)
            break;
    }
    return 1;
}
}  // extern "C"

void VObj2_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" u32 __fastcall Object_UpdateShadow(u8 *self) {
    const auto bits = [](float f) { return *reinterpret_cast<u32 *>(&f); };
    u32 made = 0;
    if (!at<u32>(self, 0x13c)) {
        if (!g_calls.Th(0x0057b490, static_cast<u32>(reinterpret_cast<uintptr_t>(self))))
            return 0;
        made = 1;
    }
    const float *const light = global<u32>(0x008c1818) ? reinterpret_cast<const float *>((uintptr_t)0x008c17c8) : nullptr;
    if (!light || light[1] >= 0.0f)   // 0x701abc (a NaN goes on: fcom's C0)
        return 1;
    const u8 *const source = at<u8 *>(self, 8) + 0x80;
    u8 *const block = at<u8 *>(self, 0x13c);
    if (!source) {
        at<u32>(self, 4) &= ~8u;
    } else if (block) {
        std::memcpy(block + 0x20, source, 0x18);
        at<u32>(self, 4) |= 8;
    }
    for (u32 low = 0x20; low <= 0x28; low += 4) {
        if (at<float>(block, low) > at<float>(block, low + 0xc)) {
            const u32 was = at<u32>(block, low);
            at<u32>(block, low) = at<u32>(block, low + 0xc);
            at<u32>(block, low + 0xc) = was;
        }
    }

    // The box grown by 1 in 100 of its size each way, x87 as the original keeps it: x's growth and the three ends unrounded while they last.
    const float kGrow = 0.01f;   // 0x701ac0
    const X87Reg growY = (X87Reg(at<float>(block, 0x30)) - at<float>(block, 0x24)) * kGrow;
    const X87Reg growX = (X87Reg(at<float>(block, 0x2c)) - at<float>(block, 0x20)) * kGrow;
    const X87Reg growZ = (X87Reg(at<float>(block, 0x34)) - at<float>(block, 0x28)) * kGrow;
    const X87Reg lowY = X87Reg(at<float>(block, 0x24)) - growY;
    const float growXf = (float)growX, growZf = (float)growZ;
    const float lowX = (float)(X87Reg(at<float>(block, 0x20)) - growXf);
    const float highX = (float)(growX + at<float>(block, 0x2c));
    const float highY = (float)(growY + at<float>(block, 0x30));
    const X87Reg span = lowY - highY;
    const float lowZ = (float)(X87Reg(at<float>(block, 0x28)) - growZf);
    const float highZ = (float)(growZ + at<float>(block, 0x34));
    at<u32>(block, 4) = bits((float)lowY);
    at<u32>(block, 0) = bits(lowX);
    at<u32>(block, 8) = bits(lowZ);
    at<u32>(block, 0xc) = bits(highX);
    at<u32>(block, 0x10) = bits(highY);
    at<u32>(block, 0x14) = bits(highZ);

    // Its shadow along the light: the far corners moved to the low y.
    const X87Reg along = span / light[1];
    const u32 shadowY = bits((float)(along * light[1] + highY));
    const X87Reg alongZ = along * light[2];
    const float moveZ = (float)alongZ;
    const u32 nearZ = bits((float)(alongZ + lowZ));
    const X87Reg alongX = along * light[0];
    const float moveX = (float)alongX;
    Box_Extend(reinterpret_cast<u32 *>(block), nullptr, bits((float)(alongX + lowX)), shadowY, nearZ);
    const u32 farZ = bits((float)(X87Reg(moveZ) + highZ));
    Box_Extend(reinterpret_cast<u32 *>(block), nullptr, bits((float)(X87Reg(moveX) + highX)), shadowY, farZ);

    if (block && at<s32>(block, 0x1c) > global<s32>(0x008bcc40)) {
        at<u32>(block, 0x1c) = global<u32>(0x008bcc40) + gVobj2_8bd4b4;
        return 1;
    }

    // The matrix to the block's texture: 1 over the box's size (0 for none, or a NaN), then flipped.
    const X87Reg sizeX = X87Reg(at<float>(block, 0xc)) - at<float>(block, 0);
    const float sizeZ = (float)(X87Reg(at<float>(block, 0x14)) - at<float>(block, 8));
    const float scaleX = sizeX.v != 0.0 && sizeX.v == sizeX.v ? (float)(X87Reg(1.0f) / sizeX) : 0.0f;   // 0x701ab8
    const float scaleZ = sizeZ != 0.0f && sizeZ == sizeZ ? (float)(X87Reg(1.0f) / sizeZ) : 0.0f;
    const X87Reg slopeX = X87Reg(light[0]) / light[1];
    const float cornerX = at<float>(block, 0), cornerZ = at<float>(block, 8);
    const u32 pair[2] = {bits(moveX), at<u32>(at<u8 *>(self, 8), 0x78)};   // (Model_AddTree's model: its +4 the model)
    float matrix[16] = {}, scale[16] = {}, flip[16] = {};
    matrix[0] = matrix[10] = matrix[15] = scale[15] = 1.0f;
    matrix[4] = (float)X87Reg(-slopeX.v);
    matrix[6] = (float)X87Reg(-(X87Reg(light[2]) / light[1]).v);
    scale[0] = scaleX;
    scale[10] = scaleZ;
    u8 &guard = gPointsprites_fb33e0;
    float *const product = reinterpret_cast<float *>((uintptr_t)0x00fb3400);   // (a function-local static, an identity)
    if (!(guard & 1)) {
        guard |= 1;
        std::memset(product, 0, 0x40);
        product[0] = product[5] = product[10] = product[15] = 1.0f;
        g_calls.Cd(0x0067a7d0, 0x005d1750u);
    }
    u8 *const transformer = global<u8 *>(0x0087b1d8);
    const auto multiply = reinterpret_cast<void(__cdecl *)(u8 *, const float *, const float *, float *)>((uintptr_t)(*reinterpret_cast<u32 **>(transformer))[6]);
    multiply(transformer, matrix, scale, product);
    std::memcpy(matrix, product, 0x40);
    const float y = at<float>(block, 4);
    matrix[12] = (float)X87Reg(-(X87Reg(y) * matrix[4] + X87Reg(cornerX) * scaleX).v);
    matrix[14] = (float)X87Reg(-(X87Reg(matrix[6]) * y + X87Reg(cornerZ) * scaleZ).v);
    flip[0] = -1.0f;
    flip[5] = flip[10] = flip[15] = 1.0f;
    multiply(transformer, matrix, flip, product);
    std::memcpy(matrix, product, 0x40);
    matrix[12] = (float)(X87Reg(product[12]) - -1.0f);   // 0x701ac4

    const u32 batch = g_calls.Th(0x00565240, global<u32>(0x00879ab0), 2u, 4u, 0x1c4u, 0x1256u, 0u);
    if (!batch)
        return 1;
    const u32 vertices = g_calls.Th(0x00582640, global<u32>(0x008bcbc4), 0x2000u);
    if (!vertices)
        return 1;
    const u32 me = static_cast<u32>(reinterpret_cast<uintptr_t>(self));
    if (!g_calls.Th(0x0057b610, me, pair, matrix, batch, vertices))
        return 0;
    if (!g_calls.Th(0x0057b6e0, me, batch))
        return 0;
    g_calls.Th(0x0058dc50, gTexcache_8c4b44, at<u32>(block, 0x18));
    if (made) {
        const u32 interval = gVobj2_8bd4b4;
        const u32 turn = ++gVobj2_8bd4b8;
        at<u32>(block, 0x1c) = ((interval - 1) & turn) + global<u32>(0x008bcc40) + interval;
    }
    return 1;
}
