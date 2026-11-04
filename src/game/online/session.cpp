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
#include "../ui/dialog7cc530open.h"
#include "../ui/dialogs.h"
#include "../ui/dialuphangup.h"
#include "itemlist.h"
#include "items.h"
#include "listrequest.h"

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
        Say(0x0077386c);
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
    Say(0x007738b0);
    const u32 name = *reinterpret_cast<const u32 *>((uintptr_t)0x007a1524);
    const u32 server = name ? name : 0x006fe5b0;
    const u32 port = (name & 0xffff0000) | *reinterpret_cast<const u16 *>((uintptr_t)0x007a1556);   // (mov ax: the high half left)
    using Connect = u32(__fastcall *)(u8 *, void *, u32, u8 *, u32, u32);
    const u8 *const picked = Global7cc53c_Pick7cc4f0();
    if ((*reinterpret_cast<const Connect *>((uintptr_t)0x006fd534))(at<u8 *>(session, 0xd4), nullptr, server, const_cast<u8 *>(picked), port, 0x005b72c0) != 1) {
        Say(0x00773890);
        ReleaseCity(session);
        return at<u8 *>(session, 0xd4);
    }
    Say(0x00773854);
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
const OnlineStartEnv kStartGameEnv = {&Online_ReleaseNews, &Online_ResetCity, &Online_ResetUpload, &Online_ResetDownload, &Dialog7cca94_Message4IfUp, &Online_Instance, &ListRequest_Start66, &Dialog7cc530_Open, &Owner2c_FindKind100, &Online_UploadConnect, &CommandQueue_Post, &Names_FindLastValue, &CRT_free};
const OnlineStartEnv *g_start = &kStartGameEnv;
}  // namespace

void Online_SetStartEnv(const OnlineStartEnv *env) { g_start = env ? env : &kStartGameEnv; }

