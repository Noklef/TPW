// tpw_fncmp: calls any hooked reimplementation by its original address, for tools/fncmp.py
// to compare against the original code under tools/x87emu.py.
//
// usage: tpw_fncmp.exe path\to\tp.exe   then one case per stdin line:
//   ADDRESS CONV RET NARGS ARG... NREGIONS (REGION_ADDRESS HEX)...
// (all hex; CONV c = cdecl, s = stdcall, t = thiscall (first arg in ECX), f = fastcall
// (ECX, EDX); RET d = also fstp the double the function leaves in st0). Prints per case
//   EAX EDX ST0 REGION_HEX...     or "crash CODE" / "nohook".
//
// Memory is laid out as the emulator has it: the game's data sections sit at their own addresses,
// reloaded from tp.exe before each case, and any other region is mapped on demand and released
// after each case. The driver's image is fixed at 0x100000 (CMakeLists.txt), so the game's
// whole image lies in the driver's g_game, a section of its own (.game) that the linker puts
// right after the driver's code; that it covers the range is checked at start-up:
//   0x100000 ..             the driver's headers and code (.text), below the game's image
//   .. 0x400000             g_game's head: no access (nothing of the game's)
//   0x400000 - 0x401000     the game's headers: no access (the emulator has only its sections)
//   0x401000 - 0x6e7000     the game's code, loaded once from tp.exe, read-only: the emulator reads
//                           it (constants, wild pointers) and so does the driver; calling into it
//                           faults, but at the addresses of kRawCalled, each a jump to its port
//   0x6e7000 - 0xfc5200     the game's data (kGameLow - kGameHigh), reloaded before each case
//   .. g_game's end         no access, so a pointer past the game's data faults as in the emulator
//   then                    the driver's .rdata, .data and .bss (the CRT's statics among them)
// Elsewhere: the stubs at kStubs, the case heap at kHeap, the token buffer at 0x5a000000 (no
// access while a case runs) and the regions a case maps (0x10000000 and up), all 64 KB chunks.
// (Until 2026-10-27 the driver was fixed at 0x400000 like the game: its own code covered the game's
// .text, so a raw handler there could not be jumped to, and a pointer into it read driver code.)
//
// Each case runs with the x87 at single precision, as the game runs it (Direct3D sets it),
// so that float code the compiler leaves on the x87 rounds there as it will in the game.
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

#include <cfloat>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <set>
#include <string>
#include <vector>

#include "../../src/game/core/cpuinfo.h"
#include "../../src/game/core/events.h"
#include "../../src/game/gfx/cursor.h"
#include "../../src/game/gfx/pointer.h"
#include "../../src/game/gfx/screen.h"
#include "../../src/game/gfx/statsgraph.h"
#include "../../src/game/ui/thingbrowser.h"
#include "../../src/game/ui/dialog7ca150.h"
#include "../../src/game/ui/thingpanelhandlers.h"
#include "../../src/game/ui/viewbuttons.h"
#include "../../src/game/ui/dialog7cc530open.h"
#include "../../src/game/ui/dialog7cc61c.h"
#include "../../src/game/ui/gamepartdialogs.h"
#include "../../src/game/ui/panel7b9940.h"
#include "../../src/game/world/patrolcoverage.h"
#include "../../src/game/ui/dialog7cc184.h"
#include "../../src/game/ui/signnamedialog.h"
#include "../../src/game/ui/window7cb2a4.h"
#include "../../src/game/ui/dialog7cc984.h"
#include "../../src/game/gfx/movieframe.h"
#include "../../src/game/ui/panel7cc3b0handler.h"
#include "../../src/game/world/rideupkeep.h"
#include "../../src/game/ui/mousehelp.h"
#include "../../src/game/world/staffthink.h"
#include "../../src/game/world/objectride.h"
#include "../../src/game/ui/dialog7cc1f8stats.h"
#include "../../src/game/ui/dialog7cc5f4handler.h"
#include "../../src/game/ui/window7cb2cc.h"
#include "../../src/game/ui/window7cc8bcmenu.h"
#include "../../src/game/ui/window7cc8c0handler.h"
#include "../../src/game/ui/dialog7ca1e0.h"
#include "../../src/game/ui/moneydisplayhandler.h"
#include "../../src/game/ui/window7cc8bchandler.h"
#include "../../src/game/ui/window7cb2b0handler.h"
#include "../../src/game/ui/window705b20thumb.h"
#include "../../src/game/ui/gamepause.h"
#include "../../src/game/ui/gamewindowinput.h"
#include "../../src/game/ui/panel7c2534menus.h"
#include "../../src/game/ui/panel7c2534menus.h"
#include "../../src/game/ui/windowtick.h"
#include "../../src/game/ui/guistartup.h"
#include "../../src/game/ui/guiframe.h"
#include "../../src/game/ui/window705a20char.h"
#include "../../src/game/ui/dialog7cc944list.h"
#include "../../src/game/ui/moneydisplay.h"
#include "../../src/game/ui/panel7ca228handler.h"
#include "../../src/game/world/objectctor.h"
#include "../../src/game/ui/panel7c2534handler.h"
#include "../../src/game/ui/chatnamemenu.h"
#include "../../src/game/ui/dialog7cc984open.h"
#include "../../src/game/ui/dialog7cb2fcopen.h"
#include "../../src/game/ui/mouseclick.h"
#include "../../src/game/ui/dialog7cca70list.h"
#include "../../src/game/ui/dialog7cb2b8tabs.h"
#include "../../src/game/ui/dialog7cc4b4sparkle.h"
#include "../../src/game/render/passwordpart.h"
#include "../../src/game/render/rasterformat.h"
#include "../../src/game/ui/dialog7cb400close.h"
#include "../../src/game/ui/dialog7cc188address.h"
#include "../../src/game/ui/versionlabel.h"
#include "../../src/game/files/userslots.h"
#include "../../src/game/ui/quitconfirm.h"
#include "../../src/game/world/mapsweep.h"
#include "../../src/game/ui/imehostattach.h"
#include "../../src/game/ui/textrecordpart.h"
#include "../../src/game/world/objectwear.h"
#include "../../src/game/world/celleffect.h"
#include "../../src/game/ui/dialog7cb2b8build.h"
#include "../../src/game/ui/dialog7cc4b4update.h"
#include "../../src/game/ui/panel7cc408info.h"
#include "../../src/game/ui/dialuplist.h"
#include "../../src/game/ui/thinggoto.h"
#include "../../src/game/ui/dialogtabhandlers.h"
#include "../../src/game/ui/dialogtabs.h"
#include "../../src/game/ui/thingsign.h"
#include "../../src/game/gfx/dctmacroblock.h"
#include "../../src/game/gfx/tqiframe.h"
#include "../../src/game/ui/dialog7cc1f8preview.h"
#include "../../src/game/ui/dialog7cc188.h"
#include "../../src/game/ui/dialog7cca70.h"
#include "../../src/game/ui/dialog7cc944.h"
#include "../../src/game/ui/dialog7ca1bc.h"
#include "../../src/game/ui/dialog7ca2e8.h"
#include "../../src/game/ui/dialog7ca150rows.h"
#include "../../src/game/ui/menuactions.h"
#include "../../src/game/ui/signrect.h"
#include "../../src/game/ui/dialog7ca210.h"
#include "../../src/game/ui/dialog7cc2ec.h"
#include "../../src/game/ui/dialog7cb25c.h"
#include "../../src/game/ui/dialog7cc1f8.h"
#include "../../src/game/ui/panel7cc218.h"
#include "../../src/game/ui/dialog7ca168.h"
#include "../../src/game/ui/dialog7ca308.h"
#include "../../src/game/ui/panel7cc408.h"
#include "../../src/game/gfx/overlayload.h"
#include "../../src/game/ui/dialog7cb400.h"
#include "../../src/game/ui/panel7ca178.h"
#include "../../src/game/ui/dialog7ca1c8.h"
#include "../../src/game/ui/dialog74fb20.h"
#include "../../src/game/ui/dialogs.h"
#include "../../src/game/ui/panel7c2534.h"
#include "../../src/game/ui/dialog7cb2b8.h"
#include "../../src/game/ui/dialog7ca098.h"
#include "../../src/game/ui/dialog7cdac4.h"
#include "../../src/game/ui/dialogbase.h"
#include "../../src/game/ui/levelmeter.h"
#include "../../src/game/ui/dialog7cb398.h"
#include "../../src/game/ui/thingbrowser.h"
#include "../../src/game/ui/msgtags.h"
#include "../../src/game/world/guardescort.h"
#include "../../src/game/world/staffctors.h"
#include "../../src/game/world/staffsteps.h"
#include "../../src/game/world/staffwork.h"
#include "../../src/game/ui/staffrename.h"
#include "../../src/game/ui/textpanel7cc18c.h"
#include "../../src/game/world/objectmsg.h"
#include "../../src/game/world/objectunlock.h"
#include "../../src/game/core/fpu.h"
#include "../../src/game/core/gameheap.h"
#include "../../src/game/core/string.h"
#include "../../src/game/core/ptrlist.h"
#include "../../src/game/core/regkey.h"
#include "../../src/game/core/stdstring.h"
#include "../../src/game/core/textarg.h"
#include "../../src/game/files/bufsource.h"
#include "../../src/game/files/dirsource.h"
#include "../../src/game/files/drives.h"
#include "../../src/game/files/file.h"
#include "../../src/game/files/handles.h"
#include "../../src/game/files/waddir.h"
#include "../../src/game/files/streamopen.h"
#include "../../src/game/files/wadfind.h"
#include "../../src/game/files/wadsource.h"
#include "../../src/game/files/wrapsources.h"
#include "../../src/game/lib/jpeg6a.h"
#include "../../src/game/lib/jpeg6b.h"
#include "../../src/game/files/zlibcodec.h"
#include "../../src/game/gfx/canvas.h"
#include "../../src/game/gfx/canvasdraw.h"
#include "../../src/game/gfx/convert.h"
#include "../../src/game/gfx/draw.h"
#include "../../src/game/gfx/glyphrender.h"
#include "../../src/game/gfx/imagecodec.h"
#include "../../src/game/gfx/invmap.h"
#include "../../src/game/gfx/pixconv.h"
#include "../../src/game/gfx/surfbuf.h"
#include "../../src/game/gfx/tga.h"
#include "../../src/game/render/d3dprobe.h"
#include "../../src/game/render/enginedebug.h"
#include "../../src/game/world/cameraupdate.h"
#include "../../src/game/files/handles.h"
#include "../../src/game/render/weatherfx.h"
#include "../../src/game/coaster/arreplay.h"
#include "../../src/game/coaster/arreplayaction.h"
#include "../../src/game/coaster/coastersload.h"
#include "../../src/game/frontend/lobbyscript.h"
#include "../../src/game/files/gamepaths.h"
#include "../../src/game/core/globalsave.h"
#include "../../src/game/coaster/rebuild.h"
#include "../../src/game/coaster/carpose.h"
#include "../../src/game/model/rideresource.h"
#include "../../src/game/coaster/placecar.h"
#include "../../src/game/gfx/texcache.h"
#include "../../src/game/frontend/lobbymake.h"
#include "../../src/game/core/gameframe.h"
#include "../../src/game/render/engineerrors.h"
#include "../../src/game/online/chatavatar.h"
#include "../../src/game/online/chatcommand.h"
#include "../../src/game/online/session.h"
#include "../../src/game/online/chatevents.h"
#include "../../src/game/sprite/draw.h"
#include "../../src/game/sprite/script.h"
#include "../../src/game/ui/cursor.h"
#include "../../src/game/ui/uimessage.h"
#include "../../src/game/world/advisorscore.h"
#include "../../src/game/world/research.h"
#include "../../src/game/model/holder.h"
#include "../../src/game/model/objmodel.h"
#include "../../src/game/core/sharedglobals.h"
#include "../../src/game/core/releaselist.h"
#include "../../src/game/core/soundcategory.h"
#include "../../src/game/core/sounddevice.h"
#include "../../src/game/core/mp2synth.h"
#include "../../src/game/world/mapcell.h"
#include "../../src/game/world/mapclick.h"
#include "../../src/game/world/mappress.h"
#include "../../src/game/world/staffcandidate.h"
#include "../../src/game/world/mapline.h"
#include "../../src/game/ui/dialog7ca168handler.h"
#include "../../src/game/ui/dialog7cc530handler.h"
#include "../../src/game/ui/dialog7ca150list.h"
#include "../../src/game/ui/dialog7cb3f8slots.h"
#include "../../src/game/ui/panel7cc328handler.h"
#include "../../src/game/ui/savedialog.h"
#include "../../src/game/ui/savedialogcreate.h"
#include "../../src/game/ui/window7059f0texts.h"
#include "../../src/game/world/bankaccount.h"
#include "../../src/game/world/messagecentre.h"
#include "../../src/game/coaster/registry.h"
#include "../../src/game/world/money.h"
#include "../../src/game/coaster/train.h"
#include "../../src/game/coaster/edittools.h"
#include "../../src/game/coaster/editupdate.h"
#include "../../src/game/coaster/testrun.h"
#include "../../src/game/world/trakcarroute.h"
#include "../../src/game/coaster/typemodel.h"
#include "../../src/game/world/sign.h"
#include "../../src/game/world/flyinglaunch.h"
#include "../../src/game/rsse/rsseexec.h"
#include "../../src/game/rsse/rssetick.h"
#include "../../src/game/world/trakridecars.h"
#include "../../src/game/world/trakcarpose.h"
#include "../../src/game/model/basecells.h"
#include "../../src/game/world/trakcarupdate.h"
#include "../../src/game/world/flyingupdate.h"
#include "../../src/game/render/ribbon.h"
#include "../../src/game/render/batch.h"
#include "../../src/game/render/batchquad.h"
#include "../../src/game/terrain/landscape.h"
#include "../../src/game/model/hoardingmesh.h"
#include "../../src/game/render/tilegrid.h"
#include "../../src/game/render/spritegroups.h"
#include "../../src/game/render/tiledpanel.h"
#include "../../src/game/render/groundshadow.h"
#include "../../src/game/gfx/spriteframe.h"
#include "../../src/game/world/scapeobjects.h"
#include "../../src/game/gfx/spriteframelights.h"
#include "../../src/game/model/ridebase.h"
#include "../../src/game/world/parkentrance.h"
#include "../../src/game/core/keyactions.h"
#include "../../src/game/frontend/viewcontrols.h"
#include "../../src/game/coaster/bendmesh.h"
#include "../../src/game/coaster/carsound.h"
#include "../../src/game/coaster/trainrun.h"
#include "../../src/game/core/wscanner.h"
#include "../../src/game/online/chatcursor.h"
#include "../../src/game/frontend/lobbyscene.h"
#include "../../src/game/world/parkmap.h"
#include "../../src/game/world/rideskin.h"
#include "../../src/game/core/advisorscoring.h"
#include "../../src/game/frontend/sceneleave.h"
#include "../../src/game/terrain/raypick.h"
#include "../../src/game/terrain/ground.h"
#include "../../src/game/terrain/water.h"
#include "../../src/game/render/weatherfx.h"
#include "../../src/game/render/modelgroup.h"
#include "../../src/game/math/transform.h"
#include "../../src/game/world/mousemode.h"
#include "../../src/game/world/person.h"
#include "../../src/game/world/pathdrag.h"
#include "../../src/game/world/cellkind.h"
#include "../../src/game/world/demolish.h"
#include "../../src/game/world/tracksquare.h"
#include "../../src/game/world/placeobject.h"
#include "../../src/game/world/visitorsteps.h"
#include "../../src/game/world/worldsave.h"
#include "../../src/game/world/endthing.h"
#include "../../src/game/world/parkopen.h"
#include "../../src/game/world/gamemode.h"
#include "../../src/game/world/cellmessage.h"
#include "../../src/game/world/trackmessage.h"
#include "../../src/game/world/cellrun.h"
#include "../../src/game/world/trackcircuit.h"
#include "../../src/game/world/mappreview.h"
#include "../../src/game/world/personscripts.h"
#include "../../src/game/core/funnytime.h"
#include "../../src/game/core/wfilebuf.h"
#include "../../src/game/core/channelvoice.h"
#include "../../src/game/core/soundchannel.h"
#include "../../src/game/core/soundsource.h"
#include "../../src/game/core/soundworker.h"
#include "../../src/game/world/staff.h"
#include "../../src/game/world/weather.h"
#include "../../src/game/world/objectupdate.h"
#include "../../src/game/core/soundset.h"
#include "../../src/game/core/soundseed.h"
#include "../../src/game/core/soundmanager.h"
#include "../../src/game/core/soundwait.h"
#include "../../src/game/core/soundobjectslots.h"
#include "../../src/game/world/mapreset.h"
#include "../../src/game/world/staffheld.h"
#include "../../src/game/world/researcher.h"
#include "../../src/game/world/things.h"
#include "../../src/game/world/queuepieces.h"
#include "../../src/game/world/cellmodel.h"
#include "../../src/game/world/rides.h"
#include "../../src/game/world/camera.h"
#include "../../src/game/sprite/loading.h"
#include "../../src/game/world/object.h"
#include "crt.h"
#include "../../src/hook/hooks.h"

namespace {

using u8 = std::uint8_t;
using u32 = std::uint32_t;
using s32 = std::int32_t;

const u32 kGameBase = 0x00400000, kGameCode = 0x00401000;  // the game's image base, its .text
const u32 kGameLow = 0x006e7000, kGameHigh = 0x00fc5200;  // cseg .. .rsrc
const u32 kGameTailEnd = 0x01110000;                       // g_game reaches at least this far (see below)
// The game's whole image, in a section of its own that the linker places right after the driver's .text (the image is fixed at 0x100000,
// so the code ends below 0x400000: load_exe checks it). It reaches from wherever the code ends to past kGameTailEnd: the driver's
// .rdata / .data / .bss follow it, and what follows the game's data natively is g_game's own tail, which load_exe takes all access from
// (when the CRT's statics followed the game's data, a case's wild pointer read natively what the emulator does not have:
// windowtimers.py's 0x65f024 read the CRT's stdout buffer at 0x10d30e4).
#pragma bss_seg(".game")
u8 g_game[kGameTailEnd - 0x00101000];  // (sized for a driver .text of no size at all; it is ~1.8 MB, so g_game ends near 0x12c0000)
#pragma bss_seg()

struct Section {
    u32 address;
    std::vector<u8> raw;
    u32 size;  // virtual size; past raw is zero
};
std::vector<Section> g_sections;
std::set<u32> g_chunks;

void fail(const char *what, u32 value) {
    std::printf("fncmp: %s %08x\n", what, value);
    std::exit(3);
}

void load_exe(const char *path) {
    FILE *f = std::fopen(path, "rb");
    if (!f) fail("cannot open exe", 0);
    std::vector<u8> d;
    u8 buf[65536];
    for (size_t n; (n = std::fread(buf, 1, sizeof buf, f)) > 0;) d.insert(d.end(), buf, buf + n);
    std::fclose(f);
    auto rd16 = [&](u32 o) { return (u32)(d[o] | d[o + 1] << 8); };
    auto rd32 = [&](u32 o) { return rd16(o) | rd16(o + 2) << 16; };
    const u32 lo = (u32)(uintptr_t)g_game, end = lo + (u32)sizeof g_game;  // (page-aligned: the section starts on a page)
    if (lo > kGameBase || end < kGameTailEnd) fail("g_game does not cover the game's image; it starts at", lo);
    const u32 pe = rd32(0x3c), count = rd16(pe + 6), opt = rd16(pe + 20), base = rd32(pe + 52);
    if (base != kGameBase) fail("the exe's base is not 0x400000 but", base);
    for (u32 i = 0; i < count; ++i) {
        const u32 s = pe + 24 + opt + 40 * i;
        const u32 vsize = rd32(s + 8), va = base + rd32(s + 12), rsize = rd32(s + 16), raw = rd32(s + 20);
        Section sec{va, {}, vsize};
        sec.raw.assign(d.begin() + raw, d.begin() + raw + (rsize < vsize ? rsize : vsize));
        if (va >= kGameLow) {
            g_sections.push_back(sec);
            continue;
        }
        // The code: loaded once (read-only, nothing writes it), as the emulator reads it (x87emu's exe_sections: the raw bytes)
        if (va < kGameCode || va + sec.raw.size() > kGameLow) fail("a section of the exe's code out of place at", va);
        std::memcpy(reinterpret_cast<void *>((uintptr_t)va), sec.raw.data(), sec.raw.size());
    }
    // No access below the game's code (g_game's head and the game's headers, which the emulator does not have) or past the game's data
    // (nothing is the game's and no region is mapped there: tools/fncmp.py's GAME_HIGH), so a wild pointer there faults natively as in
    // the emulator; the code read-only (map_raw_called makes the pages of its jumps executable).
    const u32 tail = (kGameHigh + 0xfff) & ~0xfffu;
    DWORD was;
    if (!VirtualProtect((void *)(uintptr_t)lo, kGameCode - lo, PAGE_NOACCESS, &was)) fail("cannot protect g_game's head at", lo);
    if (!VirtualProtect((void *)(uintptr_t)kGameCode, kGameLow - kGameCode, PAGE_READONLY, &was)) fail("cannot protect the game's code at", kGameCode);
    if (!VirtualProtect((void *)(uintptr_t)tail, end - tail, PAGE_NOACCESS, &was)) fail("cannot protect g_game's tail at", tail);
}

void reset_memory() {
    for (const Section &s : g_sections) {
        u8 *p = reinterpret_cast<u8 *>((uintptr_t)s.address);
        std::memcpy(p, s.raw.data(), s.raw.size());
        std::memset(p + s.raw.size(), 0, s.size - s.raw.size());
    }
    // Released, not zeroed: a chunk an earlier case mapped stayed reachable natively where the emulator, which maps only this case's
    // chunks, faults (dialogbuild.py's HelpBg_Build wrote 0x2e020000, just past its case's heap, into an earlier case's bigger one).
    for (u32 c : g_chunks) VirtualFree(reinterpret_cast<void *>((uintptr_t)c), 0, MEM_RELEASE);
    g_chunks.clear();
}

void map(u32 addr, u32 size) {
    const u32 end = (u32)(uintptr_t)g_game + sizeof g_game;
    if (addr >= kGameLow && addr + size <= end) return;  // (the game's data, g_game's; past kGameHigh its tail, no access)
    if (addr < kGameLow) fail("region in the game's code or below it", addr);
    for (u32 p = addr & ~0xffffu; p < addr + size; p += 0x10000) {  // allocation granularity
        if (g_chunks.count(p)) continue;
        if (!VirtualAlloc((void *)(uintptr_t)p, 0x10000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)) fail("cannot map", p);
        g_chunks.insert(p);
    }
}

// Stubs for what a case hands the code under test, at fixed addresses that tools/fncmp.py's
// emulator hooks too: a record stream's read (vtable +8) and write (+0xc) at kStubs and
// kStubs + 0x10. The stream is +4 cursor, +8 bytes left; each copies at most what is left
// from or to the cursor, moves it on and returns the count.
const u32 kStubs = 0x2f000000;

s32 __fastcall stub_read(u8 *self, void *, u8 *buffer, u32 count) {
    u32 &cursor = *reinterpret_cast<u32 *>(self + 4), &left = *reinterpret_cast<u32 *>(self + 8);
    const u32 n = count < left ? count : left;
    std::memmove(buffer, reinterpret_cast<const void *>((uintptr_t)cursor), n);
    cursor += n;
    left -= n;
    return (s32)n;
}

s32 __fastcall stub_write(u8 *self, void *, const u8 *buffer, u32 count) {
    u32 &cursor = *reinterpret_cast<u32 *>(self + 4), &left = *reinterpret_cast<u32 *>(self + 8);
    const u32 n = count < left ? count : left;
    std::memmove(reinterpret_cast<void *>((uintptr_t)cursor), buffer, n);
    cursor += n;
    left -= n;
    return (s32)n;
}

// The heap a case hands the code (tools/fncmp.py heap(), emulated the same way there): a bump
// allocator whose state is the head of its region at kHeap, +0 next block, +4 a hash of the
// sizes asked for, +8 frees, +0xc a hash of the freed pointers. Blocks are 16-byte aligned;
// nothing is reused.
const u32 kHeap = 0x2e000000;

void *__cdecl heap_alloc(u32 size) {
    u32 *h = reinterpret_cast<u32 *>((uintptr_t)kHeap);
    const u32 countdown = h[0] & 15, p = h[0] & ~15u;  // (fncmp.heap(fail=): the allocation that answers 0)
    h[1] = h[1] * 33 ^ size;
    if (countdown == 1) {
        h[0] = p;
        return nullptr;
    }
    h[0] = p + ((size + 15) & ~15u) + (countdown ? countdown - 1 : 0);
    return reinterpret_cast<void *>((uintptr_t)p);
}

void __cdecl heap_free(void *block) {
    u32 *h = reinterpret_cast<u32 *>((uintptr_t)kHeap);
    ++h[2];
    h[3] = h[3] * 33 ^ (u32)(uintptr_t)block;
}

// GlobalAlloc / GlobalFree (stdcall) for code that calls through the game's import slots
// (tools/fncmp.py global_heap() points them here, at kStubs + 0x20 and + 0x30): the same heap;
// GlobalAlloc also hashes the flags in and zeroes the block got for GMEM_ZEROINIT (0x40).
void *__stdcall stub_global_alloc(u32 flags, u32 size) {
    u8 *p = static_cast<u8 *>(heap_alloc(size));
    u32 *h = reinterpret_cast<u32 *>((uintptr_t)kHeap);
    h[1] = h[1] * 33 ^ flags;
    if (flags & 0x40 && p) std::memset(p, 0, size);  // (not for a failed allocation)
    return p;
}

void *__stdcall stub_global_free(void *block) {
    heap_free(block);
    return nullptr;
}

// zlib's alloc_func / free_func (cdecl) for a z_stream's zalloc / zfree (kStubs + 0xa0 / + 0xb0, tools/fncmp.py ZALLOC / ZFREE): the same
// heap, with opaque and items hashed in before the size (items * size), and opaque before the freed block.
void *__cdecl stub_zalloc(u32 opaque, u32 items, u32 size) {
    u32 *h = reinterpret_cast<u32 *>((uintptr_t)kHeap);
    h[1] = (h[1] * 33 ^ opaque) * 33 ^ items;
    return heap_alloc(items * size);
}

void __cdecl stub_zfree(u32 opaque, void *block) {
    u32 *h = reinterpret_cast<u32 *>((uintptr_t)kHeap);
    h[3] = h[3] * 33 ^ opaque;
    heap_free(block);
}

// The fake CPU a case maps at kCpu (tools/fncmp.py CPU, cpu_region(); its emulator answers CPUID and EFLAGS.ID from the same bytes): +0 1 when
// EFLAGS.ID can be toggled, then from +0x10 the (EAX, EBX, ECX, EDX) answers for leaf 0, 1, 0x80000001 and any other leaf.
const u32 kCpu = 0x2d000000;

u32 __cdecl stub_has_cpuid() { return *reinterpret_cast<const u32 *>((uintptr_t)kCpu) != 0; }

void __cdecl stub_cpuid(u32 leaf, u32 *regs) {
    const u32 i = leaf == 0 ? 0 : leaf == 1 ? 1 : leaf == 0x80000001 ? 2 : 3;
    std::memcpy(regs, reinterpret_cast<const void *>((uintptr_t)(kCpu + 0x10 + 0x10 * i)), 16);
}

// A constructor / destructor for code that calls one through a pointer (kStubs + 0x40, tools/fncmp.py STUBS + 0x40): thiscall, no other arguments;
// puts a vtable pointer (0x6fd800) in the first dword of the object.
void __fastcall stub_mark(u32 *self) { *self = 0x006fd800; }

u32 __cdecl record_call(u32 k, u32 n, u32 self, const u32 *stack);

// A callback the code under test calls through a function pointer, cdecl with one argument (kStubs + 0x50, tools/fncmp.py CDECL_LOG): logged as the COM-style entry k 0x13f.
u32 __cdecl stub_cdecl_log(u32 arg) {
    const u32 stack[2] = {0, arg};
    return record_call(0x13f, 1, 0, stack);
}

// The same with two arguments (kStubs + 0x80, tools/fncmp.py CDECL_LOG2): the entry k 0x13e.
u32 __cdecl stub_cdecl_log2(u32 a, u32 b) {
    const u32 stack[3] = {0, a, b};
    return record_call(0x13e, 2, 0, stack);
}

u32 wide_hash(u32 h, const u16 *s) {
    if (!s) return h;
    for (u32 i = 0; i < 0x400 && s[i]; ++i) h = (h ^ s[i]) * 0x01000193u;
    return h;
}

// A text argument of the formatter's (src/game/core/textarg.h) as one dword: its vtable, tag and number, then a string's text (vtable 0x6ff174;
// it lives on the stack) or the dwords +0xc and +0x10 (a number's); 0 for none.
u32 text_arg_hash(const u32 *arg) {
    if (!arg) return 0;
    u32 h = 0x811c9dc5u;
    for (u32 i = 0; i < 3; ++i) h = (h ^ arg[i]) * 0x01000193u;
    if (arg[0] == 0x006ff174u) return wide_hash(h, reinterpret_cast<const u16 *>((uintptr_t)arg[3]));
    h = (h ^ arg[3]) * 0x01000193u;
    return (h ^ arg[4]) * 0x01000193u;
}

// The same with four arguments (kStubs + 0x90, tools/fncmp.py CDECL_LOG4): the entry k 0x13d. As the text formatter 0x6acd60 (fncmp.text_args()):
// with a count of text arguments (1-5) at log + 0x7e0, that many more (pointers to them) logged as text_arg_hash()es, and the buffer (b; c its size
// in characters, 9 or more) given the hash of the text's id and those as 8 hex digits, so that what shows the text compares it. With a range
// of ids at log + 0x7e4 (its first and last words: text_args()'s only), any other id (d) takes none (its buffer still given the hash): so
// the texts formatted without arguments, and other calls of the stub (a window's handler), read nothing past their four.
u32 __cdecl stub_cdecl_log4(u32 a, u32 b, u32 c, u32 d, ...) {
    const u32 given = *reinterpret_cast<const u32 *>((uintptr_t)(kStubs + 0x100000 + 0x7e0));
    const u32 only = *reinterpret_cast<const u32 *>((uintptr_t)(kStubs + 0x100000 + 0x7e4));
    const u32 texts = only && (d < (only & 0xffff) || d > (only >> 16)) ? 0 : given;
    u32 stack[10] = {0, a, b, c, d};
    u32 n = 4;
    if (given) {
        va_list rest;
        va_start(rest, d);
        u32 h = (0x811c9dc5u ^ d) * 0x01000193u;
        for (; n < 4 + (texts < 5 ? texts : 5); ++n) {
            stack[1 + n] = text_arg_hash(reinterpret_cast<const u32 *>((uintptr_t)va_arg(rest, u32)));
            h = (h ^ stack[1 + n]) * 0x01000193u;
        }
        va_end(rest);
        if (b && c >= 9) {
            u16 *const to = reinterpret_cast<u16 *>((uintptr_t)b);
            for (u32 i = 0; i < 8; ++i) to[i] = (u16)"0123456789ABCDEF"[h >> (28 - 4 * i) & 0xf];
            to[8] = 0;
        }
    }
    return record_call(0x13d, n, 0, stack);
}

// QueryPerformanceFrequency / QueryPerformanceCounter (stdcall) for code that calls through the game's import slots (kStubs + 0x60, tools/fncmp.py COUNTER;
// the case maps the log): writes the 64-bit value at log + 0x7f0, then steps it by the one at log + 0x7f8; returns bit 0 of the dword at log + 0x7e8, which then shifts right.
s32 __stdcall stub_counter(unsigned long long *value) {
    u8 *const log = reinterpret_cast<u8 *>((uintptr_t)(kStubs + 0x100000));
    unsigned long long *const now = reinterpret_cast<unsigned long long *>(log + 0x7f0);
    *value = *now;
    *now += *reinterpret_cast<unsigned long long *>(log + 0x7f8);
    u32 *const results = reinterpret_cast<u32 *>(log + 0x7e8);
    const s32 ok = (s32)(*results & 1);
    *results >>= 1;
    return ok;
}

// A memory stream's seek (kStubs + 0x70, tools/fncmp.py SEEK; memory_stream()), thiscall (offset, origin) over +4 cursor, +8 bytes left, +0xc start,
// +0x10 size: the new position from the start (origin 0), the cursor (1) or the end (else), or -1 past the end, nothing moved.
s32 __fastcall stub_seek(u8 *self, void *, u32 offset, u32 origin) {
    u32 *const s = reinterpret_cast<u32 *>(self + 4);
    const u32 to = (origin == 0 ? 0 : origin == 1 ? s[0] - s[2] : s[3]) + offset;
    if (to > s[3]) return -1;
    s[0] = s[2] + to;
    s[1] = s[3] - to;
    return (s32)to;
}

// A memory stream's size (kStubs + 0xc0, tools/fncmp.py STREAM_SIZE), thiscall, no arguments: its +0x10, whatever the bytes left (+8) are.
u32 __fastcall stub_stream_size(const u8 *self) { return *reinterpret_cast<const u32 *>(self + 0x10); }
// A memory stream's tell (kStubs + 0xd0, tools/fncmp.py TELL), thiscall, no arguments: the cursor (+4) from the start (+0xc).
u32 __fastcall stub_tell(const u8 *self) { return *reinterpret_cast<const u32 *>(self + 4) - *reinterpret_cast<const u32 *>(self + 0xc); }

// What throws in the game (std::_Xlen, _Xran): a case that gets there reports a crash.
void __cdecl heap_throw() { RaiseException(0xe0000001, 0, 0, nullptr); }

const StdStringEnv kStdStringEnv = {&heap_alloc, &heap_free, &heap_throw, &heap_throw};
// The mouse modes' calls: atexit stub_cdecl_log; agent A's unported 0x5234d0 and 0x52f200 the cdecl recorders 30 (3 arguments) and 24 (2)
// (jpeg_recorder(30, 3) / (24, 2), defined below), as tests/fncmp/mousetile.py hooks the game addresses.
const MouseModeEnv kMouseModeEnv = {&heap_free, reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)(kStubs + 0x50)),
                                    reinterpret_cast<void(__cdecl *)(s32, s32, u32)>((uintptr_t)(kStubs + 0x1000 + 0xa000 + 0x400 * 3 + 0x20 * 30)),
                                    reinterpret_cast<void(__cdecl *)(u32, u32)>((uintptr_t)(kStubs + 0x1000 + 0xa000 + 0x400 * 2 + 0x20 * 24))};
const AdvisorScoreEnv kAdvisorScoreEnv = {reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)(kStubs + 0x50))};  // atexit: stub_cdecl_log
const ResearchEnv kResearchEnv = {reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)(kStubs + 0x50))};  // atexit: stub_cdecl_log
const GlyphRenderEnv kGlyphRenderEnv = {reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)(kStubs + 0x50))};  // atexit: stub_cdecl_log
const ObjModelEnv kObjModelEnv = {reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)(kStubs + 0x50))};  // atexit: stub_cdecl_log
const SharedGlobalsEnv kSharedGlobalsEnv = {reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)(kStubs + 0x50))};  // atexit: stub_cdecl_log
const ReleaseListEnv kReleaseListEnv = {reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)(kStubs + 0x50))};  // atexit: stub_cdecl_log
const SoundCategoryEnv kSoundCategoryEnv = {reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)(kStubs + 0x50))};  // atexit: stub_cdecl_log
const SoundDeviceEnv kSoundDeviceEnv = {reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)(kStubs + 0x50))};  // atexit: stub_cdecl_log
// The person's calls: the cseg angle 0x6e7074 stub_cdecl_log2; 0x501db0 (Visitor_SetState, ported; its own bench is visitorstate.py) the thiscall recorder 26 of 1 argument (recorder(26, 1), defined below), as tests/fncmp/visitorleave.py hooks 0x501db0.
const PersonEnv kPersonEnv = {reinterpret_cast<u32(__cdecl *)(s32, s32)>((uintptr_t)(kStubs + 0x80)),
                              reinterpret_cast<void(__fastcall *)(u8 *, void *, u32)>((uintptr_t)(kStubs + 0x1000 + 0x400 * 1 + 0x20 * 26))};
const PtrListEnv kPtrListEnv = {&heap_alloc};
const GameHeapEnv kGameHeapEnv = {&heap_alloc, &heap_free};
// libjpeg 6b's calls outside itself (src/game/lib/jpeg6b.h): jmemansi.c's malloc / free the heap; jmemmgr.c's getenv, sscanf and
// jpeg_open_backing_store and jerror.c's two sprintf calls cdecl recorders 24 (1 argument), 25 (4), 26 (3), 27 (3) and 28 (10), which the
// benches hook the exe's 0x67fbf0, 0x67f620, 0x6429c0 and 0x67a650 with (tests/fncmp/jpeg6b_memmgr.py, jpeg6b_error.py).
constexpr u32 jpeg_recorder(u32 k, u32 n) { return kStubs + 0x1000 + 0xa000 + 0x400 * n + 0x20 * k; }
const Jpeg6bCrtEnv kJpeg6bCrtEnv = {&heap_alloc, &heap_free, reinterpret_cast<char *(__cdecl *)(const char *)>((uintptr_t)jpeg_recorder(24, 1)),
                                    reinterpret_cast<int(__cdecl *)(const char *, const char *, long *, char *)>((uintptr_t)jpeg_recorder(25, 4)),
                                    reinterpret_cast<void(__cdecl *)(jpeg_common_struct *, void *, long)>((uintptr_t)jpeg_recorder(26, 3)),
                                    reinterpret_cast<int(__cdecl *)(char *, const char *, const char *)>((uintptr_t)jpeg_recorder(27, 3)),
                                    reinterpret_cast<int(__cdecl *)(char *, const char *, int, int, int, int, int, int, int, int)>(
                                        (uintptr_t)jpeg_recorder(28, 10))};
