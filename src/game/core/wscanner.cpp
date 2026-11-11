#include "wscanner.h"

#include <cstdint>
#include <initializer_list>

#include "../files/charsets.h"
#include "buffers.h"
#include "gameheap.h"
#include "stdtree.h"
#include "string.h"
#include "trivial.h"
#include "wtraits.h"
#include "../ui/window7cc8c0handler.h"
#include "staticobjects.h"
#include "addrcalls.h"

extern "C" u32 __cdecl CRT_wcslen(const u16 *text);   // (online/chatlists)

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u16 *const kNullStr = reinterpret_cast<u16 *>(0x006fd6c4);  // _Nullstr

u8 &refs(const u16 *p) { return *reinterpret_cast<u8 *>((uintptr_t)p - 1); }

// std::wstring::assign(text, 0, npos) inlined (text a by-value argument): the text's block shared when it is whole, not empty and its
// count byte is below 0xfe; else _Grow(n, true), the characters copied and the end set.
void assign(StdWString *s, StdWString &text) {
    const u32 npos = *reinterpret_cast<const u32 *>(0x006fef00);
    const u32 n = npos >= text.len ? text.len : npos;
    if (s == &text) {
        StdWString_Erase(s, nullptr, n, npos);
        StdWString_Erase(s, nullptr, 0, 0);
    } else if (n != 0 && n == text.len && refs(text.ptr ? text.ptr : kNullStr) < 0xfe) {
        CountedBuf_Reset(reinterpret_cast<u8 *>(s), nullptr, 1);
        u16 *const ptr = text.ptr ? text.ptr : kNullStr;
        s->ptr = ptr;
        s->len = text.len;
        s->res = text.res;
        ++refs(ptr);
    } else if (StdWString_Grow(s, nullptr, n, 1)) {
        WTraits_Copy(s->ptr, text.ptr ? text.ptr : kNullStr, n);
        ImeWString_SetLength(reinterpret_cast<u8 *>(s), nullptr, n);
    }
}

// The by-value text destroyed: its count byte down by one, or (0 or 0xff) its block freed.
void destroy(StdWString &text) {
    if (text.ptr) {
        const u8 count = refs(text.ptr);
        if (count != 0 && count != 0xff)
            refs(text.ptr) = count - 1;
        else
            GameHeap_Free(reinterpret_cast<u8 *>(text.ptr) - 2);
    }
}

// The string's operator[] at the position: past the length, or with no block, _Nullstr's; else the string frozen first.
const u16 *charAt(u8 *scanner, u32 position) {
    StdWString *const s = reinterpret_cast<StdWString *>(scanner + 4);
    if (s->len < position || !s->ptr) return kNullStr;
    StdWString_Freeze(s, nullptr);
    return s->ptr + position;
}

bool inSet(u8 *scanner, u32 set, const u16 *c) {
    u8 *found;
    Tree16b_Find(scanner + set, nullptr, &found, c);
    return found != at<u8 *>(scanner, set + 4);
}

// The position moved past the characters of the set at +0x14, up to the text's length.
void skipSet14(u8 *scanner) {
    do {
        u8 *const head = at<u8 *>(scanner, 0x18);
        u8 *found;
        Tree16b_Find(scanner + 0x14, nullptr, &found, charAt(scanner, at<u32>(scanner, 0x38)));
        if (found == head) return;
    } while (++at<u32>(scanner, 0x38) < at<u32>(scanner, 0xc));
}

// out assigned [position, position + n) of the text, by a temporary.
void assignPart(u8 *scanner, StdWString *out, u32 n) {
    StdWString part;
    StdWString_Substr(reinterpret_cast<StdWString *>(scanner + 4), nullptr, &part, at<u32>(scanner, 0x38), n);
    StdWString_AssignSub(out, nullptr, &part, 0, *reinterpret_cast<const u32 *>(0x006fef00));
    destroy(part);
}

}  // namespace

