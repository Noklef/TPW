// What two mouse modes do with the tile under the pointer: see mousemodeclick.h.
#include "mousemodeclick.h"

#include <cstdint>
#include "../ui/dialog74fb20.h"
#include "../ui/panel7b9940.h"
#include "world.h"

namespace {

AddrCalls g_calls;

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(u32 address, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)address + offset); }

const u32 kHolder = reinterpret_cast<uintptr_t>(&gPanel7b9940_7b05e8);

u32 level() { return global<u32>((u32)&gWorldPtr); }
u16 tileId() { return global<u16>(0x007b05cc); }
u32 tileOf(u16 id) { return level() + 0x294 + (u32)id * 0x44; }
u32 alloc(u32 size) { return g_calls.Cd(0x0067acc0, size); }
void setCursor(u32 which) { g_calls.Th(0x004814a0, 0x007b51f0u, which); }
void setMode(u32 holder, u32 mode) { g_calls.Th(0x0046c350, holder, mode); }
u32 placer() { return g_calls.Th(0x00519450, level()); }
void placeStaff(u32 id) { g_calls.Th(0x00509af0, placer(), id); }

u32 plainMode() {
    const u32 mode = alloc(8);
    if (mode) {
        at<u32>(mode, 0) = 0x006fea10;
        at<u32>(mode, 4) = 1;
    }
    return mode;
}

u32 onlineMode() {   // (MMOnline_Ctor, inlined)
    const u32 mode = alloc(8);
    if (mode) {
        at<u32>(mode, 4) = 0;
        at<u32>(mode, 0) = 0x006fe980;
        setCursor(0);
    }
    return mode;
}

u32 holderMadeInline() {   // (MouseModeHolder_Ctor, inlined)
    u32 holder = global<u32>(kHolder);
    if (holder) return holder;
    holder = alloc(1);
    if (holder) {
        setMode(holder, plainMode());
        global<u32>(0x007b05e0) = 0xffffffffu;
        global<u32>(0x007b05d4) = 0xffffffffu;
    }
    global<u32>(kHolder) = holder;
    return holder;
}

u32 holderMade() {
    u32 holder = global<u32>(kHolder);
    if (holder) return holder;
    const u32 block = alloc(1);
    holder = block ? g_calls.Th(0x0046d550, block) : 0;
    global<u32>(kHolder) = holder;
    return holder;
}

bool onlineGame() {
    u8 &made = global<u8>(0x00fb3518);
    if (!(made & 1)) {
        made |= 1;
        g_calls.Th(0x00550ca0, reinterpret_cast<uintptr_t>(&gDialog74fb20_fb3b7c));
        g_calls.Cd(0x0067a7d0, 0x005d1750u);
    }
    return gDialog74fb20_fb3b7c == 1;
}

bool tilePasses(u32 tile) {
    if (!tile) return false;
    if (g_calls.Th(0x00535db0, tile, 0x40u) && (!g_calls.Th(0x00535db0, tile, 0x80u) || !g_calls.Th(0x005363a0, tile))) return false;
    u32 object = g_calls.Th(0x004d0af0, tile);
    if (g_calls.Th(0x0053ad90, object)) object = level() + ((u32)at<u16>(object, 0x10) + 0x6cde) * 0x28;
    return !g_calls.Th(0x0053ad60, object);
}

}  // namespace

void MouseModeClick_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

void __fastcall MMStaffDrop_Click(void *, void *, u32, u32) {
    const u16 id = tileId();
    const u32 tile = tileOf(id);
    if (g_calls.Th(0x00535db0, tile, 0x40u) || g_calls.Th(0x00536450, tile) || g_calls.Th(0x005363f0, tile)) return;
    if (!g_calls.Th(0x00536390, tile) && !g_calls.Th(0x00536310, tile) && !g_calls.Th(0x00536320, tile)) return;
    placeStaff(id);
    const u32 mode = plainMode();
    setMode(holderMadeInline(), mode);
}

void __fastcall MMStaffDrop_Cancel(u8 *self) {
    const u32 sprite = at<u32>((u32)(uintptr_t)self, 8);
    if (sprite) g_calls.Cd(0x00475550, sprite);
    at<u32>((u32)(uintptr_t)self, 8) = 0;
    if (!at<u16>(placer(), 0x60)) return;
    const u16 id = at<u16>(placer(), 0x60);
    const u32 record = global<u32>(0x007cfb90 + (u32)id * 0x14);
    placeStaff((u16)(at<u8>(record, 7) * 0x80 + at<u8>(record, 5) + 1));
}

void __fastcall MouseMode6fead0_Hover(void *, void *, u32, u32, u32, u32) {
    const u16 id = tileId();
    if (!id) return;
    const u32 below = (u32)id - 1;
    g_calls.Cd(0x005234d0, below & 0x7f, below >> 7, 0u);
    setCursor(tilePasses(tileOf(id)) ? 0x13 : 8);
}

void __fastcall MouseMode6fead0_Click(void *, void *, u32, u32) {
    setCursor(1);
    const u16 id = tileId();
    if (id) {
        const u32 below = (u32)id - 1;
        u16 found = id;
        g_calls.Cd(0x00527e80, &found, below & 0x7f, below >> 7);
        if (found) {
            const u32 thing = global<u32>(0x007cfb90 + (u32)found * 0x14);
            if (!at<u32>(g_calls.Th(0x004dd4e0, thing), 0x4ac)) {
                g_calls.Th(0x004e15b0, thing, 0u);
                const u32 mode = onlineGame() ? onlineMode() : plainMode();
                setMode(holderMade(), mode);
                return;
            }
        }
    }
    const u16 again = tileId();
    if (!again) return;
    if (!tilePasses(tileOf(again))) {
        setCursor(8);
        return;
    }
    g_calls.Cd(0x0042ae70);
    const u32 mode = onlineGame() ? onlineMode() : plainMode();
    setMode(holderMadeInline(), mode);
    setCursor(0);
}
}
