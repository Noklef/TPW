#include "crtrealloc.h"

#include <cstdint>
#include <cstring>

#include "crt.h"
#include "crtsbh.h"

namespace {

u32 &word(u32 address) { return *reinterpret_cast<u32 *>((uintptr_t)address); }

u8 &byte_at(u32 address) { return *reinterpret_cast<u8 *>((uintptr_t)address); }

template <typename F> F import_slot(u32 slot) { return *reinterpret_cast<F *>((uintptr_t)slot); }

const u32 static_region = 0x0077eda0;
const u32 uncommitted = 0xffffffff;

s32 virtual_free(u32 address, u32 size, u32 kind) { return import_slot<s32(__stdcall *)(u32, u32, u32)>(0x006fd1b8)(address, size, kind); }

// The group of a page.
u32 group_of(u32 region, u32 page) {
    const s32 index = static_cast<s32>(page - word(region + 0x10)) >> 12;
    return region + 0x18 + static_cast<u32>(index) * 8;
}

// The bytes of the old block that the new one holds, copied, and the old one freed.
void move_block(void *to, const void *block, void *region, void *page, u8 *entry, u32 size) {
    const u32 old_size = static_cast<u32>(*entry) << 4;
    std::memcpy(to, block, old_size < size ? old_size : size);
    CRT_sbh_free_block(region, page, entry);
}

}  // namespace

void *__cdecl CRT_sbh_find_block(void *block, void **region, void **page) {
    const u32 at = static_cast<u32>(reinterpret_cast<uintptr_t>(block));
    u32 holder = static_region;
    while (!(at > word(holder + 0x10) && at < word(holder + 0x14))) {
        holder = word(holder);
        if (holder == static_region) return nullptr;
    }
    if (at & 0xf) return nullptr;
    if ((at & 0xfff) < 0x100) return nullptr;
    *region = reinterpret_cast<void *>((uintptr_t)holder);
    const u32 start = at & 0xfffff000;
    *page = reinterpret_cast<void *>((uintptr_t)start);
    return reinterpret_cast<void *>((uintptr_t)(start + 8 + ((at - start - 0x100) >> 4)));
}

void __cdecl CRT_sbh_release_region(void *region) {
    const u32 at = static_cast<u32>(reinterpret_cast<uintptr_t>(region));
    virtual_free(word(at + 0x10), 0, 0x8000);
    if (word(0x00780dc0) == at) word(0x00780dc0) = word(at + 4);
    if (at == static_region) {
        word(0x0077edb0) = uncommitted;
        return;
    }
    word(word(at + 4)) = word(at);
    word(word(at) + 4) = word(at + 4);
    import_slot<s32(__stdcall *)(u32, u32, u32)>(0x006fd210)(CRT_crtheap, 0, at);
}

void __cdecl CRT_sbh_decommit_pages(s32 count) {
    const u32 first = 0x0077eda4;
    u32 region = word(first);
    do {   // (sic: a region without memory does not advance: when it is not the first, this never ends)
        if (word(region + 0x10) != uncommitted) {
            s32 done = 0;
            u32 group = region + 0x2010;
            for (s32 offset = 0x3ff000; offset >= 0; offset -= 0x1000, group -= 8) {
                if (word(group) != 0xf0 || !virtual_free(word(region + 0x10) + static_cast<u32>(offset), 0x1000, 0x4000)) continue;
                word(group) = uncommitted;
                --CRT_sbh_empty_groups_fb1428;
                if (!word(region + 0xc) || word(region + 0xc) > group) word(region + 0xc) = group;
                ++done;
                if (!--count) break;
            }
            const u32 finished = region;
            region = word(region + 4);
            if (done && word(finished + 0x18) == uncommitted) {
                s32 index = 1;
                while (index < 0x400 && word(finished + 0x18 + static_cast<u32>(index) * 8) == uncommitted) ++index;
                if (index == 0x400) CRT_sbh_release_region(reinterpret_cast<void *>((uintptr_t)finished));
            }
        }
    } while (region != word(first) && count > 0);
}

