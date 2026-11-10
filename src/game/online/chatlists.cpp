// The chat object's list files: see chatlists.h.
#include "chatlists.h"

#include <cstdint>
#include <cstring>

#include "../core/buffers.h"
#include "../core/gameheap.h"
#include "../core/initializers.h"
#include "../core/stdstring.h"
#include "../core/stdtree.h"
#include "../core/stdwstring.h"
#include "../core/vcall.h"
#include "../core/wtraits.h"
#include "postcardsend.h"
#include "../core/iosbase.h"
#include "../core/wfilebuf.h"
#include "../core/imports.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename F> F game(u32 address) { return reinterpret_cast<F>((uintptr_t)address); }
const char *text_at(u32 address) { return reinterpret_cast<const char *>((uintptr_t)address); }

const ChatListsEnv kGameEnv = {
    game<u8 *(__fastcall *)(u8 *)>(0x005c7420),
    game<u32(__stdcall *)(u8 *)>(0x005c8980),
    game<u32(__cdecl *)(char *, u32, u32)>(0x0067a650),
    game<u8 *(__fastcall *)(u8 *, void *, u8 *, u32, u32, u32)>(0x004f1560),
    game<u8 *(__fastcall *)(u8 *, void *, u8 *, u32, u32)>(0x004f1be0),
    game<u8 *(__fastcall *)(u8 *, void *, u32)>(0x004f3720),
    game<u8 *(__fastcall *)(u8 *, void *, const char *, u32)>(0x004f3920),
    game<void(__fastcall *)(u8 *, void *, u32, s32)>(0x004f5990),
    game<void(__fastcall *)(u8 *)>(0x004f5ad0),
    game<void(__fastcall *)(u8 *)>(0x004f08c0),
    game<void(__fastcall *)(u8 *, void *, u32, u32)>(0x004f0110),
    game<void(__fastcall *)(u8 *, void *, u32, u32)>(0x004f1620),
    game<void(__fastcall *)(u8 *)>(0x004f0540),
    game<void(__fastcall *)(u8 *)>(0x00693830),
    game<void(__fastcall *)(u8 *)>(0x004f0100),
    game<void(__fastcall *)(u8 *)>(0x005f6a60),
    game<u32(__cdecl *)(u32)>(0x0067b620),
    game<u32(__cdecl *)(const char *, u32)>(0x00693a70),
    game<u32(__cdecl *)(u16 *, u32, u32)>(0x0067af00),
    game<u32(__cdecl *)(u8 *, const u16 *)>(0x004f6be0),
    game<u8 **(__fastcall *)(u8 *, void *, u8 **)>(0x004ef7f0),
    game<u8 *(__cdecl *)(u8 **, u32, u32)>(0x004f6a30),
    game<void(__fastcall *)(u8 **)>(0x004ef790),
    game<u32(__fastcall *)(u8 *, void *, u32)>(0x004f1640),
    game<u8 *(__fastcall *)(u8 *, void *, u16 *, u32, u32)>(0x004f1ca0),
    game<u32(__fastcall *)(u8 *, void *, u8 *, u32)>(0x004ee710),
    game<void(__fastcall *)(u8 *, void *, const u16 *, u32)>(0x004051d0),
    game<void(__fastcall *)(u8 *, void *, u32 *, const u8 *)>(0x004f03c0),
    game<void(__fastcall *)(u8 *, void *, u8 *)>(0x004eb450),
};
const ChatListsEnv *g_env = &kGameEnv;

constexpr u32 kFile = 0x50;   // a filebuf's file
constexpr u32 kIosSize = 0x94;
constexpr u32 kPathSize = 0x104;

u8 *ios_of(const u8 *stream) { return const_cast<u8 *>(stream) + at<u32>(at<const u8 *>(stream, 0), 4); }
void set_ios_vtable(u8 *stream, u32 vtable) { at<u32>(ios_of(stream), 0) = vtable; }

// The object 0xf81ebc (made once) and the online folder's WString; a narrow copy of the name and its suffix; the path from the WString's text.
void build_path(const u8 *self, const char *name, u8 *folder, StdString *narrow, char *path, u32 wide_suffix, u32 narrow_suffix) {
    if (!reinterpret_cast<u32 &>(gChatlists_f81ebc)) {
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x110));
        gChatlists_f81ebc = block ? g_env->singletonCtor(block) : nullptr;
    }
    g_env->onlineFolder(folder);
    vcall<u32>(folder, 0x28, 0x007cebd8u);
    const u32 text = at<u32>(self, 0xc);
    vcall<u32>(folder, 0x20, text ? text : 0x006fd6c4u);
    vcall<u32>(folder, 0x20, wide_suffix);
    StdString_AssignChars(narrow, nullptr, name, (u32)std::strlen(name));
    StdString_AppendChars(narrow, nullptr, text_at(narrow_suffix), (u32)std::strlen(text_at(narrow_suffix)));
    g_env->format(path, reinterpret_cast<uintptr_t>(&gPostcardsend_74f8c4), at<u32>(folder, 8));
}

