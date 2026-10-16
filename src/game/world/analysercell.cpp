#include "analysercell.h"

#include <cstdint>

#include "mapcell.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 __fastcall AnalyserCell_TypePlain(const u8 *record) {
    const u32 index = (((u32)record[7] << 7) + record[5] + 1) & 0xffff;
    const u8 *cell = at<const u8 *>(reinterpret_cast<const void *>(0x008023a0), 0) + (index - 1) * 0x44;
    return MapCell_IsType1(cell) || MapCell_IsType3Or9(cell) || MapCell_IsType9(cell) || MapCell_IsTypeA(cell) ||
                   MapCell_IsType0(cell)
               ? 1
               : 0;
}
}
