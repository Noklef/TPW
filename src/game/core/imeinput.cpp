#include "imeinput.h"

#include <cstdint>

#include "buffers.h"
#include "crt.h"
#include "gameheap.h"
#include "ime.h"
#include "result.h"
#include "stdstring.h"
#include "stdtree.h"
#include "stdwstring.h"
#include "../ui/window7cc8c0handler.h"
#include "imports.h"

namespace {

AddrCalls g_calls;

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u32 ImmGetContext(u32 window) { return global<u32(__stdcall *)(u32)>(IMPORT(Imm32_ImmGetContext))(window); }
u32 ImmReleaseContext(u32 window, u32 context) { return global<u32(__stdcall *)(u32, u32)>(IMPORT(Imm32_ImmReleaseContext))(window, context); }
u32 ImmNotifyIME(u32 context, u32 action, u32 index, u32 value) { return global<u32(__stdcall *)(u32, u32, u32, u32)>(IMPORT(Imm32_ImmNotifyIME))(context, action, index, value); }
s32 Widen(const char *text, s32 count, u16 *out, s32 size) { return global<s32(__stdcall *)(u32, u32, const char *, s32, u16 *, s32)>(IMPORT(Kernel32_MultiByteToWideChar))(0, 0, text, count, out, size); }
u32 DefaultProc(u32 window, u32 message, u32 wparam, u32 lparam) { return global<u32(__stdcall *)(u32, u32, u32, u32)>(IMPORT(User32_DefWindowProcA))(window, message, wparam, lparam); }

const u32 kNarrowNpos = 0x006fe5ac, kWideNpos = 0x006fef00;
const u16 *EmptyText() { return reinterpret_cast<const u16 *>(gWindow7cc8c0handler_78a374); }
u8 *ImeMap() { return reinterpret_cast<u8 *>((uintptr_t)0x007b4a40); }
u8 *MapHead() { return global<u8 *>(0x007b4a44); }

// The map's guard: taken by its first user, whose construction differs (the allocator and compare bytes at +0 and +1 are unset locals in the two that write them).
bool TakeMapGuard() {
    if (gImeinput_7b4a38 & 1) return false;
    gImeinput_7b4a38 |= 1;
    return true;
}
void MapAtExit() { g_calls.Cd(0x0067a7d0, 0x004787e0u); }

void Unmap(u32 window) {
    u8 *found;
    Tree32_Find(ImeMap(), nullptr, &found, &window);
    if (found != MapHead()) {
        u8 *next;
        StdTree_EraseA(ImeMap(), nullptr, &next, found);
    }
}

void ClearComposition(u8 *ime) {
    StdWString_AssignChars(reinterpret_cast<StdWString *>(ime + 8), nullptr, EmptyText(), Crt().wcslen(EmptyText()));
}

// `text` narrowed into s, sized by a first call; the second call's answer.
s32 Narrow(StdString *s, const u16 *text) {
    const auto convert = global<s32(__stdcall *)(u32, u32, const u16 *, s32, char *, s32, u32, u32)>(IMPORT(Kernel32_WideCharToMultiByte));
    const s32 n = convert(0, 0, text, -1, nullptr, 0, 0, 0);
    StdString temp;
    StdString_Tidy(&temp, nullptr, 0);
    StdString_AssignFill(&temp, nullptr, (u32)n, 0x58);
    StdString_AssignSub(s, nullptr, &temp, 0, global<u32>(kNarrowNpos));
    StdString_Tidy(&temp, nullptr, 1);
    StdString_Freeze(s, nullptr);
    return convert(0, 0, text, -1, s->ptr, n, 0, 0);
}

const char *NarrowChars(const StdString &s) { return s.ptr ? s.ptr : reinterpret_cast<const char *>((uintptr_t)0x006fe5b0); }

// A composition string of the kind into the wide string at ime + offset, when it has a length that is not negative.
void ReadInto(u8 *ime, u32 offset, const u32 *context, u32 kind) {
    const s32 n = ImeContext_ReadComposition(context, nullptr, kind, nullptr, 0);
    if (n < 0) return;
    StdWString *const s = reinterpret_cast<StdWString *>(ime + offset);
    StdWString temp;
    const u8 allocator = 0;   // (an unset local's: never read again)
    StdWString_CtorFill(&temp, nullptr, (u32)n, 0x58, &allocator);
    StdWString_AssignSub(s, nullptr, &temp, 0, global<u32>(kWideNpos));
    CountedBuf_Reset(reinterpret_cast<u8 *>(&temp), nullptr, 1);
    StdWString_Freeze(s, nullptr);
    ImeContext_ReadComposition(context, nullptr, kind, s->ptr, n);
}

// The lists brought to 32, by a blank list added or the last ones destroyed.
void SizeLists(u8 *ime, u32 window) {
    u32 blank[9] = {};   // (+4, +8, +0xc and +0x20 are unset in the original)
    at<u8>(blank, 0x10) = (u8)window;
    u8 *const vector = ime + 0x38;
    u8 *const first = at<u8 *>(vector, 4);
    u8 *const last = at<u8 *>(vector, 8);
    const u32 size = first ? (u32)((s32)(last - first) / 0x24) : 0;
    if (size < 0x20) {
        CountedVector14Vector_InsertN(vector, nullptr, last, 0x20 - size, reinterpret_cast<const u8 *>(blank));
    } else if (size > 0x20) {
        u8 *const end = CountedVector14s_Copy(last, last, first + 0x480);
        CountedVector14_ClearRange(end, last);
        at<u8 *>(vector, 8) = end;
    }
    GameHeap_Free(nullptr);   // (the blank's strings' block)
}

}  // namespace

