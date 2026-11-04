#include "globalsave.h"

#include <cstdint>

#include "balancefile.h"
#include "gameheap.h"
#include "savegame.h"
#include "stdlist.h"
#include "stdtree.h"
#include "string.h"
#include "vcall.h"
#include "../files/file.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

AddrCalls g_calls;

// A tree's constructor as the game inlines it (std::_Tree with a shared nil node; the STL lock around it left out): the nil made the first
// time (new(0, 1): parent 0, black; its left and right 0), its users counted, the head new(nil, 0) (red) with its left and right itself,
// the size 0.
void tree_ctor(u8 *tree, u32 nilAt, u32 usersAt, u8 *(__stdcall *make)(u32, u32)) {
    if (!global<u8 *>(nilAt)) {
        u8 *const nil = make(0, 1);
        global<u8 *>(nilAt) = nil;
        at<u32>(nil, 0) = 0;
        at<u32>(global<u8 *>(nilAt), 8) = 0;
    }
    ++global<u32>(usersAt);
    u8 *const head = make((u32)(uintptr_t)global<u8 *>(nilAt), 0);
    at<u8 *>(tree, 4) = head;
    at<u32>(tree, 0xc) = 0;
    at<u8 *>(head, 0) = head;
    at<u8 *>(at<u8 *>(tree, 4), 8) = at<u8 *>(tree, 4);
}

// One field written or read whole (Stream_Write / Stream_Read; a negative result or fewer bytes: false).
bool Field(Stream *stream, u8 *at, s32 count, bool save) {
    s32 result, done = 0;
    const s32 got = *(save ? Stream_Write(&result, stream, at, count, &done) : Stream_Read(&result, stream, at, count, &done));
    return got >= 0 && done == count;   // sic: the result is 0 or 1, never negative
}

// The current theme's sign name at +at given (the CRT's wcsncpy, 0x67b0d0, of 0x21 units: up to and with the NUL, then zeros) and
// mNameChanged set; nothing without a current theme.
void SetSignName(u8 *info, u32 at, const u16 *name) {
    u8 *const theme = ::at<u8 *>(info, 0x48);
    if (!theme)
        return;
    u16 *to = reinterpret_cast<u16 *>(theme + at);
    u32 count = 0x21;
    while (count && (*to++ = *name++) != 0)
        --count;
    if (count)
        while (--count)
            *to++ = 0;
    theme[0xb8] = 1;
}

// 0x5af4a0's count (inlined in 0x5af530, 0x5af5d0 and 0x5af680): the theme map's head re-read each step.
s32 CountSetFlags(const u8 *info) {
    s32 count = 0;
    for (u32 i = 0; i < 4; ++i)
        count += info[0x18 + i] != 0;
    u8 *node = at<u8 *>(at<u8 *const>(info, 0x3c), 0);
    while (node != at<u8 *const>(info, 0x3c)) {
        if (ThemeRecord_LoadGlobalSam(at<u8 *>(node, 0x20))) {
            s32 theme = 0;
            for (u32 i = 0; i < 6; ++i)
                theme += at<const u8 *>(node, 0x20)[i] != 0;
            count += theme;
        }
        StdTree_IncF(&node);
    }
    return count;
}

// 0x5af530's count as the mark setters inline it: GlobalSave_CountSet4 and the bytes +0x26, +0x27 set.
s32 CountWithExtras(const u8 *info) {
    s32 count = CountSetFlags(info);
    for (u32 k = 0; k < 2; ++k)
        count += info[0x26 + k] != 0;
    return count;
}

// The level after a mark (see 0x5af940 in the header): 1, 2 or 3.
u32 LevelReached(const u8 *info, s32 first) {
    if ((s32)((u32)(first / 3) + at<u32>(info, 0x20)) <= 0)
        return 1;
    if (CountWithExtras(info) % 3 != 0)
        return 1;
    u8 *node = at<u8 *>(at<u8 *const>(info, 0x3c), 0);
    while (node != at<u8 *const>(info, 0x3c)) {
        const s32 count = CountWithExtras(info);
        u8 *const record = at<u8 *>(node, 0x20);
        const s32 sam = ThemeRecord_LoadGlobalSam(record) ? at<s32>(at<u8 *>(record, 0x1c), 0x14) : -1;
        if ((s32)((u32)(count / 3) + at<u32>(info, 0x20)) == sam)
            return 3;
        StdTree_IncF(&node);
    }
    return 2;
}

}  // namespace

