#include "rendererframe.h"

#include <cstdint>
#include <cstring>

#include "../math/mat4.h"
#include "../math/x87.h"
#include "../frontend/lobbysky.h"
#include "../frontend/lobbyscene.h"
#include "renderer.h"
#include "../gfx/texcache.h"

namespace {

AddrCalls g_calls;

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const u32 kViewFlags = 0x008bcbc8, kSceneFlags = 0x008bd508, kMode = 0x008bd524, kDevice = 0x00879ab0, kWorldRender = reinterpret_cast<uintptr_t>(&gRenderer_87a6d0), kCamera = 0x0087aff0;

template <typename... A>
u32 Slot(u32 object, u32 offset, A... args) {
    return reinterpret_cast<u32(__fastcall *)(u32, u32, A...)>((uintptr_t)at<u32>((void *)(uintptr_t)at<u32>((void *)(uintptr_t)object, 0), offset))(object, 0, args...);
}
u32 Device() { return global<u32>(kDevice); }

bool DrawViewSteps(u8 *renderer, const u32 *view) {
    g_calls.Cd(0x005783b0);   // Matrices_Update
    g_calls.Th(0x00582f70, kViewFlags, view[1], view[2], view[3], view[4]);   // View_SetWindow
    if (view[0] & 1) {
        if (!Slot(global<u32>(0x008bd52c), 0)) return false;
    }
    if (global<u32>(kViewFlags) & 0x2000) {
        global<u32>(0x008bd4ec) = 0x3e800000;
    } else {
        global<u32>(0x008bd4ec) = 0x3f19999a;
        if (global<u32>(kMode) == 4) global<u32>(0x008bd4ec) = 0x3e800000;
    }
    if ((view[0] & 0x1000) && at<u32>(renderer, 0x34)) g_calls.Th(0x00575fb0, at<u32>(renderer, 0x34));   // TexSet_BltToBack
    if (global<u8>(0x00790ab0) & 0x16) global<u32>(kSceneFlags) |= 0x100;
    if (global<u32>(kSceneFlags) & 0x100) {
        const u32 terrain = global<u32>(0x00879d60);
        g_calls.Th(0x0056ea90, terrain);   // TerrainTex_CullView
        at<u32>(renderer, 0x44) = terrain + 0x128;
    } else {
        g_calls.Th(0x0056bb00, global<u32>(0x00879d54));   // WorldView_UpdateFootprint
        at<u32>(renderer, 0x44) = g_calls.Th(0x0056dd60, global<u32>(0x00879d54));
    }
    g_calls.Th(0x0058d8e0, gTexcache_8c4b44);   // TexCache_BeginFrame
    g_calls.Th(0x00581da0, global<u32>(0x008bcbb8));   // Ptr_Ctor
    g_calls.Th(0x0058a630, global<u32>(0x008c47a0));   // SpriteQueue_Clear
    g_calls.Th(0x00582390, global<u32>(0x008bcbc4), (global<u32>(kSceneFlags) & 0xc0000000) ? 3u : 2u);   // DrawBuffer_Reset
    at<u32>(renderer, 0x30) = 0;
    g_calls.Th(0x00579c70, 0x008bb220u);   // MeshQueue_Clear
    Slot(Device(), 0x18, gLobbysky_8bd490, gLobbysky_8bd494, gLobbysky_8bd47c);
    if (!Slot(Device(), 0x1c)) return false;
    if ((view[0] & 0x10) && !g_calls.Th(0x00576910, (u32)(uintptr_t)renderer)) return false;   // Renderer_DrawCellLists
    if (view[0] & 0x20) {
        for (u32 i = 0; i < 6; ++i) {
            u32 node = at<u32>(renderer, 0x14 + i * 4);
            at<u32>((void *)(uintptr_t)global<u32>(kWorldRender), 0x30) = node;
            for (; node; node = at<u32>((void *)(uintptr_t)node, 4))
                if (!Slot(at<u32>((void *)(uintptr_t)node, 0), 4)) return false;
        }
    }
    if (view[0] & 0x200) {   // (through the virtual base's offset)
        const u32 groups = global<u32>(0x008bba48);
        Slot(groups + at<u32>((void *)(uintptr_t)at<u32>((void *)(uintptr_t)groups, 0), 4), 4);
    }
    if (view[0] & 0x100) g_calls.Th(0x0057dc80, global<u32>(0x008bcbac));   // WeatherFx_TouchTextures
    if (view[0] & 0x400) g_calls.Th(0x00585f10, 0x008c1828u);   // ScrollLayers_Update
    if (view[0] & 0x2000) g_calls.Th(0x00577a30, 0x0087aee8u);   // TileGrid_TouchVisible
    if (view[0] & 0x4000) g_calls.Th(0x0058ac70, 0x008c47b0u);   // Global8c47b0_Touch
    const u32 once = at<u32>(renderer, 0x78);
    if (once && at<u32>(renderer, 0x74) != global<u32>(0x008bcc40)) {
        reinterpret_cast<u32(__fastcall *)(u32, u32)>((uintptr_t)once)(at<u32>(renderer, 0x74), 0);
        at<u32>(renderer, 0x74) = global<u32>(0x008bcc40);
    }
    g_calls.Th(0x0058a720, global<u32>(0x008c47a0));   // SpriteQueue_ShowAll
    g_calls.Th(0x00579c80, 0x008bb220u);   // Pointers_MarkAndCallSlot4
    if (!g_calls.Th(0x0058cf40, gTexcache_8c4b44)) return false;   // TexCache_Update
    if (view[0] & 0x10) {
        if (!g_calls.Th(0x005863c0, 0x008c1828u)) return false;   // Landscape_Draw
        if (!g_calls.Th(0x0056f670, global<u32>(0x00879d60))) return false;   // TerrainGround_Draw
        if ((global<u32>(kViewFlags) & 0x10000) && !g_calls.Th(0x00570d90, global<u32>(0x00879d60))) return false;   // TerrainWater_Draw
        if (!g_calls.Th(0x0057c620, global<u32>(0x008bba48))) return false;   // SpriteGroups_Draw
    }
    if ((view[0] & 0x2000) && !g_calls.Th(0x00577ae0, 0x0087aee8u)) return false;   // TileGrid_Draw
    if ((view[0] & 0x4000) && !g_calls.Th(0x0058ac90, 0x008c47b0u)) return false;   // PointSprites_Draw
    if (view[0] & 0x800) {
        g_calls.Th(0x00584b10, 0x008c1658u);   // LightList_Clear
        if (!(global<u32>(kViewFlags) & 0x20000) && at<u32>(renderer, 0x2c)) {
            u32 node = at<u32>(renderer, 0x2c);
            at<u32>((void *)(uintptr_t)global<u32>(kWorldRender), 0x30) = node;
            for (; node; node = at<u32>((void *)(uintptr_t)node, 4)) {
                if (Slot(at<u32>((void *)(uintptr_t)node, 0), 8) != 1) continue;
                const u32 place = Slot(at<u32>((void *)(uintptr_t)node, 0), 0xc);
                if (place) {
                    const u32 *const p = reinterpret_cast<const u32 *>((uintptr_t)place);
                    g_calls.Th(0x00584be0, 0x008c1658u, p[0], p[1], p[2]);   // LightList_Add
                }
            }
        }
    }
    if (!g_calls.Th(0x0058a660, global<u32>(0x008c47a0))) return false;   // SpriteQueue_DrawWithPanels
    if ((global<u32>(kViewFlags) & 0x200000) && !g_calls.Th(0x0057c2a0, 0x008bb638u)) return false;   // GroundShadows_DrawAll
    if ((view[0] & 0x100) && !g_calls.Th(0x0057ff60, global<u32>(0x008bcbac))) return false;   // WeatherFx_Frame
    if (!g_calls.Th(0x00582170, global<u32>(0x008bcbb8))) return false;   // DrawBuffer_DrawFrame
    if (global<u32>(kMode) == 1 && !(global<u32>(kViewFlags) & 0x18000000) && !(global<u32>(kViewFlags) & 2)) Slot(Device(), 0x30);
    g_calls.Cd(0x00590750);   // AnimTextures_DrawSheet
    if (!Slot(Device(), 0x20)) return false;
    if ((global<u32>(kViewFlags) & 0x80000) && global<u32>(kMode) == 1) {
        if (!Slot(Device(), 0x1c)) return false;
        const float width = (float)global<s32>(0x008bcbd0), height = (float)global<s32>(0x008bcbd4);
        u32 widthBits, heightBits;
        std::memcpy(&widthBits, &width, 4);
        std::memcpy(&heightBits, &height, 4);
        if (!Slot(Device(), 0x28, 0u, 0u, widthBits, heightBits, gLobbyscene_8bd4e4, 0u)) return false;
        if (!Slot(Device(), 0x20)) return false;
    }
    return true;
}

u32 CallView(u8 *renderer, const u32 *view) { return g_calls.Th(0x00576a00, (u32)(uintptr_t)renderer, (u32)(uintptr_t)view); }

}  // namespace

