#include "window705a20.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/initializers.h"
#include "../core/vcall.h"
#include "guirect.h"
#include "window.h"
#include "windowtimers.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __fastcall Window705a20_SaveRect(u8 *window) { GuiRect_Assign(window + 0x2c, nullptr, window + 8); }

u8 *__fastcall Window705a20_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom) {
    Window_Ctor(window, nullptr, parent, flags, id, left, top, right, bottom);
    at<u32>(window, 0) = 0x705a20;
    at<u32>(window, 0x13c) = 0;
    if (flags & 0x40) {
        at<u8 *>(window, 0x138) = static_cast<u8 *>(GameHeap_Alloc(0x200));
        if (at<u8 *>(window, 0x138)) {
            at<u16>(at<u8 *>(window, 0x138), 0) = 0;
            at<u32>(window, 0x13c) = 0xff;
        }
    } else {
        at<u32>(window, 0x138) = 0;
        at<u32>(window, 0x13c) = 0;
    }
    Window705a20_SaveRect(window);
    at<u8 *>(window, 0x164) = WindowTimers_Add((u32)(uintptr_t)window, 0x2ee, 0x75e88);
    at<u16>(window, 0x14a) = 0xffff;
    at<u16>(window, 0x156) = 0;
    at<u16>(window, 0x154) = 0;
    at<u16>(window, 0x152) = 0;
    at<u16>(window, 0x150) = 0;
    at<u16>(window, 0x158) = 0;
    at<u16>(window, 0x14c) = 0xffff;
    at<u32>(window, 0x140) = 0;
    at<u32>(window, 0x144) = 0;
    at<u16>(window, 0x148) = 0;
    at<u16>(window, 0x14e) = 0;
    at<u32>(window, 0x15c) = 0;
    window[0x161] = 0;
    Window_SetHandler(window, nullptr, 0x66857d);
    at<u32>(window, 0x118) = 0x66857d;
    Window_SetFocus(window, nullptr, 0);
    window[0x160] = 1;
    window[0x162] = 0;
    if (*reinterpret_cast<u8 *>((uintptr_t)0x00faa56c)) window[0x162] = 1;
    return window;
}

u32 __fastcall Window705a20_Dtor(u8 *window) {
    at<u32>(window, 0) = 0x705a20;
    if (at<u32>(window, 0x138) && (at<u32>(window, 0x48) & 0x40)) GameHeap_Free(at<void *>(window, 0x138));
    if (at<u32>(window, 0x164)) WindowTimers_Kill((u32)(uintptr_t)window, 0x75e88);
    if (u8 *const caret = at<u8 *>(window, 0x15c)) {
        if ((s16)WindowImpl_Release(caret) == -1) vcall<u32>(caret, 0, 1u);
    }
    return Window_Dtor(window);
}

void __fastcall Window705a20_SetCaret(u8 *window, void *, u8 *caret) {
    if (caret == at<u8 *>(window, 0x15c)) return;
    if (u8 *const old = at<u8 *>(window, 0x15c)) {
        if ((s16)WindowImpl_Release(old) == -1) vcall<u32>(old, 0, 1u);
    }
    at<u8 *>(window, 0x15c) = caret;
    if (!caret) return;
    Fn_6617f0(caret);
    u8 event[0x28];  // (only the words +4-+9, +0x14 (the rectangle) and +0x24 set)
    at<u16>(event, 4) = 0;
    at<u16>(event, 6) = (u16)(at<s16>(window, 0xe0) + 3);
    at<u16>(event, 8) = at<u16>(window, 0xe2);
    const s32 left = (s32)at<s16>(window, 0x30) + at<s16>(window, 0x150), top = (s32)at<s16>(window, 0x32) + at<s16>(window, 0x154);
    u8 rect[12];
    GuiRect_CtorWith(rect, nullptr, (u32)left, (u32)top, (u32)(left + 8), (u32)(top + at<s16>(window, 0x156)));
    at<u8 *>(event, 0x14) = rect;
    at<u32>(event, 0x24) = 0x17;
    vcall<void>(caret, 0xc, 0x17u, event);
    if ((s16)Window_IsShown(window) != 0) vcall<void>(at<u8 *>(window, 0x15c), 0x14, 1u);
    GuiRect_Dtor(rect);
}

