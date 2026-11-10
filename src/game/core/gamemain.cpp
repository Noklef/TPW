#include "gamemain.h"

#include <cstdint>

#include "gameheap.h"
#include "string.h"
#include "../online/chatlists.h"
#include "../online/chatcommand.h"
#include "../online/chat.h"
#include "../ui/windowproc.h"

namespace {

AddrCalls g_calls;

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const u32 kLanguage = 0x0078b530;
const u32 kStopped = 2, kStarted = 4;

u32 &Flags() { return gWindowproc_7a1a14; }

// new UserSlots: the constructor's answer, or 0 with no block.
u32 MakeSlots() {
    void *const block = GameHeap_Alloc(0x110);
    const u32 slots = block ? g_calls.Th(0x005c7420, (u32)(uintptr_t)block) : 0;   // UserSlots_Ctor
    reinterpret_cast<u32 &>(gChatlists_f81ebc) = slots;
    return slots;
}

// Everything once, before the first frame: false when one fails (bit 2 set).
bool StartGame() {
    const auto stop = [] {
        Flags() |= kStopped;
        return false;
    };
    if (g_calls.Th(0x00419710, kLanguage) == 0) return stop();   // Language_Load
    if ((u8)g_calls.Cd(0x0045f7a0) == 0) return stop();   // GamePaths_CheckDisc
    g_calls.Th(0x00408470, 0x00786b68u);   // NamedObjects_AddFixedFolders
    g_calls.Cd(0x00449eb0);   // SinTable_BuildGame
    g_calls.Cd(0x00449ec0);   // Screen_Init
    g_calls.Th(0x0041a5a0, kLanguage, global<u32>(0x007a1968));   // Language_SetFontSet
    if (g_calls.Cd(0x0044e080) == 0) return stop();   // GameWindow_Create
    g_calls.Cd(0x0054dcf0);   // Game_Start
    Flags() |= kStarted;
    return true;
}

void Frame() {
    if (!(Flags() & kStarted) && !StartGame()) return;
    if (reinterpret_cast<u32 &>(gChatcommand_f7d88c) == 0) {
        void *const block = GameHeap_Alloc(0x404);
        reinterpret_cast<u32 &>(gChatcommand_f7d88c) = block ? g_calls.Th(0x005b50b0, (u32)(uintptr_t)block) : 0;   // Online_Construct
    }
    g_calls.Th(0x005b5bf0, reinterpret_cast<u32 &>(gChatcommand_f7d88c));   // Online_Poll (the session in ECX)
    Flags() |= g_calls.Cd(0x0054e360);   // Game_RunFrame
}

}  // namespace

void GameMain_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" u32 __stdcall Game_WinMain(u32 instance, u32 previous, u32 commandLine, u32 show) {
    u32 message[7] = {};   // MSG
    if ((u8)g_calls.Cd(0x0045f640) == 1) {   // Startup_CheckCanRun
        global<u32>(0x007a0eb0) = previous;
        global<u32>(0x007a1a44) = instance;
        global<u32>(0x007a1614) = commandLine;
        global<u32>(0x007a0eac) = show;
        g_calls.Cd(0x005f86b0, instance);   // App_SetInstance
        char folder[0x104];
        g_calls.Cd(0x005f5690, g_calls.Cd(0x0067b9f0, folder, 0x104u));   // Dir_SetBase of the C library's getcwd
        u32 result;
        g_calls.Cd(0x005c0c10, &result);   // GamePaths_SetRoots
        g_calls.Cd(0x005c0d00, &result);   // Registry_LoadLanguage
        g_calls.Cd(0x005c2ce0, &result, g_calls.Th(0x005c5520, 0x00f816c0u));   // NamedObjects_AddSubfolders of Obj_Self
        g_calls.Cd(0x0045b1f0);   // Options_LoadBalanceFiles
        if (const u32 slots = reinterpret_cast<u32 &>(gChatlists_f81ebc)) {
            g_calls.Th(0x005c74b0, slots);
            GameHeap_Free(reinterpret_cast<void *>((uintptr_t)slots));
        }
        reinterpret_cast<u32 &>(gChatlists_f81ebc) = 0;
        g_calls.Th(0x005c7590, MakeSlots());
        if (const u32 online = reinterpret_cast<u32 &>(gChatcommand_f7d88c)) {
            g_calls.Th(0x005b5920, online);   // Online_Dtor
            GameHeap_Free(reinterpret_cast<void *>((uintptr_t)online));
        }
        reinterpret_cast<u32 &>(gChatcommand_f7d88c) = 0;
        g_calls.Cd(0x00449df0);   // Options_LoadStartup
        Flags() |= 1;
        g_calls.Cd(0x0045ba20);   // GlobalMem_InitUnusedFlag
        String path;
        String_CtorStr(&path, nullptr, reinterpret_cast<const char *>((uintptr_t)0x0074d12c));   // "save\config.tcf"
        g_calls.Th(0x00424930, 0x0078d8d8u, &path);   // Options_LoadFile
        String_Dtor(&path);
        global<u32>(0x007a1c5c) = global<u32(__stdcall *)(u32)>(0x006fd350)(0x0074d11c);   // RegisterWindowMessageA ("MSWHEEL_ROLLMSG")

        if (!(reinterpret_cast<u8 &>(gWindowproc_7a1a14) & kStopped)) {
            const auto peek = global<u32(__stdcall *)(u32 *, u32, u32, u32, u32)>(0x006fd354);
            const auto translate = global<u32(__stdcall *)(const u32 *)>(0x006fd358);
            const auto dispatch = global<u32(__stdcall *)(const u32 *)>(0x006fd35c);
            bool quit = false;
            do {
                while (!quit && peek(message, 0, 0, 0, 0)) {
                    if (global<u32(__stdcall *)(u32 *, u32, u32, u32)>(0x006fd360)(message, 0, 0, 0) == 0) {   // GetMessageA
                        quit = true;
                        break;
                    }
                    translate(message);
                    dispatch(message);
                }
                if (!quit && (Flags() & 1)) Frame();
            } while (!quit && !(reinterpret_cast<u8 &>(gWindowproc_7a1a14) & kStopped));
        }
    }

    g_calls.Cd(0x0051af90);   // Global8023d8_Release
    g_calls.Cd(0x0044a1d0);   // Screen_Shutdown
    g_calls.Cd(0x005503f0);   // Game_Shutdown
    u32 slots = reinterpret_cast<u32 &>(gChatlists_f81ebc);
    if (!slots) slots = MakeSlots();
    // (the original then makes them again if none: not reachable, the read of +0x60 faults first)
    if (*reinterpret_cast<s32 *>((uintptr_t)slots + 0x60) != -1) {
        g_calls.Th(0x005c8650, slots);
        if (const u32 chat = reinterpret_cast<u32 &>(gChat_7cddec)) {
            g_calls.Th(0x004e5950, chat);   // ChatClient_Dtor
            GameHeap_Free(reinterpret_cast<void *>((uintptr_t)chat));
        }
        reinterpret_cast<u32 &>(gChat_7cddec) = 0;
    }
    g_calls.Cd(0x0044e560);   // Game_ShutdownEngine
    g_calls.Th(0x00405750, 0x00785de8u);   // GameError_Show
    g_calls.Th(0x0041a5f0, kLanguage);   // Language_FreeFonts
    return message[2];
}
