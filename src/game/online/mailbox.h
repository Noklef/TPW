// The online mail's files (subject from the texts "/outbox.dat", "/%08x.dat", "/%08x.jpg" and "nobody@nowhere.net"; all names here invented, unconfirmed). A mailbox object: +0 its folder (a std::string), +0x10 and +0x20 a message's
// two texts, +0x30 its lines (a list of strings), +0x3c its picture's path, +0x4c to +0x54 its three numbers; +0x5c, +0x68 and +0x74 lists of numbers (at a node's +8). The file streams are the original's locals: a basic_ofstream or
// basic_ifstream<char> of its own frame's size. Reimplementation of 0x5cb720, 0x5cba70 and 0x5cde40.
#pragma once

#include "../../../re/types.h"

struct StdString;

// The calls of the functions below (the bench's recorders; 0x67a520 is still the game's).
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
    u32 *(__fastcall *eraseNumber)(u8 *list, void *, u32 *out, u32 node);                  // NumberList_Erase
    u32 *(__fastcall *insertNumber)(u8 *list, void *, u32 *out, u32 before, const s32 *value);  // 0x634c60
    void(__fastcall *clear)(u8 *ios, void *, u32 state, u32 reraise);                      // BasicIosW_Clear
    u32(__fastcall *languageId)(const u8 *language);                                       // Language_Id
    void(__fastcall *pushBack)(u8 *list, void *, const StdString *x);                      // StdListString_PushBack
    void(__fastcall *sort)(u8 *list);                                                      // StdListString_Sort
    void(__fastcall *unique)(u8 *list);                                                    // StdListString_Unique
    s32(__fastcall *addSorted)(u8 *list, void *, const StdString *x);                      // StdListString_AddSorted
    s32(__fastcall *compare)(const u8 *object, void *, const u8 *other);                   // ByteString_Compare
    void(__fastcall *numbersDtor)(void *list);                                             // 0x634bf0
    u32(__cdecl *time)(u32 *to);                                                           // 0x67d3f0
    u8 *(__cdecl *localtime)(const u32 *time);                                             // 0x67d8e0
};
void Mailbox_SetEnv(const MailboxEnv *env);  // nullptr: the game's

// The calls of 0x5cd220, 0x5ccda0 and 0x5c99f0 (0x634cc0, 0x67e560 and 0x5c9720 are still the game's).
struct MailboxDropEnv {
    u32 *(__fastcall *eraseNumber)(u8 *list, void *, u32 *out, u32 node);  // NumberList_Erase
    s32(__cdecl *remove)(const char *path);                                // 0x67e560
    u8(__fastcall *saveOutbox)(u8 *self);                                  // Mailbox_SaveOutbox
    u8(__fastcall *loadMessage)(u8 *self, void *, u32 number);             // Mailbox_LoadMessage
    void(__fastcall *deleteMessage)(u8 *self, void *, u32 number);         // Mailbox_DeleteMessage
    u8 *(__cdecl *instance)();                                             // Online_Instance
    const u16 *(__fastcall *userName)(const u8 *session);                  // Online_UserName
    u32(__fastcall *get34)(const u8 *o);                                   // Get34_41ce80
    u32(__fastcall *get38)(const u8 *o);                                   // Get38_41ce90
    const char *(__fastcall *string14)(const u8 *session);                 // Online_String14
    const char *(__fastcall *string28)(const u8 *session);                 // Online_String28
    void(__fastcall *pushChars)(u8 *owner, void *, const char *chars);     // TokenList_PushChars
    void *(__fastcall *assignWide)(void *s, void *, const u16 *chars, u32 n);  // StdWString_AssignChars
    u32(__fastcall *attach)(u8 *target, void *, const char *path);         // Mailbox_NewAttachment
    u8 *(__fastcall *attachmentCtor)(void *self, void *, const char *path);  // 0x5c9720
};
void MailboxDrop_SetEnv(const MailboxDropEnv *env);  // nullptr: the game's

// The calls of 0x5c9720, 0x5cc8a0, 0x5c9a80 and 0x5ca010.
struct MailClientEnv {
    void *(__cdecl *fopen)(const char *name, const char *mode);                     // CRT_fopen
    s32(__cdecl *fseek)(void *file, s32 offset, s32 origin);                        // CRT_fseek
    s32(__cdecl *ftell)(void *file);                                                // CRT_ftell
    u32(__cdecl *fread)(char *data, u32 size, u32 count, void *file);               // CRT_fread
    void(__cdecl *encode)(char *dst, const u8 *src, s32 length);                    // Base64_Encode
    u8 *(__fastcall *ctor)(void *block);                                            // MailClient_Ctor
    u32(__fastcall *connect)(u8 *self, void *, const char *host, u32 port, u32 arg3, u8 *owner);  // MailClient_Connect
    void(__fastcall *build)(u8 *mail);                                              // PostcardMail_Build
    u8(__fastcall *dropFirst)(u8 *self, void *, u8 *target);                        // Mailbox_DropFirst
    u8(__fastcall *saveMessage)(u8 *self);                                          // Mailbox_SaveMessage
};
void MailClient_SetEnv(const MailClientEnv *env);  // nullptr: the game's

