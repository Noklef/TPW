#include "focuscell.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/actionrec.h"
#include "../core/globals.h"
#include "../core/thunks.h"
#include "cellbuild.h"
#include "cellrun.h"
#include "dirstep.h"
#include "gamemode.h"
#include "mapcell.h"
#include "mapline.h"
#include "object.h"
#include "objectdefs.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u32 CellId(const u8 *thing) { return ((u32)thing[7] << 7) + thing[5] + 1; }

// The 0x28-byte cell of a map cell id (cut to 16 bits) in the world [0x7cf83c].
u8 *Cell28(u32 id) { return reinterpret_cast<u8 *>((uintptr_t)(global<u32>(0x007cf83c) + ((id & 0xffff) + 0x6cde) * 0x28)); }

}  // namespace

extern "C" {

void __cdecl FocusCell_Update(u32 record) {
    global<u16>(0x008186a0) = global<u16>(0x00818698);
    global<u16>(0x00818698) = 0;
    global<s32>(0x007639a4) = -1;
    global<s32>(0x007639a8) = -1;
    global<s32>(0x007639ac) = -1;
    s32 direction;
    if ((Global81ae2c_Equals(0x19) || Global81ae2c_Equals(0x1a)) && global<s32>(0x007639f0) != -1) {
        const s32 x = global<s32>(0x007639f0), y = global<s32>(0x007639f4);
        global<s32>(0x007639a4) = x;
        global<s32>(0x007639e0) = x;
        global<s32>(0x007639a8) = y;
        global<s32>(0x007639e4) = y;
        global<u16>(0x00818698) = (u16)(((u32)y << 7) + (u32)x + 1);
        global<s32>(0x007639ac) = 0;
        global<s32>(0x007639e8) = x;
        global<s32>(0x007639ec) = y;
        global<u8>(0x00818c50) = 4;
        global<u32>(0x008186c4) = global<u32>(0x007cf83c) + ((u32)global<u16>(0x00818698) + 0x6cde) * 0x28;
        direction = 0;
    } else {
        u8 *const selected = reinterpret_cast<u8 *>((uintptr_t)Wrap_Global7c2654_Get_4815a0());
        global<u8 *>(0x008186ac) = selected;
        if (!selected)
            return;
        u16 handle;
        if (*reinterpret_cast<u16 *>((uintptr_t)Fn_4e0df0(selected, nullptr, (u32)(uintptr_t)&handle)) != 0) {
            Fn_4e0df0(selected, nullptr, (u32)(uintptr_t)&handle);
            u8 *const thing = global<u8 *>(0x007cfb90 + (u32)handle * 20);
            if (at<u32>(ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(selected, 0xe)), 0x288) == 1) {
                global<u8 *>(0x008186ac) = thing;
                Wrap_Global7c2654_Set_4815b0((u32)(uintptr_t)thing);
            }
        }
        const u8 *const focus = global<u8 *>(0x008186ac);
        global<u16>(0x00818698) = (u16)CellId(focus);
        const u32 cell = global<u16>(0x00818698);
        global<u32>(0x008186c4) = global<u32>(0x007cf83c) + (cell + 0x6cde) * 0x28;
        const s32 x = (s32)((cell - 1) & 0x7f), y = (s32)((cell - 1) >> 7);
        global<s32>(0x007639a4) = x;
        global<s32>(0x007639e0) = x;
        global<s32>(0x007639a8) = y;
        global<s32>(0x007639e4) = y;
        global<s32>(0x007639e8) = x;
        global<s32>(0x007639ac) = at<s32>(focus, 0x10);
        global<s32>(0x007639ec) = y;
        global<u8>(0x00818c50) = (u8)Position_StepA(&global<s32>(0x007639e0), &global<s32>(0x007639e4), global<s32>(0x007639ac));
        direction = global<s32>(0x007639ac);
    }
    Position_StepB(&global<s32>(0x007639e8), &global<s32>(0x007639ec), direction);
    const u8 *const focus = global<u8 *>(0x008186ac);
    if (focus && global<u16>(0x00818698) != global<u16>(0x008186a0) && (u8)record)
        ActionRec_WriteW(reinterpret_cast<ActionRec *>(0x00785a08), nullptr, 0x32, CellId(focus));
}

