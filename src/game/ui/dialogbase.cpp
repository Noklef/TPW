#include "dialogbase.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/keymap.h"
#include "../world/world.h"
#include "focus.h"
#include "guidepth.h"
#include "guirect.h"
#include "window.h"
#include "windowfactory.h"
#include "windowreg.h"
#include "windowscript.h"

namespace {
const DialogKeyEnv kGameKeyEnv = {Window_SendMessage, reinterpret_cast<u8 *(__fastcall *)(const u8 *)>(GameTable7cfb90_Get1da71c_5194d0), reinterpret_cast<u32(__fastcall *)(u8 *)>((uintptr_t)0x0059ab50), Actions_FireDown, Actions_FireUp, Window7c24c8_Set, Window_CallDefault};
const DialogKeyEnv *g_keys = &kGameKeyEnv;

u8 *actions() { return *reinterpret_cast<u8 **>(static_cast<uintptr_t>(0x0078721c)); }

AddrCalls g_calls;

const u32 kGameOwner = 0x00786b68, kFirstButton = 0x009873c8;

u32 &global(u32 address) { return *reinterpret_cast<u32 *>(static_cast<uintptr_t>(address)); }
u32 slot(u8 *window, u32 index) { return g_calls.Th(0x0065e705, (u32)(uintptr_t)window, index); }
void setSlot(u8 *window, u32 index, u32 value) { g_calls.Th(0x0065e6d5, (u32)(uintptr_t)window, index, value); }

void boxButton(u8 *window, u32 index, u32 kind, u32 callback) {
    const u32 button = g_calls.Th(0x0065d7d0, (u32)(uintptr_t)window, kFirstButton + index);
    if (!kind) {
        g_calls.Th(0x0065d9dd, button, 0u);
        return;
    }
    const u32 impl = kind == 1 ? g_calls.Cd(0x00659bc9, 0x0074f9c4u) : kind == 2 ? g_calls.Cd(0x00659bc9, 0x0074f9ccu) : 0;
    g_calls.Th(0x0065d070, button, impl);
    g_calls.Th(0x0065d9dd, button, 1u);
    setSlot(window, index, callback);
}
}  // namespace

void Dialog_SetKeyEnv(const DialogKeyEnv *env) { g_keys = env ? env : &kGameKeyEnv; }
void Dialog_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

s32 __cdecl Dialog_KeyHandler(u8 *window, u32 message, u32 a, u32 b) {
    const u32 state = (b >> 16) & 0xff;
    if (message == 0x1000b) {
        if (a == 0x1b && !state) g_keys->send(window, 4, 0, 0);
        else if (a == 0x70 && !state) g_keys->helpKey(g_keys->advisor(*reinterpret_cast<const u8 **>(static_cast<uintptr_t>(0x007cf83c))));
        else g_keys->fireUp(actions(), nullptr, a, state);
        return 0;
    }
    if (message == 0x1000a) {
        g_keys->fireDown(actions(), nullptr, a, state);
        return 0;
    }
    if (message == 0x14) g_keys->setWindow(nullptr);
    return g_keys->callDefault(window, nullptr, message, a, b);
}

s32 __cdecl Dialog_BoxHandler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x14) {
        if (slot(window, 4) != 0xffffffffu) g_calls.Cd(slot(window, 4) ? 0x00486b70 : 0x00486b60);
        if (slot(window, 5) && !global(0x00786ba4) && !global(0x00f82884)) {
            global(0x00786ba4) = 1;
            g_calls.Th(0x00409300, kGameOwner);
            g_calls.Cd(0x00662420);
        }
        g_calls.Cd(0x00658ccc, g_calls.Th(0x0065d55a, (u32)(uintptr_t)window), 4u, 0u, 0u);
        g_calls.Cd(0x004862a0);
    } else if (message == 0x100 && a - kFirstButton < 3) {
        const u32 value = slot(window, 3);
        const u32 callback = slot(window, a - kFirstButton);
        if (!callback || reinterpret_cast<u32(__cdecl *)(u32)>(static_cast<uintptr_t>(callback))(value)) {
            g_calls.Cd(0x00658d9f, window, 4u, 0u, 0u);
            return 0;
        }
    }
    return static_cast<s32>(g_calls.Th(0x0065f66e, (u32)(uintptr_t)window, message, a, b));
}