// libjpeg 6a's (src/game/lib/jpeg6a.h), the same: the heap and the same recorders (the benches hook the exe's 6a
// jpeg_open_backing_store, 0x650265, with recorder 26: tests/fncmp/jpeg6a_memmgr.py, jpeg6a_error.py).
const Jpeg6aCrtEnv kJpeg6aCrtEnv = {&heap_alloc, &heap_free, reinterpret_cast<char *(__cdecl *)(const char *)>((uintptr_t)jpeg_recorder(24, 1)),
                                    reinterpret_cast<int(__cdecl *)(const char *, const char *, long *, char *)>((uintptr_t)jpeg_recorder(25, 4)),
                                    reinterpret_cast<void(__cdecl *)(jpeg6a_common_struct *, void *, long)>((uintptr_t)jpeg_recorder(26, 3)),
                                    reinterpret_cast<int(__cdecl *)(char *, const char *, const char *)>((uintptr_t)jpeg_recorder(27, 3)),
                                    reinterpret_cast<int(__cdecl *)(char *, const char *, int, int, int, int, int, int, int, int)>(
                                        (uintptr_t)jpeg_recorder(28, 10))};
const WadSourceEnv kWadSourceEnv = {&heap_alloc, &heap_free};
const BufferedSourceEnv kBufferedSourceEnv = {&heap_alloc, &heap_free};
const SurfaceBufferEnv kSurfaceBufferEnv = {0x00703998, &heap_free};
// atexit is stub_cdecl_log (kStubs + 0x50; the benches hook 0x67a7d0 with fncmp._cdecl_log); the compressor's destructor stays the game's address, which is only
// handed to atexit, never called here.
const WrapSourcesEnv kWrapSourcesEnv = {&heap_alloc, &heap_free, reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)(kStubs + 0x50)),
                                        reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005f80c0), reinterpret_cast<u8 *>((uintptr_t)0x00fa75f8),
                                        reinterpret_cast<ZlibCodec *>((uintptr_t)0x00fa75fc)};

// The recorders (tools/fncmp.py recorder(k, n)): thunks at kStubs + 0x1000 + 0x400 * n + 0x20 * k, thiscall with n stack arguments (popped by the thunk), that
// log (k, n, this, the arguments) at kLog and return a hash of them, ECX left as this. The log is a count, then 12 dwords an entry; room for 40 (calls past them neither logged nor counted). An
// entry holds the first seven arguments and, for more, a hash of the rest.
const u32 kLog = kStubs + 0x100000;  // (the case maps it)
constexpr u32 recorder(u32 k, u32 n) { return kStubs + 0x1000 + 0x400 * n + 0x20 * k; }
// A recorder's address that converts to whichever function pointer an env's field is (the MapClick env's).
struct Rec {
    uintptr_t at;
    template <typename F> operator F() const { return reinterpret_cast<F>(at); }
};
// The queue pieces' and the path drag's calls into agent B's unported 0x463060 and the ports Ride_Destroy 0x463c80 / MapCell_Clear
// 0x5367a0 / 0x536100: recorders (cdecl 30 of 10
// arguments, cdecl 31 of 1, thiscall 28 of 2, cdecl 29 of 5), as tests/fncmp/queuepieces.py and pathdrag.py hook those addresses.
const QueuePiecesEnv kQueuePiecesEnv = {reinterpret_cast<u32(__cdecl *)(u32, u32, const u32 *, u32, u32, u32, u32, u32, s32, u32)>((uintptr_t)jpeg_recorder(30, 10)),
                                        reinterpret_cast<void(__cdecl *)(u32)>((uintptr_t)jpeg_recorder(31, 1))};
const PathDragEnv kPathDragEnv = {reinterpret_cast<void(__fastcall *)(u8 *, void *, u32, u32)>((uintptr_t)recorder(28, 2)),
                                  reinterpret_cast<void(__cdecl *)(u32, s32, s32, s32 *, s32 *)>((uintptr_t)jpeg_recorder(29, 5))};
// The rectangle sweep's demolish (0x527ee0) and agent B's MapCell_Clear (0x5367a0): cdecl recorder 2 of 3 and thiscall recorder 28 of 2, as
// tests/fncmp/demolish.py hooks them. The demolish's calls, as tests/fncmp/deadends.py hooks them: agent B's 0x530120 (ported) cdecl
// recorder 3 of 1, 0x52f200 / 0x52f580 cdecl recorders 4 / 5 of 2, 0x52fe50 cdecl recorder 6 of 2, agent B's 0x53b280 thiscall recorder 7
// of 3 and 0x545610 cdecl recorder 8 of 1.
const DemolishEnv kDemolishEnv = {reinterpret_cast<void(__cdecl *)(s32, s32, u32)>((uintptr_t)jpeg_recorder(2, 3)),
                                  reinterpret_cast<void(__fastcall *)(u8 *, void *, u32, u32)>((uintptr_t)recorder(28, 2)),
                                  reinterpret_cast<void(__cdecl *)(u8 *)>((uintptr_t)jpeg_recorder(3, 1)),
                                  reinterpret_cast<void(__cdecl *)(u32, u32)>((uintptr_t)jpeg_recorder(4, 2)),
                                  reinterpret_cast<void(__cdecl *)(u32, u32)>((uintptr_t)jpeg_recorder(5, 2)),
                                  reinterpret_cast<u32(__cdecl *)(u8 *, u32)>((uintptr_t)jpeg_recorder(6, 2)),
                                  reinterpret_cast<void(__fastcall *)(u8 *, void *, u32, u32, u32)>((uintptr_t)recorder(7, 3)),
                                  reinterpret_cast<void(__cdecl *)(u32)>((uintptr_t)jpeg_recorder(8, 1))};
// The track runs' call into agent B's 0x53afa0 (ported): thiscall recorder 24 of no arguments, as tests/fncmp/trackshape.py hooks it;
// CellRun_Lay's 0x5370e0 (agent B's, unported; recorder 20 of 3 as Map_ApplyLine's, which CellRun_Connect calls) and 0x538fc0 (ported; recorder 26
// of 1); CellRun_Connect's 0x53b280 (recorder 7 of 3, as tests/fncmp/deadends.py's), 0x53bc60 and 0x545450 (cdecl recorders 27 of 3 and
// 28 of 1): as tests/fncmp/cellrun.py hooks them.
const CellRunEnv kCellRunEnv = {reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)recorder(24, 0)),
                                reinterpret_cast<void(__fastcall *)(u8 *, void *, u32, u32, u32)>((uintptr_t)recorder(20, 3)),
                                reinterpret_cast<void(__fastcall *)(u8 *, void *, u32)>((uintptr_t)recorder(26, 1)),
                                reinterpret_cast<void(__fastcall *)(u8 *, void *, s32, s32, u32)>((uintptr_t)recorder(7, 3)),
                                reinterpret_cast<void(__cdecl *)(s32, s32, u32)>((uintptr_t)jpeg_recorder(27, 3)),
                                reinterpret_cast<void(__cdecl *)(u32)>((uintptr_t)jpeg_recorder(28, 1))};
// The track circuit walk's calls into agent B's code: TrakRide_Strip and TrakRide_AddSection cdecl recorders 28 of 1 and 29 of 7,
// MapCell_ApplyModel the thiscall recorder 24 of none, as tests/fncmp/trackcircuit.py hooks them.
const TrackCircuitEnv kTrackCircuitEnv = {reinterpret_cast<void(__cdecl *)(u32)>((uintptr_t)jpeg_recorder(28, 1)),
                                          reinterpret_cast<s32(__cdecl *)(u32, s32, s32, u32, u32, u32, u32)>((uintptr_t)jpeg_recorder(29, 7)),
                                          reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)recorder(24, 0))};
// The track square test's call into agent B's MapCell_HandleMessage (0x532fc0): thiscall recorder 9 of 3, as tests/fncmp/tracksquare.py hooks it.
const TrackSquareEnv kTrackSquareEnv = {reinterpret_cast<u32(__fastcall *)(u8 *, void *, u32, u32, u32)>((uintptr_t)recorder(9, 3))};
// The placed object's constructor (agent C's unported 0x4db090): thiscall recorder 11 of 4, as tests/fncmp/placeobject.py hooks it.
const PlaceObjectEnv kPlaceObjectEnv = {reinterpret_cast<u8 *(__fastcall *)(u8 *, void *, u32, u32, s32, u32)>((uintptr_t)recorder(11, 4))};
// The "End" object's constructor (0x4db090) and its pass to 0x481b60 (agent C's, both unported): thiscall recorders 11 of 4 and 12 of
// 1, as tests/fncmp/endthing.py hooks them.
const EndThingEnv kEndThingEnv = {reinterpret_cast<u8 *(__fastcall *)(u8 *, void *, u32, u32, s32, u32)>((uintptr_t)recorder(11, 4)),
                                  reinterpret_cast<void(__fastcall *)(void *, void *, u8 *)>((uintptr_t)recorder(12, 1))};
// The park opening's object open (agent C's unported 0x4df390): thiscall recorder 13 of none, as tests/fncmp/parkopen.py hooks it; the
// park end's gate call (agent C's unported 0x481b50): thiscall recorder 14 of 1, as tests/fncmp/parkend.py hooks it.
const ParkOpenEnv kParkOpenEnv = {reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)recorder(13, 0)),
                                  reinterpret_cast<void(__fastcall *)(void *, void *, u8 *)>((uintptr_t)recorder(14, 1))};
// The crowd's start: thiscall recorders 1 of 2, 2 and 3 of 2, COM recorder 4 of 2, thiscall recorders 5-7 of 1, as tests/fncmp/parkcrowd.py hooks them.
const ParkCrowdEnv kParkCrowdEnv = {Rec{recorder(1, 2)}, Rec{recorder(2, 2)}, Rec{recorder(3, 2)}, Rec{kStubs + 0x5000 + 0x800 * 2 + 0x20 * 4},
                                    Rec{recorder(5, 1)}, Rec{recorder(6, 1)}, Rec{recorder(7, 1)}};
// The game mode change's advisor cancel, track run show and tool reset (agent B's 0x59aa70, 0x4474d0 and CoasterEdit_CancelSelected 0x446050, all ported): thiscall recorder
// 13 of 1, cdecl recorders 14 of 2 and 15 of none, as tests/fncmp/gamemode.py hooks them.
const GameModeEnv kGameModeEnv = {reinterpret_cast<void(__fastcall *)(u8 *, void *, u32)>((uintptr_t)recorder(13, 1)),
                                  reinterpret_cast<void(__cdecl *)(u32, u32)>((uintptr_t)jpeg_recorder(14, 2)),
                                  reinterpret_cast<void(__cdecl *)()>((uintptr_t)jpeg_recorder(15, 0))};
// Thing_Detach's call into agent B's Ride_Destroy (0x463c80): cdecl recorder 31 of 1, as tests/fncmp/thingdetach.py hooks it.
const ThingDetachEnv kThingDetachEnv = {reinterpret_cast<void(__cdecl *)(u32)>((uintptr_t)jpeg_recorder(31, 1))};
// Thing_Delete's per-kind deletes (agent C's unported 0x4dd0a0, 0x4d9fe0, 0x4d6c90, 0x4d4620, 0x4d5eb0, 0x4c9040; thingdetach.py's k 10-15).
using ThingKindFn = void(__fastcall *)(u8 *);
const ThingDeleteEnv kThingDeleteEnv = {reinterpret_cast<ThingKindFn>((uintptr_t)recorder(10, 0)), reinterpret_cast<ThingKindFn>((uintptr_t)recorder(11, 0)),
                                        reinterpret_cast<ThingKindFn>((uintptr_t)recorder(12, 0)), reinterpret_cast<ThingKindFn>((uintptr_t)recorder(13, 0)),
                                        reinterpret_cast<ThingKindFn>((uintptr_t)recorder(14, 0)), reinterpret_cast<ThingKindFn>((uintptr_t)recorder(15, 0))};
// Thing_HandleMessage's handlers by model - 1: the thiscall recorder k of 1 argument, k the index (models 6 and 8 share 0x4d4450: k 5), as
// tests/fncmp/thingmessage.py hooks their addresses.
using ThingMessageFn = void(__fastcall *)(u8 *, void *, u8 *);
ThingMessageFn thing_message(u32 k) { return reinterpret_cast<ThingMessageFn>((uintptr_t)recorder(k, 1)); }
const ThingMessageEnv kThingMessageEnv = {{thing_message(0), nullptr, thing_message(2), thing_message(3), thing_message(4), thing_message(5),
                                           thing_message(6), thing_message(5), thing_message(8), thing_message(9), thing_message(10),
                                           thing_message(11), thing_message(12), thing_message(13), thing_message(14), thing_message(15),
                                           thing_message(16), thing_message(17), thing_message(18)}};
// StaffAdvice_KindLow's sums (agent C's unported 0x4c8180 / 0x4c8150; staffadvice.py's k 16 / 17) and StaffHQ_HandleMessage's guard call
// (agent C's unported 0x4d6260; staffadvice.py's k 18, thiscall of 1).
using StaffSumFn = s32(__fastcall *)(u8 *, void *, u32);
const StaffAdviceEnv kStaffAdviceEnv = {reinterpret_cast<StaffSumFn>((uintptr_t)recorder(16, 1)), reinterpret_cast<StaffSumFn>((uintptr_t)recorder(17, 1)),
                                        reinterpret_cast<void(__fastcall *)(u8 *, void *, u32)>((uintptr_t)recorder(18, 1))};
// Map_Serialize's per-cell calls, 0x4000 of each, as tests/fncmp/mapserialize.py's hooks: a checksum at PROBE (RAM) of what they are given
// (the temporary cells' id words and +0x26, not their stack addresses); a serializer answers 1 for the cell at PROBE + 4.
u32 &map_probe(u32 index) { return reinterpret_cast<u32 *>((uintptr_t)0x30000000)[index]; }
void map_mix(u32 value) { map_probe(0) = map_probe(0) * 31 + value; }
void __fastcall map_reset_probe(u8 *cell, void *, u32 models) {
    map_mix(*reinterpret_cast<u16 *>(cell) | (u32)cell[0x26] << 16 | (models & 0xff) << 24);
}
void __fastcall track_reset_probe(u8 *cell) { map_mix(*reinterpret_cast<u16 *>(cell)); }
u32 __fastcall serialize_probe(u8 *cell, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    map_mix((u32)(uintptr_t)cell);
    map_mix((u32)(uintptr_t)stream);
    map_mix(save);
    map_mix((u32)version);
    map_mix(unused);
    return (u32)(uintptr_t)cell == map_probe(1) ? 1 : 0;
}
const MapSerializeEnv kMapSerializeEnv = {&map_reset_probe, &track_reset_probe, &serialize_probe, &serialize_probe, &serialize_probe};
// Guard_Detach's Visitor_Arrive: thiscall recorder 19 of 1, as tests/fncmp/staffdetach.py hooks 0x4feb50.
const GuardDetachEnv kGuardDetachEnv = {reinterpret_cast<void(__fastcall *)(u8 *, void *, u32)>((uintptr_t)recorder(19, 1))};
// Ride_Destroy's calls into agent B's unported 0x42a190 and ported 0x436fa0: cdecl recorders 18 of no arguments and 19 of 1, as
// tests/fncmp/ridedestroy.py hooks those addresses.
// The cell kinds' calls into agent C's unported 0x4de1f0 and agent A's Path_CheckDeadEnds (0x527ee0): the thiscall recorder 26 of no
// arguments and the cdecl recorder 27 of 3, as tests/fncmp/cellkind.py and tests/fncmp/trackremove.py hook them.
const CellKindEnv kCellKindEnv = {reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)recorder(26, 0)),
                                  reinterpret_cast<void(__cdecl *)(s32, s32, u32)>((uintptr_t)jpeg_recorder(27, 3))};
// The coaster links' call into agent B's unported 0x463060: the cdecl recorder 30 of 10 arguments, as tests/fncmp/linkcars.py hooks it;
// CoasterLink_AddFixedPieces' calls the cdecl recorders 9 of 3, 10 of 6, 11 of 9, 12 of 2, 13 of 0 and 14 of 2, as
// tests/fncmp/linkfixed.py hooks them; CoasterLink_Create's 15 of 2, 16 of 7, 17 of 1, 18 of 2, 19 of 2, 20 of 3, 21 of 3, 22 of 4 and
// 23 of 5, as tests/fncmp/coastercreate.py hooks them.
const CoasterLinksEnv kCoasterLinksEnv = {Rec{jpeg_recorder(30, 10)}, Rec{jpeg_recorder(9, 3)},  Rec{jpeg_recorder(10, 6)},
                                          Rec{jpeg_recorder(11, 9)},  Rec{jpeg_recorder(12, 2)}, Rec{jpeg_recorder(13, 0)},
                                          Rec{jpeg_recorder(14, 2)},  Rec{jpeg_recorder(15, 2)}, Rec{jpeg_recorder(16, 7)},
                                          Rec{jpeg_recorder(17, 1)},  Rec{jpeg_recorder(18, 2)}, Rec{jpeg_recorder(19, 2)},
                                          Rec{jpeg_recorder(20, 3)},  Rec{jpeg_recorder(21, 3)}, Rec{jpeg_recorder(22, 4)},
                                          Rec{jpeg_recorder(23, 5)}};
// The chat commands' calls (src/game/online/chatcommand), as tests/fncmp/chatcommand.py hooks them: Charset_FormatWide the cdecl recorder
// 25 of 5 (a 4-argument call's fifth is stack garbage, which the bench hashes away); agent B's 0x4051d0 the stand-in ChatCommand_AssignChars
// (0x4ec030's own inlined copy of it; the bench runs B's original); B's 0x5d3230 and 0x5b50b0 the thiscall recorders 21 of 2 and 22 of 0;
// Window7cc8bc_ChatAddStd, _ListAddStd and _ListAdd1Std the COM-style (stdcall) recorders 24, 27 and 28 of 2; B's 0x47f020 the cdecl recorder 26 of 8.
const ChatCommandEnv kChatCommandEnv = {Rec{jpeg_recorder(25, 5)}, &ChatCommand_AssignChars, Rec{recorder(21, 2)},
                                        Rec{kStubs + 0x5000 + 0x800 * 2 + 0x20 * 24}, Rec{kStubs + 0x5000 + 0x800 * 2 + 0x20 * 27},
                                        Rec{kStubs + 0x5000 + 0x800 * 2 + 0x20 * 28}, Rec{recorder(22, 0)}, Rec{jpeg_recorder(26, 8)}};
const ChatEventEnv kChatEventEnv = {Rec{kStubs + 0x5000 + 0x800 * 2 + 0x20 * 29}, Rec{kStubs + 0x5000 + 0x800 * 2 + 0x20 * 30},
                                    Rec{kStubs + 0x5000 + 0x800 * 1 + 0x20 * 31}, Rec{kStubs + 0x5000 + 0x800 * 1 + 0x20 * 32},
                                    Rec{recorder(3, 4)}, Rec{recorder(4, 0)}, Rec{recorder(5, 1)}, Rec{recorder(6, 3)}, Rec{recorder(7, 2)},
                                    Rec{recorder(8, 2)}, Rec{recorder(10, 0)}, Rec{recorder(11, 5)}, Rec{jpeg_recorder(14, 3)}, Rec{recorder(13, 2)},
                                    Rec{jpeg_recorder(25, 7)}, Rec{jpeg_recorder(15, 1)},
                                    // (tests/fncmp/chatqueue.py's)
                                    Rec{jpeg_recorder(16, 2)}, Rec{jpeg_recorder(17, 0)}, Rec{recorder(18, 2)}, Rec{recorder(19, 0)}, Rec{recorder(20, 1)},
                                    Rec{recorder(4, 1)}, Rec{recorder(6, 1)}};
// The cell messages' calls into agent A's Path_CheckDeadEnds (0x527ee0) and GameMode_SetOnly (0x52f580): the cdecl recorders 27 of 3 and 28 of 2, as
// tests/fncmp/cellmessage.py hooks them.
const CellMessageEnv kCellMessageEnv = {reinterpret_cast<void(__cdecl *)(s32, s32, u32)>((uintptr_t)jpeg_recorder(27, 3)),
                                        reinterpret_cast<void(__cdecl *)(u32, u32)>((uintptr_t)jpeg_recorder(28, 2))};
// The track cell messages' (0x5370e0) calls: the thiscall recorder 1 of 3, the cdecl 2 of 11, the thiscall 3 and 4 of 3, 5 of 2, 6 and 7
// of 0 and 8 of 1, the cdecl 9 and 10 of 4 and 11 of 3, the thiscall 12 of 2 and the cdecl 13 of 2, as tests/fncmp/trackmessage.py hooks them.
const TrackMessageEnv kTrackMessageEnv = {Rec{recorder(1, 3)},       Rec{jpeg_recorder(2, 11)}, Rec{recorder(3, 3)},       Rec{recorder(4, 3)},
                                          Rec{recorder(5, 2)},       Rec{recorder(6, 0)},       Rec{recorder(7, 0)},       Rec{recorder(8, 1)},
                                          Rec{jpeg_recorder(9, 4)},  Rec{jpeg_recorder(10, 4)}, Rec{jpeg_recorder(11, 3)}, Rec{recorder(12, 2)},
                                          Rec{jpeg_recorder(13, 2)}};
// The coaster editor's frame update's (0x443340) calls: the cdecl recorders 1 of 1, 2 of 0, 3 and 4 of 2, 5 of 3, 6 of 9, 7 and 8 of 7, 9 of
// 5 and 10 of 7 (its answer a float in st0), as tests/fncmp/editupdate.py hooks them.
const EditUpdateEnv kEditUpdateEnv = {Rec{jpeg_recorder(1, 1)}, Rec{jpeg_recorder(2, 0)}, Rec{jpeg_recorder(3, 2)}, Rec{jpeg_recorder(4, 2)},
                                      Rec{jpeg_recorder(5, 3)}, Rec{jpeg_recorder(6, 9)}, Rec{jpeg_recorder(7, 7)}, Rec{jpeg_recorder(8, 7)},
                                      Rec{jpeg_recorder(9, 5)}, Rec{jpeg_recorder(10, 7)}};
// The coaster test run's (0x43c450) calls: the cdecl recorders 1 of 1, 2 of 7, 3 of 4, 4 and 5 of 2, 6 of 3, 7 of 1, 8 of 2 and 9 of 1, as
// tests/fncmp/testrun.py hooks them.
const RibbonEnv kRibbonEnv = {Rec{recorder(2, 5)}};
const BatchEnv kBatchEnv = {Rec{recorder(1, 1)}, Rec{recorder(2, 2)}};
const ViewCtlEnv kViewCtlEnv = {reinterpret_cast<void(__cdecl *)(u8 *)>((uintptr_t)jpeg_recorder(15, 1))};
const BatchQuadEnv kBatchQuadEnv = {Rec{recorder(11, 5)}, Rec{recorder(12, 5)}, Rec{recorder(13, 2)}, Rec{recorder(14, 0)}, Rec{recorder(15, 2)}, Rec{recorder(16, 1)}};
const LandscapeDrawEnv kLandscapeDrawEnv = {Rec{recorder(11, 5)}, Rec{recorder(15, 4)}, Rec{recorder(16, 1)}, Rec{jpeg_recorder(17, 1)}};
// tests/fncmp/terrainwater.py: the transformer's slot 1 is 0x579330 in the original; here its port. ClipPoly_Vertices is stdcall (the COM-style recorder 3 of 5, recorder(k, n, True) in fncmp.py).
u32 __fastcall water_transform(void *object, void *, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f) {
    Transform_Vertices(nullptr, static_cast<const float *>(object), reinterpret_cast<float *>((uintptr_t)a), reinterpret_cast<const u8 *>((uintptr_t)b), (s32)c, (s32)d, f, (s32)e);
    return 0;
}
const TerrainWaterEnv kTerrainWaterEnv = {Rec{recorder(1, 5)}, Rec{recorder(2, 1)}, &water_transform, Rec{kStubs + 0x1000 + 0x4000 + 0x800 * 5 + 0x20 * 3}};
const HoardingMeshEnv kHoardingMeshEnv = {Rec{jpeg_recorder(1, 2)}, Rec{jpeg_recorder(2, 3)}, Rec{jpeg_recorder(3, 4)}};
const TileGridEnv kTileGridEnv = {Rec{recorder(1, 2)}};
const SpriteGroupsEnv kSpriteGroupsEnv = {Rec{recorder(1, 2)}};
const TiledPanelEnv kTiledPanelEnv = {Rec{recorder(1, 2)}};
const GroundShadowEnv kGroundShadowEnv = {Rec{recorder(1, 2)}, Rec{recorder(2, 0)}};
const SpriteFrameLightsEnv kSpriteFrameLightsEnv = {Rec{recorder(2, 0)}, Rec{recorder(3, 0)}, Rec{recorder(4, 4)}, Rec{recorder(1, 2)}};
// RideBase_Prepare's fit has 28 stack dwords, past a recorder's 15: the cdecl recorder 1 of 13, the matrix by address (tests/fncmp/ridebase.py's hook does the same).
void __cdecl ride_base_fit(const u8 *node, u8 *frame, u16 **cursor, GridFitMatrix matrix, const u8 *grid, u32 place, s32 *hits, float lift, u8 **best, s32 *bestCount, u32 flags,
                           const u8 *only, u32 fine) {
    using Fit = void(__cdecl *)(const u8 *, u8 *, u16 **, const GridFitMatrix *, const u8 *, u32, s32 *, float, u8 **, s32 *, u32, const u8 *, u32);
    reinterpret_cast<Fit>((uintptr_t)jpeg_recorder(1, 13))(node, frame, cursor, &matrix, grid, place, hits, lift, best, bestCount, flags, only, fine);
}
const RideBaseEnv kRideBaseEnv = {&ride_base_fit, Rec{jpeg_recorder(2, 5)}, Rec{jpeg_recorder(3, 2)}};
const ParkEntranceEnv kParkEntranceEnv = {Rec{jpeg_recorder(1, 2)}, Rec{jpeg_recorder(2, 1)}, Rec{jpeg_recorder(3, 4)}, Rec{jpeg_recorder(4, 1)}, Rec{jpeg_recorder(5, 0)}, Rec{jpeg_recorder(6, 6)}, Rec{recorder(7, 0)}, Rec{jpeg_recorder(8, 0)}, Rec{jpeg_recorder(9, 1)}};
const CarSoundUpdateEnv kCarSoundUpdateEnv = {Rec{jpeg_recorder(1, 0)}, Rec{jpeg_recorder(2, 3)}, Rec{jpeg_recorder(3, 4)}, Rec{jpeg_recorder(4, 5)}};
const SceneLeaveEnv kSceneLeaveEnv = {Rec{jpeg_recorder(20, 1)}, Rec{jpeg_recorder(21, 6)}, Rec{jpeg_recorder(22, 0)}, Rec{recorder(23, 0)}, Rec{recorder(24, 0)}, Rec{recorder(25, 0)},
                                       Rec{jpeg_recorder(26, 0)}, Rec{jpeg_recorder(27, 0)}, Rec{jpeg_recorder(28, 0)}, Rec{jpeg_recorder(29, 0)}};
const AdvisorCriteriaEnv kAdvisorCriteriaEnv = {Rec{recorder(1, 1)}, Rec{jpeg_recorder(2, 0)}, Rec{recorder(3, 1)}, Rec{recorder(4, 0)}, Rec{recorder(5, 0)}, Rec{recorder(6, 2)}};
const CellPaintEnv kCellPaintEnv = {Rec{jpeg_recorder(1, 6)}, Rec{jpeg_recorder(2, 3)}};
const RideSkinEnv kRideSkinEnv = {Rec{jpeg_recorder(1, 1)}};
// (Class702cb0_Step's: thiscall 3 of 2 arguments, thiscall 4 of none, cdecl 6 of 6, cdecl 7 of none, thiscall 5 of 8, as tests/fncmp/lobbystep.py hooks those addresses.)
// (ParkMap_Draw's: thiscall 10 of 1 argument, 11 of none, 12 of 3, 13 of none, as tests/fncmp/parkmapdraw.py hooks those addresses.)
const ParkMapEnv kParkMapEnv = {Rec{recorder(10, 1)}, Rec{recorder(11, 0)}, Rec{recorder(12, 3)}, Rec{recorder(13, 0)}};
const LobbyMakeEnv kLobbyMakeEnv = {Rec{jpeg_recorder(20, 11)}, Rec{jpeg_recorder(21, 10)}, Rec{jpeg_recorder(22, 1)}, Rec{jpeg_recorder(23, 3)}, Rec{recorder(24, 1)}, Rec{recorder(25, 1)}, Rec{jpeg_recorder(26, 1)}};
const CoasterPlaceCarEnv kCoasterPlaceCarEnv = {Rec{jpeg_recorder(1, 6)}, Rec{jpeg_recorder(2, 9)}, Rec{jpeg_recorder(3, 7)}, Rec{jpeg_recorder(4, 5)}, Rec{jpeg_recorder(5, 2)}, Rec{jpeg_recorder(6, 7)},
                                                 Rec{jpeg_recorder(7, 1)}, Rec{jpeg_recorder(8, 1)}, Rec{jpeg_recorder(9, 0)}, Rec{jpeg_recorder(10, 0)}, Rec{jpeg_recorder(11, 0)}, Rec{jpeg_recorder(12, 0)},
                                                 Rec{recorder(13, 0)}, Rec{recorder(14, 0)}, Rec{recorder(15, 1)}};  // tests/fncmp/coasterplacecar.py
const RideResourceEnv kRideResourceEnv = {Rec{jpeg_recorder(1, 1)}, Rec{jpeg_recorder(2, 3)}, Rec{jpeg_recorder(3, 5)}, Rec{jpeg_recorder(4, 5)}, Rec{jpeg_recorder(5, 3)}, Rec{jpeg_recorder(6, 3)}, Rec{jpeg_recorder(7, 1)}, Rec{jpeg_recorder(8, 6)}, Rec{jpeg_recorder(9, 1)}, Rec{recorder(10, 1)}, Rec{jpeg_recorder(11, 6)}};
const CoasterPoseEnv kCoasterPoseEnv = {Rec{jpeg_recorder(1, 3)}, Rec{jpeg_recorder(2, 2)}, Rec{jpeg_recorder(3, 3)}, Rec{jpeg_recorder(4, 2)}};  // tests/fncmp/carpose.py
const CoasterRebuildEnv kCoasterRebuildEnv = {Rec{jpeg_recorder(1, 0)}, Rec{jpeg_recorder(2, 0)}, Rec{jpeg_recorder(3, 1)}, Rec{jpeg_recorder(4, 1)}, Rec{jpeg_recorder(5, 1)}, Rec{jpeg_recorder(6, 1)},
                                               Rec{jpeg_recorder(7, 1)}, Rec{jpeg_recorder(8, 0)}, Rec{jpeg_recorder(9, 3)}, Rec{jpeg_recorder(10, 5)}, Rec{jpeg_recorder(11, 5)}, Rec{jpeg_recorder(12, 1)}};  // tests/fncmp/coasterrebuild.py
const ArReplayEnv kArReplayEnv = {Rec{jpeg_recorder(3, 0)}, Rec{jpeg_recorder(4, 0)}, Rec{jpeg_recorder(5, 0)}, Rec{jpeg_recorder(6, 0)}, Rec{jpeg_recorder(7, 4)}, Rec{jpeg_recorder(8, 4)},
                                  Rec{recorder(9, 2)}, Rec{recorder(10, 4)}, Rec{jpeg_recorder(11, 2)}, Rec{jpeg_recorder(12, 2)}, Rec{jpeg_recorder(13, 2)}, Rec{jpeg_recorder(14, 3)},
                                  Rec{jpeg_recorder(15, 2)}, Rec{jpeg_recorder(16, 2)}, Rec{jpeg_recorder(17, 3)}};  // tests/fncmp/arreplay.py
// The local handle's address differs between the sides: these pass null as `this` (the bench's hooks clear ECX).
Handle *__fastcall ReloadCtor(Handle *, void *) { return reinterpret_cast<Handle *(__fastcall *)(Handle *, void *)>((uintptr_t)recorder(5, 0))(nullptr, nullptr); }
s32 *__fastcall ReloadOpen(Handle *, void *, s32 *r, u32 a) { return reinterpret_cast<s32 *(__fastcall *)(Handle *, void *, s32 *, u32)>((uintptr_t)recorder(6, 2))(nullptr, nullptr, r, a); }
s32 *__fastcall ReloadFirst(Handle *, void *, s32 *r, u32 a, u32 b) { return reinterpret_cast<s32 *(__fastcall *)(Handle *, void *, s32 *, u32, u32)>((uintptr_t)recorder(7, 3))(nullptr, nullptr, r, a, b); }
s32 *__fastcall ReloadClose(Handle *, void *, s32 *r, u32 a) { return reinterpret_cast<s32 *(__fastcall *)(Handle *, void *, s32 *, u32)>((uintptr_t)recorder(8, 2))(nullptr, nullptr, r, a); }
void __fastcall ReloadDtor(Handle *, void *) { reinterpret_cast<void(__fastcall *)(Handle *, void *)>((uintptr_t)recorder(9, 0))(nullptr, nullptr); }
const WeatherFxReloadEnv kWeatherFxReloadEnv = {Rec{jpeg_recorder(1, 3)}, Rec{recorder(2, 2)}, Rec{recorder(3, 1)}, Rec{recorder(4, 0)},
                                                reinterpret_cast<Handle *(__fastcall *)(Handle *)>(&ReloadCtor), &ReloadOpen, &ReloadFirst, &ReloadClose,
                                                reinterpret_cast<void(__fastcall *)(Handle *)>(&ReloadDtor), Rec{jpeg_recorder(10, 0)}};  // tests/fncmp/weatherreload.py
const CameraUpdateEnv kCameraUpdateEnv = {Rec{recorder(1, 0)}, Rec{recorder(2, 1)}, Rec{jpeg_recorder(3, 4)}, Rec{jpeg_recorder(4, 2)}, Rec{recorder(5, 1)}, Rec{recorder(6, 1)},
                                          Rec{jpeg_recorder(7, 3)}, Rec{jpeg_recorder(8, 2)}};  // tests/fncmp/cameraupdate.py
const LobbySceneEnv kLobbySceneEnv = {Rec{jpeg_recorder(1, 1)}, Rec{recorder(2, 0)}, Rec{recorder(3, 2)}, Rec{recorder(4, 0)}, Rec{jpeg_recorder(6, 6)}, Rec{jpeg_recorder(7, 0)}, Rec{recorder(5, 8)}};
const ChatCursorEnv kChatCursorEnv = {Rec{jpeg_recorder(1, 0)}, Rec{jpeg_recorder(2, 4)}, Rec{jpeg_recorder(3, 0)}, Rec{recorder(4, 1)}, Rec{recorder(5, 1)}};
// tests/fncmp/d3dprobe.py: the clock a recorder.
const SpriteFramesEnv kSpriteFramesEnv = {reinterpret_cast<u32(__fastcall *)(u8 *)>((uintptr_t)(kStubs + 0x1000 + 0x20 * 20))};   // (slot 20's thiscall recorder of no arguments)
const ScapeObjectsEnv kScapeObjectsEnv = {reinterpret_cast<s32 *(__cdecl *)(s32 *, Stream **, const char *, u32)>((uintptr_t)jpeg_recorder(21, 4)),
                                          reinterpret_cast<u32(__cdecl *)(u32, u32, u32, s32, s32, s32)>((uintptr_t)jpeg_recorder(22, 6)),
                                          reinterpret_cast<u32(__cdecl *)(u32, u32, u32, s32, s32, s32, s32)>((uintptr_t)jpeg_recorder(23, 7)),
                                          reinterpret_cast<void(__cdecl *)(u32, u32, u32, u32, u32)>((uintptr_t)jpeg_recorder(24, 5)),
                                          reinterpret_cast<s32(__cdecl *)(s32, s32, s32, s32)>((uintptr_t)jpeg_recorder(25, 4))};
const D3dProbeEnv kD3dProbeEnv = {reinterpret_cast<u32(__cdecl *)()>((uintptr_t)jpeg_recorder(15, 0))};
// tests/fncmp/enginedebug.py: the C library's file calls recorders.
const EngineDebugEnv kEngineDebugEnv = {reinterpret_cast<void *(__cdecl *)(const char *, const char *)>((uintptr_t)jpeg_recorder(11, 2)),
                                        reinterpret_cast<s32(__cdecl *)(void *, const char *, ...)>((uintptr_t)jpeg_recorder(12, 3)),
                                        reinterpret_cast<s32(__cdecl *)(void *)>((uintptr_t)jpeg_recorder(13, 1)),
                                        reinterpret_cast<s32(__cdecl *)(void *)>((uintptr_t)jpeg_recorder(14, 1))};
const TexCacheTraceEnv kTexCacheTraceEnv = {reinterpret_cast<void *(__cdecl *)(const char *, const char *)>((uintptr_t)jpeg_recorder(11, 2)),
                                            reinterpret_cast<u32(__cdecl *)(const void *, u32, u32, void *)>((uintptr_t)jpeg_recorder(12, 4)),
                                            reinterpret_cast<s32(__cdecl *)(void *)>((uintptr_t)jpeg_recorder(13, 1)),
                                            reinterpret_cast<s32(__cdecl *)(void *)>((uintptr_t)jpeg_recorder(14, 1))};  // tests/fncmp/texcachetouch.py
const WKeywordsEnv kWKeywordsEnv = {reinterpret_cast<s32(__cdecl *)(u8 *, u16 *, u32, u32, ...)>((uintptr_t)jpeg_recorder(12, 4))};  // tests/fncmp/wkeywords.py

