#include "buffers.h"

#include <cstdint>

#include "../files/file.h"
#include "crt.h"
#include "gameheap.h"
#include "stdwstring.h"
#include "string.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// One use less for a counted buffer whose count is the block's first byte, or the block freed.
void ReleaseCounted1(u8 *buffer) {
    const u8 uses = buffer[-1];
    if (uses == 0 || uses == 0xff)
        GameHeap_Free(buffer - 1);
    else
        buffer[-1] = (u8)(uses - 1);
}

u8 **Share(u8 *shared, u8 **out) {
    *out = shared;
    Counted_AddRef(shared);
    return out;
}

void SharedRelease(u8 *object, u32 elementSize, u32 destructorAt) {
    u8 *shared = at<u8 *>(object, 0x30);
    if (!shared)
        return;
    const u32 uses = at<u32>(shared, 0xc) - 1;
    at<u32>(shared, 0xc) = uses;
    if (uses == 0) {
        if (u8 *array = at<u8 *>(shared, 4)) {
            CRT_VectorDtorIterator(array, elementSize, at<s32>(array, (u32)-4), global<void(__fastcall *)(void *)>(destructorAt));
            GameHeap_Free(array - 4);
        }
        at<u32>(shared, 4) = 0;
        GameHeap_Free(shared);
    }
    at<u32>(object, 0x30) = 0;
}

void ResetAndRelease(u8 *object, u32 a, u32 b, u32 elementSize, u32 destructorAt) {
    u8 *shared = at<u8 *>(object, 0x30);
    at<u32>(object, 4) = 0xffffffffu;
    at<u32>(object, 0x10) = 0;
    at<u32>(object, 0x14) = 0;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0;
    at<u32>(object, 0x18) = 0;
    at<u32>(object, 0x1c) = a;
    at<u32>(object, 0x20) = b;
    if (!shared)
        return;
    const u32 uses = at<u32>(shared, 0xc) - 1;
    at<u32>(shared, 0xc) = uses;
    if (uses == 0) {
        if (u8 *array = at<u8 *>(shared, 4)) {
            CRT_VectorDtorIterator(array, elementSize, at<s32>(array, (u32)-4), global<void(__fastcall *)(void *)>(destructorAt));
            GameHeap_Free(array - 4);
        }
        at<u32>(shared, 4) = 0;
        GameHeap_Free(shared);
    }
    at<u32>(object, 0x30) = 0;
}

void ReleaseCounted(u8 *buffer) {
    u8 uses = buffer[-1];
    if (uses == 0 || uses == 0xff)
        GameHeap_Free(buffer - 2);
    else
        buffer[-1] = (u8)(uses - 1);
}

void ClearCounted(u8 *object) {
    at<u32>(object, 4) = 0;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0;
}

void ClearCountedVector(u8 *object, u32 begin, u32 end, u32 capacity) {
    u8 *entry = at<u8 *>(object, begin);
    u8 *last = at<u8 *>(object, end);
    while (entry != last) {
        if (at<u8 *>(entry, 4))
            ReleaseCounted(at<u8 *>(entry, 4));
        at<u32>(entry, 4) = 0;
        at<u32>(entry, 8) = 0;
        at<u32>(entry, 0xc) = 0;
        entry += 0x10;
    }
    GameHeap_Free(at<void *>(object, begin));
    at<u32>(object, begin) = 0;
    at<u32>(object, end) = 0;
    at<u32>(object, capacity) = 0;
}

}  // namespace

