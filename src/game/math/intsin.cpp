#include "intsin.h"

namespace {

s32 entry(u32 index) { return (s32)(reinterpret_cast<const u32 *>(0x00877358)[index & 0x1ff] << 2); }

}  // namespace

extern "C" {

s32 __cdecl IntSin(u32 angle) { return entry(angle); }

s32 __cdecl IntCos(u32 angle) { return entry(angle + 0x80); }
}
