// The hook registry: every original function we have reimplemented.
// Add a row here when a reimplementation is ready; mark it in re/symbols.csv too.
#include "hooks.h"

#include "../game/files/refpack.h"
#include "../game/motion/curve.h"
#include "../game/rsse/rsse.h"
#include "../game/terrain/terrain_blend.h"

namespace tpw::hook {

const Hook kHooks[] = {
    {"RSSE_FetchWord",    0x00557a70, (void *)&RSSE_FetchWord,    {0x56, 0x8b, 0x74, 0x24, 0x08}},
    {"RSSE_OperandValue", 0x005573a0, (void *)&RSSE_OperandValue, {0x8b, 0x44, 0x24, 0x08, 0x8b}},
    {"TerrainBlend_Tile", 0x0055f780, (void *)&TerrainBlend_Tile, {0x55, 0x8b, 0xec, 0x81, 0xec}},
    {"RefPack_Decode",    0x00641f10, (void *)&RefPack_Decode,    {0x8b, 0x54, 0x24, 0x08, 0x83}},
    {"RefPack_DecodedSize", 0x00642110, (void *)&RefPack_DecodedSize, {0x56, 0x8b, 0x74, 0x24, 0x08}},
    {"Curve_Bezier",        0x00474840, (void *)&Curve_Bezier,        {0x51, 0xd9, 0x44, 0x24, 0x14}},
    {"Curve_BezierTangent", 0x00474a20, (void *)&Curve_BezierTangent, {0x51, 0xd9, 0x44, 0x24, 0x14}},
    {"Curve_Linear",        0x00474bf0, (void *)&Curve_Linear,        {0x56, 0x8b, 0x74, 0x24, 0x0c}},
    {"Curve_LinearTangent", 0x00474c70, (void *)&Curve_LinearTangent, {0xd9, 0x44, 0x24, 0x10, 0xe8}},
    {"Curve_CatmullRom",    0x00474cc0, (void *)&Curve_CatmullRom,    {0x51, 0xd9, 0x44, 0x24, 0x14}},
};
const std::size_t kHookCount = sizeof(kHooks) / sizeof(kHooks[0]);

}  // namespace tpw::hook