const KeyCancelEnv kKeyCancelEnv = {Rec{recorder(20, 0)}, Rec{jpeg_recorder(21, 2)}, Rec{jpeg_recorder(22, 6)}, Rec{jpeg_recorder(23, 4)}, Rec{jpeg_recorder(24, 2)}};
const SceneViewsEnv kSceneViewsEnv = {Rec{recorder(20, 0)}, Rec{recorder(21, 0)}};
const BendMeshEnv kBendMeshEnv = {Rec{jpeg_recorder(1, 8)}, Rec{jpeg_recorder(2, 1)}};
const TrainRunEnv kTrainRunEnv = {Rec{jpeg_recorder(1, 1)}, Rec{jpeg_recorder(2, 2)}, Rec{jpeg_recorder(3, 4)}, Rec{jpeg_recorder(4, 3)}, Rec{jpeg_recorder(5, 3)}, Rec{jpeg_recorder(6, 2)}, Rec{jpeg_recorder(7, 3)}};
const RayPickEnv kRayPickEnv = {Rec{jpeg_recorder(1, 2)}, Rec{jpeg_recorder(1, 3)}, Rec{jpeg_recorder(2, 2)}, Rec{jpeg_recorder(3, 2)}, Rec{jpeg_recorder(4, 1)}, Rec{jpeg_recorder(5, 15)}, Rec{jpeg_recorder(1, 10)}, Rec{jpeg_recorder(2, 1)}};
const TerrainGroundEnv kTerrainGroundEnv = {Rec{recorder(4, 0)}, Rec{recorder(5, 2)}, Rec{recorder(6, 0)}, Rec{recorder(7, 3)}, Rec{recorder(1, 5)}, Rec{recorder(11, 5)}, &water_transform,
                                            Rec{kStubs + 0x1000 + 0x4000 + 0x800 * 5 + 0x20 * 3}, Rec{recorder(8, 0)}, Rec{recorder(9, 1)}, Rec{jpeg_recorder(10, 3)}, Rec{recorder(2, 1)}};
const WeatherDrawEnv kWeatherDrawEnv = {Rec{recorder(1, 5)}, Rec{recorder(2, 1)}, Rec{recorder(3, 1)}, Rec{recorder(4, 2)}, Rec{recorder(5, 2)}};
const WeatherDropsEnv kWeatherDropsEnv = {Rec{recorder(2, 5)}, Rec{recorder(3, 5)}, Rec{recorder(4, 1)}, Rec{recorder(5, 1)}};
const ModelGroupEnv kModelGroupEnv = {Rec{jpeg_recorder(1, 6)}, Rec{jpeg_recorder(2, 4)}, Rec{recorder(4, 2)}, Rec{kStubs + 0x1000 + 0x4000 + 0x800 * 5 + 0x20 * 5}};
const FlyingUpdateEnv kFlyingUpdateEnv = {Rec{jpeg_recorder(1, 1)}, Rec{jpeg_recorder(2, 5)}, Rec{jpeg_recorder(3, 2)}, Rec{jpeg_recorder(4, 3)}, Rec{jpeg_recorder(5, 4)},
                                          Rec{jpeg_recorder(6, 3)}, Rec{jpeg_recorder(7, 1)}, Rec{jpeg_recorder(8, 1)}, Rec{jpeg_recorder(9, 2)}};
const TrakCarUpdateEnv kTrakCarUpdateEnv = {Rec{jpeg_recorder(1, 3)}, Rec{jpeg_recorder(2, 2)}, Rec{jpeg_recorder(3, 4)}, Rec{jpeg_recorder(4, 2)}, Rec{jpeg_recorder(5, 1)},
    Rec{jpeg_recorder(6, 1)}, Rec{jpeg_recorder(7, 2)}, Rec{jpeg_recorder(8, 6)}};
const TrakRideCarsEnv kTrakRideCarsEnv = {Rec{jpeg_recorder(1, 2)}, Rec{jpeg_recorder(2, 4)}, Rec{jpeg_recorder(3, 2)}, Rec{jpeg_recorder(4, 6)}, Rec{jpeg_recorder(5, 2)},
    Rec{jpeg_recorder(6, 10)}, Rec{jpeg_recorder(7, 1)}};
// RSSE_Tick's calls: the VM as it is (or a recorder would never end its loop), the rest cdecl recorders, as tests/fncmp/rssetick.py hooks them.
// TrakCars_Update's calls: the cdecl recorders 1-5 of 1, as tests/fncmp/trakcars.py hooks them.
// TrakRide_PoseCars's calls: cdecl recorders, as tests/fncmp/trakcarpose.py's.
const TrakPoseEnv kTrakPoseEnv = {Rec{jpeg_recorder(1, 2)}, Rec{jpeg_recorder(2, 1)}, Rec{jpeg_recorder(3, 3)}, Rec{jpeg_recorder(4, 4)}};
// RideBase_FillCells's calls: cdecl recorders, as tests/fncmp/basecells.py's.
const BaseCellsEnv kBaseCellsEnv = {Rec{jpeg_recorder(1, 1)}, Rec{jpeg_recorder(2, 2)}, Rec{jpeg_recorder(3, 1)}, Rec{jpeg_recorder(4, 5)}};
const TrakCarsEnv kTrakCarsEnv = {Rec{jpeg_recorder(1, 1)}, Rec{jpeg_recorder(2, 1)}, Rec{jpeg_recorder(3, 1)}, Rec{jpeg_recorder(4, 1)}, Rec{jpeg_recorder(5, 1)}};
const RsseTickEnv kRsseTickEnv = {&RSSE_ExecuteInstruction, Rec{jpeg_recorder(29, 2)}, Rec{jpeg_recorder(22, 2)}, Rec{jpeg_recorder(19, 2)}, Rec{jpeg_recorder(3, 2)}, Rec{jpeg_recorder(30, 2)},
    Rec{jpeg_recorder(9, 4)}, Rec{jpeg_recorder(21, 4)}, Rec{jpeg_recorder(29, 4)}, Rec{jpeg_recorder(30, 4)}, Rec{jpeg_recorder(18, 2)}};
const RsseExecEnv kRsseExecEnv = {Rec{jpeg_recorder(31, 0)}, Rec{jpeg_recorder(19, 1)}, Rec{jpeg_recorder(28, 1)}, Rec{jpeg_recorder(2, 5)}, Rec{jpeg_recorder(3, 5)},
    Rec{jpeg_recorder(9, 2)}, Rec{jpeg_recorder(6, 1)}, Rec{jpeg_recorder(7, 1)}, Rec{jpeg_recorder(4, 3)}, Rec{jpeg_recorder(31, 2)},
    Rec{jpeg_recorder(1, 6)}, Rec{jpeg_recorder(2, 4)}, Rec{jpeg_recorder(1, 3)}, Rec{jpeg_recorder(2, 3)}, Rec{jpeg_recorder(3, 3)},
    Rec{jpeg_recorder(6, 2)}, Rec{jpeg_recorder(7, 2)}, Rec{jpeg_recorder(4, 1)}, Rec{jpeg_recorder(5, 1)}, Rec{jpeg_recorder(8, 2)},
    Rec{jpeg_recorder(1, 4)}, Rec{jpeg_recorder(5, 4)}, Rec{jpeg_recorder(4, 4)}, Rec{jpeg_recorder(1, 7)}, Rec{jpeg_recorder(5, 3)},
    Rec{jpeg_recorder(3, 4)}, Rec{jpeg_recorder(3, 6)}, Rec{jpeg_recorder(2, 6)}, Rec{jpeg_recorder(8, 1)}, Rec{jpeg_recorder(2, 7)},
    Rec{jpeg_recorder(22, 1)}, Rec{jpeg_recorder(20, 2)}, Rec{jpeg_recorder(23, 1)}, Rec{jpeg_recorder(25, 2)}, Rec{jpeg_recorder(22, 2)},
    Rec{jpeg_recorder(24, 2)}, Rec{jpeg_recorder(24, 1)}, Rec{jpeg_recorder(26, 1)}, Rec{jpeg_recorder(26, 2)}, Rec{jpeg_recorder(23, 2)},
    Rec{jpeg_recorder(25, 1)}, Rec{jpeg_recorder(27, 2)}, Rec{jpeg_recorder(28, 2)}, Rec{jpeg_recorder(27, 1)}, Rec{jpeg_recorder(14, 2)},
    Rec{jpeg_recorder(18, 1)}, Rec{jpeg_recorder(15, 1)}, Rec{jpeg_recorder(13, 2)}, Rec{jpeg_recorder(10, 1)}, Rec{jpeg_recorder(9, 1)},
    Rec{jpeg_recorder(13, 1)}, Rec{jpeg_recorder(14, 1)}, Rec{jpeg_recorder(12, 1)}, Rec{jpeg_recorder(11, 1)}, Rec{jpeg_recorder(16, 1)},
    Rec{jpeg_recorder(17, 1)}, Rec{jpeg_recorder(12, 2)}, Rec{jpeg_recorder(15, 2)}, Rec{jpeg_recorder(10, 2)}, Rec{jpeg_recorder(11, 2)},
    Rec{jpeg_recorder(1, 1)}, Rec{jpeg_recorder(1, 2)}, Rec{jpeg_recorder(2, 1)}, Rec{jpeg_recorder(3, 1)}, Rec{jpeg_recorder(2, 2)},
    Rec{jpeg_recorder(3, 2)}, Rec{jpeg_recorder(4, 2)}, Rec{jpeg_recorder(5, 2)}, Rec{jpeg_recorder(6, 3)}, Rec{jpeg_recorder(18, 2)},
    Rec{jpeg_recorder(1, 8)}, Rec{jpeg_recorder(17, 2)}, Rec{jpeg_recorder(20, 1)}, Rec{jpeg_recorder(6, 4)}, Rec{jpeg_recorder(16, 2)},
    Rec{jpeg_recorder(21, 1)}, Rec{jpeg_recorder(1, 5)}, Rec{jpeg_recorder(29, 1)}, Rec{jpeg_recorder(30, 1)}};
const FlyingLaunchEnv kFlyingLaunchEnv = {Rec{jpeg_recorder(1, 3)}, Rec{jpeg_recorder(2, 10)}, Rec{jpeg_recorder(3, 7)}, Rec{jpeg_recorder(4, 5)}, Rec{jpeg_recorder(5, 1)}};
const SignEnv kSignEnv = {Rec{jpeg_recorder(1, 10)}, Rec{jpeg_recorder(2, 3)}, Rec{jpeg_recorder(3, 2)}, Rec{jpeg_recorder(4, 1)}, Rec{jpeg_recorder(5, 1)}, Rec{jpeg_recorder(6, 3)}, Rec{jpeg_recorder(7, 6)}};
const TypeModelEnv kTypeModelEnv = {Rec{jpeg_recorder(1, 10)}, Rec{jpeg_recorder(2, 6)}, Rec{jpeg_recorder(3, 3)}, Rec{jpeg_recorder(4, 1)}};
const TrakCarRouteEnv kTrakCarRouteEnv = {Rec{jpeg_recorder(1, 3)}, Rec{jpeg_recorder(2, 10)}, Rec{jpeg_recorder(3, 7)}, Rec{jpeg_recorder(4, 4)}, Rec{jpeg_recorder(5, 2)}};
const TestRunEnv kTestRunEnv = {Rec{jpeg_recorder(1, 1)}, Rec{jpeg_recorder(2, 7)}, Rec{jpeg_recorder(3, 4)}, Rec{jpeg_recorder(4, 2)}, Rec{jpeg_recorder(5, 2)},
                                Rec{jpeg_recorder(6, 3)}, Rec{jpeg_recorder(7, 1)}, Rec{jpeg_recorder(8, 2)}, Rec{jpeg_recorder(9, 1)}};
// (Ride_Destroy's two above.) Ride_Create's (0x463060) calls: the cdecl recorders 1 of 5, 2 of 4, 3 of 3, 4 of 6, 5 of 7, 6 of 10, 7 of 4,
// the thiscall 8 of 4 and the cdecl 9 of 2, 10 of 1, 11 of 3, 12 and 13 of 1, 14 of 6 and 15 to 17 of 1, as tests/fncmp/ridecreate.py
// hooks them.
const RidesEnv kRidesEnv = {reinterpret_cast<void(__cdecl *)()>((uintptr_t)jpeg_recorder(18, 0)), reinterpret_cast<void(__cdecl *)(u32)>((uintptr_t)jpeg_recorder(19, 1)),
                            Rec{jpeg_recorder(1, 5)},  Rec{jpeg_recorder(2, 4)},  Rec{jpeg_recorder(3, 3)},  Rec{jpeg_recorder(4, 6)},
                            Rec{jpeg_recorder(5, 7)},  Rec{jpeg_recorder(6, 10)}, Rec{jpeg_recorder(7, 4)},  Rec{recorder(8, 4)},
                            Rec{jpeg_recorder(9, 2)},  Rec{jpeg_recorder(10, 1)}, Rec{jpeg_recorder(11, 3)}, Rec{jpeg_recorder(12, 1)},
                            Rec{jpeg_recorder(13, 1)}, Rec{jpeg_recorder(14, 6)}, Rec{jpeg_recorder(15, 1)}, Rec{jpeg_recorder(16, 1)},
                            Rec{jpeg_recorder(17, 1)}};
// Camera_SetMode's calls: the cdecl recorders 20 of 1, 22 and 23 of none, 24 of 3 and 25 of 1 and the thiscall 21 of 1, as
// tests/fncmp/cameramode.py hooks Highlight_Suspend, Terrain_ProceduralLeave / Enter, Holder_AddToGrid / Wake and ChatButtons_SetChat.
// Camera_FollowThing's: the cdecl recorders 26 of 3, 27 of 1, 28 and 29 of none, as tests/fncmp/camerafollow.py hooks
// Model_FindPartByKey4, PartStates_Update and Sprites_UseGroundFrames / UseParkFrames.
const CameraEnv kCameraEnv = {Rec{jpeg_recorder(20, 1)}, Rec{recorder(21, 1)},     Rec{jpeg_recorder(22, 0)}, Rec{jpeg_recorder(23, 0)},
                              Rec{jpeg_recorder(24, 3)}, Rec{jpeg_recorder(25, 1)}, Rec{jpeg_recorder(26, 3)}, Rec{jpeg_recorder(27, 1)},
                              Rec{jpeg_recorder(28, 0)}, Rec{jpeg_recorder(29, 0)}, Rec{jpeg_recorder(10, 6)}, Rec{jpeg_recorder(11, 0)}, Rec{jpeg_recorder(30, 6)}};
// MapCell_ApplyModel's calls into agent A's unported placers 0x522b90 / 0x522d50 / 0x522f20: cdecl recorders 20, 21 and 22 of 6 arguments,
// as tests/fncmp/cellapply.py hooks those addresses.
const CellModelEnv kCellModelEnv = {reinterpret_cast<u32(__cdecl *)(s32, s32, s32, s32, s32, u32)>((uintptr_t)jpeg_recorder(20, 6)),
                                    reinterpret_cast<u32(__cdecl *)(s32, s32, s32, s32, s32, u32)>((uintptr_t)jpeg_recorder(21, 6)),
                                    reinterpret_cast<u32(__cdecl *)(s32, s32, s32, s32, s32, u32)>((uintptr_t)jpeg_recorder(22, 6))};
// The objects' calls into agent B's 0x40fb10 (ported) and 0x500870 / 0x5012f0 (agent C's): thiscall recorders 21 of 1 argument, 22 and 23
// of none, as tests/fncmp/objectstate.py hooks those addresses.
// MessageCentre_Post's call into the unported Thing_HandleMessage 0x50b550 (agent C's): the thiscall recorder 27 of 1 argument, as
// tests/fncmp/messagecentre.py hooks that address.
const MessageCentreEnv kMessageCentreEnv = {reinterpret_cast<u32(__fastcall *)(u8 *, void *, u8 *)>((uintptr_t)recorder(27, 1))};
// The statistics graph's lines 0x49aa30 (StatsGraph_Redraw's after): the cdecl recorder 22 (tests/fncmp/gauge.py hooks 0x49aa30 with it).
// The frame-set reloads' calls (NamedList_Find, SpriteFrames_Free, 0x41e740, SpriteFrames_Load 0x587db0, SpriteFrames_MarkTextures): the
// thiscall recorders 1 of 2, 2 of none, 3 and 4 of 2 and 5 of none, as tests/fncmp/useframes.py hooks those addresses.
const SpriteLoadingEnv kSpriteLoadingEnv = {Rec{recorder(1, 2)}, Rec{recorder(2, 0)}, Rec{recorder(3, 2)}, Rec{recorder(4, 2)},
                                            Rec{recorder(5, 0)}};
// ThingBrowser_SendPatrolLines's camera and lines (tests/fncmp/thingpatrol.py).
const ThingPatrolEnv kThingPatrolEnv = {Rec{jpeg_recorder(20, 1)}, Rec{jpeg_recorder(21, 5)}};
// ObjectUnlock_CheckScores's calls: stdcall ones through the COM-style recorders (tools/fncmp.py recorder(k, n, com=True): this logged as 0; tests/fncmp/objectunlockscores.py).
constexpr u32 stdcall_recorder(u32 k, u32 n) { return kStubs + 0x1000 + 0x4000 + 0x800 * n + 0x20 * k; }
const AddrCall kArReplayActionCalls[] = {  // tests/fncmp/arreplayaction.py --table
    {0x447ae0, jpeg_recorder(7, 1)},
    {0x447b20, jpeg_recorder(8, 1)},
    {0x447d60, jpeg_recorder(10, 1)},
    {0x447da0, jpeg_recorder(9, 1)},
    {0x447dd0, jpeg_recorder(11, 1)},
    {0x4682b0, jpeg_recorder(12, 3)},
    {0x4815b0, recorder(19, 1)},
    {0x4e2860, recorder(18, 2)},
    {0x4e2920, recorder(16, 2)},
    {0x4e2950, recorder(15, 1)},
    {0x4e2a50, recorder(17, 1)},
    {0x5276d0, jpeg_recorder(6, 1)},
    {0x52f1b0, jpeg_recorder(5, 2)},
    {0x52f200, jpeg_recorder(1, 2)},
    {0x52f580, jpeg_recorder(2, 2)},
    {0x52fa90, jpeg_recorder(4, 1)},
    {0x52fad0, jpeg_recorder(3, 1)},
    {0x5aef50, recorder(13, 0)},
    {0x5d3230, recorder(14, 2)},
};
const AddrCall kCoastersLoadCalls[] = {  // tests/fncmp/coastersload.py --table
    {0x432c10, jpeg_recorder(10, 6)},
    {0x432df0, jpeg_recorder(6, 4)},
    {0x434060, jpeg_recorder(11, 9)},
    {0x435020, jpeg_recorder(25, 7)},
    {0x435710, jpeg_recorder(7, 2)},
    {0x435730, jpeg_recorder(13, 2)},
    {0x436ac0, jpeg_recorder(23, 5)},
    {0x436d10, jpeg_recorder(22, 3)},
    {0x4386d0, jpeg_recorder(9, 3)},
    {0x43ba90, jpeg_recorder(19, 4)},
    {0x43e520, jpeg_recorder(14, 2)},
    {0x441840, jpeg_recorder(21, 2)},
    {0x4426a0, jpeg_recorder(20, 0)},
    {0x442880, jpeg_recorder(12, 1)},
    {0x442cd0, jpeg_recorder(1, 0)},
    {0x444150, jpeg_recorder(3, 4)},
    {0x4441d0, jpeg_recorder(4, 4)},
    {0x444270, jpeg_recorder(5, 5)},
    {0x44b220, jpeg_recorder(8, 3)},
    {0x45bdd0, jpeg_recorder(17, 1)},
    {0x45be80, jpeg_recorder(16, 3)},
    {0x463ab0, jpeg_recorder(15, 9)},
    {0x468760, jpeg_recorder(24, 7)},
    {0x472f40, jpeg_recorder(18, 1)},
};
const AddrCall kGlobalSaveCalls[] = {  // tests/fncmp/globalsaveio.py --table
    {0x423e90, recorder(4, 3)},
    {0x5b0d70, recorder(5, 3)},
    {0x5b1620, recorder(6, 2)},
    {0x5b1650, recorder(7, 2)},
};
const AddrCall kShareTexCalls[] = {  // tests/fncmp/sharetexpath.py --table
    {0x40ef40, jpeg_recorder(1, 0)},
};
const AddrCall kLobbyScriptCalls[] = {  // tests/fncmp/lobbyscript.py --table
    {0x5c4d50, jpeg_recorder(1, 2)},
    {0x5c4f60, jpeg_recorder(4, 4)},
    {0x5c4f80, jpeg_recorder(5, 1)},
    {0x5c4fa0, jpeg_recorder(2, 3)},
    {0x5c5000, jpeg_recorder(3, 1)},
    {0x5f5660, jpeg_recorder(6, 1)},
    {0x5f5680, jpeg_recorder(7, 1)},
    {0x5d9db0, recorder(10, 7)},
    {0x5da120, recorder(11, 0)},
    {0x5dfa60, recorder(8, 12)},
    {0x5dfca0, recorder(9, 0)},
};
const GameFrameCall kGameFrameCalls[] = {  // tests/fncmp/gameframe.py --table
    {0x402d70, recorder(1, 0)},
    {0x402ea0, recorder(0, 1)},
    {0x402ed0, recorder(21, 0)},
    {0x404140, recorder(1, 1)},
    {0x407d80, recorder(2, 0)},
    {0x407e00, recorder(3, 1)},
    {0x4088a0, recorder(29, 1)},
    {0x409180, recorder(22, 0)},
    {0x409300, recorder(23, 0)},
    {0x4093d0, recorder(2, 1)},
    {0x409480, recorder(28, 1)},
    {0x40ef40, jpeg_recorder(23, 0)},
    {0x40fb10, recorder(26, 1)},
    {0x414d40, recorder(31, 3)},
    {0x41a820, recorder(26, 0)},
    {0x429ba0, jpeg_recorder(25, 4)},
    {0x429df0, jpeg_recorder(0, 1)},
    {0x429e20, jpeg_recorder(1, 1)},
    {0x42b1c0, jpeg_recorder(0, 0)},
    {0x435a30, jpeg_recorder(1, 0)},
    {0x436490, jpeg_recorder(2, 0)},
    {0x443340, jpeg_recorder(3, 0)},
    {0x44a280, jpeg_recorder(0, 3)},
    {0x44e410, jpeg_recorder(2, 1)},
    {0x44e510, jpeg_recorder(4, 0)},
    {0x44e540, jpeg_recorder(5, 0)},
    {0x44e550, jpeg_recorder(6, 0)},
    {0x457a90, jpeg_recorder(21, 0)},
    {0x457c30, jpeg_recorder(22, 0)},
    {0x457d50, jpeg_recorder(3, 1)},
    {0x458590, jpeg_recorder(4, 1)},
    {0x462870, jpeg_recorder(5, 1)},
    {0x4628a0, jpeg_recorder(7, 0)},
    {0x46c0d0, recorder(4, 1)},
    {0x46c350, recorder(5, 1)},
    {0x473440, jpeg_recorder(8, 0)},
    {0x4752e0, jpeg_recorder(9, 0)},
    {0x475360, jpeg_recorder(10, 0)},
    {0x4813b0, recorder(0, 0)},
    {0x4813e0, recorder(3, 0)},
    {0x4813f0, recorder(6, 0)},
    {0x481400, recorder(7, 0)},
    {0x481410, recorder(13, 0)},
    {0x481420, recorder(19, 0)},
    {0x481430, recorder(20, 0)},
    {0x48aff0, jpeg_recorder(11, 0)},
    {0x4c81e0, recorder(12, 0)},
    {0x4c8d30, recorder(13, 4)},
    {0x4e5260, recorder(14, 0)},
    {0x4e5950, recorder(3, 0)},
    {0x4e5be0, recorder(6, 0)},
    {0x4e5ef0, recorder(7, 0)},
    {0x4e6130, recorder(13, 0)},
    {0x4e6150, recorder(8, 0)},
    {0x4e6170, recorder(19, 0)},
    {0x4e6560, recorder(20, 0)},
    {0x516380, recorder(21, 0)},
    {0x5166b0, recorder(22, 0)},
    {0x518f90, recorder(8, 1)},
    {0x519060, recorder(9, 1)},
    {0x519510, recorder(23, 0)},
    {0x51b300, jpeg_recorder(20, 0)},
    {0x51b350, jpeg_recorder(12, 0)},
    {0x51bcb0, jpeg_recorder(13, 0)},
    {0x51bd70, jpeg_recorder(14, 0)},
    {0x51c1d0, jpeg_recorder(0, 9)},
    {0x51c700, jpeg_recorder(6, 1)},
    {0x51e6f0, jpeg_recorder(7, 1)},
    {0x51e730, jpeg_recorder(15, 0)},
    {0x51e770, jpeg_recorder(16, 0)},
    {0x51e790, jpeg_recorder(8, 1)},
    {0x51e7b0, jpeg_recorder(9, 1)},
    {0x51e830, jpeg_recorder(10, 1)},
    {0x51ec50, jpeg_recorder(11, 1)},
    {0x51eda0, jpeg_recorder(17, 0)},
    {0x51ee40, jpeg_recorder(18, 0)},
    {0x51f370, jpeg_recorder(12, 1)},
    {0x51faa0, jpeg_recorder(1, 3)},
    {0x51fd20, jpeg_recorder(25, 0)},
    {0x51fd80, jpeg_recorder(14, 1)},
    {0x520130, jpeg_recorder(26, 0)},
    {0x53e760, jpeg_recorder(15, 1)},
    {0x540900, jpeg_recorder(0, 2)},
    {0x541ed0, jpeg_recorder(27, 0)},
    {0x546c80, jpeg_recorder(28, 0)},
    {0x54df20, jpeg_recorder(29, 0)},
    {0x54e070, jpeg_recorder(30, 0)},
    {0x550950, jpeg_recorder(31, 0)},
    {0x550b30, stdcall_recorder(0, 0)},
    {0x550ca0, recorder(31, 0)},
    {0x550d80, recorder(10, 1)},
    {0x551600, stdcall_recorder(1, 0)},
    {0x5516b0, stdcall_recorder(2, 0)},
    {0x557ab0, stdcall_recorder(3, 0)},
    {0x5584b0, stdcall_recorder(4, 0)},
    {0x55a470, stdcall_recorder(5, 0)},
    {0x55a5a0, stdcall_recorder(6, 0)},
    {0x55ab50, stdcall_recorder(7, 0)},
    {0x55abf0, stdcall_recorder(8, 0)},
    {0x55ce60, jpeg_recorder(16, 1)},
    {0x55d4d0, stdcall_recorder(9, 0)},
    {0x56b790, recorder(11, 1)},
    {0x576260, recorder(12, 1)},
    {0x5765c0, recorder(14, 1)},
    {0x576820, recorder(0, 0)},
    {0x578be0, recorder(0, 3)},
    {0x587380, recorder(1, 3)},
    {0x5879c0, recorder(27, 0)},
    {0x58dd60, recorder(15, 1)},
    {0x5989c0, stdcall_recorder(11, 0)},
    {0x598ad0, stdcall_recorder(10, 0)},
    {0x5996d0, jpeg_recorder(17, 1)},
    {0x599800, stdcall_recorder(12, 0)},
    {0x599880, stdcall_recorder(13, 0)},
    {0x5ac5f0, recorder(16, 1)},
    {0x5ac610, recorder(17, 1)},
    {0x5accf0, recorder(24, 0)},
    {0x5aef50, recorder(17, 0)},
    {0x5af250, recorder(29, 0)},
    {0x5afc40, recorder(9, 0)},
    {0x5b0cb0, recorder(10, 0)},
    {0x5b0cc0, recorder(30, 0)},
    {0x5b50b0, recorder(15, 0)},
    {0x5b5920, recorder(25, 0)},
    {0x5b6480, recorder(24, 0)},
    {0x5b96a0, recorder(26, 0)},
    {0x5b96b0, recorder(5, 0)},
    {0x5bd270, recorder(6, 1)},
    {0x5c5520, recorder(25, 0)},
    {0x5c5560, recorder(18, 1)},
    {0x5c5580, recorder(27, 1)},
    {0x5c7420, recorder(16, 0)},
    {0x5c74b0, recorder(27, 0)},
    {0x5c7d00, recorder(7, 1)},
    {0x5c7f40, recorder(30, 3)},
    {0x5c83b0, recorder(19, 1)},
    {0x5c8650, recorder(28, 0)},
    {0x5d5770, recorder(18, 0)},
    {0x5d5840, recorder(31, 0)},
    {0x5d5970, recorder(28, 0)},
    {0x5d5c50, recorder(29, 0)},
    {0x5d5cc0, recorder(30, 0)},
    {0x5d5cf0, recorder(11, 0)},
    {0x5f5fa0, jpeg_recorder(24, 0)},
    {0x619240, recorder(4, 0)},
    {0x6591e3, stdcall_recorder(14, 0)},
    {0x659201, stdcall_recorder(15, 0)},
    {0x6595a8, jpeg_recorder(19, 0)},
    {0x67a7d0, jpeg_recorder(18, 1)},
    {0x67acf0, stdcall_recorder(36, 5)},
    {0x67afa0, stdcall_recorder(37, 4)},
};
const ObjectUnlockScoresEnv kObjectUnlockScoresEnv = {Rec{recorder(19, 0)}, Rec{stdcall_recorder(20, 2)}, Rec{stdcall_recorder(21, 1)}, Rec{stdcall_recorder(22, 2)},
                                                      Rec{jpeg_recorder(23, 0)}, Rec{recorder(24, 2)}, Rec{recorder(25, 2)}};
// StaffRename_Open's calls (tests/fncmp/staffrenameopen.py).
const StaffRenameEnv kStaffRenameEnv = {Rec{recorder(10, 2)}, Rec{recorder(11, 1)}, Rec{recorder(12, 2)}, Rec{jpeg_recorder(13, 1)}, Rec{recorder(14, 1)}, Rec{recorder(22, 1)}, Rec{recorder(15, 1)}, Rec{jpeg_recorder(16, 1)}, Rec{recorder(17, 1)},
                                        Rec{recorder(18, 2)}, Rec{jpeg_recorder(20, 1)}, Rec{jpeg_recorder(21, 1)}, Rec{recorder(23, 4)}, Rec{recorder(24, 2)}, Rec{jpeg_recorder(25, 1)}, Rec{recorder(26, 1)}, Rec{recorder(27, 0)}, Rec{recorder(28, 0)},
                                        Rec{jpeg_recorder(29, 0)}, Rec{recorder(30, 1)}};
// Dialog7cc1f8_TabHandler's calls (tests/fncmp/dialog7cc1f8tab.py).
const Dialog7cc1f8TabEnv kDialog7cc1f8TabEnv = {Rec{jpeg_recorder(10, 1)}, Rec{jpeg_recorder(11, 2)}, Rec{jpeg_recorder(12, 1)}, Rec{jpeg_recorder(13, 3)}, Rec{recorder(14, 3)}};
// Dialog7ca168_Open's calls (tests/fncmp/dialog7ca168open.py).
const Dialog7ca168OpenEnv kDialog7ca168OpenEnv = {Rec{jpeg_recorder(22, 2)}, Rec{recorder(23, 1)}, Rec{stdcall_recorder(24, 1)}, Rec{recorder(25, 1)}, Rec{recorder(26, 1)}};
// Dialog7cc1f8_Open's calls (tests/fncmp/dialog7cc1f8open.py).
const Dialog7cc1f8OpenEnv kDialog7cc1f8OpenEnv = {Rec{jpeg_recorder(17, 2)}, Rec{jpeg_recorder(18, 5)}, Rec{recorder(19, 1)}, Rec{jpeg_recorder(21, 3)}, Rec{recorder(25, 1)}, Rec{recorder(26, 1)}, Rec{recorder(27, 2)}};
// Dialog7cb25c_Open's calls (tests/fncmp/dialog7cb25copen.py).
const Dialog7cb25cOpenEnv kDialog7cb25cOpenEnv = {reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)(kStubs + 0x50)), Rec{jpeg_recorder(21, 3)}, Rec{recorder(22, 2)}, Rec{recorder(27, 2)}, Rec{jpeg_recorder(17, 1)}, Rec{recorder(19, 1)}, Rec{jpeg_recorder(20, 0)}};
// Dialog7ca210_Open's calls (tests/fncmp/dialog7ca210open.py).
const Dialog7ca210Env kDialog7ca210Env = {reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)(kStubs + 0x50)), Rec{jpeg_recorder(21, 3)}, Rec{recorder(22, 2)}};
// Dialog7cc2ec_Open's calls (tests/fncmp/dialog7cc2ecopen.py).
const Dialog7cc2ecEnv kDialog7cc2ecEnv = {reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)(kStubs + 0x50)), Rec{jpeg_recorder(21, 3)}, Rec{stdcall_recorder(22, 1)}};
// ResearchPanel_Open's calls (tests/fncmp/researchopen.py).
const ResearchOpenEnv kResearchOpenEnv = {reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)(kStubs + 0x50)), Rec{jpeg_recorder(15, 8)}, Rec{jpeg_recorder(16, 1)}, Rec{jpeg_recorder(18, 0)}, Rec{jpeg_recorder(19, 0)}, Rec{recorder(17, 0)}, Rec{recorder(25, 0)}, Rec{jpeg_recorder(21, 1)}, Rec{jpeg_recorder(22, 1)}, Rec{jpeg_recorder(23, 1)}, Rec{recorder(24, 1)}};
// Panel7cc218_LevelBoxHandler's and Panel7cc218_OpenLevelBox's calls (tests/fncmp/panel7cc218levelbox.py).
const Panel7cc218LevelBoxEnv kPanel7cc218LevelBoxEnv = {Rec{recorder(10, 1)}, Rec{recorder(11, 2)}, Rec{recorder(12, 2)}, Rec{jpeg_recorder(13, 4)}, Rec{recorder(14, 0)}, Rec{recorder(15, 0)}, Rec{recorder(16, 1)}, Rec{recorder(17, 1)}, Rec{recorder(18, 3)}, Rec{recorder(27, 2)}, Rec{jpeg_recorder(19, 4)}};
// The staff's constructors' calls (tests/fncmp/staffctors.py).
const StaffCtorsEnv kStaffCtorsEnv = {Rec{recorder(10, 4)}, Rec{recorder(11, 2)}, Rec{jpeg_recorder(12, 4)}, Rec{recorder(13, 1)}, Rec{recorder(14, 1)}, Rec{recorder(15, 1)}, Rec{recorder(16, 1)}, Rec{recorder(17, 2)}, Rec{recorder(18, 2)}, Rec{recorder(19, 0)}, Rec{recorder(20, 0)}, Rec{recorder(21, 0)},
                                      Rec{recorder(22, 1)}, Rec{jpeg_recorder(23, 1)}, Rec{recorder(24, 0)}, Rec{recorder(25, 0)}, Rec{recorder(26, 1)}};