void __fastcall Window705a20_MoveCaret(u8 *window, void *, u32, u32) {
    u8 *const caret = at<u8 *>(window, 0x15c);
    if (!caret) return;
    const s32 left = (s32)at<s16>(window, 0x30) + at<s16>(window, 0x150), top = (s32)at<s16>(window, 0x32) + at<s16>(window, 0x154);
    u8 rect[12], event[0x18];  // (only the event's +0x14 set: the rectangle)
    GuiRect_CtorWith(rect, nullptr, (u32)left, (u32)top, (u32)(left + 8), (u32)(top + at<s16>(window, 0x156)));
    at<u8 *>(event, 0x14) = rect;
    vcall<void>(caret, 0xc, 0x10u, event);
    GuiRect_Dtor(rect);
}

u32 __fastcall Window705a20_KeyToParent(u8 *window, void *, u32 key, u32 value) {
    u8 *const parent = at<u8 *>(window, 0x60);
    const u32 id = at<u32>(window, 0x8c);
    switch ((s16)key) {
    case 9: return (u32)Window_SendMessage(parent, 0x803, id, value);
    case 0xd:
        if (at<u32>(window, 0x48) & 0x80) return (u32)(uintptr_t)window;  // sic: EAX the window
        return (u32)Window_SendMessage(parent, 0x802, id, value);
    case 0x1b: return (u32)Window_SendMessage(parent, 0x804, id, value);
    default: return (u32)Window_SendMessage(parent, 0x806, id, (u32)(s32)(s16)key);
    }
}

void __fastcall Window705a20_ToggleInsert(u8 *window) {
    if (at<s16>(window, 0x14c) != -1 || at<s16>(window, 0x158) == -1) return;
    at<u16>(window, 0x158) ^= 1;
    if (at<u8 *>(window, 0x15c)) {
        // (the word pushed with the window's upper half: AX loaded over EAX still holding the window)
        vcall<void>(at<u8 *>(window, 0x15c), 0x14, ((u32)(uintptr_t)window & 0xffff0000u) | at<u16>(window, 0x158));
    }
    if (*reinterpret_cast<u32 *>((uintptr_t)0x00faa5d4)) Window_PostMessage(window, 3, 0, 0);
}

void __fastcall Window705a20_NotifyText(u8 *window, void *, u32 post) {
    u8 text[0x10], event[0x18];  // (only the text's dword +0 and word +0xa and the event's +0x10 set)
    at<u32>(text, 0) = at<u32>(window, 0x138);
    at<u16>(text, 0xa) = at<u16>(window, 0x148);
    at<u8 *>(event, 0x10) = text;
    if (at<u8 *>(window, 0x9c)) vcall<void>(at<u8 *>(window, 0x9c), 0xc, 0x4100u, event);
    if (post) Window_PostMessage(at<u8 *>(window, 0x60), 0x801, at<u32>(window, 0x8c), 0);
}

void __fastcall Window705a20_NotifyScroll(u8 *window) {
    u8 inner[0x14], event[0x18];  // (only the inner's word +0xc and the event's +0x10 set; the original's inner lies just before it)
    at<u16>(inner, 0xc) = at<u16>(window, 0x14e);
    at<u8 *>(event, 0x10) = inner;
    if (at<u8 *>(window, 0x9c)) vcall<void>(at<u8 *>(window, 0x9c), 0xc, 0x8000u, event);
}