void ImeInput_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

s32 __fastcall ImeContext_ReadComposition(const u32 *context, void *, u32 kind, u16 *out, s32 size) {
    char buffer[0x40];
    const s32 n = global<s32(__stdcall *)(u32, u32, char *, u32)>(IMPORT(Imm32_ImmGetCompositionStringA))(context[1], kind, buffer, 0x40);
    if (n < 0) return n;
    buffer[n] = 0;
    s32 length = 0;
    while (buffer[length]) ++length;
    const s32 count = Widen(buffer, length, out, size);
    if (out) out[count] = 0;
    return count;
}

s32 __fastcall ImeContext_SetComposition(const u32 *context, void *, u32 index, const u16 *text, u32, const u16 *reading, u32) {
    StdString first, second;
    StdString_Tidy(&first, nullptr, 0);
    StdString_Tidy(&second, nullptr, 0);
    const char *firstChars = nullptr, *secondChars = nullptr;
    u32 firstLength = 0, secondLength = 0;
    s32 result = 0;
    if (text) {
        result = Narrow(&first, text);
        if (result >= 0) {
            firstChars = NarrowChars(first);
            firstLength = first.len;
        }
    }
    if (result >= 0 && reading) {
        result = Narrow(&second, reading);
        if (result >= 0) {
            secondChars = NarrowChars(second);
            secondLength = second.len;
        }
    }
    if (result >= 0)
        result = global<s32(__stdcall *)(u32, u32, const char *, u32, const char *, u32)>(IMPORT(Imm32_ImmSetCompositionStringA))(context[1], index, firstChars, firstLength, secondChars, secondLength);
    StdString_Tidy(&first, nullptr, 1);
    StdString_Tidy(&second, nullptr, 1);
    return result;
}

