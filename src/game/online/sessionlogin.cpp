#include "sessionlogin.h"

#include <cstdint>
#include <initializer_list>

#include "../core/balancefile.h"
#include "../core/crt.h"
#include "../core/dequeiter.h"
#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/selectors.h"
#include "../core/string.h"
#include "../core/vcall.h"
#include "../files/filestream.h"
#include "../files/handles.h"
#include "../ui/dialog7cc530open.h"
#include "../ui/dialog7cc5f4.h"
#include "../ui/dialog7cc61c.h"
#include "../ui/dialog7cc654open.h"
#include "../ui/dialog7cca94.h"
#include "../ui/dialogs.h"
#include "servers.h"
#include "session.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const OnlineLoginEnv kGameEnv = {&Dialog7cc530_Open, &Dialog7cc5f4_Finish, &Dialog7cc654_Open, &Dialog7cca94_ShowText, &Dialog7cca94_Message4IfUp,
                                 &Dialog7cca94_SetProgress, &BalanceFile_Load, &Online_ApplyBalance, &Dialog7cc61c_Open, &CommandQueue_Post, &CRT_free};
const OnlineLoginEnv *g_env = &kGameEnv;

constexpr u32 kEmptyWide = 0x0078a374;

void Say(u32 text) {
    using Output = void(__stdcall *)(const char *);
    (*reinterpret_cast<const Output *>((uintptr_t)0x006fd1e0))(reinterpret_cast<const char *>((uintptr_t)text));
}

// A connection object: 0xc bytes, built by the import at the slot; none when the block is not had.
u8 *NewConnection(u32 ctor) {
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0xc));
    if (!block) return nullptr;
    using Ctor = u8 *(__fastcall *)(u8 *, void *);
    return (*reinterpret_cast<const Ctor *>((uintptr_t)ctor))(block, nullptr);
}

// X::Connect (host, pick, port, callback) through the slot: the host (0: the empty string) from the variable, the port word over its high half (mov ax).
u32 Connect(u32 slot, u8 *connection, u32 hostAt, u32 portAt, u32 callback) {
    const u32 name = *reinterpret_cast<const u32 *>((uintptr_t)hostAt);
    const u32 host = name ? name : 0x006fe5b0;
    const u32 port = (name & 0xffff0000) | *reinterpret_cast<const u16 *>((uintptr_t)portAt);
    u8 *const picked = Global7cc53c_Pick7cc4f0();
    using Method = u32(__fastcall *)(u8 *, void *, u32, u8 *, u32, u32);
    return (*reinterpret_cast<const Method *>((uintptr_t)slot))(connection, nullptr, host, picked, port, callback);
}

u32 LanguageId() { return Language_Id(reinterpret_cast<const u8 *>((uintptr_t)0x0078b530)); }

// News::GetGameServerDetails / GetGameNews through the slot (language, the game's GUID 0x702808, 0).
u32 NewsRequest(u32 slot, u8 *news) {
    const u32 language = LanguageId();
    using Method = u32(__fastcall *)(u8 *, void *, u32, u32, u32);
    return (*reinterpret_cast<const Method *>((uintptr_t)slot))(news, nullptr, language, 0x00702808, 0);
}

}  // namespace

