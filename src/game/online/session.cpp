#include "session.h"

#include <cstdint>
#include <initializer_list>

#include "../core/gameheap.h"
#include "../core/buffers.h"
#include "../core/vcall.h"
#include "../files/file.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// OutputDebugStringA (0x6fd1e0) with the game's text at address text.
void Say(u32 text) {
    using Output = void(__stdcall *)(const char *);
    (*reinterpret_cast<const Output *>((uintptr_t)0x006fd1e0))(reinterpret_cast<const char *>((uintptr_t)text));
}

// A WEA*R import with no arguments (__thiscall: the object in ECX) through its slot.
void CallImport(u32 slot, void *object) {
    using Method = u32(__fastcall *)(void *, void *);
    (*reinterpret_cast<const Method *>((uintptr_t)slot))(object, nullptr);
}

// The News connection (+0x6c, set) released (News::Release), destroyed (News::~News) and freed when still set, +0x6c cleared.
void FreeNews(u8 *session) {
    CallImport(0x006fd58c, at<u8 *>(session, 0x6c));
    if (u8 *const news = at<u8 *>(session, 0x6c)) {
        CallImport(0x006fd588, news);
        GameHeap_Free(news);
    }
    at<u32>(session, 0x6c) = 0;
    Say(0x007733f0);
}

// A list member's state reset: +4 = -1, +8 to +0x20 cleared.
void ResetList(u8 *list) {
    at<u32>(list, 4) = 0xffffffff;
    for (u32 offset = 8; offset <= 0x20; offset += 4)
        at<u32>(list, offset) = 0;
}

// The same, then its native list (+0x30, read first; when set) released through the import and +0x30 cleared.
void ResetNativeList(u8 *list, u32 release) {
    u8 *const native = at<u8 *>(list, 0x30);
    ResetList(list);
    if (native) {
        CallImport(release, native);
        at<u32>(list, 0x30) = 0;
    }
}

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

void __fastcall Online_ReleaseLogin(u8 *session) {
    if (!at<u8 *>(session, 0))
        return;
    CallImport(0x006fd45c, at<u8 *>(session, 0));   // Auth::Release
    if (u8 *const auth = at<u8 *>(session, 0)) {
        CallImport(0x006fd460, auth);   // Auth::~Auth
        GameHeap_Free(auth);
    }
    at<u32>(session, 0) = 0;
    Say(0x0077345c);
}

void __fastcall Online_ReleaseNews(u8 *session) {
    if (!at<u8 *>(session, 0x6c))
        return;
    FreeNews(session);
}

void __fastcall Online_ResetNews(u8 *session) {
    if (at<u8 *>(session, 0x6c))
        FreeNews(session);
    at<u32>(session, 0x74) = 0xffffffff;
    at<u32>(session, 0xc8) = 0;
    at<u32>(session, 0xcc) = 0;
    at<u32>(session, 0x78) = 0;
    vcall<void>(session + 0x7c, 0x14, 0x00786de4u);
    at<u32>(session, 0x90) = 0xffffffff;
    at<u32>(session, 0x94) = 0;
    vcall<void>(session + 0x98, 0x14, 0x0078a374u);
    at<u32>(session, 0xac) = 0xffffffff;
    at<u32>(session, 0xb0) = 0;
    vcall<void>(session + 0xb4, 0x14, 0x0078a374u);
    at<u32>(session, 0xd0) = 0;
}

u8 *__fastcall Auth_DeletingDtor(u8 *auth, void *, u32 flags) {
    CallImport(0x006fd460, auth);   // Auth::~Auth
    if (flags & 1)
        GameHeap_Free(auth);
    return auth;
}

void __fastcall Online_ReleaseUpload(u8 *session) {
    if (u8 *const upload = at<u8 *>(session, 0x104)) {
        CallImport(0x006fd5ec, upload);   // Upload::Release
        at<u32>(session, 0x104) = 0;
    }
}

