#include "idtable.h"

extern "C" {

void __cdecl IdTable_Init(u32 *, u32 *, u32 *next) { *next = 1; }

u32 __cdecl IdTable_Add(u32 *used, u32 *holes, u32 *next, u32 capacity, u32 *table, u32 value, s32 id) {
    if (id != -1) {
        table[id] = value;
        return (u32)id;
    }
    if (!*holes && *used >= capacity) return 0;
    const u32 taken = *next;
    table[taken] = value;
    if (taken - 1 == *used) {
        *next = taken + 1;
        ++*used;
        return taken;
    }
    const u32 last = *holes + *used;
    for (u32 i = taken; i <= last; ++i)
        if (!table[i + 1]) {
            *next = i + 1;
            break;
        }
    --*holes;
    ++*used;
    return taken;
}

void __cdecl IdTable_Remove(u32 *used, u32 *holes, u32 *next, u32 *table, u32 id) {
    if (!id) return;
    table[id] = 0;
    const u32 h = *holes;
    if (!h) {
        if (id == *used) {
            --*next;
        } else {
            *holes = 1;
            *next = id;
        }
        --*used;
        return;
    }
    if (id == *used + h) {
        --*used;
        while (*holes && !table[*used + *holes]) --*holes;
        return;
    }
    *holes = h + 1;
    --*used;
    if (id < *next) *next = id;
}
}