// The staff's update and work states' calls (tests/fncmp/staffsteps.py).
const StaffStepsEnv kStaffStepsEnv = {Rec{recorder(10, 0)}, Rec{recorder(11, 0)}, Rec{recorder(12, 2)}, Rec{recorder(13, 1)}, Rec{recorder(14, 0)}, Rec{recorder(15, 0)}, Rec{recorder(16, 0)}, Rec{recorder(17, 1)}, Rec{recorder(18, 0)}, Rec{recorder(19, 0)}, Rec{recorder(20, 1)}, Rec{recorder(21, 0)}, Rec{recorder(22, 0)}, Rec{recorder(23, 4)}, Rec{recorder(24, 1)}};
// Sign_PlaceOverRect's calls (tests/fncmp/signrect.py).
const SignRectEnv kSignRectEnv = {Rec{jpeg_recorder(10, 1)}, Rec{jpeg_recorder(11, 6)}, Rec{jpeg_recorder(12, 1)}};
// The UI's one-call entries' calls (tests/fncmp/menuactions.py).
const MenuActionsEnv kMenuActionsEnv = {Rec{jpeg_recorder(16, 2)}, Rec{jpeg_recorder(17, 1)}, Rec{jpeg_recorder(18, 2)}, Rec{jpeg_recorder(19, 2)}, Rec{jpeg_recorder(20, 1)}, Rec{jpeg_recorder(21, 0)}, Rec{jpeg_recorder(22, 1)}, Rec{jpeg_recorder(23, 1)}, Rec{jpeg_recorder(24, 1)}, Rec{recorder(25, 1)}, Rec{recorder(26, 1)}, Rec{recorder(27, 0)}, Rec{jpeg_recorder(28, 4)}, Rec{jpeg_recorder(29, 1)}, Rec{jpeg_recorder(30, 1)}, Rec{jpeg_recorder(31, 1)}, Rec{jpeg_recorder(0, 1)}, Rec{recorder(1, 0)}, Rec{recorder(2, 0)}, Rec{recorder(3, 1)}, Rec{jpeg_recorder(4, 1)}, Rec{jpeg_recorder(5, 1)}, Rec{jpeg_recorder(6, 2)}, Rec{jpeg_recorder(7, 4)}, Rec{recorder(8, 0)}, Rec{recorder(9, 0)}, Rec{recorder(10, 0)}, Rec{jpeg_recorder(11, 4)}, Rec{recorder(12, 1)}};
// The objects' lists dialog's rows' calls (tests/fncmp/dialog7ca150rows.py).
const Dialog7ca150RowsEnv kDialog7ca150RowsEnv = {Rec{recorder(10, 0)}, Rec{recorder(13, 1)}, Rec{recorder(14, 0)}, Rec{recorder(15, 0)}, Rec{recorder(16, 0)}, Rec{recorder(17, 0)}, Rec{stdcall_recorder(18, 1)}, Rec{recorder(12, 1)}, Rec{recorder(11, 4)}};
// The dialogs' handlers' calls (tests/fncmp/dialoghandlers.py).
const Dialog7ca2e8HandlerEnv kDialog7ca2e8HandlerEnv = {Rec{jpeg_recorder(10, 4)}, Rec{jpeg_recorder(11, 1)}, Rec{jpeg_recorder(12, 1)}, Rec{jpeg_recorder(13, 3)}, Rec{jpeg_recorder(14, 4)}, Rec{jpeg_recorder(15, 0)}, Rec{jpeg_recorder(16, 0)}, Rec{jpeg_recorder(17, 0)}};
const Dialog7ca1bcHandlerEnv kDialog7ca1bcHandlerEnv = {Rec{jpeg_recorder(18, 0)}, Rec{jpeg_recorder(15, 0)}, Rec{jpeg_recorder(16, 0)}, Rec{jpeg_recorder(19, 1)}, Rec{recorder(20, 3)}};
// Dialog7ca2e8_Open's calls (tests/fncmp/dialog7ca2e8open.py).
const Dialog7ca2e8OpenEnv kDialog7ca2e8OpenEnv = {reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)(kStubs + 0x50)), Rec{recorder(19, 1)}, Rec{recorder(20, 2)}};
// Dialog7cc944_Open's calls (tests/fncmp/dialog7cc944.py).
const Dialog7cc944Env kDialog7cc944Env = {Rec{recorder(11, 2)}, Rec{recorder(12, 1)}, Rec{recorder(13, 1)}, Rec{recorder(27, 2)}, Rec{recorder(14, 2)}, Rec{recorder(15, 1)}, Rec{jpeg_recorder(16, 7)}, Rec{recorder(17, 4)}, Rec{jpeg_recorder(18, 4)}, Rec{jpeg_recorder(19, 3)}, Rec{recorder(20, 2)}};
// Dialog7cca70_Open's calls (tests/fncmp/dialog7cca70.py).
const Dialog7cca70Env kDialog7cca70Env = {Rec{recorder(11, 3)}, Rec{recorder(13, 1)}, Rec{recorder(27, 2)}, Rec{recorder(14, 2)}, Rec{recorder(15, 1)}, Rec{jpeg_recorder(16, 7)}, Rec{jpeg_recorder(18, 5)}, Rec{jpeg_recorder(19, 3)}, Rec{recorder(20, 2)}};
// Dialog7cc188_FillList's calls (tests/fncmp/dialog7cc188list.py).
const Dialog7cc188ListEnv kDialog7cc188ListEnv = {Rec{jpeg_recorder(16, 0)}, Rec{jpeg_recorder(11, 3)}, reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)(kStubs + 0x50)), Rec{recorder(12, 0)}, Rec{recorder(14, 4)}, Rec{recorder(13, 0)}, Rec{jpeg_recorder(15, 1)}};
// Dialog7cc1f8_ShowPreview's calls (tests/fncmp/dialog7cc1f8preview.py).
const Dialog7cc1f8PreviewEnv kDialog7cc1f8PreviewEnv = {Rec{recorder(10, 1)}, Rec{stdcall_recorder(11, 1)}, Rec{jpeg_recorder(12, 3)}, Rec{recorder(13, 2)}, Rec{recorder(14, 0)}, Rec{jpeg_recorder(15, 4)}, Rec{jpeg_recorder(16, 1)}, Rec{jpeg_recorder(17, 3)}, Rec{jpeg_recorder(18, 1)}, Rec{recorder(19, 1)}, Rec{jpeg_recorder(20, 1)}, Rec{jpeg_recorder(21, 1)}};
// TqiVideo's frames' macroblocks (tests/fncmp/tqiframe.py).
const TqiFrameEnv kTqiFrameEnv = {Rec{jpeg_recorder(10, 3)}, Rec{jpeg_recorder(11, 3)}, Rec{jpeg_recorder(12, 3)}};
// DctVideo_DecodeMacroblock's calls (tests/fncmp/dctmacroblock.py).
const DctMacroblockEnv kDctMacroblockEnv = {Rec{jpeg_recorder(10, 0)}, Rec{jpeg_recorder(11, 2)}, Rec{jpeg_recorder(12, 4)}};
// The thing panels' sign slot's calls (tests/fncmp/thingsign.py).
const ThingSignEnv kThingSignEnv = {Rec{recorder(10, 0)}, Rec{jpeg_recorder(12, 3)}};
// DialogTabs_Reopen's calls (tests/fncmp/dialogtabs.py).
const DialogTabsEnv kDialogTabsEnv = {Rec{jpeg_recorder(10, 0)}, Rec{jpeg_recorder(11, 2)}, Rec{jpeg_recorder(12, 1)}, Rec{jpeg_recorder(13, 0)}, Rec{jpeg_recorder(14, 2)}, Rec{jpeg_recorder(15, 2)}, Rec{jpeg_recorder(16, 1)}, Rec{jpeg_recorder(17, 0)}, Rec{jpeg_recorder(18, 0)}, Rec{jpeg_recorder(19, 0)}, Rec{jpeg_recorder(20, 0)}};
// The tabbed dialogs' handlers' calls (tests/fncmp/dialogtabhandlers.py).
const DialogTabHandlersEnv kDialogTabHandlersEnv = {Rec{jpeg_recorder(10, 4)}, Rec{jpeg_recorder(11, 4)}, Rec{jpeg_recorder(12, 1)}, Rec{recorder(13, 0)}, Rec{recorder(14, 1)}, Rec{recorder(15, 2)}, Rec{jpeg_recorder(16, 0)}, Rec{jpeg_recorder(17, 0)}, Rec{jpeg_recorder(18, 0)}, Rec{jpeg_recorder(19, 0)}, Rec{jpeg_recorder(20, 0)}, Rec{recorder(21, 0)}};
// The thing panels' go-to's calls (tests/fncmp/thinggoto.py).
const ThingGotoEnv kThingGotoEnv = {Rec{jpeg_recorder(10, 8)}, Rec{jpeg_recorder(16, 1)}, Rec{jpeg_recorder(12, 1)}, Rec{jpeg_recorder(13, 1)}, Rec{jpeg_recorder(14, 2)}, Rec{jpeg_recorder(15, 4)}};
// DialupList_Fill's calls (tests/fncmp/dialuplist.py).
const DialupListEnv kDialupListEnv = {Rec{recorder(10, 0)}, Rec{recorder(11, 2)}, Rec{recorder(12, 0)}, Rec{recorder(13, 1)}, Rec{recorder(14, 4)}, Rec{recorder(15, 0)}, Rec{jpeg_recorder(16, 1)}, Rec{jpeg_recorder(17, 2)}};
// Panel7cc408_ShowVisitor's calls (tests/fncmp/panel7cc408info.py).
const Panel7cc408InfoEnv kPanel7cc408InfoEnv = {Rec{jpeg_recorder(18, 5)}, Rec{jpeg_recorder(19, 4)}, Rec{jpeg_recorder(20, 3)}, Rec{recorder(21, 0)}, Rec{recorder(22, 2)}, Rec{recorder(23, 2)}, Rec{recorder(24, 1)}, Rec{recorder(26, 1)}, Rec{jpeg_recorder(25, 1)}};
// Dialog7cc4b4_Update's calls (tests/fncmp/dialog7cc4b4update.py).
const Dialog7cc4b4UpdateEnv kDialog7cc4b4UpdateEnv = {reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)(kStubs + 0x50)), Rec{recorder(12, 0)}, Rec{recorder(13, 1)}, Rec{jpeg_recorder(14, 0)}, Rec{recorder(15, 0)}, Rec{recorder(16, 0)}, Rec{recorder(17, 1)}, Rec{recorder(18, 1)}, Rec{recorder(23, 2)}, Rec{jpeg_recorder(19, 4)}, Rec{jpeg_recorder(20, 1)}, Rec{jpeg_recorder(21, 3)}, Rec{jpeg_recorder(22, 1)}, Rec{recorder(24, 2)}};
// Dialog7cb2b8_Build's calls (tests/fncmp/dialog7cb2b8build.py).
const Dialog7cb2b8BuildEnv kDialog7cb2b8BuildEnv = {reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)(kStubs + 0x50)), Rec{jpeg_recorder(19, 7)}, Rec{recorder(20, 1)}, Rec{recorder(21, 1)}, Rec{recorder(22, 1)}};
// Dialog7cc188_Open's calls (tests/fncmp/dialog7cc188open.py).
const Dialog7cc188OpenEnv kDialog7cc188OpenEnv = {Rec{recorder(10, 2)}, Rec{jpeg_recorder(11, 1)}, Rec{recorder(12, 1)}, Rec{recorder(24, 1)}, Rec{recorder(13, 1)}, Rec{jpeg_recorder(14, 1)}, Rec{recorder(15, 1)}, Rec{recorder(16, 2)}, Rec{jpeg_recorder(17, 1)}, Rec{jpeg_recorder(18, 1)},
                                                  Rec{recorder(19, 2)}, Rec{recorder(20, 2)}, Rec{jpeg_recorder(21, 1)}, Rec{recorder(22, 1)}, Rec{jpeg_recorder(23, 1)}, Rec{recorder(25, 4)}, Rec{recorder(26, 1)}, Rec{recorder(27, 2)}, Rec{recorder(28, 1)}, Rec{jpeg_recorder(29, 1)}};
// Dialog7cc61c_FitText's calls (tests/fncmp/dialog7cc61ctext.py).
const Dialog7cc61cTextEnv kDialog7cc61cTextEnv = {Rec{jpeg_recorder(10, 1)}, Rec{recorder(11, 0)}, Rec{jpeg_recorder(12, 7)}, Rec{jpeg_recorder(13, 1)}, Rec{recorder(14, 1)}, Rec{recorder(15, 2)}};
// Dialog7cc530_Open's calls (tests/fncmp/dialog7cc530open.py).
const Dialog7cc530OpenEnv kDialog7cc530OpenEnv = {Rec{jpeg_recorder(10, 3)}, Rec{jpeg_recorder(11, 1)}, Rec{recorder(12, 1)}, Rec{recorder(27, 2)}, Rec{recorder(14, 2)}, Rec{jpeg_recorder(15, 1)}, Rec{recorder(16, 1)}, Rec{jpeg_recorder(17, 7)}, Rec{jpeg_recorder(18, 1)}, Rec{jpeg_recorder(19, 1)}};
// ViewButtons_Handler's calls (tests/fncmp/viewbuttons.py).
const ViewButtonsEnv kViewButtonsEnv = {Rec{recorder(10, 3)}, Rec{recorder(11, 0)}, Rec{recorder(12, 0)}, Rec{recorder(13, 0)}, Rec{recorder(14, 0)}, Rec{recorder(15, 0)}, Rec{recorder(16, 0)}, Rec{jpeg_recorder(17, 0)}, Rec{recorder(18, 0)}, Rec{jpeg_recorder(19, 1)}, Rec{recorder(20, 2)}};
// the thing panels' handlers' calls (tests/fncmp/thingpanelhandlers.py).
const ThingPanelHandlerEnv kThingPanelHandlerEnv = {Rec{recorder(10, 0)}, Rec{recorder(11, 0)}, Rec{recorder(24, 0)}, Rec{jpeg_recorder(12, 4)}, Rec{recorder(13, 2)}, Rec{recorder(14, 1)}, Rec{jpeg_recorder(15, 1)}, Rec{jpeg_recorder(16, 0)}, Rec{jpeg_recorder(17, 2)}, Rec{recorder(18, 0)}, Rec{recorder(19, 0)}, Rec{recorder(20, 1)}, Rec{recorder(21, 0)}, Rec{recorder(22, 0)}, Rec{jpeg_recorder(23, 2)}, Rec{recorder(25, 0)}, Rec{recorder(26, 1)}, Rec{jpeg_recorder(27, 0)}, Rec{recorder(28, 0)}, Rec{jpeg_recorder(29, 1)}, Rec{jpeg_recorder(30, 1)}, Rec{recorder(31, 0)}, Rec{recorder(2, 0)}, Rec{stdcall_recorder(3, 1)}, Rec{recorder(4, 0)}};
const GamePartDialogEnv kGamePartDialogEnv = {Rec{jpeg_recorder(10, 1)}, Rec{recorder(11, 2)}, Rec{jpeg_recorder(12, 2)}, Rec{jpeg_recorder(13, 0)}};
const PatrolCoverageEnv kPatrolCoverageEnv = {Rec{stdcall_recorder(10, 1)}};
const Dialog7cc61cOpenEnv kDialog7cc61cOpenEnv = {Rec{jpeg_recorder(10, 3)}, Rec{jpeg_recorder(11, 1)}, Rec{recorder(12, 1)}, Rec{recorder(13, 1)}, Rec{jpeg_recorder(14, 1)}, Rec{recorder(15, 1)}, Rec{recorder(16, 1)}, Rec{recorder(17, 2)}, Rec{recorder(18, 2)}, Rec{jpeg_recorder(19, 0)}, Rec{recorder(20, 1)}, Rec{recorder(21, 1)}, Rec{recorder(22, 4)}, Rec{jpeg_recorder(23, 1)}, Rec{jpeg_recorder(24, 3)}, Rec{recorder(25, 1)}};
const Dialog7cc61cPanelEnv kDialog7cc61cPanelEnv = {Rec{jpeg_recorder(10, 1)}, Rec{recorder(11, 2)}, Rec{jpeg_recorder(12, 0)}, Rec{jpeg_recorder(13, 0)}};
const Panel7b9940ExitEnv kPanel7b9940ExitEnv = {Rec{jpeg_recorder(6, 0)}};
const Dialog7cc184OpenEnv kDialog7cc184OpenEnv = {Rec{recorder(0, 0)}, Rec{jpeg_recorder(1, 1)}, Rec{jpeg_recorder(11, 1)}, Rec{recorder(2, 0)}, Rec{recorder(3, 0)}, Rec{jpeg_recorder(4, 2)}, Rec{jpeg_recorder(5, 0)}, Rec{jpeg_recorder(6, 1)}, Rec{recorder(10, 2)}, Rec{jpeg_recorder(17, 1)},
                                                  Rec{jpeg_recorder(12, 3)}, Rec{recorder(24, 1)}, Rec{recorder(27, 2)}, Rec{recorder(13, 1)}, Rec{jpeg_recorder(14, 1)}, Rec{recorder(15, 1)}, Rec{recorder(25, 4)}, Rec{recorder(18, 2)}, Rec{recorder(16, 2)}, Rec{jpeg_recorder(21, 1)},
                                                  Rec{recorder(28, 1)}, Rec{recorder(26, 1)}, Rec{recorder(19, 1)}, Rec{recorder(20, 2)}, Rec{recorder(22, 0)}, Rec{recorder(23, 4)}, Rec{recorder(7, 0)}, Rec{jpeg_recorder(29, 1)}};
const SignNameDialogOpenEnv kSignNameDialogOpenEnv = {Rec{recorder(10, 2)}, Rec{recorder(19, 2)}, Rec{jpeg_recorder(11, 1)}, Rec{recorder(12, 1)}, Rec{recorder(24, 1)}, Rec{recorder(13, 1)}, Rec{jpeg_recorder(14, 1)}, Rec{recorder(15, 1)}, Rec{recorder(16, 2)}, Rec{jpeg_recorder(17, 1)},
                                                      Rec{jpeg_recorder(18, 1)}, Rec{recorder(25, 4)}, Rec{recorder(27, 2)}, Rec{jpeg_recorder(21, 1)}, Rec{recorder(28, 1)}, Rec{recorder(20, 2)}, Rec{recorder(22, 2)}, Rec{recorder(23, 0)}, Rec{jpeg_recorder(29, 0)}, Rec{recorder(26, 1)}};
const SignNameDialogHandlerEnv kSignNameDialogHandlerEnv = {Rec{jpeg_recorder(20, 0)}, Rec{jpeg_recorder(21, 0)}, Rec{recorder(22, 1)}, Rec{recorder(23, 1)}, Rec{jpeg_recorder(24, 3)}, Rec{stdcall_recorder(25, 1)}, Rec{jpeg_recorder(26, 2)}, Rec{jpeg_recorder(27, 0)}, Rec{recorder(28, 1)},
                                                            Rec{recorder(29, 1)}, Rec{recorder(30, 0)}, Rec{jpeg_recorder(31, 1)}, Rec{recorder(10, 1)}, Rec{jpeg_recorder(11, 1)}, Rec{jpeg_recorder(12, 4)}, Rec{jpeg_recorder(13, 4)}, Rec{recorder(14, 1)}};
const Window7cb2a4Env kWindow7cb2a4Env = {Rec{recorder(10, 0)}, Rec{jpeg_recorder(11, 1)}, Rec{jpeg_recorder(12, 0)}, Rec{recorder(13, 0)}, Rec{recorder(14, 0)}, Rec{recorder(15, 1)}, Rec{jpeg_recorder(16, 3)}};
const Dialog7cc984Env kDialog7cc984Env = {Rec{jpeg_recorder(20, 4)}, Rec{recorder(21, 0)}, Rec{jpeg_recorder(22, 0)}, Rec{recorder(23, 1)}, Rec{recorder(24, 3)}, Rec{recorder(25, 1)}, Rec{jpeg_recorder(26, 0)}};
const ObjectWearEnv kObjectWearEnv = {Rec{recorder(10, 1)}, Rec{recorder(11, 0)}, Rec{jpeg_recorder(12, 1)}, Rec{jpeg_recorder(13, 1)}, Rec{jpeg_recorder(14, 2)}, Rec{jpeg_recorder(15, 1)}, Rec{recorder(16, 4)}, Rec{jpeg_recorder(17, 3)}, Rec{recorder(18, 1)}, Rec{recorder(19, 0)}, Rec{recorder(20, 1)}};
const CellEffectEnv kCellEffectEnv = {Rec{recorder(10, 0)}, Rec{jpeg_recorder(11, 1)}, Rec{recorder(12, 2)}, Rec{jpeg_recorder(13, 7)}, Rec{recorder(14, 1)}, Rec{recorder(15, 0)}, Rec{jpeg_recorder(16, 5)}, Rec{jpeg_recorder(17, 4)}};
const MouseHelpEnv kMouseHelpEnv = {Rec{jpeg_recorder(10, 0)}, Rec{jpeg_recorder(15, 0)}, Rec{jpeg_recorder(25, 0)}, Rec{jpeg_recorder(12, 0)}, Rec{jpeg_recorder(11, 0)}, Rec{jpeg_recorder(13, 1)}, Rec{jpeg_recorder(14, 1)}, Rec{recorder(17, 2)}, Rec{recorder(16, 2)}, Rec{recorder(18, 1)}, Rec{jpeg_recorder(24, 3)}, Rec{recorder(19, 0)}, Rec{recorder(20, 1)}, Rec{recorder(21, 2)}, Rec{recorder(23, 0)}, Rec{recorder(22, 0)},
                                    Rec{recorder(26, 0)}, Rec{recorder(27, 0)}, Rec{recorder(28, 0)}, Rec{recorder(29, 0)}, Rec{jpeg_recorder(30, 5)}, Rec{recorder(31, 1)}, Rec{recorder(8, 4)}};
const StaffThinkEnv kStaffThinkEnv = {Rec{recorder(10, 0)}, Rec{recorder(11, 2)}, Rec{recorder(12, 0)}, Rec{recorder(13, 0)}, Rec{recorder(14, 0)}, Rec{recorder(15, 0)}, Rec{recorder(16, 0)}, Rec{recorder(17, 0)}, Rec{recorder(18, 0)}, Rec{recorder(19, 0)}, Rec{recorder(20, 0)}, Rec{recorder(21, 0)}, Rec{recorder(22, 0)}, Rec{recorder(23, 0)}, Rec{recorder(24, 0)},
                                      Rec{recorder(25, 0)}, Rec{recorder(26, 1)}, Rec{recorder(27, 1)}, Rec{recorder(28, 1)}, Rec{recorder(29, 0)}, Rec{recorder(30, 0)}, Rec{recorder(31, 2)}, Rec{recorder(9, 1)}, Rec{recorder(8, 0)}, Rec{jpeg_recorder(2, 5)}, Rec{jpeg_recorder(3, 6)}, Rec{recorder(7, 0)}, Rec{recorder(6, 0)}, Rec{recorder(5, 0)}};
const ObjectRideEnv kObjectRideEnv = {Rec{recorder(10, 1)}, Rec{jpeg_recorder(11, 9)}, Rec{jpeg_recorder(12, 10)}, Rec{jpeg_recorder(13, 2)}, Rec{jpeg_recorder(14, 1)}};
const Dialog7cc1f8StatsEnv kDialog7cc1f8StatsEnv = {Rec{recorder(10, 1)}, Rec{stdcall_recorder(11, 1)}, Rec{jpeg_recorder(12, 1)}, Rec{recorder(13, 1)}};
const Dialog7cc5f4HandlerEnv kDialog7cc5f4HandlerEnv = {Rec{jpeg_recorder(10, 4)}, Rec{jpeg_recorder(11, 4)}, Rec{jpeg_recorder(12, 0)}, Rec{jpeg_recorder(13, 0)}, Rec{jpeg_recorder(14, 0)}, Rec{jpeg_recorder(15, 0)}, Rec{recorder(16, 2)}, Rec{recorder(17, 1)}, Rec{recorder(18, 1)}, Rec{jpeg_recorder(19, 4)}};
const Window7cb2ccEnv kWindow7cb2ccEnv = {Rec{recorder(10, 2)}, Rec{recorder(11, 1)}, Rec{recorder(12, 2)}, Rec{recorder(13, 2)}, Rec{recorder(14, 2)}, Rec{jpeg_recorder(15, 4)}};
const Dialog7cca70ListEnv kDialog7cca70ListEnv = {Rec{recorder(12, 0)}, Rec{recorder(13, 4)}, Rec{recorder(14, 0)}, Rec{recorder(15, 2)}, Rec{recorder(16, 0)}, Rec{recorder(17, 1)}, Rec{recorder(18, 2)}, Rec{jpeg_recorder(19, 0)}, Rec{recorder(20, 0)}, Rec{recorder(21, 0)}, Rec{jpeg_recorder(22, 4)}};
const MouseClickEnv kMouseClickEnv = {Rec{jpeg_recorder(10, 0)}, Rec{jpeg_recorder(11, 3)}, Rec{recorder(18, 2)}, Rec{recorder(19, 0)}, Rec{recorder(20, 0)}, Rec{jpeg_recorder(12, 6)}, Rec{jpeg_recorder(13, 2)}, Rec{jpeg_recorder(14, 1)}, Rec{recorder(21, 1)}, Rec{recorder(22, 1)}, Rec{jpeg_recorder(15, 0)}, Rec{jpeg_recorder(16, 1)}, Rec{jpeg_recorder(17, 1)}, Rec{recorder(23, 1)}};
const Dialog7cb2fcOpenEnv kDialog7cb2fcOpenEnv = {Rec{jpeg_recorder(10, 1)}, Rec{jpeg_recorder(11, 3)}, Rec{jpeg_recorder(12, 6)}, Rec{jpeg_recorder(13, 1)}, Rec{jpeg_recorder(14, 1)}, Rec{jpeg_recorder(15, 1)}, Rec{jpeg_recorder(16, 1)}, Rec{jpeg_recorder(17, 4)}, Rec{jpeg_recorder(18, 3)}, Rec{jpeg_recorder(19, 1)}, Rec{jpeg_recorder(20, 2)}, Rec{jpeg_recorder(21, 0)}, Rec{jpeg_recorder(22, 0)}, Rec{jpeg_recorder(23, 0)}, Rec{jpeg_recorder(24, 0)}, Rec{jpeg_recorder(25, 0)}, Rec{recorder(26, 2)}, Rec{recorder(27, 2)}, Rec{recorder(28, 0)}, Rec{recorder(29, 0)}, Rec{recorder(30, 1)}, Rec{recorder(31, 1)}, Rec{recorder(8, 2)}, Rec{recorder(6, 1)}, Rec{recorder(5, 3)}, Rec{recorder(4, 0)}, Rec{recorder(3, 0)}};
const Dialog7cc984OpenEnv kDialog7cc984OpenEnv = {Rec{recorder(10, 1)}, Rec{jpeg_recorder(11, 2)}, Rec{recorder(12, 2)}, Rec{recorder(13, 1)}, Rec{recorder(14, 2)}, Rec{recorder(15, 0)}, Rec{recorder(16, 1)}, Rec{recorder(17, 1)}, Rec{recorder(18, 2)}, Rec{jpeg_recorder(19, 1)}, Rec{recorder(20, 1)}, Rec{recorder(21, 1)}, Rec{jpeg_recorder(22, 1)}, Rec{recorder(23, 1)}, Rec{recorder(24, 1)}, Rec{recorder(25, 1)}, Rec{recorder(26, 0)}, Rec{recorder(27, 0)}, Rec{recorder(28, 2)}, Rec{recorder(29, 2)}, Rec{recorder(30, 2)}, Rec{recorder(31, 1)}, Rec{jpeg_recorder(8, 1)}, Rec{recorder(7, 1)}, Rec{recorder(6, 2)}};
const ChatNameMenuEnv kChatNameMenuEnv = {Rec{recorder(10, 1)}, Rec{recorder(11, 0)}, Rec{recorder(12, 1)}, Rec{recorder(13, 1)}, Rec{recorder(14, 1)}, Rec{recorder(15, 1)}, Rec{recorder(16, 1)}, Rec{recorder(17, 1)}, Rec{recorder(18, 1)}, Rec{recorder(19, 1)}, Rec{recorder(20, 1)}, Rec{recorder(21, 0)}, Rec{recorder(22, 1)}, Rec{recorder(23, 3)}};
const Panel7c2534HandlerEnv kPanel7c2534HandlerEnv = {Rec{recorder(10, 0)}, Rec{jpeg_recorder(11, 0)}, Rec{recorder(12, 0)}, Rec{recorder(13, 0)}, Rec{recorder(14, 1)}, Rec{jpeg_recorder(15, 1)}, Rec{jpeg_recorder(16, 1)}, Rec{jpeg_recorder(17, 0)}, Rec{jpeg_recorder(18, 1)}, Rec{jpeg_recorder(19, 8)}, Rec{jpeg_recorder(20, 0)}, Rec{recorder(21, 0)}, Rec{recorder(22, 0)}, Rec{jpeg_recorder(23, 1)}, Rec{recorder(24, 2)}, Rec{jpeg_recorder(25, 4)}};
const ObjectCtorEnv kObjectCtorEnv = {Rec{recorder(10, 2)}, Rec{recorder(11, 1)}, Rec{recorder(12, 2)}, Rec{recorder(13, 1)}, Rec{recorder(14, 2)}, Rec{recorder(15, 1)}, Rec{recorder(16, 1)}, Rec{recorder(17, 0)}, Rec{recorder(18, 1)}, Rec{recorder(19, 1)}, Rec{recorder(20, 1)}, Rec{recorder(21, 0)}, Rec{jpeg_recorder(22, 1)}, Rec{jpeg_recorder(23, 2)}, Rec{recorder(24, 1)}, Rec{jpeg_recorder(25, 3)}, Rec{recorder(26, 9)}, Rec{recorder(27, 2)}, Rec{recorder(28, 3)}, Rec{recorder(29, 3)}, Rec{recorder(30, 0)}, Rec{recorder(31, 0)}, Rec{jpeg_recorder(9, 2)}};
const Panel7ca228HandlerEnv kPanel7ca228HandlerEnv = {Rec{recorder(12, 0)}, Rec{recorder(13, 0)}, Rec{recorder(14, 0)}, Rec{recorder(15, 0)}, Rec{recorder(16, 0)}, Rec{recorder(17, 0)}, Rec{recorder(18, 0)}, Rec{recorder(19, 0)}, Rec{recorder(20, 1)}, Rec{recorder(21, 1)}, Rec{recorder(22, 2)}, Rec{recorder(23, 1)}, Rec{jpeg_recorder(24, 0)}, Rec{jpeg_recorder(25, 1)}, Rec{jpeg_recorder(26, 2)}, Rec{jpeg_recorder(27, 4)}};
const MoneyDisplayEnv kMoneyDisplayEnv = {Rec{recorder(12, 0)}, Rec{recorder(13, 0)}, Rec{recorder(14, 4)}, Rec{recorder(15, 1)}, Rec{recorder(16, 2)}, Rec{recorder(17, 2)}, Rec{recorder(18, 1)}, Rec{recorder(19, 0)}, Rec{recorder(20, 1)}, Rec{jpeg_recorder(21, 0)}, Rec{jpeg_recorder(22, 0)}, Rec{recorder(23, 1)}};
const Dialog7cc944ListEnv kDialog7cc944ListEnv = {Rec{recorder(10, 0)}, Rec{jpeg_recorder(11, 1)}, Rec{recorder(12, 4)}, Rec{recorder(13, 0)}, Rec{recorder(14, 0)}, Rec{recorder(15, 2)}, Rec{recorder(16, 0)}, Rec{recorder(17, 1)}, Rec{recorder(18, 2)}, Rec{recorder(19, 1)}, Rec{recorder(20, 2)}};
const Window705a20CharEnv kWindow705a20CharEnv = {Rec{recorder(10, 1)}, Rec{recorder(11, 3)}, Rec{jpeg_recorder(12, 4)}, Rec{recorder(13, 1)}};
const GuiFrameEnv kGuiFrameEnv = {Rec{jpeg_recorder(10, 0)}, Rec{jpeg_recorder(11, 0)}, Rec{recorder(12, 0)}, Rec{jpeg_recorder(13, 0)}, Rec{jpeg_recorder(14, 0)}, Rec{jpeg_recorder(15, 1)}, Rec{jpeg_recorder(16, 0)}, Rec{jpeg_recorder(17, 4)}, Rec{jpeg_recorder(18, 0)}, Rec{jpeg_recorder(19, 0)}, Rec{jpeg_recorder(20, 0)}, Rec{jpeg_recorder(21, 1)}, Rec{jpeg_recorder(22, 0)}, Rec{jpeg_recorder(23, 2)}, Rec{jpeg_recorder(24, 4)}, Rec{jpeg_recorder(25, 1)}, Rec{recorder(26, 0)}};
const GuiStartupEnv kGuiStartupEnv = {Rec{jpeg_recorder(6, 5)}, Rec{jpeg_recorder(7, 1)}, Rec{jpeg_recorder(8, 1)}, Rec{jpeg_recorder(9, 1)}, Rec{jpeg_recorder(10, 1)}, Rec{jpeg_recorder(11, 0)}, Rec{recorder(12, 5)}, Rec{recorder(13, 0)}, Rec{jpeg_recorder(14, 3)}, Rec{jpeg_recorder(15, 1)}, Rec{jpeg_recorder(16, 1)}, Rec{jpeg_recorder(17, 4)}, Rec{jpeg_recorder(18, 4)}, Rec{recorder(19, 1)}, Rec{jpeg_recorder(20, 0)}, Rec{jpeg_recorder(21, 0)}, Rec{jpeg_recorder(22, 1)}, Rec{jpeg_recorder(23, 1)}, Rec{jpeg_recorder(24, 1)}, Rec{jpeg_recorder(25, 0)}, Rec{jpeg_recorder(26, 0)}, Rec{jpeg_recorder(27, 0)}, Rec{recorder(28, 2)}, Rec{jpeg_recorder(29, 0)}}; const GuiTeardownEnv kGuiTeardownEnv = {Rec{recorder(6, 0)}, Rec{jpeg_recorder(7, 0)}, Rec{jpeg_recorder(8, 0)}, Rec{jpeg_recorder(9, 0)}, Rec{jpeg_recorder(10, 0)}, Rec{jpeg_recorder(11, 0)}, Rec{jpeg_recorder(12, 0)}, Rec{recorder(13, 0)}, Rec{jpeg_recorder(14, 1)}};
const WindowTickEnv kWindowTickEnv = {Rec{jpeg_recorder(10, 4)}};
const Panel7c2534MenusEnv kPanel7c2534MenusEnv = {Rec{recorder(10, 0)}, Rec{recorder(11, 2)}, Rec{recorder(12, 0)}, Rec{jpeg_recorder(13, 1)}, Rec{jpeg_recorder(14, 1)}, Rec{recorder(15, 7)}, Rec{recorder(16, 0)}, Rec{recorder(17, 0)}, Rec{recorder(18, 0)}};
const Panel7c2534OpenEnv kPanel7c2534OpenEnv = {Rec{jpeg_recorder(10, 0)}, Rec{jpeg_recorder(11, 0)}, Rec{jpeg_recorder(12, 1)}, Rec{recorder(13, 2)}, Rec{jpeg_recorder(14, 0)}, Rec{recorder(15, 1)}};
const GameWindowInputEnv kGameWindowInputEnv = {Rec{recorder(10, 0)}, Rec{recorder(11, 0)}, Rec{jpeg_recorder(12, 3)}, Rec{jpeg_recorder(13, 0)}, Rec{recorder(14, 2)}, Rec{recorder(15, 2)}, Rec{recorder(16, 3)}, Rec{jpeg_recorder(17, 1)}, Rec{recorder(18, 2)}, Rec{jpeg_recorder(19, 0)}, Rec{jpeg_recorder(20, 0)}};
const GamePauseEnv kGamePauseEnv = {Rec{jpeg_recorder(10, 1)}, Rec{recorder(11, 0)}, Rec{jpeg_recorder(12, 1)}, Rec{recorder(13, 2)}, Rec{jpeg_recorder(14, 0)}, Rec{recorder(15, 1)}, Rec{jpeg_recorder(16, 0)}, Rec{jpeg_recorder(17, 0)}, Rec{jpeg_recorder(18, 1)}, Rec{recorder(19, 0)}, Rec{jpeg_recorder(20, 0)}, Rec{jpeg_recorder(21, 0)}, Rec{jpeg_recorder(22, 0)}, Rec{jpeg_recorder(23, 0)}, Rec{jpeg_recorder(24, 0)}, Rec{jpeg_recorder(25, 0)}, Rec{jpeg_recorder(7, 1)}, Rec{jpeg_recorder(6, 1)}, Rec{recorder(26, 0)}, Rec{recorder(27, 0)}, Rec{recorder(28, 0)}, Rec{recorder(29, 1)}, Rec{recorder(30, 1)}, Rec{recorder(31, 0)}, Rec{recorder(8, 0)}};
const Window705b20ThumbEnv kWindow705b20ThumbEnv = {Rec{jpeg_recorder(10, 4)}, Rec{jpeg_recorder(11, 4)}, Rec{jpeg_recorder(12, 4)}, Rec{recorder(13, 0)}, Rec{recorder(14, 2)}, Rec{recorder(15, 0)}, Rec{recorder(16, 2)}, Rec{recorder(17, 2)}, Rec{recorder(18, 0)}, Rec{recorder(19, 0)}, Rec{recorder(25, 0)}, Rec{recorder(26, 0)}, Rec{jpeg_recorder(27, 7)}, Rec{recorder(28, 1)}, Rec{recorder(29, 0)}};
const Window7cb2b0HandlerEnv kWindow7cb2b0HandlerEnv = {Rec{jpeg_recorder(10, 0)}, Rec{recorder(11, 2)}, Rec{recorder(12, 2)}, Rec{recorder(13, 2)}, Rec{recorder(14, 1)}, Rec{jpeg_recorder(15, 0)}, Rec{jpeg_recorder(16, 0)}, Rec{jpeg_recorder(17, 0)}, Rec{jpeg_recorder(18, 3)}, Rec{recorder(19, 3)}};
const Window7cc8bcHandlerEnv kWindow7cc8bcHandlerEnv = {Rec{jpeg_recorder(10, 0)}, Rec{jpeg_recorder(11, 0)}, Rec{recorder(12, 0)}, Rec{recorder(13, 0)}, Rec{recorder(14, 0)}, Rec{recorder(15, 0)}, Rec{recorder(16, 0)}, Rec{recorder(17, 0)}, Rec{recorder(18, 3)}};
const MoneyDisplayHandlerEnv kMoneyDisplayHandlerEnv = {Rec{jpeg_recorder(10, 1)}, Rec{jpeg_recorder(11, 1)}, Rec{recorder(12, 3)}};
const Dialog7ca1e0Env kDialog7ca1e0Env = {Rec{jpeg_recorder(10, 0)}, Rec{jpeg_recorder(11, 0)}, Rec{jpeg_recorder(12, 1)}, Rec{jpeg_recorder(13, 1)}, Rec{recorder(14, 3)}};
const Window7cc8c0HandlerEnv kWindow7cc8c0HandlerEnv = {Rec{recorder(12, 2)}, Rec{jpeg_recorder(15, 2)}, Rec{jpeg_recorder(20, 4)}, Rec{jpeg_recorder(25, 4)}, Rec{recorder(13, 0)}, Rec{recorder(14, 0)}, Rec{jpeg_recorder(28, 3)}, Rec{recorder(15, 0)}, Rec{recorder(16, 0)}, Rec{jpeg_recorder(31, 0)}, Rec{recorder(17, 2)}, Rec{recorder(18, 2)}, Rec{jpeg_recorder(29, 3)}, Rec{recorder(19, 1)}, Rec{recorder(20, 0)}, Rec{recorder(21, 1)}, Rec{jpeg_recorder(27, 1)}, Rec{jpeg_recorder(30, 5)}, Rec{jpeg_recorder(13, 1)}, Rec{jpeg_recorder(12, 0)}, Rec{jpeg_recorder(19, 0)}, Rec{jpeg_recorder(14, 0)}, Rec{recorder(23, 2)}, Rec{recorder(24, 2)}, Rec{recorder(25, 1)}, Rec{recorder(26, 1)}, Rec{recorder(27, 2)}, Rec{jpeg_recorder(16, 5)}, Rec{jpeg_recorder(17, 0)}, Rec{jpeg_recorder(18, 1)}, Rec{recorder(28, 3)}};
const Window7cc8bcMenuEnv kWindow7cc8bcMenuEnv = {Rec{recorder(10, 2)}, Rec{recorder(11, 3)}, Rec{jpeg_recorder(12, 0)}, Rec{recorder(13, 1)}, Rec{jpeg_recorder(14, 2)}, Rec{recorder(15, 2)}, Rec{recorder(16, 2)}, Rec{recorder(17, 3)}};
const Dialog7cb2b8TabsEnv kDialog7cb2b8TabsEnv = {Rec{jpeg_recorder(10, 1)}, Rec{jpeg_recorder(11, 0)}, Rec{jpeg_recorder(12, 0)}, Rec{recorder(13, 0)}, Rec{jpeg_recorder(14, 0)}, Rec{recorder(15, 3)}};
const Dialog7cc4b4SparkleEnv kDialog7cc4b4SparkleEnv = {Rec{recorder(10, 0)}, Rec{jpeg_recorder(11, 1)}, Rec{recorder(12, 2)}, Rec{jpeg_recorder(13, 4)}, Rec{jpeg_recorder(14, 6)}};
const PasswordPartEnv kPasswordPartEnv = {Rec{recorder(10, 2)}};
const QuitConfirmEnv kQuitConfirmEnv = {Rec{recorder(10, 0)}, Rec{jpeg_recorder(11, 1)}, Rec{recorder(12, 1)}};
const MapSweepEnv kMapSweepEnv = {Rec{recorder(10, 0)}, Rec{recorder(11, 0)}, Rec{recorder(12, 0)}};
const Dialog7cb400CloseEnv kDialog7cb400CloseEnv = {Rec{jpeg_recorder(10, 4)}, Rec{recorder(11, 1)}, Rec{recorder(12, 1)}, Rec{jpeg_recorder(13, 0)}, Rec{recorder(14, 0)}, Rec{recorder(15, 0)}, Rec{jpeg_recorder(16, 1)}, Rec{jpeg_recorder(17, 0)},
                                                    Rec{recorder(18, 0)}, Rec{recorder(19, 0)}, Rec{recorder(20, 2)}, Rec{recorder(21, 1)}, Rec{recorder(22, 0)}, Rec{jpeg_recorder(23, 0)}, Rec{jpeg_recorder(24, 4)}, Rec{recorder(25, 3)}, Rec{recorder(26, 1)}};
