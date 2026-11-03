#include "panel7cc218.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../gfx/barmesh.h"
#include "../core/accessors.h"
#include "../core/actionrec.h"
#include "../core/trivial.h"
#include "../core/vwrap.h"
#include "../render/textparts.h"
#include "../core/soundplay.h"
#include "../core/textarg.h"
#include "../files/wrapsources.h"
#include "../world/advisor.h"
#include "../world/focuscell.h"
#include "../world/cellrun.h"
#include "../world/gamemode.h"
#include "../world/mapclick.h"
#include "../world/queuetrace.h"
#include "../world/mousemode.h"
#include "../world/objectdefs.h"
#include "../world/world.h"
#include "../core/circbuf.h"
#include "../core/options.h"
#include "../files/charsets.h"
#include "../math/x87.h"
#include "../world/object.h"
#include "../world/objectstatus.h"
#include "../world/objectmsg.h"
#include "../world/ridelist.h"
#include "../world/ridesweep.h"
#include "dialog7ca098.h"
#include "dialog7ca150.h"
#include "dialog7ca1c8.h"
#include "dialog7ca210.h"
#include "dialogs.h"
#include "guirect.h"
#include "hoverlabel.h"
#include "menuactions.h"
#include "panelparts.h"
#include "signrect.h"
#include "thinggoto.h"
#include "windowscript.h"
#include "windowtimers.h"
#include "window7059f0.h"
#include "window.h"
#include "window705a50.h"
#include "window705b20.h"
#include "windowreg.h"

using namespace panelparts;

namespace {
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
u8 *window(u8 *self) { return *reinterpret_cast<u8 **>(self + 0x14); }
u8 *root(u8 *self) { return *reinterpret_cast<u8 **>(self + 0x10); }
s32 mode(const u8 *self) { return *reinterpret_cast<const s32 *>(self + 8); }

// The child `id` of the panel's window +0x10 (not checked): the range 0..0x78, and its child 0x3e2e a "smallguage.wct" mesh (0x124
// bytes; none when the allocation fails) as its implementation and its slot 7 given 0.
void gauge(u8 *self, u32 id) {
    u8 *const child = Window_FindChild(root(self), nullptr, id);
    Window705b20_SetRange(child, nullptr, 0, 0x78);
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x124));
    u8 *const mesh = memory ? MeshOverlay_Construct(memory, nullptr, reinterpret_cast<const char *>(0x007558f4)) : nullptr;
    Window_SetImpl(Window_FindChild(child, nullptr, 0x3e2e), nullptr, mesh);
    vcall<void>(Window_FindChild(child, nullptr, 0x3e2e), 0x1c, 0u);
}

// The child `id` of the panel's window +0x10 (not checked): a TextPart_Ctor part (0x1a4 bytes; none when the allocation fails), the
// bytes 1, 1 as 0xc00, font 6, the bytes 0, 0, 0, 0xff as 0x40000 and no text.
void label(u8 *self, u32 id) {
    u8 *const child = Window_FindChild(root(self), nullptr, id);
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    Window_SetImpl(child, nullptr, memory ? TextPart_Ctor(memory, nullptr, 0) : nullptr);
    Window_NotifyC00(child, nullptr, 1, 1);
    Window_Set4c(child, nullptr, Language_Font(6));
    Window_Notify40000(child, nullptr, 0, 0, 0, 0xff);
    Window_NotifyImpl100(child, nullptr, 0);
}

// The text `id` formatted into the shared buffer 0x7ba058 with one number argument n: the panel's setting at `offset` (signed: a
// negative one's magnitude and the minus flag, as the original builds it inline).
void settingText(u8 *self, u32 id, u32 n, u32 offset) {
    u8 arg[0x14];
    TextArgInt_Ctor(arg, nullptr, n, at<s32>(self, offset), 1);
    TextFormat().format(*reinterpret_cast<void **>((uintptr_t)0x0078ba94), reinterpret_cast<u16 *>((uintptr_t)0x007ba058), 0x3fff, id, arg);
}
const Panel7cc218ShowEnv kShowGameEnv = {Window705b20_SetRange, Window_SetShown, Panel7cc218_ShowSettings};
const Panel7cc218ShowEnv *g_show = &kShowGameEnv;
// A table's dword (the level's row in the definition) as a part of the range, 10 bits of fraction.
s32 rangePart(const u8 *def, u32 table, u8 level, s32 from, s32 to) {
    return (s32)((u32)at<s32>(def, ((u32)level << 6) + table) << 10) / (s32)((u32)to - (u32)from);
}
}  // namespace

void Panel7cc218_SetShowEnv(const Panel7cc218ShowEnv *env) { g_show = env ? env : &kShowGameEnv; }

namespace {
const Panel7cc218RefreshEnv kRefreshGameEnv = {Thing_NameBuffer7ba058, Panel7cc218_ShowRide, Panel7cc218_UpdateButtons};
const Panel7cc218RefreshEnv *g_refresh = &kRefreshGameEnv;
}  // namespace

