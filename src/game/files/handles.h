// The file library's reference-counted objects and the classes that hold them:
// reimplementation of 0x618cd0-0x618de0, 0x619800-0x619860, the stream base class
// (0x5f7180-0x5f72c0, 0x4036a0/0x4036c0, 0x639100, FileStream's destructors 0x414d30 and
// 0x5fa950) and the handle class (0x5f8240-0x5f85d0).
//
// A RefObject (+0 vtable, +4 link, +8 count) is freed by the Release that takes its count to
// 0: vtable +0x24 first, then the linked object is released, then the deleting destructor.
// RefObject2 is the same with the count at +0xc and the hook at +0x30. Attaching an object
// to a holder that already has one hands the old one to the new (vtable +0x28 / +0x2c); the
// old one is not released there.
//
// A StreamBase (vtable 0x703100) forwards to the RefObject it holds at +4, -1 (or 0) without
// one: +8 read (vtable +4), +0xc write (+8), +0x10 seek (+0xc), +0x14 size (+0x10), +0x18
// position (+0x14), +0x1c (+0x18 with 0, 0), +0x20 (&result; +0x20). A Handle (vtable
// 0x703180) does the same for a RefObject2, its methods taking &result first (-1 without
// an object).
#pragma once

#include "../../../re/types.h"

struct RefObject {
    void *const *vtable;
    RefObject *link;
    s32 refs;
};
struct RefObject2 {
    void *const *vtable;
    RefObject2 *link;
    u32 unknown8;
    s32 refs;
};
struct StreamBase {
    void *const *vtable;  // 0x703100; FileStream 0x6fd930
    RefObject *source;
};
struct Handle {
    void *const *vtable;  // 0x703180
    RefObject2 *object;
};

