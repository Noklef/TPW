// In-game comparison of one hooked function (test tooling): each call runs the port, then the original from the same state,
// and logs what differs in tp.exe's data section and the object at [0x87afe8]. The port's result stays. The benches run the
// port beside stand-ins; this sees the real callees, the real FPU state and the game's own data. Only the first 600 calls.
// tpw.ini [tpw] shadow=ADDRESS (hex; the function's hook must be on).
//   shadow N: fpu cw C, top entry/port/orig, mxcsr entry/port/orig, D dwords differ
//     AAAAAAAA was W (float) port P (float) orig O (float)
#pragma once

namespace tpw::testing {

void start_shadow(const char *ini);

}  // namespace tpw::testing