void __fastcall Ime_OnComposition(u8 *ime, void *, u32 window, u32, u32 flags) {
    u32 context[2];
    if (flags & 0x800) {
        ImeContext_Get(context, nullptr, window);
        if (context[1]) ReadInto(ime, 0x18, context, 0x800);
        ImeContext_Release(context);
        ImeContext_Get(context, nullptr, window);
        if (context[1]) ReadInto(ime, 8, context, 8);
        ImeContext_Release(context);
    } else if (flags & 8) {
        ImeContext_Get(context, nullptr, window);
        if (context[1]) ReadInto(ime, 8, context, 8);
        ImeContext_Release(context);
    } else {
        if (flags == 0) ClearComposition(ime);
        return;
    }
    context[0] = window;
    context[1] = ImmGetContext(window);
    if (!context[1]) return;
    char description[0x100];
    global<u32(__stdcall *)(u32, char *, u32)>(IMPORT(Imm32_ImmGetDescriptionA))(global<u32(__stdcall *)(u32)>(IMPORT(User32_GetKeyboardLayout))(0), description, 0x100);
    const bool ime98 = Crt().strncmp(description, gImeinput_74f8b0, 0x10) == 0;
    const u32 limit = at<u32>(ime, 0x58);
    if (at<u32>(ime, 0x10) > limit) {
        StdWString kept;
        CountedBuf_Reset(reinterpret_cast<u8 *>(&kept), nullptr, 0);
        StdWString_AssignSub(&kept, nullptr, reinterpret_cast<const StdWString *>(ime + 8), 0, global<u32>(kWideNpos));
        StdWString_Erase(&kept, nullptr, limit, global<u32>(kWideNpos));
        if (kept.len == 0) {
            ImmNotifyIME(context[1], 0x15, 4, 0);
        } else {
            if (!ime98) ImmNotifyIME(context[1], 0x15, 4, 0);
            ImeContext_SetComposition(context, nullptr, 9, kept.ptr, kept.len, nullptr, 0);
        }
        CountedBuf_Reset(reinterpret_cast<u8 *>(&kept), nullptr, 1);
    }
    ImmReleaseContext(context[0], context[1]);
}

void __fastcall Ime_ResetIfClosed(u8 *ime, void *, u32 window) {
    const u32 context = ImmGetContext(window);
    if (!context) return;
    if (!global<u32(__stdcall *)(u32)>(IMPORT(Imm32_ImmGetOpenStatus))(context)) {   // ImmGetOpenStatus
        u8 *const first = at<u8 *>(ime, 0x3c);
        CountedVector14_ClearRange(first, at<u8 *>(ime, 0x40));
        at<u8 *>(ime, 0x40) = first;
        ClearComposition(ime);
        u32 form[7];   // COMPOSITIONFORM, only its style and position set
        form[0] = 2;
        form[1] = 0;
        form[2] = 0;
        global<u32(__stdcall *)(u32, const u32 *)>(IMPORT(Imm32_ImmSetCompositionWindow))(context, form);   // ImmSetCompositionWindow
    }
    ImmReleaseContext(window, context);
}

void __fastcall Ime_Detach(u8 *ime) {
    const u32 window = at<u32>(ime, 0x4c);
    if (!window) return;
    const u32 context = ImmGetContext(window);
    if (context) global<u32(__stdcall *)(u32, u32)>(IMPORT(Imm32_ImmSetOpenStatus))(context, 0);   // ImmSetOpenStatus
    global<u32(__stdcall *)(u32, s32, u32)>(IMPORT(User32_SetWindowLongA))(at<u32>(ime, 0x4c), -4, at<u32>(ime, 0x50));   // SetWindowLongA
    if (TakeMapGuard()) {
        global<u8>(0x007b4a48) = 0;
        StdTree_CtorA(ImeMap());
        MapAtExit();
    }
    Unmap(at<u32>(ime, 0x4c));
    at<u32>(ime, 0x4c) = 0;
    at<u32>(ime, 0x50) = 0;
    ClearComposition(ime);
    if (context) ImmReleaseContext(window, context);
}

