#include "tiny_n.h"

#include <cstdint>

#include "defids.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __stdcall DefField24_59c240(u32 v0) {
    const u8 *record = ObjectDef_FindById(v0); return record ? at<u32>(record, 0x24) : 0x266;
}

u32 __stdcall DefField28_59c2b0(u32 v0) {
    const u8 *record = ObjectDef_FindById(v0); return record ? at<u32>(record, 0x28) : 0x0;
}

u32 __stdcall DefField2c_59c490(u32 v0) {
    const u8 *record = ObjectDef_FindById(v0); return record ? at<u32>(record, 0x2c) : 0x5;
}

u32 __stdcall DefField8_59c500(u32 v0) {
    const u8 *record = ObjectDef_FindById(v0); return record ? at<u32>(record, 0x8) : 0xa;
}

}
