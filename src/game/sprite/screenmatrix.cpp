#include "screenmatrix.h"

#include <cstring>

#include "../math/matrix.h"
#include "../math/sintable.h"
#include "../math/x87.h"

namespace {

u32 bits(float f) {
    u32 u;
    std::memcpy(&u, &f, 4);
    return u;
}

float from(u32 u) {
    float f;
    std::memcpy(&f, &u, 4);
    return f;
}

float global(u32 address) { return *reinterpret_cast<const float *>(address); }

}  // namespace

extern "C" {

void __cdecl Sprite_ScreenMatrix(float *out, s16 x, s16 y, s16 w, s16 h, float left, float top, float width,
                                 float height, s16 cx, s16 cy, s16 angle) {
    float rotate[16], back[16], m[16], scale[16], turn[16];
    Matrix_Identity(rotate);
    Matrix_Identity(back);
    Matrix_Identity(m);
    Matrix_Identity(scale);
    Matrix_Identity(turn);
    const float sx = (float)(X87Reg(width) / X87Reg((double)w));  // fild w; fdivr width
    scale[10] = 0.0f;
    const float sy = (float)(X87Reg(height) / X87Reg((double)h));
    m[12] = (float)(X87Reg((double)x) * sx - left - X87Reg(sx) * 1024.0f);
    m[13] = (float)(X87_SubR(X87Reg((double)y), X87Reg(1536.0f)) * sy - top - X87Reg(sy) * 768.0f);
    scale[0] = (float)(X87Reg(2.0f) / sx * global(0x0074f888));
    scale[5] = (float)(X87Reg(2.0f) / sy * global(0x0074f88c));
    if (angle) {
        const SinTable sines = SinTable_Game();
        const float t = (float)(X87Reg((double)angle) * 3.14159274f * 0.00048828125f);
        const float sine = from(X87_FloatThrough(bits(SinTable_At(sines, (float)(X87Reg(sines.scale) * t)))));
        const float cosine = SinTable_At(sines, (float)((X87Reg(t) - -1.57079637f) * sines.scale));
        const float px = (float)((X87Reg((double)cx) - 1024.0f) * sx);
        turn[12] = from(bits(px) ^ 0x80000000);  // fst; fchs; fstp
        back[12] = px;
        const float py = (float)((X87_SubR(X87Reg((double)cy), X87Reg(1536.0f)) - 768.0f) * sy);
        turn[13] = from(bits(py) ^ 0x80000000);
        back[13] = py;
        rotate[0] = rotate[5] = from(X87_FloatThrough(bits(cosine)));
        rotate[1] = (float)(X87Reg(sine) * -1.33333337f);
        rotate[4] = (float)(X87Reg(sine) * 0.75f);
        Matrix_Mul(turn, rotate);
        Matrix_Mul(turn, back);
    }
    Matrix_Mul(m, turn);
    Matrix_Mul(m, scale);
    std::memcpy(out, m, sizeof m);
}

double __cdecl Sprite_FlipY(s32 y) { return X87_SubR(X87Reg((double)y) / global(0x0074f898), X87Reg(1.0f)).v; }
}
