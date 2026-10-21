#include "streamfilter.h"

#include <cstdint>

#include "vcall.h"

namespace {

u8 *inner(u8 *stream) { return vcall<u8 *>(stream, 0x48); }

}  // namespace

extern "C" {

u32 __fastcall StreamFilter_SampleRate(u8 *stream) { return vcall<u32>(inner(stream), 0x18); }

u32 __fastcall StreamFilter_Restart(u8 *stream) { return vcall<u32>(inner(stream), 0x20); }

u32 __fastcall StreamFilter_Position(u8 *stream) { return vcall<u32>(inner(stream), 0x34); }

u32 __fastcall StreamFilter_Length(u8 *stream) { return vcall<u32>(inner(stream), 0x38); }

u32 __fastcall StreamFilter_Bits(u8 *stream) { return vcall<u32>(inner(stream), 0x40); }

u32 __fastcall StreamFilter_Channels(u8 *stream) { return vcall<u32>(inner(stream), 0x44); }

u32 __fastcall StreamFilter_Seek(u8 *stream, void *, u32 position) { return vcall<u32>(inner(stream), 0x30, position); }

u32 __fastcall StreamFilter_SetFormat(u8 *stream, void *, u32 format) { return vcall<u32>(inner(stream), 0x3c, format); }

u32 *__fastcall StreamFilter_Query(u8 *stream, void *, u32 *out, u8 *record) {
    vcall<void>(inner(stream), 0x24, out, record);
    return out;
}

}  // extern "C"