void RendererFrame_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u32 __fastcall Renderer_DrawView(u8 *renderer, void *, const u32 *view) {
    u32 saved[16];
    std::memcpy(saved, &global<u32>(kCamera), 0x40);
    std::memcpy(&global<u32>(kCamera), reinterpret_cast<const void *>((uintptr_t)view[5]), 0x40);
    const bool drawn = DrawViewSteps(renderer, view);
    if (!drawn) Slot(Device(), 0x20);
    std::memcpy(&global<u32>(kCamera), saved, 0x40);
    if (!drawn) return 0;
    ++global<u32>(0x008bcc44);
    return 1;
}

u32 __fastcall Renderer_DrawFrame(u8 *renderer) {
    if (global<u8>(kSceneFlags) & 0x40) global<u32>(kSceneFlags) |= 0x80000000;
    const u32 mode = global<u32>(kMode);
    const u32 savedView = global<u32>(kViewFlags);
    if (mode == 4) {
        global<u32>(kViewFlags) = (savedView & ~0x200000u & ~0x2000u) | 0x20800;
        global<u32>(0x008bd550) &= ~4u;
    }
    if (global<u32(__stdcall *)()>(0x006fd328)() != global<u32>(0x008bd518)) return 1;
    if (global<u32>(kSceneFlags) & 1) {
        global<u32>(kSceneFlags) &= ~1u;
        g_calls.Th(0x005879c0, 0x008c4748u);   // LoadScreen_Close
    }
    if (global<u32>(kSceneFlags) & 0x10000000) {
        if (!g_calls.Cd(0x005774c0)) {   // Engine_ResetDevice
            g_calls.Th(0x0056af20, 0x00879ad0u, 3u, 0xb0754013u);   // EngineErrors_Push
            return 0;
        }
        global<u32>(kSceneFlags) &= ~0x10000000u;
    }
    const u32 savedScene = global<u32>(kSceneFlags);
    g_calls.Th(0x00583080, kViewFlags);   // View_ResetWindow
    const s32 control = (s16)g_calls.Cd(0x00591220);   // Fpu_SinglePrecision
    bool drawn = true;
    if (global<u8>(kViewFlags) & 4) {
        if (global<u8>(kViewFlags) & 8) {
            global<u32>(0x00768aac) = 3;
        } else if (global<u32>(0x00768aac) != 2) {
            g_calls.Cd(0x00583790);   // Display_Restore
            global<u32>(0x00768aac) = 2;
        }
        g_calls.Th(0x005836b0, kSceneFlags);   // SceneFlags_Touch
        for (s32 i = 0; drawn && i < at<s32>(renderer, 0x70); ++i) {
            const u32 *&source = at<const u32 *>(renderer, 0x60 + (u32)i * 4);
            u32 left[16], right[16];
            const float separation = gRenderer_DrawFrame_8bd478;
            std::memcpy(right, global<const void *>(0x0087afe8), 0x40);
            std::memcpy(left, global<const void *>(0x0087afe8), 0x40);
            Mat4_MoveAlongX(reinterpret_cast<float *>(left), nullptr, (float)(X87Reg(separation) * global<float>(0x007017f8)));
            Mat4_MoveAlongX(reinterpret_cast<float *>(right), nullptr, (float)(X87Reg(separation) * global<float>(0x007017fc)));
            const u32 savedFlags = global<u32>(kViewFlags);
            u32 view[6];
            std::memcpy(view, source, 0x18);
            global<u32>(kViewFlags) = savedFlags & ~0x10000u;
            if (global<u32>(kViewFlags) & 8) {
                view[0] = source[0] & 0x1b11;
                view[5] = (u32)(uintptr_t)left;
                at<u32>(global<void *>(0x0087a248), 0x2c) = 0xffff0000;
                drawn = CallView(renderer, view) != 0;
                if (drawn) {
                    global<u32>(kViewFlags) |= 0x80000000;
                    view[5] = (u32)(uintptr_t)right;
                    at<u32>(global<void *>(0x0087a248), 0x2c) = 0xff00ffff;
                    drawn = CallView(renderer, view) != 0;
                }
                if (drawn) {
                    global<u32>(kViewFlags) &= 0x7fffffff;
                    at<u32>(global<void *>(0x0087a248), 0x2c) = 0xffffffff;
                    view[0] = source[0] & 0x420;
                    drawn = CallView(renderer, view) != 0;
                }
            } else {
                float *const window = reinterpret_cast<float *>(view + 1);
                const float scale = global<float>(0x00701818);
                float x = window[0], y = window[1], w = window[2], h = window[3];
                window[0] = (float)X87_SubR((X87Reg(x) - global<float>(0x00701808)) * global<float>(0x0070180c), global<float>(0x00701810));
                window[1] = (float)X87_SubR((X87Reg(y) - global<float>(0x00701808)) * global<float>(0x0070180c), global<float>(0x00701814));
                window[2] = (float)(X87Reg(w) * scale);
                window[3] = (float)(X87Reg(h) * scale);
                view[5] = (u32)(uintptr_t)left;
                drawn = CallView(renderer, view) != 0;
                if (drawn) {
                    std::memcpy(view, source, 0x18);
                    x = window[0], y = window[1], w = window[2], h = window[3];
                    window[0] = (float)X87_SubR((X87Reg(x) - global<float>(0x00701808)) * global<float>(0x0070180c), global<float>(0x0070181c));
                    window[1] = (float)X87_SubR((X87Reg(y) - global<float>(0x00701808)) * global<float>(0x0070180c), global<float>(0x00701814));
                    window[2] = (float)(X87Reg(w) * scale);
                    window[3] = (float)(X87Reg(h) * scale);
                    view[5] = (u32)(uintptr_t)right;
                    drawn = CallView(renderer, view) != 0;
                }
            }
            if (drawn) global<u32>(kViewFlags) = savedFlags;
        }
    } else {
        if (global<u32>(0x00768aac) != 1) {
            global<u32>(0x00768aac) = 1;
            g_calls.Th(0x005836b0, kSceneFlags);   // SceneFlags_Touch
        } else if (at<s32>(renderer, 0x70) == 1) {
            u8 *const lastCamera = reinterpret_cast<u8 *>((uintptr_t)0x0087a680);
            if (!(global<u8>(0x0087a6c0) & 1)) {
                std::memset(lastCamera, 0, 0x40);
                global<u8>(0x0087a6c0) |= 1;
                at<float>(lastCamera, 0x3c) = at<float>(lastCamera, 0x28) = at<float>(lastCamera, 0x14) = at<float>(lastCamera, 0) = 1.0f;
                g_calls.Cd(0x0067a7d0, 0x005774b0u);   // atexit
            }
            const void *const camera = global<const void *>(0x0087afe8);
            if (!(global<u32>(kSceneFlags) & 0xc0000000) && std::memcmp(camera, lastCamera, 0x40) != 0) global<u32>(kSceneFlags) |= 0x80000000;
            std::memcpy(lastCamera, camera, 0x40);
        } else {
            global<u32>(kSceneFlags) |= 0x80000000;
        }
        for (s32 i = 0; drawn && i < at<s32>(renderer, 0x70); ++i) drawn = CallView(renderer, at<const u32 *>(renderer, 0x60 + (u32)i * 4)) != 0;
    }
    if (!drawn) {
        global<u32>(kViewFlags) = savedView;
        global<u32>(kSceneFlags) = savedScene;
        return g_calls.Th(0x0056afc0, 0x00879ad0u);   // EngineErrors_DropSurfaceLost
    }
    g_calls.Th(0x00561760, 0x008798d0u);   // ScreenGrab_Save
    global<u32>(kSceneFlags) &= 0x3fffffff;
    g_calls.Cd((global<u32>(kSceneFlags) & 2) ? 0x00591250u : 0x00591240u, (u32)control);   // Fpu_LoadControlFullPrecision or Fpu_LoadControl
    global<u32>(kViewFlags) = savedView;
    global<u32>(kSceneFlags) = savedScene;
    return 1;
}

u32 __cdecl Engine_DrawFrame() { return Renderer_DrawFrame(global<u8 *>(kWorldRender)); }

void __cdecl RendererFrame_MatrixAtExit() {}
}