void __fastcall Online_ResetUpload(u8 *session) {
    Online_ReleaseUpload(session);
    for (u32 offset : {0x328u, 0x320u, 0x324u, 0x32cu, 0x344u})
        at<u32>(session, offset) = 0;
    vcall<void>(session + 0x348, 0x14, 0x0078a374u);
    vcall<void>(session + 0x35c, 0x14, 0x0078a374u);
    for (u32 offset = 0x370; offset <= 0x380; offset += 4)
        at<u32>(session, offset) = 0;
}

void __fastcall Online_ResetDownload(u8 *session) {
    Say(0x007733d0);
    if (u8 *const object = at<u8 *>(session, 0x3a0)) {
        at<u32>(session, 0x3a0) = 0;
        vcall<void>(object, 0, 1u);
    }
    if (u8 *const block = at<u8 *>(session, 0x39c)) {
        GameHeap_Free(block);
        at<u32>(session, 0x39c) = 0;
    }
    for (u32 offset : {0x390u, 0x394u, 0x384u, 0x388u, 0x38cu, 0x398u, 0x3a8u, 0x3c0u, 0x3c4u})
        at<u32>(session, offset) = 0;
    vcall<void>(session + 0x3dc, 0x14, 0x00786de4u);
    at<u32>(session, 0x3f0) = 0;
}

void __fastcall Online_LoginResult(u8 *session, void *, s32 status, u32, u32, u8 *record) {
    u32 error;
    switch (status) {
    case 60:
        Say(0x00773480);
        at<u32>(session, 0x34) = at<u32>(record, 0);
        at<u32>(session, 0x38) = at<u32>(record, 4);
        vcall<void>(session + 0x3c, 0x14, record + 0x30);
        for (u32 offset = 0; offset < 0x10; offset += 4)
            at<u32>(session, 0x50 + offset) = at<u32>(record, 0x58 + offset);
        if (--at<s32>(record, 0x68) == 0 && record)
            GameHeap_Free(record);
        at<u32>(session, 0x68) = 0;
        at<u32>(session, 0x60) = 1;
        at<u32>(session, 0x64) = 1;
        return;
    case -6:
        error = 0x4e;
        break;
    case -5:
        error = 0x4d;
        break;
    case -4:
        error = 0x4c;
        break;
    case -2:
        error = 0x4b;
        break;
    default:
        error = 0x37;
        break;
    }
    at<u32>(session, 0x68) = error;
    at<u32>(session, 0x60) = 0;
    at<u32>(session, 0x64) = 1;
}

void __fastcall Online_ResetCity(u8 *session) {
    at<u32>(session, 0xd8) = Time_GetMs();
    at<u32>(session, 0xdc) = 1;
    if (at<u8 *>(session, 0xd4)) {
        CallImport(0x006fd4f4, at<u8 *>(session, 0xd4));   // City::Release
        if (u8 *const city = at<u8 *>(session, 0xd4)) {
            CallImport(0x006fd540, city);   // City::~City
            GameHeap_Free(city);
        }
        at<u32>(session, 0xd4) = 0;
        Say(0x0077386c);
    }
    ResetNativeList(at<u8 *>(session, 0xe0), 0x006fd538);   // ccCityList::Release
    for (u32 offset = 0xe4; offset <= 0xec; offset += 4)
        ResetNativeList(at<u8 *>(session, offset), 0x006fd4e8);   // ccParkList::Release
    ResetList(at<u8 *>(session, 0xf0));
    Shared_Release66(at<u8 *>(session, 0xf0));
    ResetList(at<u8 *>(session, 0xf4));
    Shared_Release66(at<u8 *>(session, 0xf4));
    ResetList(at<u8 *>(session, 0xf8));
    Shared_Release10(at<u8 *>(session, 0xf8));
    Obj_ResetAndRelease26(at<u8 *>(session, 0xfc), nullptr, 0, 0);
    Obj_ResetAndRelease34(at<u8 *>(session, 0x100), nullptr, 0, 0);
}
}
