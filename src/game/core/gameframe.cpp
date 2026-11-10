// The game's step each frame: see gameframe.h.
#include "gameframe.h"

#include <cstdint>
#include <cstring>

#include "../gfx/screengrab.h"
#include "../math/x87.h"
#include "crt.h"
#include "gameheap.h"
#include "string.h"
#include "../online/chatlists.h"
#include "../online/chatcommand.h"
#include "../online/chat.h"
#include "../ui/dialog74fb20.h"
#include "../ui/windowproc.h"
#include "../ui/panel7b9940.h"
#include "../render/renderer.h"
#include "../render/graphicssettings.h"
#include "../gfx/texcache.h"
#include "../coaster/arreplayaction.h"
#include "../world/thingmsg.h"

namespace {

const GameFrameCall *g_table = nullptr;
u32 g_count = 0;

template <typename T>
T &G(u32 address) {
    return *reinterpret_cast<T *>((uintptr_t)address);
}

template <typename T>
T &at(u32 base, u32 offset) {
    return G<T>(base + offset);
}

u32 Target(u32 address) {
    for (u32 i = 0; i < g_count; ++i)
        if (g_table[i].address == address) return g_table[i].target;
    return g_addressTarget ? g_addressTarget(address) : address;   // (addrcalls.h)
}

// A cdecl call; a thiscall one on object (an address or a pointer); a stdcall one (the C library's array iterators).
template <typename... A>
u32 Cd(u32 address, A... args) {
    return reinterpret_cast<u32(__cdecl *)(A...)>((uintptr_t)Target(address))(args...);
}
template <typename... A>
u32 Th(u32 address, u32 object, A... args) {
    return reinterpret_cast<u32(__fastcall *)(u32, void *, A...)>((uintptr_t)Target(address))(object, nullptr, args...);
}
template <typename... A>
u32 Sd(u32 address, A... args) {
    return reinterpret_cast<u32(__stdcall *)(A...)>((uintptr_t)Target(address))(args...);
}
u32 Ptr(const void *p) { return (u32)reinterpret_cast<uintptr_t>(p); }

constexpr u32 kClocks = 0x00785970, kPanel = 0x007b51f0, kOwner = 0x00786b68, kLoadScreen = 0x008c4748, kCameraMode = 0x00f7b560, kPaths = 0x00f816c0, kState = 0x0087906c;

u32 Now() { return Th(0x00402d70, kClocks); }            // Clocks_Now
u32 Ticks() { return G<u32(__stdcall *)()>(0x006fd1d0)(); }   // timeGetTime
bool Key(s32 key) { return G<u16(__stdcall *)(s32)>(0x006fd320)(key) != 0; }   // GetAsyncKeyState, its word
void Say(u32 text) { G<void(__stdcall *)(u32)>(0x006fd1e0)(text); }   // OutputDebugStringA
u32 Bits(float f) {
    u32 b;
    std::memcpy(&b, &f, 4);
    return b;
}

// The setting at 0xfb3b7c, a function's static: made on the first use.
u32 Mode() {
    u8 &made = G<u8>(0x00fb34a8);
    if (!(made & 1)) {
        made |= 1;
        Th(0x00550ca0, Ptr(&gDialog74fb20_fb3b7c));   // Setting_ReadMode
        Cd(0x0067a7d0, 0x005d1750);  // atexit
    }
    return gDialog74fb20_fb3b7c;
}

// The singletons, made on the first use wherever they are asked for (a failed allocation leaves null, which the next use faults on or asks again for).
u32 Single(u32 slot, u32 size, u32 ctor) {
    u32 &p = G<u32>(slot);
    if (!p) {
        void *const block = GameHeap_Alloc(size);
        p = block ? Th(ctor, Ptr(block)) : 0;
    }
    return p;
}
u32 Chat() { return Single(reinterpret_cast<uintptr_t>(&gChat_7cddec), 0xf0c, 0x004e5260); }      // OnlineChat_Ctor
u32 Online() { return Single(reinterpret_cast<uintptr_t>(&gChatcommand_f7d88c), 0x404, 0x005b50b0); }
u32 Records() { return Single(reinterpret_cast<uintptr_t>(&gChatlists_f81ebc), 0x110, 0x005c7420); }
u32 Saves() { return Single(reinterpret_cast<uintptr_t>(&gArreplayaction_f7c58c), 0x50, 0x005aef50); }      // GlobalSave_Construct
u32 Centre() {
    u32 &p = G<u32>(0x00788d3c);
    if (!p) {
        void *const block = GameHeap_Alloc(0x244);
        if (block) Sd(0x0067acf0, block, 0x14, 0x1d, 0x00405460, 0x0040f970);   // the C library's array constructor
        p = Ptr(block);
    }
    return p;
}

bool Stopped() { return (u16)Cd(0x006595a8) != 0 || Key(0x1b) || Key(0x20); }

void LoadText() {
    const u32 text = Th(0x0041a820, 0x0078b530);   // Language_Buffer, (its result and the two after it are the call's)
    Th(0x00587380, kLoadScreen, text, 0x00764fe4, 0x1f4);
}

void Particles() {
    Cd(0x0051faa0, 0, G<u32>(0x007858c8), 0x800);   // Particles_Init
    Cd(0x0051fd80, G<u32>(0x0078d8e0) == 0 ? 1 : 0);   // Particles_SetReduced
    Cd(0x0051f370, Cd(0x0044a280, 0x007651b8, 1, 0));   // ParticleLib_Load of Quickload_Locate
}

void Post(u32 kind) {   // a ThingMsg13 (0x40f630; vtable 0x6fd810) to the message centre
    const u32 message[3] = {(u32)(uintptr_t)ThingMsg13_vtable, 0, kind};
    Th(0x0040fb10, Centre(), message);
}

void Shift(u32 first, u32 value) {   // the last eight times, a new one in
    for (u32 i = 0; i < 7; ++i) G<u32>(first + 4 * i) = G<u32>(first + 4 * i + 4);
    G<u32>(first + 0x1c) = value;
}

u32 Mean(u32 first) {
    u32 sum = 0;
    for (u32 i = 0; i < 8; ++i) sum += G<u32>(first + 4 * i);
    return sum >> 3;
}

// A step timed: its start kept at slot, then what it took there and at the end of the times.
template <typename F>
void Timed(u32 slot, u32 times, F step) {
    G<u32>(slot) = Ticks();
    step();
    const u32 took = Ticks() - G<u32>(slot);
    G<u32>(slot) = took;
    Shift(times, took);
}

void ResetClocks() {
    G<u32>(0x00878c74) = Now();
    G<u32>(0x0087879c) = Now();
    G<u32>(0x00878a1c) = Now();
}

// fild n (signed); fmul f.
u32 ScaledSigned(s32 ticks, u32 factor) {
    const float &f = G<const float>(factor);
    const double r = X87_MulInt(ticks < 0 ? 0u - (u32)ticks : (u32)ticks, f);
    return Bits((float)(ticks < 0 ? -r : r));
}

void Logo(u32 next) {   // states 6 and 8
    Th(0x004813b0, kPanel);
    if (Cd(0x0051b300)) {
        G<u32>(kState) = next;
    } else if (Stopped()) {
        Cd(0x0051b350);
        G<u32>(kState) = next;
    }
}

void Frontend() {   // state 1
    char line[0x104];
    LoadText();
    Cd(0x006591e3);   // Gui_CursorSlot3
    Cd(0x00550950);   // WeatherFx_ReloadEffects
    Cd(0x00462870, 1);   // WorldVerts_Alloc
    Th(0x00402ed0, kClocks);   // GameClock_Unfreeze
    Th(0x00481400, kPanel);
    Crt().sprintf(line, reinterpret_cast<const char *>(0x0074d0c4), at<const char *>(Th(0x00619240, kPaths), 8));
    Cd(0x00429ba0, line, 0, 0, 0);   // ModelSlot_LoadAdvisor
    Cd(0x005989c0);   // Advisor_Init
    Particles();
    void *const block = GameHeap_Alloc(0x38);
    const u32 object = block ? Th(0x005d5770, Ptr(block)) : 0;   // Class7029f0_Ctor
    G<u32>(0x00f82884) = object;
    Th(0x005d5970, object);
    Th(0x00409300, kOwner);
    G<u32>(0x00786ba4) = 0;
    Cd(0x00540900, 0, 1);   // Sprites_Init
    G<u32>(kState) = 2;
    Th(0x005879c0, kLoadScreen);   // LoadScreen_Close
    Cd(0x00659201);   // Gui_CursorSlot2
    Cd(0x0051e6f0, 0);
    const u32 now = Now();
    G<u32>(0x008786bc) = now;
    G<u32>(0x00878c74) = now;
}

void FrontendFrame() {   // state 2
    if (at<u32>(G<u32>(0x00f82884), 0x14) == 1) {
        Cd(0x00473440);   // Motion_UpdateClocks
        Cd(0x0051bd70);   // Sound_ApplySettings
        Cd(0x00599880);   // Advisor_UpdateSpeech
        Cd(0x00429df0, 0);   // ModelSlot_Animate
        Cd(0x00599800);
        Cd(0x0051c1d0, 0, 0, 0xc2480000, 0, 0, 0x3f800000, 0, 0x3f800000, 0);   // Sound_SetPlacement
        Th(0x004813b0, kPanel);
        Cd(0x0044e410, 3);
        Cd(0x0044e510);   // Frame_Begin
        Cd(0x0044e540);
        Th(0x005d5c50, G<u32>(0x00f82884));
        Th(0x005d5cc0, G<u32>(0x00f82884));
        Cd(0x0044e550);
        const s32 now = (s32)Now();
        s32 last = G<s32>(0x00878c74);
        G<s32>(0x008786bc) = now;
        if ((s32)((u32)now - (u32)last) > 0x1f4) {
            last = (s32)((u32)now - 0x1f4);
            G<s32>(0x00878c74) = last;
        }
        while (G<s32>(0x008786bc) > last) {
            G<s32>(0x00878c74) = (s32)((u32)last + 0x1f);
            Cd(0x00520130);   // Particles_Tick
            last = G<s32>(0x00878c74);
        }
    }
    if (at<u32>(G<u32>(0x00f82884), 0x14) != 1) G<u32>(kState) = 3;
}

void FrontendEnd() {   // state 3
    const u32 answer = Th(0x005d5cf0, G<u32>(0x00f82884));   // Class7029f0_Close
    const u32 object = G<u32>(0x00f82884);
    if (object) {
        Th(0x005d5840, object);   // Class7029f0_Dtor
        GameHeap_Free(reinterpret_cast<void *>((uintptr_t)object));
    }
    G<u32>(0x00f82884) = 0;
    Th(0x00481410, kPanel);
    Cd(0x00598ad0);
    Cd(0x00429e20, 0);   // ModelSlot_Free
    Th(0x0058dd60, gTexcache_8c4b44, 0);   // TexCache_ReleaseItems
    Th(0x00576820, reinterpret_cast<u32 &>(gRenderer_87a6d0));   // Renderer_ClearAll
    Cd(0x0051bcb0);   // Sound_Post700b60
    Cd(0x0051bd70);
    LoadText();
    G<u32>(0x008bd508) |= 1;
    Th(0x005765c0, reinterpret_cast<u32 &>(gRenderer_87a6d0), 0x70);   // Renderer_ClearSlot
    Th(0x00578be0, 0x0087b110, 0x3dcccccd, 0x447a0000, 0x42b40000);   // Mat4_Perspective
    Th(0x0056b790, G<u32>(0x00879d54), 0x42b40000);   // ViewQuad_SetFov
    Th(0x005c5560, kPaths, 0x00877f68);
    {
        String path;
        String_CtorStr(&path, nullptr, reinterpret_cast<const char *>(0x00747810));
        Th(0x005c5580, kPaths, &path);   // Strings5_JoinPath
        String_Dtor(&path);
    }
    G<u32>(0x008bcbc8) &= 0xffffffdf;
    Cd(0x004628a0);   // WorldVerts_Free
    Cd(0x0051fd20);   // Globals816d40_Free
    Cd(0x00541ed0);   // Sprites_FreeAll
    G<u32>(kState) = answer != 2 ? 0xc : 9;
}

void WaitForChat() {   // state 9, mode 1
    Say(0x00765190);
    const u32 start = Cd(0x005f5fa0);   // Time_GetMs
    Th(0x004e5be0, Chat());
    for (;;) {
        Th(0x004e6560, Chat());   // ChatAvatar_PumpEvents
        if (at<u8>(Chat(), 0x28) == 1) {
            Th(0x004e5ef0, Chat());
            Say(0x0076512c);
            return;
        }
        if ((u8)Th(0x004e6150, Chat())) {   // OnlineChat_Get2a
            G<u32>(kState) = 0xb;
            return;
        }
        if (Cd(0x005f5fa0) - start > 0x493e0) {
            Th(0x004e6130, Chat());   // OnlineChat_Set29And2a
            G<u32>(kState) = 0xb;
            Say(0x007650ec);
            return;
        }
    }
}

u32 ParkLoad() {   // state 9
    u32 result = 0;
    if (at<u32>(Online(), 0x3f0)) {
        const u32 key = at<u32>(Th(0x005b96b0, Online()), 0x4c);   // Obj_Get3a4
        const u32 name = Th(0x005bd270, at<u32>(Online(), 0xf8), key);   // Owner2c_ValueByKey
        String folder;
        String_CtorStr(&folder, nullptr, reinterpret_cast<const char *>((uintptr_t)name));
        Th(0x00409480, kOwner, &folder);
        String_Dtor(&folder);
        G<u32>(0x00786ba4) = 0;
    } else {
        G<u32>(0x00786ba4) = 1;
    }
    {
        String folders;
        String_CtorStr(&folders, nullptr, reinterpret_cast<const char *>((uintptr_t)Cd(0x0040ef40)));
        Th(0x004088a0, kOwner, &folders);   // NamedObjects_AddFolders
        String_Dtor(&folders);
    }
    LoadText();
    Cd(0x006591e3);
    Cd(0x00551600);   // RSSE_Init
    Cd(0x0055a5a0);   // FlyingRides_Init
    Particles();
    if (!(gGraphicssettings_7a1a8c & 0x200)) {
        if (at<s32>(Records(), 0x60) == -1) {
            const u32 name = Th(0x005b6480, kOwner);   // Online_String14
            if (!Th(0x005c7d00, Records(), 0)) {   // Records24_IfUsed
                WString text;
                WString_CtorStr(&text, nullptr, reinterpret_cast<const u16 *>(0x007651ac));
                Th(0x005c7f40, Records(), 0, &text, 0);
                WString_Dtor(&text);
            }
            Th(0x005c83b0, Records(), 0);
            Th(0x004093d0, kOwner, name);
        }
        Cd(0x00550950);
    }
    Cd(0x0051ec50, Th(0x005c5520, kOwner));   // Sound_RegisterThemeCategories of Obj_Self
    Cd(0x0051bd70);
    Cd(0x0051e730);   // Sound_StartMusic
    if (!Cd(0x00457a90)) {
        result = 2;
    } else {
        Th(0x00576260, reinterpret_cast<u32 &>(gRenderer_87a6d0), 0x0054e2b0);   // Renderer_SetRestoreCallback
        if (!Th(0x00407d80, kOwner)) {   // NamedObjects_CreateWorld
            result = 2;
        } else {
            if (Mode() == 1)
                Th(0x00481420, kPanel);
            else
                Th(0x004813e0, kPanel);
            Cd(0x00540900, 0, 0);
            if (!Th(0x00407e00, kOwner, Cd(0x0040ef40)) || !Cd(0x00457c30)) {
                result = 2;
            } else {
                G<u32>(0x00879080) = 1;
                Cd(0x00458590, 1);   // LightSource_UpdateMain
                G<u32>(kState) = 0xf;
                G<u32>(0x00879088) = 0;
            }
        }
    }
    ResetClocks();
    G<u32>(0x00877d34) = 0;
    G<u32>(0x008786c0) = 0;
    if (Mode() == 1) WaitForChat();
    if (!G<u32>(0x00788268)) {
        if (Mode() == 1) {
            if (!(u8)Th(0x004e6150, Chat())) {
                u8 buffer[0x3d4];
                const u32 online = Online();
                Th(0x00414d40, 0x0078a450, Th(0x005b96a0, online), buffer, 1);   // SaveGame_Load of Obj_Get3e4
                Th(0x00404140, 0x00785a08, 0);
            }
        } else {
            Th(0x005accf0, kCameraMode);
        }
    }
    if (Mode() != 1 && (u8)Th(0x005b0cb0, Saves())) {   // GlobalSave_Byte4c
        Th(0x005b0cc0, Saves());   // GlobalSave_ClearByte4c
        if (Mode() == 2) {
            Post(10);
            Post(0);
        }
    }
    Th(0x005879c0, kLoadScreen);
    Cd(0x00659201);
    at<u32>(Online(), 0x3f0) = 0;
    Cd(0x0051e6f0, 0);
    return result;
}

void WorldStep() {   // state 0xa: every eighth step
    const u32 ran = G<u32>(0x00879064);
    G<u32>(0x00878a1c) = G<u32>(0x00878c74);
    if (ran < 3) {
        G<u32>(0x00879064) = ran + 1;
        if (Mode() == 0 || Mode() == 2) {
            Timed(0x00878688, 0x00878088, [] { Th(0x00516380, G<u32>(0x007cf83c)); });   // World_Tick
        } else if (Mode() == 1) {
            Th(0x005166b0, G<u32>(0x007cf83c));
            Th(0x004e6170, Chat());
        }
    }
    Cd(0x0055a470);   // RideScripts_SetReverb
}

void MusicLevels() {   // state 0xa: every 32nd step
    constexpr u32 kKind = 0x001da738;
    s32 count = (s32)Th(0x004c81e0, Th(0x00519510, G<u32>(G<u32>(0x007cf6ec))));   // ParkAnalyser_Kind1OnPlainHalf of GameTable7cfb90_Get1da720
    if (count > 0x59) count = 0x59;
    if (at<u32>(G<u32>(0x007cf83c), kKind) == 4) count = 0;
    Cd(0x0051e790, count);   // Sound_SetMusicLevel
    const u32 word = G<u16>(0x007b05cc);   // (pushed over a stale high word)
    const u32 table = Th(0x00519510, G<u32>(0x007cf83c));
    s32 around = (s32)Th(0x004c8d30, table, 1, word, 4, 0);   // Things_CountOfKindAround
    if (around > 0x64) around = 0x64;
    if (at<u32>(G<u32>(0x007cf83c), kKind) == 4) around = 0;
    Cd(0x0051e7b0, around);   // Sound_SetKidsLevel
    Cd(0x0055ab50);   // FlyingRide_SetBounds
}

void FixedSteps() {   // state 0xa
    s32 last = G<s32>(0x00878c74);
    while (G<s32>(0x008786bc) > last) {
        const u8 flags = reinterpret_cast<u8 &>(gWindowproc_7a1a14);
        G<s32>(0x00878c74) = (s32)((u32)last + 0x1f);
        ++G<u32>(0x00877d34);
        if ((flags & 8) || !(reinterpret_cast<u8 &>(gGraphicssettings_7a1a8c) & 1)) {
            Timed(0x00878694, 0x008780e8, [] { Cd(0x00520130); });
            Cd(0x00546c80);   // TrakCars_Update
            Timed(0x00878690, 0x008780c8, [] { Cd(0x005516b0); });   // RSSE_Tick
            if (!(G<u8>(0x00877d34) & 1)) {
                G<u32>(0x0087879c) = G<u32>(0x00878c74);
                Cd(0x0055abf0);   // FlyingRides_Update
                Timed(0x00878698, 0x00878108, [] { Cd(0x00475360); });   // SpriteScripts_Update
            }
            if (!(G<u8>(0x00877d34) & 7)) WorldStep();
            if (!(G<u8>(0x00877d34) & 0x1f)) MusicLevels();
        }
        last = G<s32>(0x00878c74);
    }
}

void SoundPlacement() {   // state 0xa
    const u32 xBits = G<u32>(0x007909c0), yBits = G<u32>(0x007909c4), zBits = G<u32>(0x007909c8);
    const u32 a0 = G<u32>(0x007909a0), a4 = G<u32>(0x007909a4), a8 = G<u32>(0x007909a8), b0 = G<u32>(0x007909b0), b4 = G<u32>(0x007909b4), b8 = G<u32>(0x007909b8);
    if (G<u8>(0x00790ab0) & 0x16) {
        Cd(0x0051c1d0, xBits, yBits, zBits, b0, b4, b8, a0, a4, a8);
        return;
    }
    float x, y, z;
    std::memcpy(&x, &xBits, 4);
    std::memcpy(&y, &yBits, 4);
    std::memcpy(&z, &zBits, 4);
    const float &lag = G<const float>(0x00700f88);
    const X87Reg dz = X87Reg(z) - G<const float>(0x00790ac0);
    const X87Reg dy = X87Reg(y) - G<const float>(0x00790abc);
    const X87Reg dx = X87Reg(x) - G<const float>(0x00790ab8);
    const X87Reg nz = z - (dz * lag);
    const X87Reg ny = y - (dy * lag);
    const X87Reg sx = dx * lag;
    const u32 zOut = Bits((float)nz);
    const X87Reg nx = x - sx;
    const u32 yOut = Bits((float)ny);
    Cd(0x0051c1d0, Bits((float)nx), yOut, zOut, b0, b4, b8, a0, a4, a8);
}

void Drawing() {   // state 0xa
    SoundPlacement();
    Cd(0x0051bd70);
    Cd(0x00599880);
    Th(0x004813b0, kPanel);
    Cd(0x00557ab0);   // Rsse_UpdateAll
    Th(0x00519060, G<u32>(0x007cf83c), ScaledSigned((s32)(G<u32>(0x008786bc) - G<u32>(0x00878c74)), 0x00700f8c));   // Things_PoseRides
    Cd(0x0055ce60, ScaledSigned((s32)(G<u32>(0x008786bc) - G<u32>(0x0087879c)), 0x00700f90));   // FlyingRides_LerpSlots
    Th(0x00518f90, G<u32>(0x007cf83c), ScaledSigned((s32)(G<u32>(0x008786bc) - G<u32>(0x00878a1c)), 0x00700f94));   // World_PlaceSprites
    Cd(0x00443340);   // CoasterEdit_Update
    Cd(0x00435a30);
    Cd(0x0044e410, 2);
    Cd(0x0042b1c0);   // Camera_Update
    Cd(0x00429df0, 0);
    Cd(0x00429df0, 1);
    Cd(0x00599800);
    const bool camera = (G<u32>(0x00790ab0) & 0x16) != 0;
    const u8 was = G<u8>(0x00878e84);
    if (camera ? was == 0 : was == 1) {
        G<u32>(0x008bcbc8) ^= 0x2080;
        Cd(0x0051c700, G<u32>(0x0078d908));   // Sound_SetCameraOcclusion
        G<u32>(0x0087502c) = 0x12c;
        G<u32>(0x00875028) = Ticks();
    }
    const u8 cameraByte = G<u8>(0x00790ab0);
    G<u8>(0x00878e84) = (cameraByte & 0x16) != 0;
    if (G<s32>(0x0087502c) > 0) {
        const u32 surface = at<u32>(G<u32>(0x00879a24), 0x14);
        u32 out;
        G<u32(__stdcall *)(u32, u32, u32 *)>(G<u32>(surface))(surface, 0x00707210, &out);   // QueryInterface
        Cd(0x0053e760, out);
        const u32 ticks = Ticks();
        G<u32>(0x0087502c) += G<u32>(0x00875028) - ticks;
        G<u32>(0x00875028) = Ticks();
    } else {
        G<u32>(0x0087868c) = Ticks();
        Cd(0x0044e540);
        const u32 took = Ticks() - G<u32>(0x0087868c);
        G<u32>(0x0087868c) = took;
        G<u32>(0x0087907c) += took;
    }
    Cd(0x00436490);   // Coasters_DropPendingPiece
}

void ParkFrame() {   // state 0xa
    if (G<u32>(0x00879070)) {
        ResetClocks();
        G<u32>(0x00877d34) = 0;
        G<u32>(0x008786c0) = 0;
        G<u32>(0x00879070) = 0;
    }
    if (gGameframe_878128)
        Th(0x00402ea0, kClocks, 0x20);   // GameClock_Freeze
    else
        Th(0x00402ed0, kClocks);
    Cd(0x00473440);
    const u32 now = Now();
    G<u32>(0x008786bc) = now;
    if ((s32)(now - G<u32>(0x00878c74)) > 0x7d0) G<u32>(0x00878c74) = now - 0x7d0;
    FixedSteps();
    if ((reinterpret_cast<u8 &>(gWindowproc_7a1a14) & 8) || !(reinterpret_cast<u8 &>(gGraphicssettings_7a1a8c) & 1)) Drawing();
    if (gGameframe_878128) ScreenGrab_NextName();
    G<u32>(0x00879064) = 0;
    G<u32>(0x0087868c) = Ticks();
    Cd(0x0044e550);
    const u32 ended = Ticks();
    const u32 s108 = Mean(0x00878108), se8 = Mean(0x008780e8), sc8 = Mean(0x008780c8);
    const u32 took = ended - G<u32>(0x0087868c);
    G<u32>(0x0087868c) = took;
    const u32 total = (G<u32>(0x0087907c) += took);
    Shift(0x008780a8, total);
    Crt().sprintf(reinterpret_cast<char *>(0x00877eb8), reinterpret_cast<const char *>(0x007650b4), Mean(0x00878088), Mean(0x008780a8), sc8, se8, s108);
    const s32 ending = G<s32>(0x00879088);
    if (ending == 1)
        G<u32>(kState) = 0xd;
    else if (ending > 1 && ending <= 3)
        G<u32>(kState) = 0xb;
    if (Mode() == 1 && (u8)Th(0x004e6150, Chat())) G<u32>(kState) = 0xb;
}

void FreeChat() {
    const u32 chat = reinterpret_cast<u32 &>(gChat_7cddec);
    if (chat) {
        Th(0x004e5950, chat);
        GameHeap_Free(reinterpret_cast<void *>((uintptr_t)chat));
    }
    reinterpret_cast<u32 &>(gChat_7cddec) = 0;
}

void ParkEnd() {   // state 0xb
    if (Mode() != 1) Th(0x005ac610, kCameraMode, 0x00877c60);
    Cd(0x0048aff0);   // CameraMode_Leave
    Th(0x00576260, reinterpret_cast<u32 &>(gRenderer_87a6d0), 0);
    Th(0x0058dd60, gTexcache_8c4b44, 0);
    Th(0x00576820, reinterpret_cast<u32 &>(gRenderer_87a6d0));
    if (Mode() == 1)
        Th(0x00481430, kPanel);
    else
        Th(0x004813f0, kPanel);
    Th(0x00409180, kOwner);
    Cd(0x00457d50, 0);
    Cd(0x004752e0);   // SpriteScripts_Shutdown
    if (G<u32>(0x00879088) != 3 && (gGraphicssettings_7a1a8c & 0x200))
        G<u32>(kState) = at<u32>(Online(), 0x3f0) ? 9 : 1;
    else
        G<u32>(kState) = 0xc;
    Cd(0x0051bcb0);
    Cd(0x0051e7b0, 0);
    Cd(0x0051e770);   // Sound_StopMusic
    Cd(0x0051e830, 0);   // Sound_SetAmbientLevel
    Cd(0x0051ee40);   // Sound_ReleaseThemeCategories
    Cd(0x0051fd20);
    Cd(0x005584b0);   // RSSE_CheckLeaks
    Cd(0x0055d4d0);   // FlyingRides_Shutdown
    if (Mode() != 1) return;
    FreeChat();
    if (at<u32>(Online(), 0x3f0)) return;
    const u32 set = (u8)Th(0x005afc40, Saves()) ? 2 : 0;   // GlobalSave_Byte24
    Mode();
    Th(0x00550d80, Ptr(&gDialog74fb20_fb3b7c), set);   // Setting_SetMode
}

void FreeOf(u32 slot, u32 dtor) {
    const u32 p = G<u32>(slot);
    if (p) {
        Th(dtor, p);
        GameHeap_Free(reinterpret_cast<void *>((uintptr_t)p));
    }
    G<u32>(slot) = 0;
}

void Shutdown() {   // state 0xc
    if (at<s32>(Records(), 0x60) != -1) {
        Th(0x005c8650, Records());
        FreeChat();
    }
    Cd(0x0051eda0);   // Sound_ReleaseGlobalCategories
    gWindowproc_7a1a14 |= 2;
    const u32 centre = G<u32>(0x00788d3c);
    if (centre) {
        Sd(0x0067afa0, centre, 0x14, 0x1d, 0x0040f970);   // the C library's array destructor
        GameHeap_Free(reinterpret_cast<void *>((uintptr_t)centre));
    }
    G<u32>(0x00788d3c) = 0;
    FreeOf(reinterpret_cast<uintptr_t>(&gArreplayaction_f7c58c), 0x005af250);
    FreeOf(reinterpret_cast<uintptr_t>(&gChatcommand_f7d88c), 0x005b5920);
    FreeOf(reinterpret_cast<uintptr_t>(&gChatlists_f81ebc), 0x005c74b0);
    const u32 mouse = reinterpret_cast<u32 &>(gPanel7b9940_7b05e8);
    if (mouse) {
        Th(0x0046c350, mouse, 0);   // MouseMode_Set
        GameHeap_Free(reinterpret_cast<void *>((uintptr_t)mouse));
    }
    reinterpret_cast<u32 &>(gPanel7b9940_7b05e8) = 0;
    FreeChat();
}

}  // namespace