void OnlineLogin_SetEnv(const OnlineLoginEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl Online_LoginDialogClosed() { Online_LoginFinish(Online_Instance(), nullptr, 1); }

void __fastcall Online_OpenLoginDialog(u8 *session, void *, u32 callback, u32 user) {
    at<u32>(session, 4) = callback;
    at<u32>(session, 8) = user;
    g_env->openLoginDialog(0x005b5cb0);
}

void __stdcall Online_LoginAnswer(s32 status, u32 a, u32 b, u8 *record) { Online_LoginResult(Online_Instance(), nullptr, status, a, b, record); }

void __fastcall Online_LogIn(u8 *session, void *, const u16 *name, const u16 *password) {
    vcall<void>(session + 0xc, 0x14, name);
    vcall<void>(session + 0x20, 0x14, password);
    Online_ReleaseLogin(session);
    at<u8 *>(session, 0) = NewConnection(0x006fd46c);   // Auth::Auth
    Say(reinterpret_cast<uintptr_t>(&gOnline_LogIn_77343c));
    if (Connect(0x006fd464, at<u8 *>(session, 0), 0x007a1514, 0x007a1554, 0x005b5ce0) != 1) {
        at<u32>(session, 0x68) = 0x4f;
        Online_LoginFinish(session, nullptr, 0);
        return;
    }
    Say(reinterpret_cast<uintptr_t>(&gOnline_LogIn_773414));
    const u32 password28 = at<u32>(session, 0x28), name14 = at<u32>(session, 0x14), language = LanguageId();
    using Login = void(__fastcall *)(u8 *, void *, u32, u32, u32, u32);
    (*reinterpret_cast<const Login *>((uintptr_t)0x006fd468))(at<u8 *>(session, 0), nullptr, language, name14, password28, 0);
}

void __fastcall Online_LoginFinish(u8 *session, void *, u32 flag) {
    if (at<u32>(session, 0x68)) {
        g_env->post(at<u32>(session, 0x68), 0);
        at<u32>(session, 0x68) = 0;
    }
    if (flag) {
        using Callback = void(__cdecl *)(u32, u32);
        if (const Callback callback = at<Callback>(session, 4)) {
            callback(flag, at<u32>(session, 8));
            at<u32>(session, 4) = 0;
            at<u32>(session, 8) = 0;
        }
        Online_ReleaseLogin(session);
        for (u32 offset : {0x64u, 0x60u, 0x68u, 0x4u, 0x8u})
            at<u32>(session, offset) = 0;
        vcall<void>(session + 0xc, 0x14, kEmptyWide);
        vcall<void>(session + 0x20, 0x14, kEmptyWide);
        at<u32>(session, 0x34) = 0;
        at<u32>(session, 0x38) = 0;
        vcall<void>(session + 0x3c, 0x14, kEmptyWide);
        at<u32>(session, 0x50) = 0;
        return;
    }
    if (!at<u32>(session, 0x60))
        return;
    Online_ReleaseLogin(session);
    g_env->finishDialog(0);
    at<u32>(session, 0xc8) = at<u32>(session, 4);
    at<u32>(session, 0xcc) = at<u32>(session, 8);
    if (g_env->openListsDialog() == 0)
        Online_OpenNews(session);
}

void __stdcall Online_NewsAnswer(s32 status, u32 a, u32 b, u8 *record) { Online_NewsResult(Online_Instance(), nullptr, status, a, b, record); }

void __fastcall Online_OpenNews(u8 *session) {
    Online_ReleaseNews(session);
    at<u8 *>(session, 0x6c) = NewConnection(0x006fd5a0);   // News::News
    Say(reinterpret_cast<uintptr_t>(&gOnline_OpenNews_7734e8));
    g_env->showText(0xf4);
    if (Connect(0x006fd59c, at<u8 *>(session, 0x6c), 0x007a1534, 0x007a1558, 0x005b6630) != 1) {
        Say(reinterpret_cast<uintptr_t>(&gOnline_OpenNews_7734c8));
        at<u32>(session, 0xd0) = 0x4f;
        Online_NewsFinish(session);
        return;
    }
    Say(reinterpret_cast<uintptr_t>(&gOnline_OpenNews_7734a0));
    at<u32>(session, 0x74) = NewsRequest(0x006fd598, at<u8 *>(session, 0x6c));
    at<u32>(session, 0x90) = NewsRequest(0x006fd594, at<u8 *>(session, 0x6c));
    const u32 language = LanguageId();
    using ServerNews = u32(__fastcall *)(u8 *, void *, u32, u32);
    at<u32>(session, 0xac) = (*reinterpret_cast<const ServerNews *>((uintptr_t)0x006fd590))(at<u8 *>(session, 0x6c), nullptr, language, 0);
}

void __fastcall Online_NewsResult(u8 *session, void *, s32 status, u32 a, u32, u8 *record) {
    switch (status) {
    case -6: at<u32>(session, 0xd0) = 0x4e; return;
    case -5: at<u32>(session, 0xd0) = 0x4d; return;
    case -4: at<u32>(session, 0xd0) = 0x4c; return;
    case -2: at<u32>(session, 0xd0) = 0x4b; return;
    case 51:
        if (at<u32>(session, 0x74)) {
            Say(reinterpret_cast<uintptr_t>(&gOnline_NewsResult_773508));
            vcall<void>(session + 0x7c, 0x14, at<u32>(record, 0));
            at<u32>(session, 0x78) = 1;
        }
        return;
    case 50: break;
    default: return;
    }
    const u16 *text = at<const u16 *>(record, 0);
    if (*text == 0xfffe || *text == 0xfeff)
        ++text;
    if (a == at<u32>(session, 0x90)) {
        Say(reinterpret_cast<uintptr_t>(&gOnline_NewsResult_773568));
        vcall<void>(session + 0x98, 0x14, text);
        at<u32>(session, 0x94) = 1;
    } else if (a == at<u32>(session, 0xac)) {
        Say(reinterpret_cast<uintptr_t>(&gOnline_NewsResult_773548));
        vcall<void>(session + 0xb4, 0x14, text);
        at<u32>(session, 0xb0) = 1;
    } else {
        Say(reinterpret_cast<uintptr_t>(&gOnline_NewsResult_77352c));
    }
    if (--at<s32>(record, 8) == 0 && record) {
        if (at<void *>(record, 0))
            g_env->free(at<void *>(record, 0));
        GameHeap_Free(record);
    }
}

void __fastcall Online_NewsPoll(u8 *session) {
    u8 *const news = at<u8 *>(session, 0x6c);
    if (!news)
        return;
    if (at<u32>(session, 0xd0) || (at<u32>(session, 0x78) && at<u32>(session, 0x94) && at<u32>(session, 0xb0))) {
        g_env->message4();
        Online_NewsFinish(session);
        return;
    }
    using Active = u32(__fastcall *)(u8 *, void *, u32);
    const Active active = *reinterpret_cast<const Active *>((uintptr_t)0x006fd5a4);
    u32 request = active(news, nullptr, 0);
    if (!request)
        request = active(at<u8 *>(session, 0x6c), nullptr, 1);
    if (request == at<u32>(session, 0x74))
        g_env->showText(0xf5);
    else if (request == at<u32>(session, 0x90))
        g_env->showText(0xf6);
    else if (request == at<u32>(session, 0xac))
        g_env->showText(0xf7);
    else
        g_env->message4();
    s32 done, total;
    using Progress = void(__fastcall *)(u8 *, void *, u32, s32 *, s32 *);
    (*reinterpret_cast<const Progress *>((uintptr_t)0x006fd584))(at<u8 *>(session, 0x6c), nullptr, request, &done, &total);
    if (total > 0)
        g_env->setProgress(static_cast<u32>(static_cast<s32>(static_cast<u32>(done) * 100) / total));
}

void __fastcall Online_NewsFinish(u8 *session) {
    using Callback = void(__cdecl *)(u32, u32);
    if (const Callback callback = at<Callback>(session, 0xc8)) {
        callback(0, at<u32>(session, 0xcc));
        at<u32>(session, 0xc8) = 0;
        at<u32>(session, 0xcc) = 0;
    }
    if (at<u32>(session, 0xd0)) {
        g_env->post(at<u32>(session, 0xd0), 0);
        Online_ResetNews(session);
        return;
    }
    String path;
    String_CtorCopy(&path, nullptr, reinterpret_cast<const StringView *>((uintptr_t)0x00f816e8));
    vcall<void>(&path, 0x28, 0x00f7cd80u);
    vcall<void>(&path, 0x28, 0x00f7c9f0u);
    StreamBase stream;
    StreamBase_Ctor(&stream);
    stream.vtable = reinterpret_cast<void *const *>(0x006fd930);
    s32 result;
    FileStream_OpenPath(&stream, nullptr, &result, path.data, 0xc0000010);
    u8 *const inner = at<u8 *>(&stream, 4);
    const s32 written = inner ? vcall<s32>(inner, 8, at<u32>(session, 0x84), at<u32>(session, 0x80)) : -1;
    StreamBase_Close(&stream);
    if (written >= 0 && g_env->loadBalance(reinterpret_cast<u8 *>((uintptr_t)0x00785828), nullptr, path.data, 1) == 1)
        g_env->applyBalance();
    Online_ReleaseNews(session);
    if (!at<u32>(session, 0x70)) {
        g_env->openNewsDialog();
        at<u32>(session, 0x70) = 1;
    }
    StreamBase_Dtor(&stream);
    String_Dtor(&path);
}

}  // extern "C"
