#include "imageload.h"

#include <cstdint>

#include "../core/gameheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const ImageLoadEnv kPorts = {&ImageCodec_InfoFromFile, &SurfaceInfo_Alloc, &ImageCodec_LoadFromFile, &SurfaceInfo_Free};
const ImageLoadEnv *g_env = &kPorts;

}  // namespace

void ImageLoad_SetEnv(const ImageLoadEnv *env) { g_env = env ? env : &kPorts; }

extern "C" u8 __fastcall Image_LoadFile(u8 *image, void *, const char *folder, const char *name) {
    ImageCodec codec;
    SurfaceInfo_Ctor(&codec.info);
    codec.vtable = reinterpret_cast<void *const *>((uintptr_t)0x006fefa8);
    SurfaceInfo surface;
    SurfaceInfo_Ctor(&surface);
    char path[256];
    u32 length = 0;
    while (*folder)
        path[length++] = *folder++;
    while (*name)
        path[length++] = *name++;
    path[length] = 0;

    s32 status;
    if (*g_env->infoFromFile(&codec, nullptr, &status, path, nullptr, &surface) < 0)
        return 0;  // sic: nothing freed
    g_env->alloc(&surface, nullptr, &status);
    if (*g_env->loadFromFile(&codec, nullptr, &status, &surface, path, nullptr, nullptr) < 0) {
        g_env->free(&surface);
        return 0;
    }
    const bool paletted = surface.paletted == 1;
    if (at<void *>(image, 0xc))
        GameHeap_Free(at<void *>(image, 0xc));
    const u32 size = paletted ? surface.width * surface.height : (surface.width * surface.height) << 2;
    u8 *out = static_cast<u8 *>(GameHeap_Alloc(size));
    at<u8 *>(image, 0xc) = out;
    if (!out)
        return 0;  // sic: the surface's bits kept
    const u8 *in = static_cast<const u8 *>(surface.bits);
    if (paletted) {
        for (u32 k = 0; k < size; ++k)
            out[k] = in[k];
    } else {
        for (u32 y = 0; y < surface.height; ++y) {
            for (u32 x = 0; x < surface.width; ++x) {
                out[0] = in[3];
                out[1] = in[2];
                out[2] = in[1];
                out[3] = in[0];
                out += 4;
                in += 4;
            }
        }
    }
    at<u32>(image, 0) = surface.width;
    at<u32>(image, 4) = surface.height;
    at<u32>(image, 8) = paletted ? 1 : 4;
    g_env->free(&surface);
    return 1;
}