extern "C" {

void __fastcall CountedList_Clear(u8 *object) {
    u8 *sentinel = at<u8 *>(object, 4);
    u8 *node = at<u8 *>(sentinel, 0);
    while (node != sentinel) {
        u8 *erased = node;
        node = at<u8 *>(node, 0);
        at<u32>(at<u8 *>(erased, 4), 0) = at<u32>(erased, 0);
        at<u32>(at<u8 *>(erased, 0), 4) = at<u32>(erased, 4);
        CountedBuf_Reset(erased + 8, nullptr, 1);
        GameHeap_Free(erased);
        at<u32>(object, 8) -= 1;
    }
    GameHeap_Free(at<void *>(object, 4));
    at<u32>(object, 4) = 0;
    at<u32>(object, 8) = 0;
}

u8 *__fastcall CountedVector14_Deleter(u8 *object, void *, u32 flags) {
    CountedVector14_Clear(object);
    if (flags & 1)
        GameHeap_Free(object);
    return object;
}

void __stdcall CountedVector14_ClearRange(u8 *first, u8 *last) {
    for (u8 *element = first; element != last; element += 0x24)
        CountedVector14_Clear(element);
}

void __fastcall CountedList_ClearRelease(u8 *object) {
    u8 *sentinel = at<u8 *>(object, 4);
    u8 *node = at<u8 *>(sentinel, 0);
    while (node != sentinel) {
        u8 *erased = node;
        node = at<u8 *>(node, 0);
        at<u32>(at<u8 *>(erased, 4), 0) = at<u32>(erased, 0);
        at<u32>(at<u8 *>(erased, 0), 4) = at<u32>(erased, 4);
        CountedBuf_ReleaseAndClear(erased + 8);
        GameHeap_Free(erased);
        at<u32>(object, 8) -= 1;
    }
    GameHeap_Free(at<void *>(object, 4));
    at<u32>(object, 4) = 0;
    at<u32>(object, 8) = 0;
}

void __fastcall Counted2_Clear(u8 *object) {
    if (at<void *>(object, 0x20)) {
        GameHeap_Free(at<void *>(object, 0x20));
        at<u32>(object, 0x20) = 0;
    }
    if (at<u8 *>(object, 0x14))
        ReleaseCounted1(at<u8 *>(object, 0x14));
    at<u32>(object, 0x14) = 0;
    at<u32>(object, 0x18) = 0;
    at<u32>(object, 0x1c) = 0;
    if (at<u8 *>(object, 4))
        ReleaseCounted1(at<u8 *>(object, 4));
    at<u32>(object, 4) = 0;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0;
}

void __fastcall CountedPair_Clear(u8 *object) {
    if (at<u8 *>(object, 0x14))
        ReleaseCounted(at<u8 *>(object, 0x14));
    at<u32>(object, 0x14) = 0;
    at<u32>(object, 0x18) = 0;
    at<u32>(object, 0x1c) = 0;
    if (at<u8 *>(object, 4))
        ReleaseCounted1(at<u8 *>(object, 4));
    at<u32>(object, 4) = 0;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0;
}

void __stdcall CountedPair_ClearRange(u8 *first, u8 *last) {
    for (u8 *element = first; element != last; element += 0x24)
        CountedPair_Clear(element);
}

u32 __fastcall CountedPairVector_Clear(u8 *object) {
    CountedPair_ClearRange(at<u8 *>(object, 4), at<u8 *>(object, 8));
    GameHeap_Free(at<void *>(object, 4));
    at<u32>(object, 4) = 0;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0;
    return 0;
}

u8 *__fastcall CountedPairVector_Deleter(u8 *object, void *, u32 flags) {
    CountedPairVector_Clear(object);
    if (flags & 1)
        GameHeap_Free(object);
    return object;
}

u8 *__fastcall CountedPair_Deleter(u8 *object, void *, u32 flags) {
    CountedPair_Clear(object);
    if (flags & 1)
        GameHeap_Free(object);
    return object;
}

void __cdecl CountedPair_Destroy(u8 *element) { CountedPair_Clear(element); }

void __fastcall CountedPairVectorDeque_Clear(u8 *deque) {
    while (at<u32>(deque, 0x2c)) {
        u8 *const vector = at<u8 *>(deque, 0xc);
        at<u8 *>(deque, 0xc) = vector + 0x10;
        u8 *const end = at<u8 *>(vector, 8);
        for (u8 *entry = at<u8 *>(vector, 4); entry != end; entry += 0x24)
            CountedPair_Destroy(entry);
        GameHeap_Free(at<void *>(vector, 4));
        at<u32>(vector, 4) = 0;
        at<u32>(vector, 8) = 0;
        at<u32>(vector, 0xc) = 0;
        const u32 count = at<u32>(deque, 0x2c) - 1;
        at<u32>(deque, 0x2c) = count;
        if (count != 0 && at<u32>(deque, 0xc) != at<u32>(deque, 8))
            continue;
        // The block just finished is freed and the map moves on.
        u32 *const slot = at<u32 *>(deque, 0x10);
        at<u32 *>(deque, 0x10) = slot + 1;
        GameHeap_Free(reinterpret_cast<void *>(*slot));
        if (at<u32>(deque, 0x2c) == 0) {
            for (u32 offset = 0x4; offset <= 0x10; offset += 4)
                at<u32>(deque, offset) = 0;
            for (u32 offset = 0x14; offset <= 0x20; offset += 4)
                at<u32>(deque, offset) = 0;
            GameHeap_Free(at<void *>(deque, 0x24));
        } else {
            const u32 first = *at<u32 *>(deque, 0x10);
            at<u32>(deque, 0x4) = first;
            at<u32>(deque, 0x8) = first + 0x1000;
            at<u32>(deque, 0xc) = first;
        }
    }
}

void __fastcall Blocks7_Free(u8 *object) {
    for (s32 base = 0x60; base >= 0; base -= 0x10) {
        at<u32>(object, base) = 0;
        at<u32>(object, base + 4) = 0;
        at<u32>(object, base + 8) = 0;
        if (at<void *>(object, base + 0xc)) {
            GameHeap_Free(at<void *>(object, base + 0xc));
            at<u32>(object, base + 0xc) = 0;
        }
    }
}

u8 *__fastcall Blocks7_Ctor(u8 *object) {
    for (u32 offset = 0; offset < 0x70; offset += 4)
        at<u32>(object, offset) = 0;
    return object;
}

void __fastcall Ptr_FreeIfSet(u8 **ref) {
    if (*ref)
        GameHeap_Free(*ref);
}

void __fastcall Counted_AddRef(u8 *counted) {  // (the STL lock around it is left out)
    const u32 count = at<u32>(counted, 4);
    if (count != 0xffffffffu)
        at<u32>(counted, 4) = count + 1;
}

u8 **__fastcall Obj_ShareMember20(u8 *object, void *, u8 **out) { return Share(at<u8 *>(object, 0x20), out); }

u8 **__fastcall Obj_ShareMember34(u8 *object, void *, u8 **out) { return Share(at<u8 *>(object, 0x34), out); }

u8 *__fastcall Counted_CopyCtor(u8 *object, void *, const u8 *from) {
    u8 *shared = at<u8 *>(from, 0);
    at<u8 *>(object, 0) = shared;
    Counted_AddRef(shared);
    return object;
}

void __fastcall Shared_Release66(u8 *object) { SharedRelease(object, 0x66, 0x006fd504); }

void __fastcall Shared_Release10(u8 *object) { SharedRelease(object, 0x10, 0x006fd50c); }

void __fastcall Obj_FreeFourBuffers(u8 *object) {
    for (u32 offset = 0x4f4, i = 0; i < 4; ++i, offset += 4) {
        u8 *buffer = at<u8 *>(object, offset);
        if (buffer && buffer != reinterpret_cast<u8 *>(0x006fd880)) {
            Mem_GlobalFree(buffer);
            at<u32>(object, offset) = 0;
        }
    }
}

void __fastcall Obj_ResetAndRelease26(u8 *object, void *, u32 a, u32 b) { ResetAndRelease(object, a, b, 0x26, 0x006fd508); }

void __fastcall Obj_ResetAndRelease34(u8 *object, void *, u32 a, u32 b) { ResetAndRelease(object, a, b, 0x34, 0x006fd510); }

void __fastcall IntArray_SetSize(u8 *array, void *, s32 newSize, s32 growBy) {
    if (growBy != -1) at<s32>(array, 0xc) = growBy;
    if (newSize == 0) {
        if (at<void *>(array, 0)) GameHeap_Free(at<void *>(array, 0));
        at<u32>(array, 0) = 0;
        at<u32>(array, 8) = 0;
        at<u32>(array, 4) = 0;
        return;
    }
    if (at<u32 *>(array, 0) == nullptr) {
        u32 *const block = static_cast<u32 *>(GameHeap_Alloc((u32)newSize * 4));
        at<u32 *>(array, 0) = block;
        for (u32 i = 0; i < (u32)newSize; ++i) block[i] = 0;
        at<s32>(array, 8) = newSize;
        at<s32>(array, 4) = newSize;
        return;
    }
    const s32 max = at<s32>(array, 8);
    if (newSize <= max) {
        const s32 size = at<s32>(array, 4);
        if (newSize > size) {
            u32 *p = at<u32 *>(array, 0) + size;
            for (s32 n = newSize - size; n > 0; --n) *p++ = 0;
        }
        at<s32>(array, 4) = newSize;
        return;
    }
    s32 step = at<s32>(array, 0xc);
    if (step == 0) {
        step = at<s32>(array, 4);
        step = (step + ((step >> 31) & 7)) >> 3;
        if (step < 4) step = 4;
        else if (step > 0x400) step = 0x400;
    }
    s32 newMax = step + max;
    if (newSize >= newMax) newMax = newSize;
    u32 *const block = static_cast<u32 *>(GameHeap_Alloc((u32)newMax * 4));
    const s32 size = at<s32>(array, 4);
    const u32 *from = at<u32 *>(array, 0);
    for (s32 i = 0; i < size; ++i) block[i] = from[i];
    u32 *fill = block + size;
    for (s32 n = newSize - size; n > 0; --n) *fill++ = 0;
    GameHeap_Free(at<void *>(array, 0));
    at<s32>(array, 4) = newSize;
    at<u32 *>(array, 0) = block;
    at<s32>(array, 8) = newMax;
}

void __fastcall Buffer_FreeIfOwned(u8 *object) {
    if (at<u8>(object, 0) != 0)
        GameHeap_Free(at<void *>(object, 4));
}

u32 __fastcall Buffer_FreeAndClear(u8 *object) {
    GameHeap_Free(at<void *>(object, 4));
    at<u32>(object, 4) = 0;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0;
    return 0;
}

void __stdcall CountedEntries_Release(u8 *first, u8 *last) {
    for (u8 *entry = first; entry != last; entry += 0x10) {
        if (at<u8 *>(entry, 4))
            ReleaseCounted(at<u8 *>(entry, 4));
        ClearCounted(entry);
    }
}

void __fastcall CountedBuf_Reset(u8 *object, void *, u32 release) {
    if ((u8)release && at<u8 *>(object, 4))
        ReleaseCounted(at<u8 *>(object, 4));
    ClearCounted(object);
}

u8 *__fastcall CountedBuf_Deleter(u8 *object, void *, u32 flags) {
    if (at<u8 *>(object, 4))
        ReleaseCounted(at<u8 *>(object, 4));
    ClearCounted(object);
    if (flags & 1)
        GameHeap_Free(object);
    return object;
}

void __fastcall BufferC_FreeAndClear(u8 *object) {
    if (at<void *>(object, 0xc))
        GameHeap_Free(at<void *>(object, 0xc));
    at<u32>(object, 0) = 0;
    at<u32>(object, 4) = 0;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0;
}

void __fastcall CountedBuf_ReleaseAndClear(u8 *object) {
    u8 *buffer = at<u8 *>(object, 4);
    if (buffer) {
        const u8 uses = buffer[-1];
        if (uses == 0 || uses == 0xff)
            GameHeap_Free(buffer - 2);
        else
            buffer[-1] = (u8)(uses - 1);
    }
    at<u32>(object, 4) = 0;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0;
}

u8 *__fastcall Block50_Ctor(u8 *object) {
    u8 *block = static_cast<u8 *>(GameHeap_Alloc(0x50));
    if (block) {
        for (u32 i = 0; i < 10; ++i) {
            at<u32>(block, 8 * i) = 0;
            at<u32>(block, 8 * i + 4) = 0;
        }
        at<u8 *>(object, 0) = block;
        at<u8 *>(object, 4) = block;
        at<u8 *>(object, 8) = block + 0x280;
        return object;
    }
    at<u32>(object, 0) = 0;
    at<u32>(object, 4) = 0;
    at<u32>(object, 8) = 0x280;
    return object;
}

void __fastcall CountedVector14_Clear(u8 *object) { ClearCountedVector(object, 0x14, 0x18, 0x1c); }

void __fastcall CountedVector4_Clear(u8 *object) { ClearCountedVector(object, 4, 8, 0xc); }

u8 *__fastcall CountedVector14_Assign(u8 *object, void *, const u8 *x) {
    for (u32 offset = 0; offset < 0x10; offset += 4) at<u32>(object, offset) = at<u32>(x, offset);
    StdWStringVector_Assign(object + 0x10, nullptr, x + 0x10);  // (inlined; nothing when object is x)
    at<u32>(object, 0x20) = at<u32>(x, 0x20);
    return object;
}

u8 *__cdecl CountedVector14s_Copy(const u8 *first, const u8 *last, u8 *dest) {
    for (; first != last; first += 0x24, dest += 0x24) CountedVector14_Assign(dest, nullptr, first);
    return dest;
}

u8 *__fastcall CountedVector14_CtorCopy(u8 *object, void *, const u8 *x) {
    for (u32 offset = 0; offset < 0x10; offset += 4) at<u32>(object, offset) = at<u32>(x, offset);
    at<u8>(object, 0x10) = at<u8>(x, 0x10);
    const StdWString *const xFirst = at<const StdWString *>(x, 0x14);
    const s32 n = xFirst ? (s32)((uintptr_t)at<const StdWString *>(x, 0x18) - (uintptr_t)xFirst) >> 4 : 0;
    StdWString *const first = static_cast<StdWString *>(GameHeap_Alloc((u32)(n < 0 ? 0 : n) << 4));
    at<StdWString *>(object, 0x14) = first;
    const StdWString *const xLast = at<const StdWString *>(x, 0x18);
    StdWString *const last = StdWStrings_UninitCopy(object + 0x10, nullptr, at<const StdWString *>(x, 0x14), xLast, first);  // (inlined)
    at<StdWString *>(object, 0x18) = last;
    at<StdWString *>(object, 0x1c) = last;
    at<u32>(object, 0x20) = at<u32>(x, 0x20);
    return object;
}

void __fastcall CountedVector14Vector_InsertN(u8 *vector, void *, u8 *position, u32 n, const u8 *x) {
    // (the counts: the byte spans divided by 0x24 signed, truncating; compared unsigned)
    auto count = [](const u8 *first, const u8 *to) { return (u32)((s32)((uintptr_t)to - (uintptr_t)first) / 0x24); };
    u8 *&vFirst = at<u8 *>(vector, 4);
    u8 *&vLast = at<u8 *>(vector, 8);
    u8 *const last = vLast;
    const u32 bytes = n * 0x24;
    if (count(last, at<u8 *>(vector, 0xc)) < n) {
        const u32 size = vFirst ? count(vFirst, last) : 0;
        const u32 grown = (n < size ? size : n) + size;
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc((u32)((s32)grown < 0 ? 0 : grown) * 0x24));
        u8 *to = block;
        for (const u8 *from = vFirst; from != position; from += 0x24, to += 0x24)
            if (to) CountedVector14_CtorCopy(to, nullptr, from);
        u8 *p = to;
        for (u32 k = n; k; --k, p += 0x24)
            if (p) CountedVector14_CtorCopy(p, nullptr, x);
        to += bytes;
        for (const u8 *from = position, *const end = vLast; from != end; from += 0x24, to += 0x24)
            if (to) CountedVector14_CtorCopy(to, nullptr, from);
        for (u8 *q = vFirst, *const stop = vLast; q != stop; q += 0x24) CountedVector14_Clear(q);
        GameHeap_Free(vFirst);
        at<u8 *>(vector, 0xc) = block + grown * 0x24;
        const u32 old = vFirst ? count(vFirst, vLast) : 0;  // (the freed block's pointers)
        vFirst = block;
        vLast = block + (n + old) * 0x24;
        return;
    }
    if (count(position, last) < n) {
        u8 *to = position + bytes;
        for (const u8 *from = position; from != last; from += 0x24, to += 0x24)
            if (to) CountedVector14_CtorCopy(to, nullptr, from);
        u8 *p = vLast;
        for (u32 k = n - count(position, vLast); k; --k, p += 0x24)
            if (p) CountedVector14_CtorCopy(p, nullptr, x);
        for (u8 *q = position, *const stop = vLast; q != stop; q += 0x24) CountedVector14_Assign(q, nullptr, x);
        vLast += bytes;
        return;
    }
    if (!n) return;
    u8 *to = last;
    for (const u8 *from = last - bytes; from != last; from += 0x24, to += 0x24)
        if (to) CountedVector14_CtorCopy(to, nullptr, from);
    u8 *e = vLast;
    for (const u8 *from = e - bytes; from != position;) {  // (copy_backward)
        from -= 0x24;
        e -= 0x24;
        CountedVector14_Assign(e, nullptr, from);
    }
    for (u8 *q = position, *const stop = position + bytes; q != stop; q += 0x24) CountedVector14_Assign(q, nullptr, x);
    vLast += bytes;
}
}  // extern "C"