void __cdecl FocusCell_TrackDrag(s32 x, s32 y) {
    if (u8 *const thing = global<u8 *>(0x008186ac)) {
        if (const u8 *const def = Object_GetDef(thing)) {
            Global81b0cc_SetRecorded(at<u32>(def, 0x40) != 0);
            Global81ecac_SetRecorded(at<u32>(def, 0x9c));
        }
    }
    u32 mode = 0;
    switch (Global81ecac_IfNoFlag()) {
    case 1: mode = 0xe; break;
    case 2: mode = 0x13; break;
    case 4: mode = 0x1a; break;
    default: break;
    }
    if (mode && !Global81ae2c_Equals(mode))
        GameMode_SetOnly(mode, 1);
    if (global<u16>(0x00818698) == 0)
        return;
    s32 stepX = global<s32>(0x007639a4), stepY = global<s32>(0x007639a8);
    global<s32>(0x007639e0) = stepX;
    global<s32>(0x007639e4) = stepY;
    global<s32>(0x007639e8) = stepX;
    global<s32>(0x007639ec) = stepY;
    global<s32>(0x007639ac) = at<s32>(global<u8 *>(0x008186ac), 0x10);
    global<u8>(0x00818c50) = (u8)Position_StepA(&stepX, &stepY, global<s32>(0x007639ac));
    Position_StepB(&global<s32>(0x007639e8), &global<s32>(0x007639ec), global<s32>(0x007639ac));
    // the point the even number of cells from the stepped cell towards (x, y) (the difference halved rounding down, doubled)
    s32 cx = (s32)((u32)stepX + (((u32)x - (u32)stepX) & ~1u));
    s32 cy = (s32)((u32)stepY + (((u32)y - (u32)stepY) & ~1u));
    global<s32>(0x008186b8) = cx;
    global<s32>(0x008186b4) = cy;
    if (global<s16>(0x007639b0) == -1) {
        s32 endX = cx, endY = cy;
        MapCell_PathNudge(&endX, &endY);
        Map_ApplyLine(Global81ae2c_Get_52f840() | 0x100, cx, cy, &endX, &endY);
    }
    u8 *const focus = global<u8 *>(0x008186ac);
    if (at<u32>(focus, 0x2c) == 0) {
        TrackRun_TraceA(focus);
        global<float>(0x00820afc) = (float)global<s32>(0x0081ede4);
        global<float>(0x00820af8) = (float)global<s32>(0x0081ede8);
        return;
    }
    if (global<s16>(0x007639b0) == -1)
        return;
    u8 *const here = Cell28(((u32)cy << 7) + (u32)cx + 1);
    s32 nudgedX = cx, nudgedY = cy;
    MapCell_PathNudge(&nudgedX, &nudgedY);
    const s16 nudgedWord = (s16)MapCell_Word24(Cell28(((u32)nudgedY << 7) + (u32)nudgedX + 1));
    if (nudgedWord > (s16)MapCell_Word24(here)) {
        cx = nudgedX;
        cy = nudgedY;
    }
    global<u16>(0x00818684) = (u16)(((u32)cy << 7) + (u32)cx + 1);
    global<u16>(0x007639b4) = MapCell_Word24(Cell28(global<u16>(0x00818684)));
    u8 *const cell = Cell28(global<u16>(0x00818684));
    const u32 built = MapCell_CanBuildTrack(cell, nullptr, Global81ae2c_Get_52f840(), 0, 0);
    const s16 startWord = global<s16>(0x007639b0);
    const u32 start = global<u32>(0x00818694);
    if (built != 9) {
        global<u16>(0x00818684) = (u16)start;
        global<s16>(0x007639b4) = startWord;
        global<u32>(0x00816d48) = 0;
        global<u32>(0x00816d50) = 0;
        global<u32>(0x00816d58) = 0;
    }
    if (startWord > global<s16>(0x007639b4))
        CellRun_Connect(Cell28(global<u32>(0x00818684)), Cell28(start), 0);
    else
        CellRun_Connect(Cell28(start), Cell28(global<u32>(0x00818684)), 0);
}
}