extern "C" {

u32 __fastcall ThemeSaveRecord_Serialize(u8 *record, void *, Stream *stream, u32 save, u32) {
    for (u32 i = 0; i < 6; ++i)
        if (!Field(stream, record + i, 1, save != 0))
            return 1;   // (Debug_Printf "File write failed" / "File read failed")
    for (u32 k = 0; k < 4; ++k) {
        if (!Field(stream, record + 6 + k, 1, save != 0))
            return 1;
        if (!Field(stream, record + 0xc + 4 * k, 4, save != 0))
            return 1;
    }
    if (save) {
        for (u32 k = 0; k < 0x21; ++k) {
            if (!Field(stream, record + 0x34 + 2 * k, 2, true))
                return 1;
            if (!Field(stream, record + 0x76 + 2 * k, 2, true))
                return 1;
        }
        if (!Field(stream, record + 0xb8, 1, true) || !Field(stream, record + 0xb9, 1, true))
            return 1;
        return 0;
    }
    for (u32 k = 0; k < 0x21; ++k) {
        if (Save_ReadU16(stream, record + 0x34 + 2 * k, "mSignNameA[i]", 1))
            return 1;
        if (Save_ReadU16(stream, record + 0x76 + 2 * k, "mSignNameB[i]", 1))
            return 1;
    }
    if (Save_ReadU8(stream, record + 0xb8, "mNameChanged", 1))
        return 1;
    if (Save_ReadU8(stream, record + 0xb9, "mAllResearchCompleted", 1))
        return 1;
    return 0;
}

u8 __fastcall GlobalSave_ThemeByte(const u8 *info, void *, u32 i) { return at<const u8 *>(info, 0x48)[i]; }

u8 __fastcall GlobalSave_Byte26(const u8 *info, void *, u32 i) { return info[0x26 + i]; }

void __fastcall GlobalSave_Inc20(u8 *info) { ++at<u32>(info, 0x20); }

u8 __fastcall GlobalSave_Byte24(const u8 *info) { return info[0x24]; }

void __fastcall GlobalSave_SetByte24(u8 *info, void *, u32 value) { info[0x24] = (u8)value; }

u8 __fastcall GlobalSave_Byte25(const u8 *info) { return info[0x25]; }

void __fastcall GlobalSave_SetByte25(u8 *info, void *, u32 value) { info[0x25] = (u8)value; }

void __fastcall GlobalSave_SetSignNameA(u8 *info, void *, const u16 *name) { SetSignName(info, 0x34, name); }

void __fastcall GlobalSave_SetSignNameB(u8 *info, void *, const u16 *name) { SetSignName(info, 0x76, name); }

void __fastcall GlobalSave_SetAllResearchCompleted(u8 *info) {
    u8 *const theme = at<u8 *>(info, 0x48);
    if (theme) theme[0xb9] = 1;
}

u8 __fastcall GlobalSave_Byte4c(const u8 *info) { return info[0x4c]; }

void __fastcall GlobalSave_ClearByte4c(u8 *info) { info[0x4c] = 0; }

void GlobalSave_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

u8 *__fastcall GlobalSave_ThemeMake(u8 *info, void *, const u8 *name) {
    u8 *const record = static_cast<u8 *>(GameHeap_Alloc(0xbc));
    if (record) {
        at<u32>(record, 0x1c) = 0;
        String *const held = reinterpret_cast<String *>(record + 0x20);
        String_Ctor(held);
        at<u32>(record, 0) = 0;
        at<u16>(record, 4) = 0;
        at<u32>(record, 6) = 0;
        vcall<void>(held, 0x18, name);
        at<u8>(record, 0xb8) = 0;
        at<u8>(record, 0xb9) = 0;
        at<u16>(record, 0x34) = 0;
        at<u16>(record, 0x76) = 0;
    }
    struct {
        String key;
        u8 *value;
    } pair;
    String_CtorCopy(&pair.key, nullptr, reinterpret_cast<const StringView *>(name));
    pair.value = record;
    u8 *result[2];
    StdMapF_Insert(info + 0x38, nullptr, result, reinterpret_cast<const u8 *>(&pair));
    String_Dtor(&pair.key);
    return record;
}

static u8 *theme_record(u8 *info, const u8 *name) {
    u8 *const tree = info + 0x38;
    u8 *const node = StdTreeF_LowerBound(tree, nullptr, name);
    const bool there = node != at<u8 *>(tree, 4) && !String_Less(reinterpret_cast<const String *>(name), nullptr, reinterpret_cast<const StringView *>(node + 0xc));
    return there ? at<u8 *>(node, 0x20) : GlobalSave_ThemeMake(info, nullptr, name);
}

u8 *__fastcall GlobalSave_ThemeGet(u8 *info, void *, const u8 *name) {
    u8 *const record = theme_record(info, name);
    return static_cast<u8>(g_calls.Th(0x005b11c0, (u32)(uintptr_t)record)) ? record : nullptr;   // ThemeRecord_LoadGlobalSam
}

u32 __fastcall GlobalSave_ThemeSelect(u8 *info, void *, const u8 *name) {
    u8 *const record = theme_record(info, name);
    at<u8 *>(info, 0x48) = record;
    return g_calls.Th(0x005b11c0, (u32)(uintptr_t)record);
}

void __fastcall GlobalSave_ClearThemes(u8 *info) {
    u8 *it = *at<u8 **>(info, 0x3c);
    while (it != at<u8 *>(info, 0x3c)) {
        if (u8 *const record = at<u8 *>(it, 0x20)) {
            ThemeRecord_Dtor(record);
            GameHeap_Free(record);
        }
        StdTree_IncF(&it);
    }
    u8 *const head = at<u8 *>(info, 0x3c);
    u8 *out;
    StdTree_EraseRangeF(info + 0x38, nullptr, &out, *reinterpret_cast<u8 **>(head), head);
}

u32 __fastcall GlobalSave_Serialize(u8 *info, void *, Stream *stream, u32 saving, u32 third) {
    const bool save = saving != 0;
    const u32 streamAt = (u32)(uintptr_t)stream;
    const u32 kOptions = 0x0078d8d8;
    for (u32 i = 0; i < 4; ++i)
        if (!Field(stream, info + 0x18 + i, 1, save))
            return 1;
    for (u32 i = 0; i < 2; ++i)
        if (!Field(stream, info + 0x26 + i, 1, save))
            return 1;
    if (!Field(stream, info + 0x1c, 4, save) || !Field(stream, info + 0x20, 4, save))
        return 1;
    if (!Field(stream, info + 0x24, 1, save) || !Field(stream, info + 0x25, 1, save) || !Field(stream, info + 0x4c, 1, save))
        return 1;
    if (save) {
        u32 count = at<u32>(info, 0x44);
        if (!Field(stream, reinterpret_cast<u8 *>(&count), 4, true))
            return 1;
        u8 *const themes = at<u8 *>(info, 0x3c);
        for (u8 *node = at<u8 *>(themes, 0); node != at<u8 *>(info, 0x3c); StdTree_IncF(&node)) {
            u32 length = at<u32>(node, 0x10);
            if (!Field(stream, reinterpret_cast<u8 *>(&length), 4, true))
                return 1;
            char *const text = static_cast<char *>(GameHeap_Alloc(length + 1));
            const char *const from = at<const char *>(node, 0x14);
            u32 size = 0;
            while (from[size]) ++size;
            for (u32 i = 0; i <= size; ++i) text[i] = from[i];
            for (u32 i = 0; i < length; ++i)
                if (!Field(stream, reinterpret_cast<u8 *>(text + i), 1, true))
                    return 1;
            GameHeap_Free(text);
            if (g_calls.Th(0x005b0d70, at<u32>(node, 0x20), streamAt, saving, third))   // ThemeSaveRecord_Serialize
                return 1;
        }
        if (g_calls.Th(0x00423e90, kOptions, streamAt, saving, third))
            return 1;
        u32 keys = at<u32>(info, 0x34);
        if (!Field(stream, reinterpret_cast<u8 *>(&keys), 4, true))
            return 1;
        u8 *const rides = at<u8 *>(info, 0x2c);
        for (u8 *node = at<u8 *>(rides, 0); node != at<u8 *>(info, 0x2c); StdTree_IncG(&node)) {
            u32 value = ((u32)(uintptr_t)node & 0xffff0000u) | at<u16>(node, 0xc);   // (the original's register: its high half is the node's)
            if (!Field(stream, reinterpret_cast<u8 *>(&value), 2, true))
                return 1;
        }
        return 0;
    }
    s32 count;
    if (!Field(stream, reinterpret_cast<u8 *>(&count), 4, false))
        return 1;
    for (s32 i = 0; i < count; ++i) {
        u32 length;
        if (!Field(stream, reinterpret_cast<u8 *>(&length), 4, false))
            return 1;
        char *const text = static_cast<char *>(GameHeap_Alloc(length + 1));
        for (u32 k = 0; k < length; ++k)
            if (!Field(stream, reinterpret_cast<u8 *>(text + k), 1, false))
                return 1;
        text[length] = 0;
        String name;
        String_CtorStr(&name, nullptr, text);
        GameHeap_Free(text);
        u8 *const record = static_cast<u8 *>(GameHeap_Alloc(0xbc));
        if (record) {
            at<u32>(record, 0x1c) = 0;
            String *const held = reinterpret_cast<String *>(record + 0x20);
            String_Ctor(held);
            vcall<void>(held, 0x18, &name);
        }
        if (g_calls.Th(0x005b0d70, (u32)(uintptr_t)record, streamAt, 0u, third)) {
            String_Dtor(&name);
            return 1;
        }
        struct {
            String key;
            u8 *value;
        } pair;
        String_CtorCopy(&pair.key, nullptr, reinterpret_cast<const StringView *>(&name));
        pair.value = record;
        u32 inserted[2];
        g_calls.Th(0x005b1620, (u32)(uintptr_t)(info + 0x38), inserted, &pair);
        String_Dtor(&pair.key);
        if ((u8)inserted[1] == 0) {
            String_Dtor(&name);
            return 1;
        }
        String_Dtor(&name);
    }
    if (g_calls.Th(0x00423e90, kOptions, streamAt, 0u, third))
        return 1;
    s32 keys;
    if (Save_ReadU32(stream, &keys, reinterpret_cast<const char *>((uintptr_t)0x007731b0), 1))
        return 1;
    for (s32 i = 0; i < keys; ++i) {
        if (Save_ReadU16(stream, &third, reinterpret_cast<const char *>((uintptr_t)0x007731a8), 1))
            return 1;
        u32 inserted[2];
        g_calls.Th(0x005b1650, (u32)(uintptr_t)(info + 0x28), inserted, &third);
    }
    return 0;
}

u8 *__fastcall GlobalSave_Construct(u8 *info) {
    u8 *const node = static_cast<u8 *>(GameHeap_Alloc(0x18));   // (not checked)
    at<u8 *>(node, 0) = node;
    at<u8 *>(node, 4) = node;
    at<u8 *>(info, 4) = node;
    at<u32>(info, 8) = 0;
    at<u8 *>(info, 0x10) = Node18_New(0, 0);
    at<u32>(info, 0x14) = 0;
    at<u8>(info, 0x30) = 0;
    tree_ctor(info + 0x28, 0x00f7c970, 0x00f7c96c, &Node14_New);
    at<u8>(info, 0x40) = 0;
    tree_ctor(info + 0x38, 0x00f7c968, 0x00f7c964, &Node28_New);
    at<u32>(info, 0x18) = 0;
    at<u16>(info, 0x1c) = 0;
    at<u16>(info, 0x26) = 0;
    at<u32>(info, 0x1c) = 0;
    at<u32>(info, 0x20) = 0;
    at<u8>(info, 0x24) = 0;
    at<u8>(info, 0x25) = 1;
    at<u8>(info, 0x4c) = 1;
    return info;
}

u8 *__cdecl GlobalSave_Instance() {
    u8 *&info = global<u8 *>(0x00f7c58c);
    if (!info) {
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x50));
        info = block ? GlobalSave_Construct(block) : nullptr;
    }
    return info;
}