const Dialog7cc188AddressEnv kDialog7cc188AddressEnv = {Rec{recorder(10, 1)}, Rec{jpeg_recorder(11, 1)}, Rec{recorder(12, 1)}, Rec{jpeg_recorder(13, 1)}, Rec{jpeg_recorder(14, 5)}, Rec{jpeg_recorder(15, 8)}, Rec{recorder(16, 3)}};
const UserSlotsEnv kUserSlotsEnv = {Rec{recorder(27, 0)}};
const BankAccountCtorEnv kBankAccountCtorEnv = {Rec{jpeg_recorder(10, 4)}};
const Dialog7ca150ListEnv kDialog7ca150ListEnv = {Rec{recorder(10, 2)}, Rec{recorder(11, 1)}, Rec{recorder(12, 2)}, Rec{recorder(13, 5)}, Rec{recorder(14, 1)}, Rec{recorder(15, 1)}, Rec{recorder(16, 0)}, Rec{recorder(17, 0)}, Rec{recorder(18, 0)}, Rec{recorder(19, 0)}, Rec{recorder(20, 3)}};
const Dialog7cb3f8SlotsEnv kDialog7cb3f8SlotsEnv = {Rec{jpeg_recorder(10, 0)}, Rec{recorder(11, 1)}, Rec{recorder(12, 1)}, Rec{jpeg_recorder(13, 1)}, Rec{recorder(14, 1)}};
const Panel7cc328HandlerEnv kPanel7cc328HandlerEnv = {Rec{recorder(10, 0)}, Rec{recorder(11, 2)}, Rec{recorder(12, 5)}, Rec{recorder(13, 2)}, Rec{recorder(14, 1)}, Rec{jpeg_recorder(15, 1)}, Rec{recorder(16, 0)}, Rec{recorder(17, 0)}, Rec{recorder(18, 0)}, Rec{jpeg_recorder(19, 2)}, Rec{jpeg_recorder(20, 2)},
                                                      Rec{recorder(21, 0)}, Rec{recorder(22, 1)}, Rec{jpeg_recorder(23, 1)}, Rec{jpeg_recorder(24, 4)}, Rec{recorder(25, 1)}};
const Dialog7ca168HandlerEnv kDialog7ca168HandlerEnv = {Rec{jpeg_recorder(10, 4)}, Rec{recorder(11, 2)}, Rec{recorder(12, 0)}, Rec{recorder(13, 0)}, Rec{recorder(14, 1)}, Rec{jpeg_recorder(15, 1)}, Rec{recorder(16, 0)}, Rec{recorder(17, 0)}, Rec{recorder(18, 5)}, Rec{jpeg_recorder(19, 4)},
                                                        Rec{jpeg_recorder(20, 1)}, Rec{jpeg_recorder(21, 3)}, Rec{jpeg_recorder(22, 1)}, Rec{jpeg_recorder(23, 0)}, Rec{jpeg_recorder(24, 2)}, Rec{jpeg_recorder(25, 1)}, Rec{recorder(26, 1)}, Rec{jpeg_recorder(27, 1)}, Rec{recorder(28, 1)}};
const Dialog7cc530HandlerEnv kDialog7cc530HandlerEnv = {Rec{jpeg_recorder(11, 4)}, Rec{jpeg_recorder(12, 0)}, Rec{jpeg_recorder(13, 4)}, Rec{recorder(14, 2)}, Rec{jpeg_recorder(15, 1)}, Rec{jpeg_recorder(16, 2)}, Rec{jpeg_recorder(17, 1)}};
const Window7059f0TextsEnv kWindow7059f0TextsEnv = {Rec{recorder(10, 2)}, Rec{recorder(11, 1)}, Rec{recorder(12, 0)}};
const SaveDialogCreateEnv kSaveDialogCreateEnv = {Rec{recorder(10, 2)}, Rec{jpeg_recorder(11, 0)}, Rec{jpeg_recorder(12, 3)}, Rec{recorder(13, 1)}, Rec{jpeg_recorder(14, 1)}, Rec{jpeg_recorder(15, 3)}, Rec{recorder(16, 1)}, Rec{recorder(17, 2)}, Rec{recorder(18, 2)}, Rec{jpeg_recorder(19, 1)}, Rec{recorder(20, 1)},
                                                  Rec{jpeg_recorder(21, 1)}, Rec{jpeg_recorder(22, 1)}, Rec{recorder(23, 1)}, Rec{recorder(24, 1)}, Rec{recorder(25, 1)}, Rec{jpeg_recorder(26, 1)}, Rec{recorder(27, 2)}, Rec{recorder(28, 1)}, Rec{recorder(29, 1)}, Rec{jpeg_recorder(30, 0)}};
const SaveDialogEnv kSaveDialogEnv = {Rec{recorder(10, 0)}, Rec{recorder(11, 1)}, Rec{recorder(12, 1)}, Rec{recorder(13, 0)}, Rec{jpeg_recorder(14, 4)}, Rec{jpeg_recorder(15, 4)}, Rec{jpeg_recorder(16, 0)}, Rec{jpeg_recorder(17, 0)}, Rec{jpeg_recorder(18, 4)}, Rec{recorder(19, 2)}, Rec{jpeg_recorder(20, 7)}, Rec{jpeg_recorder(21, 8)}, Rec{recorder(22, 4)}};
const VersionLabelEnv kVersionLabelEnv = {Rec{jpeg_recorder(10, 7)}, Rec{recorder(11, 7)}, Rec{recorder(12, 1)}, Rec{recorder(13, 1)}, Rec{recorder(14, 1)}, Rec{recorder(15, 1)}};
const RasterFormatEnv kRasterFormatEnv ={Rec{jpeg_recorder(10, 1)}, Rec{jpeg_recorder(11, 1)}, Rec{jpeg_recorder(12, 0)}, Rec{jpeg_recorder(13, 2)}, Rec{jpeg_recorder(14, 2)}};
const ImeHostAttachEnv kImeHostAttachEnv = {Rec{recorder(10, 2)}, Rec{recorder(11, 0)}, Rec{recorder(12, 0)}, Rec{recorder(13, 1)}, Rec{recorder(14, 1)}, Rec{jpeg_recorder(15, 4)}};
const TextRecordPartEnv kTextRecordPartEnv = {Rec{jpeg_recorder(10, 4)}, Rec{recorder(11, 1)}, Rec{jpeg_recorder(12, 1)}, Rec{recorder(13, 2)}, Rec{recorder(14, 1)}, Rec{recorder(15, 0)}, Rec{recorder(16, 1)}};
const MovieFrameEnv kMovieFrameEnv = {&heap_alloc, Rec{jpeg_recorder(10, 3)}};
const Panel7cc3b0HandlerEnv kPanel7cc3b0HandlerEnv = {Rec{jpeg_recorder(12, 4)}, Rec{recorder(13, 0)}, Rec{jpeg_recorder(14, 1)}, Rec{recorder(15, 2)}, Rec{recorder(16, 1)}, Rec{recorder(17, 0)}, Rec{jpeg_recorder(18, 0)}, Rec{recorder(19, 0)}, Rec{recorder(20, 0)}, Rec{jpeg_recorder(21, 2)}, Rec{recorder(22, 0)}, Rec{recorder(23, 0)},
                                                      Rec{recorder(24, 1)}, Rec{jpeg_recorder(25, 1)}, Rec{jpeg_recorder(26, 8)}, Rec{recorder(27, 0)}, Rec{jpeg_recorder(28, 1)}, Rec{recorder(29, 0)}, Rec{jpeg_recorder(30, 1)}};
const RideUpkeepEnv kRideUpkeepEnv = {Rec{recorder(10, 1)}, Rec{recorder(11, 0)}, Rec{recorder(12, 2)}, Rec{recorder(13, 2)}, Rec{recorder(14, 2)}, Rec{recorder(15, 0)}};
const ObjectUnlockThemesEnv kObjectUnlockThemesEnv = {Rec{recorder(10, 1)}, Rec{recorder(11, 1)}, Rec{recorder(12, 2)}, Rec{recorder(13, 0)}, Rec{recorder(14, 0)}, Rec{jpeg_recorder(15, 0)}, Rec{recorder(16, 1)}, Rec{jpeg_recorder(17, 0)}, Rec{recorder(18, 0)}, Rec{recorder(19, 0)}, Rec{recorder(20, 1)}, Rec{recorder(21, 1)}};
const Dialog7cb25cHandlerEnv kDialog7cb25cHandlerEnv = {Rec{jpeg_recorder(10, 4)}, Rec{jpeg_recorder(11, 1)}, Rec{recorder(12, 1)}, Rec{recorder(13, 2)}, Rec{recorder(14, 0)}, Rec{recorder(15, 2)}, Rec{recorder(16, 0)}, Rec{jpeg_recorder(17, 5)}, Rec{jpeg_recorder(18, 8)},
                                                        Rec{jpeg_recorder(19, 0)}, Rec{jpeg_recorder(20, 0)}, Rec{jpeg_recorder(21, 0)}, Rec{jpeg_recorder(22, 4)}};
const Panel7cc218HandlerEnv kPanel7cc218HandlerEnv = {Rec{recorder(10, 0)}, Rec{recorder(11, 0)}, Rec{jpeg_recorder(12, 4)}, Rec{recorder(13, 2)}, Rec{recorder(14, 1)}, Rec{jpeg_recorder(15, 1)}, Rec{jpeg_recorder(16, 0)}, Rec{jpeg_recorder(17, 2)}, Rec{recorder(18, 0)}, Rec{recorder(19, 0)},
                                                      Rec{recorder(20, 1)}, Rec{recorder(21, 0)}, Rec{recorder(22, 0)}, Rec{recorder(23, 0)}, Rec{recorder(24, 1)}, Rec{recorder(25, 1)}, Rec{recorder(26, 0)}, Rec{recorder(27, 1)}, Rec{recorder(28, 1)}};
const Panel7b9940OpenEnv kPanel7b9940OpenEnv = {Rec{jpeg_recorder(10, 0)}, Rec{jpeg_recorder(11, 0)}, Rec{jpeg_recorder(12, 1)}, Rec{recorder(13, 0)}, Rec{stdcall_recorder(14, 1)}, Rec{recorder(15, 2)}, Rec{recorder(16, 7)}, Rec{recorder(17, 1)}, Rec{recorder(18, 1)}, Rec{recorder(19, 2)}, Rec{recorder(20, 1)}, Rec{recorder(21, 2)},
                                                Rec{jpeg_recorder(22, 0)}, Rec{jpeg_recorder(23, 0)}, Rec{jpeg_recorder(24, 0)}, Rec{jpeg_recorder(25, 0)}, Rec{jpeg_recorder(26, 0)}, Rec{jpeg_recorder(27, 0)}, Rec{jpeg_recorder(28, 0)}, Rec{jpeg_recorder(29, 0)}, Rec{jpeg_recorder(30, 0)}};
const Panel7cc218RefreshEnv kPanel7cc218RefreshEnv = {Rec{jpeg_recorder(10, 1)}, Rec{recorder(11, 0)}, Rec{recorder(12, 0)}};
// Online_Instance's constructor (tests/fncmp/onlineinstance.py).
const OnlineInstanceEnv kOnlineInstanceEnv = {Rec{recorder(10, 0)}};
// Dialog7cdac4_Handler's calls (tests/fncmp/dialog7cdac4handler.py).
const Dialog7cdac4HandlerEnv kDialog7cdac4HandlerEnv = {Rec{jpeg_recorder(10, 4)}, Rec{jpeg_recorder(11, 4)}, Rec{jpeg_recorder(12, 0)}, Rec{jpeg_recorder(13, 0)}, Rec{jpeg_recorder(14, 0)}, Rec{jpeg_recorder(15, 4)}, Rec{recorder(16, 0)}, Rec{jpeg_recorder(17, 1)},
                                                        Rec{jpeg_recorder(18, 0)}, Rec{recorder(19, 7)}, Rec{recorder(20, 0)}, Rec{recorder(21, 1)}};
// Dialog7ca098_RunAction's and _Handler's calls (tests/fncmp/dialog7ca098.py).
const Dialog7ca098Env kDialog7ca098Env = {Rec{jpeg_recorder(10, 1)}, Rec{jpeg_recorder(11, 1)}, Rec{jpeg_recorder(12, 2)}, Rec{jpeg_recorder(13, 1)}, Rec{jpeg_recorder(14, 0)}, Rec{jpeg_recorder(15, 0)}, Rec{jpeg_recorder(16, 0)}, Rec{jpeg_recorder(17, 0)}, Rec{jpeg_recorder(18, 0)}, Rec{jpeg_recorder(19, 0)},
                                          Rec{recorder(20, 1)}, Rec{recorder(21, 0)}, Rec{recorder(22, 0)}, Rec{recorder(23, 1)}, Rec{jpeg_recorder(24, 2)}, Rec{recorder(25, 1)}, Rec{jpeg_recorder(26, 1)}, Rec{recorder(27, 1)}, Rec{recorder(28, 2)}, Rec{recorder(29, 0)}, Rec{recorder(30, 1)},
                                          Rec{jpeg_recorder(31, 4)}, Rec{jpeg_recorder(7, 4)}, Rec{jpeg_recorder(8, 1)}, Rec{recorder(9, 3)}};
// The guard's escort (tests/fncmp/guardescort.py).
const GuardEscortEnv kGuardEscortEnv = {Rec{recorder(10, 0)}, Rec{recorder(11, 0)}, Rec{recorder(12, 0)}, Rec{recorder(13, 1)}, Rec{recorder(14, 0)}, Rec{recorder(15, 1)}, Rec{recorder(16, 2)}, Rec{recorder(17, 0)}, Rec{recorder(18, 2)}, Rec{recorder(19, 2)}, Rec{recorder(20, 1)}, Rec{recorder(21, 1)}};
// MsgTag_Handler's calls (tests/fncmp/msgtaghandler.py).
const MsgTagHandlerEnv kMsgTagHandlerEnv = {Rec{jpeg_recorder(10, 3)}, Rec{recorder(11, 1)}, Rec{jpeg_recorder(12, 1)}, Rec{jpeg_recorder(13, 1)}, Rec{recorder(14, 1)}, Rec{recorder(15, 3)}, Rec{recorder(16, 4)}, Rec{recorder(17, 2)}, Rec{recorder(18, 2)}, Rec{recorder(19, 0)}, Rec{jpeg_recorder(20, 1)},
                                            Rec{jpeg_recorder(21, 4)}, Rec{jpeg_recorder(22, 4)}, Rec{recorder(23, 2)}, Rec{recorder(24, 0)}, Rec{jpeg_recorder(25, 2)}, Rec{jpeg_recorder(26, 4)}, Rec{jpeg_recorder(27, 2)}, Rec{jpeg_recorder(28, 1)}, Rec{jpeg_recorder(29, 1)}, Rec{recorder(30, 3)}};
// ThingBrowser_Handler's calls (tests/fncmp/thingbrowserhandler.py): the step recorded, then [0x7cc308] from the bench's dword at RAM + 0x800 (the bench's hook does the same).
void __cdecl browser_step() {
    const u32 next = *reinterpret_cast<u32 *>((uintptr_t)(0x30000000 + 0x800));   // (-1: left as it is)
    if (next != 0xffffffff) *reinterpret_cast<u32 *>((uintptr_t)0x007cc308) = next;
    reinterpret_cast<void(__cdecl *)()>(jpeg_recorder(12, 0))();
}
const ThingBrowserHandlerEnv kThingBrowserHandlerEnv = {Rec{jpeg_recorder(10, 0)}, Rec{jpeg_recorder(11, 0)}, &browser_step, Rec{jpeg_recorder(13, 0)}, Rec{recorder(14, 1)}, Rec{recorder(15, 0)}, Rec{recorder(16, 3)}, Rec{jpeg_recorder(17, 1)}, Rec{recorder(18, 3)}};
// ResearchPanel_Handler's calls (tests/fncmp/researchpanel.py).
const ResearchPanelEnv kResearchPanelEnv = {Rec{jpeg_recorder(10, 4)}, Rec{jpeg_recorder(11, 1)}, Rec{jpeg_recorder(12, 1)}, Rec{recorder(13, 2)}, Rec{recorder(14, 1)}, Rec{jpeg_recorder(15, 1)}, Rec{jpeg_recorder(16, 3)}, Rec{jpeg_recorder(17, 1)}, Rec{jpeg_recorder(18, 1)}, Rec{recorder(19, 0)},
                                            Rec{jpeg_recorder(20, 0)}, Rec{recorder(21, 0)}, Rec{recorder(22, 1)}, Rec{recorder(23, 2)}, Rec{recorder(24, 1)}, Rec{recorder(25, 1)}, Rec{jpeg_recorder(26, 3)}, Rec{jpeg_recorder(27, 1)}, Rec{jpeg_recorder(28, 1)}, Rec{jpeg_recorder(29, 4)}};
// Dialog7cb398_Handler's calls (tests/fncmp/dialog7cb398handler.py).
const Dialog7cb398Env kDialog7cb398Env = {Rec{jpeg_recorder(10, 4)}, Rec{recorder(11, 1)}, Rec{recorder(12, 0)}, Rec{jpeg_recorder(13, 0)}, Rec{recorder(14, 0)}, Rec{recorder(15, 1)}, Rec{recorder(16, 1)}, Rec{jpeg_recorder(17, 0)}, Rec{jpeg_recorder(18, 1)}, Rec{jpeg_recorder(19, 3)},
                                          Rec{jpeg_recorder(20, 1)}, Rec{jpeg_recorder(21, 2)}, Rec{jpeg_recorder(22, 2)}, Rec{jpeg_recorder(23, 1)}, Rec{recorder(24, 0)}, Rec{recorder(25, 1)}, Rec{jpeg_recorder(26, 4)}};
// Dialog7ca150_PageHandler's calls (tests/fncmp/dialog7ca150page.py).
const Dialog7ca150PageEnv kDialog7ca150PageEnv = {Rec{jpeg_recorder(10, 4)}, Rec{jpeg_recorder(11, 0)}, Rec{jpeg_recorder(12, 2)}, Rec{jpeg_recorder(13, 1)}, Rec{jpeg_recorder(14, 1)}, Rec{jpeg_recorder(15, 4)}};
// Dialog7ca308_Handler's calls (tests/fncmp/dialog7ca308handler.py).
const Dialog7ca308HandlerEnv kDialog7ca308HandlerEnv = {Rec{jpeg_recorder(10, 4)}, Rec{recorder(11, 1)}, Rec{recorder(12, 0)}, Rec{recorder(13, 0)}, Rec{jpeg_recorder(14, 5)}, Rec{jpeg_recorder(15, 1)}, Rec{jpeg_recorder(16, 1)}, Rec{recorder(17, 2)}, Rec{recorder(18, 1)}, Rec{jpeg_recorder(19, 2)},
                                                        Rec{recorder(20, 2)}, Rec{recorder(21, 1)}, Rec{recorder(22, 0)}, Rec{jpeg_recorder(23, 1)}, Rec{recorder(24, 1)}, Rec{jpeg_recorder(25, 1)}, Rec{jpeg_recorder(26, 4)}};
// Panel7cc218_FillLevels' calls (tests/fncmp/panel7cc218levels.py).
const Panel7cc218LevelsEnv kPanel7cc218LevelsEnv = {Rec{recorder(10, 0)}, Rec{recorder(11, 0)}, Rec{recorder(12, 0)}, Rec{recorder(13, 0)}, Rec{jpeg_recorder(14, 1)}, Rec{recorder(15, 2)}, Rec{recorder(16, 3)}, Rec{recorder(17, 4)}, Rec{recorder(18, 0)}, Rec{recorder(19, 0)},
                                                    Rec{jpeg_recorder(20, 5)}, Rec{jpeg_recorder(21, 1)}, Rec{jpeg_recorder(22, 2)}, Rec{recorder(23, 0)}};
// Panel7cc218_RefreshFigures' calls (tests/fncmp/panel7cc218figures.py).
const Panel7cc218FiguresEnv kPanel7cc218FiguresEnv = {Rec{recorder(10, 0)}, Rec{recorder(11, 1)}, Rec{recorder(12, 0)}, Rec{jpeg_recorder(13, 5)}, Rec{recorder(14, 1)}, Rec{recorder(15, 3)}, Rec{recorder(16, 3)}, Rec{recorder(18, 0)}, Rec{jpeg_recorder(19, 2)}, Rec{recorder(20, 0)}, Rec{recorder(21, 0)}, Rec{jpeg_recorder(22, 4)}};
// An idle handyman's and mechanic's calls (tests/fncmp/staffwork.py).
const StaffWorkEnv kStaffWorkEnv = {Rec{recorder(10, 0)}, Rec{recorder(11, 0)}, Rec{recorder(12, 0)}, Rec{recorder(13, 4)}, Rec{recorder(14, 1)}, Rec{recorder(15, 2)}, Rec{recorder(16, 1)}, Rec{recorder(17, 1)}, Rec{recorder(18, 2)}, Rec{recorder(19, 1)}, Rec{recorder(20, 1)}, Rec{recorder(21, 1)}, Rec{recorder(22, 1)}, Rec{stdcall_recorder(23, 3)}, Rec{jpeg_recorder(24, 2)}};
// Dialog_KeyHandler's calls (tests/fncmp/dialogkeys.py).
const DialogKeyEnv kDialogKeyEnv = {Rec{jpeg_recorder(10, 4)}, Rec{recorder(11, 0)}, Rec{recorder(12, 0)}, Rec{recorder(13, 2)}, Rec{recorder(14, 2)}, Rec{jpeg_recorder(15, 1)}, Rec{recorder(16, 3)}};
// Dialog7cdac4_Open's calls (tests/fncmp/dialog7cdac4.py).
const Dialog7cdac4Env kDialog7cdac4Env = {Rec{jpeg_recorder(10, 0)}, Rec{jpeg_recorder(11, 3)}, Rec{recorder(12, 1)}, Rec{recorder(13, 1)}, Rec{recorder(14, 1)}, Rec{jpeg_recorder(15, 1)}, Rec{recorder(16, 1)}, Rec{recorder(17, 2)}, Rec{jpeg_recorder(18, 1)}, Rec{recorder(19, 1)},
                                          Rec{recorder(20, 2)}, Rec{recorder(21, 2)}, Rec{recorder(22, 2)}, Rec{jpeg_recorder(23, 1)}, Rec{jpeg_recorder(24, 3)}, Rec{recorder(25, 2)}};
// Obj7b51f0_ShowNow's calls (tests/fncmp/obj7b51f0now.py).
const Obj7b51f0NowEnv kObj7b51f0NowEnv = {Rec{jpeg_recorder(10, 0)}, Rec{jpeg_recorder(11, 0)}, Rec{jpeg_recorder(12, 0)}, Rec{recorder(13, 9)}, Rec{jpeg_recorder(14, 3)}};
// Objects_MeanQueuePosition's Object_QueuePositionOfNone (tests/fncmp/objectsmeanqueue.py).
const ObjectsMeanQueueEnv kObjectsMeanQueueEnv = {Rec{recorder(10, 0)}};
// Dialog7cc4b4_SetFlag's Window_SetShown (tests/fncmp/dialog7cc4c4thing.py).
const Dialog7cc4b4FlagEnv kDialog7cc4b4FlagEnv = {Rec{recorder(11, 1)}};
// Dialog7ca1c8_ResetMouseMode's calls (tests/fncmp/dialog7ca1c8reset.py).
const Dialog7ca1c8ResetEnv kDialog7ca1c8ResetEnv = {Rec{jpeg_recorder(10, 0)}, Rec{jpeg_recorder(11, 1)}, Rec{recorder(12, 0)}, Rec{stdcall_recorder(13, 1)}, Rec{jpeg_recorder(14, 2)}};
// Guard_SetState's calls (tests/fncmp/guardstate.py).
const GuardStateEnv kGuardStateEnv = {Rec{recorder(10, 1)}, Rec{recorder(11, 1)}};
// Panel7c2534_AddRow's calls (tests/fncmp/panel7c2534.py).
const Panel7c2534Env kPanel7c2534Env = {Rec{jpeg_recorder(10, 7)}, Rec{recorder(11, 2)}, Rec{recorder(12, 7)}, Rec{recorder(13, 1)}, Rec{jpeg_recorder(14, 1)}, Rec{recorder(15, 2)}, Rec{jpeg_recorder(16, 1)},
                                        Rec{recorder(17, 1)}, Rec{recorder(18, 1)}, Rec{jpeg_recorder(19, 1)}, Rec{recorder(20, 1)}, Rec{recorder(21, 2)}, Rec{recorder(23, 4)}};
// Panel7cc218_EnterTileMode's calls (tests/fncmp/panel7cc218tile.py).
const Panel7cc218TileEnv kPanel7cc218TileEnv = {Rec{jpeg_recorder(10, 1)}, Rec{jpeg_recorder(11, 1)}, Rec{recorder(20, 1)}, Rec{jpeg_recorder(12, 1)}, Rec{jpeg_recorder(13, 1)},
                                                Rec{recorder(14, 1)}, Rec{jpeg_recorder(15, 1)}, Rec{recorder(16, 0)}, Rec{stdcall_recorder(17, 1)}, Rec{jpeg_recorder(18, 4)},
                                                Rec{recorder(21, 0)}, Rec{jpeg_recorder(22, 1)}, Rec{jpeg_recorder(23, 0)}, Rec{jpeg_recorder(24, 2)}, Rec{recorder(25, 2)}, Rec{jpeg_recorder(26, 2)}, Rec{jpeg_recorder(27, 2)}};
// TextPanel7cc18c_Show's calls (tests/fncmp/textpanel7cc18c.py).
const TextPanel7cc18cEnv kTextPanel7cc18cEnv = {Rec{recorder(10, 2)}, Rec{recorder(11, 0)}, Rec{jpeg_recorder(12, 9)}, Rec{recorder(13, 4)}, Rec{recorder(14, 0)}};
// Dialog7ca098_Create's calls (tests/fncmp/dialog7ca098create.py).
const Dialog7ca098CreateEnv kDialog7ca098CreateEnv = {Rec{recorder(10, 2)}, Rec{recorder(11, 0)}, Rec{recorder(12, 2)}, Rec{recorder(13, 1)}, Rec{jpeg_recorder(14, 1)}, Rec{recorder(15, 1)},
                                                      Rec{recorder(16, 1)}, Rec{jpeg_recorder(17, 1)}, Rec{recorder(18, 1)}, Rec{recorder(19, 2)}, Rec{recorder(20, 2)}};
// Dialog74fb20's calls (tests/fncmp/dialog74fb20.py).
const Dialog74fb20Env kDialog74fb20Env = {Rec{jpeg_recorder(10, 0)}, Rec{jpeg_recorder(11, 0)}, Rec{recorder(12, 2)}, Rec{recorder(13, 2)}, Rec{recorder(14, 2)}, Rec{recorder(15, 2)},
                                          Rec{recorder(16, 1)}, Rec{jpeg_recorder(17, 0)}, Rec{jpeg_recorder(18, 4)}, Rec{jpeg_recorder(19, 0)}, Rec{jpeg_recorder(20, 0)}, Rec{recorder(21, 3)},
                                          Rec{jpeg_recorder(22, 0)}, Rec{recorder(23, 1)}, Rec{jpeg_recorder(24, 0)}, Rec{jpeg_recorder(25, 0)}, Rec{recorder(26, 7)}, Rec{recorder(27, 1)}, Rec{recorder(28, 0)}, Rec{jpeg_recorder(29, 1)}, Rec{jpeg_recorder(30, 8)}, Rec{recorder(31, 0)}};
// Dialog7ca1c8_Handler's and _Hide's calls (tests/fncmp/dialog7ca1c8handler.py).
const Dialog7ca1c8HandlerEnv kDialog7ca1c8HandlerEnv = {Rec{jpeg_recorder(10, 0)}, Rec{jpeg_recorder(11, 0)}, Rec{jpeg_recorder(12, 0)}, Rec{jpeg_recorder(13, 1)}, Rec{jpeg_recorder(14, 2)}, Rec{jpeg_recorder(15, 1)}, Rec{jpeg_recorder(16, 1)}, Rec{recorder(17, 2)}, Rec{recorder(18, 2)}, Rec{jpeg_recorder(19, 0)},
                                                        Rec{jpeg_recorder(20, 2)}, Rec{jpeg_recorder(21, 0)}, Rec{jpeg_recorder(22, 1)}, Rec{recorder(23, 3)}, Rec{jpeg_recorder(24, 1)}};
// Dialog7ca1c8_OpenFor's calls (tests/fncmp/dialog7ca1c8open.py).
const Dialog7ca1c8OpenEnv kDialog7ca1c8OpenEnv = {Rec{jpeg_recorder(10, 1)}, Rec{jpeg_recorder(11, 1)}, Rec{jpeg_recorder(12, 1)}, Rec{recorder(13, 1)}, Rec{jpeg_recorder(14, 1)}, Rec{jpeg_recorder(15, 0)},
                                                  Rec{jpeg_recorder(16, 2)}, Rec{jpeg_recorder(17, 0)}, Rec{jpeg_recorder(18, 1)}, Rec{jpeg_recorder(19, 1)}, Rec{jpeg_recorder(20, 1)},
                                                  Rec{jpeg_recorder(21, 1)}, Rec{jpeg_recorder(22, 1)}, Rec{jpeg_recorder(23, 0)}};
// Panel7ca178_ShowStats's status, can-open, refund and state (tests/fncmp/panel7ca178stats.py).
const Panel7ca178StatsEnv kPanel7ca178StatsEnv = {Rec{jpeg_recorder(23, 2)}, Rec{recorder(24, 0)}, Rec{recorder(25, 0)}, Rec{recorder(26, 2)}};
// Dialog7ca308_FillList's list calls, name and wage (tests/fncmp/dialog7ca308.py).
const Dialog7cb400Env kDialog7cb400Env = {Rec{recorder(10, 2)}, Rec{jpeg_recorder(11, 3)}, Rec{recorder(12, 2)}, Rec{recorder(13, 1)}, Rec{recorder(14, 1)}, Rec{recorder(15, 1)}, Rec{recorder(16, 1)}, Rec{recorder(17, 1)}, Rec{recorder(18, 4)}, Rec{recorder(19, 2)}, Rec{recorder(20, 2)}, Rec{recorder(21, 1)}, Rec{jpeg_recorder(22, 1)}, Rec{recorder(23, 1)}, Rec{jpeg_recorder(24, 1)}, Rec{recorder(25, 1)}, Rec{recorder(26, 0)}, Rec{recorder(27, 1)}, Rec{jpeg_recorder(28, 1)}};
const OverlayLoadEnv kOverlayLoadEnv = {Rec{jpeg_recorder(17, 5)}, Rec{jpeg_recorder(18, 3)}, Rec{recorder(19, 1)}, Rec{recorder(20, 1)}, Rec{recorder(21, 1)}, Rec{jpeg_recorder(22, 2)}};
const Panel7cc408Env kPanel7cc408Env = {Rec{recorder(10, 1)}, Rec{recorder(11, 1)}, Rec{recorder(12, 1)}, Rec{recorder(13, 2)}, Rec{recorder(14, 2)}, Rec{recorder(15, 1)}, Rec{recorder(16, 4)}, Rec{recorder(17, 1)}};
const Dialog7ca308Env kDialog7ca308Env = {Rec{recorder(10, 0)}, Rec{recorder(11, 4)}, Rec{recorder(12, 0)}, Rec{recorder(13, 2)}, Rec{recorder(14, 0)}, Rec{recorder(15, 1)}, Rec{recorder(16, 1)}, Rec{jpeg_recorder(17, 2)}, Rec{jpeg_recorder(18, 5)}, Rec{recorder(19, 1)}, Rec{jpeg_recorder(21, 3)}, Rec{jpeg_recorder(22, 1)}, Rec{jpeg_recorder(23, 1)}};
// Panel7cc218_ShowRide's range, shown and settings (tests/fncmp/panel7cc218show.py).
const Panel7cc218ShowEnv kPanel7cc218ShowEnv = {Rec{recorder(21, 2)}, Rec{recorder(22, 1)}, Rec{recorder(23, 1)}};
// Dialog7cc1f8_FillList's list calls, full name and text (tests/fncmp/dialog7cc1f8.py).
const Dialog7cc1f8Env kDialog7cc1f8Env = {Rec{recorder(10, 0)}, Rec{recorder(11, 4)}, Rec{recorder(12, 0)}, Rec{recorder(13, 3)}, Rec{jpeg_recorder(14, 1)}};
// Dialog7ca150_Handler's tabs, text, label and thing panel (tests/fncmp/dialog7ca150handler.py; dialog7ca150.py calls the tabs directly: their hooks are unaffected).
const Dialog7ca150Env kDialog7ca150Env = {{Rec{jpeg_recorder(24, 1)}, Rec{jpeg_recorder(25, 1)}, Rec{jpeg_recorder(26, 1)}, Rec{jpeg_recorder(27, 1)}}, Rec{jpeg_recorder(28, 1)},
                                          Rec{jpeg_recorder(29, 3)}, Rec{jpeg_recorder(30, 1)}};
// (the other graph's 0x4a4750 the same recorder, the handler's 0x49ab80 the cdecl recorder 23 of 1: tests/fncmp/statshandler.py.)
const StatsGraphEnv kStatsGraphEnv = {Rec{jpeg_recorder(22, 1)}, Rec{jpeg_recorder(22, 1)}, Rec{jpeg_recorder(23, 1)}, Rec{recorder(24, 5)}};

const ObjectEnv kObjectEnv = {reinterpret_cast<void(__fastcall *)(u8 *, void *, u8 *)>((uintptr_t)recorder(21, 1)),
                              reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)recorder(22, 0)),
                              reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)recorder(23, 0)),
                              reinterpret_cast<void(__cdecl *)(u32, s32, u32, u32, u32, u32)>((uintptr_t)jpeg_recorder(26, 6))};
// Object_Delete's call into agent B's RSSE_KillScript (0x559060): cdecl recorder 20 of 2, as tests/fncmp/objectdelete.py hooks it.
const ObjectDeleteEnv kObjectDeleteEnv = {reinterpret_cast<void(__cdecl *)(s32, u32)>((uintptr_t)jpeg_recorder(20, 2))};
// Object_PathChanged's call into agent A's unported 0x501390: thiscall recorder 26 of none, as tests/fncmp/objectqueue.py hooks it.
const ObjectPathEnv kObjectPathEnv = {reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)recorder(26, 0))};
// Object_MechanicDone's script and effect calls: cdecl recorders 27 of 1, 28 of 7 and 29 of 1, as tests/fncmp/objectservice.py hooks them.
const ObjectServiceEnv kObjectServiceEnv = {Rec{jpeg_recorder(27, 1)}, Rec{jpeg_recorder(28, 7)}, Rec{jpeg_recorder(29, 1)}};
// BankAccount_HandleMessage's call into agent A's unported 0x5168f0 (the game lost): the thiscall recorder 24 of 0, as tests/fncmp/money.py
// hooks it.
const BankAccountEnv kBankAccountEnv = {reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)recorder(24, 0))};

// Optional settings in the log region: at + 0xa00 a mask per slot (k & 63): bits 0-15 the arguments that are C strings, logged and hashed as a hash of the string
// (so that a path in a stack buffer, whose address differs between the sides, compares by its text), bits 16-31 out arguments, logged as 0 (a stack address;
// what is written through them is a fill's, and shows in what the code does with it); at + 0xb00 and + 0xb80 two fills and from + 0x1000 six more (k | 0x80000000, the argument, the
// length; the bytes from + 0x10, at most 0x70; length bit 31: argument length & 0xff's value, fncmp.fill_arg()) written through that argument before the
// recorder returns (an out structure, FindFirstFile's); at + 0xc00 a mask per slot of BYTE arguments, logged as their low byte (fncmp.LOG_BYTES); at + 0xd00
// two dwords per slot, an argument + 1 (bits 0-4) and a dword mask: that argument (unless null) logged as a hash of the dwords it points to that the mask
// selects, a structure only partly set (fncmp.blob()); with bit 31 of the first dword the mask selects bytes (0-31) instead, and bits 8-15 (an offset + 1)
// name a pointer in the structure to a second one whose bytes base to base + 14 bits 16-30 select (unless that pointer is null; base 4 * bits 5-7;
// fncmp.blob_bytes()).
u32 string_hash(const u8 *s) {
    if (!s) return 0;
    u32 h = 0x811c9dc5u;
    for (u32 i = 0; i < 0x400 && s[i]; ++i) h = (h ^ s[i]) * 0x01000193u;
    return h;
}

u32 blob_hash(const u8 *p, u32 mask) {
    u32 h = 0x811c9dc5u;
    for (u32 i = 0; i < 32; ++i)
        if (mask >> i & 1)
            for (u32 b = 0; b < 4; ++b) h = (h ^ p[4 * i + b]) * 0x01000193u;
    return h;
}

u32 bytes_hash(u32 h, const u8 *p, u32 mask) {
    for (u32 i = 0; i < 32; ++i)
        if (mask >> i & 1) h = (h ^ p[i]) * 0x01000193u;
    return h;
}

