// The system-info object's drive queries: see drives.h.
#include "drives.h"

#include <cstdint>
#include <cstring>

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const DrivesEnv kGameEnv = {
    at<void(__fastcall *)(SystemInfo *)>(0x0055e470),
    at<u32(__stdcall *const *)(u32, char *)>(0x006fd0a0),
    at<u32(__stdcall *const *)(const char *)>(0x006fd09c),
    at<s32(__stdcall *const *)(const char *, char *, u32, u32 *, u32 *, u32 *, char *, u32)>(0x006fd0a4),
    at<String *(__fastcall *)(String *, void *, const char *)>(0x005f60b0),
    at<String *(__fastcall *)(String *, void *, const StringView *)>(0x005f6040),
    at<void(__fastcall *)(String *)>(0x005f61e0),
    at<void *(__cdecl *)(u32)>(0x0067acc0),
    at<StringListNode *(__fastcall *)(StringListNode *, void *, String)>(0x005f8a50),
    at<s32(__cdecl *)(const char *, const char *)>(0x0067df40),
};
const DrivesEnv *g_env = &kGameEnv;

const u32 kDriveStringsSize = 0x100;
const u32 kLabelSize = 0x14;

bool wanted(u32 types, u32 type) {
    return (types & 1 && type == 2) || (types & 2 && type == 3) || (types & 4 && type == 4) ||
           (types & 8 && type == 5);  // the original tests (2, fixed) a second time
}

}  // namespace

void Drives_SetEnv(const DrivesEnv *e) { g_env = e ? e : &kGameEnv; }

const DrivesEnv *Drives_GetEnv() { return g_env; }

extern "C" {

SystemInfo *__fastcall SystemInfo_Ctor(SystemInfo *s) {
    g_env->cpuCtor(s);
    s->drives[0] = s->drives[1] = s->drives[2] = s->drives[3] = 0;
    return s;
}

s32 *__fastcall SystemInfo_ListDrives(SystemInfo *, void *, s32 *out, StringList *list, u32 types) {
    char roots[kDriveStringsSize];
    const u32 n = (*g_env->getLogicalDriveStrings)(kDriveStringsSize - 1, roots);
    if (n == 0 || n >= kDriveStringsSize) {
        *out = -1;
        return out;
    }
    for (const char *p = roots; *p; p += std::strlen(p) + 1) {
        if (!wanted(types, (*g_env->getDriveType)(p))) continue;
        String root;
        String *const made = g_env->stringCtor(&root, nullptr, p);
        void *const block = g_env->alloc(sizeof(StringListNode));
        StringListNode *node = nullptr;
        if (block) {
            String text;
            g_env->stringCopy(&text, nullptr, reinterpret_cast<const StringView *>(made));
            node = g_env->nodeCtor(static_cast<StringListNode *>(block), nullptr, text);  // it frees text
        }
        node->prev = list->tail;  // a null node is written through (sic)
        if (list->tail)
            list->tail->next = node;
        else
            list->head = node;
        list->tail = node;
        ++list->count;
        g_env->stringDtor(&root);
    }
    *out = 0;
    return out;
}

u8 __fastcall SystemInfo_FindVolume(SystemInfo *s, void *, const StringList *list, const char *label) {
    for (const StringListNode *n = list->head; n;) {
        const StringListNode *const node = n;
        n = n->next;
        char name[kLabelSize];
        s32 result;
        if (*SystemInfo_VolumeLabel(s, nullptr, &result, node->text.data, name, kLabelSize) >= 0 &&
            g_env->stricmp(name, label) == 0)
            return SystemInfo_DriveLetter(s, nullptr, node->text.data);
    }
    return 0;
}

u8 __fastcall SystemInfo_DriveLetter(SystemInfo *, void *, const char *path) {
    return path[1] == ':' ? static_cast<u8>(path[0]) : 0;
}

s32 *__fastcall SystemInfo_VolumeLabel(SystemInfo *, void *, s32 *out, const char *root, char *label, u32 size) {
    *out = (*g_env->getVolumeInformation)(root, label, size, nullptr, nullptr, nullptr, nullptr, 0) ? 0 : -1;
    return out;
}

}  // extern "C"