void Panel7cc218_SetRefreshEnv(const Panel7cc218RefreshEnv *env) { g_refresh = env ? env : &kRefreshGameEnv; }

extern "C" void __fastcall Panel7cc218_Refresh(u8 *self) {
    u8 *const name = g_refresh->name(*reinterpret_cast<u8 **>((uintptr_t)0x007c2658));
    vcall<void>(at<u8 *>(self, 0x20), 0x24, name);
    vcall<void>(self, 0x18);
    vcall<void>(self, 0x14);
    g_refresh->showRide(self);
    g_refresh->buttons(self);
}

namespace {

const Panel7cc218TileEnv kTileGameEnv = {Global7c2654_Set, FocusCell_Update, ObjectDefs_Find, Dialog7ca1c8_OpenFor, GameHeap_Alloc, MouseModeTile_Ctor, Global81ecac_SetRecorded, MouseModeOnlineBase_Ctor, MouseMode_Set, GameMode_MapClick,
                                         Object_GetDef, RideQueue_Trace, PairTable_Commit, GameMode_Set, MouseModeTile_CtorKind, Global8186e0_SetRecorded, Global81d7a4_SetRecorded};
const Panel7cc218TileEnv *g_tile = &kTileGameEnv;

// A new tile mode of the kind, or none.
u8 *newTileMode(u32 kind) {
    u8 *const block = static_cast<u8 *>(g_tile->alloc(0x14));
    return block ? g_tile->tileCtor(block, nullptr, kind) : nullptr;
}

// MouseModes_EnsureHolder, inline in the original, then the mode current.
void setMode(u8 *mode) {
    u8 *&holder = *reinterpret_cast<u8 **>((uintptr_t)0x007b05e8);
    if (!holder) {
        u8 *const block = static_cast<u8 *>(g_tile->alloc(1));
        if (block) {
            u8 *const base = static_cast<u8 *>(g_tile->alloc(8));
            g_tile->setMode(base ? g_tile->baseCtor(base) : nullptr);
            *reinterpret_cast<u32 *>((uintptr_t)0x007b05e0) = 0xffffffffu;
            *reinterpret_cast<u32 *>((uintptr_t)0x007b05d4) = 0xffffffffu;
        }
        holder = block;
    }
    g_tile->setMode(mode);
}

void setTileMode(u8 *mode) {
    setMode(mode);
    g_tile->mapClick(0, 0, 0, 0);
}

}  // namespace

void Panel7cc218_SetTileEnv(const Panel7cc218TileEnv *env) { g_tile = env ? env : &kTileGameEnv; }

extern "C" void __fastcall Panel7cc218_EnterMoveMode(u8 *self) {
    u8 *const thing = *reinterpret_cast<u8 **>((uintptr_t)0x007c2658);
    g_tile->select((u32)(uintptr_t)thing);
    g_tile->focus(1);
    const u16 cell = (u16)((thing[7] << 7) + thing[5] + 1);
    if (*reinterpret_cast<u32 *>(g_tile->getDef(thing) + 0x40)) {
        *reinterpret_cast<u8 *>((uintptr_t)0x0081b14c) = g_tile->traceQueue(thing);
        g_tile->commitPairs();
    }
    g_tile->gameMode(0x33, 1);
    const u32 before = (u32)cell - 1;
    g_tile->mapClick((s32)(before & 0x7f), (s32)(before >> 7), 0, 0);
    u8 *const block = static_cast<u8 *>(g_tile->alloc(0x14));
    u8 *const mode = block ? g_tile->tileCtorKind(block, nullptr, 0x3b, *reinterpret_cast<u16 *>(thing + 0xe)) : nullptr;
    setMode(mode);
    *reinterpret_cast<u8 **>((uintptr_t)0x0081b3ac) = thing;
    g_tile->gameMode(0x3b, 1);
    const u16 id = *reinterpret_cast<u16 *>(thing + 0xe);
    g_tile->setId(&id, 0);
    g_tile->setValue(*reinterpret_cast<u32 *>(thing + 0x10), 1);
    *reinterpret_cast<u8 **>((uintptr_t)0x007c2658) = nullptr;
    vcall<void>(self, 0x2c);
}

extern "C" void __fastcall Panel7cc218_EnterTileMode(u8 *self, void *, u32 flag) {
    u8 *const thing = *reinterpret_cast<u8 **>((uintptr_t)0x007c2658);
    g_tile->select((u32)(uintptr_t)thing);
    g_tile->focus(1);
    if (!flag) {
        setTileMode(newTileMode(0x14));
    } else {
        u8 *const def = g_tile->findDef(reinterpret_cast<u8 *>((uintptr_t)0x007890a0), nullptr, *reinterpret_cast<u16 *>(thing + 0xe));
        const u32 type = *reinterpret_cast<u32 *>(def + 0x9c);
        if (type == 3) {
            g_tile->openFor(thing);
        } else {
            u8 *const mode = newTileMode(type == 1 ? 0xe : type == 2 ? 0x13 : 0);
            g_tile->setRecorded(*reinterpret_cast<u32 *>(def + 0x9c));
            setTileMode(mode);
        }
    }
    vcall<void>(self, 0x2c);
}

