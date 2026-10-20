// The game's pointer list: see ptrlist.h.
#include "ptrlist.h"

#include <cstdint>

#include "../gfx/spriteframe.h"
#include "../render/engineerrors.h"
#include "../render/renderer.h"
#include "gameheap.h"
#include "liveobjects.h"
#include "stdlist.h"
#include "vcall.h"

namespace {

const PtrListEnv kGameEnv = {
    reinterpret_cast<void *(__cdecl *)(u32)>(static_cast<std::uintptr_t>(0x0067acc0)),
};
const PtrListEnv *g_env = &kGameEnv;

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

void PtrList_SetEnv(const PtrListEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

void __fastcall PtrList_Append(PtrList *l, void *, void *item) {
    PtrListNode *const n = static_cast<PtrListNode *>(g_env->alloc(sizeof(PtrListNode)));
    if (n) {
        n->next = nullptr;
        n->item = item;
        n->prev = nullptr;
    }
    n->prev = l->tail;
    if (!l->tail) {
        l->head = n;
        l->tail = n;
    } else {
        l->tail->next = n;
        l->tail = n;
    }
    ++l->count;
}

u32 *__fastcall ListCursor_Next(u32 *cursor, void *, u32 *out, u32) {
    const u32 node = *cursor;
    const u32 next = *reinterpret_cast<const u32 *>(node);
    *cursor = next;
    *out = node;
    return out;
}

void __fastcall PtrList_Insert(u8 *list, void *, u32 value, u8 **position) {
    u8 *const at_position = *position;
    u8 *const node = static_cast<u8 *>(GameHeap_Alloc(0xc));
    at<u32>(node, 4) = 0;
    at<u32>(node, 0) = value;
    at<u32>(node, 8) = 0;
    if (at_position == nullptr) {
        at<u8 *>(node, 8) = at<u8 *>(list, 8);
        if (at<u8 *>(list, 8) == nullptr) {
            at<u8 *>(list, 4) = node;
        } else {
            at<u8 *>(at<u8 *>(list, 8), 4) = node;
        }
        at<u8 *>(list, 8) = node;
    } else if (at_position == at<u8 *>(list, 4)) {
        at<u8 *>(node, 4) = at<u8 *>(list, 4);
        if (at<u8 *>(list, 4) == nullptr) {
            at<u8 *>(list, 8) = node;
        } else {
            at<u8 *>(at<u8 *>(list, 4), 8) = node;
        }
        at<u8 *>(list, 4) = node;
    } else {
        at<u8 *>(node, 4) = at_position;
        at<u8 *>(node, 8) = at<u8 *>(at_position, 8);
        at<u8 *>(at<u8 *>(at_position, 8), 4) = node;
        at<u8 *>(at_position, 8) = node;
    }
    at<u32>(list, 0xc) = at<u32>(list, 0xc) + 1;
}

u32 __fastcall PtrArray_RemoveAndFree(u8 *object, void *, u32 item) {
    s32 index = at<s32>(object, 0x1c) - 1;
    u32 *items = at<u32 *>(object, 8);
    while (index >= 0 && items[index] != item)
        --index;
    if (index < 0)
        return 0;
    GameHeap_Free(reinterpret_cast<void *>(item));
    s32 count = at<s32>(object, 0x1c) - 1;
    at<s32>(object, 0x1c) = count;
    items = at<u32 *>(object, 8);
    items[index] = items[count];
    at<s32>(at<u8 *>(object, 4), 4) = at<s32>(object, 0x1c);
    return 1;
}

u8 *__fastcall PtrArray_AddSpriteFrame(u8 *object, void *, u32 image) {
    u8 *frame = static_cast<u8 *>(GameHeap_Alloc(0x44));
    if (frame) frame = SpriteFrame_Init(frame);
    if (!frame) {
        EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 2, 0xb0754000);
        return nullptr;
    }
    at<u32>(frame, 0x24) = 0xffffffff;
    at<u32>(frame, 0x10) = 0x58000;
    at<u32>(frame, 0x2c) = 0x41200000;  // 10.0f
    at<u32>(frame, 0x28) = image;
    at<u8 *>(at<u8 *>(object, 8), 4 * at<u32>(object, 0x1c)) = frame;
    const u32 count = at<u32>(object, 0x1c) + 1;
    u8 *const header = at<u8 *>(object, 4);
    at<u32>(object, 0x1c) = count;
    at<u32>(header, 4) = count;
    return frame;
}

void __fastcall ListRemovec_5d8a80(u8 *o, void *, u32 v0) {
    u8 *list = at<u8 *>(o, 0xc); u8 *node = at<u8 *>(list, 4); while (node && at<u32>(node, 0) != v0) node = at<u8 *>(node, 4); if (!node) return; if (node == at<u8 *>(list, 4)) at<u8 *>(list, 4) = at<u8 *>(node, 4); else at<u8 *>(at<u8 *>(node, 8), 4) = at<u8 *>(node, 4); if (node == at<u8 *>(list, 8)) at<u8 *>(list, 8) = at<u8 *>(node, 8); else at<u8 *>(at<u8 *>(node, 4), 8) = at<u8 *>(node, 8); at<u32>(list, 0xc) -= 1; GameHeap_Free(node);
}

void __fastcall ListRemove58_5da180(u8 *o, void *, u32 v0) {
    u8 *list = at<u8 *>(o, 0x58); u8 *node = at<u8 *>(list, 4); while (node && at<u32>(node, 0) != v0) node = at<u8 *>(node, 4); if (!node) return; if (node == at<u8 *>(list, 4)) at<u8 *>(list, 4) = at<u8 *>(node, 4); else at<u8 *>(at<u8 *>(node, 8), 4) = at<u8 *>(node, 4); if (node == at<u8 *>(list, 8)) at<u8 *>(list, 8) = at<u8 *>(node, 8); else at<u8 *>(at<u8 *>(node, 4), 8) = at<u8 *>(node, 8); at<u32>(list, 0xc) -= 1; GameHeap_Free(node);
}

void __fastcall ListRemove4_5da450(u8 *o, void *, u32 v0) {
    u8 *list = at<u8 *>(o, 0x4); u8 *node = at<u8 *>(list, 4); while (node && at<u32>(node, 0) != v0) node = at<u8 *>(node, 4); if (!node) return; if (node == at<u8 *>(list, 4)) at<u8 *>(list, 4) = at<u8 *>(node, 4); else at<u8 *>(at<u8 *>(node, 8), 4) = at<u8 *>(node, 4); if (node == at<u8 *>(list, 8)) at<u8 *>(list, 8) = at<u8 *>(node, 8); else at<u8 *>(at<u8 *>(node, 4), 8) = at<u8 *>(node, 8); at<u32>(list, 0xc) -= 1; GameHeap_Free(node);
}

void __fastcall ListRemove6c_5dd4b0(u8 *o, void *, u32 v0) {
    u8 *list = at<u8 *>(o, 0x6c); u8 *node = at<u8 *>(list, 4); while (node && at<u32>(node, 0) != v0) node = at<u8 *>(node, 4); if (!node) return; if (node == at<u8 *>(list, 4)) at<u8 *>(list, 4) = at<u8 *>(node, 4); else at<u8 *>(at<u8 *>(node, 8), 4) = at<u8 *>(node, 4); if (node == at<u8 *>(list, 8)) at<u8 *>(list, 8) = at<u8 *>(node, 8); else at<u8 *>(at<u8 *>(node, 4), 8) = at<u8 *>(node, 8); at<u32>(list, 0xc) -= 1; GameHeap_Free(node);
}

void __fastcall ListRemove8_5e15c0(u8 *o, void *, u32 v0) {
    u8 *list = at<u8 *>(o, 0x8); u8 *node = at<u8 *>(list, 4); while (node && at<u32>(node, 0) != v0) node = at<u8 *>(node, 4); if (!node) return; if (node == at<u8 *>(list, 4)) at<u8 *>(list, 4) = at<u8 *>(node, 4); else at<u8 *>(at<u8 *>(node, 8), 4) = at<u8 *>(node, 4); if (node == at<u8 *>(list, 8)) at<u8 *>(list, 8) = at<u8 *>(node, 8); else at<u8 *>(at<u8 *>(node, 4), 8) = at<u8 *>(node, 8); at<u32>(list, 0xc) -= 1; GameHeap_Free(node);
}

void __fastcall ListRemove18_5e4250(u8 *o, void *, u32 v0) {
    u8 *list = at<u8 *>(o, 0x18); u8 *node = at<u8 *>(list, 4); while (node && at<u32>(node, 0) != v0) node = at<u8 *>(node, 4); if (!node) return; if (node == at<u8 *>(list, 4)) at<u8 *>(list, 4) = at<u8 *>(node, 4); else at<u8 *>(at<u8 *>(node, 8), 4) = at<u8 *>(node, 4); if (node == at<u8 *>(list, 8)) at<u8 *>(list, 8) = at<u8 *>(node, 8); else at<u8 *>(at<u8 *>(node, 4), 8) = at<u8 *>(node, 8); at<u32>(list, 0xc) -= 1; GameHeap_Free(node);
}

void __fastcall ListRemove20080_5f42f0(u8 *o, void *, u32 v0) {
    u8 *list = at<u8 *>(o, 0x20080);
    u8 *node = at<u8 *>(list, 4);
    while (node && at<u32>(node, 0) != v0) node = at<u8 *>(node, 4);
    if (!node) return;
    if (node == at<u8 *>(list, 4)) at<u8 *>(list, 4) = at<u8 *>(node, 4);
    else at<u8 *>(at<u8 *>(node, 8), 4) = at<u8 *>(node, 4);
    if (node == at<u8 *>(list, 8)) at<u8 *>(list, 8) = at<u8 *>(node, 8);
    else at<u8 *>(at<u8 *>(node, 4), 8) = at<u8 *>(node, 8);
    at<u32>(list, 0xc) -= 1;
    GameHeap_Free(node);
}

void __fastcall SortedPtrList_Add(u8 *holder, void *, u8 *item) {
    u8 *const list = at<u8 *>(holder, 8);
    if (at<u32>(list, 0xc) == 0) {
        List_PushHead(list, nullptr, (u32)(uintptr_t)item);
        at<u8 *>(holder, 0x10) = item;
        return;
    }
    u8 *node = at<u8 *>(list, 4);
    while (node && at<s32>(at<u8 *>(node, 0), 0x24) < at<s32>(item, 0x24))
        node = at<u8 *>(node, 4);
    if (node)
        PtrList_Insert(list, nullptr, (u32)(uintptr_t)item, &node);
    else
        PtrList_Append(reinterpret_cast<PtrList *>(list), nullptr, item);
    at<u8 *>(holder, 0x10) = item;
}

u32 __fastcall PtrArray_RemoveImage(u8 *object, void *, u8 *image) {
    s32 index = at<s32>(object, 0x18) - 1;
    u8 **items = at<u8 **>(object, 0xc);
    while (index >= 0 && items[index] != image)
        --index;
    if (index < 0)
        return 0;
    SpriteImage_Release(image);
    if (image) {
        SpriteImage_Dtor(image);
        GameHeap_Free(image);
    }
    const s32 count = at<s32>(object, 0x18) - 1;
    at<s32>(object, 0x18) = count;
    items = at<u8 **>(object, 0xc);
    items[index] = items[count];
    return 1;
}

u32 __fastcall PtrArray_ReleaseAll(u8 *object, void *) {
    if (at<u32>(object, 0) & 1) {
        at<u32>(object, 0) &= ~1u;
        u8 *renderer = *reinterpret_cast<u8 **>((uintptr_t)0x0087a6d0);
        if (renderer) {
            u8 *owner = at<u8 *>(object, 4);
            const u32 part = owner ? (u32)(uintptr_t)owner + at<u32>(at<u8 *>(owner, 0), 4) : 0;
            Renderer_RemoveFromSlot(renderer, nullptr, part, 0x60);
        }
    }
    if (at<u32>(object, 0xc)) {
        for (s32 i = 0; i < at<s32>(object, 0x18); ++i) {
            SpriteImage_Release(at<u8 **>(object, 0xc)[i]);
            u8 *image = at<u8 **>(object, 0xc)[i];
            if (image) {
                SpriteImage_Dtor(image);
                GameHeap_Free(image);
            }
        }
        TrackedGlobal_Free(at<u32>(object, 0xc));
        at<u32>(object, 0xc) = 0;
        at<u32>(object, 0x18) = 0;
    }
    if (at<u32>(object, 4)) {
        for (s32 i = 0; i < at<s32>(object, 0x1c); ++i)
            GameHeap_Free(at<void **>(object, 8)[i]);
        u8 *owner = at<u8 *>(object, 4);
        if (owner)
            vcall<void>(owner + at<u32>(at<u8 *>(owner, 0), 4), 0, 1u);
        at<u32>(object, 4) = 0;
        at<u32>(object, 0x1c) = 0;
    }
    at<u32>(object, 8) = 0;
    at<u32>(object, 0x10) = 0;
    return 1;
}

u32 __fastcall PtrArray_Init(u8 *object, void *, u32 frames, u32 images) {
    u8 *const errors = reinterpret_cast<u8 *>((uintptr_t)0x00879ad0);
    if (at<u32>(object, 0x10)) {
        EngineErrors_Push(errors, nullptr, 2, 0xb0754015);
        return 0;
    }
    u8 *set = static_cast<u8 *>(GameHeap_Alloc(0x24));
    if (set) {   // (the sprite set's construction, inlined as in RendererPart_Ctor)
        at<u32>(set, 0) = 0x006ff318;
        at<u32>(set, 0x20) = 0x006fdce8;
        at<u32>(set, at<s32>(at<u8 *>(set, 0), 4)) = 0x006ff308;
        const s32 base = at<s32>(at<u8 *>(set, 0), 4);
        at<s32>(set, base - 4) = base - 0x20;
        u8 *const table = at<u8 *>(set, 0);
        at<u32>(set, 4) = 0;
        at<u32>(set, 8) = 0;
        at<u32>(set, 0x10) = 0;
        at<u32>(set, 0xc) = 0xffffffff;
        at<u32>(set, 0x14) = 0;
        at<u32>(set, 0x18) = 0;
        LiveObjects_Add(set + at<s32>(table, 4));
    }
    at<u8 *>(object, 4) = set;
    if (!set) {
        EngineErrors_Push(errors, nullptr, 2, 0xb0754000);
        return 0;
    }
    at<u32>(set, 4) = 0;
    const u32 array = TrackedArray_Alloc(at<u8 *>(object, 4), nullptr, frames);
    at<u32>(object, 8) = array;
    if (!array) {
        PtrArray_ReleaseAll(object, nullptr);
        return 0;
    }
    if (!images) images = frames;
    const u32 block = TrackedGlobal_Alloc(images * 4);
    at<u32>(object, 0xc) = block;
    if (!block) {
        PtrArray_ReleaseAll(object, nullptr);
        return 0;
    }
    at<u32>(object, 0x10) = frames;
    at<u32>(object, 0x14) = images;
    return 1;
}

u32 __fastcall PtrArray_SetVectorOf(const u8 *list, void *, u8 *key, u32 a, u32 b, u32 c) {
    const u32 *items = at<const u32 *>(list, 8);
    s32 index = (s32)at<u32>(list, 0x1c) - 1;
    if (index >= 0) {
        while (items[index] != (u32)(uintptr_t)key) {
            index--;
            if (index < 0)
                break;
        }
    }
    if (index < 0)
        return 0;
    at<u32>(key, 0x14) = a;
    at<u32>(key, 0x18) = b;
    at<u32>(key, 0x1c) = c;
    return 1;
}
}  // extern "C"
