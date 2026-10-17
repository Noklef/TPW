// The drive queries of the game's system-info object (0x5f8790-0x5f8a26): its constructor over
// the CPU probe, the logical drives listed by type (SystemInfo_ListDrives 0x5f87b0), a volume
// found by label (SystemInfo_FindVolume 0x5f8940, the CD check's: CD-ROM drives, the label at
// *0xf7a7d0) and its two helpers. None but the constructor uses the object.
//
// The originals' quirks are kept:
// - ListDrives reads the drive strings into 0x100 bytes asking for 0xff, and fails on none or
//   on a count of 0x100 or more; an empty first string still succeeds, with nothing listed.
//   A failed node allocation is dereferenced (the game crashes too).
// - FindVolume compares labels with _stricmp (A-Z folded) and asks for at most 0x14 bytes of
//   label; it gives the drive letter of the first match, which is 0 for a root without a
//   colon (the search still stops there).
#pragma once

#include "../../../re/types.h"
#include "../core/string.h"
#include "waddir.h"

struct SystemInfo {
    u8 cpu[0x1c];  // CpuInfo (0x55e470: CPUID)
    u16 drives[4];  // zeroed by the constructor; not read here
};

struct DrivesEnv {
    void(__fastcall *cpuCtor)(SystemInfo *);  // CpuInfo_Ctor 0x55e470
    u32(__stdcall *const *getLogicalDriveStrings)(u32 size, char *buffer);  // the import table's (0x6fd0a0)
    u32(__stdcall *const *getDriveType)(const char *root);                  // 0x6fd09c
    s32(__stdcall *const *getVolumeInformation)(const char *root, char *name, u32 size, u32 *serial,
                                               u32 *maxComponent, u32 *flags, char *fsName,
                                               u32 fsNameSize);  // 0x6fd0a4
    String *(__fastcall *stringCtor)(String *, void *, const char *);        // 0x5f60b0
    String *(__fastcall *stringCopy)(String *, void *, const StringView *);  // 0x5f6040
    void(__fastcall *stringDtor)(String *);                                 // 0x5f61e0
    void *(__cdecl *alloc)(u32 size);                                       // Mem_Alloc 0x67acc0
    StringListNode *(__fastcall *nodeCtor)(StringListNode *, void *, String text);  // 0x5f8a50
    s32(__cdecl *stricmp)(const char *, const char *);                       // _stricmp 0x67df40
};
// nullptr restores the game's.
void Drives_SetEnv(const DrivesEnv *e);
// The env in use (for tools/fncmp's driver, which resolves the game code it calls).
const DrivesEnv *Drives_GetEnv();

extern "C" {
SystemInfo *__fastcall SystemInfo_Ctor(SystemInfo *s);
// (&out, list, types: 1 removable, 2 fixed, 4 remote, 8 CD-ROM): each matching drive's root
// ("C:\") appended to the list; 0, or -1 when the drives can't be read.
s32 *__fastcall SystemInfo_ListDrives(SystemInfo *s, void *edx, s32 *out, StringList *list, u32 types);
// The drive letter of the first listed root whose volume is labelled so, or 0.
u8 __fastcall SystemInfo_FindVolume(SystemInfo *s, void *edx, const StringList *list, const char *label);
// path[0] when path[1] is ':', else 0.
u8 __fastcall SystemInfo_DriveLetter(SystemInfo *s, void *edx, const char *path);
// (&out, root, buffer, size): the volume's label; 0, or -1 when it can't be read.
s32 *__fastcall SystemInfo_VolumeLabel(SystemInfo *s, void *edx, s32 *out, const char *root, char *label, u32 size);
}
