#include "scriptfile.h"

#include <cstdint>
#include <cstring>

#include "../coaster/ridebounds.h"
#include "../core/soundset.h"
#include "../core/vwrap.h"
#include "../files/file.h"
#include "../files/vfs.h"
#include "../math/x87.h"
#include "../model/parts.h"
#include "../render/particles.h"
#include "../world/objectdefs.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

s32 Ftol(X87Reg x) { return (s32)(long long)x.v; }

u32 bits(float f) {
    u32 b;
    std::memcpy(&b, &f, 4);
    return b;
}

u8 *thing_slot(u16 handle) { return *reinterpret_cast<u8 **>((uintptr_t)0x007cfb90 + 0x14u * handle); }

void read(void *buffer, u32 size, u32 count, u8 *stream) {
    Object_CallSlot8Product(static_cast<u32>(reinterpret_cast<uintptr_t>(buffer)), size, count, stream);
}

// A count read into the record at `field` and, when not 0, its array (count << shift bytes, 0 for a size of 0) at `array`, zeroed or not.
void counted(u8 *vm, u32 field, u32 array, u32 shift, bool zero, u8 *stream) {
    read(vm + field, 4, 1, stream);
    if (!at<u32>(vm, field)) return;
    const u32 size = at<u32>(vm, field) << shift;
    u8 *const block = size ? static_cast<u8 *>(Mem_GlobalAlloc(size)) : nullptr;
    at<u8 *>(vm, array) = block;
    if (zero) std::memset(block, 0, at<u32>(vm, field) << shift);
}

}  // namespace

