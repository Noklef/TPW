// The options dialog: see dialog7cb2fcopen.h.
#include "dialog7cb2fcopen.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/crt.h"
#include "../core/language.h"
#include "../core/options.h"
#include "../core/soundset.h"
#include "../core/vcall.h"
#include "../gfx/texcache.h"
#include "../render/particles.h"
#include "../render/renderer.h"
#include "dialog7cb2fc.h"
#include "focus.h"
#include "guimanager.h"
#include "hoverlabel.h"
#include "window.h"
#include "window705a50.h"
#include "windowscript.h"
#include "panel7b9940.h"
#include "../gfx/texcache.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T game(u32 address) { return reinterpret_cast<T>((uintptr_t)address); }

const Dialog7cb2fcOpenEnv kGameEnv = {Language_WideText, Dialog7cb2fc_SetValueRow, Dialog7cb2fc_SetToggle, Sound_Set803abc, Sound_Set803ac0, Sound_Set803ac4,
                                      game<void(__cdecl *)(u32)>(0x005996d0), Window_SendMessage, HoverLabel_Attach, Dialog7cb2fc_DressRow,
                                      Dialog7cb2fc_DressSlider, Focus_Restore, GuiManager_CallSlot3, GuiManager_CallSlot4,
                                      ParticlePool2_Finish, game<u32(__cdecl *)()>(0x00662420), Window705a50_SetState,
                                      game<void(__fastcall *)(u8 *, void *, u32, u32)>(0x004092a0), game<void(__fastcall *)(u8 *)>(0x00409300), Renderer_StateFlags,
                                      Renderer_SetStateFlags, TexCache_ReleaseItems, WindowScript_Build, Window_SetDepth, Window_CallDefault,
                                      game<void(__fastcall *)(u8 *)>(0x004237f0), Options_Revert};
const Dialog7cb2fcOpenEnv *g_env = &kGameEnv;

u8 *const kOptions = reinterpret_cast<u8 *>((uintptr_t)0x0078d8d8);
u16 *const kBuffer = reinterpret_cast<u16 *>((uintptr_t)0x007ba058);

u8 *dialog() { return gDialog7cb2fcopen_7cb2fc; }
u8 *child(u32 id) { return Window_FindChild(dialog(), nullptr, id); }
u8 *rowItem(u32 row) { return Window_FindChild(child(row), nullptr, 0x1d4c9); }
u32 option(u32 offset) { return global<u32>(0x0078d8d8 + offset); }

// The row's child gets the label's text with the text after it.
void textRow(u32 row, u32 label, u32 after) {
    u8 *const item = rowItem(row);
    u16 copy[0x40];   // sic: its last unit never set
    Crt().wcsncpy(copy, g_env->text(after), 0x3f);
    g_env->text(label);
    Crt().wcscat(kBuffer, copy);
    vcall<void>(item, 0x24, kBuffer);
}

// The text after a slider's label: the first's own of three, else the text 0.
u32 stepText(u32 value, u32 first) { return value < 3 ? first + value : 0; }

// The value of a volume slider: stored after the flag byte; shown -1 with the flag clear.
s32 flaggedValue(u32 offset, s32 value) {
    global<s32>(0x0078d8d8 + offset + 4) = value;
    return global<u8>(0x0078d8d8 + offset) ? value : -1;
}

u32 volumeOf(s32 value) { return value < 0 ? 0 : (u32)value; }

// The percentage shown by a slider of steps (0-2): the step * 100 / 2.
s32 stepPercent(u32 step) { return (s32)(step * 100) / 2; }

// The row with the value after its label: Dialog7cb2fc_SetValueRow, inlined in the original (and through the env's text here).
void valueRow(u32 id, u32 label, s32 value) {
    u8 *const row = child(id);
    u16 state[10];
    if (value < 0) Crt().swprintf(state, reinterpret_cast<const u16 *>(gDialog7cb2fc_7537c8), g_env->text(0x14e));
    else Crt().swprintf(state, reinterpret_cast<const u16 *>((uintptr_t)0x007537b8), value);
    g_env->text(label);
    Crt().wcscat(kBuffer, state);
    vcall<void>(Window_FindChild(row, nullptr, 0x1d4c9), 0x24, kBuffer);
    if (value >= 0) vcall<void>(row, 0x1c, value);
}

