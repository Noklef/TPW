#include "windowreg.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/vcall.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *&Head() { return *reinterpret_cast<u8 **>(0x00faa610); }
u8 *&Next(u8 *node) { return at<u8 *>(node, 0x14); }

// the window of the first node with the hash asked through its slot 0x20, or 0
u32 Find(s32 hash) {
    u8 *window = nullptr;
    bool found = false;
    for (u8 *node = Head(); node && !found; node = Next(node))
        if (at<s32>(node, 0x10) == hash) {
            found = true;
            window = at<u8 *>(node, 0);
        }
    return window ? vcall<u32>(window, 0x20) : 0;
}
}

extern "C" {

s32 __cdecl Gui_HashName(const char *text, s32 multiplier) {
    u32 h = 0;
    for (; *text; ++text) h = ((u32)(s32)(signed char)*text ^ h) * (u32)multiplier;
    return (s32)h;
}

u32 __cdecl GuiWindows_Register(u8 *window, const char *name) {
    if (!window || !name[0]) return 0;   // (strlen(name) above 0)
    const s32 hash = Gui_HashName(name, 0x2f);
    bool fresh = true;
    u8 *node = Head();
    for (; node && Next(node) && fresh; node = Next(node))   // (the last node is never compared, sic)
        if (at<s32>(node, 0x10) == hash) fresh = false;
    if (!fresh) return 0;
    u8 *const added = static_cast<u8 *>(GameHeap_Alloc(0x18));
    if (!added) return 0;
    at<u16>(window, 0x10) = 1;
    Next(added) = nullptr;
    at<u8 *>(added, 0) = window;
    Crt().strncpy(reinterpret_cast<char *>(added + 4), name, 0xc);
    at<s32>(added, 0x10) = hash;
    if (!Head())
        Head() = added;
    else
        Next(node) = added;
    return 1;
}

void __cdecl GuiWindows_Clear(u32 destroy) {
    for (u8 *node = Head(); node;) {
        u8 *const next = Next(node);
        if ((u16)destroy && at<u8 *>(node, 0)) vcall<u32>(at<u8 *>(node, 0), 0, 1u);
        GameHeap_Free(node);
        node = next;
    }
}

u32 __cdecl GuiWindows_FindByName(const char *name) { return Find(Gui_HashName(name, 0x2f)); }

u32 __cdecl GuiWindows_FindByHash(s32 hash) { return Find(hash); }
}
