#include "state.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/globals.h"
#include "../core/vcall.h"
#include "../math/x87.h"
#include "engineerrors.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall SceneFlags_Touch(u32 *flags) {
    flags[1] = global<u32>(0x008bcc40);
    flags[0] |= 0x40000000;
}

u32 __cdecl RenderDevice_Call0() { return vcall<u32>(global<u8 *>(0x008bd52c), 0); }

u32 __cdecl Display_RestoreIfFocused() {
    u32(__stdcall *const getFocus)() = *reinterpret_cast<u32(__stdcall *const *)()>((uintptr_t)0x006fd328);
    if (getFocus() != global<u32>(0x008bd518))
        return 1;
    const u32 generation = global<u32>(0x008bcc40) + 1;
    u8 *const device = global<u8 *>(0x008bd52c);
    global<u32>(0x008bcc44) = 0;
    global<u32>(0x008bcc40) = generation;
    if (vcall<u32>(device, 4))
        return 1;
    return EngineErrors_DropSurfaceLost(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0));
}

u32 __cdecl Display_Restore() {
    const u32 flags = global<u32>(0x008bd508);
    global<u32>(0x008bd508) = flags & ~1u;
    for (int round = 0; round < 3; ++round) {
        RenderDevice_Call0();
        Display_RestoreIfFocused();
    }
    global<u32>(0x008bd508) = flags;
    return 1;
}

void __fastcall Render_FogOn(void *) { global<u32>(0x008bcbc8) |= 0x2000; }

void __fastcall Render_FogOff(void *) { global<u32>(0x008bcbc8) &= ~0x2000u; }

// The start and end are floats, moved as dwords.
void __fastcall Render_SetFog(void *, void *, u32 colour, u32 start, u32 end) {
    global<u32>(0x008bd47c) = colour;
    global<u32>(0x008bd490) = start;
    global<u32>(0x008bd494) = end;
}

void __fastcall Render_SetAmbient(void *, void *, u32 on, u32 colour) {
    global<u32>(0x00879dd8) = 0x3f800000;  // 1.0f
    if (on == 0) {
        global<u32>(0x00879ddc) = 0;
        global<u32>(0x00879de0) = 0;
        global<u32>(0x00879de4) = 0;
        return;
    }
    const float scale = 1.0f / 255.0f;  // 0x702a4c; fild qword of each byte, fmul
    global<float>(0x00879ddc) = (float)(X87Reg((double)(colour >> 16 & 0xff)) * scale);
    global<float>(0x00879de0) = (float)(X87Reg((double)(colour >> 8 & 0xff)) * scale);
    global<float>(0x00879de4) = (float)(X87Reg((double)(colour & 0xff)) * scale);
}

u32 __cdecl Options_Apply() {
    const u32 a = global<u32>(0x0079d86c);
    u32 render = global<u32>(0x008bcbc8);
    render = (a & 0x100) ? render | 0x10000 : render & 0xfffeffff;
    const u32 b = global<u32>(0x007a1a8c);
    global<u32>(0x008bcbc8) = render;
    render = (b & 0x8000) ? render | 1 : render & 0xfffffffe;
    global<u32>(0x008bcbc8) = render;
    u32 settings = global<u32>(0x008bd550);
    settings = (b & 0x80000) ? settings | 2 : settings & 0xfffffffd;
    global<u32>(0x008bd550) = settings;
    const u32 shifted = global<u32>(0x0079d9c8);
    if (shifted)
        global<u32>(0x008c1620) = shifted << 20;
    u32 scene = global<u32>(0x008bd508);
    scene = (b & 2) ? scene & 0xffffffbf : scene | 0x40;
    global<u32>(0x008bd508) = scene;
    if (a & 0x40) {
        Obj_SetFirst(reinterpret_cast<u8 *>(0x0078d8d8), nullptr, 0);
        const u32 options = global<u32>(0x0079d86c) & 0xfffff8ff;
        const u32 flags = global<u32>(0x007a1a8c) & 0xffe50fff;
        global<u32>(0x008bd524) = 4;
        global<u32>(0x0079d86c) = options;
        global<u32>(0x007a1a8c) = flags;
        return flags;
    }
    Obj_SetFirst(reinterpret_cast<u8 *>(0x0078d8d8), nullptr, 1);
    global<u32>(0x008bd524) = 1;
    return 1;
}

u32 __cdecl Environment_Apply() {
    global<u32>(0x008bd47c) = global<u32>(0x007856d4);
    global<u32>(0x007a0b44) = global<u32>(0x007856d8);
    global<u32>(0x007a0b3c) = global<u32>(0x007856dc);
    const s32 c = global<s32>(0x007856e0), e = global<s32>(0x007856e4);
    global<s32>(0x007a006c) = c > 0 ? c : 0;
    global<s32>(0x007a01c0) = e > 0 ? e : 0;
    global<u32>(0x007a0b30) = global<u32>(0x007856e8);
    global<u32>(0x007a0b34) = global<u32>(0x007856ec);
    global<u32>(0x007a0b38) = global<u32>(0x007856f0);
    global<u32>(0x00870ec8) = global<u32>(0x00785718);
    global<u32>(0x00870ecc) = global<u32>(0x0078571c);
    global<u32>(0x00870ed0) = global<u32>(0x00785720);
    for (u32 i = 0; i < 4; i++) {
        global<u32>(0x00f797a4 + 4 * i) = global<u32>(0x007856f4 + 8 * i);
        global<u32>(0x00f797b4 + 4 * i) = global<u32>(0x007856f8 + 8 * i);
    }
    Global81b6dc_Set_52fb10(global<u32>(0x00785600));
    Global81ea14_Set_52fb30(global<u32>(0x007855fc));
    Global81aebc_Set_52fb50(global<u32>(0x00785604));
    Global81b43c_Set_52fb70(global<u32>(0x00785608));
    return 1;
}

u32 __fastcall Render_FadeFog(u8 *fade) {
    u32 colour = global<u32>(0x008bd47c);
    if (global<s32>(0x00785888) > 1 && global<u32>(0x0078d8d8) == 1) {
        const s32 a = at<s32>(fade, 0x44), b = at<s32>(fade, 0x48), c = at<s32>(fade, 0x4c);
        const s32 targetB = at<s32>(fade, 0x54), targetC = at<s32>(fade, 0x58);
        if (a != at<s32>(fade, 0x50) || b != targetB || c != targetC) {
            const s32 na = a + ((targetC - a) >> 4);  // sic: +0x58, not +0x50
            const s32 nb = b + ((targetB - b) >> 4);
            const s32 nc = c + ((targetC - c) >> 4);
            at<s32>(fade, 0x44) = na;
            at<s32>(fade, 0x48) = nb;
            at<s32>(fade, 0x4c) = nc;
            colour = ((((0xffffff00u | (u32)(na >> 16)) << 8) | (u32)(nb >> 16)) << 8) | (u32)(nc >> 16);
        }
    }
    for (u32 i = 0; i < 0x100; i++)
        global<u32>(0x008c2788 + 4 * i) = colour;
    return colour;
}
}
