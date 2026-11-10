#include "session.h"

#include <cstdint>
#include <cstring>
#include <initializer_list>

#include "../core/crt.h"
#include "../core/dequeiter.h"
#include "../core/gameheap.h"
#include "../core/buffers.h"
#include "../core/language.h"
#include "../core/selectors.h"
#include "../core/string.h"
#include "../core/vcall.h"
#include "../core/vdtor.h"
#include "../files/file.h"
#include "../files/gamepaths.h"
#include "../files/userslots.h"
#include "../ui/dialog7cc530open.h"
#include "../ui/dialog7cc944row.h"
#include "../ui/dialog7cca94.h"
#include "../ui/dialogs.h"
#include "../ui/dialuphangup.h"
#include "ftp.h"
#include "itemlist.h"
#include "items.h"
#include "listrequest.h"
#include "sessionlogin.h"
#include "chatlists.h"
#include "chatcommand.h"
#include "../ui/window7cc8c0handler.h"

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
    Say(reinterpret_cast<uintptr_t>(&gSession_7733f0));
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

// The City (+0xd4, when set) released (City::Release), destroyed (City::~City) and freed when still set, +0xd4 cleared; "ONLINE CITY:
// Connection released" said.
void ReleaseCity(u8 *session) {
    if (at<u8 *>(session, 0xd4)) {
        CallImport(0x006fd4f4, at<u8 *>(session, 0xd4));   // City::Release
        if (u8 *const city = at<u8 *>(session, 0xd4)) {
            CallImport(0x006fd540, city);   // City::~City
            GameHeap_Free(city);
        }
        at<u32>(session, 0xd4) = 0;
        Say(reinterpret_cast<uintptr_t>(&gSession_77386c));
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

namespace {

// The login's state cleared, its three strings assigned "" (the auth at +0 is another's).
void ClearLogin(u8 *session) {
    for (u32 offset : {0x64u, 0x60u, 0x68u, 4u, 8u})
        at<u32>(session, offset) = 0;
    vcall<void>(session + 0xc, 0x14, reinterpret_cast<uintptr_t>(&gWindow7cc8c0handler_78a374));
    vcall<void>(session + 0x20, 0x14, reinterpret_cast<uintptr_t>(&gWindow7cc8c0handler_78a374));
    at<u32>(session, 0x34) = 0;
    at<u32>(session, 0x38) = 0;
    vcall<void>(session + 0x3c, 0x14, reinterpret_cast<uintptr_t>(&gWindow7cc8c0handler_78a374));
    at<u32>(session, 0x50) = 0;
}

// The News' state cleared, its three strings assigned "".
void ClearNews(u8 *session) {
    at<u32>(session, 0x74) = 0xffffffff;
    at<u32>(session, 0xc8) = 0;
    at<u32>(session, 0xcc) = 0;
    at<u32>(session, 0x78) = 0;
    vcall<void>(session + 0x7c, 0x14, 0x00786de4u);
    at<u32>(session, 0x90) = 0xffffffff;
    at<u32>(session, 0x94) = 0;
    vcall<void>(session + 0x98, 0x14, reinterpret_cast<uintptr_t>(&gWindow7cc8c0handler_78a374));
    at<u32>(session, 0xac) = 0xffffffff;
    at<u32>(session, 0xb0) = 0;
    vcall<void>(session + 0xb4, 0x14, reinterpret_cast<uintptr_t>(&gWindow7cc8c0handler_78a374));
    at<u32>(session, 0xd0) = 0;
}

}  // namespace

void __fastcall Online_ReleaseLogin(u8 *session) {
    if (!at<u8 *>(session, 0))
        return;
    CallImport(0x006fd45c, at<u8 *>(session, 0));   // Auth::Release
    if (u8 *const auth = at<u8 *>(session, 0)) {
        CallImport(0x006fd460, auth);   // Auth::~Auth
        GameHeap_Free(auth);
    }
    at<u32>(session, 0) = 0;
    Say(reinterpret_cast<uintptr_t>(&gOnline_ReleaseLogin_77345c));
}

void __fastcall Online_ReleaseNews(u8 *session) {
    if (!at<u8 *>(session, 0x6c))
        return;
    FreeNews(session);
}

void __fastcall Online_ResetNews(u8 *session) {
    if (at<u8 *>(session, 0x6c))
        FreeNews(session);
    ClearNews(session);
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
    vcall<void>(session + 0x348, 0x14, reinterpret_cast<uintptr_t>(&gWindow7cc8c0handler_78a374));
    vcall<void>(session + 0x35c, 0x14, reinterpret_cast<uintptr_t>(&gWindow7cc8c0handler_78a374));
    for (u32 offset = 0x370; offset <= 0x380; offset += 4)
        at<u32>(session, offset) = 0;
}

namespace {

// "Reseting" said; the object at +0x3a0 deleted, the block at +0x39c freed, both cleared.
void DropDownload(u8 *session) {
    Say(reinterpret_cast<uintptr_t>(&gSession_7733d0));
    if (u8 *const object = at<u8 *>(session, 0x3a0)) {
        at<u32>(session, 0x3a0) = 0;
        vcall<void>(object, 0, 1u);
    }
    if (u8 *const block = at<u8 *>(session, 0x39c)) {
        GameHeap_Free(block);
        at<u32>(session, 0x39c) = 0;
    }
}

}  // namespace

void __fastcall Online_ResetDownload(u8 *session) {
    DropDownload(session);
    for (u32 offset : {0x390u, 0x394u, 0x384u, 0x388u, 0x38cu, 0x398u, 0x3a8u, 0x3c0u, 0x3c4u})
        at<u32>(session, offset) = 0;
    vcall<void>(session + 0x3dc, 0x14, 0x00786de4u);
    at<u32>(session, 0x3f0) = 0;
}

void __fastcall Online_LoginResult(u8 *session, void *, s32 status, u32, u32, u8 *record) {
    u32 error;
    switch (status) {
    case 60:
        Say(reinterpret_cast<uintptr_t>(&gOnline_LoginResult_773480));
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
    ReleaseCity(session);
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
u8 *__fastcall Online_OpenCity(u8 *session) {
    // The SEH frame is left out (it frees the block when City::City throws).
    if (at<u8 *>(session, 0xd4))
        return at<u8 *>(session, 0xd4);
    u8 *city = static_cast<u8 *>(GameHeap_Alloc(0xc));
    if (city) {
        using Ctor = u8 *(__fastcall *)(u8 *, void *);
        city = (*reinterpret_cast<const Ctor *>((uintptr_t)0x006fd4e4))(city, nullptr);   // City::City
    }
    at<u8 *>(session, 0xd4) = city;
    Say(reinterpret_cast<uintptr_t>(&gOnline_OpenCity_7738b0));
    const u32 name = *reinterpret_cast<const u32 *>((uintptr_t)0x007a1524);
    const u32 server = name ? name : 0x006fe5b0;
    const u32 port = (name & 0xffff0000) | *reinterpret_cast<const u16 *>((uintptr_t)0x007a1556);   // (mov ax: the high half left)
    using Connect = u32(__fastcall *)(u8 *, void *, u32, u8 *, u32, u32);
    const u8 *const picked = Global7cc53c_Pick7cc4f0();
    if ((*reinterpret_cast<const Connect *>((uintptr_t)0x006fd534))(at<u8 *>(session, 0xd4), nullptr, server, const_cast<u8 *>(picked), port, 0x005b72c0) != 1) {
        Say(reinterpret_cast<uintptr_t>(&gOnline_OpenCity_773890));
        ReleaseCity(session);
        return at<u8 *>(session, 0xd4);
    }
    Say(reinterpret_cast<uintptr_t>(&gOnline_OpenCity_773854));
    at<u32>(session, 0xd8) = Time_GetMs();
    const u32 a = at<u32>(session, 0x34), b = at<u32>(session, 0x38), c = at<u32>(session, 0x28);
    const u32 language = Language_Id(reinterpret_cast<const u8 *>((uintptr_t)0x0078b530));
    using SetSessionInfo = void(__fastcall *)(u8 *, void *, u32, u32, u32, u32);
    (*reinterpret_cast<const SetSessionInfo *>((uintptr_t)0x006fd53c))(at<u8 *>(session, 0xd4), nullptr, language, a, b, c);
    at<u32>(session, 0xdc) = 1;
    return at<u8 *>(session, 0xd4);
}

u32 __fastcall Online_UnpublishPark(u8 *session, void *, u32 park) {
    if (!Online_OpenCity(session))
        return 0;
    using Unpublish = u32(__fastcall *)(u8 *, void *, u32, u32, u32);
    return (*reinterpret_cast<const Unpublish *>((uintptr_t)0x006fd4fc))(at<u8 *>(session, 0xd4), nullptr, 0, park, at<u32>(session, 0x34));
}
}

namespace {
const OnlineInstanceEnv kInstanceGameEnv = {&Online_Construct};
const OnlineInstanceEnv *g_instance = &kInstanceGameEnv;
}  // namespace

void Online_SetInstanceEnv(const OnlineInstanceEnv *env) { g_instance = env ? env : &kInstanceGameEnv; }

void __fastcall Online_ResetConnections(u8 *session) {
    Online_ResetNews(session);
    Online_ResetCity(session);
    Online_ResetUpload(session);
    Online_ResetDownload(session);
    Dialog7cca94_Message4IfUp();
}

namespace {
const OnlineStartEnv kStartGameEnv = {&Online_ReleaseNews, &Online_ResetCity, &Online_ResetUpload, &Online_ResetDownload, &Dialog7cca94_Message4IfUp, &Online_Instance, &ListRequest_Start66, &Dialog7cc530_Open, &Owner2c_FindKind100, &Online_UploadConnect, &CommandQueue_Post, &Names_FindLastValue, &CRT_free, &Class70293c_Assign, &Online_DownloadStart, &Online_DownloadEnd, &Online_DownloadNext};
const OnlineStartEnv *g_start = &kStartGameEnv;
}  // namespace

void Online_SetStartEnv(const OnlineStartEnv *env) { g_start = env ? env : &kStartGameEnv; }

void __fastcall Online_Start(u8 *session, void *, u32 a, u32 b, u32 c) {
    g_start->releaseNews(session);
    ClearNews(session);
    g_start->resetCity(session);
    g_start->resetUpload(session);
    g_start->resetDownload(session);
    g_start->message4();
    at<u32>(session, 0x3c4) = c;
    at<u32>(session, 0x3a8) = a;
    at<u32>(session, 0x3c0) = b;
    if (at<u32>(session, 0x60)) {
        u8 *const instance = g_start->instance();
        g_start->listStart(at<u8 *>(instance, 0xf0), nullptr, at<u32>(instance, 0x3a8), 0x005b88e0, 0);
    } else {
        at<u32>(session, 4) = 0x005b87a0;
        at<u32>(session, 8) = 0;
        g_start->openDialog(0x005b5cb0);
    }
}

void __fastcall Online_StartItem(u8 *session, void *, const u8 *item, u32 b, u32 c) {
    g_start->releaseNews(session);
    ClearNews(session);
    g_start->resetCity(session);
    g_start->resetUpload(session);
    g_start->resetDownload(session);
    g_start->message4();
    at<u32>(session, 0x384) = 1;
    g_start->assign(at<u8 *>(session, 0x3a4), nullptr, item);
    at<u32>(session, 0x3c4) = c;
    at<u32>(session, 0x3c0) = b;
    if (at<u32>(session, 0x60)) {
        g_start->downloadStart(g_start->instance(), nullptr, nullptr);
    } else {
        at<u32>(session, 0x388) = 1;
        at<u32>(session, 4) = 0x005b8680;
        at<u32>(session, 8) = 0;
        g_start->openDialog(0x005b5cb0);
    }
}

void __cdecl Online_StartItemLoggedIn(u32 failed) {
    u8 *const instance = g_start->instance();
    if (failed)
        g_start->downloadEnd(instance, nullptr, 1);
    else
        g_start->downloadStart(instance, nullptr, nullptr);
}

void __cdecl Online_StartLoggedIn(u32 failed) {
    u8 *const instance = g_start->instance();
    if (failed)
        g_start->downloadEnd(instance, nullptr, 1);
    else
        g_start->listStart(at<u8 *>(instance, 0xf0), nullptr, at<u32>(instance, 0x3a8), 0x005b88e0, 0);
}

void __cdecl Online_StartListAnswered(u32 error, u32, const u8 *answer) {
    const u8 *const item = at<const u8 *>(at<const u8 *>(answer, 4), 0);
    u8 *const instance = g_start->instance();
    if (!error && item)
        g_start->downloadStart(instance, nullptr, item);
    else
        g_start->downloadEnd(instance, nullptr, 1);
}

void __stdcall Online_DownloadCallback(u32 kind, u32 value) { Online_DownloadEvent(g_start->instance(), nullptr, kind, value); }

void __fastcall Online_DownloadEvent(u8 *session, void *, u32 kind, u32 value) {
    if (!at<u32>(session, 0x3a0) || kind == 0)
        return;
    if ((kind == 1 || kind == 3) && !value) {
        Say(kind == 1 ? reinterpret_cast<uintptr_t>(&gOnline_DownloadEvent_773ae0) : reinterpret_cast<uintptr_t>(&gOnline_DownloadEvent_773ab8));
        g_start->downloadNext(session);
        return;
    }
    if (value)
        at<u32>(session, 0x398) = value;
    if (kind != 2)
        at<u32>(session, 0x394) = 0;
    at<u32>(session, 0x390) = 1;
}

void __fastcall Online_ResetAll(u8 *session) {
    Online_ReleaseLogin(session);
    ClearLogin(session);
    Online_ResetConnections(session);
    Dialup_HangUpIfOpen();
}

u32 __fastcall Online_UploadConnect(u8 *session) {
    if (void *const old = at<void *>(session, 0x104)) {
        CallImport(0x006fd5ec, old);
        at<u32>(session, 0x104) = 0;
    }
    using Make = u32(__fastcall *)(void *, void *);
    void *const block = GameHeap_Alloc(0xc);
    at<u32>(session, 0x104) = block ? (*reinterpret_cast<const Make *>((uintptr_t)0x006fd5dc))(block, nullptr) : 0;
    Say(reinterpret_cast<uintptr_t>(&gOnline_UploadConnect_773a1c));
    WString wide;
    WString_CtorStr(&wide, nullptr, at<const u16 *>(g_start->findKind(at<u8 *>(session, 0xfc)), 0x2c));
    String host;
    String_Ctor(&host);
    String_AssignFromWide(&host, &wide);
    const u8 *const record = g_start->findKind(at<u8 *>(session, 0xfc));
    const u32 port = (static_cast<u32>(reinterpret_cast<uintptr_t>(record)) & 0xffff0000u) | at<u16>(record, 0x3c);  // (sic)
    using Connect = s32(__fastcall *)(void *, void *, const char *, u8 *, u32, u32);
    const s32 answer = (*reinterpret_cast<const Connect *>((uintptr_t)0x006fd5e0))(at<void *>(session, 0x104), nullptr, host.data, Global7cc53c_Pick7cc4f0(), port, 0x005b8260);
    u32 result = 0;
    if (answer == 1) {
        Say(reinterpret_cast<uintptr_t>(&gOnline_UploadConnect_7739dc));
        using Lobby = void(__fastcall *)(void *, void *, u32, u32, u32, u32);
        const u32 language = Language_Id(reinterpret_cast<const u8 *>((uintptr_t)0x0078b530));
        (*reinterpret_cast<const Lobby *>((uintptr_t)0x006fd5e4))(at<void *>(session, 0x104), nullptr, language, at<u32>(session, 0x34), at<u32>(session, 0x38), at<u32>(session, 0x28));
        result = 1;
    } else {
        Say(reinterpret_cast<uintptr_t>(&gOnline_UploadConnect_7739f8));
    }
    String_Dtor(&host);
    WString_Dtor(&wide);
    return result;
}

namespace {

// The upload given up: the code posted; with a login (+0x320) the whole session reset; the callback (+0x374) told 1; the Upload released (0x6fd5ec) when set.
void UploadGiveUp(u8 *session, u32 code) {
    at<u32>(session, 0x37c) = code;
    at<u32>(session, 0x380) = 0;
    g_start->post(code, 0);
    at<u32>(session, 0x37c) = 0;
    at<u32>(session, 0x380) = 0;
    using Callback = void(__cdecl *)(u32, u32);
    const Callback callback = at<Callback>(session, 0x374);
    const u32 user = at<u32>(session, 0x378);
    if (at<u32>(session, 0x320)) Online_ResetAll(session);
    if (callback) callback(1, user);
    if (void *const upload = at<void *>(session, 0x104)) {
        CallImport(0x006fd5ec, upload);
        at<u32>(session, 0x104) = 0;
    }
}

}  // namespace

void __fastcall Online_UploadStart(u8 *session) {
    if (!(at<u8>(session, 0x58) & 0x10)) {
        Say(reinterpret_cast<uintptr_t>(&gOnline_UploadStart_7738d0));
        UploadGiveUp(session, 0x4e);
        return;
    }
    if (!at<u32>(session, 0x104) && !g_start->uploadConnect(session)) {
        Say(reinterpret_cast<uintptr_t>(&gOnline_UploadStart_773940));
        UploadGiveUp(session, 0x4f);
        return;
    }
    Say(reinterpret_cast<uintptr_t>(&gOnline_UploadStart_773924));
    at<u32>(session, 0x324) = 1;
    u8 request[0x430];
    request[4] = 0;
    request[5] = 0;
    CRT_wcscpy(reinterpret_cast<u16 *>(request + 6), at<const u16 *>(session, 0x350));
    CRT_wcscpy(reinterpret_cast<u16 *>(request + 0x2e), at<const u16 *>(session, 0x364));
    std::memcpy(request + 0x22e, session + 0x120, 0x200);
    at<u32>(request, 0) = at<u32>(session, 0x344);
    request[4] = static_cast<u8>(g_start->findLast(at<u8 *>(session, 0xf8), nullptr, at<const char *>(session, 0x114)));
    request[5] = at<u8>(session, 0x370);
    using Send = void(__fastcall *)(void *, void *, u32, u8 *, u32);
    (*reinterpret_cast<const Send *>((uintptr_t)0x006fd5d0))(at<void *>(session, 0x104), nullptr, at<u32>(session, 0x338), request, 0);
    Say(reinterpret_cast<uintptr_t>(&gOnline_UploadStart_773900));
}

void __fastcall Online_UploadEvent(u8 *session, void *, u32 code, u32, u32, u8 *record) {
    u32 answer;
    switch (code) {
        case 0x50: {
            if (--at<u32>(record, 8) == 0) {
                if (void *const block = at<void *>(record, 0)) g_start->crtFree(block);
                at<u32>(record, 0) = 0;
                GameHeap_Free(record);
            }
            Say(reinterpret_cast<uintptr_t>(&gOnline_UploadEvent_7739a4));
            using Next = void(__fastcall *)(void *, void *, u32, u32);
            (*reinterpret_cast<const Next *>((uintptr_t)0x006fd5e8))(at<void *>(session, 0x104), nullptr, at<u32>(session, 0x338), 0);
            return;
        }
        case 0:
            Say(reinterpret_cast<uintptr_t>(&gOnline_UploadEvent_773980));
            at<u32>(session, 0x32c) = 1;
            at<u32>(session, 0x328) = 1;
            return;
        case 0xffffffd8u:
            at<u32>(session, 0x37c) = 0xde;
            at<u32>(session, 0x380) = *reinterpret_cast<const u32 *>((uintptr_t)0x0078ba94);
            at<u32>(session, 0x32c) = 0;
            at<u32>(session, 0x328) = 1;
            return;
        case 0xffffffecu: answer = 0x55; break;
        case 0xfffffffau: answer = 0x4e; break;
        case 0xfffffffbu: answer = 0x4d; break;
        case 0xfffffffcu: answer = 0x4c; break;
        case 0xfffffffeu: answer = 0x4b; break;
        case 0xffffffffu: answer = 0x37; break;
        default: return;
    }
    at<u32>(session, 0x37c) = answer;
    at<u32>(session, 0x32c) = 0;
    at<u32>(session, 0x328) = 1;
}

void __stdcall Online_UploadCallback(u32 code, u32 a, u32 b, u8 *record) { Online_UploadEvent(g_start->instance(), nullptr, code, a, b, record); }

namespace {

const OnlineCityPollEnv kCityPollGameEnv = {&ListRequest_Start10, &ListRequest_Start26, &ListRequest_Start34, &ListRequest_Start66, &ListRequest_Done10, &ListRequest_Done26, &ListRequest_Done34, &ListRequest_Done66,
                                            &ListRequest_Done30, &Time_GetMs, reinterpret_cast<decltype(OnlineCityPollEnv::showText)>(&Dialog7cca94_ShowText), &Dialog7cca94_Message4IfUp,
                                            reinterpret_cast<decltype(OnlineCityPollEnv::setProgress)>(&Dialog7cca94_SetProgress), reinterpret_cast<decltype(OnlineCityPollEnv::waitingRow)>(&Dialog7cc944_SetWaitingRow)};
const OnlineCityPollEnv *g_cityPoll = &kCityPollGameEnv;

// A list flagged is answered; its +0xc after.
u32 AnswerList(u8 *list, void(__fastcall *done)(u8 *, void *, u32)) {
    if (at<u32>(list, 0xc) && at<u32>(list, 0x10)) {
        at<u32>(list, 0xc) = 0;
        done(list, nullptr, 0);
    }
    return at<u32>(list, 0xc);
}

}  // namespace

void Online_SetCityPollEnv(const OnlineCityPollEnv *env) { g_cityPoll = env ? env : &kCityPollGameEnv; }

namespace {
const OnlineDtorEnv kDtorGameEnv = {&Online_ReleaseLogin, &Online_ResetNews, &Online_ResetCity, &Online_ResetUpload, &Online_ResetDownload, &Dialog7cca94_Message4IfUp, &Dialup_HangUpIfOpen,
                                    reinterpret_cast<decltype(OnlineDtorEnv::stringDtor)>(&String_Dtor), reinterpret_cast<decltype(OnlineDtorEnv::wstringDtor)>(&WString_Dtor)};
const OnlineDtorEnv *g_dtor = &kDtorGameEnv;
}  // namespace

void Online_SetDtorEnv(const OnlineDtorEnv *env) { g_dtor = env ? env : &kDtorGameEnv; }

void __fastcall Online_Dtor(u8 *session) {
    g_dtor->releaseLogin(session);
    ClearLogin(session);
    g_dtor->resetNews(session);
    g_dtor->resetCity(session);
    g_dtor->resetUpload(session);
    g_dtor->resetDownload(session);
    g_dtor->message4();
    g_dtor->hangUp();
    if (at<u32>(session, 0x6c)) FreeNews(session);
    for (u32 offset = 0xe0; offset <= 0x100; offset += 4)
        if (u8 *const list = at<u8 *>(session, offset)) vcall<void>(list, 0, 1u);
    Online_ReleaseUpload(session);
    DropDownload(session);
    if (u8 *const object = at<u8 *>(session, 0x3a4)) vcall<void>(object, 0, 1u);   // sic: +0x3a4 stays
    g_dtor->stringDtor(session + 0x3dc);
    u8 *node = at<u8 *>(session, 0x3cc);
    at<u32>(session, 0x3c8) = (u32)(uintptr_t)ListDtor7028a0_vtable;
    at<u32>(session, 0x3d0) = 0;
    at<u32>(session, 0x3cc) = 0;
    while (node) {
        u8 *const next = at<u8 *>(node, 4);
        GameHeap_Free(node);
        node = next;
    }
    at<u32>(session, 0x3d4) = 0;
    g_dtor->stringDtor(session + 0x3ac);
    g_dtor->wstringDtor(session + 0x35c);
    g_dtor->wstringDtor(session + 0x348);
    g_dtor->stringDtor(session + 0x330);
    g_dtor->stringDtor(session + 0x10c);
    g_dtor->wstringDtor(session + 0xb4);
    g_dtor->wstringDtor(session + 0x98);
    g_dtor->stringDtor(session + 0x7c);
    g_dtor->wstringDtor(session + 0x3c);
    g_dtor->wstringDtor(session + 0x20);
    g_dtor->wstringDtor(session + 0xc);
}

namespace {

const OnlineEndEnv kEndGameEnv = {&CommandQueue_Post, &Online_OpenCity, &Online_ReleaseNews, &Online_ResetNews, &Online_ResetCity, &Online_ResetUpload, &Online_ResetDownload, &Dialog7cca94_Message4IfUp, &Dialup_HangUpIfOpen,
                                  reinterpret_cast<decltype(OnlineEndEnv::showText)>(&Dialog7cca94_ShowText), reinterpret_cast<decltype(OnlineEndEnv::setProgress)>(&Dialog7cca94_SetProgress), &Online_Instance, &Online_UploadStart, &Dialog7cc530_Open};
const OnlineEndEnv *g_end = &kEndGameEnv;

using TransferCallback = void(__cdecl *)(u32 result, u32 user);

// The upload's posting: +0x37c and +0x380 posted and cleared when either is set; whether it did.
bool PostUpload(u8 *session) {
    if (!at<u32>(session, 0x37c) && !at<u32>(session, 0x380)) return false;
    g_end->post(at<u32>(session, 0x37c), at<u32>(session, 0x380));
    at<u32>(session, 0x37c) = 0;
    at<u32>(session, 0x380) = 0;
    return true;
}

}  // namespace

void Online_SetEndEnv(const OnlineEndEnv *env) { g_end = env ? env : &kEndGameEnv; }

void __fastcall Online_DownloadEnd(u8 *session, void *, u32 failed) {
    bool posted = false;
    if (at<u32>(session, 0x398)) {
        g_end->post(at<u32>(session, 0x398), 0);
        at<u32>(session, 0x398) = 0;
        posted = true;
    }
    const TransferCallback callback = at<TransferCallback>(session, 0x3c0);
    const u32 user = at<u32>(session, 0x3c4);
    if (!failed && !posted) {
        const u32 size = at<u32>(at<u8 *>(session, 0x3a4), 8);
        if (g_end->openCity(session)) {
            using Start = void(__fastcall *)(void *, void *, u32, u32, u32);
            (*reinterpret_cast<const Start *>((uintptr_t)0x006fd52c))(at<void *>(session, 0xd4), nullptr, 0, size, at<u32>(session, 0x34));
        }
        at<u32>(session, 0x3f0) = 1;
    } else if (at<u32>(session, 0x388)) {
        Online_ReleaseLogin(session);
        ClearLogin(session);
        g_end->releaseNews(session);
        ClearNews(session);
        g_end->resetCity(session);
        g_end->resetUpload(session);
        g_end->resetDownload(session);
        g_end->message4();
        g_end->hangUp();
    }
    if (callback) callback(failed, user);
    DropDownload(session);
    at<u32>(session, 0x384) = 0;
}

void __fastcall Online_UploadPoll(u8 *session) {
    if (!at<u32>(session, 0x104) || !at<u32>(session, 0x324)) return;
    if (!at<u32>(session, 0x328)) {
        using Current = u32(__fastcall *)(void *, void *, u32);
        const Current current = *reinterpret_cast<const Current *>((uintptr_t)0x006fd5d4);
        u32 id = current(at<void *>(session, 0x104), nullptr, 0);
        if (!id) id = current(at<void *>(session, 0x104), nullptr, 1);
        g_end->showText(0x101);
        s32 done = 0, total = 0;
        using Progress = void(__fastcall *)(void *, void *, u32, s32 *, s32 *);
        (*reinterpret_cast<const Progress *>((uintptr_t)0x006fd5d8))(at<void *>(session, 0x104), nullptr, id, &done, &total);
        if (total > 0) g_end->setProgress(static_cast<s32>(static_cast<u32>(done) * 100u) / total);
        return;
    }
    g_end->message4();
    const u32 succeeded = at<u32>(session, 0x32c) == 0;
    at<u32>(session, 0x324) = 0;
    const bool posted = PostUpload(session);
    const TransferCallback callback = at<TransferCallback>(session, 0x374);
    const u32 user = at<u32>(session, 0x378);
    if ((succeeded || posted) && at<u32>(session, 0x320)) {
        Online_ReleaseLogin(session);
        ClearLogin(session);
        g_end->resetNews(session);
        g_end->resetCity(session);
        g_end->resetUpload(session);
        g_end->resetDownload(session);
        g_end->message4();
        g_end->hangUp();
    }
    if (callback) callback(succeeded, user);
    Online_ReleaseUpload(session);
}

void __cdecl Online_UploadLoginDone(u32 failed) {
    u8 *const session = g_end->instance();
    if (!failed) {
        g_end->uploadStart(g_end->instance());
        return;
    }
    PostUpload(session);
    const TransferCallback callback = at<TransferCallback>(session, 0x374);
    const u32 user = at<u32>(session, 0x378);
    if (at<u32>(session, 0x320)) {
        Online_ReleaseLogin(session);
        ClearLogin(session);
        g_end->resetNews(session);
        g_end->resetCity(session);
        Online_ResetUpload(session);   // the game has it inline
        g_end->resetDownload(session);
        g_end->message4();
        g_end->hangUp();
    }
    if (callback) callback(1, user);
    Online_ReleaseUpload(session);
}

void __fastcall Online_UploadBegin(u8 *session, void *, const void *a, u32 b, const void *c, const void *d, u32 e, u32 f, u32 g) {
    Online_ResetUpload(session);   // the game has it inline
    vcall<void>(session + 0x330, 0x14, a);
    at<u32>(session, 0x344) = b;
    vcall<void>(session + 0x348, 0x14, c);
    vcall<void>(session + 0x35c, 0x14, d);
    at<u32>(session, 0x370) = e;
    at<u32>(session, 0x374) = f;
    at<u32>(session, 0x378) = g;
    if (at<u32>(session, 0x60)) {
        g_end->instance();   // sic: asked twice
        g_end->uploadStart(g_end->instance());
        return;
    }
    at<u32>(session, 0x320) = 1;
    at<u32>(session, 4) = 0x005b79f0;
    at<u32>(session, 8) = 0;
    g_end->openDialog(0x005b5cb0);
}

namespace {
const OnlinePollEnv kPollGameEnv = {&Online_LoginFinish, &Online_NewsPoll, &Online_CityPoll, &Online_UploadPoll, &Online_DownloadEnd, &Dialog7cca94_Message4IfUp,
                                    reinterpret_cast<decltype(OnlinePollEnv::showText)>(&Dialog7cca94_ShowText), reinterpret_cast<decltype(OnlinePollEnv::setProgress)>(&Dialog7cca94_SetProgress)};
const OnlinePollEnv *g_poll = &kPollGameEnv;
}  // namespace

void Online_SetPollEnv(const OnlinePollEnv *env) { g_poll = env ? env : &kPollGameEnv; }

void __fastcall Online_Poll(u8 *session) {
    if (at<u32>(session, 0) && at<u32>(session, 0x64)) {
        g_poll->loginFinish(session, nullptr, 0);
        at<u32>(session, 0x64) = 0;
    }
    g_poll->newsPoll(session);
    g_poll->cityPoll(session);
    g_poll->uploadPoll(session);
    if (!at<u32>(session, 0x39c) || !at<u32>(session, 0x3a0) || !at<u32>(session, 0x38c)) return;
    if (at<u32>(session, 0x390)) {
        g_poll->message4();
        const u32 failed = at<u32>(session, 0x394) == 0;
        at<u32>(session, 0x38c) = 0;
        g_poll->downloadEnd(session, nullptr, failed);
        return;
    }
    g_poll->showText(0x102);
    const u8 *const transfer = at<u8 *>(session, 0x3a0);
    const s32 total = at<s32>(transfer, 0x18);
    if (total > 0) g_poll->setProgress(static_cast<s32>(at<u32>(transfer, 0x1c) * 100u) / total);
}

namespace {

const OnlineNextEnv kNextGameEnv = {&Owner2c_FindKind1, &Owner2c_FindByKey, reinterpret_cast<decltype(OnlineNextEnv::fetch)>((uintptr_t)0x005d22a0), reinterpret_cast<decltype(OnlineNextEnv::finish)>((uintptr_t)0x005d2180)};
const OnlineNextEnv *g_next = &kNextGameEnv;

// The kind 1 record's address, then "/": into a new wide string.
void ServerFolder(u8 *session, WString *url) {
    WString_Ctor(url);
    vcall<void>(url, 0x14, at<u32>(g_next->findKind1(at<u8 *>(session, 0xfc)), 0x74));
    const u32 slash = 0x2f;
    vcall<void>(url, 0x24, &slash);
}

}  // namespace

void Online_SetNextEnv(const OnlineNextEnv *env) { g_next = env ? env : &kNextGameEnv; }

namespace {
const OnlineDownloadStartEnv kDownloadStartGameEnv = {&Class70293c_Assign, &Owner2c_FindByPair, reinterpret_cast<void(__cdecl *)(const u16 *, u16 *, u16 *, u16 *, u16 *)>((uintptr_t)0x0067dda0),
                                                      &UserSlots_Ctor, &GamePaths_OnlineFolder, &File_Exists, &FtpClient_Ctor, &Ftp_Connect, &Online_DownloadEnd, &Online_OpenCity, &Owner2c_FindKind1,
                                                      &Global7cc53c_Pick7cc4f0};
const OnlineDownloadStartEnv *g_fetch = &kDownloadStartGameEnv;

// The queue's front dropped, its string deleted.
void DropQueued(u8 *session) {
    u8 *object = nullptr;
    if (u8 *const node = at<u8 *>(session, 0x3cc)) {
        u8 *const next = at<u8 *>(node, 4);
        at<u8 *>(session, 0x3cc) = next;
        if (next)
            at<u32>(next, 8) = 0;
        else
            at<u32>(session, 0x3d0) = 0;
        at<u32>(session, 0x3d4) -= 1;
        object = at<u8 *>(node, 0);
        GameHeap_Free(node);
    }
    if (object) vcall<void>(object, 0xc, 1u);
}

void QueueBack(u8 *session, void *object) {
    u8 *const node = static_cast<u8 *>(GameHeap_Alloc(0xc));
    if (node) {
        at<void *>(node, 0) = object;
        at<u32>(node, 4) = 0;
        at<u32>(node, 8) = 0;
    }
    u8 *const tail = at<u8 *>(session, 0x3d0);
    at<u8 *>(node, 8) = tail;
    if (tail)
        at<u8 *>(tail, 4) = node;
    else
        at<u8 *>(session, 0x3cc) = node;
    at<u8 *>(session, 0x3d0) = node;
    at<u32>(session, 0x3d4) += 1;
}

// One bit of the item's: its file queued unless it is there.
void QueueMissing(u8 *session, u32 number) {
    WString path;
    u8 *const found = g_fetch->findByPair(at<u8 *>(session, 0x100), nullptr, at<u32>(at<u8 *>(session, 0x3a4), 0x4c), number);
    Obj_NameToWString(found, nullptr, reinterpret_cast<u8 *>(&path));
    u16 extension[0x10], drive[0x10], name[0x100], folder[0x100];
    g_fetch->splitPath(path.data, drive, folder, name, extension);
    String root;
    String_CtorCopy(&root, nullptr, reinterpret_cast<const StringView *>((uintptr_t)0x00f816c0));
    String_EnsureTrailingBackslash(&root);
    vcall<void>(&root, 0x28, 0x00f7ccd8u);
    String_EnsureTrailingBackslash(&root);
    WString plain;
    WString_Ctor(&plain);
    WString_AssignFromAnsi(&plain, &root);
    vcall<void>(&plain, 0x20, folder);
    WString prefixed;
    WString_CtorCopy(&prefixed, nullptr, &plain);
    vcall<void>(&plain, 0x20, name);
    vcall<void>(&plain, 0x20, extension);
    vcall<void>(&prefixed, 0x20, reinterpret_cast<uintptr_t>(&gSession_773ab4));
    vcall<void>(&prefixed, 0x20, name);
    vcall<void>(&prefixed, 0x20, extension);
    String plainText;
    String_Ctor(&plainText);
    String_AssignFromWide(&plainText, &plain);
    String prefixedText;
    String_Ctor(&prefixedText);
    String_AssignFromWide(&prefixedText, &prefixed);
    if (!g_fetch->exists(plainText.data) && !g_fetch->exists(prefixedText.data)) {
        void *const memory = GameHeap_Alloc(0x14);
        WString *const entry = memory ? WString_Ctor(static_cast<WString *>(memory)) : nullptr;
        vcall<void>(entry, 0x14, folder);
        vcall<void>(entry, 0x20, reinterpret_cast<uintptr_t>(&gSession_773ab4));
        vcall<void>(entry, 0x20, name);
        vcall<void>(entry, 0x20, extension);
        QueueBack(session, entry);
    }
    String_Dtor(&prefixedText);
    String_Dtor(&plainText);
    WString_Dtor(&prefixed);
    WString_Dtor(&plain);
    String_Dtor(&root);
    WString_Dtor(&path);
}

// The client made and connected: the server of the item of kind 1, the fixed user at 0x773a80, the password at +0x44.
void ConnectClient(u8 *session) {
    u32 *const listener = static_cast<u32 *>(GameHeap_Alloc(4));
    if (listener) *listener = (u32)(uintptr_t)Class7028a4_vtable;
    at<u32 *>(session, 0x39c) = listener;
    void *const memory = GameHeap_Alloc(0x2780);
    at<u32 *>(session, 0x3a0) = memory ? g_fetch->ftpCtor(static_cast<u32 *>(memory), nullptr, at<void *>(session, 0x39c), 1) : nullptr;
    Say(reinterpret_cast<uintptr_t>(&gSession_773a8c));
    const u8 *const kind = g_fetch->findKind1(at<u8 *>(session, 0xfc));
    WString wideHost;
    WString_CtorStr(&wideHost, nullptr, at<const u16 *>(kind, 0x2c));
    String host;
    String_Ctor(&host);
    String_AssignFromWide(&host, &wideHost);
    WString widePassword;
    WString_CtorStr(&widePassword, nullptr, at<const u16 *>(session, 0x44));
    String password;
    String_Ctor(&password);
    String_AssignFromWide(&password, &widePassword);
    at<u32>(session, 0x38c) = 1;
    const char *const bind = reinterpret_cast<const char *>(g_fetch->pick());
    g_fetch->ftpConnect(at<void *>(session, 0x3a0), nullptr, host.data, gSession_773a80, password.data, bind);
    String_Dtor(&password);
    WString_Dtor(&widePassword);
    String_Dtor(&host);
    WString_Dtor(&wideHost);
}
}  // namespace

void Online_SetDownloadStartEnv(const OnlineDownloadStartEnv *env) { g_fetch = env ? env : &kDownloadStartGameEnv; }

void __fastcall Online_DownloadStart(u8 *session, void *, const u8 *item) {
    if (item) g_fetch->assign(at<u8 *>(session, 0x3a4), nullptr, item);
    Say(reinterpret_cast<uintptr_t>(&gSession_7733d0));
    if (void *const client = at<void *>(session, 0x3a0)) {
        at<u32>(session, 0x3a0) = 0;
        vcall<void>(client, 0, 1u);
    }
    if (void *const listener = at<void *>(session, 0x39c)) {
        GameHeap_Free(listener);
        at<u32>(session, 0x39c) = 0;
    }
    const u8 flags = at<u8>(session, 0x58);
    if (!(at<u32>(at<u8 *>(session, 0x3a4), 0x78) ? flags & 4 : flags & 8)) {
        Say(reinterpret_cast<uintptr_t>(&gOnline_DownloadStart_773a40));
        at<u32>(session, 0x398) = 0x4e;
        g_fetch->downloadEnd(session, nullptr, 1);
        return;
    }
    while (at<u32>(session, 0x3d4))
        DropQueued(session);
    const u8 *const bits = at<u8 *>(session, 0x3a4) + 0x58;
    for (u32 byte = 0; byte < 0x20; byte++) {
        for (u32 bit = 0; bit < 8; bit++) {
            if (bits[byte] & 1 << bit) QueueMissing(session, bit - 8 * byte + 0xf9);
        }
    }
    if (!at<u32>(0, reinterpret_cast<uintptr_t>(&gChatlists_f81ebc))) {
        void *const memory = GameHeap_Alloc(0x110);
        at<u8 *>(0, reinterpret_cast<uintptr_t>(&gChatlists_f81ebc)) = memory ? g_fetch->slotsCtor(static_cast<u8 *>(memory)) : nullptr;
    }
    WString folder;
    g_fetch->folder(&folder);
    vcall<void>(&folder, 0x28, 0x00f7d740u);
    String_AssignFromWide(reinterpret_cast<String *>(session + 0x3dc), &folder);
    vcall<void>(session + 0x3dc, 0x20, at<u32>(at<u8 *>(session, 0x3a4), 0x84));
    vcall<void>(session + 0x3dc, 0x28, 0x00f7d1e8u);
    at<u32>(session, 0x3d8) = g_fetch->exists(at<const char *>(session, 0x3e4)) ? 0 : 1;
    if (!at<u32>(session, 0x3d8) && !at<u32>(session, 0x3d4)) {
        if (const TransferCallback callback = at<TransferCallback>(session, 0x3c0)) callback(0, at<u32>(session, 0x3c4));
        const u32 size = at<u32>(at<u8 *>(session, 0x3a4), 8);
        if (g_fetch->openCity(session)) {
            using Start = void(__fastcall *)(void *, void *, u32, u32, u32);
            (*reinterpret_cast<const Start *>((uintptr_t)0x006fd52c))(at<void *>(session, 0xd4), nullptr, 0, size, at<u32>(session, 0x34));
        }
        at<u32>(session, 0x3f0) = 1;
    } else {
        ConnectClient(session);
    }
    WString_Dtor(&folder);
}

void __fastcall Online_DownloadNext(u8 *session) {
    if (at<u32>(session, 0x3d4)) {
        Say(reinterpret_cast<uintptr_t>(&gOnline_DownloadNext_773b64));
        u8 *item = nullptr;
        if (u8 *const node = at<u8 *>(session, 0x3cc)) {
            u8 *const next = at<u8 *>(node, 4);
            at<u8 *>(session, 0x3cc) = next;
            if (next)
                at<u32>(next, 8) = 0;
            else
                at<u32>(session, 0x3d0) = 0;
            at<u32>(session, 0x3d4) -= 1;
            item = at<u8 *>(node, 0);
            GameHeap_Free(node);
        }
        WString url;
        ServerFolder(session, &url);
        vcall<void>(&url, 0x28, item);
        String from;
        String_Ctor(&from);
        String_AssignFromWide(&from, &url);
        String to;
        String_CtorCopy(&to, nullptr, reinterpret_cast<const StringView *>((uintptr_t)0x00f816c0));
        String_EnsureTrailingBackslash(&to);
        vcall<void>(&to, 0x28, 0x00f7ccd8u);
        String_EnsureTrailingBackslash(&to);
        WString wide;
        WString_Ctor(&wide);
        WString_AssignFromAnsi(&wide, &to);
        vcall<void>(&wide, 0x28, item);
        String_AssignFromWide(&to, &wide);
        g_next->fetch(at<void *>(session, 0x3a0), nullptr, from.data, to.data);
        if (item) vcall<void>(item, 0xc, 1u);
        WString_Dtor(&wide);
        String_Dtor(&to);
        String_Dtor(&from);
        WString_Dtor(&url);
        return;
    }
    if (at<u32>(session, 0x3d8)) {
        Say(reinterpret_cast<uintptr_t>(&gOnline_DownloadNext_773b3c));
        WString url;
        ServerFolder(session, &url);
        const u8 *const key = g_next->findByKey(at<u8 *>(session, 0xe0), nullptr, at<u32>(at<u8 *>(session, 0x3a4), 0x48));
        vcall<void>(&url, 0x20, at<u32>(key, 0x4c));
        const u32 slash = 0x2f;
        vcall<void>(&url, 0x24, &slash);
        String from;
        String_Ctor(&from);
        String_AssignFromWide(&from, &url);
        vcall<void>(&from, 0x20, at<u32>(at<u8 *>(session, 0x3a4), 0x84));
        g_next->fetch(at<void *>(session, 0x3a0), nullptr, from.data, at<const char *>(session, 0x3e4));
        at<u32>(session, 0x3d8) = 0;
        String_Dtor(&from);
        WString_Dtor(&url);
        return;
    }
    Say(reinterpret_cast<uintptr_t>(&gOnline_DownloadNext_773b00));
    g_next->finish(at<void *>(session, 0x3a0));
    at<u32>(session, 0x394) = 1;
}

void __fastcall Online_CityPoll(u8 *session) {
    if (!at<u32>(session, 0xd4)) return;
    if (at<u32>(session, 0xdc)) {
        g_cityPoll->start10(at<u8 *>(session, 0xf8));
        g_cityPoll->start26(at<u8 *>(session, 0xfc));
        g_cityPoll->start34(at<u8 *>(session, 0x100));
        g_cityPoll->start66(at<u8 *>(session, 0xf4), nullptr, 1, 0, 0);
        at<u32>(session, 0xdc) = 0;
    }
    u32 busy = AnswerList(at<u8 *>(session, 0xf8), g_cityPoll->done10);
    busy |= AnswerList(at<u8 *>(session, 0xfc), g_cityPoll->done26);
    busy |= AnswerList(at<u8 *>(session, 0x100), g_cityPoll->done34);
    busy |= AnswerList(at<u8 *>(session, 0xf4), g_cityPoll->done66);
    busy |= AnswerList(at<u8 *>(session, 0xe0), g_cityPoll->done30);
    for (const u32 offset : {0xe4u, 0xe8u, 0xecu, 0xf0u})
        busy |= AnswerList(at<u8 *>(session, offset), g_cityPoll->done66);
    if (!busy) {
        g_cityPoll->message4();
        if (static_cast<s32>(g_cityPoll->now() - at<u32>(session, 0xd8)) <= 0x57e40) return;
        if (!at<u32>(session, 0xd4)) return;
        CallImport(0x006fd4f4, at<void *>(session, 0xd4));
        if (void *const city = at<void *>(session, 0xd4)) {
            CallImport(0x006fd540, city);
            GameHeap_Free(city);
        }
        at<u32>(session, 0xd4) = 0;
        Say(reinterpret_cast<uintptr_t>(&gSession_77386c));
        return;
    }
    at<u32>(session, 0xd8) = g_cityPoll->now();
    using Current = u32(__fastcall *)(void *, void *, u32);
    const Current current = *reinterpret_cast<const Current *>((uintptr_t)0x006fd4f8);
    u32 id = current(at<void *>(session, 0xd4), nullptr, 0);
    if (!id) id = current(at<void *>(session, 0xd4), nullptr, 1);
    const struct { u32 offset, text; } kTexts[] = {{0xe0, 0xf9}, {0xe4, 0xfa}, {0xe8, 0xfb}, {0xec, 0xfc}, {0xf4, 0xfd}, {0xf8, 0xfe}, {0xfc, 0xff}, {0x100, 0x100}};
    bool shown = false;
    for (const auto &t : kTexts) {
        if (at<u32>(at<u8 *>(session, t.offset), 4) == id) {
            g_cityPoll->showText(t.text);
            shown = true;
            break;
        }
    }
    if (!shown) g_cityPoll->message4();
    s32 done, total;
    using Progress = void(__fastcall *)(void *, void *, u32, s32 *, s32 *);
    (*reinterpret_cast<const Progress *>((uintptr_t)0x006fd4e0))(at<void *>(session, 0xd4), nullptr, id, &done, &total);
    if (total <= 0) return;
    const s32 percent = static_cast<s32>(static_cast<u32>(done) * 100u) / total;
    g_cityPoll->setProgress(percent);
    if (at<u32>(at<u8 *>(session, 0xec), 4) == id) g_cityPoll->waitingRow(percent);
}

namespace {

// A city list's block: its base (Class702890_Ctor), the vtable, +0x34 = -1; none when the block is not had.
u8 *NewList(u32 size, u32 table) {
    u8 *const list = static_cast<u8 *>(GameHeap_Alloc(size));
    if (!list) return nullptr;
    Class702890_Ctor(list);
    at<u32>(list, 0) = table;
    at<u32>(list, 0x34) = 0xffffffff;
    return list;
}

template <typename Ctor> u8 *NewObject(u32 size, Ctor ctor) {
    u8 *const object = static_cast<u8 *>(GameHeap_Alloc(size));
    return object ? ctor(object) : nullptr;
}

}  // namespace

u8 *__fastcall Online_Construct(u8 *session) {
    for (u32 offset : {0xcu, 0x20u, 0x3cu})
        WString_Ctor(reinterpret_cast<WString *>(session + offset));
    String_Ctor(reinterpret_cast<String *>(session + 0x7c));
    WString_Ctor(reinterpret_cast<WString *>(session + 0x98));
    WString_Ctor(reinterpret_cast<WString *>(session + 0xb4));
    String_Ctor(reinterpret_cast<String *>(session + 0x10c));
    String_Ctor(reinterpret_cast<String *>(session + 0x330));
    WString_Ctor(reinterpret_cast<WString *>(session + 0x348));
    WString_Ctor(reinterpret_cast<WString *>(session + 0x35c));
    String_Ctor(reinterpret_cast<String *>(session + 0x3ac));
    at<u32>(session, 0x3cc) = 0;
    at<u32>(session, 0x3d0) = 0;
    at<u32>(session, 0x3d4) = 0;
    at<u32>(session, 0x3c8) = ((u32)(uintptr_t)Class702868_vtable + 0x10);
    String_Ctor(reinterpret_cast<String *>(session + 0x3dc));
    at<u32>(session, 0) = 0;
    ClearLogin(session);
    at<u32>(session, 0x6c) = 0;
    at<u32>(session, 0x70) = 0;
    Online_ResetNews(session);
    at<u32>(session, 0xd4) = 0;
    u8 *cities = static_cast<u8 *>(GameHeap_Alloc(0x34));
    if (cities) {
        Class702880_Ctor(cities);
        at<u32>(cities, 0) = (u32)(uintptr_t)Class702868_vtable;
    }
    at<u8 *>(session, 0xe0) = cities;
    at<u8 *>(session, 0xe4) = NewList(0x38, (u32)(uintptr_t)Class702858_vtable);
    at<u8 *>(session, 0xe8) = NewList(0x38, (u32)(uintptr_t)Class702848_vtable);
    u8 *const named = static_cast<u8 *>(GameHeap_Alloc(0x48));
    if (named) {
        Class702890_Ctor(named);
        at<u32>(named, 0) = (u32)(uintptr_t)Class702838_vtable;
        WString_Ctor(reinterpret_cast<WString *>(named + 0x34));
        at<u32>(named, 0) = (u32)(uintptr_t)Class702828_vtable;
        vcall<void>(named + 0x34, 0x14, reinterpret_cast<uintptr_t>(&gWindow7cc8c0handler_78a374));
    }
    at<u8 *>(session, 0xec) = named;
    at<u8 *>(session, 0xf0) = NewList(0x38, (u32)(uintptr_t)Class702818_vtable);
    at<u8 *>(session, 0xf4) = NewList(0x38, (u32)(uintptr_t)Class702848_vtable);
    at<u8 *>(session, 0xf8) = NewObject(0x34, [](u8 *object) { return Class7028b8_Ctor(object); });
    at<u8 *>(session, 0xfc) = NewObject(0x34, [](u8 *object) { return Class7028e0_Ctor(object); });
    at<u8 *>(session, 0x100) = NewObject(0x34, [](u8 *object) { return Class702908_Ctor(object); });
    Online_ResetCity(session);
    at<u32>(session, 0x104) = 0;
    at<u32>(session, 0x108) = 0;
    for (u32 offset : {0x328u, 0x320u, 0x324u, 0x32cu, 0x344u})
        at<u32>(session, offset) = 0;
    vcall<void>(session + 0x348, 0x14, reinterpret_cast<uintptr_t>(&gWindow7cc8c0handler_78a374));
    vcall<void>(session + 0x35c, 0x14, reinterpret_cast<uintptr_t>(&gWindow7cc8c0handler_78a374));
    for (u32 offset = 0x370; offset <= 0x380; offset += 4)
        at<u32>(session, offset) = 0;
    at<u32>(session, 0x39c) = 0;
    at<u32>(session, 0x3a0) = 0;
    at<u8 *>(session, 0x3a4) = NewObject(0xc0, [](u8 *object) { return Class70293c_Ctor(object); });
    Online_ResetDownload(session);
    return session;
}

u8 *__cdecl Online_Instance() {
    u8 *&session = gChatcommand_f7d88c;
    if (session) return session;
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x404));
    session = memory ? g_instance->ctor(memory) : nullptr;
    return session;
}

namespace {

// The answer's kinds of native list: 0xe0: cities, 0xe4-0xf4: parks, 0xf8: themes, 0xfc: servers, 0x100: rides; the imports' slots.
constexpr u32 kCityRelease = 0x006fd538, kCityDtor = 0x006fd4ec, kParkRelease = 0x006fd4e8, kParkDtor = 0x006fd4f0, kThemeRelease = 0x006fd528, kThemeDtor = 0x006fd524,
              kServerRelease = 0x006fd520, kServerDtor = 0x006fd51c, kRideRelease = 0x006fd518, kRideDtor = 0x006fd514;

void HandList(u8 *list, u8 *data, u32 release) {
    if (at<u8 *>(list, 0x30)) {
        CallImport(release, at<u8 *>(list, 0x30));
        at<u8 *>(list, 0x30) = nullptr;
    }
    at<u32>(list, 0x14) = 1;
    at<u8 *>(list, 0x30) = data;
    at<u32>(list, 0x10) = 1;
}

void DropData(u8 *data, u32 dtor) {
    if (!data)
        return;
    if (--at<s32>(data, 0xc))
        return;
    CallImport(dtor, data);
    GameHeap_Free(data);
}

// One list's answer: to the list at offset when it asked; returns whether it did.
bool OfferList(u8 *session, u32 offset, s32 request, u8 *data, u32 text, u32 release) {
    if (at<s32>(at<u8 *>(session, offset), 4) != request)
        return false;
    Say(text);
    HandList(at<u8 *>(session, offset), data, release);
    return true;
}

}  // namespace

extern "C" void __fastcall Online_ListAnswered(u8 *session, void *, s32 code, s32 request, u32, u8 *data) {
    bool failed = false;
    u32 error = 0;
    switch (code) {
    case -10: failed = true; error = 0x56; break;
    case -6: failed = true; error = 0x4e; break;
    case -5: failed = true; error = 0x4d; break;
    case -4: failed = true; error = 0x4c; break;
    case -2: failed = true; error = 0x4b; break;
    case -1: failed = true; error = 0x37; break;
    case 2:
        if (!OfferList(session, 0xe0, request, data, reinterpret_cast<uintptr_t>(&gOnline_ListAnswered_77382c), kCityRelease)) {
            Say(reinterpret_cast<uintptr_t>(&gOnline_ListAnswered_7737fc));
            DropData(data, kCityDtor);
        }
        break;
    case 3:
        if (!OfferList(session, 0xe4, request, data, reinterpret_cast<uintptr_t>(&gOnline_ListAnswered_7737cc), kParkRelease) && !OfferList(session, 0xe8, request, data, reinterpret_cast<uintptr_t>(&gOnline_ListAnswered_7737a0), kParkRelease) &&
            !OfferList(session, 0xec, request, data, reinterpret_cast<uintptr_t>(&gOnline_ListAnswered_773770), kParkRelease) && !OfferList(session, 0xf0, request, data, reinterpret_cast<uintptr_t>(&gOnline_ListAnswered_773740), kParkRelease) &&
            !OfferList(session, 0xf4, request, data, reinterpret_cast<uintptr_t>(&gOnline_ListAnswered_773708), kParkRelease)) {
            Say(reinterpret_cast<uintptr_t>(&gOnline_ListAnswered_7736c8));
            DropData(data, kParkDtor);
        }
        break;
    case 4:
        if (!OfferList(session, 0xfc, request, data, reinterpret_cast<uintptr_t>(&gOnline_ListAnswered_773644), kServerRelease)) {
            Say(reinterpret_cast<uintptr_t>(&gOnline_ListAnswered_773610));
            DropData(data, kServerDtor);
        }
        break;
    case 5:
        if (!OfferList(session, 0xf8, request, data, reinterpret_cast<uintptr_t>(&gOnline_ListAnswered_7736a0), kThemeRelease)) {
            Say(reinterpret_cast<uintptr_t>(&gOnline_ListAnswered_77366c));
            DropData(data, kThemeDtor);
        }
        break;
    case 6:
        if (!OfferList(session, 0x100, request, data, reinterpret_cast<uintptr_t>(&gOnline_ListAnswered_7735e8), kRideRelease)) {
            Say(reinterpret_cast<uintptr_t>(&gOnline_ListAnswered_7735b8));
            DropData(data, kRideDtor);
        }
        break;
    case 7:
        Say(reinterpret_cast<uintptr_t>(&gOnline_ListAnswered_7735a0));
        failed = true;
        break;
    case 8:
        Say(reinterpret_cast<uintptr_t>(&gOnline_ListAnswered_773584));
        break;
    default:
        break;
    }
    if (!failed)
        return;
    for (u32 offset = 0xe0; offset <= 0x100; offset += 4) {
        u8 *const list = at<u8 *>(session, offset);
        if (at<s32>(list, 4) == request) {
            at<u32>(list, 0x18) = error;
            at<u32>(list, 0x14) = 0;
            at<u32>(list, 0x10) = 1;
            return;
        }
    }
}
