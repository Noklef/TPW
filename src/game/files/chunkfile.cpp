#include "chunkfile.h"

#include <cstdint>

#include "../core/gameerror.h"
#include "../core/vcall.h"
#include "file.h"

namespace {

u8 *gameError() { return reinterpret_cast<u8 *>((uintptr_t)0x00785de8); }

}  // namespace

extern "C" {

u32 __fastcall ChunkFile_ReadStream(u8 *file, void *, u8 *stream, const char *name) {
    const s32 size = vcall<s32>(stream, 0x14);
    if (size == -1) {
        GameError_Set(gameError(), nullptr, 3, 0, name);
        vcall<void>(stream, 4);
        return 0;
    }
    u8 *const data = static_cast<u8 *>(Mem_GlobalAlloc((u32)size));
    if (data == nullptr) {
        vcall<void>(stream, 4);
        GameError_Set(gameError(), nullptr, 8, 0, name);
        return 0;
    }
    if (vcall<s32>(stream, 8, data, size) != size) {
        GameError_Set(gameError(), nullptr, 3, 0, name);
        vcall<void>(stream, 4);
        Mem_GlobalFree(data);
        return 0;
    }
    vcall<void>(stream, 4);
    if (size < 0x2c || *reinterpret_cast<const u32 *>(data) != *reinterpret_cast<const u32 *>(file + 0x10)) {
        Mem_GlobalFree(data);
        return 0;
    }
    const uintptr_t base = (uintptr_t)data;
    uintptr_t chunk = base + 0x24;
    while ((s32)(chunk - base) <= size - 8) {
        const u32 tag = *reinterpret_cast<const u32 *>(chunk);
        const s32 length = *reinterpret_cast<const s32 *>(chunk + 4);
        const uintptr_t body = chunk + 8;
        const uintptr_t end = body + (u32)length;
        if ((u32)(end - base) > (u32)size) break;
        if (tag == 0x20444e45 && length == 0) break;  // "END "
        for (u8 *const *node = *reinterpret_cast<u8 *const *const *>(file + 4); node != nullptr;) {
            u8 *const handler = node[0];
            node = reinterpret_cast<u8 *const *>(node[1]);
            if (vcall<u32>(handler, 0, tag) != 0) vcall<void>(handler, 4, reinterpret_cast<u8 *>(body), length);
        }
        chunk = end;
    }
    Mem_GlobalFree(data);
    return 1;
}
}
