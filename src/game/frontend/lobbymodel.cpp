#include "lobbymodel.h"

#include <cstdint>
#include <cstring>

#include "../math/matrix.h"
#include "../math/x87.h"
#include "../model/holder.h"
#include "../motion/animator.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *entry(const u8 *model) { return reinterpret_cast<u8 *const *>(0x007a4610)[at<u32>(model, 4)]; }

float *matrix(const u8 *model) {
    return reinterpret_cast<float *>(at<u8 *>(at<u8 *>(at<u8 *>(entry(model), 8), 4), 0x78) + 0x10);
}

}  // namespace

extern "C" {

void __fastcall LobbyModel_SetPosition(const u8 *model, void *, const float *v) {
    u32 bits[3];
    for (int i = 2; i >= 0; --i) {
        std::memcpy(&bits[i], &v[i], 4);
        bits[i] = X87_FloatThrough(bits[i]);
    }
    float *const m = matrix(model);
    std::memcpy(&m[0xc], bits, 12);
}

void __fastcall LobbyModel_TurnY(const u8 *model, void *, float angle) {
    Matrix_RotateEulerFast(matrix(model), 2, &angle);
}

void __fastcall LobbyModel_SetScale(const u8 *model, void *, float x, float y, float z) {
    const float scale[3] = {x, y, z};
    Holder_SetScale(at<u8 *>(entry(model), 8), scale);
}

u32 __fastcall LobbyModel_Play(const u8 *model, void *, u32 entry_, u32 now) {
    if (at<u32>(model, 4) == 0) return 0;
    return Animator_Play(entry(model), 5, entry_, now ? 1 : 0, 1.0f, 0);
}

u32 __fastcall LobbyModel_TimeLeft(const u8 *model) {
    if (at<u32>(model, 4) == 0) return 0;
    return Layer_GetPlayback(entry(model), nullptr, nullptr, nullptr, 0);
}
}