void __fastcall Window705a20_SetInsertMode(u8 *window, void *edx, u32 mode) {
    const bool none = (s16)mode == 0;
    at<u16>(window, 0x158) = none ? 0xffff : (u16)mode;
    if (at<u8 *>(window, 0x15c)) {
        // (the word pushed with the upper half of EDX: the caller's without a mode, else the window's address's; sic)
        const u32 high = (u32)(uintptr_t)(none ? edx : window) & 0xffff0000u;
        vcall<void>(at<u8 *>(window, 0x15c), 0x14, high | (mode & 0xffff));
    }
    if (*reinterpret_cast<u32 *>((uintptr_t)0x00faa5d4)) Window_PostMessage(window, 3, 0, 0);
}

u32 __fastcall Window705a20_SetBuffer(u8 *window, void *, u32 buffer, u32 size) {
    const u32 high = (u32)(uintptr_t)window & 0xffff0000u;
    if ((at<u32>(window, 0x48) & 0x40) || !buffer || !size) return high;
    at<u32>(window, 0x138) = buffer;
    at<u32>(window, 0x13c) = size;
    at<u8>(window, 0x160) = 1;
    return high | 1;
}

u32 __fastcall Window705a20_SelectionStart(u8 *window) {
    const s16 anchor = at<s16>(window, 0x14c);
    if (anchor == -1) return ((u32)(uintptr_t)window & 0xffff0000u) | at<u16>(window, 0x14a);
    if (anchor < at<s16>(window, 0x14a)) return ((u32)(uintptr_t)window & 0xffff0000u) | (u16)anchor;
    return ((u32)(s32)anchor & 0xffff0000u) | at<u16>(window, 0x14a);  // (the anchor's sign above, sic)
}

void __fastcall Window705a20_Selection(u8 *window, void *, u16 *from, u16 *to) {
    const s16 anchor = at<s16>(window, 0x14c), cursor = at<s16>(window, 0x14a);
    if (from) *from = anchor != -1 && anchor < cursor ? anchor : cursor;
    if (to) *to = anchor > cursor ? anchor : cursor;
}

