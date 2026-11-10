#include "postcardsend.h"

#include <cstdint>
#include <cstring>

#include "../core/accessors.h"
#include "../core/crt.h"
#include "../core/dequeiter.h"
#include "../core/gameheap.h"
#include "../core/selectors.h"
#include "../core/stdlist.h"
#include "../core/stdstring.h"
#include "../world/messagecentre.h"
#include "../world/thingmsg.h"
#include "session.h"
#include "mailbox.h"
#include "chatlists.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
const char *text(u32 address) { return reinterpret_cast<const char *>((uintptr_t)address); }
char *buffer(u32 address) { return reinterpret_cast<char *>((uintptr_t)address); }

const PostcardSendEnv kGameEnv = {reinterpret_cast<void(__fastcall *)(u8 *, void *, const char *)>((uintptr_t)0x005d4a30),
                                  reinterpret_cast<u8 *(__fastcall *)(u8 *)>((uintptr_t)0x005c93c0),
                                  reinterpret_cast<void *(__fastcall *)(u8 *, void *, const char *, u32)>(&StdString_AssignChars),
                                  reinterpret_cast<char *(__cdecl *)(char *, const char *)>((uintptr_t)0x0067c4e0),
                                  &TokenList_PushChars,
                                  reinterpret_cast<void(__fastcall *)(u8 *, void *, const char *)>((uintptr_t)0x005c99f0),
                                  reinterpret_cast<u8 *(__fastcall *)(u8 *)>((uintptr_t)0x005c7420),
                                  reinterpret_cast<void(__fastcall *)(u8 *, void *, u8 *)>((uintptr_t)0x005ca010),
                                  &MessageCentre_Post,
                                  reinterpret_cast<void *(__fastcall *)(u8 *, void *, const u16 *, u32)>((uintptr_t)0x004051d0),
                                  reinterpret_cast<u32(__fastcall *)(u8 *, void *, const char *, u32, u8 *, u32)>((uintptr_t)0x005c9f10),
                                  &CommandQueue_Post};
const PostcardSendEnv *g_env = &kGameEnv;

}  // namespace

void PostcardSend_SetEnv(const PostcardSendEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl Postcard_Send() {
    Crt().sprintf(buffer(0x007cbcb0), text(reinterpret_cast<uintptr_t>(&gPostcardsend_74d320)), text(0x007cb958), text(reinterpret_cast<uintptr_t>(&gMailbox_75447c)));
    g_env->savePicture(global<u8 *>(0x007cc18c), nullptr, text(0x007cbcb0));
    char local[0x100];
    Crt().sprintf(local, text(reinterpret_cast<uintptr_t>(&gPostcard_Send_754470)), text(0x007cb958), global<u32>(0x007cbf38)++);
    (*reinterpret_cast<s32(__stdcall *const *)(const char *, const char *, s32)>((uintptr_t)0x006fd158))(text(0x007cbcb0), local, 0);

    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x94));
    u8 *const mail = memory ? g_env->mailCtor(memory) : nullptr;
    char addresses[0x200];
    char body[0x64];
    Crt().sprintf(addresses, text(reinterpret_cast<uintptr_t>(&gPostcardsend_74f8c4)), text(0x007cb4d0));
    Crt().sprintf(body, text(reinterpret_cast<uintptr_t>(&gPostcardsend_74f8c4)), text(0x007cb408));
    g_env->assign(mail + 0x1c, nullptr, body, static_cast<u32>(strlen(body)));
    for (char *token = g_env->strtok(addresses, text(reinterpret_cast<uintptr_t>(&gPostcard_Send_754468))); token; token = g_env->strtok(nullptr, text(reinterpret_cast<uintptr_t>(&gPostcard_Send_754468)))) g_env->pushToken(mail, nullptr, token);
    g_env->assign(mail + 0x2c, nullptr, local, static_cast<u32>(strlen(local)));
    g_env->attach(mail, nullptr, local);
    at<u32>(mail, 0x90) = 0x004a7bf0;

    if (!Online_LoggedIn(Online_Instance())) {
        u8 *manager = gChatlists_f81ebc;
        if (!manager) {
            u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x110));
            manager = block ? g_env->managerCtor(block) : nullptr;
            gChatlists_f81ebc = manager;
        }
        g_env->useManager(mail, nullptr, manager + 0x64);
        u8 msg[0xc];
        ThingMsg13_Ctor(msg, nullptr, 8);
        g_env->post(MessageCentre_Instance(), nullptr, msg);
        return;
    }
    char name[0x80];
    const u8 *wide = reinterpret_cast<const u8 *>(Online_UserName(Online_Instance()));
    char *to = name;
    do {
        *to = static_cast<char>(*wide);
        wide += 2;
    } while (*to++);
    g_env->assign(mail, nullptr, name, static_cast<u32>(strlen(name)));
    at<u32>(mail, 0x3c) = Get34_41ce80(Online_Instance());
    at<u32>(mail, 0x40) = Get38_41ce90(Online_Instance());
    const u16 *string = reinterpret_cast<const u16 *>(Online_String14(Online_Instance()));
    g_env->assignWide(mail + 0x44, nullptr, string, Crt().wcslen(string));
    string = reinterpret_cast<const u16 *>(Online_String28(Online_Instance()));
    g_env->assignWide(mail + 0x54, nullptr, string, Crt().wcslen(string));
    at<u32>(mail, 0x64) = 9;
    const char *host = global<const char *>(0x007a14f4);
    if (!host) host = text(0x006fe5b0);
    u32 answer;
    if (!*host)
        answer = g_env->connect(mail, nullptr, text(reinterpret_cast<uintptr_t>(&gPostcardsend_754454)), 0x19, Global7cc53c_Pick7cc4f0(), 0);
    else
        answer = g_env->connect(mail, nullptr, host, static_cast<u32>(static_cast<s32>(global<s16>(0x007a1550))), Global7cc53c_Pick7cc4f0(), 0);
    if (answer) g_env->command(0x39, 0);
}
}