u32 *__fastcall Ime_Attach(u8 *ime, void *, u32 *out, u32 window) {
    *out = 0xffffffff;
    if (at<u32>(ime, 0x4c)) return out;
    if (!at<u32>(ime, 0x5c)) {
        at<u32>(ime, 0x5c) = global<u32(__stdcall *)()>(IMPORT(Imm32_ImmCreateContext))();   // ImmCreateContext
        if (!at<u32>(ime, 0x5c)) return out;
    }
    for (u32 i = 0; i < 0x40; ++i) at<u32>(ime, 0x60 + i * 4) = 0;
    at<u32>(ime, 0x58) = 0xc;
    at<u32>(ime, 0x4c) = 0;
    if (TakeMapGuard()) {
        const u8 comp = 0, alloc = 0;   // (unset locals in the original)
        StdMapA_Ctor(ImeMap(), nullptr, &comp, &alloc);
        MapAtExit();
    }
    u8 *found;
    Tree32_Find(ImeMap(), nullptr, &found, &window);
    if (found != MapHead()) return out;
    u32 *const value = StdMapA_Index(ImeMap(), nullptr, &window);
    value[0] = (u32)(uintptr_t)ime;
    value[1] = 0xffffffff;
    const u32 old = global<u32(__stdcall *)(u32, s32, u32)>(IMPORT(User32_SetWindowLongA))(window, -4, 0x0047a880);   // SetWindowLongA
    at<u32>(ime, 0x50) = old;
    if (!old) {
        s32 error;
        Result_FromLastError(&error);
        if (error) {
            Unmap(window);
            return out;
        }
    }
    at<u32>(ime, 0x4c) = window;
    const u32 context = ImmGetContext(window);
    if (context) ImmNotifyIME(context, 0x15, 4, 0);
    *out = 0;
    if (context) ImmReleaseContext(window, context);
    return out;
}

u32 __fastcall Ime_ReadCandidates(u8 *ime, void *, const u32 *context, u32 index) {
    const auto list = global<u32(__stdcall *)(u32, u32, u8 *, u32)>(IMPORT(Imm32_ImmGetCandidateListA));   // ImmGetCandidateListA
    u8 *const entry = at<u8 *>(ime, 0x3c) + index * 0x24;
    u8 *const strings = entry + 0x10;
    StdWStringVector_Erase(strings, nullptr, at<StdWString *>(strings, 4), at<StdWString *>(strings, 8));
    ++at<u32>(entry, 0);
    const u32 size = list(context[1], index, nullptr, 0);
    if (!size) return 0;
    u8 *const data = static_cast<u8 *>(GameHeap_Alloc(size));
    list(context[1], index, data, size);
    at<u32>(entry, 4) = at<u32>(data, 0x14);
    at<u32>(entry, 0xc) = at<u32>(data, 0xc);
    at<u32>(entry, 8) = at<u32>(data, 0x10);
    const u8 allocator = (u8)((uintptr_t)context >> 24);   // (sic)
    StdWString text;
    at<u8>(&text, 0) = allocator;
    CountedBuf_Reset(reinterpret_cast<u8 *>(&text), nullptr, 0);
    u32 hash = 0;
    for (u32 i = 0; i < at<u32>(data, 8); ++i) {
        const char *const candidate = reinterpret_cast<const char *>(data + at<u32>(data, 0x18 + i * 4));
        bool widened = false;
        const s32 n = Widen(candidate, -1, nullptr, 0);
        if (n) {
            StdWString temp;
            StdWString_CtorFill(&temp, nullptr, (u32)(n - 1), 0x58, &allocator);
            StdWString_AssignSub(&text, nullptr, &temp, 0, global<u32>(kWideNpos));
            CountedBuf_Reset(reinterpret_cast<u8 *>(&temp), nullptr, 1);
            StdWString_Freeze(&text, nullptr);
            widened = Widen(candidate, -1, text.ptr, n) != 0;
        }
        if (widened) {
            StdWStringVector_InsertN(strings, nullptr, at<StdWString *>(strings, 8), 1, &text);
            StdWString_Freeze(&text, nullptr);
            const u16 *const end = text.ptr ? text.ptr + text.len : nullptr;
            for (const u16 *c = text.ptr; c != end; ++c) hash = (hash << 1 | hash >> 31) ^ *c;
        } else {
            StdWString blank;
            at<u8>(&blank, 0) = allocator;
            CountedBuf_Reset(reinterpret_cast<u8 *>(&blank), nullptr, 0);
            StdWString_AssignChars(&blank, nullptr, EmptyText(), Crt().wcslen(EmptyText()));
            StdWStringVector_InsertN(strings, nullptr, at<StdWString *>(strings, 8), 1, &blank);
            CountedBuf_Reset(reinterpret_cast<u8 *>(&blank), nullptr, 1);
        }
    }
    at<u32>(entry, 0x20) = hash;
    CountedBuf_Reset(reinterpret_cast<u8 *>(&text), nullptr, 1);
    if (data) GameHeap_Free(data);
    return 1;
}