void __fastcall Online_Start(u8 *session, void *, u32 a, u32 b, u32 c) {
    g_start->releaseNews(session);
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

void __fastcall Online_ResetAll(u8 *session) {
    Online_ReleaseLogin(session);
    for (u32 offset : {0x64u, 0x60u, 0x68u, 0x4u, 0x8u})
        at<u32>(session, offset) = 0;
    vcall<void>(session + 0xc, 0x14, 0x0078a374u);
    vcall<void>(session + 0x20, 0x14, 0x0078a374u);
    at<u32>(session, 0x34) = 0;
    at<u32>(session, 0x38) = 0;
    vcall<void>(session + 0x3c, 0x14, 0x0078a374u);
    at<u32>(session, 0x50) = 0;
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
    Say(0x00773a1c);
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
        Say(0x007739dc);
        using Lobby = void(__fastcall *)(void *, void *, u32, u32, u32, u32);
        const u32 language = Language_Id(reinterpret_cast<const u8 *>((uintptr_t)0x0078b530));
        (*reinterpret_cast<const Lobby *>((uintptr_t)0x006fd5e4))(at<void *>(session, 0x104), nullptr, language, at<u32>(session, 0x34), at<u32>(session, 0x38), at<u32>(session, 0x28));
        result = 1;
    } else {
        Say(0x007739f8);
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
        Say(0x007738d0);
        UploadGiveUp(session, 0x4e);
        return;
    }
    if (!at<u32>(session, 0x104) && !g_start->uploadConnect(session)) {
        Say(0x00773940);
        UploadGiveUp(session, 0x4f);
        return;
    }
    Say(0x00773924);
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
    Say(0x00773900);
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
            Say(0x007739a4);
            using Next = void(__fastcall *)(void *, void *, u32, u32);
            (*reinterpret_cast<const Next *>((uintptr_t)0x006fd5e8))(at<void *>(session, 0x104), nullptr, at<u32>(session, 0x338), 0);
            return;
        }
        case 0:
            Say(0x00773980);
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
    at<u32>(session, 0x3c8) = 0x00702878;
    String_Ctor(reinterpret_cast<String *>(session + 0x3dc));
    for (u32 offset : {0x0u, 0x64u, 0x60u, 0x68u, 0x4u, 0x8u})
        at<u32>(session, offset) = 0;
    vcall<void>(session + 0xc, 0x14, 0x0078a374u);
    vcall<void>(session + 0x20, 0x14, 0x0078a374u);
    at<u32>(session, 0x34) = 0;
    at<u32>(session, 0x38) = 0;
    vcall<void>(session + 0x3c, 0x14, 0x0078a374u);
    at<u32>(session, 0x50) = 0;
    at<u32>(session, 0x6c) = 0;
    at<u32>(session, 0x70) = 0;
    Online_ResetNews(session);
    at<u32>(session, 0xd4) = 0;
    u8 *cities = static_cast<u8 *>(GameHeap_Alloc(0x34));
    if (cities) {
        Class702880_Ctor(cities);
        at<u32>(cities, 0) = 0x00702868;
    }
    at<u8 *>(session, 0xe0) = cities;
    at<u8 *>(session, 0xe4) = NewList(0x38, 0x00702858);
    at<u8 *>(session, 0xe8) = NewList(0x38, 0x00702848);
    u8 *const named = static_cast<u8 *>(GameHeap_Alloc(0x48));
    if (named) {
        Class702890_Ctor(named);
        at<u32>(named, 0) = 0x00702838;
        WString_Ctor(reinterpret_cast<WString *>(named + 0x34));
        at<u32>(named, 0) = 0x00702828;
        vcall<void>(named + 0x34, 0x14, 0x0078a374u);
    }
    at<u8 *>(session, 0xec) = named;
    at<u8 *>(session, 0xf0) = NewList(0x38, 0x00702818);
    at<u8 *>(session, 0xf4) = NewList(0x38, 0x00702848);
    at<u8 *>(session, 0xf8) = NewObject(0x34, [](u8 *object) { return Class7028b8_Ctor(object); });
    at<u8 *>(session, 0xfc) = NewObject(0x34, [](u8 *object) { return Class7028e0_Ctor(object); });
    at<u8 *>(session, 0x100) = NewObject(0x34, [](u8 *object) { return Class702908_Ctor(object); });
    Online_ResetCity(session);
    at<u32>(session, 0x104) = 0;
    at<u32>(session, 0x108) = 0;
    for (u32 offset : {0x328u, 0x320u, 0x324u, 0x32cu, 0x344u})
        at<u32>(session, offset) = 0;
    vcall<void>(session + 0x348, 0x14, 0x0078a374u);
    vcall<void>(session + 0x35c, 0x14, 0x0078a374u);
    for (u32 offset = 0x370; offset <= 0x380; offset += 4)
        at<u32>(session, offset) = 0;
    at<u32>(session, 0x39c) = 0;
    at<u32>(session, 0x3a0) = 0;
    at<u8 *>(session, 0x3a4) = NewObject(0xc0, [](u8 *object) { return Class70293c_Ctor(object); });
    Online_ResetDownload(session);
    return session;
}

u8 *__cdecl Online_Instance() {
    u8 *&session = *reinterpret_cast<u8 **>((uintptr_t)0x00f7d88c);
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
        if (!OfferList(session, 0xe0, request, data, 0x0077382c, kCityRelease)) {
            Say(0x007737fc);
            DropData(data, kCityDtor);
        }
        break;
    case 3:
        if (!OfferList(session, 0xe4, request, data, 0x007737cc, kParkRelease) && !OfferList(session, 0xe8, request, data, 0x007737a0, kParkRelease) &&
            !OfferList(session, 0xec, request, data, 0x00773770, kParkRelease) && !OfferList(session, 0xf0, request, data, 0x00773740, kParkRelease) &&
            !OfferList(session, 0xf4, request, data, 0x00773708, kParkRelease)) {
            Say(0x007736c8);
            DropData(data, kParkDtor);
        }
        break;
    case 4:
        if (!OfferList(session, 0xfc, request, data, 0x00773644, kServerRelease)) {
            Say(0x00773610);
            DropData(data, kServerDtor);
        }
        break;
    case 5:
        if (!OfferList(session, 0xf8, request, data, 0x007736a0, kThemeRelease)) {
            Say(0x0077366c);
            DropData(data, kThemeDtor);
        }
        break;
    case 6:
        if (!OfferList(session, 0x100, request, data, 0x007735e8, kRideRelease)) {
            Say(0x007735b8);
            DropData(data, kRideDtor);
        }
        break;
    case 7:
        Say(0x007735a0);
        failed = true;
        break;
    case 8:
        Say(0x00773584);
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