extern "C" {

u32 __cdecl RSSE_LoadScriptFile(const char *name, u32 handle, u32 quiet) {
    if (!global<u32>(0x008791a0)) return 0;
    u32 result = 0;
    u8 *const stream = reinterpret_cast<u8 *>(VFS_Open(name));
    if (!stream) return 0;
    u32 word;
    read(&word, 4, 1, stream);
    if (word != 0x45535352) {   // "RSSE"
        Object_FinishAndDelete(stream);
        return result;
    }
    u32 version;
    read(&version, 4, 1, stream);
    u8 *const vm = static_cast<u8 *>(Mem_GlobalAlloc(0xf4));
    std::memset(vm, 0, 0xf4);
    ++global<u32>(0x008791ac);
    at<u8 *>(vm, 0) = global<u8 *>(0x008791b0);
    at<u8 *>(vm, 4) = nullptr;
    if (global<u8 *>(0x008791b0)) at<u8 *>(global<u8 *>(0x008791b0), 4) = vm;
    global<u8 *>(0x008791b0) = vm;
    counted(vm, 0x8c, 0x1c, 2, true, stream);
    counted(vm, 0x54, 0x20, 2, false, stream);
    read(vm + 0x94, 4, 1, stream);
    counted(vm, 0x58, 0x24, 3, true, stream);
    counted(vm, 0x64, 0x28, 4, true, stream);
    counted(vm, 0x7c, 0x2c, 5, true, stream);
    for (int k = 0; k < 4; ++k) read(&word, 4, 1, stream);
    read(vm + 0x50, 4, 1, stream);
    if (at<u32>(vm, 0x50)) {
        const u32 size = at<u32>(vm, 0x50) * 4;
        u8 *const code = size ? static_cast<u8 *>(Mem_GlobalAlloc(size)) : nullptr;
        at<u8 *>(vm, 0x18) = code;
        read(code, 4, at<u32>(vm, 0x50), stream);
    }
    read(vm + 0x90, 4, 1, stream);
    if (const u32 size = at<u32>(vm, 0x90)) {
        u8 *const data = static_cast<u8 *>(Mem_GlobalAlloc(size));
        at<u8 *>(vm, 0x34) = data;
        read(data, 1, at<u32>(vm, 0x90), stream);
    }
    at<u32>(vm, 0x40) = at<u32>(vm, 0x54) - 1;
    at<u32>(vm, 8) = global<u32>(0x008791a8);
    ++global<u32>(0x008791a8);
    at<u16>(vm, 0x88) = 1000;
    at<u16>(vm, 0x8a) = 1000;
    at<u16>(vm, 0xe4) = 1000;
    at<u16>(vm, 0xc0) = 0x32;
    at<u32>(vm, 0x70) = 1;
    at<u32>(vm, 0x74) = 0xffffffff;
    at<u32>(vm, 0xa8) = 0xffff;
    at<u32>(vm, 0xd8) = 0xffffffff;
    at<u32>(vm, 0xd4) = 0;
    at<u32>(vm, 0xd0) = 0;
    at<u32>(vm, 0xe0) = 0;
    at<u16>(vm, 0xe6) = 0xffff;
    char folder[0x100];
    std::strcpy(folder, name);
    s32 cut = static_cast<s32>(std::strlen(folder));
    while (cut > 0 && folder[cut] != '\\') --cut;
    folder[cut + 1] = 0;
    char *const copy = static_cast<char *>(Mem_GlobalAlloc(static_cast<u32>(std::strlen(folder)) + 1));
    at<char *>(vm, 0x38) = copy;
    std::strcpy(copy, folder);
    at<u16>(vm, 0xac) = static_cast<u16>(handle);
    result = at<u32>(vm, 8);
    if (static_cast<u16>(handle)) at<u32>(vm, 0xc8) = at<u32>(thing_slot(static_cast<u16>(handle)), 0x20);
    if (at<u16>(vm, 0xac) && at<u32>(vm, 0xc8)) {
        u32 count = 0;
        for (u32 key = 1; Model_FindPartByKey4(global<u8 *>(0x007a4610 + 4 * at<u32>(vm, 0xc8)), 0x80, key) != 0xffffffff; ++key) ++count;
        if (count) {
            u8 *const parts = static_cast<u8 *>(Mem_GlobalAlloc(count * 4));
            at<u8 *>(vm, 0x30) = parts;
            std::memset(parts, 0, count * 4);
        }
        at<u32>(vm, 0x4c) = count;
    }
    if (!quiet && at<u16>(vm, 0xac)) {
        const u16 id = at<u16>(thing_slot(at<u16>(vm, 0xac)), 0xe);
        const s32 effect = at<s32>(ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, id), 0x60);
        float box[6];
        if (effect && at<u32>(vm, 0xc8) && Ride_Bounds(at<s32>(vm, 0xc8), box)) {
            const float half = global<float>(0x00700fd0), scale = global<float>(0x00700fdc);
            u8 *const instance = global<u8 *>(0x007a4610 + 4 * at<u32>(vm, 0xc8));
            const X87Reg zh = (X87Reg(box[2]) + box[5]) * half;
            const X87Reg xh = (X87Reg(box[0]) + box[3]) * half;
            const float cz = static_cast<float>(zh);
            const float y = X87_Through(at<float>(at<u8 *>(at<u8 *>(at<u8 *>(instance, 8), 4), 0x78), 0x44));
            const float cx = static_cast<float>(xh);
            const s32 iz = Ftol(zh * scale);
            const s32 iy = Ftol(X87Reg(y) * scale);
            const s32 ix = Ftol(X87Reg(cx) * scale);
            const s32 system = Particle_Spawn(effect, ix, iy, iz);
            if (at<u32>(vm, 0xc8) && Ride_Bounds(at<s32>(vm, 0xc8), box)) {
                const X87Reg dz = X87Reg(box[5]) - box[2];
                const X87Reg dy = X87Reg(box[4]) - box[1];
                const X87Reg dx = X87Reg(box[3]) - box[0];
                const float dzf = static_cast<float>(dz), dyf = static_cast<float>(dy), dxf = static_cast<float>(dx);
                const s32 jz = Ftol(dz * global<float>(0x00701050));
                const s32 jy = Ftol(X87Reg(dyf) * global<float>(0x00701054));
                const s32 jx = Ftol(X87Reg(dxf) * global<float>(0x00701050));
                ParticleSystem_SetVectorB(static_cast<u32>(system), jx, jy, jz);
                const float radius = static_cast<float>((X87Reg(dzf) + dxf) * half);
                at<u32>(vm, 0xd4) = Sound_AddOccluder(bits(cx), bits(y), bits(cz), bits(radius), bits(dyf));
            }
        }
    }
    Object_FinishAndDelete(stream);
    return result;
}
}