u8 *__cdecl Dialog_Box(const u8 *record, u32 kind0, u32 callback0, u32 kind1, u32 callback1, u32 kind2, u32 callback2, u8 keysKept) {
    u8 *const window = reinterpret_cast<u8 *>(static_cast<uintptr_t>(g_calls.Cd(0x0047ed80, 0x0074f920u, 0x0047eed0u, 1u)));   // Dialog_Create
    if (!window) return nullptr;
    const u32 self = (u32)(uintptr_t)window;
    u8 text[0xc];   // (a string: made and destroyed, not otherwise used)
    g_calls.Th(0x00659d00, (u32)(uintptr_t)text);
    const u32 block = g_calls.Cd(0x0067acc0, 0x1a4u);
    g_calls.Th(0x0065d1c8, self, block ? g_calls.Th(0x0048f6a0, block, 0u) : 0u);   // TextPart_Ctor
    g_calls.Th(0x0065c428, self, 1u, 1u);
    g_calls.Th(0x0065c4b9, self, 1u, 1u);
    g_calls.Th(0x0065c388, self, g_calls.Cd(0x00485a70, *reinterpret_cast<const u32 *>(record + 8)));   // Language_Font
    const s8 *const colour = reinterpret_cast<const s8 *>(record + 4);
    g_calls.Th(0x0065c5d5, self, (s32)colour[0], (s32)colour[1], (s32)colour[2], 0xffu);
    g_calls.Th(0x0065f5b7, self, *reinterpret_cast<const u32 *>(record));
    boxButton(window, 0, kind0, callback0);
    boxButton(window, 1, kind1, callback1);
    boxButton(window, 2, kind2, callback2);
    setSlot(window, 3, *reinterpret_cast<const u32 *>(record + 0xc));
    if (keysKept) {
        setSlot(window, 4, 0xffffffffu);
    } else {
        setSlot(window, 4, g_calls.Cd(0x00486b80) & 0xff);   // Global7c24d0_GetByte
        g_calls.Cd(0x00486b70);   // KeyMaps_ResetAllAndSetFlag
    }
    setSlot(window, 5, 0);
    if (global(0x00786ba4) == 1 && !global(0x00f82884)) {
        g_calls.Th(0x004092a0, kGameOwner, 0u, 0u);   // GameOwner_Pause
        global(0x00786ba4) = 0;
        g_calls.Cd(0x00662411);
        setSlot(window, 5, 1);
    }
    g_calls.Th(0x00659d90, (u32)(uintptr_t)text);
    return window;
}

s32 __cdecl Dialog_BaseHandler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x14) {
        Window_PostMessage(reinterpret_cast<u8 *>(static_cast<uintptr_t>(Fn_65d55a(window))), 4, 0, 0);
        Focus_Restore();
    }
    return Window_CallDefault(window, nullptr, message, a, b);
}

u8 *__cdecl Dialog_Create(u8 *script, u32 handler, u32 shaded) {
    u8 *const frame = Window_CreateChild(*reinterpret_cast<u8 **>(static_cast<uintptr_t>(0x00faa5d8)), nullptr, 1, 1, 1, 0, 0, 0x7ff, 0x5ff);
    if (!frame) return nullptr;
    u8 *const window = WindowScript_Build(frame, nullptr, script, handler);
    if (!window) return nullptr;
    s32 depth;
    if (shaded) {
        depth = Gui_TopDepth(nullptr, nullptr);
        Window_SetImpl(frame, nullptr,
                       reinterpret_cast<u8 *>(static_cast<uintptr_t>(GuiWindows_FindByName(reinterpret_cast<const char *>(static_cast<uintptr_t>(0x0074f9bc))))));
    } else {
        u8 rect[0xc];
        GuiRect_Ctor(rect);
        Window_GetRect(window, nullptr, rect, 1);
        depth = Gui_TopDepth(nullptr, rect);
        GuiRect_Dtor(rect);
    }
    Window_SetDepth(frame, nullptr, static_cast<u32>(depth + 1));
    if (handler) Window_SetHandler(window, nullptr, handler);
    Window_SetFocus(window, nullptr, 0);
    return window;
}
}