// wide: the slot's entry at + 0xfc0 (two of (k | 0x80000000, start, mask), fncmp.LOG_BLOB_WIDE), or null: the second structure's bytes start to start + 31
// that its mask selects, hashed after the first fifteen (an inner mask past bit 14); start bit 31: the wide text its dword at start & 0xff points to.
u32 blob_bytes_hash(const u8 *p, u32 how, u32 mask, const u32 *wide) {
    u32 h = bytes_hash(0x811c9dc5u, p, mask);
    if (const u32 at = how >> 8 & 0xff) {
        const u8 *inner;
        std::memcpy(&inner, p + at - 1, 4);
        if (inner) {
            h = bytes_hash(h, inner + 4 * (how >> 5 & 7), how >> 16 & 0x7fff);
            if (wide && wide[1] & 0x80000000u) {  // (fncmp.blob_inner_text(): the wide text the second structure's dword at that offset points to)
                const u16 *text;
                std::memcpy(&text, inner + (wide[1] & 0xff), 4);
                h = wide_hash(h, text);
            } else if (wide) {
                h = bytes_hash(h, inner + wide[1], wide[2]);
            }
        }
    }
    return h;
}

u32 __cdecl record_call(u32 k, u32 n, u32 self, const u32 *stack) {
    u32 *log = reinterpret_cast<u32 *>((uintptr_t)kLog);
    u32 args[16];
    const u32 mask = log[0x280 + (k & 63)];
    u32 earlier = 0;  // (this slot's calls in the log so far: fncmp.fail_after(), a fill's records)
    for (u32 i = 0; i < log[0] && i < 40; ++i) earlier += log[1 + 12 * i] == k;
    // Answers by name at + 0x1400 (fncmp.named()): (k | 0x80000000, the name argument | (an offset + 1) << 8, the out argument + 1 | a length (0-8)
    // << 8, the count (up to 30), a miss flag, the miss value), then from + 0x1420 four dwords an entry (a string hash, the answer, two dwords). A
    // call whose name (the C string the argument points to or, with an offset, the one the pointer at that offset in the argument's structure points
    // to) hashes as an entry's answers the entry's answer, the entry's first length bytes written through the out argument (unless null) before the
    // fills; another name answers the miss value with the miss flag set, else as without the table.
    const u32 *const named = log + 0x500;
    bool by_name = false;
    u32 named_answer = 0;
    if (named[0] == ((k & 63) | 0x80000000u) && (named[1] & 0xffu) < n) {
        u32 p = stack[1 + (named[1] & 0xffu)];
        if (const u32 through = named[1] >> 8 & 0xffu)
            if (p) std::memcpy(&p, reinterpret_cast<const u8 *>((uintptr_t)p) + through - 1, 4);
        const u32 name = string_hash(reinterpret_cast<const u8 *>((uintptr_t)p));
        for (u32 e = 0; e < named[3] && e < 30; ++e) {
            const u32 *const entry = log + 0x508 + 4 * e;
            if (entry[0] != name) continue;
            by_name = true;
            named_answer = entry[1];
            const u32 out = named[2] & 0xffu, length = named[2] >> 8 & 0xffu;
            if (out && out <= n && stack[out] && length <= 8) std::memcpy(reinterpret_cast<void *>((uintptr_t)stack[out]), entry + 2, length);
            break;
        }
        if (!by_name && named[4]) {
            by_name = true;
            named_answer = named[5];
        }
    }
    for (u32 i = 0; i < n; ++i) {  // (stack[0] is the thunk's own return address)
        args[i] = stack[1 + i];
        // Eight fills: two at + 0xb00, six more from + 0x1000 (0x80 each; a log region that sets them is that much longer). Dword 3: bits 0-7 an offset
        // into the out structure, bits 8-15 records: with 2 or more the length is one record's, and call j (earlier) writes record j % records;
        // bits 16-23 an offset + 1 in the argument's structure of a pointer to the out structure (unless that pointer is null): a buffer that a
        // structure argument points to.
        for (u32 f = 0; f < 8; ++f) {
            const u32 *fill = log + (f < 2 ? 0x2c0 + 0x20 * f : 0x400 + 0x20 * (f - 2));
            if (fill[0] != ((k & 63) | 0x80000000u) || fill[1] != i || !args[i]) continue;
            u32 base = args[i];
            if (const u32 through = fill[3] >> 16 & 0xff) {
                std::memcpy(&base, reinterpret_cast<const u8 *>((uintptr_t)args[i]) + through - 1, 4);
                if (!base) continue;
            }
            u8 *const to = reinterpret_cast<u8 *>((uintptr_t)base) + (fill[3] & 0xff) + (fill[3] >> 24 << 8);
            const u32 records = fill[3] >> 8 & 0xff;
            if (fill[2] & 0x80000000u) {  // (fncmp.fill_arg(): an argument's value, as passed)
                const u32 value = (fill[2] & 0xff) < n ? stack[1 + (fill[2] & 0xff)] : 0;
                std::memcpy(to, &value, 4);
            } else {
                const u32 length = fill[2] < 0x70 ? fill[2] : 0x70;
                const u32 skip = records > 1 ? earlier % records * length : 0;
                if (skip + length <= 0x70) std::memcpy(to, reinterpret_cast<const u8 *>(fill + 4) + skip, length);
            }
        }
        const u32 *blob = log + 0x340 + 2 * (k & 63);  // (LOG_BLOBS)
        if ((blob[0] & 0x1f) == i + 1 && args[i]) {
            const u8 *const p = reinterpret_cast<const u8 *>((uintptr_t)args[i]);
            u32 blob_mask = blob[1], how = blob[0];
            bool matched = false;  // (fncmp.blob(only_when=True), bit 30 of a dword blob's first dword: logged as 0 unless a blob_when() matches)
            for (u32 w = 0; w < 4; ++w) {  // (at + 0xf80, fncmp.blob_when(): (k | 0x80000000, argument, value, mask), the mask for this call's argument;
                                           // fncmp.blob_when_inner(): k | 0xc0000000 and the second structure's selection, bits 5-30 of how)
                const u32 *const when = log + 0x3e0 + 4 * w;
                if (when[1] >= 0x200u || (when[1] & 0xffu) >= n) continue;
                u32 value = stack[1 + (when[1] & 0xffu)];
                if (when[1] & 0x100u) {  // (the argument's bit 8, blob_when()'s deref: the dword it points to; a null one matches nothing)
                    if (!value) continue;
                    value = *reinterpret_cast<const u32 *>((uintptr_t)value);
                }
                if (value != when[2]) continue;
                if (when[0] == ((k & 63) | 0x80000000u)) blob_mask = when[3], matched = true;
                else if (when[0] == ((k & 63) | 0xc0000000u)) how = (how & 0x8000001fu) | (when[3] & 0x7fffffe0u);
            }
            const u32 *wide = nullptr;
            for (u32 w = 0; w < 2; ++w)
                if (log[0x3f0 + 4 * w] == ((k & 63) | 0x80000000u)) wide = log + 0x3f0 + 4 * w;
            args[i] = (how & 0xc0000000u) == 0x40000000u && !matched ? 0
                      : how & 0x80000000u                         ? blob_bytes_hash(p, how, blob_mask, wide)
                                                                  : blob_hash(p, blob_mask);
        }
        for (u32 w = 0; w < 4; ++w) {  // (at + 0x13c0, fncmp.blob_array(): (k | 0x80000000, argument, count argument, stride): the argument logged as a hash of its records)
            const u32 *const array = log + 0x4f0 + 4 * w;
            if (array[0] != ((k & 63) | 0x80000000u) || array[1] != i || !args[i]) continue;
            const s32 count = array[2] < n ? static_cast<s32>(stack[1 + array[2]]) : 0;
            const u32 bytes = (count <= 0 ? 0u : count > 0x100 ? 0x100u : static_cast<u32>(count)) * array[3];
            const u8 *const p = reinterpret_cast<const u8 *>((uintptr_t)args[i]);
            u32 h = 0x811c9dc5u;
            for (u32 b = 0; b < bytes; ++b) h = (h ^ p[b]) * 0x01000193u;
            args[i] = h;
        }
        if (mask >> i & 1) args[i] = string_hash(reinterpret_cast<const u8 *>((uintptr_t)args[i]));
        if (mask >> (16 + i) & 1) args[i] = 0;
        if (log[0x300 + (k & 63)] >> i & 1) args[i] &= 0xff;
        if (log[0x300 + (k & 63)] >> (16 + i) & 1) args[i] &= 0xffff;  // (bits 16-31: WORD arguments)
    }
    // A sequenced read at + 0xf40 (fncmp.seq_read()): (k | 0x80000000, the buffer argument, the count argument, the done argument + 1 or 0, the data's
    // address, its length, the cursor, flags): each call copies the next count bytes of the data (fewer at its end, none for a negative count or a null
    // buffer) to the buffer, moves the cursor on, writes the bytes copied through the done argument (unless null) and, with flags bit 0, returns them.
    u32 *const seq = log + 0x3d0;
    u32 copied = 0;
    const bool sequenced = seq[0] == ((k & 63) | 0x80000000u) && seq[1] < n && seq[2] < n;
    if (sequenced) {
        const u32 buffer = stack[1 + seq[1]], count = stack[1 + seq[2]];
        if (buffer && (s32)count > 0 && seq[6] < seq[5]) copied = count < seq[5] - seq[6] ? count : seq[5] - seq[6];
        if (copied) std::memcpy(reinterpret_cast<void *>((uintptr_t)buffer), reinterpret_cast<const void *>((uintptr_t)(seq[4] + seq[6])), copied);
        seq[6] += copied;
        if (seq[3] && seq[3] <= n && stack[seq[3]]) std::memcpy(reinterpret_cast<void *>((uintptr_t)stack[seq[3]]), &copied, 4);
    }
    // A seek on that data at + 0xf60 (fncmp.seq_seek()): (k | 0x80000000, the offset argument, the origin argument): the cursor moves to the offset
    // from the data's start, the cursor or its end (origin 0 / 1 / 2, any other as 0) and is returned.
    const u32 *const seek = log + 0x3d8;
    const bool seeking = seek[0] == ((k & 63) | 0x80000000u) && seek[1] < n && seek[2] < n;
    if (seeking) {
        const u32 origin = stack[1 + seek[2]];
        seq[6] = (origin == 1 ? seq[6] : origin == 2 ? seq[5] : 0) + stack[1 + seek[1]];
    }
    if (log[0] < 40) {
        u32 *entry = log + 1 + 12 * log[0];
        entry[0] = k;
        entry[1] = n;
        entry[2] = self;
        for (u32 i = 0; i < 7; ++i) entry[3 + i] = i < n ? args[i] : 0;
        entry[10] = 0;   // (past seven arguments, a hash of the rest: tools/fncmp.py tail_hash())
        if (n > 7) {
            u32 t = 0x811c9dc5u;
            for (u32 i = 7; i < n; ++i) t = (t ^ args[i]) * 0x01000193u;
            entry[10] = t;
        }
        entry[11] = 0;
        ++log[0];
    } else if (log[0x784 / 4]) {   // (fncmp.log_fold(): the calls past the room in a hash)
        u32 t = log[0x788 / 4];
        const auto fold = [&t](u32 word) { t = (t ^ word) * 0x01000193u; };
        fold(k);
        fold(n);
        fold(self);
        for (u32 i = 0; i < 7; ++i) fold(i < n ? args[i] : 0);
        u32 tail = 0;
        if (n > 7) {
            tail = 0x811c9dc5u;
            for (u32 i = 7; i < n; ++i) tail = (tail ^ args[i]) * 0x01000193u;
        }
        fold(tail);
        log[0x788 / 4] = t;
    }
    u32 h = self * 0x9e3779b1u + (k + 1) * 0x10001u + n;
    for (u32 i = 0; i < n; ++i) h = h * 33 ^ args[i];
    const u32 *override = log + 0x200 + 2 * (k & 63);  // (LOG_RETURNS: a flag and a value for the slot; flag 2: argument value, as passed;
    if (by_name) return named_answer;
    if (sequenced && (seq[7] & 1)) return copied;
    if (seeking) return seq[6];
    if (override[0] == 4) {                             //  (4: a float, its bits in EAX and pushed in st0)
        const u32 *bits = override + 1;
        __asm {
            mov eax, bits
            fld dword ptr [eax]
        }
        return override[1];
    }
    if (override[0] == 3)                               //  3: the first value & 0xffff calls unanswered, then value >> 16 sign-extended;
        return earlier < (override[1] & 0xffff) ? h : (u32)(s32)(s16)(override[1] >> 16);
    if (override[0] == 5)                               //  5: call j unanswered when bit j of the value is set, else 0: fncmp.fail_mask();
        return earlier < 32 && (override[1] >> earlier & 1) ? h : 0;
    if (override[0] == 6) {                             //  6: as 3, the calls counted down in the value's low word (past the log's 40 entries)
        u32 *const left = log + 0x200 + 2 * (k & 63) + 1;
        if (*left & 0xffff) {
            --*left;
            return h;
        }
        return (u32)(s32)(s16)(*left >> 16);
    }
    if (override[0] == 7)                               //  7: the dword `earlier` of the table at value: fncmp.answers())
        return reinterpret_cast<const u32 *>((uintptr_t)override[1])[earlier];
    return override[0] == 2 && override[1] < n ? stack[1 + override[1]] : override[0] ? override[1] : h;
}

void map_com_recorders(u8 *page) {  // the COM-style ones (this on the stack, ECX undefined): this is logged as 0 and k as k | 0x100
    for (u32 n = 0; n < 16; ++n) {  // (up to 11 stack arguments to + 0xb000, 12-15 from + 0x10000: CreateWindowEx's 12)
        for (u32 k = 0; k < 64; ++k) {
            u8 *at = page + (n < 12 ? 0x5000 + 0x800 * n : 0x10000 + 0x800 * (n - 12)) + 0x20 * k;
            const u8 head[] = {0x89, 0xe0, 0x50, 0x6a, 0x00, 0x6a, (u8)n, 0x68, (u8)k, 0x01, 0x00, 0x00, 0xe8};  // mov eax,esp; push eax; push 0; push n; push k|0x100; call rel32
            std::memcpy(at, head, sizeof head);
            const u32 rel = (u32)(uintptr_t)&record_call - ((u32)(uintptr_t)at + sizeof head + 4);
            std::memcpy(at + sizeof head, &rel, 4);
            u8 *tail = at + sizeof head + 4;
            tail[0] = 0x83, tail[1] = 0xc4, tail[2] = 0x10;  // add esp,16
            if (n) {
                tail[3] = 0xc2, tail[4] = (u8)(4 * n), tail[5] = 0;  // ret 4n
            } else {
                tail[3] = 0xc3;
            }
        }
    }
}

void map_cdecl_recorders(u8 *page) {  // the cdecl ones (the caller pops the arguments, ECX undefined): this is logged as 0 and k as k | 0x200
    for (u32 n = 0; n < 16; ++n) {  // (up to 15 stack arguments: + 0xb000 to + 0xf000)
        for (u32 k = 0; k < 32; ++k) {
            u8 *at = page + 0xb000 + 0x400 * n + 0x20 * k;
            const u8 head[] = {0x89, 0xe0, 0x50, 0x6a, 0x00, 0x6a, (u8)n, 0x68, (u8)k, 0x02, 0x00, 0x00, 0xe8};  // mov eax,esp; push eax; push 0; push n; push k|0x200; call rel32
            std::memcpy(at, head, sizeof head);
            const u32 rel = (u32)(uintptr_t)&record_call - ((u32)(uintptr_t)at + sizeof head + 4);
            std::memcpy(at + sizeof head, &rel, 4);
            u8 *tail = at + sizeof head + 4;
            tail[0] = 0x83, tail[1] = 0xc4, tail[2] = 0x10;  // add esp,16
            tail[3] = 0xc3;                                  // ret (the arguments are the caller's to pop)
        }
    }
}

void map_recorders(u8 *page) {
    map_com_recorders(page);
    map_cdecl_recorders(page);
    for (u32 n = 0; n < 16; ++n) {  // (up to 15 arguments: the layout ends where the COM recorders begin)
        for (u32 k = 0; k < 32; ++k) {
            u8 *at = page + 0x1000 + 0x400 * n + 0x20 * k;
            const u8 head[] = {0x89, 0xe0, 0x50, 0x51, 0x6a, (u8)n, 0x6a, (u8)k, 0xe8};  // mov eax,esp; push eax; push ecx; push n; push k; call rel32
            std::memcpy(at, head, sizeof head);
            const u32 rel = (u32)(uintptr_t)&record_call - ((u32)(uintptr_t)at + sizeof head + 4);
            std::memcpy(at + sizeof head, &rel, 4);
            u8 *tail = at + sizeof head + 4;
            tail[0] = 0x8b, tail[1] = 0x4c, tail[2] = 0x24, tail[3] = 0x08;  // mov ecx,[esp+8]: ECX left as this, as the emulator's recorders leave it
            tail += 4;
            tail[0] = 0x83, tail[1] = 0xc4, tail[2] = 0x10;  // add esp,16
            if (n) {
                tail[3] = 0xc2, tail[4] = (u8)(4 * n), tail[5] = 0;  // ret 4n
            } else {
                tail[3] = 0xc3;
            }
        }
    }
}

// The zlib stand-ins for the codec's env (tools/fncmp.py hooks the game's zlib with the same): no compression, a copy of what fits from the input to the
// output, answering 1 (Z_STREAM_END) when the input is used up, 0 (Z_OK) when it moved some, -5 (Z_BUF_ERROR) when it could not; the inits 0. A case can
// override the inits', deflate's and inflate's answers at kLog + 0xf00 (fncmp.zlib_results(): a flag and a value each). Each call is logged as the entry k
// 0x140 + its index in deflateInit2, deflate, deflateEnd, inflateInit2, inflate, inflateEnd with its scalar arguments (the stream is a stack address; deflate / inflate: the flush, then avail_in and avail_out).
s32 zlib_result(u32 which, s32 natural) {
    const u32 *const config = reinterpret_cast<const u32 *>((uintptr_t)(kLog + 0xf00)) + 2 * which;
    return config[0] ? (s32)config[1] : natural;
}

void zlib_log(u32 k, u32 n, u32 a = 0, u32 b = 0, u32 c = 0, u32 d = 0, u32 e = 0, u32 f = 0, u32 g = 0) {
    u32 *const log = reinterpret_cast<u32 *>((uintptr_t)kLog);
    if (log[0] >= 40) return;
    u32 *const entry = log + 1 + 12 * log[0]++;
    const u32 values[12] = {k, n, 0, a, b, c, d, e, f, g, 0, 0};
    std::memcpy(entry, values, sizeof values);
}

s32 zlib_copy(ZStream *z, u32 which, s32 flush) {
    zlib_log(0x140 + 3 * which - 2, 3, (u32)flush, z->availIn, z->availOut);
    const u32 n = z->availIn < z->availOut ? z->availIn : z->availOut;
    std::memmove(z->nextOut, z->nextIn, n);
    z->nextIn += n, z->availIn -= n, z->totalIn += n;
    z->nextOut += n, z->availOut -= n, z->totalOut += n;
    return zlib_result(which, !z->availIn ? 1 : n ? 0 : -5);
}

s32 __cdecl zlib_deflate_init(ZStream *, s32 level, s32 method, s32 windowBits, s32 memLevel, s32 strategy, const char *version, s32 size) {
    zlib_log(0x140, 7, (u32)level, (u32)method, (u32)windowBits, (u32)memLevel, (u32)strategy, (u32)(uintptr_t)version, (u32)size);
    return zlib_result(0, 0);
}
s32 __cdecl zlib_inflate_init(ZStream *, s32 windowBits, const char *version, s32 size) {
    zlib_log(0x143, 3, (u32)windowBits, (u32)(uintptr_t)version, (u32)size);
    return zlib_result(0, 0);
}
s32 __cdecl zlib_deflate(ZStream *z, s32 flush) { return zlib_copy(z, 1, flush); }
s32 __cdecl zlib_inflate(ZStream *z, s32 flush) { return zlib_copy(z, 2, flush); }
s32 __cdecl zlib_deflate_end(ZStream *) {
    zlib_log(0x142, 0);
    return 0;
}
s32 __cdecl zlib_inflate_end(ZStream *) {
    zlib_log(0x145, 0);
    return 0;
}

void *find(u32 address);

void jump(u8 *at, const void *to) {
    at[0] = 0xe9;  // jmp rel32
    const u32 rel = (u32)(uintptr_t)to - ((u32)(uintptr_t)at + 5);
    std::memcpy(at + 1, &rel, 4);
}

// Game vtables whose slots point at the replacements in the driver's copy of the game's data, as the hooked game resolves them (its originals jump to the
// replacements): code that constructs one of these objects itself then calls through the real vtable. (address, slots; a slot not hooked is left as it is,
// and a case that calls it crashes natively)
const u32 kResolvedVtables[][2] = {{0x00703090, 11},  // String_vtable
                                   {0x007030c8, 11},  // WString_vtable
                                   {0x00703e08, 9},   // TextWriter_vtable
                                   {0x00703908, 13},  // WadSource_vtable
                                   {0x006fd930, 10},  // FileStream_vtable (AdvisorScoring's follows at 0x6fd958)
                                   {0x00703ea0, 12},  // SharedSource_vtable
                                   {0x00703ed0, 12},  // BufferSource_vtable (RefPack)
                                   {0x00703f18, 12},  // ZlibBufferSource_vtable
                                   {0x00703f48, 12},  // SliceSource_vtable
                                   {0x007038d0, 14},  // DirSource_vtable
                                   {0x00703e30, 12},  // FileSource_vtable
                                   {0x00703e60, 12},  // MmioSource_vtable
                                   {0x00703128, 8},   // ZlibCodec_vtable
                                   {0x00703180, 12},  // Handle_vtable
                                   {0x006fd9e8, 12},  // FolderHandle_vtable
                                   {0x006fda18, 12},  // WadHandle_vtable
                                   {0x00703898, 13},  // RefObject2_vtable
                                   {0x00703100, 9},   // StreamBase_vtable
                                   {0x0070a090, 6},   // the narrow-text writer (narrowwriter.h; slot 3 ByteBuf_Write)
                                   {0x0070a0c0, 6},   // the wide-text writer (widewriter.h; slot 4 Obj_PutWord)
                                   {0x006ff170, 2},   // the text formatter's number and string arguments (textarg.h; one slot each)
                                   {0x00703838, 12},  // RefObject_vtable
                                   {0x00703f78, 12},  // BufferedSource_vtable
                                   {0x00703868, 12},  // MemorySource_vtable
                                   {0x00703160, 3},   // CallbackStream_vtable
                                   {0x006fd6a0, 9},   // the action recorder's streams (FileStream's deleting destructor, StreamBase's)
                                   {0x006fd618, 2},   // the balance files (slot 0 the schema getter, 1 BalanceFile_GetValues): Hoardings,
                                   {0x006fd620, 2},   // Level,
                                   {0x006fd628, 2},   // Online,
                                   {0x006fd630, 2},   // Graphics,
                                   {0x006fd638, 2},   // Resolution,
                                   {0x006fd640, 2},   // Sound,
                                   {0x006fd8a8, 2},   // ObjectDef,
                                   {0x006fd8b0, 2},   // TrackType,
                                   {0x00700398, 2},   // Challenges,
                                   {0x007034d8, 16},  // the draw primitives (DrawDriver tables): 8 bits (slot 14 hands over to the blitter, see Draw_SetEnv below),
                                   {0x00703518, 16},  // 16,
                                   {0x00703558, 16},  // 24,
                                   {0x00703598, 16},  // 32 bits
                                   {0x00703650, 12},  // the canvas primitives (CanvasDriver tables): 8,
                                   {0x00703680, 12},  // 16,
                                   {0x007036b0, 12},  // 24,
                                   {0x007036e0, 12},  // 32 bits
                                   {0x006fd958, 2},   // AdvisorScoring,
                                   {0x00702800, 2},   // a theme's (Table73f8a8_At, BalanceFile_GetValues)
                                   {0x00702250, 10},   // the texture surface B's virtual base (its thunks; slot 1 FreePixels' the destructor calls)
                                   {0x00705920, 4},   // the GUI rectangle (GuiRect_Contains, _Centre, _Offset, _Adjust)
                                   {0x0070a260, 1},   // the sound channel's stack functor (SoundChannelFn70a260_Run; SoundCmd700b6c_Run's)
                                   {0x0070a278, 1},   // its settings functor (SoundChannelFn70a278_Run: the channel's setters')
                                   {0x0070a610, 1},   // its rates functor (SoundChannel_ApplyVoiceRates: SoundChannel_SetRateModifier's)
                                   {0x007118e0, 6},   // the MP2 buffer decoder (Mp2BufferDecoder_DeletingDtor, _Open, Mp2Decoder_Close, ...)
                                   {0x0070af00, 6},   // the MP2 stream decoder (Mp2StreamDecoder_DeletingDtor, Mp2BufferDecoder_Open, ...)
                                   {0x0070adb8, 2},   // the voice codec's bit readers over memory (VoiceBits_Skip, _Read)
                                   {0x0070ae28, 2},   // and over a refilled buffer (VoiceBitsBuffered_Skip, _Read)
                                   {0x00703630, 4},   // the TGA reader (Begin, ReadRow, RowBytes, RowReader_CopyRow)
                                   {0x006fefa8, 4},   // the TGA codec (slot 0 the game's: no hook), GetInfo, Load, IsTga
                                   {0x007035e0, 9},   // the image loaders' file stream,
                                   {0x00703608, 9},   // and buffered stream (StreamBase's slots)
                                   {0x007039a0, 3},   // the pixel converters (deleting destructor, forwards, backwards): palette,
                                   {0x007039b0, 3},   // RGB,
                                   {0x00703fa8, 3},   // index,
                                   {0x00703fb8, 3},   // 16-bit (slot 2 the empty 0x5da3d0)
                                   {0x00705980, 6},   // the GUI window (Debug_Printf, Window_SaveRect, _DeletingDtor; 5 _SetEnabled)
                                   {0x00705a50, 6},   // its subclass (the same, but Window705a50_DeletingDtor, _SetEnabled)
                                   {0x00705b20, 6},   // the scroll bar window (slot 5 its SetEnabled, through its children's)
                                   {0x00705950, 4},   // the other window kinds (slot 3 the kind, which the window scripts ask): 4,
                                   {0x007059f0, 4},   // 7,
                                   {0x00705a20, 4},   // 5,
                                   {0x00705af0, 4},   // 12,
                                   {0x00705b98, 4},   // 13,
                                   {0x00705bc8, 4},   // 11,
                                   {0x00705bf8, 4},   // 10,
                                   {0x00705c28, 4},   // 9,
                                   {0x00705c58, 4},   // 6,
                                   {0x00705ac0, 4},   // 8 (the radio group)
                                   {0x00705b50, 5},   // the GUI shapes (slot 4 the deleting destructor a window's new hit area calls): circle,
                                   {0x00705b68, 5},   // ellipse,
                                   {0x00705b80, 5},   // polygon
                                   {0x006ff308, 3},   // the sprite set's virtual base (its deleting destructor's thunk, PtrArray_ReleaseAll's)
                                   {0x006ff320, 11},  // the text part's (a list row's impl: slots 3, 5 and 10 from Window_SetImpl)
                                   {0x006ff3b0, 11},  // two more text part kinds' (list rows' children: 0x49c880, 0x4ae7d0; slot 3,
                                   {0x006ff410, 11},  // 0x4903e0 / 0x490940, unported: the cases give it a recorder)
                                   {0x006ff350, 11},  // the other text part kinds' (the windows dialogbuild.py's builders dress; their
                                   {0x006ff380, 11},  // slots 3 and 9 unported, given recorders by the cases)
                                   {0x006ff3e0, 11},
                                   {0x006ff440, 11},
                                   {0x006ff470, 11},
                                   {0x006ff4a0, 11},
                                   {0x006ff4d0, 11},
                                   {0x006ff500, 11},
                                   {0x006ff530, 11},
                                   {0x007011c8, 4},   // the WCT image's virtual base (the deleting destructor's thunk TexFile_Load calls)
                                   {0x00701168, 4},   // the RLE image's (the deleting destructor's thunk SpriteImage_LoadRle calls)
                                   {0x006fdcc8, 4},   // the light parts' (directional, point: slot 2 the thunk to Stub_One the renderer's sorted add calls)
                                   {0x006fdcd8, 5},   // and a light's virtual base made by Obj_CtorVirtual (slot 2 the same; 5-7 are _purecall)
                                   {0x006fe548, 4},
                                   {0x0070a960, 3},   // the sound library's bank (Release, AddRef, deleting destructor: SoundCategory_ReadBankMap's)
                                   {0x0070a950, 3},   // and category record (the same; slot 2 0x6c1110)
                                   {0x00700aa8, 2},   // the TP2M reader's MAP handler (AttrMapHandler_MatchTag, AttrMap_ReadMapChunk: TP2M_ReadChunks's)
                                   {0x00700b90, 15},  // the sound play command (Get4_5dc650, Get8_51c050: Voice70a8d0_PickQueuedSound's SfxSounds_Find);
                                                      // its slots 4-14 the one-slot commands 0x700ba0-0x700bc8 (soundset.py's senders)
                                   {0x00700b2c, 11},  // the one-slot commands 0x700b2c-0x700b54 (0x700b30 Sound_Load's EAX record apply)
                                   {0x00700b58, 13},  // the sound commands' base (_purecall) and the one-slot commands 0x700b5c-0x700b88
                                                      // (soundset.py's senders 0x51bc70-0x51bd70)
                                   {0x0070a390, 22},   // the group voice's (slot 4 SoundVoice_Init: Voice70a390_Create's; 21 its new child),
                                   {0x0070a2e0, 22},   // and those of the voices 0x70a2e0, 0x70a3e8 and 0x70a878 (slot 4 SoundVoice_Init: their
                                   {0x0070a3e8, 21},   // creates'; 14 and 15 its picks)
                                   {0x0070a4f0, 21},   // the mixer's voices (slot 4 SoundVoice_Init: SoundMixer_AddVoice's)
                                   {0x0070a878, 21},
                                   {0x0070a8d0, 21},   // the queued voice's (slot 4 SoundVoice_Init: Voice70a8d0_Create's)
                                   {0x0070a498, 21},   // the switch voice's (slot 4 SoundVoice_Init, 14 SoundVoice_SwitchVariation, 15 PickSample)
                                   {0x0070a440, 21},   // the sequence voice's (slot 4 SoundVoice_Init, 14 SoundVoice_NextVariation, 15 PickSample)
                                   {0x0070aa80, 16},   // the sound resources SoundResource_Create (0x6c5240) makes, which it loads through slot 3: the
                                   {0x0070aac0, 16},   // stream index (SoundStreamIndex_Load), the mapped index (SoundMapIndex_Map) and the SoundFont
                                   {0x00711ba8, 16},   // bank (SfBank_Load; 0x711ba0 / 0x711ba4 before it and 0x711be8 after it are floats, not slots)
                                   {0x0070ab58, 20},   // the silence stream's (slot 15 Obj_SetRateBits20: SoundStream_SetFormat's in SilenceStream_Ctor)
                                   {0x0070abf8, 19},   // the seeking stream's, the looping stream's and the plain filter's (slots 1 and 3, and
                                   {0x0070aba8, 19},   // the filters' slots SoundStream_SeekTime calls: SoundStream_BuildChain's; slot 5 of
                                   {0x0070a678, 19},   // 0x70abf8, 0x6c6e50, is not hooked)
                                   {0x0070ac50, 21},   // the sample stream's (slot 15 Obj_SetRateBits20: SoundStream_SetFormat's in SoundSample_Ctor)
                                   {0x0070a6f8, 20},   // the samples the sample factory (ChannelVoices_CreateStream) makes: memory,
                                   {0x0070ae30, 24},   // MP2 memory (slot 20 the empty 0x5da3c0),
                                   {0x0070ae90, 24},   // MP2 streamed,
                                   {0x0070ad60, 21},   // voice-coded memory,
                                   {0x0070adc0, 21},   // voice-coded streamed,
                                   {0x0070acf0, 20},   // file (slots 0-19);
                                   {0x0070a6c8, 12},   // the streams it chains in front: 0x70a6c8 (the play's 0x800) and
                                   {0x0070a748, 12},   // the notify stream (SoundStream_BuildChain's);
                                   {0x0070a7a8, 13},   // the channel voice they hang off (ChannelVoice_Release's, the reads');
                                   {0x0070a5d8, 15},   // the channel as the sound system (soundlifecycle.py's trip: slot 4 SoundChannel_RunSlot1)
                                   {0x0070a2d0, 2},    // and the mixer's stop functor through it
                                   {0x0070aa34, 1},    // the memory samples' plain copier ([0xfb2018], SoundSample_ReadMemory's)
                                   {0x0070aef0, 3},    // the streamed samples' windows on their files (slots 1 the read, 2 the seek; 0 an
                                   {0x0070ae18, 3},    // adjusting thunk, not hooked): MP2 (+0x40), voice-coded (+0x40) and the file
                                   {0x0070ad40, 3},    // sample's (+0x2c)
                                   {0x006fe980, 12},   // CMMOnline's (slot 10 Debug_Nop_46cfb0, which MouseMode_Set calls on the new mode; 11 its leave)
                                   {0x006fea10, 12},   // CMMOnline's base (slots 0, 9 and 11 MouseMode_Set's; 0x481a10 / 0x485b70 make it)
                                   {0x006fea70, 12},   // MMStaffDrop's (slots 9, 10 AssignDrop: MouseMode_Set's on the mode 0x5054d0 makes)
                                   {0x006feaa0, 12},   // the 0x6feaa0 mode's (slots 9, 10 the empty 0x5da3c0: MouseMode_Set's on the mode 0x497bc0 makes)
                                   {0x006fead0, 12},   // the 0x6fead0 mode's (slots 9, 10: MouseMode_Set's on the mode 0x481a10 makes)
                                   {0x007058d8, 4},    // the GUI font's (slot 3 GuiFont_CharWidth: Window7cc8bc_ChatAdd's measure)
                                   {0x006fe9e0, 12}};  // MouseModeTile's (slots 9 Const3_6b71c0, 10 MouseModeTile_Assign: MouseMode_Set's on the
                                                       // mode MouseMode_EnterTile15 / 16 make)

// Hooked game functions that ported code stores by their address and that are later called through it (a new window's handlers +0x114 /
// +0x118: Window_DefaultProc). The game's code lies in g_game, read-only, so each gets a jump there to its replacement, as the hooked
// game's original jumps to it: anywhere in the game's code (0x401000-0x6e7000). The pages of the jumps become executable (the rest of such a
// page is the original's code, which a call into it then runs natively, as the hooked game would; elsewhere a call into the code faults).
const u32 kRawCalled[] = {0x0065f6d1,   // Window_DefaultProc (src/game/ui/window's Window_Ctor; tests/fncmp/dialogbuild.py's helpbg())
                          0x0050f1f0,   // IntPair_Ctor and Stub_Ret, the game's constructor and destructor handed to the array iterators
                          0x005d1750,   // by address (tests/fncmp/crt_vector_iterators.py's GameCtor / GameDtor: below the old g_game)
                          0x005f8e50,   // Font_Measure and Font_DrawGlyphShape, the text driver 0x7031c0's measure and draw (Text_SelectFont's
                          0x005f8e80,   // mode 0; tests/fncmp/window705950draw.py)
                          0x00480d10,   // ImeHost_WindowProc, the IME's candidate window's handler (ui/imecandidates' ImeHost_OpenCandidates)
                          0x004959c0,   // List7ca158_RowProc, the objects' lists' row handler (ui/listrows' List7ca158_NewRow)
                          0x004bed60,   // Windows7cc8c4_Handler, told 0x11 when Window7cc8bc_Create hides its windows (ui/window7cc8bc)
                          0x004be730,   // List758f04_RowProcA and RowProcB, the name lists' row handlers (ui/listrows' List758f04_NewRowA /
                          0x004be7c0};  // NewRowB; the kept rows sent 0x12)

void map_raw_called() {
    for (u32 a : kRawCalled) {
        void *const fn = find(a);
        if (!fn || a < kGameCode || a + 5 > kGameLow) fail("cannot jump from", a);
        DWORD was;
        const u32 page = a & ~0xfffu, size = ((a + 4) & ~0xfffu) + 0x1000 - page;
        if (!VirtualProtect((void *)(uintptr_t)page, size, PAGE_READWRITE, &was)) fail("cannot write at", a);
        jump(reinterpret_cast<u8 *>((uintptr_t)a), fn);
        if (!VirtualProtect((void *)(uintptr_t)page, size, PAGE_EXECUTE_READ, &was)) fail("cannot run at", a);
    }
}

void __cdecl debug_printf(const char *, ...) {}  // Debug_Printf 0x5da3c0 (RefObject's teardown slot among others)

// Each slot once: a resolved slot holds a driver address, and while the driver's code lay at the game's .text addresses, resolving it again (two
// entries overlapping) could take it for a hooked game function there (BalanceFile_GetValues was linked at 0x492320, HelpBg_Refresh's address).
// The code now lies below 0x400000, where no game function is, but a slot is still resolved once.
void resolve_vtables() {
    std::set<u32> done;
    for (const auto &v : kResolvedVtables)
        for (Section &s : g_sections)
            if (v[0] >= s.address && v[0] + 4 * v[1] <= s.address + s.raw.size())
                for (u32 i = 0; i < v[1]; ++i) {
                    if (!done.insert(v[0] + 4 * i).second) continue;
                    u32 *const slot = reinterpret_cast<u32 *>(s.raw.data() + (v[0] - s.address) + 4 * i);
                    if (*slot == 0x005da3c0) *slot = (u32)(uintptr_t)&debug_printf;  // (empty in the retail build)
                    else if (void *const fn = find(*slot)) *slot = (u32)(uintptr_t)fn;  // (a slot not hooked stays the game's)
                }
}

void map_stubs() {
    u8 *p = static_cast<u8 *>(VirtualAlloc((void *)(uintptr_t)kStubs, 0x12000, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE));
    if (!p) fail("cannot map the stubs at", kStubs);
    const void *stubs[] = {(const void *)&stub_read, (const void *)&stub_write, (const void *)&stub_global_alloc,
                           (const void *)&stub_global_free, (const void *)&stub_mark, (const void *)&stub_cdecl_log, (const void *)&stub_counter,
                           (const void *)&stub_seek, (const void *)&stub_cdecl_log2, (const void *)&stub_cdecl_log4, (const void *)&stub_zalloc,
                           (const void *)&stub_zfree, (const void *)&stub_stream_size, (const void *)&stub_tell};
    for (u32 i = 0; i < sizeof stubs / sizeof stubs[0]; ++i) jump(p + 0x10 * i, stubs[i]);
    map_recorders(p);
}