void __fastcall Ime_OpenCandidates(u8 *ime, void *, u32 window, u32 mask) {
    u32 context[2] = {window, ImmGetContext(window)};
    if (!context[1]) return;
    SizeLists(ime, window);
    for (u32 i = 0; i < 0x20; ++i) {
        if ((mask >> i & 1) && Ime_ReadCandidates(ime, nullptr, context, i)) {
            at<u32>(ime, 4) = 2;
            at<u32>(ime, 0x54) = i;
        }
    }
    ImmReleaseContext(context[0], context[1]);
}

void __fastcall Ime_ChangeCandidates(u8 *ime, void *, u32 window, u32 mask) {
    u32 context[2] = {window, ImmGetContext(window)};
    SizeLists(ime, window);
    for (u32 i = 0; i < 0x20; ++i) {
        if ((mask >> i & 1) && Ime_ReadCandidates(ime, nullptr, context, i)) {
            at<u32>(ime, 4) = 2;
            at<u32>(ime, 0x54) = i;
            break;
        }
    }
    if (context[1]) ImmReleaseContext(context[0], context[1]);
}

u32 __cdecl Ime_PickLayout(u32 layout) {
    const auto layouts = global<s32(__stdcall *)(s32, u32 *)>(IMPORT(User32_GetKeyboardLayoutList));   // GetKeyboardLayoutList
    const s32 n = layouts(0, nullptr);
    if (!n) return 0;
    const u32 zero = 0;
    u32 *const first = static_cast<u32 *>(GameHeap_Alloc((u32)(n < 0 ? 0 : n) << 2));
    ImeVector_Fill(nullptr, nullptr, first, (u32)n, &zero);
    u32 *const last = first + n;
    // (a failed block: the original's reserve, into a second block, reads through the null first and faults)
    if (!first) ImeVector_Copy(nullptr, nullptr, first, last, static_cast<u32 *>(GameHeap_Alloc((u32)n << 2)));
    layouts(n, first);
    u32 *found = last;
    while (found != first && found[-1] != layout) --found;
    u32 result = 0;
    if (found != first) {
        u32 order[4] = {};
        u8 *const vector = reinterpret_cast<u8 *>(order);
        for (u32 *p = found; p != first;) {
            --p;
            ImeVector_InsertOne(vector, nullptr, at<u32 *>(vector, 8), p);
        }
        for (u32 *p = last; p != found;) {
            --p;
            ImeVector_InsertN(vector, nullptr, at<u32 *>(vector, 8), 1, p);
        }
        for (u32 *p = at<u32 *>(vector, 4); p != at<u32 *>(vector, 8); ++p) {
            if (!(global<u32(__stdcall *)(u32, u32)>(IMPORT(Imm32_ImmGetProperty))(*p, 4) & 0x30000)) {   // ImmGetProperty
                result = *p;
                break;
            }
        }
        GameHeap_Free(at<u32 *>(vector, 4));
    }
    GameHeap_Free(first);
    return result;
}

