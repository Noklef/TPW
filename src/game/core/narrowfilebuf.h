// basic_filebuf<char>'s destructor, pbackfail and uflow, by their shape (vtable 0x7098dc; names unconfirmed): the narrow twins of core/wfilebuf's, thiscall. An int answer, -1 for eof.
#pragma once

#include "../../../re/types.h"

struct NarrowFileBufEnv {
    s32(__cdecl *fclose)(void *file);       // CRT_fclose (0x67b620)
    s32(__cdecl *ungetc)(u32 c, u32 file);  // 0x67cbe0
    s32(__cdecl *fgetc)(u32 file);          // 0x67cb90
};
void NarrowFileBuf_SetEnv(const NarrowFileBufEnv *env);

extern "C" {
// 0x5d06c0 (thiscall (file), pops 4; an SEH frame, dropped; basic_filebuf<char>'s constructor, by its shape; name invented, unconfirmed): the streambuf part: its locale reference +0x34 (LocaleRef_Ctor), the six slot pointers,
// three slots zeroed, vtable 0x709914, Targets3_Set (0, 0, 0). LocaleRef_Ctor on +0x4c, vtable 0x7098dc, byte +0x48 = 0; the reference loses a use and is made again, the pointers again, +0x44 = 0, Targets2_Set (0, 0),
// Targets3_Set (0, 0, 0). With a file, the slots to its fields: Fn_5d0e00 (+8, +0, +4, +8, +0, +4 of the file). +0x50 = file, +0x40 and +0x3c = [0xfb2760], +0x38 = 0. Returns the buffer.
u8 *__fastcall NFileBuf_Ctor(u8 *buf, void *, u8 *file);
// 0x5d09b0 (thiscall; name invented, unconfirmed): a file +0x50 whose fclose answers 0: +0x48 = 0, the pointers cleared, +0x50 = 0, +0x40 and +0x3c = [0xfb2760], +0x38 = 0, returns the buffer; else 0.
// 0x5d0a90 _Init (file, mode), pops 8 (name invented, unconfirmed): +0x48 = mode is 1. Mode 0: the reference +0x4c loses a use and is made again, +0x44 = 0. The pointers cleared; a file with +0x48 clear: the slots to its fields. Then as above with the file.
// 0x5cf890 (flags), pops 4 (name invented, unconfirmed): 0x5cec80, then freed with bit 0 of the flags. Returns the buffer.
u8 *__fastcall NFileBuf_Close(u8 *buf);
void __fastcall NFileBuf_Init(u8 *buf, void *, u8 *file, u32 mode);
u8 *__fastcall NFileBuf_DeletingDtor(u8 *buf, void *, u32 flags);
// 0x5cec80 (an SEH frame, dropped; name invented, unconfirmed): vtable 0x7098dc. Byte +0x48 and a file +0x50 whose fclose answers 0: +0x48 = 0, Wstreambuf_InitPointers, +0x50 = 0, +0x40 and +0x3c = [0xfb2760], +0x38 = 0.
// The string +0x44, when set, tidied and freed. The locale references +0x4c, then (after vtable 0x709914) +0x34, each when set lose a use, deleted (slot 0 with 1) at the last.
void __fastcall NFileBuf_Dtor(u8 *buf);
// 0x5ce920, 0x5cf630 (thiscall on the virtual base; names invented, unconfirmed): basic_ifstream<char>'s (the stream at ios - 0x5c, its buffer at +8) and basic_ofstream<char>'s (at ios - 0x58, +4) destructors. The ios the
// stream's table locates gets vtable 0x702960 / 0x702970, the buffer is destroyed (0x5cec80, whose fclose path here is _Init (0, 2), which does the same), and it gets 0x70994c / 0x70995c.
void __fastcall NIfstream_DtorOnIos(u8 *ios);
void __fastcall NOfstream_DtorOnIos(u8 *ios);
// 0x5cefe0 pbackfail (c), pops 4 (name invented, unconfirmed): a get area with something behind (+0x1c's pointer past +0xc's), and eof or the same char: the pointer back, the count +0x2c up; the answer c, or 0 for eof.
// Otherwise no file or eof: eof. No conversion: ungetc (the low byte). Otherwise the string's bytes, last first, through ungetc: any failure or an empty string is eof; then the string emptied, +0x40 = +0x3c, the answer c.
s32 __fastcall NFileBuf_PutBack(u8 *buf, void *, s32 c);
// 0x5cf200 uflow (name invented, unconfirmed): a char in the get area taken. No file: eof. No conversion: fgetc. Otherwise +0x3c = +0x40, the string emptied, bytes from fgetc appended one at a time, each followed by the
// conversion's slot 4 (the string as from, a local byte as to) from the state +0x40 = +0x3c: 0 answers the local; 1 reads on; 3 with a byte or more answers the string's first, with none reads on; others, or no byte, eof.
s32 __fastcall NFileBuf_Uflow(u8 *buf);
}
