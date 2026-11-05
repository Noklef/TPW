// The online mail's files (subject from the texts "/outbox.dat", "/%08x.dat", "/%08x.jpg" and "nobody@nowhere.net"; all names here invented, unconfirmed). A mailbox object: +0 its folder (a std::string), +0x10 and +0x20 a message's
// two texts, +0x30 its lines (a list of strings), +0x3c its picture's path, +0x4c to +0x54 its three numbers; +0x5c, +0x68 and +0x74 lists of numbers (at a node's +8). The file streams are the original's locals: a basic_ofstream or
// basic_ifstream<char> of its own frame's size. Reimplementation of 0x5cb720, 0x5cba70 and 0x5cde40.
#pragma once

#include "../../../re/types.h"

struct StdString;

// The calls of the functions below (the bench's recorders; 0x5d08e0, 0x5d0e70, 0x5cfe40 and 0x67a520 are still the game's).
struct MailboxEnv {
    u8 *(__fastcall *iosInit)(u8 *ios);                                                    // Init_5cfd80
    u8 *(__fastcall *ostreamCtor)(u8 *self, void *, u8 *buffer, u32 addstd, u32 init, u32 derived);  // NOstream_Ctor
    u8 *(__fastcall *istreamCtor)(u8 *self, void *, u8 *buffer, u32 addstd, u32 derived);  // NIstream_Ctor
    u8 *(__fastcall *fileCtor)(u8 *buf, void *, u8 *file);                                 // NFileBuf_Ctor
    u8 *(__fastcall *open)(u8 *buf, void *, const char *path, u32 mode);                   // 0x5d08e0
    void(__fastcall *setState)(u8 *ios, void *, u32 state, u32 reraise);                   // BasicIosW_SetState
    void(__fastcall *ofstreamDtor)(u8 *ios);                                               // NOfstream_DtorOnIos
    void(__fastcall *ifstreamDtor)(u8 *ios);                                               // NIfstream_DtorOnIos
    u8(__fastcall *widen)(u8 *ios, void *, u32 c);                                         // NIos_Widen
    u8 *(__fastcall *getline)(u8 *self, void *, char *to, u32 size, u32 end);              // 0x5cfe40
    u8 *(__cdecl *write)(u8 *self, const char *text);                                      // 0x5d0e70
    s32(__cdecl *fclose)(void *file);                                                      // CRT_fclose
    void(__fastcall *fileInit)(u8 *buf, void *, u8 *file, u32 mode);                       // NFileBuf_Init
    u8 *(__fastcall *fileClose)(u8 *buf);                                                  // NFileBuf_Close
    void(__fastcall *fileDtor)(u8 *buf);                                                   // NFileBuf_Dtor
    void(__fastcall *iosDtor)(u8 *self);                                                   // IosBase_Dtor
    s32(__cdecl *atoi)(const char *text);                                                  // 0x67a520
    u32 *(__fastcall *erase)(u8 *list, void *, u32 *out, u8 *node);                        // List_EraseNodeCounted
    void(__fastcall *insert)(u8 *list, void *, u8 **out, u8 *before, const StdString *x);  // StdListString_Insert
};
void Mailbox_SetEnv(const MailboxEnv *env);  // nullptr: the game's

extern "C" {
// 0x5cde40 (thiscall): basic_ios<char>'s destructor: the vtable 0x709954, IosBase_Dtor.
void __fastcall NIos_Dtor(u8 *self);

// 0x5cb720 (thiscall): "<folder>/outbox.dat" written: the numbers of the lists +0x5c, +0x74 and +0x68, one per line ("%d\n" 0x7740c8), each list ended by a line feed (0x74da30). Answers 0 when the file does not open, else 1.
u8 __fastcall Mailbox_SaveOutbox(u8 *self);

// 0x5cba70 (thiscall (number), pops 4): "<folder>/%08x.dat" read: lines of at most 0x100: numbers into +0x4c, +0x50, +0x54, texts into +0x10 and +0x20, the rest into the emptied list +0x30 until the stream fails. +0x3c = "<folder>/%08x.jpg".
// Answers 0 when the file does not open, else 1.
u8 __fastcall Mailbox_LoadMessage(u8 *self, void *, u32 number);
}
