// A list of models' material groups put in drawing order and drawn: see modellist.h.
#include "modellist.h"

#include <cstdint>

namespace {

AddrCalls g_calls;

template <typename T>
T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}
template <typename T>
T &global(u32 address) {
    return *reinterpret_cast<T *>((uintptr_t)address);
}

struct Group {   // 0x14 bytes
    const u8 *entry;
    u32 texture;
    s32 order;
    u8 batching;
    u8 index;
    Group *next;
};

struct Node {   // 0x14 bytes
    u32 key;
    Node *lesser;
    Node *greater;
    Group *batching;
    Group *apart;
};

// A new node holding the group.
void Start(Node *node, u32 key, Group *group) {
    node->key = key;
    node->greater = nullptr;
    node->lesser = nullptr;
    node->batching = group->batching & 1 ? group : nullptr;
    node->apart = group->batching & 1 ? nullptr : group;
}

// In order, ahead of those of its order with a texture not below its own.
void InsertBatching(Node *node, Group *group) {
    Group *before = nullptr, *after = node->batching;
    while (after && after->order < group->order) {
        before = after;
        after = after->next;
    }
    while (after && after->order == group->order && after->texture < group->texture) {
        before = after;
        after = after->next;
    }
    group->next = after;
    (before ? before->next : node->batching) = group;
}

// The group into the tree of *end nodes from the first.
void Insert(Node *nodes, Node *&end, u32 key, Group *group) {
    for (Node *node = nodes;;) {
        if (node->key == key) {
            if (group->batching & 1) {
                InsertBatching(node, group);
            } else {
                if (node->apart) group->next = node->apart;
                node->apart = group;
            }
            return;
        }
        Node *&child = node->key > key ? node->greater : node->lesser;
        if (!child) {
            Start(end, key, group);
            child = end++;
            return;
        }
        node = child;
    }
}

}  // namespace

extern "C" {

void ModelList_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

u32 __fastcall ModelList_Draw(const u8 *list) {
    Node nodes[0x1000];     // (0x14000 bytes on the stack, as the original; neither array is checked)
    Group groups[0x8000];   // (0xa0000)
    Node *end = nodes;
    Group *group = groups;
    const u32 batchFlags = at<u32>(global<const u8 *>(0x00879ab0), 0xc);
    const u32 count = at<u32>(list, 0);
    for (u32 e = count; e; --e) {
        const u8 *const entry = list + 4 + (e - 1) * 0x14;
        const u8 *const model = at<const u8 *>(entry, 4);
        const u32 groupCount = at<u16>(model, 0x5a);
        const u8 *source = at<const u8 *>(model, 0x6c) + groupCount * 0x10 - 0x10;
        for (u32 left = groupCount; left; --left, source -= 0x10, ++group) {
            const u8 *const material = at<const u8 *>(source, 0);
            const u32 set = material ? at<u32>(material, 4) : 0;
            const u32 mip = material && (global<u8>(0x008bd550) & 0x40) ? at<u16>(material, 2) : 0;
            const u32 texture = set ? g_calls.Th(0x00575f90, set, mip) : 0;   // TexSet_Texture
            group->texture = texture;
            u32 key = 0;
            if (texture) {
                if (at<u32>(reinterpret_cast<const void *>((uintptr_t)texture), 8) & 0x1000) key = at<u32>(reinterpret_cast<const void *>((uintptr_t)texture), 0x1c);
                const u32 order = g_calls.Th(0x00581790, (u32)(uintptr_t)entry, source, texture);   // TexLoad_DriverFlags
                group->order = (s32)order;
                group->batching = (batchFlags & order) != 0;
                if (at<u32>(entry, 8) & 0x2000000) group->batching = 0;
            } else {
                group->order = 0x1256;
                group->batching = 1;
            }
            group->index = (u8)(left - 1);
            group->entry = entry;
            group->next = nullptr;
            if (group == groups) {
                Start(end++, key, group);
            } else {
                Insert(nodes, end, key, group);
            }
        }
    }
    for (Node *node = nodes; node != end; ++node) {
        u32 size, batch = 0;
        for (const Group *g = node->batching; g; g = g->next) {
            const u32 owner = (u32)(uintptr_t)g->entry;
            if (!batch) {
                batch = g_calls.Th(0x00581880, owner, (u32)g->index, &size);   // Model_BeginBatch
                if (!batch) return 1;   // sic
            }
            if (!g_calls.Th(0x00580530, owner, batch, (u32)g->index, size)) return 0;   // ModelGroup_AddToBatch
            if (!g->next || g->next->order != g->order) {
                if (!g_calls.Th(0x005658c0, global<u32>(0x00879ab0), 0xbf800000u)) return 0;   // Batch_Close
                batch = 0;
            }
        }
        for (const Group *g = node->apart; g; g = g->next) {
            const u32 owner = (u32)(uintptr_t)g->entry;
            const u32 own = g_calls.Th(0x00581880, owner, (u32)g->index, &size);
            if (!own) return 0;
            if (!g_calls.Th(0x00580530, owner, own, (u32)g->index, size)) return 0;
            if (!g_calls.Th(0x005658c0, global<u32>(0x00879ab0), 0xbf800000u)) return 0;
        }
    }
    return 1;
}
}