void *find(u32 address) {
    for (std::size_t i = 0; i < tpw::hook::kHookCount; ++i)
        if (tpw::hook::kHooks[i].address == address) return tpw::hook::kHooks[i].replacement;
    for (u32 i = 0; i < kCrtStandInCount; ++i)
        if (kCrtStandIns[i].address == address) return kCrtStandIns[i].fn;
    return nullptr;
}

// With FNCMP_CRASHAT set (debugging a bench), where a case faults goes to stderr: the instruction and the address it touched.
int crash_filter(EXCEPTION_POINTERS *x) {
    static const bool report = std::getenv("FNCMP_CRASHAT") != nullptr;
    if (report) {
        const EXCEPTION_RECORD *const r = x->ExceptionRecord;
        std::fprintf(stderr, "crash %08x at %08x touching %08x (esp %08x)\n", (u32)r->ExceptionCode, (u32)(uintptr_t)r->ExceptionAddress,
                     r->NumberParameters > 1 ? (u32)r->ExceptionInformation[1] : 0, (u32)x->ContextRecord->Esp);
        const u32 *const sp = reinterpret_cast<const u32 *>((uintptr_t)x->ContextRecord->Esp);  // (return addresses to look up in the map)
        std::fprintf(stderr, "  stack");
        for (int i = 0; i < 12; ++i) std::fprintf(stderr, " %08x", sp[i]);
        std::fprintf(stderr, "\n");
    }
    return EXCEPTION_EXECUTE_HANDLER;
}

// invoke's frame and target while EBP holds a register argument.
void *g_invokeFn;
u32 g_invokeFrame;

// Pushes the stack arguments right to left, sets the registers (EAX EBX ECX EDX ESI EDI, EBP when nonzero: ECX / EDX for thiscall
// and fastcall, any of them for a register convention), calls, and restores ESP whatever the convention popped. EAX, EDX and EDI
// come back (EDI for ret 'edi').
int invoke(void *fn, const u32 *regs, const u32 *stack, u32 count, int wantSt0, u32 *eax, u32 *edx, double *st0, u32 *edi) {
    u32 ra = 0, rd = 0, rdi = 0, saved = 0;
    double f = 0;
    __try {
        __asm {
            mov saved, esp
            mov esi, stack
            mov ecx, count
        next:
            test ecx, ecx
            jz pushed
            dec ecx
            push dword ptr [esi + ecx * 4]
            jmp next
        pushed:
            mov eax, fn
            mov g_invokeFn, eax
            mov g_invokeFrame, ebp
            mov eax, regs
            mov ebx, [eax + 4]
            mov ecx, [eax + 8]
            mov edx, [eax + 12]
            mov esi, [eax + 16]
            mov edi, [eax + 20]
            cmp dword ptr [eax + 24], 0
            je frame
            mov ebp, [eax + 24]
        frame:
            mov eax, [eax]
            call g_invokeFn
            mov ebp, g_invokeFrame
            mov ra, eax
            mov rd, edx
            mov rdi, edi
            mov esp, saved
            cmp wantSt0, 0
            je done
            fstp f
        done:
        }
    } __except (crash_filter(GetExceptionInformation())) {
        __asm fninit
        return (int)GetExceptionCode();
    }
    *eax = ra;
    *edx = rd;
    *edi = rdi;
    *st0 = f;
    return 0;
}

void *hooked(u32 address) {
    void *const fn = find(address);
    if (!fn) fail("not hooked", address);
    return fn;
}

// The sprite script list's clock (its env's now): the hooked 0x402d70 on the game clock (0x785970).
s32 __cdecl script_now() {
    static const auto now = reinterpret_cast<s32(__fastcall *)(void *, void *)>(hooked(0x00402d70));
    return now(reinterpret_cast<void *>(0x00785970), nullptr);
}

// SpriteScript_Run's opcode call (its env's op): the hooked opcode at that address; END (0x5da3c0, the empty Debug_Printf) returns at
// once; any other address faults, as the code the emulator would run there (a stray word) is not the driver's.
void __cdecl script_op(u32 address) {
    if (address == 0x005da3c0) return;
    void *const fn = find(address);
    if (!fn) *reinterpret_cast<volatile u32 *>(static_cast<std::uintptr_t>(address & 0xfff)) = 0;  // (an access violation)
    reinterpret_cast<void(__cdecl *)()>(fn)();
}

// Mem_AllocCounted / Mem_FreeCounted (0x642860 / 0x642880, stdcall, not ported): the hooked Mem_GlobalAlloc / Free (0x5f5660 / 0x5f5680, through the
// GlobalAlloc import slots the case maps, global_heap()) and the count of blocks at 0xfaa534, as the originals keep it.
void *__stdcall stub_alloc_counted(u32 size) {
    static const auto alloc = reinterpret_cast<void *(__cdecl *)(u32)>(hooked(0x005f5660));
    void *const p = alloc(size);
    if (p) ++*reinterpret_cast<volatile u32 *>(static_cast<std::uintptr_t>(0x00faa534));
    return p;
}

s32 __stdcall stub_free_counted(void *block) {
    static const auto release = reinterpret_cast<void(__cdecl *)(void *)>(hooked(0x005f5680));
    if (block) release(block);
    --*reinterpret_cast<volatile u32 *>(static_cast<std::uintptr_t>(0x00faa534));
    return 1;
}

// StreamBase_OpenIn's stand-in for the image codecs (tests/fncmp/imagecodec.py's _open_in): the name's first dword is the result; when it is not
// negative, the stream gets the name's second dword as its source (one the case lays out).
s32 *__fastcall stub_open_in(StreamBase *s, void *, s32 *out, const char *name, const char *) {
    const u32 *const p = reinterpret_cast<const u32 *>(name);
    *out = static_cast<s32>(p[0]);
    if (*out >= 0) s->source = reinterpret_cast<RefObject *>(static_cast<std::uintptr_t>(p[1]));
    return out;
}

s32 __cdecl stub_strncmp(const char *a, const char *b, u32 n) { return std::strncmp(a, b, n); }

u8 nibble(char c) { return (u8)(c <= '9' ? c - '0' : (c | 0x20) - 'a' + 10); }

// (by hand: sscanf("%2x") per byte measures the rest of the string each call, which made a 1 MB region cost seconds)
std::vector<u8> unhex(const char *s) {
    std::vector<u8> out(std::strlen(s) / 2);
    for (std::size_t i = 0; i < out.size(); ++i) out[i] = (u8)(nibble(s[2 * i]) << 4 | nibble(s[2 * i + 1]));
    return out;
}

// A libjpeg 6b bench whose cases run methods the shims store (FNCMP_JPEG6B_NATIVE, set by the bench: the shims leave the library's
// addresses, which the driver can call, see Jpeg6b_SetNativeMethods): the heap's method fields back to the exe's addresses, as the
// game's memory has them, before the heap is reported. Only the heap: the shims' structures are allocated there, and its unwritten
// bytes are 0xcd, never a function's address.
void jpeg6b_methods_back(u32 at, u32 size) {
    for (u32 o = 0; o + 4 <= size; o += 4) {
        u32 *const p = reinterpret_cast<u32 *>((uintptr_t)(at + o));
        if (const u32 exe = Jpeg6b_ExeMethod(*p)) *p = exe;
    }
}

// libjpeg 6a's, the same (FNCMP_JPEG6A_NATIVE, Jpeg6a_SetNativeMethods / Jpeg6a_ExeMethod).
void jpeg6a_methods_back(u32 at, u32 size) {
    for (u32 o = 0; o + 4 <= size; o += 4) {
        u32 *const p = reinterpret_cast<u32 *>((uintptr_t)(at + o));
        if (const u32 exe = Jpeg6a_ExeMethod(*p)) *p = exe;
    }
}

void put_hex(const u8 *p, u32 n) {
    static const char digits[] = "0123456789abcdef";
    char buf[4096];
    for (u32 i = 0; i < n;) {
        u32 k = 0;
        for (; i < n && k < sizeof buf; ++i) {
            buf[k++] = digits[p[i] >> 4];
            buf[k++] = digits[p[i] & 15];
        }
        std::fwrite(buf, 1, k, stdout);
    }
}

}  // namespace

