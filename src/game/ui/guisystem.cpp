#include "guisystem.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/initializers.h"
#include "../core/vcall.h"
#include "guinamed.h"
#include "guitimers.h"
#include "msgqueue.h"
#include "window.h"
#include "windowreg.h"
#include "windowtimers.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// a new message queue (Mem_Alloc 0x14, Fn_669700), or 0
u8 *new_queue() {
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x14));
    if (!block) return nullptr;
    Fn_669700(block);
    return block;
}

}  // namespace

extern "C" {

u32 __cdecl Gui_Init(u32, u32 capacity, u8 flag) {
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x138));
    u8 *const root = block ? Window_Ctor(block, nullptr, nullptr, 0xf0000001u, 0xfffffd66u, 0, 0, 0x7ff, 0x5ff) : nullptr;
    global<u8 *>(0x00faa5d8) = root;
    if (!root) return 0;
    global<u32>(0x00faa568) = 1;
    global<u8 *>(0x00faa5e0) = root;
    global<u32>(0x00faa608) = 0;
    global<u32>(0x00faa57c) = 0;
    global<u32>(0x00faa578) = 0;
    global<u16>(0x00faa5bc) = 0;
    global<u8>(0x00faa56c) = flag;
    global<u8 *>(0x00faa5f0) = new_queue();
    if (!global<u8 *>(0x00faa5f0) || !(u16)MsgQueue_Init(global<u8 *>(0x00faa5f0), nullptr, capacity)) return 0;
    global<u8 *>(0x00faa5f4) = new_queue();
    if (!global<u8 *>(0x00faa5f4) || !(u16)MsgQueue_Init(global<u8 *>(0x00faa5f4), nullptr, capacity)) return 0;
    WindowTimers_Init(0x100);
    u8 *const table = static_cast<u8 *>(GameHeap_Alloc(8));
    if (table) Fn_669c41(table);
    global<u8 *>(0x00faa5f8) = table;
    if (!table || !(u16)GuiTimers_Init(table, nullptr, 0x40)) return 0;
    return 1;
}

u32 __cdecl Gui_Shutdown() {
    u8 *const root = global<u8 *>(0x00faa5d8);
    if (!root) return 0;
    vcall<u32>(root, 8, 1u);
    if (u8 *const queue = global<u8 *>(0x00faa5f0)) MsgQueue_Delete(queue, nullptr, 1);
    if (u8 *const queue = global<u8 *>(0x00faa5f4)) MsgQueue_Delete(queue, nullptr, 1);
    if (u8 *const table = global<u8 *>(0x00faa5f8)) GuiTimers_Delete(table, nullptr, 1);
    global<u8 *>(0x00faa5f0) = nullptr;
    global<u8 *>(0x00faa5f4) = nullptr;
    global<u8 *>(0x00faa5f8) = nullptr;
    WindowTimers_Shutdown();
    GuiWindows_Clear(1);
    GuiNamed_FreeAll();
    global<u8 *>(0x00faa5d8) = nullptr;
    global<u8 *>(0x00faa5dc) = nullptr;
    global<u8 *>(0x00faa5e0) = nullptr;
    return 1;
}
}