u32 __fastcall Ime_OnMessage(u8 *ime, void *, u32 window, u32 message, u32 wparam, u32 lparam) {
    bool pass = true;
    switch (message) {
    case 0x50:   // WM_INPUTLANGCHANGEREQUEST
        lparam = Ime_PickLayout(lparam);
        if (!lparam) return 0;
        break;
    case 0x100:   // WM_KEYDOWN
        at<u8>(ime, wparam + 0x60) = 1;
        break;
    case 0x101:   // WM_KEYUP
        if (!at<u8>(ime, wparam + 0x60)) pass = false;
        at<u8>(ime, wparam + 0x60) = 0;
        break;
    case 0x102:   // WM_CHAR
        if (Ime_OnChar(ime, nullptr, window, wparam & 0xffff, lparam)) return 1;
        break;
    case 0x10d:   // WM_IME_STARTCOMPOSITION
        at<u8>(ime, 0x48) = 1;
        return 1;
    case 0x10e:   // WM_IME_ENDCOMPOSITION
        at<u8>(ime, 0x48) = 0;
        ClearComposition(ime);
        return 1;
    case 0x10f:   // WM_IME_COMPOSITION
        Ime_OnComposition(ime, nullptr, window, wparam, lparam);
        return DefaultProc(window, 0x10f, wparam, lparam);
    case 0x281:   // WM_IME_SETCONTEXT
        lparam &= 0x3ffffff0;
        break;
    case 0x282:   // WM_IME_NOTIFY
        switch (wparam) {
        case 3:   // IMN_CHANGECANDIDATE
            Ime_ChangeCandidates(ime, nullptr, window, lparam);
            return 1;
        case 4: {   // IMN_CLOSECANDIDATE
            u32 i = 0;
            for (u8 *entry = at<u8 *>(ime, 0x3c); entry != at<u8 *>(ime, 0x40); entry += 0x24, ++i) {
                if (lparam >> (i & 31) & 1) {
                    if (i == at<u32>(ime, 0x54)) at<u32>(ime, 0x54) = 0xffffffff;
                    u8 *const strings = at<u8 *>(entry, 0x14);
                    CountedEntries_Release(strings, at<u8 *>(entry, 0x18));   // (the original's std::copy runs from the last to the last, sic)
                    at<u8 *>(entry, 0x18) = strings;
                }
            }
            at<u32>(ime, 4) = 1;
            return 1;
        }
        case 5: {   // IMN_OPENCANDIDATE, then the messages waiting (at most 200)
            Ime_OpenCandidates(ime, nullptr, window, lparam);
            const auto peek = global<u32(__stdcall *)(u32 *, u32, u32, u32, u32)>(IMPORT(User32_PeekMessageA));
            u32 waiting[7];   // MSG
            s32 left = 200;
            if (peek(waiting, 0, 0, 0, 1)) {
                while (left-- != 0) {
                    global<u32(__stdcall *)(const u32 *)>(IMPORT(User32_TranslateMessage))(waiting);   // TranslateMessage
                    global<u32(__stdcall *)(const u32 *)>(IMPORT(User32_DispatchMessageA))(waiting);   // DispatchMessageA
                    if (!peek(waiting, 0, 0, 0, 1)) break;
                }
            }
            return 1;
        }
        case 8:   // IMN_SETCOMPOSITIONWINDOW
            Ime_ResetIfClosed(ime, nullptr, window);
            return 1;
        default:
            break;
        }
        break;
    case 0x283:
    case 0x284:
    case 0x290:
    case 0x291:
        return 1;
    case 0x286: {   // WM_IME_CHAR: a double-byte character's bytes put in order, then one wide character sent as message 0x410
        u32 bytes = wparam & 0xffff;
        if (bytes & 0xff00) bytes = bytes >> 8 | (bytes & 0xff) << 8;
        const s32 length = !(bytes & 0xff) ? 0 : (bytes & 0xff00) ? 2 : 1;
        u32 character = 0;
        if (Widen(reinterpret_cast<const char *>(&bytes), length, reinterpret_cast<u16 *>(&character), 1) == 1) {
            const auto handler = at<u32(__stdcall *)(u32, u32, u32, u32)>(ime, 0x50);
            if (handler)
                handler(window, 0x410, character & 0xffff, lparam);
            else
                DefaultProc(window, 0x410, character & 0xffff, lparam);
        }
        return 1;
    }
    default:
        break;
    }
    const auto handler = at<u32(__stdcall *)(u32, u32, u32, u32)>(ime, 0x50);
    if (handler && pass) return handler(window, message, wparam, lparam);
    return DefaultProc(window, message, wparam, lparam);
}

u32 __stdcall Ime_WindowProc(u32 window, u32 message, u32 wparam, u32 lparam) {
    if (TakeMapGuard()) {
        StdTree_CtorFullA(ImeMap());
        MapAtExit();
    }
    u8 *const node = Tree32_LowerBound(ImeMap(), nullptr, &window);
    u8 *const ime = node != MapHead() && window >= at<u32>(node, 0xc) ? at<u8 *>(node, 0x10) : nullptr;
    if (ime) return Ime_OnMessage(ime, nullptr, window, message, wparam, lparam);
    return DefaultProc(window, message, wparam, lparam);
}
}