void __cdecl CRT_sbh_free_block(void *region, void *page, void *entry) {
    const u32 group = group_of(static_cast<u32>(reinterpret_cast<uintptr_t>(region)), static_cast<u32>(reinterpret_cast<uintptr_t>(page)));
    u8 *const at = static_cast<u8 *>(entry);
    word(group) += *at;
    *at = 0;
    word(group + 4) = 0xf1;
    if (word(group) == 0xf0 && ++CRT_sbh_empty_groups_fb1428 == 0x20) CRT_sbh_decommit_pages(0x10);
}

s32 __cdecl CRT_sbh_resize_block(void *region, void *page, u8 *entry, u32 para) {
    const u32 start = static_cast<u32>(reinterpret_cast<uintptr_t>(page));
    const u32 at = static_cast<u32>(reinterpret_cast<uintptr_t>(entry));
    const u32 group = group_of(static_cast<u32>(reinterpret_cast<uintptr_t>(region)), start);
    const u32 old = *entry;
    if (old > para) {
        *entry = static_cast<u8>(para);
        word(group) += old - para;
        word(group + 4) = 0xf1;
        return 1;
    }
    if (old == para) return 0;
    const u32 end = at + para;
    if (end > start + 0xf8) return 0;
    for (u32 next = at + old; next < end; ++next) {
        if (byte_at(next)) return 0;
    }
    *entry = static_cast<u8>(para);
    if (at <= word(start) && end > word(start)) {
        if (end < start + 0xf8) {
            word(start) = end;
            u32 run = 0;
            while (!byte_at(end + run)) ++run;
            word(start + 4) = run;
        } else {
            word(start + 4) = 0;
            word(start) = start + 8;
        }
    }
    word(group) += old - para;
    return 1;
}

void *__cdecl CRT_realloc(void *block, u32 size) {
    if (!block) return CRT_malloc(size);
    if (!size) {
        Crt().free(block);
        return nullptr;
    }
    if (size <= 0xffffffe0) size = (size + 0xf) & 0xfffffff0;
    for (;;) {
        void *result = nullptr;
        if (size <= 0xffffffe0) {
            CrtInner().lock(9);
            void *region;
            void *page;
            u8 *const entry = static_cast<u8 *>(CRT_sbh_find_block(block, &region, &page));
            if (entry) {
                if (size < CRT_sbh_threshold) {
                    const u32 para = size >> 4;
                    if (CRT_sbh_resize_block(region, page, entry, para)) {
                        result = block;
                    } else {
                        result = CRT_sbh_alloc_block(para);
                        if (result) move_block(result, block, region, page, entry, size);
                    }
                }
                if (!result) {
                    result = import_slot<void *(__stdcall *)(u32, u32, u32)>(0x006fd224)(CRT_crtheap, 0, size);
                    if (result) move_block(result, block, region, page, entry, size);
                }
                CrtInner().unlock(9);
            } else {
                CrtInner().unlock(9);
                result = import_slot<void *(__stdcall *)(u32, u32, void *, u32)>(0x006fd258)(CRT_crtheap, 0, block, size);
            }
        }
        if (result || !CRT_newmode) return result;
        if (!CRT_callnewh(size)) return nullptr;
    }
}

u32 __cdecl CRT_msize(void *block) {
    CrtInner().lock(9);
    void *region, *page;
    const u8 *const entry = static_cast<const u8 *>(CRT_sbh_find_block(block, &region, &page));
    if (entry) {
        const u32 size = static_cast<u32>(*entry) << 4;
        CrtInner().unlock(9);
        return size;
    }
    CrtInner().unlock(9);
    return import_slot<u32(__stdcall *)(u32, u32, void *)>(0x006fd208)(CRT_crtheap, 0, block);   // HeapSize
}
