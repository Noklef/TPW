#include "ime.h"

#include <cstdint>

#include "accessors.h"
#include "buffers.h"
#include "crt.h"
#include "gameheap.h"
#include "stdwstring.h"
#include "../ui/window.h"
#include "../ui/window705a20.h"
#include "../ui/window705a50.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// The difference of two pointers in dwords, shifted arithmetically as the compiled code does.
s32 Dwords(const u32 *a, const u32 *b) { return (s32)((uintptr_t)a - (uintptr_t)b) >> 2; }

template <typename F> F import(std::uintptr_t slot) { return *reinterpret_cast<const F *>(slot); }

struct Message {   // (MSG)
    u32 window, message, wparam, lparam, time, x, y;
};

// The wide string's _Tidy(true), inlined in the destructor: the block (two bytes before the characters) freed when the count byte before
// them is 0 or 0xff (frozen), else that count lowered; then ptr, len and res 0. s is the string { allocator, ptr, len, res }.
void TidyWide(u8 *s) {
    u8 *const characters = at<u8 *>(s, 4);
    if (characters) {
        const u8 count = characters[-1];
        if (count != 0 && count != 0xff)
            characters[-1] = (u8)(count - 1);
        else
            GameHeap_Free(characters - 2);
    }
    at<u32>(s, 4) = 0;
    at<u32>(s, 8) = 0;
    at<u32>(s, 0xc) = 0;
}

}  // namespace