extern "C" {

u8 *__fastcall WScanner_ConstructBase(u8 *scanner) {
    at<u32>(scanner, 8) = 0;
    at<u32>(scanner, 0xc) = 0;
    at<u32>(scanner, 0x10) = 0;
    at<u8>(scanner, 0x1c) = 0;
    StdTree_CtorG(scanner + 0x14);
    at<u8>(scanner, 0x2c) = 0;
    StdTree_CtorG(scanner + 0x24);
    at<u32>(scanner, 0) = (u32)(uintptr_t)Class702f18_vtable;
    for (const u32 set : {0x24u, 0x14u}) {
        for (const u16 c : {u16(0x20), u16(0xa), u16(9), u16(0xd)}) {
            u8 *result[2];
            StdMapG_Insert(scanner + set, nullptr, result, &c);
        }
    }
    return scanner;
}

void __fastcall WScanner_Destruct(u8 *scanner) {
    CountedList_ClearRelease(scanner + 0x44);
    WScanner_DestructBase(scanner);
}

void __fastcall WScanner_DestructBase(u8 *scanner) {
    at<u32>(scanner, 0) = (u32)(uintptr_t)Class702f18_vtable;
    StdTree_DtorG(scanner + 0x24);
    StdTree_DtorG(scanner + 0x14);
    if (u16 *const chars = at<u16 *>(scanner, 8)) {
        u8 &count = reinterpret_cast<u8 *>(chars)[-1];
        if (count == 0 || count == 0xff)
            GameHeap_Free(reinterpret_cast<u8 *>(chars) - 2);
        else
            --count;
    }
    at<u32>(scanner, 8) = 0;
    at<u32>(scanner, 0xc) = 0;
    at<u32>(scanner, 0x10) = 0;
}

void __fastcall WScanner_SetText(u8 *scanner, void *, StdWString text, u32 context, const u8 *flags) {
    at<u16>(scanner, 0x3d) = at<u16>(flags, 0);
    at<u32>(scanner, 0x40) = context;
    at<u8>(scanner, 0x3f) = flags[2];
    StdWString *const s = reinterpret_cast<StdWString *>(scanner + 4);
    assign(s, text);
    if (s->ptr && refs(s->ptr) != 0 && refs(s->ptr) != 0xff)
        StdWString_Grow(s, nullptr, s->len, 0);
    if (s->ptr)
        refs(s->ptr) = 0xff;
    u16 *const chars = s->ptr;
    at<u32>(scanner, 0x38) = 0;
    at<u16 *>(scanner, 0x34) = chars;
    destroy(text);
}

u8 *__fastcall WKeywordPair_Ctor(u8 *pair, void *, StdWString key, u32 v0, u32 v1, u32 v2, u32 v3) {
    StdWString *const s = reinterpret_cast<StdWString *>(pair);
    at<u8>(pair, 0) = static_cast<u8>(key.allocator);
    s->ptr = nullptr;
    s->len = 0;
    s->res = 0;
    assign(s, key);
    at<u32>(pair, 0x10) = v0;
    at<u32>(pair, 0x14) = v1;
    at<u32>(pair, 0x18) = v2;
    at<u32>(pair, 0x1c) = v3;
    destroy(key);
    return pair;
}

}  // extern "C"

namespace {
const WKeywordsEnv kKeywordsPorts = {Charset_FormatWide, GameAddress<u32(__cdecl *)(u32)>(0x0067ed00), GameAddress<void(__fastcall *)(u8 *)>(0x004ea7e0)};
const WKeywordsEnv *g_keywords = &kKeywordsPorts;
}  // namespace

void WKeywords_SetEnv(const WKeywordsEnv *env) { g_keywords = env ? env : &kKeywordsPorts; }