int main(int argc, char **argv) {
    if (argc < 2) {
        std::printf("usage: tpw_fncmp tp.exe < cases\n");
        return 1;
    }
    load_exe(argv[1]);
    map_stubs();
    map_raw_called();
    resolve_vtables();
    StdStrings_SetEnv(&kStdStringEnv);
    MouseModes_SetEnv(&kMouseModeEnv);
    AdvisorScore_SetEnv(&kAdvisorScoreEnv);
    Research_SetEnv(&kResearchEnv);
    ObjModel_SetEnv(&kObjModelEnv);
    SharedGlobals_SetEnv(&kSharedGlobalsEnv);
    ReleaseList_SetEnv(&kReleaseListEnv);
    SoundCategory_SetEnv(&kSoundCategoryEnv);
    SoundDevice_SetEnv(&kSoundDeviceEnv);
    Person_SetEnv(&kPersonEnv);   // (tests/fncmp/personwalk.py hooks 0x6e7074 with fncmp._cdecl_log2)
    // The chat player's step (ChatPlayer_StepWalk): Sprite_WalkNav the thiscall recorder 5 of 0, as tests/fncmp/chatavatar.py hooks 0x4fa2a0.
    static const ChatAvatarEnv chatAvatarEnv = {reinterpret_cast<u32(__fastcall *)(u8 *)>((uintptr_t)recorder(5, 0))};
    ChatAvatar_SetEnv(&chatAvatarEnv);
    ChatCommand_SetEnv(&kChatCommandEnv);
    ChatEvents_SetEnv(&kChatEventEnv);
    // The interface object's message handler (UiObject_HandleMessage): what 0x7b51f0 is told the thiscall recorders 1-6, as tests/fncmp/uimessage.py
    // hooks 0x481af0, 0x481520, 0x481530, 0x4818c0, 0x481920 and 0x481960.
    static const UiMessageEnv uiMessageEnv = {reinterpret_cast<void(__fastcall *)(void *, void *, u8)>((uintptr_t)recorder(1, 1)),
                                              reinterpret_cast<void(__fastcall *)(void *, void *, u32)>((uintptr_t)recorder(2, 1)),
                                              reinterpret_cast<void(__fastcall *)(u8 *, void *, u32)>((uintptr_t)recorder(3, 1)),
                                              reinterpret_cast<void(__fastcall *)(void *, void *, u32)>((uintptr_t)recorder(4, 1)),
                                              reinterpret_cast<void(__fastcall *)(void *, void *, u32)>((uintptr_t)recorder(5, 1)),
                                              reinterpret_cast<void(__fastcall *)(void *, void *, u32, u32)>((uintptr_t)recorder(6, 2))};
    UiMessage_SetEnv(&uiMessageEnv);
    QueuePieces_SetEnv(&kQueuePiecesEnv);
    PathDrag_SetEnv(&kPathDragEnv);
    Demolish_SetEnv(&kDemolishEnv);
    CellRun_SetEnv(&kCellRunEnv);
    TrackCircuit_SetEnv(&kTrackCircuitEnv);
    TrackSquare_SetEnv(&kTrackSquareEnv);
    PlaceObject_SetEnv(&kPlaceObjectEnv);
    EndThing_SetEnv(&kEndThingEnv);
    ParkOpen_SetEnv(&kParkOpenEnv);
    ParkCrowd_SetEnv(&kParkCrowdEnv);
    GameMode_SetEnv(&kGameModeEnv);
    Things_SetDetachEnv(&kThingDetachEnv);
    Things_SetDeleteEnv(&kThingDeleteEnv);
    Things_SetMessageEnv(&kThingMessageEnv);
    Staff_SetAdviceEnv(&kStaffAdviceEnv);
    Guard_SetDetachEnv(&kGuardDetachEnv);
    Map_SetSerializeEnv(&kMapSerializeEnv);
    // Staff_WalkToRestArea's, Staff_Dismiss's and Staff_ModelState's calls: Sprite_WalkNav, Sprite_SpawnParticle, Thing_Delete and Thoughts_Show
    // the thiscall recorders 28 of 0, 29 of 1, 30 of 0 and 31 of 2, as tests/fncmp/staffstate.py hooks 0x4fa2a0, 0x4fa470, 0x50b780 and 0x50be80.
    static const StaffWalkEnv staffWalkEnv = {reinterpret_cast<u32(__fastcall *)(u8 *)>((uintptr_t)recorder(28, 0)),
                                              reinterpret_cast<void(__fastcall *)(u8 *, void *, s32)>((uintptr_t)recorder(29, 1)),
                                              reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)recorder(30, 0)),
                                              reinterpret_cast<void(__fastcall *)(u8 *, void *, u32, u8)>((uintptr_t)recorder(31, 2)),
                                              reinterpret_cast<u32(__fastcall *)(u8 *)>((uintptr_t)recorder(27, 0)),   // (Person_SetRandomDest, 0x4f9490)
                                              reinterpret_cast<u32(__fastcall *)(const u8 *, void *, u32, u32)>((uintptr_t)recorder(25, 2)),   // (Sprite_PlaySound, 0x4faa00)
                                              // (the four kinds' idles: 0x4da5b0, 0x4d7100, 0x4d46d0, 0x4d63d0)
                                              reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)recorder(22, 0)), reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)recorder(23, 0)),
                                              reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)recorder(24, 0)), reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)recorder(18, 0)),
                                              // (Staff_CtorTyped, 0x504b10; Charset_FormatWide, 0x6acd60: cdecl)
                                              reinterpret_cast<u8 *(__fastcall *)(u8 *, void *, u32, u32, u32, u32)>((uintptr_t)recorder(19, 4)),
                                              reinterpret_cast<s32(__cdecl *)(u8 *, u16 *, u32, u32, ...)>((uintptr_t)(kStubs + 0x1000 + 0xa000 + 0x400 * 4 + 0x20 * 17))};
    Staff_SetWalkEnv(&staffWalkEnv);
    // StaffPoolEntry_Generate's 0x481550: the thiscall recorder 20 of 1, as tests/fncmp/staffcandidate.py hooks it.
    static const StaffCandidateEnv staffCandidateEnv = {reinterpret_cast<void(__fastcall *)(void *, void *, u32)>((uintptr_t)recorder(20, 1))};
    StaffCandidate_SetEnv(&staffCandidateEnv);
    Rides_SetEnv(&kRidesEnv);
    Camera_SetEnv(&kCameraEnv);
    CoasterLinks_SetEnv(&kCoasterLinksEnv);
    CellKind_SetEnv(&kCellKindEnv);
    CellModel_SetEnv(&kCellModelEnv);
    CellMessage_SetEnv(&kCellMessageEnv);
    TrackMessage_SetEnv(&kTrackMessageEnv);
    EditUpdate_SetEnv(&kEditUpdateEnv);
    TestRun_SetEnv(&kTestRunEnv);
    TrakCarRoute_SetEnv(&kTrakCarRouteEnv);
    TypeModel_SetEnv(&kTypeModelEnv);
    Sign_SetEnv(&kSignEnv);
    FlyingLaunch_SetEnv(&kFlyingLaunchEnv);
    RsseExec_SetEnv(&kRsseExecEnv);
    RsseTick_SetEnv(&kRsseTickEnv);
    TrakCars_SetEnv(&kTrakCarsEnv);
    TrakPose_SetEnv(&kTrakPoseEnv);
    BaseCells_SetEnv(&kBaseCellsEnv);
    TrakRideCars_SetEnv(&kTrakRideCarsEnv);
    TrakCarUpdate_SetEnv(&kTrakCarUpdateEnv);
    FlyingUpdate_SetEnv(&kFlyingUpdateEnv);
    Ribbon_SetEnv(&kRibbonEnv);
    Batch_SetEnv(&kBatchEnv);
    BatchQuad_SetEnv(&kBatchQuadEnv);
    ViewCtl_SetEnv(&kViewCtlEnv);
    Landscape_SetDrawEnv(&kLandscapeDrawEnv);
    TerrainWater_SetEnv(&kTerrainWaterEnv);
    TerrainGround_SetEnv(&kTerrainGroundEnv);
    HoardingMesh_SetEnv(&kHoardingMeshEnv);
    TileGrid_SetEnv(&kTileGridEnv);
    SpriteGroups_SetEnv(&kSpriteGroupsEnv);
    TiledPanel_SetEnv(&kTiledPanelEnv);
    GroundShadow_SetEnv(&kGroundShadowEnv);
    SpriteFrameLights_SetEnv(&kSpriteFrameLightsEnv);
    RideBase_SetEnv(&kRideBaseEnv);
    ParkEntrance_SetEnv(&kParkEntranceEnv);
    KeyCancel_SetEnv(&kKeyCancelEnv);
    WKeywords_SetEnv(&kWKeywordsEnv);
    EngineDebug_SetEnv(&kEngineDebugEnv);
    CameraUpdate_SetEnv(&kCameraUpdateEnv);
    WeatherFx_SetReloadEnv(&kWeatherFxReloadEnv);
    ArReplay_SetEnv(&kArReplayEnv);
    LobbyScript_SetEnv(kLobbyScriptCalls, sizeof(kLobbyScriptCalls) / sizeof(kLobbyScriptCalls[0]));
    GamePaths_SetCalls(kShareTexCalls, sizeof(kShareTexCalls) / sizeof(kShareTexCalls[0]));
    GlobalSave_SetCalls(kGlobalSaveCalls, sizeof(kGlobalSaveCalls) / sizeof(kGlobalSaveCalls[0]));
    CoastersLoad_SetEnv(kCoastersLoadCalls, sizeof(kCoastersLoadCalls) / sizeof(kCoastersLoadCalls[0]));
    ArReplayAction_SetEnv(kArReplayActionCalls, sizeof(kArReplayActionCalls) / sizeof(kArReplayActionCalls[0]));
    CoasterRebuild_SetEnv(&kCoasterRebuildEnv);
    CoasterPose_SetEnv(&kCoasterPoseEnv);
    RideResource_SetEnv(&kRideResourceEnv);
    CoasterPlaceCar_SetEnv(&kCoasterPlaceCarEnv);
    TexCache_SetTraceEnv(&kTexCacheTraceEnv);
    LobbyMake_SetEnv(&kLobbyMakeEnv);
    GameFrame_SetEnv(kGameFrameCalls, sizeof(kGameFrameCalls) / sizeof(kGameFrameCalls[0]));
    D3dProbe_SetEnv(&kD3dProbeEnv);
    SpriteFrames_SetEnv(&kSpriteFramesEnv);
    ScapeObjects_SetEnv(&kScapeObjectsEnv);
    ChatCursor_SetEnv(&kChatCursorEnv);
    LobbyScene_SetEnv(&kLobbySceneEnv);
    ParkMap_SetEnv(&kParkMapEnv);
    RideSkin_SetEnv(&kRideSkinEnv);
    CellPaint_SetEnv(&kCellPaintEnv);
    AdvisorCriteria_SetEnv(&kAdvisorCriteriaEnv);
    SceneLeave_SetEnv(&kSceneLeaveEnv);
    CarSoundUpdate_SetEnv(&kCarSoundUpdateEnv);
    SceneViews_SetEnv(&kSceneViewsEnv);
    BendMesh_SetEnv(&kBendMeshEnv);
    TrainRun_SetEnv(&kTrainRunEnv);
    RayPick_SetEnv(&kRayPickEnv);
    WeatherFx_SetDrawEnv(&kWeatherDrawEnv);
    WeatherFx_SetDropsEnv(&kWeatherDropsEnv);
    ModelGroup_SetEnv(&kModelGroupEnv);
    Object_SetEnv(&kObjectEnv);
    StatsGraph_SetEnv(&kStatsGraphEnv);
    ThingBrowser_SetPatrolEnv(&kThingPatrolEnv);
    Panel7cc218_SetShowEnv(&kPanel7cc218ShowEnv);
    Dialog7ca168_SetOpenEnv(&kDialog7ca168OpenEnv);
    Online_SetInstanceEnv(&kOnlineInstanceEnv);
    ThingPanelHandlers_SetEnv(&kThingPanelHandlerEnv);
    GamePartDialogs_SetEnv(&kGamePartDialogEnv);
    PatrolCoverage_SetEnv(&kPatrolCoverageEnv);
    Dialog7cc61c_SetOpenEnv(&kDialog7cc61cOpenEnv);
    Dialog7cc61c_SetPanelEnv(&kDialog7cc61cPanelEnv);
    Panel7b9940_SetExitEnv(&kPanel7b9940ExitEnv);
    Dialog7cc184_SetOpenEnv(&kDialog7cc184OpenEnv);
    SignNameDialog_SetOpenEnv(&kSignNameDialogOpenEnv);
    SignNameDialog_SetHandlerEnv(&kSignNameDialogHandlerEnv);
    Window7cb2a4_SetEnv(&kWindow7cb2a4Env);
    Dialog7cc984_SetEnv(&kDialog7cc984Env);
    MouseHelp_SetEnv(&kMouseHelpEnv);
    StaffThink_SetEnv(&kStaffThinkEnv);
    ObjectRide_SetEnv(&kObjectRideEnv);
    Dialog7cc1f8_SetStatsEnv(&kDialog7cc1f8StatsEnv);
    Dialog7cc5f4_SetHandlerEnv(&kDialog7cc5f4HandlerEnv);
    Window7cb2cc_SetEnv(&kWindow7cb2ccEnv);
    Window7cc8bc_SetMenuEnv(&kWindow7cc8bcMenuEnv);
    Window7cc8c0Handler_SetEnv(&kWindow7cc8c0HandlerEnv);
    Dialog7ca1e0_SetEnv(&kDialog7ca1e0Env);
    MoneyDisplayHandler_SetEnv(&kMoneyDisplayHandlerEnv);
    Window7cc8bcHandler_SetEnv(&kWindow7cc8bcHandlerEnv);
    Window7cb2b0Handler_SetEnv(&kWindow7cb2b0HandlerEnv);
    Window705b20Thumb_SetEnv(&kWindow705b20ThumbEnv);
    GamePause_SetEnv(&kGamePauseEnv);
    GameWindowInput_SetEnv(&kGameWindowInputEnv);
    Panel7c2534Open_SetEnv(&kPanel7c2534OpenEnv);
    Panel7c2534Menus_SetEnv(&kPanel7c2534MenusEnv);
    WindowTick_SetEnv(&kWindowTickEnv);
    GuiStartup_SetEnv(&kGuiStartupEnv, &kGuiTeardownEnv);
    GuiFrame_SetEnv(&kGuiFrameEnv);
    Window705a20Char_SetEnv(&kWindow705a20CharEnv);
    Dialog7cc944List_SetEnv(&kDialog7cc944ListEnv);
    MoneyDisplay_SetEnv(&kMoneyDisplayEnv);
    Panel7ca228Handler_SetEnv(&kPanel7ca228HandlerEnv);
    ObjectCtor_SetEnv(&kObjectCtorEnv);
    Panel7c2534Handler_SetEnv(&kPanel7c2534HandlerEnv);
    ChatNameMenu_SetEnv(&kChatNameMenuEnv);
    Dialog7cc984Open_SetEnv(&kDialog7cc984OpenEnv);
    Dialog7cb2fcOpen_SetEnv(&kDialog7cb2fcOpenEnv);
    MouseClick_SetEnv(&kMouseClickEnv);
    Dialog7cca70List_SetEnv(&kDialog7cca70ListEnv);
    Dialog7cb2b8_SetTabsEnv(&kDialog7cb2b8TabsEnv);
    Dialog7cc4b4_SetSparkleEnv(&kDialog7cc4b4SparkleEnv);
    PasswordPart_SetEnv(&kPasswordPartEnv);
    QuitConfirm_SetEnv(&kQuitConfirmEnv);
    MapSweep_SetEnv(&kMapSweepEnv);
    RasterFormat_SetEnv(&kRasterFormatEnv);
    Dialog7cb400Close_SetEnv(&kDialog7cb400CloseEnv);
    UserSlots_SetEnv(&kUserSlotsEnv);
    Dialog7cc188Address_SetEnv(&kDialog7cc188AddressEnv);
    VersionLabel_SetEnv(&kVersionLabelEnv);
    BankAccountCtor_SetEnv(&kBankAccountCtorEnv);
    SaveDialog_SetEnv(&kSaveDialogEnv);
    SaveDialogCreate_SetEnv(&kSaveDialogCreateEnv);
    Panel7cc328Handler_SetEnv(&kPanel7cc328HandlerEnv);
    Dialog7cb3f8Slots_SetEnv(&kDialog7cb3f8SlotsEnv);
    Dialog7ca150List_SetEnv(&kDialog7ca150ListEnv);
    Dialog7ca168Handler_SetEnv(&kDialog7ca168HandlerEnv);
    Dialog7cc530Handler_SetEnv(&kDialog7cc530HandlerEnv);
    Window7059f0Texts_SetEnv(&kWindow7059f0TextsEnv);
    ImeHost_SetAttachEnv(&kImeHostAttachEnv);
    TextRecordPart_SetEnv(&kTextRecordPartEnv);
    MovieFrame_SetEnv(&kMovieFrameEnv);
    Panel7cc3b0_SetHandlerEnv(&kPanel7cc3b0HandlerEnv);
    RideUpkeep_SetEnv(&kRideUpkeepEnv);
    ObjectWear_SetEnv(&kObjectWearEnv);
    CellEffect_SetEnv(&kCellEffectEnv);
    ObjectUnlock_SetThemesEnv(&kObjectUnlockThemesEnv);
    Dialog7cb25c_SetHandlerEnv(&kDialog7cb25cHandlerEnv);
    Panel7cc218_SetHandlerEnv(&kPanel7cc218HandlerEnv);
    Panel7b9940_SetOpenEnv(&kPanel7b9940OpenEnv);
    Panel7cc218_SetRefreshEnv(&kPanel7cc218RefreshEnv);
    ViewButtons_SetEnv(&kViewButtonsEnv);
    Dialog7cc530_SetOpenEnv(&kDialog7cc530OpenEnv);
    Dialog7cb2b8_SetBuildEnv(&kDialog7cb2b8BuildEnv);
    Dialog7cc61c_SetTextEnv(&kDialog7cc61cTextEnv);
    Dialog7cc188_SetOpenEnv(&kDialog7cc188OpenEnv);
    Dialog7cc4b4_SetUpdateEnv(&kDialog7cc4b4UpdateEnv);
    Panel7cc408Info_SetEnv(&kPanel7cc408InfoEnv);
    DialupList_SetEnv(&kDialupListEnv);
    ThingGoto_SetEnv(&kThingGotoEnv);
    DialogTabHandlers_SetEnv(&kDialogTabHandlersEnv);
    DialogTabs_SetEnv(&kDialogTabsEnv);
    ThingSign_SetEnv(&kThingSignEnv);
    DctMacroblock_SetEnv(&kDctMacroblockEnv);
    TqiFrame_SetEnv(&kTqiFrameEnv);
    Dialog7cc1f8_SetPreviewEnv(&kDialog7cc1f8PreviewEnv);
    Dialog7cc188_SetListEnv(&kDialog7cc188ListEnv);
    Dialog7cca70_SetEnv(&kDialog7cca70Env);
    Dialog7cc944_SetEnv(&kDialog7cc944Env);
    Dialog7ca2e8_SetOpenEnv(&kDialog7ca2e8OpenEnv);
    Dialog7ca2e8_SetHandlerEnv(&kDialog7ca2e8HandlerEnv);
    Dialog7ca1bc_SetHandlerEnv(&kDialog7ca1bcHandlerEnv);
    Dialog7ca150_SetRowsEnv(&kDialog7ca150RowsEnv);
    MenuActions_SetEnv(&kMenuActionsEnv);
    SignRect_SetEnv(&kSignRectEnv);
    StaffSteps_SetEnv(&kStaffStepsEnv);
    StaffCtors_SetEnv(&kStaffCtorsEnv);
    Panel7cc218_SetLevelBoxEnv(&kPanel7cc218LevelBoxEnv);
    ResearchPanel_SetOpenEnv(&kResearchOpenEnv);
    Dialog7cc2ec_SetEnv(&kDialog7cc2ecEnv);
    Dialog7ca210_SetEnv(&kDialog7ca210Env);
    Dialog7cb25c_SetOpenEnv(&kDialog7cb25cOpenEnv);
    Dialog7ca308_SetEnv(&kDialog7ca308Env);
    Panel7cc408_SetEnv(&kPanel7cc408Env);
    OverlayLoad_SetEnv(&kOverlayLoadEnv);
    Dialog7cb400_SetEnv(&kDialog7cb400Env);
    Panel7ca178_SetStatsEnv(&kPanel7ca178StatsEnv);
    Dialog7ca1c8_SetOpenEnv(&kDialog7ca1c8OpenEnv);
    Dialog7ca1c8_SetHandlerEnv(&kDialog7ca1c8HandlerEnv);
    Dialog74fb20_SetEnv(&kDialog74fb20Env);
    Dialog7ca098_SetCreateEnv(&kDialog7ca098CreateEnv);
    TextPanel7cc18c_SetEnv(&kTextPanel7cc18cEnv);
    Panel7cc218_SetTileEnv(&kPanel7cc218TileEnv);
    Panel7c2534_SetEnv(&kPanel7c2534Env);
    Guard_SetStateEnv(&kGuardStateEnv);
    Dialog7ca1c8_SetResetEnv(&kDialog7ca1c8ResetEnv);
    Dialog7cc4b4_SetFlagEnv(&kDialog7cc4b4FlagEnv);
    Objects_SetMeanQueueEnv(&kObjectsMeanQueueEnv);
    Obj7b51f0_SetNowEnv(&kObj7b51f0NowEnv);
    Dialog7cc1f8_SetOpenEnv(&kDialog7cc1f8OpenEnv);
    Dialog7cdac4_SetEnv(&kDialog7cdac4Env);
    Dialog_SetKeyEnv(&kDialogKeyEnv);
    Panel7cc218_SetLevelsEnv(&kPanel7cc218LevelsEnv);
    Panel7cc218_SetFiguresEnv(&kPanel7cc218FiguresEnv);
    Dialog7ca308_SetHandlerEnv(&kDialog7ca308HandlerEnv);
    ResearchPanel_SetEnv(&kResearchPanelEnv);
    Dialog7cb398_SetEnv(&kDialog7cb398Env);
    Dialog7ca150_SetPageEnv(&kDialog7ca150PageEnv);
    ThingBrowser_SetHandlerEnv(&kThingBrowserHandlerEnv);
    MsgTag_SetHandlerEnv(&kMsgTagHandlerEnv);
    GuardEscort_SetEnv(&kGuardEscortEnv);
    StaffWork_SetEnv(&kStaffWorkEnv);
    Dialog7ca098_SetEnv(&kDialog7ca098Env);
    Dialog7cdac4_SetHandlerEnv(&kDialog7cdac4HandlerEnv);
    Dialog7cc1f8_SetTabEnv(&kDialog7cc1f8TabEnv);
    StaffRename_SetEnv(&kStaffRenameEnv);
    ObjectUnlock_SetScoresEnv(&kObjectUnlockScoresEnv);
    Dialog7ca150_SetEnv(&kDialog7ca150Env);
    Dialog7cc1f8_SetEnv(&kDialog7cc1f8Env);
    SpriteLoading_SetEnv(&kSpriteLoadingEnv);
    Object_SetDeleteEnv(&kObjectDeleteEnv);
    Object_SetPathEnv(&kObjectPathEnv);
    Object_SetServiceEnv(&kObjectServiceEnv);
    // The clock's tick (FunnyTime_Update): the post the thiscall recorder 21 of 1, Obj7b51f0_ShowDate 20 of 9, 0x4a5e10 / StatsGraph_Redraw / Dialog7cb25c_Refresh
    // the cdecl recorders 19, 18 and 17 of 1, as tests/fncmp/funnyupdate.py hooks those addresses.
    static const FunnyTimeEnv funnyTimeEnv = {reinterpret_cast<u32(__fastcall *)(u8 *, void *, u8 *)>((uintptr_t)recorder(21, 1)),
                                              reinterpret_cast<void(__fastcall *)(void *, void *, u32, u32, u32, u32, u32, u32, u32, u32, u32)>((uintptr_t)recorder(20, 9)),
                                              reinterpret_cast<void(__cdecl *)(u32)>((uintptr_t)jpeg_recorder(19, 1)),
                                              reinterpret_cast<void(__cdecl *)(u32)>((uintptr_t)jpeg_recorder(18, 1)),
                                              reinterpret_cast<void(__cdecl *)(u32)>((uintptr_t)jpeg_recorder(17, 1))};
    FunnyTime_SetEnv(&funnyTimeEnv);
    // The locale reference's 0x699da0: the cdecl recorder 15 of 0, as tests/fncmp/wfilebuf.py hooks it.
    static const WFileBufEnv wFileBufEnv = {reinterpret_cast<u32(__cdecl *)()>((uintptr_t)jpeg_recorder(15, 0))};
    WFileBuf_SetEnv(&wFileBufEnv);
    // The worker's 0x67e050: the cdecl recorder 14 of 3, as tests/fncmp/soundworker.py hooks it.
    static const SoundWorkerEnv soundWorkerEnv = {reinterpret_cast<u32(__cdecl *)(u32, u32, void *)>((uintptr_t)jpeg_recorder(14, 3))};
    SoundWorker_SetEnv(&soundWorkerEnv);
    // The voices' pool's 0x67e050: the same recorder, as tests/fncmp/channelvoice.py hooks it.
    static const ChannelVoicesEnv channelVoicesEnv = {soundWorkerEnv.beginThread};
    ChannelVoices_SetEnv(&channelVoicesEnv);
    // The things' sweeps: COM recorder 20 of 1, thiscall 21, 22, cdecl 23, 24, 25, 26 of 2, thiscall 27 of 1, as tests/fncmp/thingsrelease.py hooks them.
    static const ThingSweepEnv thingSweepEnv = {Rec{kStubs + 0x5000 + 0x800 * 1 + 0x20 * 20}, Rec{recorder(21, 0)}, Rec{recorder(22, 0)}, Rec{jpeg_recorder(23, 0)},
                                                Rec{jpeg_recorder(24, 0)}, Rec{jpeg_recorder(25, 0)}, Rec{jpeg_recorder(26, 2)}, Rec{recorder(27, 1)}, Rec{jpeg_recorder(28, 0)}};
    Things_SetSweepEnv(&thingSweepEnv);
    static const ObjectUpdateEnv objectUpdateEnv = {Rec{jpeg_recorder(1, 1)}, Rec{jpeg_recorder(2, 2)}, Rec{jpeg_recorder(3, 3)}, Rec{jpeg_recorder(4, 3)},
                                                    Rec{recorder(5, 2)},      Rec{recorder(6, 0)},      Rec{recorder(7, 0)},      Rec{recorder(8, 0)},
                                                    Rec{recorder(9, 0)},      Rec{recorder(10, 0)}};
    ObjectUpdate_SetEnv(&objectUpdateEnv);
    static const SoundStartEnv soundStartEnv = {Rec{jpeg_recorder(1, 3)}, Rec{recorder(2, 2)}};
    Sound_SetStartEnv(&soundStartEnv);
    static const SoundChannelSetupEnv soundChannelSetupEnv = {Rec{recorder(1, 2)}, Rec{recorder(2, 2)}, Rec{recorder(3, 2)}, Rec{recorder(4, 0)}};
    SoundChannel_SetSetupEnv(&soundChannelSetupEnv);
    static const SoundSeedEnv soundSeedEnv = {Rec{jpeg_recorder(1, 0)}};
    SoundSeed_SetEnv(&soundSeedEnv);
    static const SoundManagerEnv soundManagerEnv = {Rec{recorder(20, 0)}};
    SoundManager_SetEnv(&soundManagerEnv);
    static const SoundWaitEnv soundWaitEnv = {Rec{recorder(20, 0)}, Rec{recorder(21, 1)}};
    SoundWait_SetEnv(&soundWaitEnv);
    static const SoundObjectSlotsEnv soundObjectSlotsEnv = {Rec{recorder(20, 3)}, Rec{recorder(21, 2)}, Rec{recorder(22, 3)}, Rec{recorder(23, 2)}, Rec{recorder(24, 2)}};
    SoundObjectSlots_SetEnv(&soundObjectSlotsEnv);
    static const MapResetEnv mapResetEnv = {Rec{recorder(1, 1)}, Rec{recorder(2, 0)}};
    MapReset_SetEnv(&mapResetEnv);
    static const StaffHeldEnv staffHeldEnv = {Rec{recorder(1, 1)}, Rec{recorder(2, 1)}};
    StaffHeld_SetEnv(&staffHeldEnv);
    static const ResearcherEnv researcherEnv = {Rec{recorder(1, 0)}, Rec{recorder(2, 0)}, Rec{recorder(3, 1)}, Rec{recorder(4, 0)}};
    Researcher_SetEnv(&researcherEnv);
    // The channel's open: thiscall recorders 1 of 1, 2 and 3 of 2, COM recorder 4 of 4, thiscall recorders 5-9, as tests/fncmp/soundopen.py hooks them.
    static const SoundChannelOpenEnv soundChannelOpenEnv = {Rec{recorder(1, 1)}, Rec{recorder(2, 2)}, Rec{recorder(3, 2)}, Rec{kStubs + 0x5000 + 0x800 * 4 + 0x20 * 4},
                                                            Rec{recorder(5, 1)}, Rec{recorder(6, 0)}, Rec{recorder(7, 1)}, Rec{recorder(8, 0)}, Rec{recorder(9, 2)}};
    SoundChannel_SetOpenEnv(&soundChannelOpenEnv);
    // The source maker's two constructors: thiscall recorders 20 of 1 and 21 of none, as tests/fncmp/soundmake.py hooks them.
    static const SoundMakeEnv soundMakeEnv = {Rec{recorder(20, 1)}, Rec{recorder(21, 0)}};
    SoundSource_SetMakeEnv(&soundMakeEnv);
    // The weather's constructor and tick: the post the thiscall recorder 21 of 1, WeatherFx_ReloadTextures 20 of 0, agent B's 0x580320 19 of 8, Sound_SetAmbientLevel the cdecl 18 of 1, as
    // tests/fncmp/weathertick.py hooks those addresses.
    static const WeatherEnv weatherEnv = {reinterpret_cast<u32(__fastcall *)(u8 *, void *, u8 *)>((uintptr_t)recorder(21, 1)),
                                          reinterpret_cast<u32(__fastcall *)(u8 *)>((uintptr_t)recorder(20, 0)),
                                          reinterpret_cast<void(__fastcall *)(u8 *, void *, u32, u32, u32, u32, u32, u32, u32, u32)>((uintptr_t)recorder(19, 8)),
                                          reinterpret_cast<void(__cdecl *)(s32)>((uintptr_t)jpeg_recorder(18, 1))};
    Weather_SetEnv(&weatherEnv);
    BankAccount_SetEnv(&kBankAccountEnv);
    MessageCentre_SetEnv(&kMessageCentreEnv);
    GlyphRender_SetEnv(&kGlyphRenderEnv);   // (tests/fncmp/glyphrender.py hooks 0x67a7d0 with fncmp._cdecl_log)
    PtrList_SetEnv(&kPtrListEnv);
    GameHeap_SetEnv(&kGameHeapEnv);
    Jpeg6b_SetCrtEnv(&kJpeg6bCrtEnv);
    Jpeg6a_SetCrtEnv(&kJpeg6aCrtEnv);
    const bool jpeg6b_native = std::getenv("FNCMP_JPEG6B_NATIVE") != nullptr;  // (a libjpeg 6b bench's, see jpeg6b_methods_back)
    Jpeg6b_SetNativeMethods(jpeg6b_native);
    const bool jpeg6a_native = std::getenv("FNCMP_JPEG6A_NATIVE") != nullptr;  // (a libjpeg 6a bench's, see jpeg6a_methods_back)
    Jpeg6a_SetNativeMethods(jpeg6a_native);
    WadSource_SetEnv(&kWadSourceEnv);
    BufferedSource_SetEnv(&kBufferedSourceEnv);
    SurfaceBuffer_SetEnv(&kSurfaceBufferEnv);
    WrapSources_SetEnv(&kWrapSourcesEnv);
    static FileEnv files = *Files_GetEnv();  // (the import slots the case maps; Mem_Alloc is game code)
    files.alloc = &heap_alloc;
    Files_SetEnv(&files);
    // The WAD tree's and finder's tolower / toupper: the CRT stand-ins (crt.cpp; the C locale's, as the cases map it).
    static const WadDirEnv wadDir = {&heap_alloc, &heap_free, reinterpret_cast<s32(__cdecl *)(s32)>(hooked(0x0067fa60))};
    WadDir_SetEnv(&wadDir);
    static const WadFindEnv wadFind = {&heap_alloc, &heap_free, reinterpret_cast<s32(__cdecl *)(s32)>(hooked(0x0067f1a0))};
    WadFind_SetEnv(&wadFind);
    static const DirSourceEnv dirSource = {reinterpret_cast<RefObject *(__fastcall *)(void *)>(hooked(0x00636e40)),
                                           reinterpret_cast<s32 *(__fastcall *)(RefObject *, void *, s32 *, const char *, u32)>(hooked(0x00636f10)),
                                           reinterpret_cast<RefObject *(__fastcall *)(void *)>(hooked(0x006371e0)),
                                           reinterpret_cast<s32 *(__fastcall *)(RefObject *, void *, s32 *, const char *, u32)>(hooked(0x006372b0))};
    DirSource_SetEnv(&dirSource);
    static const ZlibEnv zlib = {&zlib_deflate_init, &zlib_deflate, &zlib_deflate_end, &zlib_inflate_init, &zlib_inflate, &zlib_inflate_end,
                                 reinterpret_cast<void *(__cdecl *)(void *, u32, u32)>(hooked(0x00619250)),
                                 reinterpret_cast<void(__cdecl *)(void *, void *)>((uintptr_t)0x005da3c0),  // (Debug_Printf: the stand-ins never free)
                                 reinterpret_cast<const char *>((uintptr_t)0x007780f0), reinterpret_cast<const u32 *>((uintptr_t)0x007780ec)};
    ZlibCodec_SetEnv(&zlib);
    // Envs whose game defaults call game code by its address, which the driver cannot (the game's code is data there, read-only): that code resolved to its hooked
    // replacement, as the hooked game resolves it; the CPU information constructor (CPUID, the host's) is stub_mark, as tools/fncmp.py's emulator has it.
    static DrivesEnv drives = *Drives_GetEnv();
    drives.cpuCtor = reinterpret_cast<void(__fastcall *)(SystemInfo *)>(&stub_mark);
    drives.stringCtor = reinterpret_cast<String *(__fastcall *)(String *, void *, const char *)>(hooked(0x005f60b0));
    drives.stringCopy = reinterpret_cast<String *(__fastcall *)(String *, void *, const StringView *)>(hooked(0x005f6040));
    drives.stringDtor = reinterpret_cast<void(__fastcall *)(String *)>(hooked(0x005f61e0));
    drives.alloc = &heap_alloc;
    drives.nodeCtor = reinterpret_cast<StringListNode *(__fastcall *)(StringListNode *, void *, String)>(hooked(0x005f8a50));
    drives.stricmp = reinterpret_cast<s32(__cdecl *)(const char *, const char *)>(hooked(0x0067df40));
    Drives_SetEnv(&drives);
    static const CpuInfoEnv cpu = {&stub_has_cpuid, &stub_cpuid};  // the case's fake CPU, as the emulator's
    CpuInfo_SetEnv(&cpu);
    // The registry key's constructor (0x581da0) resolved to its hook; the registry imports stay the import table's slots, which a case maps
    // (import_table(): recorders).
    static const RegKeyEnv regKey = {reinterpret_cast<void(__fastcall *)(RegKey *)>(hooked(0x00581da0)),
                                     reinterpret_cast<s32(__stdcall *const *)(u32)>((uintptr_t)0x006fd000),
                                     reinterpret_cast<s32(__stdcall *const *)(u32, const char *, u32 *)>((uintptr_t)0x006fd004),
                                     reinterpret_cast<s32(__stdcall *const *)(u32, const char *, u32 *, u32 *, void *, u32 *)>((uintptr_t)0x006fd008),
                                     reinterpret_cast<s32(__stdcall *const *)(u32, const char *, u32, char *, u32, u32, void *, u32 *, u32 *)>((uintptr_t)0x006fd00c),
                                     reinterpret_cast<const char *>((uintptr_t)0x00775748)};
    RegKey_SetEnv(&regKey);
    // Game code a port calls directly that is not hooked (yet): a recorder, as the bench's hooks= has it in the emulator.
    static const MapLineEnv mapLine = {reinterpret_cast<u32(__fastcall *)(u8 *, void *, u32, s32, s32)>((uintptr_t)recorder(20, 3))};  // 0x5370e0: tests/fncmp/mapline.py
    MapLine_SetEnv(&mapLine);
    // 0x473c70 (Animator_Update, under Train_PlaceCars): the cdecl recorder k 27 (three arguments), as tests/fncmp/placecars.py hooks it
    // (fncmp.recorder(27, 3, cdecl=True)).
    // Coaster_DeleteCar's calls: the cdecl recorders (k, n) 9 / 2, 10 / 1, 11 / 2, 12 / 2, 13 / 3, 14 / 5, 15 / 2, 16 / 4 and 17 / 0, as
    // tests/fncmp/cardelete.py hooks 0x442d70, 0x469650, 0x43af70, 0x434570, 0x442ea0, 0x452d30, 0x432d50, 0x432df0 and 0x434f10;
    // Coaster_DropCar's others 19 / 3, 20 / 10, 21 / 5, 22 / 7, 23 / 6, 24 / 1 and 25 / 4, as tests/fncmp/cardrop.py hooks 0x433a90,
    // 0x433bd0, 0x433300, 0x4671f0, 0x432c10, 0x4346f0 and 0x469940.
    const auto trainRec = [](u32 k, u32 n) { return Rec{(uintptr_t)(kStubs + 0xb000 + 0x400 * n + 0x20 * k)}; };
    static const TrainEnv train = {trainRec(27, 3), trainRec(9, 2), trainRec(10, 1), trainRec(11, 2), trainRec(12, 2), trainRec(13, 3),
                                   trainRec(14, 5), trainRec(15, 2), trainRec(16, 4), trainRec(17, 0), trainRec(19, 3), trainRec(20, 10),
                                   trainRec(21, 5), trainRec(22, 7), trainRec(23, 6), trainRec(24, 1), trainRec(25, 4)};
    Train_SetEnv(&train);
    // The coaster editor's car deletions' calls: the cdecl recorders (k, n) 18 / 2, 19 / 1, 20 / 2 and 21 / 0, as tests/fncmp/deletecars.py
    // hooks 0x444cb0, 0x435530, 0x435730 and 0x497d20; CoasterEdit_PlaceSelected's (tests/fncmp/placeselected.py) 29 / 0, 30 / 3, 31 / 9,
    // 18 / 0, 19 / 5, 20 / 7, 21 / 2, 17 / 0, 16 / 0 and 15 / 0.
    static const EditToolsEnv editTools = {trainRec(18, 2), trainRec(19, 1), trainRec(20, 2), trainRec(21, 0), trainRec(22, 0),
                                           trainRec(23, 0), trainRec(24, 0), trainRec(25, 0), trainRec(26, 0), trainRec(27, 0),
                                           trainRec(28, 0), trainRec(22, 1), trainRec(29, 0), trainRec(30, 3), trainRec(31, 9),
                                           trainRec(18, 0), trainRec(19, 5), trainRec(20, 7), trainRec(21, 2), trainRec(17, 0),
                                           trainRec(16, 0), trainRec(15, 0)};
    EditTools_SetEnv(&editTools);
    // GameMode_MapClick's calls (src/game/world/mapclick.h): each a recorder (k, n), cdecl ones logged as k | 0x200, as tests/fncmp/mapclick.py
    // hooks their addresses; its `advise` cdecl 9 (the bench's 0x59b590 hook logs the advisor message's id so).
    {
        const auto c = [](u32 k, u32 n) { return Rec{(uintptr_t)(kStubs + 0x1000 + 0xa000 + 0x400 * n + 0x20 * k)}; };
        const auto t = [](u32 k, u32 n) { return Rec{(uintptr_t)recorder(k, n)}; };
        static const MapClickEnv mapClick = {
            c(30, 3), c(12, 0), c(15, 1), c(16, 1), c(13, 4), c(23, 2), c(31, 3), c(17, 6), c(5, 1), c(8, 3), c(14, 4), c(6, 4), c(15, 4),
            c(21, 2), c(17, 2), c(17, 1), c(18, 1), c(16, 4), c(0, 5), c(19, 1),
            c(10, 6), c(1, 5), c(26, 2), c(2, 2), c(10, 1), c(3, 3), c(4, 4), c(11, 3), c(12, 3), c(27, 2), c(28, 2), c(21, 1), c(13, 0),
            c(7, 5), c(14, 0), c(11, 1), c(29, 2), c(9, 1),
            t(26, 0), t(27, 0), t(16, 0), t(28, 0), t(20, 1), t(30, 2), t(22, 2), t(24, 1), t(25, 1), t(12, 0), t(13, 0), t(14, 0), t(15, 0),
            t(18, 3), t(19, 2), t(31, 3), t(31, 4), t(29, 0), t(30, 1)};
        MapClick_SetEnv(&mapClick);
        // GameMode_MapPress's calls (src/game/world/mappress.h), as tests/fncmp/mappress.py hooks their addresses.
        static const MapPressEnv mapPress = {c(30, 3), t(31, 3), t(31, 4), c(10, 1), c(20, 2), c(0, 5), c(23, 2), c(15, 1)};
        MapPress_SetEnv(&mapPress);
        // GameMode_StepBack's calls (src/game/world/gamemode.h GameModeStepEnv), as tests/fncmp/gamemodestep.py hooks their addresses.
        static const GameModeStepEnv gameModeStep = {c(1, 0), t(2, 0), c(3, 4), c(4, 2), c(5, 2), c(6, 1), c(7, 4), c(8, 4)};
        GameMode_SetStepEnv(&gameModeStep);
        // PlaceObject_Footprint's calls (src/game/world/placeobject.h PlaceFootprintEnv), as tests/fncmp/placefootprint.py hooks their addresses
        // (0x532fc0 the track square test's recorder too).
        static const PlaceFootprintEnv footprint = {t(9, 3), t(28, 2), t(10, 0), t(11, 0), t(12, 4), c(13, 1), c(14, 1), c(15, 6)};
        PlaceFootprint_SetEnv(&footprint);
        // The visitor steps' calls (src/game/world/visitorsteps.h VisitorStepsEnv), as tests/fncmp/visitorsteps.py hooks their addresses
        // (Visitor_SetState the person env's recorder 26).
        static const VisitorStepsEnv visitorSteps = {t(1, 0), t(2, 2), t(3, 1), t(4, 1), t(5, 0), t(26, 1), t(6, 1), t(7, 1), t(8, 4), t(9, 1), t(10, 2), t(11, 0), t(12, 0), t(13, 1), t(14, 1),
                                                      t(15, 1), t(16, 2), t(17, 0), t(18, 1), reinterpret_cast<u32(__cdecl *)(s32, s32)>(&stub_cdecl_log2), t(19, 2),
                                                      t(20, 1), t(21, 3), t(22, 1), t(23, 0), t(24, 0), t(25, 0), t(27, 1), t(28, 0), t(29, 0)};
        VisitorSteps_SetEnv(&visitorSteps);
        // World_Save / World_Load's calls (src/game/world/worldsave.h WorldSaveEnv), as tests/fncmp/worldsave.py hooks their addresses.
        static const WorldSaveEnv worldSave = {c(1, 0), t(13, 4), t(14, 4), t(15, 4),
                                               {t(16, 4), nullptr, t(17, 4), t(18, 4), t(19, 4), t(20, 4), t(21, 4), t(22, 4), t(23, 4), t(24, 4), t(25, 4),
                                                t(26, 4), t(27, 4), t(28, 4), t(29, 4), t(30, 4), t(26, 4), nullptr, t(31, 4)}};
        WorldSave_SetEnv(&worldSave);
        // GameMode_MapPreview's calls (src/game/world/mappreview.h MapPreviewEnv), as tests/fncmp/mappreview.py hooks their addresses.
        static const MapPreviewEnv mapPreview = {c(0, 5), c(1, 5), c(2, 6),  c(3, 2),  c(4, 3),  c(5, 4),  c(6, 2),  c(7, 0),  c(8, 3),
                                                 c(9, 5), c(10, 4), c(11, 7), c(12, 0), t(13, 3), t(14, 1), t(15, 1), t(16, 1), t(17, 0)};
        MapPreview_SetEnv(&mapPreview);
        // Thoughts_Show's calls (src/game/world/personscripts.h ThoughtsEnv), as tests/fncmp/thoughtshow.py hooks their addresses.
        static const ThoughtsEnv thoughts = {t(0, 0), c(1, 1), c(2, 1), c(3, 7)};
        Thoughts_SetEnv(&thoughts);
    }
    // 0x564650 (the engine debug log, a file): stub_cdecl_log, as tests/fncmp/engineerrors.py hooks it (the text hashed: strings(log, 0x3f, 1)).
    static const EngineErrorsEnv engineErrors = {reinterpret_cast<void(__cdecl *)(const char *)>(&stub_cdecl_log)};
    EngineErrors_SetEnv(&engineErrors);
    // Animator_Create's texture directories and holder (0x46e4e0, 0x45bbc0): stub_cdecl_log2 and stub_cdecl_log, as tests/fncmp/animatormesh.py
    // hooks them (their own benches are texdirs.py and objmodel.py).
    static const AnimatorMeshEnv animatorMeshEnv = {reinterpret_cast<void(__cdecl *)(u8 *, u8 *)>(&stub_cdecl_log2), reinterpret_cast<void(__cdecl *)(u8 *)>(&stub_cdecl_log)};
    Animator_SetMeshEnv(&animatorMeshEnv);
    // 0x6acd60 and 0x6acca0 (the text formatters, wide and narrow, library code): stub_cdecl_log4, as tests/fncmp/objectdefs.py and
    // dialog7cb2b8.py hook them (the narrow buffer given the stub's wide digits all the same).
    static const TextFormatEnv textFormat = {reinterpret_cast<u32(__cdecl *)(void *, u16 *, u32, u32, ...)>(&stub_cdecl_log4),
                                             reinterpret_cast<u32(__cdecl *)(void *, u8 *, u32, u32, ...)>(&stub_cdecl_log4)};
    TextFormat_SetEnv(&textFormat);
    // The 8-bit sprite's hand-written blitter (0x6e73c0, cseg: not in the listing): stub_cdecl_log4, as tests/fncmp/draw8.py hooks it; the rest the game's.
    static const DrawEnv draw = {reinterpret_cast<DrawContext *>((uintptr_t)0x00fa76b8), reinterpret_cast<const u8 *>((uintptr_t)0x00778108),
                                 reinterpret_cast<SpriteDrawParams *>((uintptr_t)0x00faa4d8),
                                 reinterpret_cast<void(__cdecl *)(s32, s32, const RleSprite *)>(&stub_cdecl_log4), reinterpret_cast<const u8 *>((uintptr_t)0x00fa71f8)};
    Draw_SetEnv(&draw);
    // The canvas primitives': the colour conversion's and rectangles' hooks; the font and the game's clip bottom the game's.
    static const CanvasDrawEnv canvasDraw = {reinterpret_cast<u32(__fastcall *)(Color *, void *, const PixelFormat *)>(hooked(0x005fd050)),
                                             reinterpret_cast<void(__fastcall *)(Rect *)>(hooked(0x0062f940)),
                                             reinterpret_cast<u32(__fastcall *)(const Rect *, void *, const Rect *)>(hooked(0x0062f890)),
                                             reinterpret_cast<Rect *(__fastcall *)(const Rect *, void *, Rect *, const Rect *)>(hooked(0x0062f810)),
                                             reinterpret_cast<const u8 *>((uintptr_t)0x007796e0), reinterpret_cast<const s32 *>((uintptr_t)0x00fa76cc)};
    CanvasDraw_SetEnv(&canvasDraw);
    static CanvasEnv canvas = *Canvas_GetEnv();
    canvas.infoCtor = reinterpret_cast<SurfaceInfo *(__fastcall *)(SurfaceInfo *)>(hooked(0x005fa680));
    Canvas_SetEnv(&canvas);
    static HandlesEnv handles = *Handles_GetEnv();
    handles.alloc = &heap_alloc;
    handles.free = &heap_free;
    handles.ctor28 = reinterpret_cast<RefObject2 *(__fastcall *)(void *)>(hooked(0x00619950));
    handles.open28 = reinterpret_cast<s32 *(__fastcall *)(RefObject2 *, void *, s32 *, u32)>(hooked(0x00619d70));
    handles.ctorE0 = reinterpret_cast<RefObject2 *(__fastcall *)(void *)>(hooked(0x0061a060));
    handles.openE0 = reinterpret_cast<s32 *(__fastcall *)(RefObject2 *, void *, s32 *, u32, u32)>(hooked(0x0061a250));
    handles.ctorMemory = reinterpret_cast<RefObject *(__fastcall *)(void *, void *, u32)>(hooked(0x00618df0));
    handles.bufferCtor = reinterpret_cast<void *(__fastcall *)(void *, void *, char *, u32, u32)>(hooked(0x00636bc0));
    handles.bufferDtor = reinterpret_cast<void(__fastcall *)(void *)>(hooked(0x00636c70));
    Handles_SetEnv(&handles);
    static const StreamOpenEnv streamOpen = {
        reinterpret_cast<s32 *(__cdecl *)(s32 *, char *, u32, const char *)>(hooked(0x005f5710)),
        reinterpret_cast<u32(__stdcall *const *)(const char *)>((uintptr_t)0x006fd10c),
        reinterpret_cast<Handle *(__fastcall *)(Handle *)>(hooked(0x005f8240)),
        reinterpret_cast<void(__fastcall *)(Handle *)>(hooked(0x005f82a0)),
        reinterpret_cast<s32 *(__fastcall *)(Handle *, void *, s32 *, u32, u32)>(hooked(0x005f85d0)),
        reinterpret_cast<s32 *(__fastcall *)(Handle *, void *, s32 *, u32)>(hooked(0x005f8510)),
        reinterpret_cast<s32 *(__fastcall *)(Handle *, void *, s32 *, u32, u32, u32, u32)>(hooked(0x005f83d0)),
        reinterpret_cast<String *(__fastcall *)(String *, void *, const char *)>(hooked(0x005f60b0)),
        reinterpret_cast<String *(__fastcall *)(String *, void *, const StringView *)>(hooked(0x005f6040)),
        reinterpret_cast<void(__fastcall *)(String *)>(hooked(0x005f61e0)),
        reinterpret_cast<s32(__fastcall *)(const String *, void *, const char *, u32)>(hooked(0x005f7b90)),
        reinterpret_cast<StringView *(__fastcall *)(const String *, void *, StringView *, u32)>(hooked(0x005f7d60)),
        reinterpret_cast<StringView *(__fastcall *)(const String *, void *, StringView *, u32)>(hooked(0x005f7dd0)),
        reinterpret_cast<void *const *>((uintptr_t)0x006fda18),
        reinterpret_cast<void *const *>((uintptr_t)0x006fd9e8),
        reinterpret_cast<RefObject *(__fastcall *)(StreamBase *)>(hooked(0x004217e0)),
        &heap_alloc,
        reinterpret_cast<BufferedSource *(__fastcall *)(BufferedSource *)>(hooked(0x00638ef0)),
        reinterpret_cast<s32 *(__fastcall *)(BufferedSource *, void *, s32 *, u32)>(hooked(0x00638f20)),
        reinterpret_cast<void(__fastcall *)(StreamBase *, void *, RefObject *)>(hooked(0x005f7240))};
    StreamOpen_SetEnv(&streamOpen);
    static CrtEnv crt = kDriverCrt;   // (free: the stand-in heap, as a bench's hook of 0x67ad90 gives the emulator)
    crt.free = &heap_free;
    Crt_SetEnv(&crt);
    Libc_SetEnv(&kDriverLibc);
    static FpuEnv fpu = *Fpu_GetEnv();  // (the guard's deleting destructors free with the stand-in heap)
    fpu.free = &heap_free;
    Fpu_SetEnv(&fpu);
    // The sprite script list's calls: the hooked originals (Mem_GlobalAlloc / Free through the GlobalAlloc import slots the case maps, global_heap()).
    static const SpriteScriptEnv sprites = {reinterpret_cast<void *(__cdecl *)(u32)>(hooked(0x005f5660)), reinterpret_cast<void(__cdecl *)(void *)>(hooked(0x005f5680)),
                                            &script_now, reinterpret_cast<u32(__cdecl *)(u8 *)>(hooked(0x00475010)),
                                            reinterpret_cast<void(__cdecl *)()>(hooked(0x00540ab0)), reinterpret_cast<void(__cdecl *)(u8 *)>(hooked(0x00542010)),
                                            reinterpret_cast<void(__cdecl *)()>(hooked(0x00540b30)),
                                            reinterpret_cast<void(__cdecl *)(u32 *, void *, const void *, u32, u32)>(hooked(0x005f5e50)),
                                            reinterpret_cast<void(__cdecl *)(u32 *, void *, void *, u32, u32)>(hooked(0x005f5e20)), &script_op};
    SpriteScripts_SetEnv(&sprites);
    // The pixel conversion's: the heap, the counted allocations above, the inverse colour map's hooks; the game's two formats.
    static const PixConvEnv pixConv = {&heap_free, &stub_alloc_counted, &stub_free_counted, reinterpret_cast<void *(__fastcall *)(void *, void *)>(hooked(0x00642150)),
                                       reinterpret_cast<void(__fastcall *)(void *, void *, u32, u32, const u32 *, u32, u32 *, u8 *)>(hooked(0x00642190)),
                                       reinterpret_cast<void(__fastcall *)(void *, void *)>(hooked(0x00642180)), reinterpret_cast<const u32 *>((uintptr_t)0x00faa268),
                                       reinterpret_cast<const u32 *>((uintptr_t)0x00faa280), &heap_alloc};
    PixConv_SetEnv(&pixConv);
    // The inverse colour map's: the heap; its loops' statics stay the game's (0xfaa500, mapped).
    static const InverseMapEnv inverseMap = {&heap_free, reinterpret_cast<InverseMapStatics *>((uintptr_t)0x00faa500)};
    InverseMap_SetEnv(&inverseMap);
    // The static conversion's set-up and the bitmap copy: the pixel conversion's ports; atexit stub_cdecl_log (tests/fncmp/convert.py hooks 0x67a7d0
    // with fncmp._cdecl_log), handed the game's 0x5fc350 as the original is.
    static const ConvertEnv convert = {reinterpret_cast<PixelConversion *>((uintptr_t)0x00fa7d18), &PixelConversion_Ctor, &PixelConversion_Dtor,
                                       &PixelConversion_Setup, &PixelConversion_Rect, reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)(kStubs + 0x50)),
                                       reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005fc350)};
    Convert_SetEnv(&convert);
    // The TGA reader's: the hooked Mem_GlobalAlloc / Free (the GlobalAlloc import slots the case maps) and Palette_ExpandRgb, the pixel conversion's ports.
    static const TgaEnv tga = {reinterpret_cast<void *(__cdecl *)(u32)>(hooked(0x005f5660)), reinterpret_cast<void *(__cdecl *)(void *)>(hooked(0x005f5680)),
                               reinterpret_cast<void(__cdecl *)(u8 *, const u8 *, u32)>(hooked(0x005f7360)), &PixelConversion_Ctor, &PixelConversion_Setup,
                               &PixelConversion_Row, &PixelConversion_Dtor, reinterpret_cast<const u32 *>((uintptr_t)0x00faa298),
                               reinterpret_cast<const u32 *>((uintptr_t)0x00faa2b0), reinterpret_cast<const u32 *>((uintptr_t)0x00faa268),
                               reinterpret_cast<const u32 *>((uintptr_t)0x00faa280)};
    Tga_SetEnv(&tga);
    // The image codecs': the stream and TGA reader ports, StreamBase_OpenIn's stand-in (stub_open_in), strncmp; the game's vtables (resolved above).
    static const ImageCodecEnv imageCodec = {reinterpret_cast<StreamBase *(__fastcall *)(StreamBase *)>(hooked(0x005f7180)),
                                             reinterpret_cast<void(__fastcall *)(StreamBase *)>(hooked(0x005f71e0)),
                                             reinterpret_cast<s32(__fastcall *)(StreamBase *, void *, s32, u32)>(hooked(0x005f7210)), &stub_open_in,
                                             reinterpret_cast<s32 *(__fastcall *)(StreamBase *, void *, s32 *, StreamBase *, u32)>(hooked(0x0062fd30)),
                                             0x007035e0, 0x00703608, 0x00703630,
                                             reinterpret_cast<s32 *(__fastcall *)(TgaReader *, void *, s32 *, StreamBase *, TgaCodec *)>(hooked(0x00630150)),
                                             reinterpret_cast<s32 *(__fastcall *)(TgaReader *, void *, s32 *, SurfaceInfo *, StreamBase *, TgaCodec *)>(
                                                 hooked(0x006300f0)),
                                             &stub_strncmp, reinterpret_cast<const char *>((uintptr_t)0x00775860)};
    ImageCodec_SetEnv(&imageCodec);
    // The game strings': the game heap (the stand-in's, through GameHeap), the host's vsprintf / vswprintf for Format (tests/fncmp/stringformat.py
    // hooks the game's, 0x67c1e0 / 0x67f4d0, with the same formats).
    static const StringEnv strings = {[](u32 size) -> void * { return GameHeap_Alloc(size); }, [](void *block) { GameHeap_Free(block); },
                                      reinterpret_cast<s32(__cdecl *)(char *, const char *, va_list)>(kDriverLibc.vsprintf),
                                      reinterpret_cast<s32(__cdecl *)(u16 *, const u16 *, va_list)>(kDriverLibc.vswprintf)};
    Strings_SetEnv(&strings);
    // The sprite draw lists': the ground height the game's (out of range; tests/fncmp/spritedraw.py's scripts carry their own); the thing under the
    // cursor (0x532bd0) cdecl recorder 1, its hand-over (0x423b50 on 0x7b05a8) cdecl recorder 2 (the bench hooks both with them).
    static const SpriteDrawEnv spriteDraw = {reinterpret_cast<double(__cdecl *)(u32, u32)>((uintptr_t)0x004527f0),
                                             reinterpret_cast<u32(__cdecl *)()>((uintptr_t)(kStubs + 0x1000 + 0xa000 + 0x20 * 1)),
                                             reinterpret_cast<void(__cdecl *)(u32)>((uintptr_t)(kStubs + 0x1000 + 0xa000 + 0x400 + 0x20 * 2))};
    SpriteDraw_SetEnv(&spriteDraw);
    // The event queue's: the heap; atexit stub_cdecl_log (tests/fncmp/events.py hooks 0x67a7d0 with fncmp._cdecl_log), handed the game's 0x5fbe10 as
    // the original is; the instance 0xfa7cb0 and the critical section's import slots the game's (the case maps them, the slots COM recorders).
    static EventQueueEnv events = *EventQueue_GetEnv();
    events.alloc = &heap_alloc;
    events.free = &heap_free;
    events.atExit = reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)(kStubs + 0x50));
    EventQueue_SetEnv(&events);
    // The screen's: the hooked EventQueue_Post (0x5fbf80) and the heap; the rest the game's addresses (the case maps them, the import slots COM recorders).
    static ScreenEnv screen = *Screen_GetEnv();
    screen.post = reinterpret_cast<void(__fastcall *)(void *, void *, u32, const u32 *)>(hooked(0x005fbf80));
    screen.alloc = &heap_alloc;
    screen.free = &heap_free;
    Screen_SetEnv(&screen);
    // The software cursor's: atExit stub_cdecl_log (tests/fncmp/softcursor.py hooks 0x67a7d0 with fncmp._cdecl_log), handed the game's 0x5fadc0 as the
    // original is; the heap.
    static CursorEnv cursor = *Cursor_GetEnv();
    cursor.atExit = reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)(kStubs + 0x50));
    cursor.free = &heap_free;
    Cursor_SetEnv(&cursor);
    // The asynchronous pointer's: the heap; _beginthreadex and _endthreadex the cdecl recorders k 30 (six arguments) and 31 (one), as
    // tests/fncmp/asyncpointer.py hooks 0x67f780 and 0x67f8b0 (fncmp.recorder(k, n, cdecl=True)); the thread procedure stays the game's 0x5fba00, a value.
    static AsyncPointerEnv pointer = *AsyncPointer_GetEnv();
    pointer.free = &heap_free;
    pointer.beginThread = reinterpret_cast<void *(__cdecl *)(void *, u32, u32(__stdcall *)(void *), void *, u32, u32 *)>(
        (uintptr_t)(kStubs + 0xb000 + 0x400 * 6 + 0x20 * 30));
    pointer.endThread = reinterpret_cast<void(__cdecl *)(u32)>((uintptr_t)(kStubs + 0xb000 + 0x400 * 1 + 0x20 * 31));
    AsyncPointer_SetEnv(&pointer);
    // The game cursors' file loaders (.ani / .cur and .tga) the cdecl recorders k 28 and 29 (one argument), as tests/fncmp/cursor.py hooks
    // 0x4895b0 and 0x489670 (fncmp.recorder(k, 1, cdecl=True)).
    static GameCursorsEnv gameCursors = *GameCursors_GetEnv();
    gameCursors.loadAni = reinterpret_cast<u8 *(__cdecl *)(const char *)>((uintptr_t)(kStubs + 0xb000 + 0x400 * 1 + 0x20 * 28));
    gameCursors.loadTga = reinterpret_cast<u8 *(__cdecl *)(const char *)>((uintptr_t)(kStubs + 0xb000 + 0x400 * 1 + 0x20 * 29));
    GameCursors_SetEnv(&gameCursors);
    // A region of up to 8 MB, as hex. Mapped rather than static: a static this size grew the driver's .bss by 12 MB past the game's
    // addresses, where a case's wild pointer read memory natively that the emulator (the original's side) does not have. At 0x5a000000,
    // not top-down: that put it at 0x7ef00000-0x7fef0000, under the bits of +inf, NaN and big floats (0x7f800000, 0x7fc00000, 3e38),
    // which a case's float stored over a pointer then wrote through natively while the emulator faulted (barmesh.py's 0x477d20).
    // Wherever it is, a random pointer can land in it (partstate.py's 0x44b410 read 0x5ac39898): the case loop takes its access away
    // while a case runs.
    char *token = static_cast<char *>(VirtualAlloc((void *)0x5a000000, 1 << 24, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
    if (!token) token = static_cast<char *>(VirtualAlloc(nullptr, 1 << 24, MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN, PAGE_READWRITE));
    if (!token) fail("cannot map the token buffer", 0);
    for (;;) {
        u32 address, count;
        char conv[24], ret[8];
        if (std::scanf("%x %23s %7s %x", &address, conv, ret, &count) != 4) break;
        std::vector<u32> args(count);
        for (u32 &a : args) std::scanf("%x", &a);
        u32 regions;
        std::scanf("%x", &regions);
        reset_memory();
        std::vector<std::pair<u32, u32>> spans;
        DWORD was;
        VirtualProtect(token, 1 << 24, PAGE_READWRITE, &was);
        for (u32 i = 0; i < regions; ++i) {
            u32 at;
            std::scanf("%x %16777215s", &at, token);
            std::vector<u8> bytes = unhex(token);
            map(at, (u32)bytes.size());
            std::memcpy(reinterpret_cast<void *>((uintptr_t)at), bytes.data(), bytes.size());
            spans.push_back({at, (u32)bytes.size()});
        }
        // No access while the case runs: a wild pointer into it faults natively, as in the emulator, which has no such memory.
        VirtualProtect(token, 1 << 24, PAGE_NOACCESS, &was);
        // ret '+': a sequence (fncmp.Call's then): the count, then each further call's address, convention, count and arguments.
        struct Step {
            void *fn;
            std::string conv;
            std::vector<u32> args;
        };
        std::vector<Step> steps;
        void *fn = find(address);
        steps.push_back({fn, conv, args});
        if (std::strchr(ret, '+')) {
            u32 more;
            std::scanf("%x", &more);
            for (u32 i = 0; i < more; ++i) {
                u32 at, n;
                char stepConv[16];
                std::scanf("%x %15s %x", &at, stepConv, &n);
                std::vector<u32> stepArgs(n);
                for (u32 &a : stepArgs) std::scanf("%x", &a);
                void *const stepFn = find(at);
                if (!stepFn) fn = nullptr;
                steps.push_back({stepFn, stepConv, stepArgs});
            }
        }
        if (!fn) {
            std::printf("nohook\n");
            std::fflush(stdout);
            continue;
        }
        u32 eax = 0, edxOut = 0, ediOut = 0;
        double st0 = 0;
        _control87(_PC_24 | _RC_NEAR, _MCW_PC | _MCW_RC);
        if (std::strchr(ret, 'c')) {  // the case's control word (ret 'c:CW'), with nothing flagged it could unmask
            u16 cw = (u16)std::strtoul(std::strchr(ret, ':') + 1, nullptr, 16);
            __asm fnclex
            __asm fldcw cw
        }
        int code = 0;
        for (Step &step : steps) {
            if (&step != &steps[0])  // (a first call's arguments are its own: a -5 there is a -5)
                for (u32 &a : step.args)
                    if (a == 0xfffffffb) a = eax;  // (fncmp.PREV: the step before's EAX)
            u32 regs[7] = {}, skip = 0;  // EAX EBX ECX EDX ESI EDI EBP
            const char *const how = step.conv.c_str();
            const u32 n = (u32)step.args.size();
            if (how[0] == 't' && n > 0) regs[2] = step.args[0], skip = 1;
            if (how[0] == 'f') {
                if (n > 0) regs[2] = step.args[0], skip = 1;
                if (n > 1) regs[3] = step.args[1], skip = 2;
            }
            if (how[0] == 'e') {  // a register convention (tools/fncmp.py register_args): register names, each taking the next argument
                static const char *const kNames[7] = {"eax", "ebx", "ecx", "edx", "esi", "edi", "ebp"};
                for (const char *c = how; c[0] && c[1] && c[2] && skip < n; c += 3)
                    for (int k = 0; k < 7; ++k)
                        if (std::strncmp(c, kNames[k], 3) == 0) regs[k] = step.args[skip++];
            }
            code = invoke(step.fn, regs, step.args.data() + skip, n - skip, ret[0] == 'd', &eax, &edxOut, &st0, &ediOut);
            if (code) break;
        }
        if (std::strcmp(ret, "edi") == 0) eax = ediOut;  // (ret 'edi': EDI in EAX's place)
        if (std::strchr(ret, 'c')) {  // the control word the call left, then the driver's back
            u16 cw, ours = 0x7f;
            __asm fnstcw cw
            __asm fldcw ours
            edxOut = cw;
        }
        if (code) {
            std::printf("crash %08x\n", (u32)code);
            std::fflush(stdout);
            continue;
        }
        unsigned long long bits;
        std::memcpy(&bits, &st0, 8);
        std::printf("%08x %08x %016llx", eax, edxOut, bits);
        for (auto &s : spans) {
            std::putchar(' ');
            const u8 *p = reinterpret_cast<const u8 *>((uintptr_t)s.first);
            if (jpeg6b_native && s.first == kHeap) jpeg6b_methods_back(s.first, s.second);
            if (jpeg6a_native && s.first == kHeap) jpeg6a_methods_back(s.first, s.second);
            put_hex(p, s.second);
        }
        std::putchar('\n');
        std::fflush(stdout);
    }
    return 0;
}