extern "C" {

u8 *__fastcall Ime_Ctor(u8 *ime) {
    // The original's `push ecx` makes room for a local allocator it never sets, then copies its
    // byte (the pushed this's top byte) into each vector (sic).
    const u8 junk = (u8)((uintptr_t)ime >> 24);
    for (u32 v = 8; v < 0x48; v += 0x10) {
        at<u8>(ime, v) = junk;
        at<u32>(ime, v + 4) = 0;
        at<u32>(ime, v + 8) = 0;
        at<u32>(ime, v + 0xc) = 0;
    }
    at<u8>(ime, 0x48) = 0;
    at<u32>(ime, 0x4c) = 0;
    at<u32>(ime, 0x50) = 0;
    at<u32>(ime, 0x54) = 0xffffffff;
    at<u32>(ime, 0x5c) = 0;
    at<u32>(ime, 0) = 0x006fef04;
    return ime;
}

u32 *__fastcall ImeVector_Copy(void *, void *, const u32 *first, const u32 *last, u32 *dest) {
    for (; first != last; ++first, ++dest)
        if (dest) *dest = *first;
    return dest;
}

void __fastcall ImeVector_Fill(void *, void *, u32 *dest, u32 n, const u32 *value) {
    for (; n != 0; --n, ++dest)
        if (dest) *dest = *value;
}

u32 __fastcall ImeVector_Size(const u8 *vec) {
    const u8 *first = at<const u8 *>(vec, 4);
    return first ? (u32)((s32)(at<const u8 *>(vec, 8) - first) >> 2) : 0;
}

u32 __fastcall ImeVector_Capacity(const u8 *vec) {
    const u8 *first = at<const u8 *>(vec, 4);
    return first ? (u32)((s32)(at<const u8 *>(vec, 0xc) - first) >> 2) : 0;
}

u32 __fastcall ImeVector16_Size(const u8 *vec) {
    const u8 *first = at<const u8 *>(vec, 4);
    return first ? (u32)((s32)(at<const u8 *>(vec, 8) - first) >> 4) : 0;
}

u32 __fastcall ImeVector16_Capacity(const u8 *vec) {
    const u8 *first = at<const u8 *>(vec, 4);
    return first ? (u32)((s32)(at<const u8 *>(vec, 0xc) - first) >> 4) : 0;
}

void __cdecl ImeVector_Construct(u32 *slot, const u32 *value) {
    if (slot) *slot = *value;
}

u8 *__fastcall Ime_Current(const u8 *ime) {
    const s32 index = at<s32>(ime, 0x54);
    return index == -1 ? nullptr : at<u8 *>(ime, 0x3c) + index * 0x24;
}

u32 __fastcall Ime_CacheCurrent(u8 *ime) {
    const u8 *cur = Ime_Current(ime);
    if (!cur) {
        at<u32>(ime, 0x188) = 0;
        at<u8>(ime, 0x184) = 0;
    } else if (!at<u8>(ime, 0x184) || at<u32>(cur, 0) != at<u32>(ime, 0x160)) {
        CountedVector14_Assign(ime + 0x160, nullptr, cur);  // (inlined)
        if (!at<u32>(ime, 0x188)) at<u32>(ime, 0x188) = at<u32>(ime, 0x180);
        at<u8>(ime, 0x184) = 1;
    }
    return at<u8>(ime, 0x184) ? ImeVector16_Size(ime + 0x170) : 0;
}

void __fastcall Ime_SetLimit(u8 *ime, void *, u32 limit) {
    const u32 most = *reinterpret_cast<const u32 *>(0x006feee0);
    at<u32>(ime, 0x58) = limit < most ? limit : most;
}

u8 *__fastcall ImeHost_Ctor(u8 *host) {
    Ime_Ctor(host + 0x1c);
    at<u32>(host, 0x17c) = 0;
    // Another unset local (the push ecx's slot): the pushed this's top byte (sic).
    at<u8>(host, 0x18c) = (u8)((uintptr_t)host >> 24);
    at<u32>(host, 0x190) = 0;
    at<u32>(host, 0x194) = 0;
    at<u32>(host, 0x198) = 0;
    at<u32>(host, 8) = 0;
    at<u32>(host, 0) = 0;
    at<u8>(host, 0xc) = 0;
    at<u32>(host, 0x10) = 0xffffffff;
    return host;
}

u32 *__fastcall ImeVector_InsertOne(u8 *vec, void *, u32 *position, const u32 *value) {
    u32 *const first = at<u32 *>(vec, 4);
    u32 *const last = at<u32 *>(vec, 8);
    const s32 index = Dwords(position, first);
    if ((u32)Dwords(at<u32 *>(vec, 0xc), last) == 0) {
        const s32 size = first == nullptr ? 0 : Dwords(last, first);
        const s32 grow = (u32)size <= 1 ? 1 : size;
        const s32 capacity = size + grow;
        u32 *const block = static_cast<u32 *>(GameHeap_Alloc((u32)(capacity < 0 ? 0 : capacity) << 2));
        u32 *to = block;
        for (u32 *from = first; from != position; ++from, ++to)
            ImeVector_Construct(to, from);
        ImeVector_Construct(to, value);
        ImeVector_Copy(vec, nullptr, position, at<u32 *>(vec, 8), to + 1);
        GameHeap_Free(at<u32 *>(vec, 4));
        at<u32 *>(vec, 0xc) = block + capacity;
        const u32 old = ImeVector_Size(vec);
        at<u32 *>(vec, 8) = block + old + 1;
        at<u32 *>(vec, 4) = block;
        return block + index;
    }
    if ((u32)Dwords(last, position) < 1) {
        ImeVector_Copy(vec, nullptr, position, last, position + 1);
        ImeVector_Fill(vec, nullptr, at<u32 *>(vec, 8), (u32)(1 - Dwords(at<u32 *>(vec, 8), position)), value);
        u32 *const end = at<u32 *>(vec, 8);
        if (position != end) {
            u32 *p = position;
            do {
                *p = *value;
                ++p;
            } while (p != end);
        }
    } else {
        ImeVector_Copy(vec, nullptr, last - 1, last, last);
        u32 *to = at<u32 *>(vec, 8);
        u32 *from = to - 1;
        if (position != from) {
            do {
                const u32 v = from[-1];
                --from;
                --to;
                *to = v;
            } while (from != position);
        }
        u32 *const stop = position + 1;
        u32 *p = position;
        if (position != stop) {
            do {
                *p = *value;
                ++p;
            } while (p != stop);
        }
    }
    at<u32 *>(vec, 8) = at<u32 *>(vec, 8) + 1;
    return at<u32 *>(vec, 4) + index;
}

void __fastcall ImeVector_InsertN(u8 *vec, void *, u32 *position, u32 count, const u32 *value) {
    u32 *const last = at<u32 *>(vec, 8);
    if ((u32)Dwords(at<u32 *>(vec, 0xc), last) >= count) {
        if ((u32)Dwords(last, position) >= count) {
            if (count == 0) return;
            u32 *to = last;
            for (u32 *from = last - count; from != last; ++from, ++to)
                if (to) *to = *from;
            u32 *e = at<u32 *>(vec, 8) - count;
            to = at<u32 *>(vec, 8);
            if (position != e) {
                do {
                    const u32 v = e[-1];
                    --e;
                    --to;
                    *to = v;
                } while (e != position);
            }
            u32 *const stop = position + count;
            u32 *p = position;
            if (position != stop) {
                do {
                    *p = *value;
                    ++p;
                } while (p != stop);
            }
        } else {
            u32 *to = position + count;
            if (position != last) {
                u32 *from = to - count;
                do {
                    if (to) *to = *from;
                    ++from;
                    ++to;
                } while (from != last);
            }
            u32 *fill = at<u32 *>(vec, 8);
            for (s32 n = (s32)count - Dwords(fill, position); n != 0; --n, ++fill)
                if (fill) *fill = *value;
            u32 *const end = at<u32 *>(vec, 8);
            u32 *p = position;
            if (position != end) {
                do {
                    *p = *value;
                    ++p;
                } while (p != end);
            }
        }
        at<u32 *>(vec, 8) = at<u32 *>(vec, 8) + count;
        return;
    }
    u32 *const first = at<u32 *>(vec, 4);
    const s32 size = first == nullptr ? 0 : Dwords(last, first);
    const s32 grow = count >= (u32)size ? (s32)count : size;
    s32 capacity = grow + size;
    u32 *const block = static_cast<u32 *>(GameHeap_Alloc((u32)(capacity < 0 ? 0 : capacity) << 2));
    u32 *to = block;
    for (u32 *from = at<u32 *>(vec, 4); from != position; ++from, ++to)
        if (to) *to = *from;
    u32 *fill = to;
    for (u32 n = count; n != 0; --n, ++fill)
        if (fill) *fill = *value;
    u32 *const end = at<u32 *>(vec, 8);
    u32 *dst = to + count;
    if (position != end) {
        for (u32 *from = position; from != end; ++from, ++dst)
            if (dst) *dst = *from;
    }
    GameHeap_Free(at<u32 *>(vec, 4));
    at<u32 *>(vec, 0xc) = block + capacity;
    u32 *const old = at<u32 *>(vec, 4);
    at<u32 *>(vec, 4) = block;
    at<u32 *>(vec, 8) = block + count + (old == nullptr ? 0 : Dwords(at<u32 *>(vec, 8), old));
}

void __fastcall ImeHost_OnWindow(u8 *host, void *, u8 *window) {
    if (window != at<u8 *>(host, 4)) return;
    at<u16>(host, 0x1a8) = 0;
    at<u16>(host, 0x1aa) = 1;
    const s32 room = at<s16>(window, 0x13c);
    Ime_SetLimit(host + 0x1c, nullptr, (u32)(room - (s16)Window705a50_TextLength(window)));
}

u32 __fastcall Ime_OnChar(u8 *ime, void *, u32 window, u32 character, u32 lparam) {
    if (!import<u32(__stdcall *)(u32)>(0x006fd154)(character)) return 0;   // (IsDBCSLeadByte of the whole dword, as pushed)
    const auto peek = import<u32(__stdcall *)(Message *, u32, u32, u32, u32)>(0x006fd354);
    Message message;
    s32 tries = 10;
    while (!peek(&message, window, 0x102, 0x102, 1)) {
        if (--tries == 0) return 1;
    }
    u32 bytes = (u8)character | (u32)(u8)message.wparam << 8;
    u32 wide = 0;
    if (import<s32(__stdcall *)(u32, u32, const u32 *, s32, u32 *, s32)>(0x006fd174)(0, 0, &bytes, 2, &wide, 1) != 1) return 1;
    const auto handler = at<u32(__stdcall *)(u32, u32, u32, u32)>(ime, 0x50);
    if (handler) {
        handler(window, 0x410, wide & 0xffff, lparam);
    } else {
        import<u32(__stdcall *)(u32, u32, u32, u32)>(0x006fd340)(window, 0x410, wide & 0xffff, lparam);
    }
    return 1;
}

u32 *__fastcall ImeContext_Get(u32 *context, void *, u32 window) {
    context[0] = window;
    context[1] = import<u32(__stdcall *)(u32)>(0x006fd084)(window);
    return context;
}

u32 __fastcall ImeContext_Release(const u32 *context) {
    const u32 himc = context[1];
    if (!himc) return 0;
    return import<u32(__stdcall *)(u32, u32)>(0x006fd080)(context[0], himc);
}

void __fastcall Ime_Dtor(u8 *ime) {
    at<u32>(ime, 0) = 0x006fef04;
    const u32 context = at<u32>(ime, 0x5c);
    if (context) import<u32(__stdcall *)(u32)>(0x006fd07c)(context);
    u8 *const last = at<u8 *>(ime, 0x40);
    for (u8 *entry = at<u8 *>(ime, 0x3c); entry != last; entry += 0x24) CountedVector4_Clear(entry + 0x10);
    GameHeap_Free(at<void *>(ime, 0x3c));
    at<u32>(ime, 0x3c) = 0;
    at<u32>(ime, 0x40) = 0;
    at<u32>(ime, 0x44) = 0;
    TidyWide(ime + 0x28);
    TidyWide(ime + 0x18);
    TidyWide(ime + 8);
}

void __fastcall ImeHost_Dtor(u8 *host) {
    u8 *const ime = host + 0x1c;
    u8 *const last = at<u8 *>(ime, 0x178);
    for (u8 *text = at<u8 *>(ime, 0x174); text != last; text += 0x10) CountedBuf_Reset(text, nullptr, 1);
    GameHeap_Free(at<void *>(ime, 0x174));
    at<u32>(ime, 0x174) = 0;
    at<u32>(ime, 0x178) = 0;
    at<u32>(ime, 0x17c) = 0;
    Ime_Dtor(ime);
}

u8 *__fastcall Ime_DeletingDtor(u8 *ime, void *, u32 flags) {
    Ime_Dtor(ime);
    if (flags & 1) GameHeap_Free(ime);
    return ime;
}

void __fastcall Ime_SelectCandidate(u8 *ime, void *, u32 value) {
    const u32 window = at<u32>(ime, 0x4c);
    const u32 context = import<u32(__stdcall *)(u32)>(0x006fd084)(window);
    if (!context) return;
    const u32 index = at<u32>(ime, 0x54);
    const auto notify = import<u32(__stdcall *)(u32, u32, u32, u32)>(0x006fd068);
    notify(context, 0x12, index, value);
    notify(context, 0x11, index, 0);
    import<u32(__stdcall *)(u32, u32)>(0x006fd080)(window, context);
}

void __fastcall Ime_SetCandidatePage(u8 *ime, void *, u32 first, u32 last, u32) {
    const u32 window = at<u32>(ime, 0x4c);
    const u32 context = import<u32(__stdcall *)(u32)>(0x006fd084)(window);
    if (!context) return;
    const u32 index = at<u32>(ime, 0x54);
    const auto notify = import<u32(__stdcall *)(u32, u32, u32, u32)>(0x006fd068);
    notify(context, 0x17, index, last - first);
    notify(context, 0x16, index, first);
    char description[0x100];
    import<u32(__stdcall *)(u32, char *, u32)>(0x006fd088)(import<u32(__stdcall *)(u32)>(0x006fd304)(0), description, 0x100);
    const auto widen = import<s32(__stdcall *)(u32, u32, const char *, s32, u16 *, s32)>(0x006fd174);
    StdWString *const name = reinterpret_cast<StdWString *>(ime + 0x28);
    const s32 count = widen(0, 0, description, -1, nullptr, 0);
    if (count) {
        StdWString temp = {};   // (the original's allocator byte is an unset local's: never read)
        CountedBuf_Reset(reinterpret_cast<u8 *>(&temp), nullptr, 0);
        StdWString_AssignFill(&temp, nullptr, (u32)(count - 1), 0x58);
        StdWString_AssignSub(name, nullptr, &temp, 0, *reinterpret_cast<const u32 *>((uintptr_t)0x006fef00));
        CountedBuf_Reset(reinterpret_cast<u8 *>(&temp), nullptr, 1);
        StdWString_Freeze(name, nullptr);
        widen(0, 0, description, -1, name->ptr, count);
    }
    const u16 *const characters = name->ptr ? name->ptr : reinterpret_cast<const u16 *>((uintptr_t)0x006fd6c4);
    Crt().wcscmp(characters, reinterpret_cast<const u16 *>((uintptr_t)0x006feee8));   // (the answer unused, sic)
    import<u32(__stdcall *)(u32, u32)>(0x006fd080)(window, context);
}

void __fastcall ImeHost_InsertResult(u8 *host) {
    u8 *const ime = host + 0x1c;
    const u32 length = Crt().wcslen(reinterpret_cast<const u16 *>((uintptr_t)Obj_GetCOr6fd6c4(ime)));
    u16 *const text = reinterpret_cast<u16 *>(host + 0x1a8);
    if (text[0] == 0 && at<u16>(host, 0x1aa) == 1) {   // (ImeHost_OnWindow's mark: the window's text not fetched yet)
        Window705a20_GetText(at<u8 *>(host, 4), nullptr, text, 0x100);
        u8 *const window = at<u8 *>(host, 4);
        const s32 room = (s32)at<s16>(window, 0x13c) - (s16)Window705a50_TextLength(window);
        Ime_SetLimit(ime, nullptr, (u32)room);
        if (room == 0 && length != 0) {
            import<void(__stdcall *)(const char *)>(0x006fd1e0)(reinterpret_cast<const char *>((uintptr_t)0x0074fa8c));
            return;
        }
    }
    const s32 start = (s16)Window705a20_SelectionStart(at<u8 *>(host, 4));
    u16 joined[0x102], rest[0x102];
    Crt().wcscpy(rest, text);
    u16 *const cut = rest + start;
    const u16 kept = *cut;
    *cut = 0;
    Crt().wcscpy(joined, rest);
    Crt().wcscat(joined, reinterpret_cast<const u16 *>((uintptr_t)Obj_GetCOr6fd6c4(ime)));
    *cut = kept;
    Crt().wcscat(joined, cut);
    Window705a20_SetTextQuiet(at<u8 *>(host, 4), nullptr, joined);
    Window705a20_SetCursor(at<u8 *>(host, 4), nullptr, (u32)start, (u32)(start + (s32)length));
}

s32 __fastcall ImeHost_OnMessage(u8 *host, void *, u8 *window, u32 message, u32 a, u32 b) {
    u8 *const ime = host + 0x1c;
    if (message == 0x14) {
        at<u32>(host, 0) = 0;
        at<u32>(host, 8) = 0;
    } else if (message == 0x400) {
        Ime_SelectCandidate(ime, nullptr, b);
        at<u8>(ime, 0x184) = 0;
    } else if (message == 0x405) {
        const u32 page = at<u32>(host, 0x10);
        at<u32>(host, 0x14) = a;
        at<u32>(host, 0x18) = b;
        if (!at<u8>(ime, 0x184) || at<u32>(ime, 0x168) != a || page != at<u32>(ime, 0x16c)) Ime_SetCandidatePage(ime, nullptr, a, b, page);
    } else if (message == 0x401 && at<u32>(host, 0x10) != b) {
        const u32 first = at<u32>(host, 0x14), last = at<u32>(host, 0x18);
        at<u32>(host, 0x10) = b;
        if (!at<u8>(ime, 0x184) || at<u32>(ime, 0x168) != first || b != at<u32>(ime, 0x16c)) Ime_SetCandidatePage(ime, nullptr, first, last, b);
        ImeHost_InsertResult(host);
    }
    return Window_CallDefault(window, nullptr, message, a, b);
}

s32 __cdecl ImeHost_WindowProc(u8 *window, u32 message, u32 a, u32 b) {
    return ImeHost_OnMessage(reinterpret_cast<u8 *>((uintptr_t)0x007b4e38), nullptr, window, message, a, b);
}
}
