// The SoundFont synthesiser (Creative's SoundFont Manager, SFMAN32.DLL): a 0x1c-byte object (built by 0x6d3b90, initializers.h Fn_6d3b90) that loads the library,
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
// 0x6d4530: with the API table, its slot 14 (+0x38; the bank unload) on the device and the bank, a word pair {1, 0} on the stack: 1 when it answers 0, else 0;
// without one, 0.
u32 __fastcall SfMan_UnloadBank(u8 *self);
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
// 0x6d4340 (thiscall out, preset, sink; pops 0xc): -1 without the API table; else its slot 16 (+0x40) on the device, the word pair {1,
// preset} and {0x40, a 0x40-byte name buffer on the stack}: -1 when it fails, else the sink's slot 5 (+0x14) on the name and 0. Returns out.
u32 *__fastcall SfMan_GetName(u8 *self, void *, u32 *out, u32 preset, u8 *sink);
// 0x6d43c0: 0 without the API table; else for presets 0 to 0x7f the name buffer set to the string at 0x786de4 (unbounded, sic) and slot
// 16 asked for the preset's name: 0 when it fails; the count at +0x18 when the name is still that string, else the count incremented
// (from what it held), which is returned after preset 0x7f.
u32 __fastcall SfMan_CountPresets(u8 *self);
// 0x6d44a0 (thiscall out, path, unused; pops 0xc): -1 unless open (+0x14 bit 0) with the API table; else slot 11 (+0x2c, the bank
// load) on the device, the word pair {1, 0} and {0x80, path}: -1 when it fails, else 0 when the presets counted are not 0, -1 when they
// are. Returns out.
u32 *__fastcall SfMan_LoadBank(u8 *self, void *, u32 *out, u32 path, u32);

// 0x6d3c90 (thiscall out, count; pops 8): -1 for a count of 0 (as a word) or without the API table; else a score for each of the count
// devices (one byte each in a 16-byte array; past 16 the original writes over its other locals, sic): from slot 1's capabilities (a
// 0x66-byte structure, its flags at +6) 0x20 for flag 0x40000000, 0x80 for 0x80000000, else after slot 4 (+0x10, open) slot 7 (+0x1c)'s
// third out word >> 19 + 0x40 (as a byte; 0x40 counts as 0) and slot 5 (+0x14, close). Then the best device (the first of the highest)
// opened through slot 4 until that answers 0, each refusal scoring it 0, for at most count (as a word) tries; -1 when it scores 0; else its
// capabilities again, slot 7 unless flag 0x80000000, slot 2 (+8)'s version (four bytes & 0x7f) in Creative's system-exclusive message
// F0 00 20 21 5F 00 00 v v v v F7 sent with midiOutPrepareHeader and midiOutLongMsg: -1 when either fails, else 0. Returns out.
u32 *__fastcall SfMan_SelectDevice(u8 *self, void *, u32 *out, u32 count);
// 0x6d3bc0 (thiscall out; pops 4): 0 when open (+0x14 bit 0); else SFMAN32.DLL loaded (into +0), its "SFManager" export (+4) asked for the
// API table (slot 0, cdecl (0x10001, &table at +8)), the devices counted (the table's slot 0), the MIDI out device opened (0x6d4240) and the
// best device selected (0x6d3c90): open and 0; when a step fails the table cleared, the library freed (even when it did not load) and -1.
// Returns out.
u32 *__fastcall SfMan_Open(u8 *self, void *, u32 *out);
// 0x6d3f20: 0 unless open; else all notes off, the bank unloaded, the MIDI out handle closed (and cleared), the device closed with the
// table's slot 5 (+0x14, the device word) and the table cleared, the library freed, the flags cleared: 1.
u32 __fastcall SfMan_Close(u8 *self);
// 0x6d3bb0: the destructor, a jump to 0x6d3f20.
u32 __fastcall SfMan_Dtor(u8 *self);

// The synthesiser's holder (8 bytes, built by Init_6c47b0: +0 0, +4 the synthesiser or null; the sound system's +0x1c, 0x6b7870; names
// invented). 0x6c47f0 (thiscall out; pops 4; an SEH frame for the new): a 0x1c-byte block (Mem_Alloc; Fn_6d3b90 on it when not null)
// into +4 and SfMan_Open on it (sic: on null too); when that answers >= 0, *out = 0; else the synthesiser (+4 read again) destroyed
// (SfMan_Dtor) and freed (Mem_Free) when not null, +4 = 0 and *out = -1. Returns out.
u32 *__fastcall SfHost_Open(u8 *host, void *, u32 *out);
// 0x6c47c0 (thiscall out; pops 4): SfHost_Open. Returns out.
u32 *__fastcall SfHost_OpenOut(u8 *host, void *, u32 *out);
// 0x6c48b0 (thiscall): with a synthesiser (+4), SfMan_Close, then (+4 read again) SfMan_Dtor and Mem_Free when not null, +4 = 0.
void __fastcall SfHost_Close(u8 *host);
// 0x6c47e0 (thiscall): the destructor: SfHost_Close, +0 = 0.
void __fastcall SfHost_Dtor(u8 *host);

}  // extern "C"