extern "C" {

void __fastcall Panel7cc218_ShowRide(u8 *self) {
    const u8 *const thing = *reinterpret_cast<u8 **>((uintptr_t)0x007c2658);
    const u8 *const def = Object_GetDef(thing);
    if (!root(self)) return;
    if (u8 *const child = Window_FindChild(root(self), nullptr, 0x3e30)) {
        const s32 from = at<s32>(def, 0x134), to = at<s32>(def, 0x138);
        g_show->setRange(child, nullptr, from, to);
        vcall<void>(child, 0x1c, at<u32>(thing, 0x58));
        const u8 level = at<u8>(thing, 0x50);
        u8 *const part = Window_FindChild(child, nullptr, 0x3e2e);
        vcall<void>(part, 0x1c, rangePart(def, 0x1ac, level, from, to));
    }
    if (u8 *const child = Window_FindChild(root(self), nullptr, 0x3e2f)) {
        const u32 shown = at<u32>(def, 0x70);
        at<u32>(self, 0x28) = shown;
        g_show->setShown(child, nullptr, shown ? 1u : 0u);
        g_show->setRange(child, nullptr, at<s32>(def, 0x12c), at<s32>(def, 0x130));
        vcall<void>(child, 0x1c, (u32)at<u8>(thing, 0x5c));
    }
    if (u8 *const child = Window_FindChild(root(self), nullptr, 0x3e2d)) {
        const s32 from = at<s32>(def, 0x124), to = at<s32>(def, 0x128);
        if (from == to) {
            g_show->setShown(child, nullptr, 0);
            g_show->setShown(Window_FindChild(root(self), nullptr, 0x3e31), nullptr, 0);
        } else {
            g_show->setShown(child, nullptr, 1);
            g_show->setShown(Window_FindChild(root(self), nullptr, 0x3e31), nullptr, 1);
            const u8 level = at<u8>(thing, 0x50);
            u8 *const part = Window_FindChild(child, nullptr, 0x3e2e);
            vcall<void>(part, 0x1c, rangePart(def, 0x19c, level, from, to));
        }
        g_show->setRange(child, nullptr, from, to);
        vcall<void>(child, 0x1c, (u32)at<u8>(thing, 0x5d));
    }
    g_show->showSettings(self, nullptr, 7);
}

void __fastcall Panel7cc218_Dress(u8 *self) {
    if (!window(self)) return;
    text(window(self), 0x3e20, 0x11);
    text(window(self), 0x3e1a, 0x12);
    text(window(self), 0x3e1c, 0x13);
    text(window(self), 0x3e1e, 0x14);
    text(window(self), 0x3e1f, 0x15);
    text(window(self), 0x3e1d, 0x16);
    text(window(self), 0x3e22, 0x17);
    part(window(self), TextPart6ff350_Ctor, 0x3e21, 0);
    textNone(window(self), 0x3e1b);
    bar(window(self), kRideStatBar, 0x3e16, 0);
    bar(window(self), kRideStatBar, 0x3e18, 0);
    bar(window(self), kRideStatBar, 0x3e19, 0);
    bar(window(self), kRideStatBar, 0x3e17, 0);
    part(window(self), TextPart6ff3b0_Ctor, 0x3e23, 0);
}

void __fastcall Panel7cc218_DressTip(u8 *self) { panelparts::tip(self); }

void __fastcall Panel7cc218_SetUpGauges(u8 *self) {
    u8 rect[12];
    GuiRect_Ctor(rect);
    if (root(self)) {
        gauge(self, 0x3e2d);
        gauge(self, 0x3e30);
        Window705b20_SetRange(Window_FindChild(root(self), nullptr, 0x3e2f), nullptr, 0, 10);
        label(self, 0x3e31);
        label(self, 0x3e33);
        label(self, 0x3e32);
    }
    GuiRect_Dtor(rect);
}

void __fastcall Panel7cc218_Show(u8 *self, void *, u8 *thing) {
    *reinterpret_cast<u8 **>((uintptr_t)0x007cc208) = thing;
    WindowAt10_Create(self, nullptr, reinterpret_cast<u8 *>((uintptr_t)0x00755150), 0x004af600, 0x3e24, 0x3e15, 0x3e25);
    at<u32>(self, 0x24) = 0;
    const u8 value = (u8)(u32)(long long)at<float>(thing, 0x44);   // (__ftol's low byte)
    Advisor_PostId(value <= 0x14 && value != 0 ? 0x129 : 0xcf);
}

void __fastcall Panel7cc218_ApplyToShown(u8 *self) {
    u8 *const thing = *reinterpret_cast<u8 **>((uintptr_t)0x007c2658);
    Obj_SetSpeed(thing, nullptr, at<u32>(self, 0x2c));
    Object_SetCapacity(thing, nullptr, at<u8>(self, 0x30));
    if (at<u32>(Object_GetDef(thing), 0x70)) Obj_SetDuration(thing, nullptr, at<u8>(self, 0x34));
}

void __fastcall Panel7cc218_Open(u8 *self) {
    if (Things_CountGroup(self, nullptr, *reinterpret_cast<const u32 *>((uintptr_t)0x007cc208)) < 2) {
        Window_SetShown(Window_FindChild(root(self), nullptr, 0x3e26), nullptr, 0);
        Window_SetShown(Window_FindChild(root(self), nullptr, 0x3e27), nullptr, 0);
    }
    Panel7cc218_SetUpGauges(self);
    Window_SetShown(root(self), nullptr, 1);
    HoverLabel_Bind(self, nullptr, 0x3e28);
}

void __fastcall Panel7cc218_UpdateButtons(u8 *self) {
    u8 *const thing = *reinterpret_cast<u8 **>((uintptr_t)0x007c2658);
    if (!thing) return;
    const u8 *const def = Object_GetDef(thing);
    if (mode(self) == 3) {
        static const u32 kButtons[] = {0x3e38, 0x3e34, 0x3e2a, 0x3e36, 0x3e2c, 0x3e37, 0x3e35};
        for (u32 i = 0; i < sizeof(kButtons) / sizeof(kButtons[0]); ++i)
            Window_SetShown(Window_FindChild(root(self), nullptr, kButtons[i]), nullptr, 0);
        return;
    }
    u8 *const open = Window_FindChild(root(self), nullptr, 0x3e38);
    Window_SetShown(open, nullptr, 1);
    bool enabled = false;
    if (mode(self) != 0x19 && mode(self) != 0x1a && !(Is68_Zero_4e0440(thing) && !Object_CanOpen(thing)) &&
        (Is68_Zero_4e0440(thing) || Const1_424ec0())) {
        vcall<void>(open, 0x14, 1u);
        Window705a50_SetState(open, nullptr, Is68_Zero_4e0440(thing) ? 1 : 0, 1);
        enabled = true;
    }
    if (!enabled) vcall<void>(open, 0x14, 0u);
    Window_SetShown(Window_FindChild(root(self), nullptr, 0x3e34), nullptr, 1);
    Window_SetShown(Window_FindChild(root(self), nullptr, 0x3e2a), nullptr, at<s32>(def, 0x9c) != 0 ? 1 : 0);
    u8 *const kind = Window_FindChild(root(self), nullptr, 0x3e36);
    Window_SetShown(kind, nullptr, 1);
    const s32 m = mode(self);
    Window705a50_SetState(kind, nullptr, (m == 0x13 || m == 0x15 || m == 0x12 || m == 0x14 || m == 0x11) ? 1 : 0, 1);
    Window_SetShown(Window_FindChild(root(self), nullptr, 0x3e35), nullptr, 1);
    u8 *const track = Window_FindChild(root(self), nullptr, 0x3e2c);
    Window_SetShown(track, nullptr, 1);
    vcall<void>(track, 0x14, (u8)Object_TrackNeighboursOk(thing) ? 1u : 0u);
    Window_SetShown(Window_FindChild(root(self), nullptr, 0x3e37), nullptr, 1);
}

u32 __fastcall Panel7cc218_Choose(u8 *, void *, s32 index, u8 close) {
    u8 *const thing = *reinterpret_cast<u8 **>(0x007c2658);
    u8 *const game = *reinterpret_cast<u8 **>(0x007cf83c);
    const s32 balance = (s32)Source_Field0c(reinterpret_cast<const TrackedSource *>((uintptr_t)GameTable7cfb90_Get1da726_5195d0(game)));
    if (close == 1) {
        Object_SetOpen(thing, nullptr, 0);
        return 1;
    }
    if (index < 0) {
        if (balance > at<s32>(Object_GetDef(thing), 0x1b8 - (u32)index * 0x40)) Object_SetOpen(thing, nullptr, 1);
        return 0;
    }
    if (balance < at<s32>(ObjectControls_Find(game + 0x1d943c, nullptr, (u32)index, 0), 4)) {
        SoundUi_Play1d();
        return 0;
    }
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x14));
    u8 *const mode = block ? MouseModeTile_CtorKind(block, nullptr, 4, (u16)index) : nullptr;
    MouseModes_EnsureHolder();
    MouseMode_Set(mode);
    Global7c2654_Set((u32)(uintptr_t)thing);
    FocusCell_Update(1);
    return 1;
}

