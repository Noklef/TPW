// The action replay's actions: see arreplayaction.h.
#include "arreplayaction.h"

#include "../core/buffers.h"
#include "../core/gameheap.h"
#include "../core/stdwstring.h"
#include "../world/world.h"

namespace {

AddrCalls g_calls;

template <typename T>
T &G(u32 address) {
    return *reinterpret_cast<T *>((uintptr_t)address);
}

const u32 kWorld = (u32)&gWorldPtr;
constexpr u32 kThings = 0x007cfb90;

u32 ThingAt(u32 handle) { return G<u32>(kThings + 20 * handle); }
u32 CellHead(u32 id) { return G<u16>(G<u32>(kWorld) + 0x44 * id + 0x2b8); }

}  // namespace

extern "C" {

void ArReplayAction_SetEnv(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

void __fastcall ArReplay_Action(u8 *, void *, u32 action, u32 word) {
    const u32 id = word & 0xffff;
    switch (action) {
        case 0: g_calls.Cd(0x0052f200, id, 0u); break;     // GameMode_Set
        case 0xa: g_calls.Cd(0x0052f1b0, id, 0u); break;   // Global81d7a4_SetRecorded
        case 0x19: g_calls.Cd(0x0052f580, id, 0u); break;  // GameMode_SetOnly
        case 0x1e: g_calls.Cd(0x0052fad0, id); break;      // Global81ecac_SetRecorded
        case 0x23: g_calls.Cd(0x0052fa90, id); break;      // Global81b0cc_SetRecorded
        case 0x32:
            for (u32 h = CellHead(id); h; h = G<u16>(ThingAt(h) + 0xa)) {
                const u32 thing = ThingAt(h);
                if (G<u8>(thing + 2) == 3) {
                    g_calls.Th(0x004815b0, 0x007b51f0u, thing);
                    g_calls.Cd(0x005276d0, 0u);   // FocusCell_Update
                    return;
                }
            }
            break;
        case 0x3d: g_calls.Cd(0x00447ae0, word); break;   // Car_SwapWithNextAt
        case 0x3e: g_calls.Cd(0x00447b20, word); break;   // Car_SwapWithPrevAt
        case 0x42: g_calls.Cd(0x00447da0, id); break;     // CoasterEdit_SetEditingById
        case 0x43: g_calls.Cd(0x00447d60, id); break;
        case 0x44: g_calls.Cd(0x00447dd0, id); break;     // CoasterEdit_SetEditingAt
        default: break;
    }
}

void __fastcall ArReplay_TextAction(u8 *, void *, u32 action, u32 word, u32 n, const u16 *words) {
    if (action != 0x50 && action != 0x55) return;
    const u32 id = word & 0xffff;
    StdWString text;
    CountedBuf_Reset(reinterpret_cast<u8 *>(&text), nullptr, 0);
    StdWString_AssignChars(&text, nullptr, words, n & 0xff);
    u32 saves = reinterpret_cast<u32 &>(gArreplayaction_f7c58c);
    if (!saves) {
        void *block = GameHeap_Alloc(0x50);
        saves = block ? g_calls.Th(0x005aef50, (u32)reinterpret_cast<uintptr_t>(block)) : 0;
        reinterpret_cast<u32 &>(gArreplayaction_f7c58c) = saves;
    }
    g_calls.Th(0x005d3230, saves, &text, 1u);
    u32 thing = 0;
    if (id == 0xea60) {
        thing = ThingAt(G<u16>(G<u32>(kWorld) + 0x1da732));
    } else {
        for (u32 h = CellHead(id); h; h = G<u16>(thing + 0xa)) {
            thing = ThingAt(h);
            if (G<u8>(thing + 2) == 3) break;
        }
    }
    const u16 *shown = text.ptr ? text.ptr : reinterpret_cast<const u16 *>((uintptr_t)0x006fd6c4);
    u16 buffer[0x64];
    if (action == 0x50) {
        g_calls.Th(0x004e2950, thing, shown);
        g_calls.Th(0x004e2920, thing, buffer, 0x64u);
        g_calls.Cd(0x004682b0, thing, shown, buffer);
    } else {
        g_calls.Th(0x004e2a50, thing, shown);
        g_calls.Th(0x004e2860, thing, buffer, 0x64u);
        g_calls.Cd(0x004682b0, thing, buffer, shown);
    }
    CountedBuf_ReleaseAndClear(reinterpret_cast<u8 *>(&text));
}
}
