#include "sndreverb.h"

#include <cstdint>
#include <cstring>

#include "../files/filestream.h"
#include "../files/handles.h"
#include "gameheap.h"
#include "string.h"
#include "vcall.h"

namespace {

template <typename T> T Get(const void *p, u32 offset) {
    T v;
    std::memcpy(&v, static_cast<const u8 *>(p) + offset, sizeof v);
    return v;
}
template <typename T> void Put(void *p, u32 offset, T v) { std::memcpy(static_cast<u8 *>(p) + offset, &v, sizeof v); }
u8 *Ptr(u32 address) { return reinterpret_cast<u8 *>(static_cast<uintptr_t>(address)); }

constexpr u32 kVersion = 2000;
constexpr u32 kRecord = 0x34;

}  // namespace

void __fastcall SndReverb_Load(u8 *system) {
    if (Get<u32>(system, 0x68)) return;
    StreamBase stream;
    StreamBase_Ctor(&stream);
    stream.vtable = reinterpret_cast<void *const *>(static_cast<uintptr_t>(0x6fd930));   // FileStream
    String path;
    String_CtorCopy(&path, nullptr, reinterpret_cast<const StringView *>(system + 0x30));
    String_AppendStr(&path, nullptr, "SndReverb.map");
    s32 status;
    if (*FileStream_OpenPath(&stream, nullptr, &status, path.data, 0x80000001u) >= 0) {
        u8 *const count = system + 0x6c;
        if (stream.source) vcall<s32>(stream.source, 4, count, 4u);
        if (Get<u32>(count, 0) == kVersion) {
            if (stream.source) vcall<s32>(stream.source, 4, count, 4u);
            const u32 bytes = Get<u32>(count, 0) * kRecord;
            u8 *const records = static_cast<u8 *>(GameHeap_Alloc(bytes));
            std::memset(records, 0, bytes);
            Put<u32>(system, 0x68, static_cast<u32>(reinterpret_cast<uintptr_t>(records)));
            u32 record[13];
            u8 present;
            for (u32 i = 0, at = 0; i < Get<u32>(count, 0); ++i, at += kRecord) {
                if (stream.source) vcall<s32>(stream.source, 4, &present, 1u);
                if (!present) {
                    Put<u32>(Ptr(Get<u32>(system, 0x68)), at, 0xffffffffu);
                    continue;
                }
                for (u32 k = 0; k < 13 && stream.source; ++k) vcall<s32>(stream.source, 4, &record[k], 4u);
                std::memcpy(Ptr(Get<u32>(system, 0x68)) + at, record, kRecord);
            }
        }
        StreamBase_Close(&stream);
    }
    String_Dtor(&path);
    StreamBase_Dtor(&stream);
}
