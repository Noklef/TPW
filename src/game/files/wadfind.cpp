// The WAD directory's finder: see wadfind.h.
#include "wadfind.h"

#include <cstdint>

#include "../core/vcall.h"

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const WadFindEnv kGameEnv = {
    at<void *(__cdecl *)(u32)>(0x0067acc0),
    at<void(__cdecl *)(void *)>(0x0067acb0),
    at<s32(__cdecl *)(s32)>(0x0067f1a0),
};
const WadFindEnv *g_env = &kGameEnv;

const void *const kFinderListVtable = at<const void *>(0x0070393c);
const char kSeparators[] = "\\/";  // 0x775704

const StringView *view(const String *s) { return reinterpret_cast<const StringView *>(s); }
const String *as_string(const StringView *v) { return reinterpret_cast<const String *>(v); }

u32 length(const char *s) {
    u32 n = 0;
    while (s[n]) ++n;
    return n;
}

// The CRT's strncmp (0x67c250), strstr (0x67c080) and strncpy (0x67ae00).
s32 compare(const char *a, const char *b, u32 n) {
    for (; n; --n, ++a, ++b) {
        const u8 x = static_cast<u8>(*a), y = static_cast<u8>(*b);
        if (x != y) return x < y ? -1 : 1;
        if (!x) break;
    }
    return 0;
}

const char *find(const char *s, const char *what) {
    if (!*what) return s;
    for (; *s; ++s) {
        u32 i = 0;
        while (what[i] && s[i] == what[i]) ++i;
        if (!what[i]) return s;
    }
    return nullptr;
}

void copy(char *dst, const char *src, u32 n) {
    for (; n && *src; --n) *dst++ = *src++;
    for (; n; --n) *dst++ = 0;
}

// The list's new last node, holding a copy of text.
void push_back(StringList *l, const String *text) {
    StringListNode *n = static_cast<StringListNode *>(g_env->alloc(sizeof(StringListNode)));
    if (n) {
        String arg;
        String_CtorCopy(&arg, nullptr, view(text));
        n = StringListNode_Ctor(n, nullptr, arg);
    }
    n->prev = l->tail;
    if (l->tail)
        l->tail->next = n;
    else
        l->head = n;
    l->tail = n;
    ++l->count;
}

// An exact pattern's hash: toupper of each char, weighted 1, 2, 4, 8 by fours, the last
// 1-3 chars as bytes.
u32 name_hash(const char *p) {
    const u32 n = length(p);
    u32 h = 0;
    for (s32 groups = static_cast<s32>(n) >> 2; groups > 0; --groups, p += 4) {
        h += g_env->toupper(p[0]);
        h += g_env->toupper(p[1]) * 2;
        h += g_env->toupper(p[2]) * 4;
        h += g_env->toupper(p[3]) * 8;
    }
    for (u32 r = n & 3; r; --r) h += g_env->toupper(*p++) & 0xff;
    return h;
}

bool ends_with(const String *s, const char *what, u32 count) {
    StringView v;
    return String_EqualsStr(as_string(String_Right(s, nullptr, &v, count)), nullptr, what);
}

bool starts_with(const String *s, const char *what) {
    StringView v;
    return String_EqualsStr(as_string(String_Left(s, nullptr, &v, 1)), nullptr, what);
}

}  // namespace