void __fastcall Panel7cc218_ShowSettings(u8 *self, void *, u8 flags) {
    u8 *const thing = *reinterpret_cast<u8 **>((uintptr_t)0x007c2658);
    if (flags & 1) {
        u8 *const child = Window_FindChild(root(self), nullptr, 0x3e33);
        settingText(self, 0x1aa, 5, 0x2c);
        Window_NotifyImpl100(child, nullptr, 0x007ba058u);
    }
    if (flags & 2) {
        u8 *const child = Window_FindChild(root(self), nullptr, 0x3e31);
        if (at<s32>(Object_GetDef(thing), 0x9c) == 3) settingText(self, 0x1cf, 6, 0x30);
        else if (at<s32>(Object_GetDef(thing), 0x9c) == 2) settingText(self, 0x1cd, 6, 0x30);   // (the definition looked up again)
        else settingText(self, 0x1ab, 6, 0x30);
        Window_NotifyImpl100(child, nullptr, 0x007ba058u);
    }
    if (flags & 4) {
        u8 *const child = Window_FindChild(root(self), nullptr, 0x3e32);
        switch (at<u32>(Object_GetDef(thing), 0x70)) {
        case 0:
            Window_SetShown(child, nullptr, 0);   // (the buffer's text sent all the same, sic)
            break;
        case 1:
            Window_SetShown(child, nullptr, 1);
            settingText(self, at<s32>(self, 0x34) == 1 ? 0x1ac : 0x1ad, 7, 0x34);
            break;
        case 2:
            Window_SetShown(child, nullptr, 1);
            settingText(self, 0x1ae, 7, 0x34);
            break;
        case 3:
            Window_SetShown(child, nullptr, 1);
            settingText(self, 0x1af, 7, 0x34);
            break;
        case 4:
            Window_SetShown(child, nullptr, 1);
            settingText(self, 0x1b0, 7, 0x34);
            break;
        default:   // (the buffer's text sent as it is, sic)
            break;
        }
        Window_NotifyImpl100(child, nullptr, 0x007ba058u);
    }
}
}

