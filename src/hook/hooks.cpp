// The hook registry: every original function we have reimplemented.
// Add a row here when a reimplementation is ready; mark it in re/symbols.csv too.
#include "hooks.h"

#include "../game/rsse/rsse.h"

namespace tpw::hook {

const Hook kHooks[] = {
    {"RSSE_FetchWord",    0x00557a70, (void *)&RSSE_FetchWord,    {0x56, 0x8b, 0x74, 0x24, 0x08}},
    {"RSSE_OperandValue", 0x005573a0, (void *)&RSSE_OperandValue, {0x8b, 0x44, 0x24, 0x08, 0x8b}},
};
const std::size_t kHookCount = sizeof(kHooks) / sizeof(kHooks[0]);

}  // namespace tpw::hook
