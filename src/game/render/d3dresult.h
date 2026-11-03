// The Direct3D wrappers' handling of a device call's result: kept at 0x879cd4, a failure's message written to the engine debug log, anything but 0 on the engine's error list.
#pragma once

#include <cstdint>

#include "dxerrors.h"
#include "enginedebug.h"
#include "engineerrors.h"

namespace d3dresult {

inline u32 &result() { return *reinterpret_cast<u32 *>((uintptr_t)0x00879cd4); }

// A device call's result kept; a failure's message written to the engine debug log.
inline void keep(u32 hr) {
    result() = hr;
    if (static_cast<s32>(hr) < 0) EngineDebug_Log(DxResult_Message(hr));
}

// The kept result, unless 0, also on the engine's error list: whether it was 0.
inline u32 pushed(u32 kind) {
    if (result() != 0) EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, kind, result());
    return result() == 0;
}

}  // namespace d3dresult
