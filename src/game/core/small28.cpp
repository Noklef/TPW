#include "small28.h"

#include <cstdint>

#include "small17.h"
#include "string.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 __fastcall NamedList_Find(u8 **ref, void *, const u8 *key, u8 **out) {
    if (u8 *list = ref[0]) {
        for (u8 *node = at<u8 *>(list, 4); node;) {
            u8 *next = at<u8 *>(node, 4);
            u8 *object = at<u8 *>(node, 0);
            if (Names_Equal(object + 4, key) == 1) {
                *out = object;
                ref[1] = object;
                return 1;
            }
            node = next;
        }
    }
    ref[1] = nullptr;
    return 0;
}

u32 __fastcall Obj_HasNamedObject(u8 *object, void *, const u8 *key) {
    if (u8 *list = at<u8 *>(object, 0x4c)) {
        for (u8 *node = at<u8 *>(list, 4); node;) {
            u8 *next = at<u8 *>(node, 4);
            if (Names_Equal(at<u8 *>(node, 0) + 4, key) != 0)
                return 1;
            node = next;
        }
    }
    return 0;
}

void __fastcall Strings5_Dtor(u8 *object) {
    for (int offset = 0x50; offset >= 0; offset -= 0x14)
        String_Dtor((String *)(object + offset));
}

}  // extern "C"
