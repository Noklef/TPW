// The handler of the guests' list: see dialog7ca13chandler.h.
#include "dialog7ca13chandler.h"

#include <cstdint>

#include "../core/funnytime.h"
#include "../core/language.h"
#include "../world/camera.h"
#include "../world/person.h"
#include "../world/things.h"
#include "dialog7ca098.h"
#include "dialog7ca150.h"
#include "dialog7ca168.h"
#include "dialog7cb398.h"
#include "dialogbase.h"
#include "hoverlabel.h"
#include "panel7cc408.h"
#include "window.h"
#include "window7059f0.h"
#include "window7059f0texts.h"
#include "dialog7ca13c.h"
#include "../world/world.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T field(const u8 *object, u32 offset) { return *reinterpret_cast<const T *>(object + offset); }

const Dialog7ca13cHandlerEnv kGameEnv = {&Window_SendMessage, &Window7059f0_NthItemData, &Visitor_TimeInPark, &FunnyTime_SpanOf, &Window7059f0_SetItemTexts, &Language_WideText, &HoverLabel_Attach, &Game_CancelAdvisor,
                                         &Dialog7cb398_Create, &Dialog7ca168_Open, &Dialog7ca150_Open, &Thing_GetHandle, &Camera_CentreOnThing, &Panel7cc408_Show, &Dialog_KeyHandler};
const Dialog7ca13cHandlerEnv *g_env = &kGameEnv;

void closeDialog() { g_env->send(gDialog7ca13c_7ca13c, 4, 0, 0); }

void updateRows() {
    u8 *const list = Window_FindChild(gDialog7ca13c_7ca13c, nullptr, 0x1e497);
    for (s32 row = field<s32>(list, 0x150) - 1; row >= 0; --row) {
        u32 data;
        g_env->nth(list, nullptr, row, &data);
        const u8 *const guest = reinterpret_cast<const u8 *>((uintptr_t)data);
        const u32 ticks = g_env->timeInPark(guest) << 2;
        FileTime span;
        const FileTime *const s = g_env->span(gWorldPtr + 0x2a0, nullptr, &span, ticks);
        const long long hours = (long long)(((unsigned long long)s->high << 32) | s->low) / 36000000000LL;   // (_alldiv: an hour of ticks)
        const u32 left = field<u32>(guest, 0x30);
        const u32 happy = (u32)(long long)field<float>(guest, 0x19c) & 0xff;   // (FLD, __ftol's low byte)
        const u32 texts[6] = {field<u32>(guest, 0x1d8), field<u32>(guest, 0x1a0), (u32)hours, field<u32>(guest, 0x1c4), left ? left - 1 : 999, (u32)((s32)(happy << 10) / 100)};
        g_env->setTexts(list, nullptr, row, texts, 0x3f, row == 0, 0xffffffff);
    }
}

}  // namespace

void Dialog7ca13cHandler_SetEnv(const Dialog7ca13cHandlerEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" s32 __cdecl Dialog7ca13c_Handler(u8 *window, u32 message, u32 a, u32 b) {
    switch (message) {
        case 5:
            closeDialog();
            return 1;
        case 0x10:
            if (a == 0x80083 && gDialog7ca13c_7ca13c) updateRows();
            break;
        case 0x14:
            gDialog7ca13c_7ca13c = nullptr;
            g_env->cancelAdvisor(0x11e);
            break;
        case 0x15:
            g_env->attach(window, 0x1e498, static_cast<u32>(reinterpret_cast<uintptr_t>(g_env->text(0x70))));
            break;
        case 0x100:
            switch (a) {
                case 0xfffffffe:
                    closeDialog();
                    return 0;
                case 0x1e499:
                    g_env->create7cb398();
                    return 0;
                case 0x1e49a:
                    g_env->open7ca168(0xffffffff, 0);
                    return 0;
                case 0x1e49b:
                    g_env->open7ca150(-1, 0);
                    return 0;
            }
            break;
        case 0x400: {
            u8 *const child = Window_FindChild(window, nullptr, a);
            if (child && b != 0xffffffff) {
                u32 data;
                g_env->nth(child, nullptr, static_cast<s32>(b), &data);
                g_env->show(reinterpret_cast<u8 *>((uintptr_t)0x007cc408), nullptr, reinterpret_cast<u8 *>((uintptr_t)data));
            }
            return 0;
        }
        case 0x402: {
            u8 *const child = Window_FindChild(window, nullptr, a);
            if (child && b != 0xffffffff) {
                u32 data;
                g_env->nth(child, nullptr, static_cast<s32>(b), &data);
                u16 out;
                const u16 *const handle = g_env->handle(reinterpret_cast<const u8 *>((uintptr_t)data), nullptr, &out);
                g_env->camera((static_cast<u32>(reinterpret_cast<uintptr_t>(handle)) & 0xffff0000) | *handle);   // sic: only AX loaded, the pointer's high half stays
                closeDialog();
                return 0;
            }
            break;
        }
        case 0x406:
            reinterpret_cast<u32 &>(gDialog7ca13c_7508bc) = b;
            break;
    }
    return g_env->keys(window, message, a, b);
}