// The file closed and the filebuf told so; false when there was none or fclose failed.
bool close_file(u8 *buffer) {
    const u32 file = at<u32>(buffer, kFile);
    if (!file || g_env->fclose(file)) return false;
    g_env->filebufInit(buffer, nullptr, 0, 2);
    return true;
}

void release_narrow(StdString *s) {
    if (s->ptr) {
        u8 *const count = reinterpret_cast<u8 *>(s->ptr) - 1;
        if (*count == 0 || *count == 0xff) GameHeap_Free(count);
        else --*count;
    }
    s->ptr = nullptr;
    s->len = 0;
    s->res = 0;
}

u8 save_list(u8 *self, const char *name, u32 tree_offset, void(__fastcall *advance)(u8 **), u32 wide_suffix, u32 narrow_suffix) {
    alignas(4) u8 stream[kIosSize];
    u8 *const buffer = stream + 4, *const ios = stream + 0x58;
    alignas(4) u8 folder[0x14];
    StdString narrow;
    char path[kPathSize];
    u16 line[0x20];
    at<u32>(stream, 0) = 0x00700630;
    Init_4f1bd0(ios);
    g_env->ostreamCtor(stream, nullptr, buffer, 0, 1, 0);
    g_env->filebufCtor(buffer, nullptr, 0);
    set_ios_vtable(stream, (u32)(uintptr_t)Class70062c_vtable);
    StdString_Tidy(&narrow, nullptr, 0);
    build_path(self, name, folder, &narrow, path, wide_suffix, narrow_suffix);
    if (!g_env->filebufOpen(buffer, nullptr, path, 0x12)) g_env->setState(ios_of(stream), nullptr, 2, 0);
    if (!at<u32>(buffer, kFile)) {
        g_env->wstringDtor(folder);
        StdString_Tidy(&narrow, nullptr, 1);
        g_env->ofstreamDtor(ios);
        at<u32>(ios, 0) = (u32)(uintptr_t)BasicIosW_vtable;
        g_env->iosBaseDtor(ios);
        return 0;
    }
    u8 *it = at<u8 *>(at<u8 *>(self, tree_offset), 0);
    while (it != at<u8 *>(self, tree_offset)) {
        const u32 text = at<u32>(it, 0x10);
        g_env->formatWide(line, reinterpret_cast<uintptr_t>(&gChatlists_75c6c8), text ? text : 0x00700624u);
        g_env->writeLine(stream, line);
        advance(&it);
    }
    if (!close_file(buffer)) g_env->clear(ios_of(stream), nullptr, at<u32>(ios_of(stream), 4) | 2, 0);
    g_env->wstringDtor(folder);
    release_narrow(&narrow);
    set_ios_vtable(stream, (u32)(uintptr_t)Class70062c_vtable);
    g_env->filebufDtor(buffer);
    set_ios_vtable(stream, (u32)(uintptr_t)Class7099ec_vtable);
    g_env->iosDtor(ios);
    return 1;
}

// Whether what getline answered is a stream still good.
bool stream_good(const u8 *got) {
    const u8 *const ios = got ? ios_of(got) : nullptr;
    return !(at<u8>(ios, 4) & 6) && ios;
}

void make_wide_string(u8 *string, const u16 *line) {
    CountedBuf_Reset(string, nullptr, 0);
    const u32 n = CRT_wcslen(line);
    if (StdWString_Grow(reinterpret_cast<StdWString *>(string), nullptr, n, 1)) {
        WTraits_Copy(at<u16 *>(string, 4), line, n);
        ImeWString_SetLength(string, nullptr, n);
    }
}

void open_ifstream(u8 *stream, u8 *buffer) {
    at<u32>(stream, 0) = 0x00700640;
    g_env->istreamCtor(stream, nullptr, buffer, 0, 0);
    g_env->filebufCtor(buffer, nullptr, 0);
    set_ios_vtable(stream, (u32)(uintptr_t)Class700638_vtable);
}

}  // namespace