u32 __stdcall GlobalSave_HasKey(u32 key) { return Obj_HasKey5af780(GlobalSave_Instance(), nullptr, key); }

s32 __fastcall GlobalSave_CountSet4(const u8 *info) { return CountSetFlags(info); }

s32 __fastcall GlobalSave_CountSet(const u8 *info) { return CountSetFlags(info) + (info[0x26] != 0) + (info[0x27] != 0); }

s32 __fastcall GlobalSave_CountSetLeft(const u8 *info) { return GlobalSave_CountSet(info) - at<s32>(info, 0x1c); }

s32 __fastcall GlobalSave_CountSetLevel(const u8 *info) { return GlobalSave_CountSet(info) / 3 + at<s32>(info, 0x20); }

u32 __fastcall GlobalSave_MarkTheme(u8 *info, void *, u32 i) {
    u8 *const theme = at<u8 *>(info, 0x48);
    if (theme[i] == 1)
        return 0;
    theme[i] = 1;
    return LevelReached(info, CountWithExtras(info));
}

u32 __fastcall GlobalSave_MarkExtra(u8 *info, void *, u32 i) {
    if (info[0x26 + i] == 1)
        return 0;
    info[0x26 + i] = 1;
    return LevelReached(info, CountWithExtras(info));
}

u32 __fastcall GlobalSave_MarkScore(u8 *info, void *, u32 i, s32 score) {
    if (info[0x18 + i] == 1) {
        if (at<u8 *>(info, 0x48)[6 + i] != 0)
            return 0;
        u8 *node = at<u8 *>(at<u8 *const>(info, 0x3c), 0);
        while (node != at<u8 *const>(info, 0x3c)) {
            u8 *const record = at<u8 *>(node, 0x20);
            if (record[6 + i] != 0) {
                const s32 best = record[6 + i] != 0 ? at<s32>(record, 0xc + 4 * i) : -1;
                if (score > best) {
                    record[6 + i] = 0;
                    u8 *const theme = at<u8 *>(info, 0x48);
                    theme[6 + i] = 1;
                    at<s32>(theme, 0xc + 4 * i) = score;
                    return 4;
                }
            }
            StdTree_IncF(&node);
        }
        return 0;
    }
    info[0x18 + i] = 1;
    u8 *const theme = at<u8 *>(info, 0x48);
    theme[6 + i] = 1;
    at<s32>(theme, 0xc + 4 * i) = score;
    return LevelReached(info, GlobalSave_CountSet(info));
}
}

extern "C" u32 __fastcall GlobalSave_WriteFile(u8 *info, void *, const char *path) {
    const auto status = [](u32 result) { return *reinterpret_cast<const s32 *>((uintptr_t)result); };
    u32 stream = (u32)(uintptr_t)path;   // (the path's own slot, sic)
    s32 result;
    if (status(g_calls.Cd(0x005f5ad0, &result, &stream, path, 0x40000010u)) < 0)   // FileStream_Open
        return 0;
    u32 version = 0xc, done = 0;
    const s32 wrote = status(g_calls.Cd(0x005f5e50, &result, stream, &version, 4u, &done));   // Stream_Write
    u32 saved = 0;
    if (wrote >= 0 && done == 4)
        saved = g_calls.Th(0x005aff50, (u32)(uintptr_t)info, stream, 1u, version) == 0;   // GlobalSave_Serialize
    g_calls.Cd(0x005f5b80, &result, stream);   // Stream_Close
    return saved;
}