namespace {
template <typename T>
T &global(uintptr_t address) { return *reinterpret_cast<T *>(address); }

const Panel7cc218LevelsEnv kGameLevelsEnv = {Window7059f0_Clear, ObjectDefs_First, ObjectDefs_Next, Setting_ReadMode, reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)0x0067a7d0), ObjectControls_Find, ObjectDefs_GetFullName, Window7059f0_AddItem, Object_GetDef,
                                             Is19c_Eq2_4e03e0, Charset_FormatWide, Language_WideText, reinterpret_cast<u16 *(__cdecl *)(u16 *, const u16 *)>((uintptr_t)0x0067c430), Window7059f0_RefreshRows};
const Panel7cc218LevelsEnv *g_levels = &kGameLevelsEnv;
const Panel7cc218FiguresEnv kGameFiguresEnv = {Object_GetDef, Window_FindChild, Object_AgeDays, Charset_FormatWide, Window_NotifyImpl100, Object_ComputeRating, Object_DefRowPercent, Object_ScrapRefund, Object_Status, Panel7cc218_UpdateButtons, Panel7cc218_ShowRide, Panel7cc218_FillLevels};
const Panel7cc218FiguresEnv *g_figures = &kGameFiguresEnv;

constexpr uintptr_t kWideBuffer = 0x007ba058;
u16 *wideBuffer() { return reinterpret_cast<u16 *>(kWideBuffer); }

bool modeIsTwo() {
    u8 &guard = *reinterpret_cast<u8 *>((uintptr_t)0x00fb34c5);
    const u8 bits = guard;
    if (!(bits & 1)) {
        guard = bits | 1;
        g_levels->readMode(reinterpret_cast<u8 *>((uintptr_t)0x00fb3b7c));
        g_levels->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005d1750));
    }
    return *reinterpret_cast<u32 *>((uintptr_t)0x00fb3b7c) == 2;
}

u8 *controlsTable() { return global<u8 *>(0x007cf83c) + 0x1d943c; }

void addRow(u8 *list, u32 first, u32 second, u32 data) {
    const u32 texts[2] = {first, second};
    g_levels->add(list, nullptr, texts, data, -1, 0);
}

void formatLevel(u32 entry, u32 value) {
    const u32 arg[5] = {0x006ff170, 0x5641524d, 0, value, 0};   // (TextArgInt_Ctor (0, the value, unsigned), inline)
    g_levels->format(global<u8 *>(0x0078ba94), wideBuffer(), 0x3fff, entry, arg);
}

// (A percent: the low byte * 1024 / 100.)
s32 scaled(u32 value) { return static_cast<s32>((value & 0xff) << 10) / 100; }