void __fastcall Window705a20_SetSelection(u8 *window, void *, u32 cursorArg, u32 mark, u32 anchorArg) {
    const u16 *const text = at<const u16 *>(window, 0x138);
    if (!text || !at<u8 *>(window, 0x4c)) return;
    const s16 len = (s16)Crt().wcslen(text);
    if (at<s16>(window, 0x156) == 0)
        at<s16>(window, 0x156) = (s16)((s32)(s16)vcall<u32>(at<u8 *>(window, 0x4c), 0x4) * 0x600 / (s32)global<s16>(0x00faa5c0));
    s16 cursor = at<s16>(window, 0x14a), anchor = at<s16>(window, 0x14c);
    u8 *entry = nullptr;
    bool backward = false;
    switch ((s16)cursorArg) {
    case -6:
        backward = true;
        [[fallthrough]];
    case -7: {   // the character on the line below (above) the caret's nearest its x (+0x150)
        if (!(at<u32>(window, 0x48) & 0x80)) return;
        const s32 none = (s32)0xff7233d5;
        s32 lineUp = none, lineDown = none, best = 0x4000, found = -1;
        u8 *const chars = at<u8 *>(window, 0x9c);
        if (!chars) break;
        bool done = false;
        vcall<u8 *>(chars, 0x10, 1u, 0u);
        if (backward) {
            for (s16 i = (s16)(at<s16>(window, 0x14a) - 1); i >= 0 && !done;) {
                const u8 *const e = vcall<u8 *>(chars, 0x10, 1u, (u32)(s32)i);
                if (lineUp != none) {
                    if ((s32)at<s16>(e, 6) < lineUp) {
                        done = true;
                    } else if (s32 d = (s32)at<s16>(e, 4) - at<s16>(window, 0x150); (d < 0 ? -d : d) < best) {
                        best = d < 0 ? -d : d;
                        found = i;
                        i = (s16)(i - 1);
                    } else {
                        done = true;
                    }
                } else if (at<s16>(e, 6) < at<s16>(window, 0x154)) {
                    lineUp = at<s16>(e, 6);
                } else {
                    i = (s16)(i - 1);
                }
            }
        } else {
            for (s16 i = (s16)(at<s16>(window, 0x14a) + 1); i < len && !done;) {
                const u8 *const e = vcall<u8 *>(chars, 0x10, 1u, (u32)(s32)i);
                if (lineDown != none) {
                    if ((s32)at<s16>(e, 6) > lineDown) {
                        done = true;
                    } else if (s32 d = (s32)at<s16>(e, 4) - at<s16>(window, 0x150); (d < 0 ? -d : d) < best) {
                        best = d < 0 ? -d : d;
                        found = i;
                        i = (s16)(i + 1);
                    } else {
                        done = true;
                    }
                } else if (at<s16>(e, 6) > at<s16>(window, 0x154)) {
                    lineDown = at<s16>(e, 6);
                } else {
                    i = (s16)(i + 1);
                }
            }
        }
        if (found <= -1) return;
        cursor = (s16)found;
        break;
    }
    case -5: cursor = (s16)Crt().wcslen(text); break;
    case -4: cursor = 0; break;
    case -3: cursor = (s16)(at<s16>(window, 0x14a) + 1); break;
    case -2:
        cursor = (s16)(at<s16>(window, 0x14a) - 1);
        if (cursor < 0) cursor = 0;
        break;
    case -1: break;
    default: cursor = (s16)cursorArg;
    }
    switch ((s16)anchorArg) {
    case -5: anchor = (s16)Crt().wcslen(text); break;
    case -4: anchor = 0; break;
    case -3: anchor = (s16)((at<s16>(window, 0x14c) != -1 ? at<s16>(window, 0x14c) : at<s16>(window, 0x14a)) + 1); break;
    case -2:
        anchor = (s16)((at<s16>(window, 0x14c) != -1 ? at<s16>(window, 0x14c) : at<s16>(window, 0x14a)) - 1);
        if (anchor < 0) anchor = 0;
        break;
    case -1: anchor = -1; break;
    default: anchor = (s16)anchorArg;
    }
    if (anchor > len) anchor = len;
    if (anchor == cursor) anchor = -1;
    if (cursor > len) cursor = len;
    if (at<u8>(window, 0x162) && cursor != len) return;
    if (cursor == at<s16>(window, 0x14a) && anchor == at<s16>(window, 0x14c)) return;
    if (cursor < 0 || (u32)(s32)cursor > Crt().wcslen(at<const u16 *>(window, 0x138))) return;
    at<s16>(window, 0x14a) = cursor;
    at<s16>(window, 0x14c) = anchor;
    const s16 width = (s16)(at<s16>(window, 0x34) - at<s16>(window, 0x30));
    const s16 height = (s16)(at<s16>(window, 0x36) - at<s16>(window, 0x32));
    if (!(at<u32>(window, 0x48) & 0x80)) {   // the window scrolled sideways to the caret (or the anchor)
        entry = vcall<u8 *>(at<u8 *>(window, 0x9c), 0x10, 1u, (u32)(s32)(anchor != -1 ? anchor : cursor));
        const s16 x = at<s16>(entry, 4);
        if (x > width) {
            at<s16>(window, 0x148) = (s16)(at<s16>(window, 0x148) + (width - (x + 8)));
            Window705a20_NotifyText(window, nullptr, 0);
        } else if (x < 0) {
            if (anchor == 0 || cursor == 0) at<s16>(window, 0x148) = 0;
            else at<s16>(window, 0x148) = (s16)(at<s16>(window, 0x148) - (x + 8));
            Window705a20_NotifyText(window, nullptr, 0);
        }
    }
    if (width == 0) return;
    volatile s16 unused = (s16)((s32)(s16)vcall<u32>(at<u8 *>(window, 0x4c), 0x4) * 0x600 / (s32)global<s16>(0x00faa5c0));   // (sic)
    (void)unused;
    if (at<u8 *>(window, 0x9c)) entry = vcall<u8 *>(at<u8 *>(window, 0x9c), 0x10, 1u, (u32)(s32)cursor);
    s16 entryX = 0, top = 0, left = 0, right = 0, bottom = 0;   // (entryX and bottom unset in the original on the paths noted below)
    if (entry) {
        entryX = at<s16>(entry, 4);
        top = at<s16>(entry, 6);
    }
    if (at<u32>(window, 0x48) & 0x80) {   // several lines: scrolled up or down to the caret's line
        u8 *const chars = at<u8 *>(window, 0x9c);
        entry = chars ? vcall<u8 *>(chars, 0x10, 1u, (u32)(s32)at<s16>(window, 0x14a)) : nullptr;
        left = entry ? at<s16>(entry, 4) : 0;
        top = entry ? at<s16>(entry, 6) : 0;
        right = (s16)(left + 8);
        if (chars) entry = vcall<u8 *>(chars, 0x10, 2u, at<u32>(window, 0x138));
        if (entry) {   // (none: bottom stays unset, sic)
            bool moved = false;
            at<s16>(entry, 0xa) = (s16)(at<s16>(window, 0x36) - at<s16>(window, 0x32));
            const s32 line = at<s16>(window, 0x156);
            if ((s32)top + line > (s32)at<s16>(entry, 0xa)) {
                const s32 n = ((s32)top + line - at<s16>(entry, 0xa)) / line + 1;
                at<s16>(window, 0x14e) = (s16)(at<s16>(window, 0x14e) - (s16)(n * line));
                Window705a20_NotifyScroll(window);
                Window705a20_NotifyText(window, nullptr, 0);
                moved = true;
            } else if (top < 0) {
                const s32 n = -(s32)top / line;
                at<s16>(window, 0x14e) = (s16)(at<s16>(window, 0x14e) + (s16)(n * line));
                Window705a20_NotifyScroll(window);
                Window705a20_NotifyText(window, nullptr, 0);
                moved = true;
            }
            if (moved) {
                entry = vcall<u8 *>(chars, 0x10, 1u, (u32)(s32)at<s16>(window, 0x14a));
                left = entry ? at<s16>(entry, 4) : 0;
                top = entry ? at<s16>(entry, 6) : 0;
                right = (s16)(left + 8);
            }
            bottom = (s16)(top + at<s16>(window, 0x156));
            if (at<s16>(window, 0x14c) != -1) {
                const s32 first = at<s16>(vcall<u8 *>(chars, 0x10, 1u, 0u), 6);
                entry = vcall<u8 *>(chars, 0x10, 1u, (u32)(s32)at<s16>(window, 0x14c));
                right = (s32)at<s16>(entry, 6) > first ? width : at<s16>(entry, 4);
                bottom = (s16)(at<s16>(entry, 6) + at<s16>(window, 0x156));
            }
        }
    } else {   // one line (entryX unset when the entry is none, sic)
        left = entryX;
        right = (s16)(left + 8);
        bottom = (s16)(top + at<s16>(window, 0x156));
        if (at<s16>(window, 0x14c) != -1) {
            entry = vcall<u8 *>(at<u8 *>(window, 0x9c), 0x10, 1u, (u32)(s32)at<s16>(window, 0x14c));
            right = at<s16>(entry, 4);
        }
    }
    if (right > width) right = width;
    else if (right < 0) right = 0;
    if (left > width) left = width;
    else if (left < 0) left = 0;
    if (bottom > height) bottom = height;
    if (left == at<s16>(window, 0x150) && top == at<s16>(window, 0x154) && right == at<s16>(window, 0x152)) return;
    at<s16>(window, 0x150) = left;
    at<s16>(window, 0x152) = right;
    at<s16>(window, 0x154) = top;
    if ((s16)mark != 0) at<s16>(window, 0x158) = 1;
    if (global<u32>(0x00faa5d4)) Window_PostMessage(window, 3, 0, 0);
    if (!at<u8 *>(window, 0x15c)) return;
    u8 rect[12], event[0x18];   // (only the event's +0x14 set: the rectangle)
    GuiRect_Ctor(rect);
    const s32 x0 = at<s16>(window, 0x30), y0 = at<s16>(window, 0x32);
    if (at<s16>(window, 0x152) < at<s16>(window, 0x150))
        Fn_659dac(rect, nullptr, (u32)(x0 + at<s16>(window, 0x152)), (u32)(y0 + at<s16>(window, 0x154)), (u32)(x0 + at<s16>(window, 0x150)),
                  (u32)(y0 + bottom));
    else
        Fn_659dac(rect, nullptr, (u32)(x0 + at<s16>(window, 0x150)), (u32)(y0 + at<s16>(window, 0x154)), (u32)(x0 + at<s16>(window, 0x152)),
                  (u32)(y0 + bottom));
    at<u8 *>(event, 0x14) = rect;
    if (at<s16>(window, 0x158) != -1) {
        vcall<void>(at<u8 *>(window, 0x15c), 0x14, 1u);
        at<s16>(window, 0x158) = 1;
    }
    vcall<void>(at<u8 *>(window, 0x15c), 0xc, 0x10u, event);
    GuiRect_Dtor(rect);
}

