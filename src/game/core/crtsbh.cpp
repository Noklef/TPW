#include "crtsbh.h"

#include <cstdint>
#include <cstring>

#include "crt.h"

namespace {

const u32 kStatic = 0x0077eda0;
const u32 kGroupsEnd = 0x2018;

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

template <typename F> F import_slot(u32 slot) { return *reinterpret_cast<F *>((uintptr_t)slot); }

u32 &word(u32 address) { return *reinterpret_cast<u32 *>((uintptr_t)address); }

u8 &byte_at(u32 address) { return *reinterpret_cast<u8 *>((uintptr_t)address); }

u32 virtual_alloc(u32 at, u32 size, u32 type, u32 protect) {
    return import_slot<u32(__stdcall *)(u32, u32, u32, u32)>(0x006fd1b4)(at, size, type, protect);
}

u32 heap_alloc(u32 flags, u32 size) { return import_slot<u32(__stdcall *)(u32, u32, u32)>(0x006fd224)(global<u32>(0x00fb3cac), flags, size); }

// The block at a map byte of the page.
u32 block_of(u32 map, u32 page) { return (map + 8) * 16 - page * 15; }

u32 free_run(u32 at) {
    u32 run = 1;
    while (!byte_at(at + run)) ++run;
    return run;
}

// para paragraphs at at, run of them free there: the page's start moves past them.
u32 take(u32 page, u32 at, u32 run, u32 para) {
    if (at + para < page + 0xf8) {
        word(page) = at + para;
        word(page + 4) = run - para;
    } else {
        word(page) = page + 8;
        word(page + 4) = 0;
    }
    byte_at(at) = static_cast<u8>(para);
    return block_of(at, page);
}

// The groups [from, end) with the pages from pages on: the first that has para paragraphs.
bool from_groups(u32 region, u32 from, u32 end, u32 pages, u32 para, u32 *block) {
    for (u32 group = from; group < end; group += 8, pages += 0x1000) {
        const u32 available = word(group);
        if (static_cast<s32>(available) < static_cast<s32>(para) || word(group + 4) <= para) continue;
        const u32 found = reinterpret_cast<u32>(CRT_sbh_alloc_block_from_page(reinterpret_cast<u8 *>((uintptr_t)pages), available, para));
        if (found) {
            global<u32>(0x00780dc0) = region;
            word(group) -= para;
            word(region + 8) = group;
            *block = found;
            return true;
        }
        word(group + 4) = para;
    }
    return false;
}

}  // namespace

