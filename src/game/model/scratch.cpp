#include "scratch.h"

#include "../core/globalmem.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>(address); }

}  // namespace

extern "C" {

void __cdecl Model_AllocVertexScratch() {
    global<void *>(0x0079f7e0) = GlobalMem_Alloc(global<u32>(0x0079f7d0) * 12, 1);
}

void __cdecl Model_FreeVertexScratch() {
    GlobalMem_Free(global<void *>(0x0079f7e0));
    global<void *>(0x0079f7e0) = nullptr;
}
}