u32 __fastcall Window705a20_SetText(u8 *window, void *, const u16 *text, u32 notify) {
    if (!text) return 0;
    bool changed = false;
    if (at<u32>(window, 0x48) & 0x40) {   // its own buffer: replaced by a new one when the text is longer (sic: one that fits is not copied)
        if (Crt().wcslen(text) > at<u32>(window, 0x13c)) {
            if (at<u16 *>(window, 0x138)) GameHeap_Free(at<u16 *>(window, 0x138));
            at<u16 *>(window, 0x138) = static_cast<u16 *>(GameHeap_Alloc(Crt().wcslen(text) * 2 + 2));
            at<u32>(window, 0x13c) = Crt().wcslen(text);
            if (at<u16 *>(window, 0x138)) Crt().wcscpy(at<u16 *>(window, 0x138), text);
            changed = true;
        }
    } else if (at<u32>(window, 0x13c) && at<u16 *>(window, 0x138) &&
               (Crt().wcscmp(at<u16 *>(window, 0x138), text) != 0 || at<u8>(window, 0x160))) {
        Crt().wcsncpy(at<u16 *>(window, 0x138), text, at<u32>(window, 0x13c));
        at<u16 *>(window, 0x138)[at<u32>(window, 0x13c)] = 0;
        at<u8>(window, 0x160) = 0;
        changed = true;
    } else if (at<const u16 *>(window, 0x138) == text) {
        changed = true;
        at<u8>(window, 0x160) = 0;
    }
    if (changed) {
        if (global<u32>(0x00faa5d4)) Window_PostMessage(window, 3, 0, 0);
        Window_NotifyBoth100(window, nullptr, at<u32>(window, 0x138));
        Window705a20_SetSelection(window, nullptr, (u32)-5, 0, (u32)-1);
        if (notify & 0xff) Window_PostMessage(at<u8 *>(window, 0x60), 0x801, at<u32>(window, 0x8c), 0);
    }
    return 0;
}

u32 __fastcall Window705a20_SetTextNotify(u8 *window, void *, const u16 *text) { return Window705a20_SetText(window, nullptr, text, 1); }

u32 __fastcall Window705a20_SetTextQuiet(u8 *window, void *, const u16 *text) { return Window705a20_SetText(window, nullptr, text, 0); }

u32 __fastcall Window705a20_GetText(u8 *window, void *, u16 *to, u32 n) {
    if (!at<u16 *>(window, 0x138)) return (u32)(uintptr_t)window & 0xffff0000u;
    return ((u32)(uintptr_t)Crt().wcsncpy(to, at<u16 *>(window, 0x138), n) & 0xffff0000u) | 1;
}

void __fastcall Window705a20_SetCursor(u8 *window, void *, u32 cursor, u32 anchor) {
    Window705a20_SetSelection(window, nullptr, cursor, 1, anchor);
}

u8 *__fastcall Window705a20_DeletingDtor(u8 *window, void *, u32 flags) {
    Window705a20_Dtor(window);
    if (flags & 1) GameHeap_Free(window);
    return window;
}
}