extern "C" {

s32 __cdecl CRT_heap_init() {
    const u32 heap = import_slot<u32(__stdcall *)(u32, u32, u32)>(0x006fd1bc)(0, 0x1000, 0);   // HeapCreate
    word(0x00fb3cac) = heap;
    if (!heap) return 0;
    if (CRT_sbh_alloc_new_region()) return 1;
    import_slot<s32(__stdcall *)(u32)>(0x006fd1c0)(word(0x00fb3cac));   // HeapDestroy
    return 0;
}

void *__cdecl CRT_sbh_alloc_new_region() {
    u32 region = kStatic;
    if (global<u32>(kStatic + 0x10) != 0xffffffff) {
        region = heap_alloc(0, 0x2020);
        if (!region) return nullptr;
    }
    const u32 memory = virtual_alloc(0, 0x400000, 0x2000, 4);
    if (memory) {
        if (virtual_alloc(memory, 0x10000, 0x1000, 4)) {
            if (region == kStatic) {
                if (!word(kStatic)) word(kStatic) = kStatic;
                if (!word(kStatic + 4)) word(kStatic + 4) = kStatic;
            } else {
                word(region) = kStatic;
                word(region + 4) = word(kStatic + 4);
                word(kStatic + 4) = region;
                word(word(region + 4)) = region;
            }
            word(region + 0x14) = memory + 0x400000;
            word(region + 0x10) = memory;
            word(region + 8) = region + 0x18;
            word(region + 0xc) = region + 0x98;
            for (u32 i = 0; i < 0x400; ++i) {
                word(region + 0x18 + i * 8) = i < 0x10 ? 0xf0 : 0xffffffff;
                word(region + 0x1c + i * 8) = 0xf1;
            }
            std::memset(reinterpret_cast<void *>((uintptr_t)memory), 0, 0x10000);
            for (u32 page = memory; page < word(region + 0x10) + 0x10000; page += 0x1000) {
                word(page + 4) = 0xf0;
                word(page) = page + 8;
                byte_at(page + 0xf8) = 0xff;
            }
            return reinterpret_cast<void *>((uintptr_t)region);
        }
        import_slot<s32(__stdcall *)(u32, u32, u32)>(0x006fd1b8)(memory, 0, 0x8000);
    }
    if (region != kStatic) import_slot<s32(__stdcall *)(u32, u32, u32)>(0x006fd210)(global<u32>(0x00fb3cac), 0, region);
    return nullptr;
}

void *__cdecl CRT_sbh_alloc_block_from_page(u8 *page_bytes, u32 free_paragraphs, u32 para) {
    const u32 page = static_cast<u32>(reinterpret_cast<uintptr_t>(page_bytes));
    const u32 start = word(page);
    const u32 at_start = word(page + 4);
    const u32 limit = page + 0xf8;
    if (at_start >= para) {
        byte_at(start) = static_cast<u8>(para);
        if (start + para < limit) {
            word(page) += para;
            word(page + 4) -= para;
        } else {
            word(page + 4) = 0;
            word(page) = page + 8;
        }
        return reinterpret_cast<void *>((uintptr_t)block_of(start, page));
    }
    u32 left = free_paragraphs;
    u32 at = start;
    if (byte_at(start + at_start)) at = start + at_start;
    while (at + para < limit) {
        const u8 used = byte_at(at);
        if (used) {
            at += used;
            continue;
        }
        const u32 run = free_run(at);
        if (run >= para) return reinterpret_cast<void *>((uintptr_t)take(page, at, run, para));
        if (at == start) {
            word(page + 4) = run;
        } else {
            left -= run;
            if (left < para) return nullptr;
        }
        at += run;
    }
    at = page + 8;
    if (at >= start) return nullptr;
    do {
        if (at + para >= limit) return nullptr;
        const u8 used = byte_at(at);
        if (used) {
            at += used;
        } else {
            const u32 run = free_run(at);
            if (run >= para) return reinterpret_cast<void *>((uintptr_t)take(page, at, run, para));
            left -= run;
            if (left < para) return nullptr;
            at += run;
        }
    } while (at < start);
    return nullptr;
}

void *__cdecl CRT_sbh_alloc_block(u32 para) {
    u32 found = 0;
    u32 region = global<u32>(0x00780dc0);
    do {
        const u32 memory = word(region + 0x10);
        if (memory != 0xffffffff) {
            const u32 rover = word(region + 8);
            const u32 pages = ((static_cast<s32>(rover - region - 0x18) >> 3) << 12) + memory;
            if (from_groups(region, rover, region + kGroupsEnd, pages, para, &found)) return reinterpret_cast<void *>((uintptr_t)found);
            if (from_groups(region, region + 0x18, word(region + 8), word(region + 0x10), para, &found)) return reinterpret_cast<void *>((uintptr_t)found);
        }
        region = word(region);
    } while (region != global<u32>(0x00780dc0));
    region = kStatic;
    while (word(region + 0x10) == 0xffffffff || !word(region + 0xc)) {
        region = word(region);
        if (region == kStatic) {
            const u32 fresh = reinterpret_cast<u32>(CRT_sbh_alloc_new_region());
            if (!fresh) return nullptr;
            const u32 page = word(fresh + 0x10);
            byte_at(page + 8) = static_cast<u8>(para);
            global<u32>(0x00780dc0) = fresh;
            word(page) = page + para + 8;
            word(page + 4) = 0xf0 - para;
            word(fresh + 0x18) -= para & 0xff;
            return reinterpret_cast<void *>((uintptr_t)(page + 0x100));
        }
    }
    const u32 group = word(region + 0xc);
    const u32 page = ((static_cast<s32>(group - region - 0x18) >> 3) << 12) + word(region + 0x10);
    s32 count = 0;
    for (u32 entry = group; word(entry) == 0xffffffff && count < 0x10; entry += 8) ++count;
    if (virtual_alloc(page, static_cast<u32>(count) << 12, 0x1000, 4) != page) return nullptr;
    u32 entry = group;
    u32 fresh = page;
    for (s32 i = 0; i < count; ++i) {
        word(fresh + 4) = 0xf0;
        word(fresh) = fresh + 8;
        byte_at(fresh + 0xf8) = 0xff;
        word(entry) = 0xf0;
        word(entry + 4) = 0xf1;
        fresh += 0x1000;
        entry += 8;
    }
    const u32 end = region + kGroupsEnd;
    global<u32>(0x00780dc0) = region;
    while (entry < end && word(entry) != 0xffffffff) entry += 8;
    word(region + 0xc) = entry < end ? entry : 0;
    byte_at(page + 8) = static_cast<u8>(para);
    word(region + 8) = group;
    word(group) -= para;
    word(page + 4) -= para;
    word(page) = page + para + 8;
    return reinterpret_cast<void *>((uintptr_t)(page + 0x100));
}

s32 __cdecl CRT_callnewh(u32 size) {
    const auto handler = global<s32(__cdecl *)(u32)>(0x00fb154c);
    return handler && handler(size) ? 1 : 0;
}

void *__cdecl CRT_calloc(u32 count, u32 size) {
    u32 bytes = count * size;
    if (bytes <= 0xffffffe0) bytes = bytes ? (bytes + 0xf) & 0xfffffff0 : 0x10;
    for (;;) {
        u32 block = 0;
        if (bytes <= 0xffffffe0) {
            if (bytes <= global<u32>(0x00780dc4)) {
                CrtInner().lock(9);
                block = reinterpret_cast<u32>(CRT_sbh_alloc_block(bytes >> 4));
                CrtInner().unlock(9);
                if (block) {
                    std::memset(reinterpret_cast<void *>((uintptr_t)block), 0, bytes);
                    return reinterpret_cast<void *>((uintptr_t)block);
                }
            }
            block = heap_alloc(8, bytes);
        }
        if (block) return reinterpret_cast<void *>((uintptr_t)block);
        if (!global<u32>(0x00fb1548)) return nullptr;
        if (!CRT_callnewh(bytes)) return nullptr;
    }
}

void *__cdecl CRT_heap_alloc(u32 size) {
    const u32 bytes = (size + 0xf) & 0xfffffff0;
    if (bytes <= global<u32>(0x00780dc4)) {
        CrtInner().lock(9);
        void *const block = CRT_sbh_alloc_block(bytes >> 4);
        CrtInner().unlock(9);
        if (block) return block;
    }
    return reinterpret_cast<void *>((uintptr_t)heap_alloc(0, bytes));
}

void *__cdecl CRT_nh_malloc(u32 size, s32 mode) {
    if (size > 0xffffffe0) return nullptr;
    if (!size) size = 1;
    for (;;) {
        void *const block = size <= 0xffffffe0 ? CRT_heap_alloc(size) : nullptr;
        if (block || !mode) return block;
        if (!CRT_callnewh(size)) return nullptr;
    }
}

}
