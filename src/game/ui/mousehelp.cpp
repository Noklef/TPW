// The help line and cursor for what the mouse is over: see mousehelp.h.
#include "mousehelp.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/globals.h"
#include "../core/keymap.h"
#include "../core/methods.h"
#include "../files/charsets.h"
#include "../frontend/helpbg.h"
#include "../model/highlight.h"
#include "../world/mapcell.h"
#include "../world/mapclick.h"
#include "../world/mousemode.h"
#include "../world/object.h"
#include "../world/staff.h"
#include "../world/thingscan.h"
#include "cursor.h"
#include "mouseclick.h"
#include "window7cc8c0handler.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const MouseHelpEnv kGameEnv = {MouseModes_EnsureHolder, Global7cc2f0_Get_4b4480, Global81ae2c_Get_52f840, Global7c2654_Get, Highlight_Update, HelpBg_SetText, GameCursor_Set, Map_EntrancePos, Map_TicketBoothPos, Cell_FirstObject, Map_ObjectOrEntranceAt, Object_GetDef, Object_DefWord284,
                               reinterpret_cast<void(__fastcall *)(u8 *, void *, u16 *, u32)>((uintptr_t)0x004e2780), Staff_GetName, Staff_Settle, MapCell_IsType9, MapCell_IsTypeA, MapCell_IsType4, MapCell_IsType0,
                               reinterpret_cast<s32(__cdecl *)(u8 *, u16 *, u32, u32, const u32 *)>(Charset_FormatWide), Records_DataIfTerminated, Wide_FromNarrow};
const MouseHelpEnv *g_env = &kGameEnv;

const u32 kTextSize = 0x3fff;
u16 *const kBuffer = reinterpret_cast<u16 *>((uintptr_t)0x007ba058);
const u16 *const kClear = reinterpret_cast<const u16 *>(gWindow7cc8c0handler_78a374);

u32 &shown() { return gMouseclick_7c24f4; }
u8 *&named() { return gMousehelp_7c24f8; }
u8 *thingOf(u32 handle) { return global<u8 *>(0x007cfb90 + (handle & 0xffff) * 20); }
u8 *cellOf(u8 *world, u32 tile) { return world + tile * 0x44 + 0x294; }
u8 *recordOf(u8 *world, u32 tile) { return world + (tile + 0x6cde) * 40; }

// The entry's text in the buffer (widened when the converter [0x78ba80] is on; the buffer as it was when not); null with none.
const u16 *entryText(u32 entry) {
    const u8 *const source = g_env->records(global<u8 *>(0x0078ba98), nullptr, entry);
    if (!source) return nullptr;
    if (global<u32>(0x0078ba80)) {
        u32 status;
        g_env->widen(global<u8 *>(0x0078ba7c), nullptr, &status, kBuffer, source, 0x200);
    }
    return kBuffer;
}

void format(u32 entry, const u32 *argument) { g_env->format(global<u8 *>(0x0078ba98), kBuffer, kTextSize, entry, argument); }

// The entry formatted with a string argument, shown.
void showWith(u32 entry, const void *text) {
    const u32 argument[4] = {0x006ff174, 0x5641524d, 2, (u32)(uintptr_t)text};
    format(entry, argument);
    g_env->help(kBuffer);
}

void thingHelp(u32 entry, u8 *thing) {
    g_env->copyName(thing, nullptr, kBuffer, kTextSize);
    showWith(entry, kBuffer);
}

bool hasWord284(const u8 *thing) {
    u16 out;
    return *g_env->word284(thing, nullptr, &out) != 0;
}

u32 objectHandle(u32 tile) {
    u32 out;
    g_env->objectAt(reinterpret_cast<u16 *>(&out), (tile - 1) & 0x7f, (tile - 1) >> 7);
    return out & 0xffff;
}

