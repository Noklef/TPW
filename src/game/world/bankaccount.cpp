#include "bankaccount.h"

#include "../core/savegame.h"
#include "things.h"

namespace {

// The head's fields as saved (Save_Field each); a load reads the first three the same way and the rest through Save_ReadU32.
const SaveSpec kHead[] = {{0x118, 4, 0x00759df4}, {0xc, 4, 0x00759de8}, {0x10, 4, 0x00759dd8}, {0x114, 4, 0x00759dc4},
                          {0x11c, 4, 0x00759db4}, {0x120, 4, 0x00759da4}, {0x124, 4, 0x00759d94}};
// A loan's eight dwords (the first loan's offsets; the others 0x20 on).
const SaveSpec kLoan[] = {{0x14, 4, 0x00759d78}, {0x18, 4, 0x00759d58}, {0x1c, 4, 0x00759d3c}, {0x20, 4, 0x00759d14},
                          {0x24, 4, 0x00759cf4}, {0x28, 4, 0x00759cd8}, {0x2c, 4, 0x00759cbc}, {0x30, 4, 0x00759c9c}};

bool ReadU32s(Stream *stream, u8 *object, const SaveSpec *spec, u32 count) {
    for (u32 i = 0; i < count; ++i)
        if (Save_ReadU32(stream, object + spec[i].at, reinterpret_cast<const char *>(spec[i].name), 0) != 0) return false;
    return true;
}

}  // namespace

extern "C" {

u32 __fastcall BankAccount_Serialize(u8 *account, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    if (Thing_Serialize(account, nullptr, stream, save, version, unused) != 0) return 1;
    if (version < 0 || version > 1) return 0;
    if (save != 0) {
        if (!Save_Fields(stream, save, account, kHead, 7)) return 1;
        for (u32 loan = 0; loan < 8; ++loan)
            if (!Save_Fields(stream, save, account + loan * 0x20, kLoan, 8)) return 1;
        return 0;
    }
    if (!Save_Fields(stream, 0, account, kHead, 3) || !ReadU32s(stream, account, kHead + 3, 4)) return 1;
    for (u32 loan = 0; loan < 8; ++loan)
        if (!ReadU32s(stream, account + loan * 0x20, kLoan, 8)) return 1;
    return 0;
}
}
