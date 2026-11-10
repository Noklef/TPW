// The click on what the help line shows: see mouseclick.h.
#include "mouseclick.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/actionrec.h"
#include "../core/gameheap.h"
#include "../core/soundplay.h"
#include "../core/vcall.h"
#include "../files/wrapsources.h"
#include "../world/focuscell.h"
#include "../world/mousemode.h"
#include "../world/objectdefs.h"
#include "../world/thingscan.h"
#include "../world/world.h"
#include "dialog7ca1c8.h"
#include "dialog7ca210.h"
#include "focus.h"
#include "panel7cc3b0.h"
#include "panel7cc408.h"
#include "thingpanel.h"
#include "../core/soundset.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const MouseClickEnv kGameEnv = {Global7c2534_HasSource, Map_ObjectOrEntranceAt, ObjectControls_Find, GameTable7cfb90_Get1da726_5195d0,
                                reinterpret_cast<u32(__fastcall *)(u32)>(Source_Field0c), Sound_PlayAt, Global81d7a4_SetRecorded, ThingPanel_Show,
                                Panel7cc408_Show, Panel7cc3b0_Show, Dialog7ca210_Open, Dialog7ca1c8_OpenFor, FocusCell_Update, Cell_FirstObject};
const MouseClickEnv *g_env = &kGameEnv;

u8 *game() { return gWorldPtr; }
u8 *thingOf(u32 handle) { return global<u8 *>(0x007cfb90 + (handle & 0xffff) * 20); }
u8 *cellOf(u32 tile) { return game() + tile * 0x44 + 0x294; }
u8 *recordOf(u32 tile) { return game() + (tile + 0x6cde) * 40; }

// The tile mode k made, the holder made when missing and the mode set.
void enterTile(u32 kind) {
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x14));
    u8 *const mode = block ? MouseModeTile_Ctor(block, nullptr, kind) : nullptr;
    MouseModes_EnsureHolder();
    MouseMode_Set(mode);
}

// The first object of the cell the word +0x10 of the record names.
u32 parentsFirst(const u8 *record) {
    u16 out;
    return *g_env->firstObject(cellOf(at<u16>(record, 0x10)), nullptr, &out);
}

void pick(u32 handle) {
    Global7c2654_Set((u32)(uintptr_t)thingOf(handle));
    g_env->focusCell(1);
}

// Shown kind 4: the object under the mouse.
u32 clickObject(u32 tile) {
    u16 handle = (u16)tile;
    const u32 place = tile - 1;
    g_env->objectAt(&handle, place & 0x7f, place >> 7);
    if (!handle) return 1;
    using KeyStateFn = s16(__stdcall *)(s32);
    if (global<KeyStateFn>(0x006fd320)(0x11) >= 0) {
        g_env->thingPanel(reinterpret_cast<u8 *>((uintptr_t)Global7c2654_Get()));
        return 1;
    }
    u8 *const thing = thingOf(handle);
    const u16 id = at<u16>(thing, 0xe);
    const s32 price = at<s32>(g_env->find(game() + 0x1d943c, nullptr, id, 0), 4);
    if (price > (s32)g_env->money(g_env->table26(game()))) {
        g_env->playAt(0, gSoundCategories.ui, 0x1d, 0, 0, 0);
        return 0;
    }
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x14));
    u8 *const mode = block ? MouseModeTile_CtorKind(block, nullptr, 4, id) : nullptr;
    MouseModes_EnsureHolder();
    MouseMode_Set(mode);
    g_env->setRecorded(at<u32>(thing, 0x10), 1);
    return 0;
}

// Shown kind 5: a record of a ride's or shop's parts.
u32 clickRecord(u32 tile) {
    const u8 *record = recordOf(tile);
    if (at<u32>(record, 8) == 0xc) record = recordOf(at<u16>(record, 0x10));
    u32 kind;
    switch (at<u32>(record, 8)) {
    case 0x19:
        g_env->focusCell(1);
        enterTile(0x1a);
        return 1;
    case 0xb: kind = 0xe; break;
    case 0x10: kind = 0x13; break;
    default: return 1;
    }
    const u32 handle = parentsFirst(record);
    if (!handle) return 1;
    pick(handle);
    enterTile(kind);
    return 1;
}

}  // namespace

void MouseClick_SetEnv(const MouseClickEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {
u32 __cdecl MouseClick_OnShown() {
    if (!game() || g_env->hasSource()) return 0;
    MouseModes_EnsureHolder();
    u8 *const mode = global<u8 *>(0x007b05d8);
    const u32 modeKind = mode ? vcall<u32>(mode, 0x24) : 0;
    if (modeKind > 1) return 1;
    Globals7c264c_Load();
    const u32 tile = global<u16>(0x007b05cc);
    switch (gMouseclick_7c24f4) {
    case 1:
    case 2:
        if (tile) enterTile(1);
        return 1;
    case 3: {
        if (!tile || at<u32>(cellOf(tile), 8) != 3) return 1;
        const u32 handle = parentsFirst(cellOf(tile));
        if (!handle) return 1;
        pick(handle);
        enterTile(0x14);
        return 1;
    }
    case 4: return tile ? clickObject(tile) : 1;
    case 5: return tile ? clickRecord(tile) : 1;
    case 6: {
        if (!tile) return 1;
        const u32 handle = parentsFirst(cellOf(tile));
        if (!handle) return 1;
        u8 *const thing = thingOf(handle);
        Global7c2654_Set((u32)(uintptr_t)thing);
        g_env->openFor(thing);
        return 0;
    }
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12: {
        u8 *const thing = reinterpret_cast<u8 *>((uintptr_t)Global7c2650_Get());
        if (!thing) return 1;
        if (gMouseclick_7c24f4 == 7) g_env->showVisitor(reinterpret_cast<u8 *>((uintptr_t)0x007cc408), nullptr, thing);
        else g_env->showStaff(reinterpret_cast<u8 *>((uintptr_t)0x007cc3b0), nullptr, thing);
        return 1;
    }
    case 13:
    case 14:
        g_env->open7ca210();
        return 1;
    default: return 1;
    }
}
}