extern "C" {

void __fastcall WKeywords_Add(u8 *self, void *, u32 id, u32 handler) {
    u16 text[0x80];
    g_keywords->format(*reinterpret_cast<u8 **>((u32)(uintptr_t)&gLanguage_78b530 + 0x598), text, 0x80, id);
    // The original's allocator byte is an unset stack byte: it reaches the node's +8 (the wstring's allocator, never read). 0 here.
    const u8 allocator = 0;
    StdWString key;
    StdWString_CtorChars(&key, nullptr, text, &allocator);
    u8 pair[0x20];
    WKeywordPair_Ctor(pair, nullptr, key, handler, 0, 0, 0);
    u8 *const next = at<u8 *>(at<u8 *>(self, 0x48), 0);   // (null faults at the next line in both, so the node's "or itself" for it is left out)
    u8 *const prev = at<u8 *>(next, 4);
    u8 *const node = static_cast<u8 *>(GameHeap_Alloc(0x28));
    at<u8 *>(node, 0) = next;
    at<u8 *>(node, 4) = prev ? prev : node;
    at<u8 *>(next, 4) = node;
    at<u8 *>(at<u8 *>(node, 4), 0) = node;
    StdWString_CtorCopy(reinterpret_cast<StdWString *>(node + 8), nullptr, reinterpret_cast<StdWString *>(pair));
    for (u32 i = 0; i < 0x10; i += 4)
        at<u32>(node, 0x18 + i) = at<u32>(pair, 0x10 + i);
    ++at<u32>(self, 0x4c);
    destroy(*reinterpret_cast<StdWString *>(pair));
}

void __fastcall WKeywords_AddAll(u8 *self) {
    static const u32 kKeywords[][2] = {{0x00, 0x005e4b20}, {0x01, 0x005e4ca0}, {0x02, 0x005e4f00}, {0x03, 0x005e50a0}, {0x04, 0x005e5240}, {0x05, 0x005e53c0},
                                       {0x06, 0x005e5540}, {0x0c, 0x005e57c0}, {0x0d, 0x005e58b0}, {0x0e, 0x005e5aa0}, {0x0f, 0x005e5c90}, {0x10, 0x005e5c90},
                                       {0x12, 0x005e5e80}, {0x14, 0x005e6070}, {0x15, 0x005e6160}, {0x13, 0x005e6350}, {0x16, 0x005e6540}, {0x17, 0x005e6630}};
    for (const auto &keyword : kKeywords) {
        WKeywords_Add(self, nullptr, keyword[0], keyword[1]);
        WKeywords_Add(self, nullptr, keyword[0] + 0x2b, keyword[1]);
    }
}

}  // extern "C"

namespace {

AddrCalls g_calls;

void insertToken(u8 *scanner, u32 kind) {
    CountedPair token{};
    token.value = kind;
    u8 *const tokens = at<u8 *>(scanner, 0x50);
    CountedPairVector_InsertN(tokens, nullptr, at<u32>(tokens, 8), 1, &token);
}

u32 pushToken(u8 *scanner, u32 kind) {
    insertToken(scanner, kind);
    return Obj_CopyCToMark(scanner);
}

// A token of the text inserted at the end; its pair destroyed (the narrow string holds no block).
void pushPair(u8 *scanner, StdWString &copy) {
    CountedPair pair;
    CountedPair_CtorWide(&pair, nullptr, copy);
    u8 *const tokens = at<u8 *>(scanner, 0x50);
    CountedPairVector_InsertN(tokens, nullptr, at<u32>(tokens, 8), 1, &pair);
    CountedBuf_Reset(reinterpret_cast<u8 *>(&pair.wide), nullptr, 1);
}

void pushCopy(u8 *scanner, const StdWString *text) {
    StdWString copy{};
    at<u8>(&copy, 0) = at<u8>(text, 0);
    StdWString_AssignSub(&copy, nullptr, text, 0, *reinterpret_cast<const u32 *>(0x006fef00));
    pushPair(scanner, copy);
}

u8 keywordRest(u8 *scanner, u32 kind) {
    insertToken(scanner, kind);
    StdWString *const out = at<StdWString *>(scanner, 0x54);
    if (!WScanner_TakeRest(scanner, nullptr, out, nullptr)) return 0;
    pushCopy(scanner, out);
    return 1;
}

u8 keywordCounted(u8 *scanner, u32 kind) {
    insertToken(scanner, kind);
    StdWString *const out = at<StdWString *>(scanner, 0x54);
    u32 count;
    if (!WScanner_TakeRest(scanner, nullptr, out, &count)) return 0;
    if (count != 0) pushCopy(scanner, out);
    return 1;
}

u8 keywordWord(u8 *scanner, u32 kind) {
    insertToken(scanner, kind);
    StdWString word{};
    if (!WScanner_NextToken(scanner, nullptr, &word)) {
        destroy(word);
        return 0;
    }
    pushCopy(scanner, &word);
    const u8 result = (u8)Obj_CopyCToMark(scanner);
    destroy(word);
    return result;
}

}  // namespace