void showBufferSum(u8 *gauge, const u8 *buffer) {
    u32 count = 30;
    const s32 held = buffer[0x84] ? at<s32>(buffer, 0x80) : at<s32>(buffer, 0x7c) + 1;
    if (held < 30) count = CircBufU32_30_Count(buffer);
    double sum = 0;
    for (s32 i = 0; i < static_cast<s32>(count); ++i) {
        u32 value;
        if (CircBufU32_30_Get(buffer, nullptr, &value, i) == 1) sum = X87_Add(static_cast<double>(value), sum);   // (fild qword; fadd rounds to 24 bits)
    }
    vcall<void>(gauge, 0x1c, static_cast<u32>(static_cast<long long>(sum)));   // (__ftol, the low half)
}

void showAge(u8 *label, const u8 *object) {
    const s32 age = g_figures->age(object);
    const u32 arg[5] = {0x006ff170, 0x5641524d, 8, static_cast<u32>(age < 0 ? -age : age), age < 0 ? 1u : 0u};   // (TextArgInt_Ctor (8, the size, its sign), inline)
    u16 text[0x20];
    g_figures->format(global<u8 *>(0x0078ba94), text, 0x1f, 0x1b1, arg);
    g_figures->notify(label, nullptr, static_cast<u32>(reinterpret_cast<uintptr_t>(text)));
}

void showFigures(u8 *self, u8 *window, const u8 *object) {
    if (u8 *const gauge = g_figures->child(window, nullptr, 0x3e21)) showBufferSum(gauge, object + 0x1a8);
    if (u8 *const label = g_figures->child(at<u8 *>(self, 0x14), nullptr, 0x3e1b)) showAge(label, object);
    if (u8 *const c = g_figures->child(at<u8 *>(self, 0x14), nullptr, 0x3e16)) vcall<void>(c, 0x1c, scaled(g_figures->rating(object, nullptr, at<s32>(self, 0x2c), at<s32>(self, 0x34), at<s32>(self, 0x30))));
    if (u8 *const c = g_figures->child(at<u8 *>(self, 0x14), nullptr, 0x3e18)) {
        const float third = static_cast<float>(at<s32>(self, 0x34));
        vcall<void>(c, 0x1c, scaled(g_figures->percent(object, nullptr, static_cast<float>(at<s32>(self, 0x2c)), static_cast<float>(at<s32>(self, 0x30)), *reinterpret_cast<const u32 *>(&third))));
    }
    if (u8 *const c = g_figures->child(at<u8 *>(self, 0x14), nullptr, 0x3e19)) vcall<void>(c, 0x1c, scaled(static_cast<u32>(static_cast<long long>(at<float>(object, 0x44)))));   // (__ftol)
    if (u8 *const c = g_figures->child(at<u8 *>(self, 0x14), nullptr, 0x3e17)) vcall<void>(c, 0x1c, scaled(Object_Field48AsInt(object)));
    u8 *const refund = g_figures->child(at<u8 *>(self, 0x14), nullptr, 0x3e23);
    vcall<void>(refund, 0x1c, g_figures->scrap(object));
}
}  // namespace

void Panel7cc218_SetLevelsEnv(const Panel7cc218LevelsEnv *env) { g_levels = env ? env : &kGameLevelsEnv; }
void Panel7cc218_SetFiguresEnv(const Panel7cc218FiguresEnv *env) { g_figures = env ? env : &kGameFiguresEnv; }

extern "C" void __cdecl Panel7cc218_FillLevels(u32 id, u8 *list, u32 clear, u8 *object) {
    if (clear) g_levels->clear(list);
    u8 *const defs = reinterpret_cast<u8 *>((uintptr_t)0x007890a0);
    u8 *def = g_levels->first(defs);
    const u32 level = object[0x50];
    const u32 buffer = static_cast<u32>(kWideBuffer);
    u32 rows = 0;
    const bool two = modeIsTwo();
    if (!two) {
        for (; def; def = g_levels->next(defs)) {
            if (at<u32>(def, 0x508) || at<u32>(def, 0x284) != id) continue;
            const u32 which = at<u16>(def, 0);
            const u8 *const record = g_levels->find(controlsTable(), nullptr, which, 0);
            if (!record[0x10]) continue;
            g_levels->name(defs, nullptr, which, wideBuffer(), 0x3fff);
            addRow(list, buffer, at<u32>(record, 4), which);
            ++rows;
        }
        const u8 *const own = g_levels->def(object);
        if (g_levels->busy(object)) {
            formatLevel(0x1e, level + 2);
            addRow(list, buffer, 0, 0xffffcd39u);
            rows = 1;
        } else if (static_cast<s32>(level) < 2) {
            const u8 *const record = g_levels->find(controlsTable(), nullptr, at<u16>(object, 0xe), 0);
            const s32 next = static_cast<s32>(level) + 1;
            if (next <= at<s32>(record, 0x14)) {
                formatLevel(0x1d, level + 2);
                addRow(list, buffer, at<u32>(own, (static_cast<u32>(next) << 6) + 0x1b8), static_cast<u32>(-next));
                ++rows;
            }
        }
    }
    if (!rows) {
        g_levels->text(modeIsTwo() ? 0x1b : level == 2 ? 0x1c : 0x1a);
        const u16 *const separators = reinterpret_cast<const u16 *>((uintptr_t)0x007558f0);
        for (u16 *word = g_levels->token(wideBuffer(), separators); word; word = g_levels->token(nullptr, separators))
            addRow(list, static_cast<u32>(reinterpret_cast<uintptr_t>(word)), 0, 0xffffe12eu);
    }
    g_levels->refresh(list);
}

