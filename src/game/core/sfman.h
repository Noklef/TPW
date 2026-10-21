// The SoundFont synthesiser (Creative's SoundFont Manager, SFMAN32.DLL): a 0x1c-byte object (built by 0x6d3b90) that loads the library,
// gets its API table through the "SFManager" export's query (slot 0, cdecl (0x10001, &table)), and opens the "SoundFont" MIDI out device
// to send it General MIDI messages and Creative's system-exclusive ones (F0 00 20 21 5F ...). +0 the library, +4 the export, +8 the API
// table (cdecl function pointers; +0xc the device, a word), +0x10 the MIDI out handle, +0x14 bit 0 open, +0x18 the programs' count.
// Reimplementation of 0x6d3b90-0x6d46d7; the names are invented.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6d3f80 (stdcall status, data1, data2; ECX unused): a short MIDI message on channel 2: status | 0x82 (all of it, sic) | (data1 & 0x7f)
// << 8 | (data2 & 0x7f) << 16.
u32 __stdcall SfMan_PackMsg(u32 status, u32 data1, u32 data2);
// 0x6d4530: with the API table, its slot 14 (+0x38) on the device and a word pair {1, 0} on the stack: 1 when it answers 0, else 0;
// without one, 0.
u32 __fastcall SfMan_SetLoad(u8 *self);
// 0x6d4570: with the MIDI out handle, all notes off (controller 0x7b) through midiOutShortMsg, its answer; else 0.
u32 __fastcall SfMan_NotesOff(u8 *self);
// 0x6d45a0 (thiscall out, program, flags, data2, data1; pops 0x14): -1 without the handle or for program >= +0x18; else bank 1
// (controller 0) and the program change, -1 when either fails; then one message by the flags' low byte (bit 0 note on: data1 and data2;
// bit 1 note off; bits 2, 4, 3, 5 controllers 10, 11, 0x40, 0x5b with data2; none: the out pointer as status and data1, sic), sent
// again after all notes off when it fails; 0 either way. Returns out.
u32 *__fastcall SfMan_Send(u8 *self, void *, u32 *out, u32 program, u32 flags, u32 data2, u32 data1);
// 0x6d3fb0 (thiscall out, value, message; pops 0xc): -1 without the handle or the message, or for a message type (its first dword) other
// than 0, 2, 4 and 0x7f; else Creative's system-exclusive F0 00 20 21 5F 00 type, then by type: 0 the value's four bytes (& 0x7f each),
// 2 the message's byte +4 and the value's, 4 0, the bytes +8, 0, +0xa, the value's, +0xc, 0, +0xe; then F7. Sent with
// midiOutPrepareHeader and midiOutLongMsg (a 0x40-byte header): -1 when either fails, else 0. Returns out.
u32 *__fastcall SfMan_Sysex(u8 *self, void *, u32 *out, u32 value, const u32 *message);
// 0x6d4240 (thiscall out; pops 4): 0 when the handle is set; else the first MIDI out device whose name begins "SoundFont" that
// midiOutOpen opens (into +0x10), then the system-exclusive message 0x7f through it (its answer in out); -1 when none (or no devices,
// their count taken as a word). Returns out.
u32 *__fastcall SfMan_OpenOut(u8 *self, void *, u32 *out);

}  // extern "C"