extern "C" {
// 0x5c9720 (thiscall (path), pops 4): an attachment of 0x24: +0 the text 0x75447c, +0x10 the text 0x773dd4. The file read whole; none: the path's own address taken as the length and no data (sic). +0x20 the data in base64, its block sized for the line
// breaks of each 76; no block: the data freed, unencoded. Answers self.
u8 *__fastcall MailAttachment_Ctor(u8 *self, void *, const char *path);

// 0x5c93c0 (thiscall): a client of 0x94: its strings empty, an empty list of strings at +0x10 (its head +0x14), +0x64 = 9, WSAStartup(0x101) (0x6fd42c, the import read live; its data on this frame, 0x190 as in the original), +0 the text 0x773dc0. The
// allocator bytes the original copies from its stack's garbage are not written. Answers self.
u8 *__fastcall MailClient_Ctor(u8 *self);

// 0x5c9f10 (thiscall (host, port, arg3, owner), pops 0x10): no owner: +0x85 = 1, else 0; +0x8c the owner. An empty list +0x10: the callback +0x90, when set, told (1, 8), and 1. The library's object +0x70 made if there is none (0x6fd57c; none: 1); +0x6c = 0;
// its connect (0x6fd574) with the callback 0x5c9a60 and self. Answers whether that gave 0.
u32 __fastcall MailClient_Connect(u8 *self, void *, const char *host, u32 port, u32 arg3, u8 *owner);

// 0x5c9a80 (thiscall (kind, code, unused), pops 0xc): the mail library's event. Kind 0x78: code 0xfa: the next step by the state +0x6c: 0: the sender +0 given (0x6fd578), the cursor +0x88 at the list's first; 1: the next string of the list given (0x6fd570),
// its last: the next state; 2: the body built (PostcardMail_Build), +0x74 given (0x6fd56c), the next state; 3: without +0x85, the client emptied and the owner's next message (Mailbox_DropFirst): got: state 0, the library's 0x6fd564; else, or with +0x85:
// +0x84 = 1, the library's 0x6fd568. Other codes tell the callback +0x90: 0x1a5: (1, 2); 0x1c2, 0x226: (1, 7); 0x228: (1, 6), in state 1 after the next string; 0xdd nothing; the rest (1, 5). Kind -5: (1, -1). Kind -4: (+0x84 is 0, -1).
void __fastcall MailClient_OnEvent(u8 *self, void *, s32 kind, s32 code, u32 unused);

// 0x5c9a60 (stdcall, pops 0x10): the library's callback: 0x5c9a80 on the object it was given.
void __stdcall MailClient_Callback(s32 kind, s32 code, u32 unused, u8 *self);

// 0x5c9570 (thiscall): the client's destructor: emptied as for a new message (the texts 0x773dc0 and 0x786de4), WSACleanup (0x6fd430, the import read live), its strings, its list and its head freed.
void __fastcall MailClient_Dtor(u8 *self);

// 0x5cb030 (thiscall): the mailbox's destructor: the client +0x98 destroyed and freed, then the string +0x80, the number lists +0x70, +0x64 and +0x58 with their heads, the message's strings and lines, the folder.
void __fastcall Mailbox_Dtor(u8 *self);

// 0x5cd0a0 (thiscall): each message of the list +0x70 deleted, its number out of +0x64 and +0x58; the list +0x70 emptied; the outbox saved. +0x7c the first of +0x58. Answers that one's message read, when the list has any, else 0.
u8 __fastcall Mailbox_DeleteSent(u8 *self);

// 0x5ca010 (thiscall on a client (mailbox), pops 4): no mailbox: 0. The client's message made the mailbox's: its lines emptied, +0x10 from +0, +0x20 from +0x1c, the list's strings pushed, +0x3c from +0x2c; answers Mailbox_SaveMessage's.
u8 __fastcall Mailbox_StoreFromClient(u8 *client, void *, u8 *mailbox);

// 0x5cc8a0 (thiscall (host, port, arg3), pops 0xc): nothing in +0x6c: 0. The client +0x98 made, or emptied (+0 the text 0x773dc0, its list, its attachment, +0x74 the text 0x786de4). The host kept at +0x80, the port's low word at +0x90; the client's +0x90
// is this +0x94. The first message of +0x64 read (none: 0) and put in the client as Mailbox_DropFirst does; connected with self as the owner. Answers whether that gave 0. The port's high word in the original is what EDX held; here 0.
u8 __fastcall Mailbox_Send(u8 *self, void *, const char *host, u32 port, u32 arg3);


// 0x5cde40 (thiscall): basic_ios<char>'s destructor: the vtable 0x709954, IosBase_Dtor.
void __fastcall NIos_Dtor(u8 *self);

// 0x5cb720 (thiscall): "<folder>/outbox.dat" written: the numbers of the lists +0x5c, +0x74 and +0x68, one per line ("%d\n" 0x7740c8), each list ended by a line feed (0x74da30). Answers 0 when the file does not open, else 1.
u8 __fastcall Mailbox_SaveOutbox(u8 *self);

// 0x5cba70 (thiscall (number), pops 4): "<folder>/%08x.dat" read: lines of at most 0x100: numbers into +0x4c, +0x50, +0x54, texts into +0x10 and +0x20, the rest into the emptied list +0x30 until the stream fails. +0x3c = "<folder>/%08x.jpg".
// Answers 0 when the file does not open, else 1.
u8 __fastcall Mailbox_LoadMessage(u8 *self, void *, u32 number);

// 0x5cc5e0 (thiscall; name invented, unconfirmed): the cursor +0x7c to the first message of the list +0x5c; with messages (+0x60) it is loaded. Answers its load, or 0.
u8 __fastcall Mailbox_FirstMessage(u8 *self);
// 0x5cc600 (thiscall; name invented, unconfirmed): with messages, the cursor +0x7c moves on and that one is loaded; past the last the cursor goes to the first and the answer is 0.
u8 __fastcall Mailbox_NextMessage(u8 *self);

// 0x5cb230 (thiscall (folder), pops 4): the folder kept at +0 and "/outbox" added; the number lists +0x58, +0x64 and +0x70 emptied. "<that>/outbox.dat" read: lines of at most 0x80 into +0x58, then +0x70, then +0x64, each ended by a failed
// read or an empty line. When the file does not open, the folder made (0x6fd120, the import read live) and the answer whether that worked; else 1.
u8 __fastcall Mailbox_Open(u8 *self, void *, const char *folder);

// 0x5cd9c0 (thiscall on a list<string> (folder), pops 4): "<folder>/addrbook.dat" read: lines of at most 0x80 at the end of the list, then the default address added sorted (language 0x409 (Language_Id on 0x78b530): the text 0x7740f4, else the string
// 0x7a155c), and 1. When the file does not open, that default pushed, the list sorted and made unique and walked until the default is found (no use is made of it), and 0.
u8 __fastcall AddressBook_Load(u8 *self, void *, const char *folder);

// 0x5cd710 (thiscall on a list<string> (folder), pops 4): "<folder>/addrbook.dat" written: the strings, one per line. Answers 0 when the file does not open, else 1.
u8 __fastcall AddressBook_Save(u8 *self, void *, const char *folder);

// 0x634cc0 (thiscall on a list<u32> (out, node), pops 8): the node unlinked and freed (Mem_Free), the count +8 lowered, the next node in *out. Answers out.
u32 *__fastcall NumberList_Erase(u8 *list, void *, u32 *out, u32 node);

// 0x5cbf40 (thiscall): a new message saved: its number the last of the list +0x58 plus 1 (a count but no node: this pointer as the number, sic; no count: 0). "<folder>/%08x.dat" written: today (0x67d3f0, 0x67d8e0, kept at +0x4c, +0x50 and +0x54) and
// the texts +0x10 and +0x20 and the lines +0x30, a line each. The picture +0x3c moved (0x6fd11c, the import read live) to "<folder>/%08x.jpg", which it then is. The number added at the head of +0x58 and the end of +0x64, that one sorted (std::list's
// sort, inlined) when it has two, and adjacent equal numbers erased; the outbox saved. Answers 0 when the file does not open, else 1.
u8 __fastcall Mailbox_SaveMessage(u8 *self);

// 0x5cd220 (thiscall (number), pops 4): the files "<folder>/%08x.dat" and "<folder>/%08x.jpg" removed.
void __fastcall Mailbox_DeleteMessage(u8 *self, void *, u32 number);

// 0x5c99f0 (thiscall (path), pops 4): a block of 0x24 (Mem_Alloc; none: 0) built by 0x5c9720 from the path, kept at +0x68. Answers 1 when there is none, else 0.
u32 __fastcall Mailbox_NewAttachment(u8 *target, void *, const char *path);

// 0x5ccda0 (thiscall (target), pops 4): the number at the head of the list +0x64 taken out of the lists +0x70, +0x64 and +0x58, its files removed, the outbox saved. With another left, that message read (none: 0) and the target filled: its text +0
// the session's user name (the low byte of each wide character, in a buffer of 0x80), +0x3c and +0x40 from the session, wide +0x44 and +0x54 its two strings, +0x64 = 9, the message's lines pushed, +0x1c its second text, the attachment made from
// the picture's path. Answers 1 when the attachment was made, else 0.
u8 __fastcall Mailbox_DropFirst(u8 *self, void *, u8 *target);
}
