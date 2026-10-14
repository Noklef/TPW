// The file library's reference-counted objects, the stream base and the handle class: see
// handles.h.
#include "handles.h"

#include "../core/vcall.h"

namespace {

void *const *const kStreamVtable = reinterpret_cast<void *const *>(0x00703100);
void *const *const kHandleVtable = reinterpret_cast<void *const *>(0x00703180);

const HandlesEnv kGameEnv = {
    reinterpret_cast<s32(__stdcall *const *)(s32 *)>(0x006fd0c0),
    reinterpret_cast<s32(__stdcall *const *)(s32 *)>(0x006fd12c),
    reinterpret_cast<void *(__cdecl *)(u32)>(0x0067acc0),
    reinterpret_cast<void(__cdecl *)(void *)>(0x0067acb0),
    reinterpret_cast<RefObject2 *(__fastcall *)(void *)>(0x00619950),
    reinterpret_cast<s32 *(__fastcall *)(RefObject2 *, void *, s32 *, u32)>(0x00619d70),
    reinterpret_cast<RefObject2 *(__fastcall *)(void *)>(0x0061a060),
    reinterpret_cast<s32 *(__fastcall *)(RefObject2 *, void *, s32 *, u32, u32)>(0x0061a250),
};
const HandlesEnv *g_env = &kGameEnv;

// The release both counted classes share: the hook at `teardown`, the link, the object.
template <typename T>
s32 release(T *o, u32 teardown, s32 (*releaseLink)(T *)) {
    if ((*g_env->decrement)(&o->refs) > 0) return *static_cast<volatile s32 *>(&o->refs);
    vcall<void>(o, teardown);
    if (o->link) {
        releaseLink(o->link);
        o->link = nullptr;
    }
    if (o) vcall<void *>(o, 0, 1u);
    return 0;
}
s32 release1(RefObject *o) { return RefObject_Release(o); }
s32 release2(RefObject2 *o) { return RefObject2_Release(o); }

// A handle method: the object's slot, or -1 without one.
template <typename... A>
s32 *forward(Handle *h, u32 offset, s32 *result, A... args) {
    if (!h->object) {
        *result = -1;
        return result;
    }
    vcall<s32 *>(h->object, offset, result, args...);
    return result;
}

}  // namespace

