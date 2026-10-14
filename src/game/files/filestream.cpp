// FileStream_OpenPath: see filestream.h.
#include "filestream.h"

#include <cstdint>

#include "../core/string.h"

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const FileStreamEnv kGameEnv = {
    at<s32(__stdcall *const *)(u32)>(0x006fd154),
};
const FileStreamEnv *g_env = &kGameEnv;

const char kSeparators[] = "\\/";  // 0x775704
void *const *const kFolderHandleVtable = at<void *const *>(0x006fd9e8);

}  // namespace

void FileStream_SetEnv(const FileStreamEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

s32 *__fastcall FileStream_OpenPath(StreamBase *f, void *, s32 *result, const char *path, u32 mode) {
    String s;
    String_CtorStr(&s, nullptr, path);
    // The last separator that isn't a DBCS character's trail byte; cut is past it.
    bool found = false;
    s32 cut = 20000;
    s32 before = -1;
    while (!found) {
        cut = String_ReverseFindOneOf(&s, nullptr, kSeparators, static_cast<u32>(before));
        if (cut > 0 && !(*g_env->isLeadByte)(static_cast<u8>(s.data[cut - 1]))) {
            ++cut;
            found = true;
        } else {
            before = cut - 1;
        }
        if (cut <= 0) break;
    }
    if (cut == -1) cut = 0;

    StringView view;
    String name;
    String_CtorCopy(&name, nullptr, String_From(&s, nullptr, &view, static_cast<u32>(cut)));
    String_Assign(&s, nullptr, String_Left(&s, nullptr, &view, static_cast<u32>(cut)));

    Handle folder;
    Handle_Ctor(&folder);
    folder.vtable = kFolderHandleVtable;
    s32 status;
    if (*Handle_Open28(&folder, nullptr, &status, reinterpret_cast<u32>(s.data)) >= 0 &&
        *Handle_Vfn3(&folder, nullptr, &status, reinterpret_cast<u32>(f), reinterpret_cast<u32>(name.data), mode,
                     0) >= 0) {
        *result = 0;
    } else {
        *result = -1;
    }
    Handle_Dtor(&folder);
    String_Dtor(&name);
    String_Dtor(&s);
    return result;
}

}  // extern "C"