// Mode 8, or 3 over a ride queue.
void overRideMode() {
    if (!global<u32>(0x007a1c7c)) {
        const u32 tile = global<u16>(0x007b05cc);
        if (!tile) return;
        u8 *const world = global<u8 *>(0x007cf83c);
        const u8 *const cell = cellOf(world, tile);
        if (at<u32>(cell, 8) != 0x15) return;
        u16 out;
        const u16 handle = *g_env->firstObject(cellOf(world, at<u16>(cell, 0x10)), nullptr, &out);
        if ((u32)(uintptr_t)thingOf(handle) != g_env->rideThing()) return;
    }
    g_env->highlight();
}

// Mode 9.
void overObjectMode(u32 previous) {
    const u32 tile = global<u16>(0x007b05cc);
    if (!tile) return;
    const u32 handle = objectHandle(tile);
    if (!handle) {
        if (!named() && previous == shown()) return;
        named() = nullptr;
        format(0x243, nullptr);
        g_env->help(kBuffer);
        return;
    }
    u8 *const thing = thingOf(handle);
    if (at<u8>(thing, 2) != 3 || hasWord284(thing) || at<u32>(g_env->def(thing), 0x4ac)) return;
    if (thing != named() || previous != shown()) {
        named() = thing;
        thingHelp(0x244, thing);
    }
    g_env->highlight();
}

// The cursor over the tile in the modes 0 and 1, when no object or tool took it.
void overTile(u32 previous) {
    const u32 tile = global<u16>(0x007b05cc);
    if (!tile) {
        g_env->cursor(0);
        return;
    }
    u8 *const world = global<u8 *>(0x007cf83c);
    u8 *const map = world + 0x2d8;
    u16 entrance0, entrance1, booth0, booth1;
    g_env->entrance(map, nullptr, &entrance0, 0);
    g_env->entrance(map, nullptr, &entrance1, 1);
    g_env->booth(map, nullptr, &booth0, 0);
    g_env->booth(map, nullptr, &booth1, 1);
    if (tile == entrance0 || tile == entrance1 || tile == (u16)(entrance0 - 1) || tile == (u16)(entrance1 + 1)) {
        shown() = 0xe;
        if (previous != 0xe) g_env->help(entryText(0x1cb));
        named() = nullptr;
        g_env->highlight();
        g_env->cursor(2);
        return;
    }
    if (tile == booth0 || tile == booth1) {
        shown() = 0xd;
        if (previous != 0xd) g_env->help(entryText(0x1cc));
        named() = nullptr;
        g_env->cursor(2);
        return;
    }
    const u8 *const cell = cellOf(world, tile);
    const u8 *record = recordOf(world, tile);
    const u32 handle = objectHandle(tile);
    u8 *const thing = handle ? thingOf(handle) : nullptr;
    u32 cursor = 0;
    bool done = false, queue = false;
    switch (at<u32>(record, 8)) {
    case 0xb:
    case 0x10:
        queue = true;
        break;
    case 0xc: {
        const u32 inner = at<u16>(record, 0x10);
        if (at<u32>(world, inner * 40 + 0x1102b8) == 0x19) {
            done = true;
        } else {
            record = recordOf(world, inner);
            queue = true;
        }
        break;
    }
    default:
        break;
    }
    if (queue) {
        if (g_env->isType4(cell) || g_env->isTypeA(cell) || g_env->isType9(cell)) {
            if (!thing) return;   // (no cursor)
            queue = hasWord284(thing);
        }
        if (queue) {
            u16 first;
            g_env->firstObject(cellOf(world, at<u16>(record, 0x10)), nullptr, &first);
            if (!first) {
                named() = nullptr;
                return;
            }
            const u8 *const ride = thingOf(first);
            shown() = 5;
            cursor = at<u32>(ride, 0x2c) ? 10 : 6;
            if (previous != 5) g_env->help(entryText(0x1be));
            done = true;
            named() = nullptr;
            g_env->highlight();
        }
    }
    if (!done && g_env->isType0(cell)) {
        done = true;
        cursor = 3;
        shown() = 2;
        if (previous != 2) g_env->help(entryText(0x1b9));
        named() = nullptr;
    }
    if (done) {
        g_env->cursor(cursor);
        return;
    }
    switch (at<u32>(cell, 8)) {
    case 1:
    case 3: {
        const bool one = at<u32>(cell, 8) == 1;
        cursor = one ? 3 : 4;
        const u32 kind = one ? 1 : 3;
        shown() = kind;
        if (previous != kind) g_env->help(entryText(one ? 0x1ba : 0x1bc));
        break;
    }
    case 4:
    case 9:
    case 0xa:
        if (!thing || (hasWord284(thing) && at<u32>(g_env->def(thing), 0x288) != 2)) {
            g_env->cursor(0);
            return;
        }
        shown() = 4;
        if (thing != named()) {
            named() = thing;
            thingHelp(0x226, thing);
        }
        g_env->highlight();
        g_env->cursor(2);
        return;
    case 0x15:
        shown() = 6;
        if (previous != 6) g_env->help(entryText(0x1c0));
        named() = nullptr;
        g_env->highlight();
        g_env->cursor(0xb);
        return;
    default:
        if (previous != shown()) g_env->help(kClear);
        break;
    }
    named() = nullptr;
    g_env->cursor(cursor);
}