extern "C" void __fastcall Panel7cc218_RefreshFigures(u8 *self) {
    u8 *const object = global<u8 *>(0x007c2658);
    g_figures->def(object);
    if (u8 *const window = at<u8 *>(self, 0x14)) showFigures(self, window, object);
    const u32 status = g_figures->status(object, 2);
    if (status != at<u32>(self, 8)) {
        at<u32>(self, 8) = status;
        vcall<void>(self, 0x24);
        g_figures->buttons(self);
        g_figures->showRide(self);
    }
    if (u8 *const window = at<u8 *>(self, 0x24)) g_figures->levels(at<u16>(object, 0xe), g_figures->child(window, nullptr, 0x1f4d), 1, object);
}

namespace {
const Panel7cc218LevelBoxEnv kLevelBoxGameEnv = {&Window_FindChild, &Window7059f0_NthItemData, &Panel7cc218_Choose, &Window_SendMessage, &Panel7cc218_RefreshFigures, &WindowAt10_Message4, &Window_DwordA0, &Obj_CallMember20Slot14, &Window_CallDefault, &Window7059f0_BuildRows, &Panel7cc218_FillLevels};
const Panel7cc218LevelBoxEnv *g_levelBox = &kLevelBoxGameEnv;
}  // namespace

void Panel7cc218_SetLevelBoxEnv(const Panel7cc218LevelBoxEnv *env) { g_levelBox = env ? env : &kLevelBoxGameEnv; }

s32 __cdecl Panel7cc218_LevelBoxHandler(u8 *window, u32 message, u32 a, u32 b) {
    u8 *const panel = reinterpret_cast<u8 *>((uintptr_t)0x007cc218);
    if (message == 0x14) {
        u8 *const owner = reinterpret_cast<u8 *>((uintptr_t)g_levelBox->dword(window, nullptr, 0));
        *reinterpret_cast<u32 *>(owner + 0x24) = 0;
        g_levelBox->member(owner, nullptr, 1);
    } else if (message == 0x100) {
        if (a == 0xfffffffe) {
            g_levelBox->send(window, 4, 0, 0);
            return 0;
        }
    } else if (message == 0x400) {
        u8 *const list = g_levelBox->child(window, nullptr, a);
        if (list && b != 0xffffffff) {
            u32 data = (u32)(uintptr_t)window;
            g_levelBox->nth(list, nullptr, static_cast<s32>(b), &data);
            if (data != 0xffffe12e) {
                if (data != 0xffffcd39) {
                    if (g_levelBox->choose(panel, nullptr, static_cast<s32>(data), 0) && static_cast<s32>(data) > 0) {
                        g_levelBox->close(panel);
                        return 0;
                    }
                } else {
                    g_levelBox->choose(panel, nullptr, 0, 1);
                }
                g_levelBox->send(window, 4, 0, 0);
                g_levelBox->refresh(panel);
                return 0;
            }
        }
    }
    return g_levelBox->fallback(window, nullptr, message, a, b);
}

void __fastcall Panel7cc218_OpenLevelBox(u8 *self) {
    if (at<u32>(self, 0x24) || at<u32>(self, 0xc)) return;
    at<u8 *>(self, 0x24) = WindowScript_Build(at<u8 *>(self, 0x10), nullptr, reinterpret_cast<u8 *>((uintptr_t)0x00755750), 0x004ae430);
    Obj_CallMember20Slot14(self, nullptr, 0);
    Window_SetDepth(at<u8 *>(self, 0x24), nullptr, (u16)(at<u16>(at<u8 *>(self, 0x10), 0xe0) + 0x11));
    Window_SetDwordA0(at<u8 *>(self, 0x24), nullptr, 0, (u32)(uintptr_t)self);
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    u8 *const part = memory ? TextPart6ff530_Ctor(memory, nullptr, 0) : nullptr;
    Window_SetImpl(Window_FindChild(at<u8 *>(self, 0x24), nullptr, 0x1f4e), nullptr, part);
    Window_Set4c(Window_FindChild(at<u8 *>(self, 0x24), nullptr, 0x1f4e), nullptr, Language_Font(5));
    Window_NotifyC00(Window_FindChild(at<u8 *>(self, 0x24), nullptr, 0x1f4e), nullptr, 1, 1);
    u8 *const label = Window_FindChild(at<u8 *>(self, 0x24), nullptr, 0x1f4e);
    vcall<void>(label, 0x24, (u32)(uintptr_t)Language_WideText(0x1f));
    HoverLabel_SetYellow(Window_FindChild(at<u8 *>(self, 0x24), nullptr, 0x1f4e));
    u8 *const list = Window_FindChild(at<u8 *>(self, 0x24), nullptr, 0x1f4d);
    g_levelBox->buildRows(list, nullptr, reinterpret_cast<u8 *(__cdecl *)()>((uintptr_t)0x004ae700), reinterpret_cast<void(__cdecl *)(u8 *)>((uintptr_t)0x004ae7d0));
    Window7059f0_SetColumnBottom(list, nullptr, 0, 0);
    Window7059f0_SetColumnBottom(list, nullptr, 1, 1);
    Window7059f0_SetHighlight(list, nullptr, reinterpret_cast<u8 *>((uintptr_t)GuiWindows_FindByName(reinterpret_cast<const char *>(0x0074fa84))));
    u8 *const thing = *reinterpret_cast<u8 **>((uintptr_t)0x007c2658);
    g_levelBox->levels(at<u16>(thing, 0xe), list, 0, thing);
}