extern "C" {

void GameFrame_SetEnv(const GameFrameCall *table, u32 count) {
    g_table = table;
    g_count = count;
}

u32 __cdecl Game_RunFrame() {
    u32 result = 0;
    const u32 dt = Now() - G<u32>(0x00875034);
    G<u32>(0x00875030) = dt;
    if (G<u32>(0x007b05d0)) {
        const X87Reg rate = G<const float>(0x00700f84) / X87Reg((double)dt);   // (fild qword of it; fdivr)
        Th(0x0046c0d0, 0x007b05a8, Bits((float)rate));   // MouseState_ShowFps
    }
    G<u32>(0x00875034) = Now();
    if (Key(1))
        ++G<u32>(0x008785dc);
    else
        G<u32>(0x008785dc) = 0;
    if (Key(2))
        ++G<u32>(0x008786dc);
    else
        G<u32>(0x008786dc) = 0;
    if (!G<u32>(0x00879068)) return result;
    switch (G<u32>(kState)) {
    case 1: Frontend(); break;
    case 2: FrontendFrame(); break;
    case 3: FrontendEnd(); break;
    case 4: G<u32>(kState) = 5; break;
    case 5:
        Cd(0x006591e3);
        Th(0x004813b0, kPanel);
        if (!Stopped()) {
            Cd(0x0054df20);
            G<u32>(kState) = 6;
        }
        break;
    case 6: Logo(7); break;
    case 7:
        Th(0x004813b0, kPanel);
        if (!Stopped()) {
            Cd(0x0054e070);
            G<u32>(kState) = 8;
        }
        break;
    case 8: Logo(1); break;
    case 9: result = ParkLoad(); break;
    case 0xa: ParkFrame(); break;
    case 0xb: ParkEnd(); break;
    case 0xc: Shutdown(); break;
    case 0xd: G<u32>(kState) = 0xe; break;
    case 0xe:
        Cd(0x005996d0, 1);
        Th(0x005ac5f0, kCameraMode, 0x00878a68);
        G<u32>(kState) = 0xa;
        G<u32>(0x00879088) = 0;
        break;
    case 0xf:
        if (Mode() != 1) Cd(0x00550b30);
        G<u32>(kState) = 0xa;
        break;
    default: break;
    }
    return result;
}
}