// The frame's window, or without a frame [0x7cb2ac].
u8 *frameWindow() {
    u8 *const frame = global<u8 *>(0x00f82884);
    return frame ? *reinterpret_cast<u8 **>(frame + 4) : gPanel7b9940_7cb2ac;
}

}  // namespace

void Dialog7cb2fcOpen_SetEnv(const Dialog7cb2fcOpenEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl Dialog7cb2fc_OnSlider(u32 id, s32 value) {
    switch (id) {
    case 0x1d4d5:
        Options_Set4(kOptions, nullptr, (u32)((s32)((u32)value * 2) / 100));
        textRow(id, 0x13f, stepText(option(4), 0x155));
        return;
    case 0x1d4d6:
        Options_Set8(kOptions, nullptr, (u32)((s32)((u32)value * (option(0) ? 2u : 1u)) / 100));
        textRow(id, 0x13e, stepText(option(8), 0x150));
        return;
    case 0x1d4e0:
        Options_Set30(kOptions, nullptr, (u32)value);
        valueRow(id, 0x140, value);
        return;
    case 0x1d4d8: {
        const s32 shown = flaggedValue(0x10, value);
        valueRow(id, 0x141, shown);
        g_env->soundAbc(volumeOf(shown));
        return;
    }
    case 0x1d4da: {
        const s32 shown = flaggedValue(0x18, value);
        valueRow(id, 0x142, shown);
        g_env->soundAc0(volumeOf(shown));
        return;
    }
    case 0x1d4dc: {
        const s32 shown = flaggedValue(0x20, value);
        valueRow(id, 0x143, shown);
        g_env->soundAc4(volumeOf(shown));
        return;
    }
    case 0x1d4de: g_env->valueRow(id, 0x144, flaggedValue(0x28, value)); return;
    default: return;
    }
}

void __cdecl Dialog7cb2fc_OnButton(u32 id, u32 state) {
    const u8 off = state != 1;
    switch (id) {
    case 0x1d4c1:
        global<u8>(0x0078d90c) = off;
        g_env->toggle(0x1d4cb, 0x145, off, 1, 0, 0);
        return;
    case 0x1d4c2:
        global<u8>(0x0078d90d) = off;
        g_env->toggle(0x1d4cc, 0x146, off, 1, 0, 0);
        return;
    case 0x1d4c3:
        global<u8>(0x0078d90e) = off;
        g_env->toggle(0x1d4cd, 0x147, off, 1, 0, 0);
        return;
    case 0x1d4c4:
        global<u8>(0x0078d90f) = off;
        g_env->toggle(0x1d4ce, 0x148, off, 1, 0, 0);
        return;
    case 0x1d4c5:
        global<u8>(0x0078d911) = off;
        g_env->toggle(0x1d4cf, 0x14b, off, 1, 0, 0);
        return;
    case 0x1d4c6: {
        const u8 on = !global<u8>(0x0078d912);
        global<u8>(0x0078d912) = on;
        textRow(0x1d4d0, 0x15f, 0x160 + on);
        return;
    }
    case 0x1d4c7: {
        const u8 on = !global<u8>(0x0078d910);
        global<u8>(0x0078d910) = on;
        textRow(0x1d4d1, 0x162, 0x163 + on);
        return;
    }
    case 0x1d4d2: {
        global<u32>(0x0078d8d8) = !option(0);
        const u32 label = option(0) ? 0x13c : 0x13d;
        const s32 percent = stepPercent(option(8));
        if (!(u16)vcall<u32>(child(0x1d4d6), 0x1c, percent)) Dialog7cb2fc_OnSlider(0x1d4d6, percent);
        u8 *const item = rowItem(0x1d4c8);
        g_env->text(label);
        vcall<void>(item, 0x24, kBuffer);
        return;
    }
    case 0x1d4d3:
        if (option(0xc)) Options_SetC(kOptions, nullptr, 0);
        else if (global<s32>(global<u32>(0x00879a24) + 0x788) > 1) Options_SetC(kOptions, nullptr, 1);
        textRow(0x1d4ca, 0x15c, 0x15d + (option(0xc) ? 1 : 0));
        return;
    case 0x1d4d7:
        global<u8>(0x0078d8e8) = off;
        Dialog7cb2fc_OnSlider(0x1d4d8, (s32)option(0x14));
        return;
    case 0x1d4d9:
        global<u8>(0x0078d8f0) = off;
        Dialog7cb2fc_OnSlider(0x1d4da, (s32)option(0x1c));
        return;
    case 0x1d4db:
        global<u8>(0x0078d8f8) = off;
        Dialog7cb2fc_OnSlider(0x1d4dc, (s32)option(0x24));
        return;
    case 0x1d4dd:
        global<u8>(0x0078d900) = off;
        Dialog7cb2fc_OnSlider(0x1d4de, (s32)option(0x2c));
        return;
    default: return;
    }
}

s32 __cdecl Dialog7cb2fc_Handler(u8 *window, u32 message, u32 a, s32 b) {
    switch (message) {
    case 0x14:
        gDialog7cb2fcopen_7cb2fc = nullptr;
        g_env->setFlags(gRenderer_87a6d0, nullptr, gDialog7cb2fc_7cb2f4);
        if (!global<u32>(0x00786ba4) && !gDialog7cb2fc_7cb2ec) {
            global<u32>(0x00786ba4) = 1;
            g_env->f409300(reinterpret_cast<u8 *>((uintptr_t)0x00786b68));
            g_env->f662420();
        }
        g_env->send(frameWindow(), 6, 1, 0);
        g_env->focus();
        if (global<u8>(0x0078d90e)) g_env->slot3();
        else g_env->slot4();
        g_env->particles();
        break;
    case 0x100:
        if (a == 0xfffffffeu) {
            g_env->revert(kOptions);
            g_env->send(dialog(), 4, 0, 0);
            return 0;
        }
        if (a == 0xffffffffu) {
            g_env->f4237f0(kOptions);
            return 0;
        }
        Dialog7cb2fc_OnButton(a, (u32)b);
        break;
    case 0x800: Dialog7cb2fc_OnSlider(a, b); break;
    case 0x801:
        switch (a) {
        case 0x1d4d8:
            if (global<u8>(0x0078d8e8)) g_env->soundAbc(volumeOf(b));
            break;
        case 0x1d4da:
            if (global<u8>(0x0078d8f0)) g_env->soundAc0(volumeOf(b));
            break;
        case 0x1d4dc:
            if (global<u8>(0x0078d8f8)) g_env->soundAc4(volumeOf(b));
            break;
        case 0x1d4de:
        case 0x1d4e0: break;
        default: gDialog7cb2fc_7cb300 = 0; break;
        }
        break;
    case 0x802: gDialog7cb2fc_7cb300 = 0; break;
    default: break;
    }
    return g_env->callDefault(window, nullptr, message, a, (u32)b);
}

void __cdecl Dialog7cb2fc_Open() {
    if (dialog()) return;
    Options_Keep(kOptions);
    gDialog7cb2fc_7cb300 = 0;
    if (global<u32>(0x00786ba4) == 1) {
        g_env->f4092a0(reinterpret_cast<u8 *>((uintptr_t)0x00786b68), nullptr, 0, 0);
        global<u32>(0x00786ba4) = 0;
        Fn_662411();
        gDialog7cb2fc_7cb2ec = 0;
    } else {
        gDialog7cb2fc_7cb2ec = 1;
    }
    g_env->f5996d0(1);
    gDialog7cb2fc_7cb2f4 = g_env->flags(gRenderer_87a6d0);
    g_env->setFlags(gRenderer_87a6d0, nullptr, 0x21);
    g_env->release(reinterpret_cast<u8 * &>(gTexcache_8c4b44), nullptr, 0);
    g_env->send(frameWindow(), 6, 0, 0);
    gDialog7cb2fcopen_7cb2fc = g_env->build(global<u8 *>(0x00faa5d8), nullptr, reinterpret_cast<u8 *>((uintptr_t)0x00752f30), 0x004a2bf0);
    const u32 title = (u32)(uintptr_t)g_env->text(0x13b);
    g_env->attach(dialog(), 0x1d4d4, title);

    u8 *const first = child(0x1d4d2);
    const u32 depth = (u32)(s32)*reinterpret_cast<const s16 *>(first + 0xe0) + 2;
    g_env->setDepth(first, nullptr, depth);
    static const u32 kDepthIds[] = {0x1d4d3, 0x1d4c1, 0x1d4c2, 0x1d4c3, 0x1d4c4, 0x1d4c5, 0x1d4c6, 0x1d4c7, 0x1d4d7, 0x1d4d9, 0x1d4db, 0x1d4dd, 0xffffffffu, 0xfffffffeu};
    for (const u32 id : kDepthIds) g_env->setDepth(child(id), nullptr, depth);
    static const u32 kRows[] = {0x1d4c8, 0x1d4ca, 0x1d4cb, 0x1d4cc, 0x1d4cd, 0x1d4ce, 0x1d4cf, 0x1d4d0, 0x1d4d1};
    for (const u32 id : kRows) g_env->dressRow(id);
    g_env->dressSlider(0x1d4d5, (u32)stepPercent(option(4)));
    g_env->dressSlider(0x1d4d6, (u32)stepPercent(option(8)));
    g_env->dressSlider(0x1d4e0, option(0x30));
    g_env->dressSlider(0x1d4d8, option(0x14));
    g_env->dressSlider(0x1d4da, option(0x1c));
    g_env->dressSlider(0x1d4dc, option(0x24));
    g_env->dressSlider(0x1d4de, option(0x2c));

    {
        u8 *const item = rowItem(0x1d4c8);
        g_env->text(option(0) ? 0x13c : 0x13d);
        vcall<void>(item, 0x24, kBuffer);
    }
    textRow(0x1d4ca, 0x15c, !option(0xc) ? 0x15d : (global<s32>(global<u32>(0x00879a24) + 0x788) > 1 ? 0x15e : 0x15d));
    Dialog7cb2fc_OnSlider(0x1d4e0, (s32)option(0x30));

    static const struct { u32 flag, button; } kVolumes[] = {{0x0078d8e8, 0x1d4d7}, {0x0078d8f0, 0x1d4d9}, {0x0078d8f8, 0x1d4db}, {0x0078d900, 0x1d4dd}};
    for (const auto &volume : kVolumes) {
        const u8 state = !global<u8>(volume.flag);
        g_env->setState(child(volume.button), nullptr, state, 1);
        Dialog7cb2fc_OnButton(volume.button, state);
    }
    textRow(0x1d4d5, 0x13f, stepText(option(4), 0x155));
    textRow(0x1d4d6, 0x13e, stepText(option(8), 0x150));

    g_env->toggle(0x1d4cb, 0x145, global<u8>(0x0078d90c), 1, 1, 0x1d4c1);
    g_env->toggle(0x1d4cc, 0x146, global<u8>(0x0078d90d), 1, 1, 0x1d4c2);
    g_env->toggle(0x1d4cd, 0x147, global<u8>(0x0078d90e), 1, 1, 0x1d4c3);
    g_env->toggle(0x1d4ce, 0x148, global<u8>(0x0078d90f), 1, 1, 0x1d4c4);
    g_env->toggle(0x1d4cf, 0x14b, global<u8>(0x0078d911), 1, 1, 0x1d4c5);
    textRow(0x1d4d0, 0x15f, global<u8>(0x0078d912) ? 0x161 : 0x160);
    textRow(0x1d4d1, 0x162, global<u8>(0x0078d910) ? 0x164 : 0x163);
}
}