namespace {
const Panel7cc218HandlerEnv kHandlerGameEnv = {&WindowAt10_Message4, &Panel7cc218_RefreshFigures, &ThingPanel_Handler_48d200, &Window_SetTimer, &Window_StartTimer, &Game_CancelAdvisor, &Sign_Swing, &Dialog7ca150_Open, &Things_StepGroup, &Things_StepGroupBack, &ThingPanel_RestartOrClose_48ccf0,
                                               &ThingGoto_Start, &Panel7cc218_EnterMoveMode, &Object_GetDef, &Panel7cc218_ShowSettings, &Panel7cc218_EnterTileMode, &Panel7cc218_OpenLevelBox, &ThingPanel_SetFlag64_48ce50, reinterpret_cast<u32(__fastcall *)(u8 *, void *, u32)>((uintptr_t)0x004e15b0)};
const Panel7cc218HandlerEnv *g_handler = &kHandlerGameEnv;
}  // namespace

void Panel7cc218_SetHandlerEnv(const Panel7cc218HandlerEnv *env) { g_handler = env ? env : &kHandlerGameEnv; }

s32 __cdecl Panel7cc218_Handler(u8 *window, u32 message, u32 a, u32 b) {
    u8 *const panel = reinterpret_cast<u8 *>((uintptr_t)0x007cc218);
    u8 *const thing = global<u8 *>(0x007c2658);
    switch (message) {
    case 5:
        g_handler->close(panel);
        return 1;
    case 0x10:
        if (a == 0x80080) g_handler->refresh(panel);
        break;
    case 0x14:
        g_handler->cancel(0x129);
        g_handler->cancel(0xcf);
        break;
    case 0x15:
        if ((u16)g_handler->setTimer(window, nullptr, 0x80080, 0xfa0)) g_handler->startTimer(window, nullptr, 0x80080);
        break;
    case 0x1e:
        g_handler->swing();
        break;
    case 0x800: {
        g_handler->def(thing);
        u32 offset = 0, flag = 0;
        if (a == 0x3e2d) offset = 0x30, flag = 2;
        else if (a == 0x3e2f) offset = 0x34, flag = 4;
        else if (a == 0x3e30) offset = 0x2c, flag = 1;
        if (flag) {
            at<u32>(panel, offset) = b;
            vcall<void>(panel, 0x14);
            g_handler->settings(panel, nullptr, (u8)flag);
        }
        break;
    }
    case 0x100:
        switch (a) {
        case 0xfffffffe:
        case 0xffffffff:
            g_handler->close(panel);
            return 0;
        case 0x3e26:
            g_handler->stepBack(panel);
            break;
        case 0x3e27:
            g_handler->stepGroup(panel);
            break;
        case 0x3e29:
            g_handler->open(0, (u32)(uintptr_t)thing);
            return 0;
        case 0x3e2a:
            g_handler->tile(panel, nullptr, 1);
            return 0;
        case 0x3e2b:
            g_handler->goTo(panel);
            return 0;
        case 0x3e2c:
            g_handler->moveMode(panel);
            return 0;
        case 0x3e34:
            g_handler->tile(panel, nullptr, 0);
            return 0;
        case 0x3e35:
            g_handler->levelBox(panel);
            return 0;
        case 0x3e36:
            g_handler->setFlag(panel, nullptr, b == 1 ? 0 : 1);
            return 0;
        case 0x3e37:
            g_handler->thingOp(thing, nullptr, 0);
            vcall<void>(panel, 0x2c);
            return 0;
        case 0x3e38:
            g_handler->restart(panel, nullptr, b == 1 ? 1 : 0);
            g_handler->refresh(panel);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return g_handler->base(window, message, a, b);
}