void WadFind_SetEnv(const WadFindEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

StringListNode *__fastcall StringListNode_Ctor(StringListNode *n, void *, String text) {
    String_CtorCopy(&n->text, nullptr, view(&text));
    n->next = nullptr;
    n->prev = nullptr;
    String_Dtor(&text);
    return n;
}

String *__fastcall StringList_PopHead(StringList *l, void *, String *out) {
    StringListNode *const n = l->head;
    if (!n) return String_Ctor(out);
    l->head = n->next;
    if (l->head)
        l->head->prev = nullptr;
    else
        l->tail = nullptr;
    String text;
    String_CtorCopy(&text, nullptr, view(&n->text));
    --l->count;
    String_Dtor(&n->text);
    g_env->free(n);
    String_CtorCopy(out, nullptr, view(&text));
    String_Dtor(&text);
    return out;
}

s32 *__fastcall Wildcard_Compile(StringList *l, void *, s32 *result, const char *pattern) {
    while (l->count) {
        String old;
        StringList_PopHead(l, nullptr, &old);
        String_Dtor(&old);
    }
    String all, token;
    String_CtorStr(&all, nullptr, pattern);
    String_Ctor(&token);
    const char *p = all.data;
    while (*p != '*' && *p != '?' && *p) ++p;
    if (p > all.data) {
        StringView v;
        String_Assign(&token, nullptr, String_Left(&all, nullptr, &v, static_cast<u32>(p - all.data)));
        push_back(l, &token);
    }
    if (*p) {
        String_AssignChar(&token, nullptr, p++);
        while (*p) {
            if (*p != '*' && *p != '?') {
                String_AppendChar(&token, nullptr, p++);
                continue;
            }
            push_back(l, &token);
            String_AssignChar(&token, nullptr, p);
            if (*p == '*')
                while (p[1] == '?' || p[1] == '*') ++p;
            ++p;
        }
        push_back(l, &token);
    }
    *result = 0;
    String_Dtor(&token);
    String_Dtor(&all);
    return result;
}

u32 __fastcall Wildcard_Match(const StringList *l, void *, const char *text) { return Wildcard_MatchFrom(text, l->head); }

u32 __stdcall Wildcard_MatchFrom(const char *s, const StringListNode *n) {
    while (n) {
        const StringListNode *const token = n;
        n = n->next;
        const char *const t = token->text.data;
        const u32 len = token->text.length;
        if (*t == '*') {
            const char *const found = find(s, t + 1);
            if (!found) return 0;
            s = found + len - 1;
            continue;
        }
        if (*t == '?') {
            if (!*s || compare(s + 1, t + 1, len - 1)) return 0;
        } else if (compare(s, t, len)) {
            return 0;
        }
        s += len;
        if (!n) return *s == 0;
    }
    return 1;
}

WadFolder *__fastcall WadFolder_Find(WadFolder *f, void *, const String *path, String *out) {
    const s32 cut = String_FindOneOf(path, nullptr, kSeparators, 0);
    WadFolder *child;
    for (PtrListNode *n = f->folders.head;;) {
        if (!n) return nullptr;
        child = static_cast<WadFolder *>(n->item);
        n = n->next;
        StringView v;
        if (String_EqualsStr(as_string(String_Left(path, nullptr, &v, static_cast<u32>(cut))), nullptr, child->name))
            break;
    }
    if (cut == -1) {
        String_Append(out, nullptr, view(path));
        const char separator = '\\';
        String_AppendChar(out, nullptr, &separator);
        return child;
    }
    StringView v;
    String part;
    String_CtorCopy(&part, nullptr, String_Left(path, nullptr, &v, static_cast<u32>(cut + 1)));
    String_Append(out, nullptr, view(&part));
    String_Dtor(&part);
    String_CtorCopy(&part, nullptr, String_From(path, nullptr, &v, static_cast<u32>(cut + 1)));
    WadFolder *const found = WadFolder_Find(child, nullptr, &part, out);
    String_Dtor(&part);
    return found;
}

WadFinder *__fastcall WadFinder_Ctor(WadFinder *f, void *, WadFolder **root, const String *pattern, u32 *record) {
    String_Ctor(&f->pattern);
    String_Ctor(&f->path);
    f->a.head = nullptr;
    f->a.tail = nullptr;
    f->a.count = 0;
    f->a.vtable = kFinderListVtable;
    f->b.head = nullptr;
    f->b.tail = nullptr;
    f->b.count = 0;
    f->b.vtable = kFinderListVtable;
    WadFinder_Start(f, nullptr, root, pattern, record);
    return f;
}

void __fastcall WadFinder_Start(WadFinder *f, void *, WadFolder **root, const String *pattern, u32 *record) {
    f->folder = *root;
    f->cursor = nullptr;
    f->current = nullptr;
    f->folders = 0;
    if (!f->folder) return;
    vcall<String *>(&f->path, 0x14, "");  // (through the vtable, as the original: String_AssignStr)
    u32 count = pattern->length, start = 0;
    if (ends_with(pattern, "\\", 1) || ends_with(pattern, "/", 1)) --count;
    if (starts_with(pattern, "\\") || starts_with(pattern, "/")) {
        start = 1;
        --count;
    }
    StringView v;
    String s;
    String_CtorCopy(&s, nullptr, String_Mid(pattern, nullptr, &v, start, count));
    WadDir_Normalize(reinterpret_cast<TextWriter *>(&s));
    const char star = '*', query = '?';
    if (String_FindChar(&s, nullptr, &star, 0) == -1 && String_FindChar(&s, nullptr, &query, 0) == -1) {
        f->exact = 1;
        f->hash = name_hash(s.data);
    } else {
        f->exact = 0;
        f->hash = 0;
    }
    const s32 cut = String_ReverseFindOneOf(&s, nullptr, kSeparators, 0xffffffff);
    if (cut != -1) {
        String folder;
        String_CtorCopy(&folder, nullptr, String_Left(&s, nullptr, &v, static_cast<u32>(cut)));
        f->folder = WadFolder_Find(f->folder, nullptr, &folder, &f->path);
        String_Dtor(&folder);
    }
    if (f->folder) {
        vcall<String *>(&f->pattern, 0x1c, String_From(&s, nullptr, &v, static_cast<u32>(cut + 1)));  // (String_Assign)
        s32 r;
        if (*Wildcard_Compile(&f->a, nullptr, &r, f->pattern.data) >= 0) {
            f->dotStar = 0;
            bool ok = true;
            if (ends_with(&f->pattern, ".*", 2)) {
                f->dotStar = 1;
                String stem;
                String_CtorCopy(&stem, nullptr, String_Left(&f->pattern, nullptr, &v, f->pattern.length - 2));
                ok = *Wildcard_Compile(&f->b, nullptr, &r, stem.data) >= 0;
                String_Dtor(&stem);
            }
            if (ok) {
                f->cursor = f->folder->folders.head;
                f->folders = 1;
                WadFinder_Next(f);
                if (record)
                    *record = f->current && !f->folders ? static_cast<WadFile *>(f->current->item)->offset : 0;
            }
        }
    }
    String_Dtor(&s);
}

void __fastcall WadFinder_Next(WadFinder *f) {
    if (!f->folder) return;
    if (f->folders) {
        while (PtrListNode *const n = f->cursor) {
            f->cursor = n->next;
            const char *const name = static_cast<WadFolder *>(n->item)->name;
            if (Wildcard_Match(&f->a, nullptr, name) || (f->dotStar && Wildcard_Match(&f->b, nullptr, name))) {
                f->current = n;
                return;
            }
        }
        f->folders = 0;
        f->current = nullptr;
        f->cursor = f->folder->files.head;
    }
    while (PtrListNode *const n = f->cursor) {
        f->cursor = n->next;
        const WadFile *const e = static_cast<WadFile *>(n->item);
        const bool hit = f->exact ? e->hash == f->hash && String_EqualsStr(&f->pattern, nullptr, e->name)
                                  : Wildcard_Match(&f->a, nullptr, e->name) ||
                                        (f->dotStar && Wildcard_Match(&f->b, nullptr, e->name));
        if (hit) {
            f->current = n;
            return;
        }
    }
    f->current = nullptr;
}

u32 __fastcall WadFinder_Done(const WadFinder *f) {
    const u32 current = static_cast<u32>(reinterpret_cast<std::uintptr_t>(f->current));
    if (current) return current & ~0xffu;
    return f->folders ? 0 : 1;
}

void __fastcall WadFinder_Fill(const WadFinder *f, void *, DirEntry *e) {
    if (!f->current) return;
    const WadFile *const item = static_cast<WadFile *>(f->current->item);  // a folder's name is first too
    e->writeTime[0] = e->writeTime[1] = 0;
    e->accessTime[0] = e->accessTime[1] = 0;
    e->creationTime[0] = e->creationTime[1] = 0;
    if (f->folders) {
        e->attributes = 0x10;
        e->size = 0;
    } else {
        e->attributes = 0x81;
        e->size = item->size;
    }
    e->name = e->path + f->path.length;
    copy(e->path, f->path.data, f->path.length);
    copy(e->path + f->path.length, item->name, length(item->name) + 1);
}

s32 *__fastcall WadDir_FindFirst(WadDir *d, void *, s32 *result, DirEntry *e, const String *pattern) {
    WadFinder_Start(d->finder, nullptr, &d->root, pattern, nullptr);
    if (static_cast<u8>(WadFinder_Done(d->finder))) {
        *result = -1;
        return result;
    }
    WadFinder_Fill(d->finder, nullptr, e);
    *result = 0;
    return result;
}

s32 *__fastcall WadDir_FindNext(WadDir *d, void *, s32 *result, DirEntry *e) {
    WadFinder_Next(d->finder);
    if (static_cast<u8>(WadFinder_Done(d->finder))) {
        *result = -1;
        return result;
    }
    WadFinder_Fill(d->finder, nullptr, e);
    *result = 0;
    return result;
}

}  // extern "C"