extern "C" {
// 0x618d20 / 0x619800 (__thiscall): InterlockedIncrement of the count.
s32 __fastcall RefObject_AddRef(RefObject *o);
s32 __fastcall RefObject2_AddRef(RefObject2 *o);
// 0x618d30 / 0x619810 (__thiscall): InterlockedDecrement; above 0 the count (read again),
// else the object is torn down and 0.
s32 __fastcall RefObject_Release(RefObject *o);
s32 __fastcall RefObject2_Release(RefObject2 *o);
// 0x618d80 / 0x619860 (__thiscall; link): the new link gets the old one (vtable +0x28 /
// +0x2c), then replaces it.
void __fastcall RefObject_SetLink(RefObject *o, void *edx, RefObject *link);
void __fastcall RefObject2_SetLink(RefObject2 *o, void *edx, RefObject2 *link);
// The RefObject base class (vtable 0x703838: +4..+0x14 pure). 0x618cd0: link and count 0.
// 0x618d10: the vtable back. 0x618cf0 (flags; vtable +0): deleting destructor.
RefObject *__fastcall RefObject_Ctor(RefObject *o);
void __fastcall RefObject_Dtor(RefObject *o);
RefObject *__fastcall RefObject_DeletingDtor(RefObject *o, void *edx, u32 flags);
// Its defaults forward to the link, which must be there: 0x618da0 (a, b; vtable +0x18),
// 0x618dc0 (a; +0x1c), 0x618dd0 (+0x2c, the lock); 0x618de0 (&result; +0x20): -1.
u32 __fastcall RefObject_Vfn6(RefObject *o, void *edx, u32 a, u32 b);
u32 __fastcall RefObject_Vfn7(RefObject *o, void *edx, u32 a);
void *__fastcall RefObject_GetLock(RefObject *o);
s32 *__fastcall RefObject_Vfn8(RefObject *o, void *edx, s32 *result);

// StreamBase (__thiscall). 0x5f7180 / 0x5f71b0 (other): constructors, the copy adding a
// reference. 0x5f71e0: destructor. 0x5f7190 (flags; vtable +0): deleting destructor.
StreamBase *__fastcall StreamBase_Ctor(StreamBase *s);
StreamBase *__fastcall StreamBase_CopyCtor(StreamBase *s, void *edx, const StreamBase *other);
void __fastcall StreamBase_Dtor(StreamBase *s);
StreamBase *__fastcall StreamBase_DeletingDtor(StreamBase *s, void *edx, u32 flags);
// 0x5f71f0 (vtable +4): releases the source.
void __fastcall StreamBase_Close(StreamBase *s);
// 0x5f7240 (source): attaches it (a reference added); nullptr: nothing.
void __fastcall StreamBase_Attach(StreamBase *s, void *edx, RefObject *source);
// 0x5f72c0 (&result, a): a new 0x2c-byte source (0x618df0 with a) attached (nothing when the
// allocation fails), 0; -1 when the stream has one already.
s32 *__fastcall StreamBase_OpenMemory(StreamBase *s, void *edx, s32 *result, u32 a);
// The forwarders (vtable +8 to +0x20).
s32 __fastcall StreamBase_Read(StreamBase *s, void *edx, void *buffer, s32 count);
s32 __fastcall StreamBase_Write(StreamBase *s, void *edx, const void *buffer, s32 count);
s32 __fastcall StreamBase_Seek(StreamBase *s, void *edx, s32 offset, u32 origin);
s32 __fastcall StreamBase_Size(StreamBase *s);
s32 __fastcall StreamBase_Tell(StreamBase *s);
s32 __fastcall StreamBase_Vfn7(StreamBase *s);
s32 *__fastcall StreamBase_Vfn8(StreamBase *s, void *edx, s32 *result);
// 0x414d30 / 0x5fa950 (flags): FileStream's destructors (the base's).
void __fastcall FileStream_Dtor(StreamBase *s);
StreamBase *__fastcall FileStream_DeletingDtor(StreamBase *s, void *edx, u32 flags);

// Handle (__thiscall). 0x5f8240 / 0x5f8270 (other): constructors, the copy taking the
// other's object. 0x5f82a0 / 0x5f8250 (flags; vtable +0x2c): destructors.
Handle *__fastcall Handle_Ctor(Handle *h);
Handle *__fastcall Handle_CopyCtor(Handle *h, void *edx, const Handle *other);
void __fastcall Handle_Dtor(Handle *h);
Handle *__fastcall Handle_DeletingDtor(Handle *h, void *edx, u32 flags);
// 0x5f82b0 (other): Reset (vtable +0), then the other's object; itself: nothing.
Handle *__fastcall Handle_Assign(Handle *h, void *edx, const Handle *other);
// 0x5f82e0 (vtable +0): releases the object.
void __fastcall Handle_Reset(Handle *h);
// 0x5f8420 (object): holds it (a reference added, nullptr included).
void __fastcall Handle_Set(Handle *h, void *edx, RefObject2 *object);
// The forwarders (&result, ...): vtable +4 to +0x28 of the handle, each to the object's
// slot named in the definition.
s32 *__fastcall Handle_Vfn1(Handle *h, void *edx, s32 *result, u32 a, u32 b);
s32 *__fastcall Handle_Vfn2(Handle *h, void *edx, s32 *result, u32 a);
s32 *__fastcall Handle_Vfn3(Handle *h, void *edx, s32 *result, u32 a, u32 b, u32 c, u32 d);
s32 *__fastcall Handle_Vfn4(Handle *h, void *edx, s32 *result, u32 a, u32 b);
s32 *__fastcall Handle_Vfn5(Handle *h, void *edx, s32 *result, u32 a, u32 b, u32 c);
u32 __fastcall Handle_Vfn6(Handle *h);
s32 *__fastcall Handle_Vfn7(Handle *h, void *edx, s32 *result, u32 a, u32 b);
s32 *__fastcall Handle_Vfn8(Handle *h, void *edx, s32 *result, u32 a);
s32 *__fastcall Handle_Vfn9(Handle *h, void *edx, s32 *result, u32 a);
s32 *__fastcall Handle_Vfn10(Handle *h, void *edx, s32 *result, u32 a, u32 b);
// 0x5f8510 (&result, a) / 0x5f85d0 (&result, a, b): Reset, then a new 0x28-byte object
// (0x619950, opened by 0x619d70) / 0xe0-byte one (0x61a060, 0x61a250); held when that
// succeeds, else deleted and -1.
s32 *__fastcall Handle_Open28(Handle *h, void *edx, s32 *result, u32 a);
s32 *__fastcall Handle_OpenE0(Handle *h, void *edx, s32 *result, u32 a, u32 b);
}

// What the classes call in the game, so that they can run outside it.
struct HandlesEnv {
    s32(__stdcall *const *increment)(s32 *value);  // the import table's InterlockedIncrement
    s32(__stdcall *const *decrement)(s32 *value);  // InterlockedDecrement
    void *(__cdecl *alloc)(u32 size);              // Mem_Alloc (0x67acc0)
    void(__cdecl *free)(void *block);              // Mem_Free (0x67acb0)
    RefObject2 *(__fastcall *ctor28)(void *self);  // 0x619950
    s32 *(__fastcall *open28)(RefObject2 *o, void *edx, s32 *result, u32 a);  // 0x619d70
    RefObject2 *(__fastcall *ctorE0)(void *self);  // 0x61a060
    s32 *(__fastcall *openE0)(RefObject2 *o, void *edx, s32 *result, u32 a, u32 b);  // 0x61a250
    RefObject *(__fastcall *ctorMemory)(void *self, void *edx, u32 a);                 // 0x618df0
};
// The classes' calls go to env (nullptr: the game's).
void Handles_SetEnv(const HandlesEnv *env);
// The env in use (for the file library's other classes).
const HandlesEnv *Handles_GetEnv();
