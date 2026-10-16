#include "analysercell.h"

#include <cstdint>

#include "mapcell.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall Object_NeighbourCellId(const u8 *object, void *, u16 *out) {
    const u32 id = at<u16>(object, 0x36);
    const u8 *cell = at<const u8 *>(reinterpret_cast<const void *>(0x008023a0), 0) + id * 17 * 4 - 0x44;
    const u32 bits = MapCell_Neighbours(cell) & 0xff;
    // the column and row delta globals of each direction bit
    static const struct { u32 bit, column, row; } kSteps[] = {
        {1, 0x007cdba0, 0x007cdba4}, {0x10, 0x007cdba8, 0x007cdbac}, {0x40, 0x007cdbd0, 0x007cdbd4}, {4, 0x007cdbc8, 0x007cdbcc}};
    for (const auto &step : kSteps) {
        if (bits & step.bit) {
            const u32 column = *reinterpret_cast<const u32 *>(step.column), row = *reinterpret_cast<const u32 *>(step.row);
            *out = (u16)((row << 7) + column + at<u16>(object, 0x36));
            return;
        }
    }
    *out = 0;
}

u32 __fastcall AnalyserCell_TypePlain(const u8 *record) {
    const u32 index = (((u32)record[7] << 7) + record[5] + 1) & 0xffff;
    const u8 *cell = at<const u8 *>(reinterpret_cast<const void *>(0x008023a0), 0) + (index - 1) * 0x44;
    return MapCell_IsType1(cell) || MapCell_IsType3Or9(cell) || MapCell_IsType9(cell) || MapCell_IsTypeA(cell) ||
                   MapCell_IsType0(cell)
               ? 1
               : 0;
}

u32 __fastcall AnalyserRecord_ChecksCell(const u8 *record) {
    switch (at<u32>(record, 0x19c)) {
    case 4: case 5: case 7: case 0xd: case 0x12: return 0;
    default: return AnalyserCell_TypePlain(record) != 0 ? 1 : 0;
    }
}
}