void WScanner_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

s32 __cdecl WString_CompareNoCase(const u8 *a, const u8 *b, u32 flag) {
    const auto end = [](const u8 *s) { return at<u16 *>(s, 4) ? at<u16 *>(s, 4) + at<u32>(s, 8) : nullptr; };
    const u16 *p = at<u16 *>(a, 4);
    const u16 *q = at<u16 *>(b, 4);
    while (p != end(a) && q != end(b)) {
        const u16 x = static_cast<u16>(g_keywords->upper(*p));
        const u16 y = static_cast<u16>(g_keywords->upper(*q));
        if (x != y)
            return x < y ? -1 : 1;
        ++p;
        ++q;
    }
    if (static_cast<u8>(flag) && q == end(b))
        return 0;
    const u32 n = at<u32>(a, 8);
    const u32 m = at<u32>(b, 8);
    return n == m ? 0 : n < m ? -1 : 1;
}

u8 *__fastcall WScanner_FindKeyword(u8 *scanner, void *, StdWString text) {
    u8 *found = nullptr;
    for (u8 *node = at<u8 *>(at<u8 *>(scanner, 0x40), 4); (node = at<u8 *>(node, 0)) != at<u8 *>(at<u8 *>(scanner, 0x40), 4);) {
        u8 *const key = node + 8;
        s32 difference;
        if (at<u8>(scanner, 0x3f))
            difference = WString_CompareNoCase(key, reinterpret_cast<const u8 *>(&text), 1);
        else if (at<u8>(scanner, 0x3d))
            difference = WString_CompareSub(key, nullptr, 0, at<u32>(key, 8), text.ptr ? text.ptr : kNullStr, text.len);
        else
            difference = WString_CompareNoCase(key, reinterpret_cast<const u8 *>(&text), 0);
        if (difference == 0) {
            found = key;
            break;
        }
    }
    destroy(text);
    return found;
}

s32 __fastcall WScanner_ReadKeywords(u8 *scanner) {
    using Handler = u8(__fastcall *)(u8 *);
    s32 count = 0;
    while (at<u32>(scanner, 0x38) < at<u32>(scanner, 0xc)) {
        while (inSet(scanner, 0x24, charAt(scanner, at<u32>(scanner, 0x38))) || inSet(scanner, 0x14, charAt(scanner, at<u32>(scanner, 0x38)))) {
            if (++at<u32>(scanner, 0x38) >= at<u32>(scanner, 0xc))
                break;
        }
        u32 n = 0;
        while (!inSet(scanner, 0x14, charAt(scanner, at<u32>(scanner, 0x38) + n))) {
            if (++n + at<u32>(scanner, 0x38) >= at<u32>(scanner, 0xc))
                break;
        }
        StdWString word;
        StdWString_Substr(reinterpret_cast<StdWString *>(scanner + 4), nullptr, &word, at<u32>(scanner, 0x38), n);
        u8 *const pair = WScanner_FindKeyword(scanner, nullptr, word);
        at<u32>(scanner, 0x38) += n;
        if (pair) {
            ++count;
            if (!at<Handler>(pair, 0x10)(scanner + at<u32>(pair, 0x14)))
                return -1;
        }
        if (at<u8>(scanner, 0x3e))
            return count;
    }
    return count;
}

u8 __fastcall WScanner_NextChar(u8 *scanner, void *, u16 *out) {
    const u32 position = at<u32>(scanner, 0x38);
    if (position >= at<u32>(scanner, 0xc))
        return 0;
    at<u32>(scanner, 0x38) = position + 1;
    *out = *charAt(scanner, position);
    return 1;
}

u8 *__fastcall WScanner_DeletingDtor(u8 *scanner, void *, u32 flags) {
    g_keywords->baseDtor(scanner);
    if (flags & 1) GameHeap_Free(scanner);
    return scanner;
}