// An object of a staff kind under the mouse: the kinds 4-8 shown as 9, 8, 0xa, 0xb, 0xc.
bool overStaff(u8 *under, u32 previous) {
    u32 kind;
    switch (at<u8>(under, 2)) {
    case 4: kind = 9; break;
    case 5: kind = 8; break;
    case 6: kind = 0xa; break;
    case 7: kind = 0xb; break;
    case 8: kind = 0xc; break;
    default: return false;
    }
    shown() = kind;
    if (previous != kind || named() != under) {
        showWith(0x22b, g_env->staffName(under));
        g_env->settle(under);
    }
    named() = under;
    return true;
}

// The person (kind 1) with the tool 2 and no modifier, or with shift and no tool.
void overPerson(u8 *under, u32 previous) {
    if (under && at<u8>(under, 2) == 1) {
        shown() = 7;
        if (previous != 7 || named() != under) {
            const u32 id = at<u32>(under, 0x1d8);
            if (!id) {
                format(0x22a, nullptr);
            } else {
                const u32 argument[5] = {0x006ff170, 0x5641524d, 3, id, 0};
                format(0x229, argument);
            }
            g_env->help(kBuffer);
        }
        named() = under;
        g_env->cursor(2);
        return;
    }
    if (previous != shown()) g_env->help(kClear);
    g_env->cursor(0);
}

}  // namespace

void MouseHelp_SetEnv(const MouseHelpEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" void __cdecl MouseHelp_Update(u32 forced) {
    const u32 previous = shown();
    const u32 keys = KeyMap_AsyncModifiers();
    const u32 tool = g_env->tool();
    shown() = 0;
    u32 mode = 1;
    if (!forced) {
        g_env->ensureModes();
        u8 *const holder = global<u8 *>(0x007b05d8);
        mode = holder ? reinterpret_cast<u32(__fastcall *)(u8 *)>((uintptr_t)at<u32>(at<u8 *>(holder, 0), 0x24))(holder) : 0;
    }
    const u32 other = g_env->other();
    if (mode == 8 || (mode == 3 && (other == 0x15 || other == 0x16))) {
        overRideMode();
        return;
    }
    if (mode == 9) {
        overObjectMode(previous);
        return;
    }
    if (mode != 0 && mode != 1) {
        if (previous != shown()) g_env->help(kClear);
        named() = nullptr;
        return;
    }
    u8 *const under = global<u8 *>(0x007b05ac);
    if ((tool == 2 && !keys) || (keys == 3 && !tool)) {
        overPerson(under, previous);
        return;
    }
    const bool staff = under && overStaff(under, previous);
    if (staff || tool) {
        g_env->cursor(staff ? 2 : 0);
        return;
    }
    overTile(previous);
}
