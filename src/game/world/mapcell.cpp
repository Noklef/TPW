#include "mapcell.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u16 *store_pos(u16 *out, u32 x, u32 y) {
    *out = (u16)((y << 7) + x + 1);
    return out;
}

// A gBalanceLevel pair of positions at a: A x, A y, B x, B y.
u16 *balance_pos(u16 *out, u32 b, u32 a) {
    return b == 0 ? store_pos(out, global<u32>(a), global<u32>(a + 4)) : store_pos(out, global<u32>(a + 8), global<u32>(a + 0xc));
}

// As balance_pos, but B with A's y (sic).
u16 *balance_pos_a_y(u16 *out, u32 b, u32 a) {
    return store_pos(out, global<u32>(b == 0 ? a : a + 8), global<u32>(a + 4));
}

}  // namespace

extern "C" {

u8 *__fastcall MapHandle_Entry28(const u16 *handle) {
    return global<u8 *>(0x008023a0) + 0x10ffd8 + (u32)*handle * 0x28;
}

u8 *__fastcall MapHandle_Cell44(const u16 *handle) { return global<u8 *>(0x008023a0) + (u32)*handle * 0x44 - 0x44; }

void __fastcall MemRanges_Set(u8 *ranges, void *, u32 index, u32 a, u32 b, const char *) {
    at<u32>(ranges, index * 8) = a;
    at<u32>(ranges, index * 8 + 4) = b;
}

void __cdecl MemRanges_SetCells44(u32 base, u32 stride) {
    global<u32>(0x007cdb40) = base;
    global<u32>(0x007cdb50) = stride;
    global<u32>(0x007cdb44) = (stride << 14) + base;
}

void __cdecl MemRanges_SetCells28(u32 base, u32 stride) {
    global<u32>(0x007cdb48) = base;
    global<u32>(0x007cdb54) = stride;
    global<u32>(0x007cdb4c) = (stride << 14) + base;
}

u32 __cdecl Map_InBounds(s32 x, s32 y) { return x >= 0 && y >= 0 && x < 0x80 && y < 0x80; }

u8 *__fastcall Map_RECell(u8 *map, void *, u32 pos) { return map + 0x1b1104 + ((pos & 0xffff) - 1) * 10; }

u16 *__fastcall Map_TicketBoothPos(const u8 *, void *, u16 *out, u32 b) { return balance_pos(out, b, 0x0078559c); }
u16 *__fastcall Map_BusStopPos(const u8 *, void *, u16 *out, u32 b) { return balance_pos(out, b, 0x007855ac); }
u16 *__fastcall Map_EntrancePos(const u8 *, void *, u16 *out, u32 b) { return balance_pos(out, b, 0x007855bc); }
u16 *__fastcall Map_CrossingParkSidePos(const u8 *, void *, u16 *out, u32 b) { return balance_pos_a_y(out, b, 0x007855cc); }
u16 *__fastcall Map_CrossingBSSidePos(const u8 *, void *, u16 *out, u32 b) { return balance_pos_a_y(out, b, 0x007855dc); }
}