u8 *__fastcall WScanner_Construct(u8 *scanner, void *, u8 *tokens, StdWString *out) {
    WScanner_ConstructBase(scanner);
    at<u8>(scanner, 0x44) = static_cast<u8>(reinterpret_cast<uintptr_t>(tokens));
    u8 *const sentinel = static_cast<u8 *>(GameHeap_Alloc(0x28));
    at<u8 *>(sentinel, 0) = sentinel;
    at<u8 *>(sentinel, 4) = sentinel;
    at<u8 *>(scanner, 0x48) = sentinel;
    at<u32>(scanner, 0x4c) = 0;
    at<u8 *>(scanner, 0x50) = tokens;
    at<StdWString *>(scanner, 0x54) = out;
    at<u32>(scanner, 0) = (u32)(uintptr_t)Class702f14_vtable;
    u8 *const first = at<u8 *>(tokens, 4);
    u8 *const last = at<u8 *>(tokens, 8);
    for (u8 *token = first; token != last; token += 0x24) {
        CountedBuf_Reset(token + 0x10, nullptr, 1);
        StdString_Tidy(reinterpret_cast<StdString *>(token), nullptr, 1);
    }
    at<u8 *>(tokens, 8) = first;
    const u16 *const text = reinterpret_cast<const u16 *>(gWindow7cc8c0handler_78a374);
    u32 n = 0;
    while (text[n])
        ++n;
    StdWString_AssignChars(out, nullptr, text, n);
    WKeywords_AddAll(scanner);
    return scanner;
}

void __fastcall WScanner_Parse(u8 *scanner, void *, StdWString text) {
    const u8 flags[3] = {0, 1, 1};
    StdWString copy;
    StdWString_CtorCopy(&copy, nullptr, &text);
    WScanner_SetText(scanner, nullptr, copy, reinterpret_cast<u32>(scanner + 0x44), flags);
    WScanner_SpanInSet14(scanner);
    u16 c;
    if (WScanner_NextChar(scanner, nullptr, &c) && c == '/') {
        if (WScanner_ReadKeywords(scanner) == 0)
            insertToken(scanner, 0x18);
    } else {
        StdWString_AssignSub(at<StdWString *>(scanner, 0x54), nullptr, &text, 0, *reinterpret_cast<const u32 *>(0x006fef00));
        insertToken(scanner, 0);
        pushCopy(scanner, &text);
    }
    destroy(text);
}

u32 __fastcall WScanner_PushToken0c(u8 *scanner) { return pushToken(scanner, 0xc); }
u32 __fastcall WScanner_PushToken12(u8 *scanner) { return pushToken(scanner, 0x12); }
u32 __fastcall WScanner_PushToken15(u8 *scanner) { return pushToken(scanner, 0x15); }
u32 __fastcall WScanner_PushToken16(u8 *scanner) { return pushToken(scanner, 0x16); }

u8 __fastcall WScanner_Keyword00(u8 *scanner) { return keywordRest(scanner, 0); }
u8 __fastcall WScanner_Keyword04(u8 *scanner) { return keywordRest(scanner, 4); }
u8 __fastcall WScanner_Keyword05(u8 *scanner) { return keywordRest(scanner, 5); }
u8 __fastcall WScanner_Keyword02(u8 *scanner) { return keywordCounted(scanner, 2); }
u8 __fastcall WScanner_Keyword03(u8 *scanner) { return keywordCounted(scanner, 3); }
u8 __fastcall WScanner_Keyword0d(u8 *scanner) { return keywordWord(scanner, 0xd); }
u8 __fastcall WScanner_Keyword0e(u8 *scanner) { return keywordWord(scanner, 0xe); }
u8 __fastcall WScanner_Keyword0f(u8 *scanner) { return keywordWord(scanner, 0xf); }
u8 __fastcall WScanner_Keyword11(u8 *scanner) { return keywordWord(scanner, 0x11); }
u8 __fastcall WScanner_Keyword13(u8 *scanner) { return keywordWord(scanner, 0x13); }
u8 __fastcall WScanner_Keyword14(u8 *scanner) { return keywordWord(scanner, 0x14); }

