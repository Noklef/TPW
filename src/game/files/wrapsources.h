// The sources that wrap another source, and the compressor singleton: reimplementation of
// 0x638840-0x638ec0, 0x6418e0-0x6419a0, 0x6ad810 and 0x5f8050-0x5f81d0.
//
// Each is a RefObject (+4 the wrapped source, its link) with the source interface: +0
// deleting destructor, +4 read (buffer, count), +8 write, +0xc seek (offset, origin 0-2),
// +0x10 size, +0x14 position, +0x18 map (offset, size), +0x24 clear, +0x28 attach (the
// source the holder had, see handles.h), +0x2c the lock. The WAD source stacks them over its
// file: a shared source per open entry (several can read one file: each takes the file's
// lock and seeks back to its own position first), then a slice of it (the entry's bytes), or
// a buffer that decodes the whole entry when attached (RefPack, or zlib through the
// compressor singleton).
//
// The originals' quirks are kept: the tracked and shared sources add whatever the wrapped
// read or write returns (-1 too) to their position; a buffer's clear frees the data but
// keeps the pointer, and zeroes the cursor, so a later read copies from address 0; a seek
// of -0x80000000 from a buffer's end passes the bounds check (its negation overflows); a
// buffer reads and decodes without checking sizes or allocations, and
// the zlib one's read callback answers 0 whatever it read; a slice's relative seek is passed
// on as relative, trusting the wrapped position, and its clear unmaps the view but keeps it
// (the next map hands back the stale one); the singleton's guard is the game's static-local
// bit, its destructor registered with atexit.
#pragma once

#include "../../../re/types.h"
#include "handles.h"
#include "zlibcodec.h"

struct TrackedSource {  // vtable 0x704a90; the shared source 0x703ea0
    RefObject base;
    u32 position;
};
static_assert(sizeof(TrackedSource) == 0x10, "TrackedSource is 0x10 bytes");
struct BufferSource {  // vtable 0x703ed0 (RefPack) or 0x703f18 (zlib)
    RefObject base;
    u32 size;
    u8 *data;
    u8 *cursor;
};
static_assert(sizeof(BufferSource) == 0x18, "BufferSource is 0x18 bytes");
struct SliceSource {  // vtable 0x703f48
    RefObject base;
    u32 size;
    u32 position;  // from start
    u32 start;     // in the wrapped source
    void *view;
};
static_assert(sizeof(SliceSource) == 0x1c, "SliceSource is 0x1c bytes");
// A reader or writer over callbacks (vtable 0x703160; base 0x703170): +4 write, +8 read,
// each calling its callback (buffer, count, context), __stdcall.
typedef s32(__stdcall *StreamCallback)(void *buffer, u32 count, void *context);
struct CallbackStream {
    void *const *vtable;
    StreamCallback write;
    StreamCallback read;
    void *context;
};
static_assert(sizeof(CallbackStream) == 0x10, "CallbackStream is 0x10 bytes");

