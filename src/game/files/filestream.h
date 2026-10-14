// FileStream_OpenPath (0x5f7ed0; FileStream vtable 0x6fd930 +0x24): opens a file into a stream by
// splitting its path at the last separator ('\' or '/', not the trail byte of a DBCS character),
// opening the folder as a directory source (a Handle with vtable 0x6fd9e8, Handle_Open28) and
// having that source open the name into the stream (Handle_Vfn3: the source's OpenEntry).
//
// The originals' quirks are kept: a path whose only separator is its first character keeps it
// in the name and opens the folder "" (only a separator past the start splits); the separator
// search steps back over a lead byte's separator by searching again from before it.
#pragma once

#include "../../../re/types.h"
#include "handles.h"

extern "C" {
// (&result, path, mode): 0, or -1 when the folder or the file can't be opened.
s32 *__fastcall FileStream_OpenPath(StreamBase *f, void *edx, s32 *result, const char *path, u32 mode);
}

struct FileStreamEnv {
    s32(__stdcall *const *isLeadByte)(u32 c);  // the import table's IsDBCSLeadByte (0x6fd154)
};
// The calls go through env (nullptr: the game's).
void FileStream_SetEnv(const FileStreamEnv *env);
