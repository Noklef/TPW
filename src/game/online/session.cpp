#include "session.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 __fastcall Online_LoggedIn(const u8 *session) { return at<u32>(session, 0x60); }

u32 __fastcall Online_Restricted(const u8 *session) {
    return at<u32>(session, 0x60) == 0 || (session[0x58] & 0x20) ? 1 : 0;
}

const char *__fastcall Online_String14(const u8 *session) { return at<const char *>(session, 0x14); }

const char *__fastcall Online_String28(const u8 *session) { return at<const char *>(session, 0x28); }

const u16 *__fastcall Online_UserName(const u8 *session) { return at<const u16 *>(session, 0x44); }

u32 __fastcall Online_UserFlags(const u8 *session) { return at<u32>(session, 0x58); }

u32 __fastcall Online_User5c(const u8 *session) { return at<u32>(session, 0x5c); }
}