void ChatLists_SetEnv(const ChatListsEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u32 __cdecl CRT_wcslen(const u16 *text) {
    const u16 *p = text;
    while (*p) ++p;
    return (u32)(p - text);
}

u8 __fastcall ChatLists_SaveBuddies(u8 *self, void *, const char *name) { return save_list(self, name, 0xe94, &StdTree_IncC, 0x0075c6dc, reinterpret_cast<uintptr_t>(&gChatLists_75c6d0)); }

u8 __fastcall ChatLists_SaveIgnored(u8 *self, void *, const char *name) { return save_list(self, name, 0xea4, &StdTree_IncE, 0x0075c6f4, reinterpret_cast<uintptr_t>(&gChatLists_75c6e8)); }

u8 __fastcall ChatLists_LoadBuddies(u8 *self, void *, const char *name) {
    if (!at<u32>(self, 0)) return 0;
    alignas(4) u8 stream[kIosSize];
    u8 *const buffer = stream + 8, *const ios = stream + 0x5c;
    alignas(4) u8 folder[0x14], wide[0x10];
    StdString narrow;
    char path[kPathSize];
    u16 line[0x20];
    Init_4ef830(ios);
    at<u32>(ios, 0) = (u32)(uintptr_t)BasicIosW_vtable;
    open_ifstream(stream, buffer);
    StdString_Tidy(&narrow, nullptr, 0);
    build_path(self, name, folder, &narrow, path, 0x0075c6dc, reinterpret_cast<uintptr_t>(&gChatLists_75c6d0));
    bool opened = false;
    if (!at<u32>(buffer, kFile)) {
        const u32 file = g_env->fopen(path, 1);
        if (file) {
            g_env->filebufInit(buffer, nullptr, file, 1);
            g_env->filebufRead(buffer);
            opened = true;
        }
    }
    if (!opened) g_env->setState(ios_of(stream), nullptr, 2, 0);
    for (;;) {
        u8 *locale;
        u8 *const facet = g_env->getFacet(g_env->shareLocale(ios_of(stream), nullptr, &locale), 0, 1);
        g_env->releaseLocale(&locale);
        const u32 delimiter = vcall<u32>(facet, 0x28, 10u);
        if (!stream_good(g_env->getline(stream, nullptr, line, 0x20, delimiter))) break;
        make_wide_string(wide, line);
        g_env->setBuddy(self, nullptr, wide, 0);
        CountedBuf_Reset(wide, nullptr, 1);
    }
    if (!close_file(buffer)) g_env->setState(ios_of(stream), nullptr, 2, 0);
    g_env->wstringDtor(folder);
    StdString_Tidy(&narrow, nullptr, 1);
    set_ios_vtable(stream, (u32)(uintptr_t)Class700638_vtable);
    g_env->filebufDtor(buffer);
    set_ios_vtable(stream, (u32)(uintptr_t)Class7099dc_vtable);
    at<u32>(ios, 0) = (u32)(uintptr_t)BasicIosW_vtable;
    g_env->iosBaseDtor(ios);
    return 1;
}

u8 __fastcall ChatLists_LoadIgnored(u8 *self, void *, const char *name) {
    if (!at<u32>(self, 0)) return 0;
    alignas(4) u8 stream[kIosSize];
    u8 *const buffer = stream + 8, *const ios = stream + 0x5c;
    alignas(4) u8 folder[0x14], wide[0x10], key[0x10], other[0x10];
    StdString narrow;
    char path[kPathSize];
    u16 line[0x20];
    Init_4f1bd0(ios);
    open_ifstream(stream, buffer);
    StdString_Tidy(&narrow, nullptr, 0);
    build_path(self, name, folder, &narrow, path, 0x0075c6f4, reinterpret_cast<uintptr_t>(&gChatLists_75c6e8));
    if (!g_env->filebufOpen(buffer, nullptr, path, 1)) g_env->clear(ios_of(stream), nullptr, at<u32>(ios_of(stream), 4) | 2, 0);
    const auto notify = *reinterpret_cast<u32(__fastcall *const *)(u32, u32, u8 *, u8 *)>((uintptr_t)IMPORT(Weachatr_Chat_Ignore));
    while (stream_good(g_env->getline(stream, nullptr, line, 0x20, g_env->widen(ios_of(stream), nullptr, 10)))) {
        CountedBuf_Reset(wide, nullptr, 0);
        g_env->assign(wide, nullptr, line, CRT_wcslen(line));
        WStdString_CtorCopy(reinterpret_cast<StdWString *>(key), nullptr, reinterpret_cast<const StdWString *>(wide));
        u32 found;
        g_env->findIgnored(self + 0xea0, nullptr, &found, key);
        CountedBuf_Reset(key, nullptr, 1);
        if (found == at<u32>(self, 0xea4)) g_env->toggleIgnore(self, nullptr, wide);
        CountedBuf_Reset(wide, nullptr, 1);
        CountedBuf_Reset(other, nullptr, 0);
        g_env->assign(other, nullptr, line, CRT_wcslen(line));
        notify(at<u32>(self, 0), 0, other, self);
        CountedBuf_Reset(other, nullptr, 1);
    }
    if (!close_file(buffer)) g_env->setState(ios_of(stream), nullptr, 2, 0);
    g_env->wstringDtor(folder);
    StdString_Tidy(&narrow, nullptr, 1);
    set_ios_vtable(stream, (u32)(uintptr_t)Class700638_vtable);
    g_env->filebufDtor(buffer);
    set_ios_vtable(stream, (u32)(uintptr_t)Class7099dc_vtable);
    g_env->iosDtor(ios);
    return 1;
}
}
