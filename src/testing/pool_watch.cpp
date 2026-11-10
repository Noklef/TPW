#include "pool_watch.h"

#include <intrin.h>

#include <cstdint>
#include <cstring>

#include "../game/core/streampool.h"
#include "../hook/patch.h"
#include "log.h"

namespace tpw::testing {

namespace {

// The pools of src/game/core/streampool.cpp: the globals holding the lock and the list's first block, the blocks' size, where a block keeps the next.
struct Pool {
    const char *name;
    DWORD lock, list, size, next;
    DWORD seen;   // the list's first block at the last look (under the lock)
};
#define POOL(text, g, size, next) {text, (DWORD)(uintptr_t)&g.lock, (DWORD)(uintptr_t)&g.list, size, next, 0}
Pool g_pools[] = {
    POOL("StreamFilter", gStreampool_Filter_fb2068, 0x18, 0x10), POOL("LoopStream", gStreampool_Loop_fb2080, 0x24, 0x18),
    POOL("Stream70abf8", gStreampool_70abf8_fb208c, 0x14, 0x10), POOL("Stream70a6c8", gStreampool_70a6c8_fb20a4, 0x1c, 0x14),
    POOL("VoiceSample", gStreampool_VoiceSample_fb20bc, 0x40, 0x28), POOL("VoiceStream", gStreampool_VoiceStream_fb20c8, 0x5c, 0x58),
    POOL("SilenceStream", gStreampool_Silence_fb2074, 0x30, 0x28), POOL("FileSample", gStreampool_FileSample_fb20b0, 0x48, 0x44),
    POOL("Mp2StreamSample", gStreampool_Mp2Stream_fb26e4, 0x5c, 0x58), POOL("Mp2Sample", gStreampool_Mp2Sample_fb26d8, 0x40, 0x28),
    POOL("NotifyStream", gStreampool_Callback_fb26f0, 0x1c, 0x14), POOL("MemorySample", gStreampool_MemorySample_fb2098, 0x2c, 0x28),
};
#undef POOL

constexpr DWORD kFree = 0x67ad90, kFreeBody = 0x67ad96, kMemFree = 0x67acba, kMemAlloc = 0x67acc0, kNhMalloc = 0x67dcf0;
constexpr DWORD kLeaveSlot = 0x6fd130;
constexpr DWORD kGameText = 0x401000, kGameTextEnd = 0x6e7000;
constexpr int kMaxLines = 200;
constexpr DWORD kWalk = 0x10000;   // (a list with a cycle ends here)

// By the block's address, direct-mapped: a collision forgets the older block (a missed report, never a wrong one: the key is compared).
struct Note {
    DWORD block, caller;
};
constexpr DWORD kNotes = 1u << 16;
Note g_released[kNotes];   // who last put a block on a pool's list
Note g_freed[kNotes];      // who freed a block the heap has not given back through operator new
DWORD g_looped[kNotes];    // a block already reported as its own next

DWORD index_of(DWORD block) { return (block >> 4) & (kNotes - 1); }

DWORD g_dll_base, g_dll_size;
volatile LONG g_lines;

void(WINAPI *o_Leave)(void *);
void(__cdecl *o_free)(void *);

// "game 006C8310" or "dll+1F2C0".
void name_code(char *out, DWORD address) {
    if (address - g_dll_base < g_dll_size) {
        wsprintfA(out, "dll+%X", address - g_dll_base);
    } else {
        wsprintfA(out, "game %06X", address);
    }
}

bool is_code(DWORD v) { return (v >= kGameText && v < kGameTextEnd) || v - g_dll_base < g_dll_size; }

// The code addresses on the stack above `from`: the call chain, roughly.
void name_stack(char *out, const DWORD *from) {
    int found = 0;
    *out = 0;
    __try {
        for (int i = 0; i < 96 && found < 8; ++i) {
            if (is_code(from[i])) {
                char one[24];
                name_code(one, from[i]);
                lstrcatA(out, " ");
                lstrcatA(out, one);
                ++found;
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

bool may_log() { return InterlockedIncrement(&g_lines) <= kMaxLines; }

// Whether the list from `first` holds `block`; unreadable memory ends the walk.
bool on_list(const Pool &p, DWORD first, DWORD block) {
    __try {
        DWORD at = first;
        for (DWORD i = 0; at && i < kWalk; ++i) {
            if (at == block) return true;
            at = *reinterpret_cast<const DWORD *>(static_cast<uintptr_t>(at + p.next));
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
    return false;
}

// Under the pool's lock, just before it is left: a new first block whose next is the old first was just released.
void look(Pool &p, const DWORD *stack) {
    const DWORD first = *reinterpret_cast<const DWORD *>(static_cast<uintptr_t>(p.list));
    const DWORD old = p.seen;
    p.seen = first;
    if (!first) return;
    DWORD next = 0;
    __try {
        next = *reinterpret_cast<const DWORD *>(static_cast<uintptr_t>(first + p.next));
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return;
    }
    // The first block released again leaves the list's first as it was, its next itself. (Once per block: it stays so.)
    const bool on_itself = first == old && next == first && g_looped[index_of(first)] != first;
    if (first == old && !on_itself) return;
    if (next != old) return;   // (a block taken off, or the purge)
    if (on_itself) g_looped[index_of(first)] = first;
    // The release's own frame: the pool's Delete (block, size) was called with these, its return address just below them.
    DWORD caller = stack[0];
    const DWORD *chain = stack;
    __try {
        for (int i = 0; i < 32; ++i) {
            if (stack[i] == first && stack[i + 1] == p.size) {
                caller = stack[i - 1];
                chain = stack + i + 2;
                break;
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
    const DWORD at = index_of(first);
    char who[24], earlier[24], freed[24], more[8 * 24];
    if ((on_itself || on_list(p, old, first)) && may_log()) {
        name_code(who, caller);
        name_code(earlier, g_released[at].block == first ? g_released[at].caller : 0);
        name_stack(more, chain);
        log_line("[%6.1fs] pool watch: %s block %08X released twice, from %s (it went on the free list from %s); stack:%s", log_seconds(), p.name, first, who,
                 earlier, more);
    }
    if (g_freed[at].block == first && may_log()) {
        name_code(who, caller);
        name_code(freed, g_freed[at].caller);
        name_stack(more, chain);
        log_line("[%6.1fs] pool watch: %s block %08X released, already freed by %s, from %s; stack:%s", log_seconds(), p.name, first, freed, who, more);
    }
    g_released[at] = {first, caller};
}

void WINAPI watched_leave(void *section) {
    const DWORD lock = static_cast<DWORD>(reinterpret_cast<uintptr_t>(section));
    for (Pool &p : g_pools) {
        if (lock == *reinterpret_cast<const DWORD *>(static_cast<uintptr_t>(p.lock))) {
            look(p, static_cast<const DWORD *>(_AddressOfReturnAddress()));
            break;
        }
    }
    o_Leave(section);
}

void __cdecl watched_free(void *pointer) {
    const DWORD block = static_cast<DWORD>(reinterpret_cast<uintptr_t>(pointer));
    if (block) {
        const DWORD *const frame = static_cast<const DWORD *>(_AddressOfReturnAddress());
        // (from Mem_Free 0x67acb0: its own caller two dwords up)
        const DWORD caller = frame[0] == kMemFree ? frame[2] : frame[0];
        const DWORD at = index_of(block);
        if (g_released[at].block == block) {   // (once on a pool's list: is it still?)
            for (const Pool &p : g_pools) {
                if (on_list(p, *reinterpret_cast<const DWORD *>(static_cast<uintptr_t>(p.list)), block)) {
                    if (may_log()) {
                        char who[24], earlier[24], more[8 * 24];
                        name_code(who, caller);
                        name_code(earlier, g_released[at].caller);
                        name_stack(more, frame + 1);
                        log_line("[%6.1fs] pool watch: heap free of %s block %08X still on its free list, from %s (it went on the list from %s); stack:%s", log_seconds(),
                                 p.name, block, who, earlier, more);
                    }
                    break;
                }
            }
        }
        g_freed[at] = {block, caller};
    }
    o_free(pointer);
}

void *__cdecl watched_alloc(DWORD size) {
    void *const block = reinterpret_cast<void *(__cdecl *)(DWORD, int)>(static_cast<uintptr_t>(kNhMalloc))(size, 1);
    const DWORD address = static_cast<DWORD>(reinterpret_cast<uintptr_t>(block));
    if (g_freed[index_of(address)].block == address) g_freed[index_of(address)].block = 0;
    return block;
}

}  // namespace

void start_pool_watch(HINSTANCE dll) {
    const auto *self = reinterpret_cast<const BYTE *>(dll);
    const auto *nt = reinterpret_cast<const IMAGE_NT_HEADERS *>(self + reinterpret_cast<const IMAGE_DOS_HEADER *>(self)->e_lfanew);
    g_dll_base = static_cast<DWORD>(reinterpret_cast<uintptr_t>(dll));
    g_dll_size = nt->OptionalHeader.SizeOfImage;
    // The free's first two instructions and a jump back, in a page of its own: PUSH ECX; PUSH ESI; MOV ESI,[ESP + 0xc] (6 bytes), then JMP 0x67ad96.
    static const std::uint8_t kFreeHead[6] = {0x51, 0x56, 0x8b, 0x74, 0x24, 0x0c};
    static const std::uint8_t kAllocHead[5] = {0x8b, 0x44, 0x24, 0x04, 0x6a};
    auto *const body = static_cast<std::uint8_t *>(VirtualAlloc(nullptr, 16, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE));
    auto *const target = reinterpret_cast<std::uint8_t *>(static_cast<uintptr_t>(kFree));
    if (!body || memcmp(target, kFreeHead, 6) != 0) {
        log_line("pool watch: off (the heap's free is not as expected)");
        return;
    }
    CopyMemory(body, kFreeHead, 6);
    body[6] = 0xe9;
    *reinterpret_cast<DWORD *>(body + 7) = kFreeBody - static_cast<DWORD>(reinterpret_cast<uintptr_t>(body + 11));
    o_free = reinterpret_cast<void(__cdecl *)(void *)>(body);
    void **const slot = reinterpret_cast<void **>(static_cast<uintptr_t>(kLeaveSlot));
    DWORD old;
    if (!VirtualProtect(slot, sizeof *slot, PAGE_READWRITE, &old)) {
        log_line("pool watch: off (the import slot)");
        return;
    }
    o_Leave = reinterpret_cast<void(WINAPI *)(void *)>(*slot);   // (whatever the trace left there)
    *slot = reinterpret_cast<void *>(&watched_leave);
    VirtualProtect(slot, sizeof *slot, old, &old);
    const bool ok = hook::patch_jump(target, reinterpret_cast<const void *>(&watched_free), kFreeHead) == hook::PatchResult::Ok &&
                    hook::patch_jump(reinterpret_cast<void *>(static_cast<uintptr_t>(kMemAlloc)), reinterpret_cast<const void *>(&watched_alloc), kAllocHead) ==
                        hook::PatchResult::Ok;
    log_line("pool watch: %s", ok ? "on (tpw.ini [tpw] poolwatch=0 turns it off)" : "the heap's patches FAILED");
}

}  // namespace tpw::testing