void Handles_SetEnv(const HandlesEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

s32 __fastcall RefObject_AddRef(RefObject *o) { return (*g_env->increment)(&o->refs); }

s32 __fastcall RefObject2_AddRef(RefObject2 *o) { return (*g_env->increment)(&o->refs); }

s32 __fastcall RefObject_Release(RefObject *o) { return release(o, 0x24, &release1); }

s32 __fastcall RefObject2_Release(RefObject2 *o) { return release(o, 0x30, &release2); }

void __fastcall RefObject_SetLink(RefObject *o, void *, RefObject *link) {
    if (o->link) vcall<void>(link, 0x28, o->link);
    o->link = link;
}

void __fastcall RefObject2_SetLink(RefObject2 *o, void *, RefObject2 *link) {
    if (o->link) vcall<void>(link, 0x2c, o->link);
    o->link = link;
}

StreamBase *__fastcall StreamBase_Ctor(StreamBase *s) {
    s->source = nullptr;
    s->vtable = kStreamVtable;
    return s;
}

StreamBase *__fastcall StreamBase_CopyCtor(StreamBase *s, void *, const StreamBase *other) {
    s->source = other->source;
    s->vtable = kStreamVtable;
    RefObject_AddRef(other->source);
    return s;
}

void __fastcall StreamBase_Dtor(StreamBase *s) {
    s->vtable = kStreamVtable;
    StreamBase_Close(s);
}

StreamBase *__fastcall StreamBase_DeletingDtor(StreamBase *s, void *, u32 flags) {
    StreamBase_Dtor(s);
    if (flags & 1) g_env->free(s);
    return s;
}

void __fastcall StreamBase_Close(StreamBase *s) {
    if (s->source) {
        RefObject_Release(s->source);
        s->source = nullptr;
    }
}

void __fastcall StreamBase_Attach(StreamBase *s, void *, RefObject *source) {
    if (!source) return;
    RefObject_AddRef(source);
    if (s->source) vcall<void>(source, 0x28, s->source);
    s->source = source;
}

s32 __fastcall StreamBase_Read(StreamBase *s, void *, void *buffer, s32 count) {
    return s->source ? vcall<s32>(s->source, 4, buffer, count) : -1;
}

s32 __fastcall StreamBase_Write(StreamBase *s, void *, const void *buffer, s32 count) {
    return s->source ? vcall<s32>(s->source, 8, buffer, count) : -1;
}

s32 __fastcall StreamBase_Seek(StreamBase *s, void *, s32 offset, u32 origin) {
    return s->source ? vcall<s32>(s->source, 0xc, offset, origin) : -1;
}

s32 __fastcall StreamBase_Vfn5(StreamBase *s) { return s->source ? vcall<s32>(s->source, 0x10) : -1; }

s32 __fastcall StreamBase_Tell(StreamBase *s) { return s->source ? vcall<s32>(s->source, 0x14) : -1; }

s32 __fastcall StreamBase_Vfn7(StreamBase *s) { return s->source ? vcall<s32>(s->source, 0x18, 0u, 0u) : 0; }

s32 *__fastcall StreamBase_Vfn8(StreamBase *s, void *, s32 *result) {
    if (!s->source) {
        *result = -1;
        return result;
    }
    vcall<s32 *>(s->source, 0x20, result);
    return result;
}

void __fastcall FileStream_Dtor(StreamBase *s) { StreamBase_Dtor(s); }

StreamBase *__fastcall FileStream_DeletingDtor(StreamBase *s, void *, u32 flags) {
    FileStream_Dtor(s);
    if (flags & 1) g_env->free(s);
    return s;
}

Handle *__fastcall Handle_Ctor(Handle *h) {
    h->object = nullptr;
    h->vtable = kHandleVtable;
    return h;
}

Handle *__fastcall Handle_CopyCtor(Handle *h, void *, const Handle *other) {
    h->vtable = kHandleVtable;
    h->object = nullptr;
    if (other != h && other->object) Handle_Set(h, nullptr, other->object);
    return h;
}

void __fastcall Handle_Dtor(Handle *h) {
    h->vtable = kHandleVtable;
    Handle_Reset(h);
}

Handle *__fastcall Handle_DeletingDtor(Handle *h, void *, u32 flags) {
    Handle_Dtor(h);
    if (flags & 1) g_env->free(h);
    return h;
}

Handle *__fastcall Handle_Assign(Handle *h, void *, const Handle *other) {
    if (other != h) {
        vcall<void>(h, 0);
        Handle_Set(h, nullptr, other->object);
    }
    return h;
}

void __fastcall Handle_Reset(Handle *h) {
    if (h->object) {
        RefObject2_Release(h->object);
        h->object = nullptr;
    }
}

void __fastcall Handle_Set(Handle *h, void *, RefObject2 *object) {
    RefObject2_AddRef(object);
    if (h->object) vcall<void>(object, 0x2c, h->object);
    h->object = object;
}

s32 *__fastcall Handle_Vfn1(Handle *h, void *, s32 *result, u32 a, u32 b) { return forward(h, 8, result, a, b); }

s32 *__fastcall Handle_Vfn2(Handle *h, void *, s32 *result, u32 a) { return forward(h, 0xc, result, a); }

s32 *__fastcall Handle_Vfn3(Handle *h, void *, s32 *result, u32 a, u32 b, u32 c, u32 d) {
    return forward(h, 4, result, a, b, c, d);
}

s32 *__fastcall Handle_Vfn4(Handle *h, void *, s32 *result, u32 a, u32 b) { return forward(h, 0x10, result, a, b); }

s32 *__fastcall Handle_Vfn5(Handle *h, void *, s32 *result, u32 a, u32 b, u32 c) {
    return forward(h, 0x14, result, a, b, c);
}

u32 __fastcall Handle_Vfn6(Handle *h) { return h->object ? vcall<u32>(h->object, 0x18) : 0; }

s32 *__fastcall Handle_Vfn7(Handle *h, void *, s32 *result, u32 a, u32 b) { return forward(h, 0x1c, result, a, b); }

s32 *__fastcall Handle_Vfn8(Handle *h, void *, s32 *result, u32 a) { return forward(h, 0x20, result, a); }

s32 *__fastcall Handle_Vfn9(Handle *h, void *, s32 *result, u32 a) { return forward(h, 0x24, result, a); }

s32 *__fastcall Handle_Vfn10(Handle *h, void *, s32 *result, u32 a, u32 b) { return forward(h, 0x28, result, a, b); }

s32 *__fastcall Handle_Open28(Handle *h, void *, s32 *result, u32 a) {
    vcall<void>(h, 0);
    void *const block = g_env->alloc(0x28);
    RefObject2 *const object = block ? g_env->ctor28(block) : nullptr;
    s32 r;
    if (*g_env->open28(object, nullptr, &r, a) < 0) {
        if (object) vcall<void *>(object, 0, 1u);
        *result = -1;
        return result;
    }
    Handle_Set(h, nullptr, object);
    *result = 0;
    return result;
}

s32 *__fastcall Handle_OpenE0(Handle *h, void *, s32 *result, u32 a, u32 b) {
    vcall<void>(h, 0);
    void *const block = g_env->alloc(0xe0);
    RefObject2 *const object = block ? g_env->ctorE0(block) : nullptr;
    s32 r;
    if (*g_env->openE0(object, nullptr, &r, a, b) < 0) {
        if (object) vcall<void *>(object, 0, 1u);
        *result = -1;
        return result;
    }
    Handle_Set(h, nullptr, object);
    *result = 0;
    return result;
}

}  // extern "C"