u8 __fastcall WScanner_Keyword01(u8 *scanner) {
    insertToken(scanner, 1);
    StdWString word{};
    if (!WScanner_NextToken(scanner, nullptr, &word)) return 1;   // (the word then holds no block)
    pushCopy(scanner, &word);
    StdWString *const out = at<StdWString *>(scanner, 0x54);
    u8 result = 0;
    if (WScanner_TakeRest(scanner, nullptr, out, nullptr)) {
        pushCopy(scanner, out);
        result = 1;
    }
    destroy(word);
    return result;
}

u8 __fastcall WScanner_Keyword06(u8 *scanner) {
    CountedPair token{};
    token.value = 6;
    u8 *const tokens = at<u8 *>(scanner, 0x50);
    CountedPairVector_Insert(tokens, nullptr, at<u32>(tokens, 8), &token);
    StdWString word{};
    if (WScanner_NextToken(scanner, nullptr, &word)) {
        pushCopy(scanner, &word);
    } else {   // (the rest below then throws in the game: the position is past the text's end)
        const u16 *const text = reinterpret_cast<const u16 *>(gWindow7cc8c0handler_78a374);
        u32 n = 0;
        while (text[n]) ++n;
        StdWString copy{};
        StdWString_AssignChars(&copy, nullptr, text, n);
        pushPair(scanner, copy);
    }
    StdWString *const out = at<StdWString *>(scanner, 0x54);
    u8 result = 0;
    if ((u8)g_calls.Th(0x005e7200, (u32)(uintptr_t)scanner, (u32)(uintptr_t)out, 0u)) {
        pushCopy(scanner, out);
        result = 1;
    }
    destroy(word);
    return result;
}

StdWString *__fastcall StdWString_Substr(const StdWString *s, void *, StdWString *out, u32 pos, u32 n) {
    *reinterpret_cast<u8 *>(&out->allocator) = static_cast<u8>(reinterpret_cast<uintptr_t>(out));   // (sic: the low byte of its own address, the "allocator" argument's slot)
    out->ptr = nullptr;
    out->len = 0;
    out->res = 0;
    StdWString_AssignSub(out, nullptr, s, pos, n);
    return out;
}

u8 __fastcall WScanner_NextToken(u8 *scanner, void *, StdWString *out) {
    skipSet14(scanner);
    u32 n = 0;
    for (;;) {
        const u32 position = at<u32>(scanner, 0x38);
        if (inSet(scanner, 0x24, charAt(scanner, n + position)) || n + position >= at<u32>(scanner, 0xc)) break;
        ++n;
    }
    if (n + at<u32>(scanner, 0x38) > at<u32>(scanner, 0xc)) return 0;
    assignPart(scanner, out, n);
    at<u32>(scanner, 0x38) += n;
    return 1;
}

u8 __fastcall WScanner_TakeRest(u8 *scanner, void *, StdWString *out, u32 *count) {
    skipSet14(scanner);
    if (count) *count = at<u32>(scanner, 0xc) - at<u32>(scanner, 0x38);
    assignPart(scanner, out, at<u32>(scanner, 0xc) - at<u32>(scanner, 0x38));
    at<u32>(scanner, 0x38) = at<u32>(scanner, 0xc);
    return 1;
}

s32 __fastcall WScanner_SpanInSet14(u8 *scanner) {
    StdWString *const s = reinterpret_cast<StdWString *>(scanner + 4);
    s32 count = 0;
    for (;;) {
        const u32 pos = at<u32>(scanner, 0x38);
        u8 *const head = at<u8 *>(scanner, 0x18);
        const u16 *c = kNullStr;
        if (s->len >= pos && s->ptr) {
            if (refs(s->ptr) != 0 && refs(s->ptr) != 0xff)
                StdWString_Grow(s, nullptr, s->len, 0);
            if (s->ptr)
                refs(s->ptr) = 0xff;
            c = reinterpret_cast<const u16 *>((uintptr_t)s->ptr + 2 * pos);
        }
        u8 *const node = Tree16b_LowerBound(scanner + 0x14, nullptr, c);
        if (node == head || *c < at<u16>(node, 0xc))
            return count;
        at<u32>(scanner, 0x38) = pos + 1;
        ++count;
        if (pos + 1 >= at<u32>(scanner, 0xc))
            return -1;
    }
}

}  // extern "C"