extern "C" {
// __thiscall throughout unless noted.
// 0x6ad810: the dword at +0xc (the size of a buffer or slice, a tracked source's position).
u32 __fastcall Source_Field0c(const TrackedSource *s);

// The tracked source. 0x6418e0: RefObject_Ctor, position 0. 0x641900: RefObject_Dtor.
// 0x638860 (flags; vtable +0 of all four classes here): 0x641900, freed when flags & 1.
TrackedSource *__fastcall TrackedSource_Ctor(TrackedSource *s);
void __fastcall TrackedSource_Dtor(TrackedSource *s);
TrackedSource *__fastcall TrackedSource_DeletingDtor(TrackedSource *s, void *edx, u32 flags);
void __fastcall TrackedSource_Clear(TrackedSource *s);  // 0x641910 (+0x24): position 0
// 0x641920 / 0x641950 (buffer, count): the wrapped read / write, added to the position.
s32 __fastcall TrackedSource_Read(TrackedSource *s, void *edx, void *buffer, u32 count);
s32 __fastcall TrackedSource_Write(TrackedSource *s, void *edx, const void *buffer, u32 count);
// 0x641980 (offset, origin): the wrapped seek, which becomes the position.
s32 __fastcall TrackedSource_Seek(TrackedSource *s, void *edx, s32 offset, u32 origin);
u32 __fastcall TrackedSource_Size(TrackedSource *s);  // 0x6419a0: the wrapped size

// The shared source. 0x638840: the tracked one's, with its vtable. 0x638880 / 0x638910 /
// 0x6389a0 / 0x638a30: the tracked ones under the wrapped source's lock (taken for good,
// SemaphoreLock_Acquire(1)), after Sync.
TrackedSource *__fastcall SharedSource_Ctor(TrackedSource *s);
s32 __fastcall SharedSource_Read(TrackedSource *s, void *edx, void *buffer, u32 count);
s32 __fastcall SharedSource_Write(TrackedSource *s, void *edx, const void *buffer, u32 count);
s32 __fastcall SharedSource_Seek(TrackedSource *s, void *edx, s32 offset, u32 origin);
u32 __fastcall SharedSource_Size(TrackedSource *s);
void __fastcall SharedSource_Sync(TrackedSource *s);  // 0x638ab0: seeks the wrapped one back
// 0x638ae0 (source; +0x28): RefObject_SetLink, then the position is the new link's.
void __fastcall SharedSource_Attach(TrackedSource *s, void *edx, RefObject *source);

// The buffer. 0x638b00 / 0x638c90: RefObject_Ctor, the RefPack / zlib vtable, all 0.
BufferSource *__fastcall BufferSource_Ctor(BufferSource *b);
BufferSource *__fastcall ZlibBufferSource_Ctor(BufferSource *b);
// 0x638b20 (&result, size): a block of size bytes, unchecked; 0.
s32 *__fastcall BufferSource_Open(BufferSource *b, void *edx, s32 *result, u32 size);
s32 __fastcall BufferSource_Write(BufferSource *b, void *edx, const void *buffer, u32 count);  // 0x638b50: 0
// 0x638b60 (buffer, count): what is left, up to count.
u32 __fastcall BufferSource_Read(BufferSource *b, void *edx, void *buffer, u32 count);
// 0x638cb0 (offset, origin): the new position, or -1 (outside [0, size]; origin 1 compared
// as addresses); any other origin -1.
s32 __fastcall BufferSource_Seek(BufferSource *b, void *edx, s32 offset, u32 origin);
u32 __fastcall BufferSource_Tell(const BufferSource *b);  // 0x638bb0
void __fastcall BufferSource_Clear(BufferSource *b);      // 0x638bc0
// 0x638be0 / 0x638d30 (source; +0x28): RefObject_SetLink, then the whole source decoded
// into the data: read into a block of its size and RefPack_Decode, or inflated through
// Buffer_DecompressStream with a 0x4000-byte buffer and 0x638d80.
void __fastcall BufferSource_AttachRefPack(BufferSource *b, void *edx, RefObject *source);
void __fastcall BufferSource_AttachZlib(BufferSource *b, void *edx, RefObject *source);
// 0x638d80, __stdcall (buffer, count, source): source's read; 0.
s32 __stdcall BufferSource_ReadCallback(void *buffer, u32 count, void *source);
// 0x638c60 / 0x638c70 (flags): the codec base's vtable (0x703148); freed when flags & 1.
void __fastcall Codec_Dtor(ZlibCodec *c);
ZlibCodec *__fastcall Codec_DeletingDtor(ZlibCodec *c, void *edx, u32 flags);

// The slice. 0x638da0: RefObject_Ctor, the vtable, all 0. 0x638dc0 (&result, size,
// start): those, position and view 0; 0.
SliceSource *__fastcall SliceSource_Ctor(SliceSource *s);
s32 *__fastcall SliceSource_Open(SliceSource *s, void *edx, s32 *result, u32 size, u32 start);
// 0x638df0 (buffer, count): the wrapped read of what is left, up to count (0 when nothing).
s32 __fastcall SliceSource_Read(SliceSource *s, void *edx, void *buffer, u32 count);
// 0x638e40 (offset, origin): the wrapped seek (origins 0 and 2 from start, 1 passed on),
// -1 outside [0, size]; the new position.
s32 __fastcall SliceSource_Seek(SliceSource *s, void *edx, s32 offset, u32 origin);
// 0x638ec0 (offset, size; +0x18, both ignored): the wrapped map of the slice, once.
void *__fastcall SliceSource_Map(SliceSource *s, void *edx, u32 offset, u32 size);
void __fastcall SliceSource_Clear(SliceSource *s);  // 0x638de0 (+0x24): the wrapped unmap

// The compressor singleton (a ZlibCodec at 0xfa75fc). 0x5f8080: constructed (0x5f80d0) on
// the first call, its destructor (0x5f80c0) given to atexit.
ZlibCodec *__cdecl Compressor_Get();
void __cdecl Compressor_AtExit();                    // 0x5f80c0: Codec_Dtor
ZlibCodec *__fastcall ZlibCodec_Ctor(ZlibCodec *c);  // 0x5f80d0: vtable 0x703128
// 0x5f80e0 (flags): the codec base's deleting destructor (0x703148).
ZlibCodec *__fastcall CodecBase_DeletingDtor(ZlibCodec *c, void *edx, u32 flags);
// cdecl, through the singleton: 0x5f8050 (src, dst, size, work) its Compress (dst, src,
// size, work); 0x5f8100 (src, dst, work) its Decompress (dst, src, work); 0x5f8120
// (header) its UnpackedSize.
u32 __cdecl Buffer_Compress(const void *src, void *dst, u32 size, void *work);
u32 __cdecl Buffer_Decompress(const void *src, void *dst, void *work);
u32 __cdecl Buffer_UnpackedSize(const void *header);
// 0x5f81d0, cdecl (dst, buffer, capacity, read, context, work): DecompressStream (dst,
// buffer, capacity, work) reading through a CallbackStream {read, context}.
u32 __cdecl Buffer_DecompressStream(void *dst, void *buffer, u32 capacity, StreamCallback read, void *context,
                                    void *work);
// The callback stream: 0x5f8140 (+4) / 0x5f8160 (+8) (buffer, count): its write / read
// callback's result. 0x5f81a0: the base vtable. 0x5f8180 (flags): that, freed when flags &
// 1; 0x5f81b0 (flags): the base's.
s32 __fastcall CallbackStream_Write(CallbackStream *c, void *edx, void *buffer, u32 count);
s32 __fastcall CallbackStream_Read(CallbackStream *c, void *edx, void *buffer, u32 count);
void __fastcall CallbackStream_Dtor(CallbackStream *c);
CallbackStream *__fastcall CallbackStream_DeletingDtor(CallbackStream *c, void *edx, u32 flags);
CallbackStream *__fastcall CallbackStreamBase_DeletingDtor(CallbackStream *c, void *edx, u32 flags);
}

struct WrapSourcesEnv {
    void *(__cdecl *alloc)(u32 size);     // Mem_Alloc (0x67acc0)
    void(__cdecl *free)(void *block);     // Mem_Free (0x67acb0)
    s32(__cdecl *atexit)(void(__cdecl *fn)());  // 0x67a7d0
    void(__cdecl *atexitFn)();            // what Compressor_Get registers (0x5f80c0)
    u8 *guard;                            // 0xfa75f8
    ZlibCodec *compressor;                // 0xfa75fc
};
// The calls go through env (nullptr: the game's).
void WrapSources_SetEnv(const WrapSourcesEnv *env);
