#include "depthsort.h"

#include <cstring>

extern "C" {

void __cdecl DepthSort_Triangles(u32 *keys, u8 *triangles, s32 lo, s32 hi) {
    for (;;) {
        const u32 pivot = keys[static_cast<s32>(static_cast<u32>(lo) + static_cast<u32>(hi)) >> 1];
        s32 i = lo, j = hi;
        do {
            while (i <= hi && keys[i] > pivot) i++;
            while (j > lo && keys[j] < pivot) j--;
            if (i > j) break;
            const u32 key = keys[i];
            keys[i] = keys[j];
            keys[j] = key;
            u8 triangle[6];
            std::memcpy(triangle, triangles + i * 6, 6);
            std::memcpy(triangles + i * 6, triangles + j * 6, 6);
            std::memcpy(triangles + j * 6, triangle, 6);
            i++;
            j--;
        } while (i <= j);
        if (lo < j) DepthSort_Triangles(keys, triangles, lo, j);
        if (hi <= i) return;
        lo = i;
    }
}
}
