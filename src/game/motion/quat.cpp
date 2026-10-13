// Quaternion helpers: reimplementation of 0x474120. The expression trees are the
// original's (tools/x87sym.py); Ghidra's C shows a different association.
#include "quat.h"

extern "C" void __cdecl Quat_ToMatrix(const float *q, float *m) {
    const float x = q[0], y = q[1], z = q[2], w = q[3];
    const float s = 2.0f / (((y * y + z * z) + w * w) + x * x);
    const float sy = s * y, sz = s * z, xs = x * s;
    m[0] = 1.0f - (sz * z + sy * y);
    m[1] = x * sy + sz * w;
    m[2] = x * sz - sy * w;
    m[3] = x * sy - sz * w;
    m[4] = 1.0f - (sz * z + x * xs);
    m[5] = sz * y + xs * w;
    m[6] = x * sz + sy * w;
    m[7] = sz * y - xs * w;
    m[8] = 1.0f - (sy * y + x * xs);
}
