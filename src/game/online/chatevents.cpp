#include "chatevents.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/buffers.h"
#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/indexed.h"
#include "../core/selectors.h"
#include "../core/stdstring.h"
#include "../files/charsets.h"
#include "../core/stdtree.h"
#include "../core/string.h"
#include "../core/textarg.h"
#include "../core/trivial.h"
#include "../core/wstringmap.h"
#include "../core/wtraits.h"
#include "../ui/dialogs.h"
#include "../ui/window7cc8bc.h"
#include "../world/mapcell.h"
#include "../world/person.h"
#include "../world/personscripts.h"
#include "../world/things.h"
#include "../sprite/script.h"
#include "chatavatar.h"
#include "chatcommand.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename F> F game(u32 address) { return reinterpret_cast<F>((uintptr_t)address); }

const ChatEventEnv kGameEnv = {&Window7cc8bc_ListSetDataStd,
                               &Window7cc8bc_ListSetData1Std,
                               &Window7cc8bc_ListRemoveStd,
                               &Window7cc8bc_ListRemove1Std,
                               game<void(__fastcall *)(void *, void *, u32, s32, const StdWString *, u32)>(0x004f7b20),
                               &Thing_Delete,
                               reinterpret_cast<u8 *(__fastcall *)(const u8 *, void *, void *)>(&StdString_CopyInto),
                               game<void(__fastcall *)(u8 *, void *, s32, u32, u32)>(0x005b87e0),
                               &Sprite_SetDestinationMarked,
                               &Thing_ResetPlaceAt,
                               &Person_ReleaseScripts,
                               &Person_PlaceScript,
                               &SpriteScript_SetRegF,
                               &ChatAvatar_PlaceAtCell,
                               &Charset_FormatWide,
                               &Window7cc8bc_SetC3128};
const ChatEventEnv *g_env = &kGameEnv;

const u16 *const kNullStr = reinterpret_cast<const u16 *>(0x006fd6c4);

// A method of the Chat (+0) through its import slot (__thiscall), the avatar its context (the last argument).
template <typename... A> void tell(u32 slot, u8 *avatar, A... args) {
    using Method = u32(__fastcall *)(void *, void *, A..., void *);
    (*reinterpret_cast<const Method *>((uintptr_t)slot))(at<void *>(avatar, 0), nullptr, args..., avatar);
}

void debugLine(u32 text) { (*reinterpret_cast<void(__stdcall *const *)(u32)>((uintptr_t)0x006fd1e0))(text); }

const u8 *token(const u8 *tokens, u32 i) { return at<const u8 *>(tokens, 4) + 0x24 * i; }
s32 word(const u8 *tokens, u32 i) { return at<s32>(token(tokens, i), 0x20); }
s32 count(const u8 *tokens) {
    const u32 first = at<u32>(tokens, 4);
    return first ? (s32)(at<u32>(tokens, 8) - first) / 0x24 : 0;
}

void text(const u8 *tokens, u32 i, StdWString *out) { ChatToken_Text(token(tokens, i), nullptr, out); }
void done(StdWString *s, u32 release = 1) { CountedBuf_Reset(reinterpret_cast<u8 *>(s), nullptr, release); }
const u16 *chars(const StdWString *s) { return s->ptr ? s->ptr : kNullStr; }
u16 *line(u8 *avatar) { return &at<u16>(avatar, 0x260); }

void textArg(u8 *arg, u32 n, const u16 *s) {
    TextArg_Ctor(arg, nullptr, n);
    at<u32>(arg, 0) = 0x006ff174;
    at<const u16 *>(arg, 0xc) = s;
}
template <typename... A> void format(u8 *avatar, u32 entry, A... args) {
    g_env->format(global<u8 *>(0x0078bac8), line(avatar), 0x400, entry, args...);
}
void format1(u8 *avatar, u32 entry, const u16 *s) {
    u8 arg[16];
    textArg(arg, 0, s);
    format(avatar, entry, arg);
}

void assigned(u8 *avatar, StdWString *s) {
    done(s, 0);
    ChatCommand_Env()->assign(s, nullptr, line(avatar), Crt().wcslen(line(avatar)));
}
void grown(u8 *avatar, StdWString *s) {
    done(s, 0);
    const u32 n = Crt().wcslen(line(avatar));
    if (StdWString_Grow(s, nullptr, n, 1)) {
        WTraits_Copy(s->ptr, line(avatar), n);
        ImeWString_SetLength(reinterpret_cast<u8 *>(s), nullptr, n);
    }
}
void noticeOf(u8 *avatar, const StdWString *s, u32 color) {
    StdWString copy;
    StdWString_CtorCopy(&copy, nullptr, s);
    ChatAvatar_Notice(avatar, nullptr, copy, color);
}
void noticeAssigned(u8 *avatar) {
    StdWString s;
    assigned(avatar, &s);
    noticeOf(avatar, &s, 7);
    done(&s);
}
void noticeGrown(u8 *avatar) {
    StdWString s;
    grown(avatar, &s);
    noticeOf(avatar, &s, 7);
    done(&s);
}
void noticeChars(u8 *avatar, u32 color) {
    const u8 allocator = 0;
    StdWString s;
    StdWString_CtorChars(&s, nullptr, line(avatar), &allocator);
    ChatAvatar_Notice(avatar, nullptr, s, color);
}

// Entry with token 1's text (argument 0).
void formatName(u8 *avatar, const u8 *tokens, u32 entry) {
    StdWString t;
    text(tokens, 1, &t);
    format1(avatar, entry, chars(&t));
    done(&t);
}
// Entry with tokens 1 and 2's texts (arguments 0 and 1).
void formatTwo(u8 *avatar, const u8 *tokens, u32 entry) {
    StdWString second, first;
    u8 arg1[16], arg0[16];
    text(tokens, 2, &second);
    textArg(arg1, 1, chars(&second));
    text(tokens, 1, &first);
    textArg(arg0, 0, chars(&first));
    format(avatar, entry, arg0, arg1);
    done(&first);
    done(&second);
}
// (0x1e, 0x3c, 0x51: Array36_At, TextArgString_Ctor)
void noticeTextOfName(u8 *avatar, const u8 *tokens, u32 entry, u32 n) {
    StdWString t;
    ChatToken_Text(Array36_At(tokens, nullptr, 1), nullptr, &t);
    u8 arg[16];
    format(avatar, entry, TextArgString_Ctor(arg, nullptr, n, Obj_Get4Or6fd6c4(reinterpret_cast<const u8 *>(&t))));
    CountedBuf_ReleaseAndClear(reinterpret_cast<u8 *>(&t));
    const u8 allocator = 0;
    StdWString s;
    StdWString_CtorChars(&s, nullptr, line(avatar), &allocator);
    ChatAvatar_NoticeText(avatar, nullptr, &s);
    CountedBuf_ReleaseAndClear(reinterpret_cast<u8 *>(&s));
}

void keyOfName(const u8 *tokens, StdWString *name, StdWString *key) {
    text(tokens, 1, name);
    WStdString_CtorCopy(key, nullptr, name);
}
bool buddyOfName(u8 *avatar, const u8 *tokens, u32 *it) {
    StdWString name, key;
    keyOfName(tokens, &name, &key);
    ChatBuddySet_Find(&at<u8>(avatar, 0xe90), nullptr, it, reinterpret_cast<const u8 *>(&key));
    done(&key);
    done(&name);
    return *it != at<u32>(avatar, 0xe94);
}
bool ignoredName(u8 *avatar, const u8 *tokens, u32 *it) {
    StdWString name, key;
    keyOfName(tokens, &name, &key);
    ChatIgnoreSet_Find(&at<u8>(avatar, 0xea0), nullptr, it, reinterpret_cast<const u8 *>(&key));
    done(&key);
    done(&name);
    return *it != at<u32>(avatar, 0xea4);
}
bool inRoom(u8 *avatar, const u8 *tokens) {
    StdWString name;
    u32 found;
    text(tokens, 1, &name);
    WStringMap_Find(&at<u8>(avatar, 0xe80), nullptr, &found, reinterpret_cast<const u8 *>(&name));
    done(&name);
    return found != at<u32>(avatar, 0xe84);
}
// Token 1's ChatAvatar_NameFlags told to a window list.
void tellFlags(u8 *avatar, const u8 *tokens, void(__stdcall *list)(const u16 *, u32)) {
    StdWString name, key, shown;
    keyOfName(tokens, &name, &key);
    text(tokens, 1, &shown);
    const u16 *const s = chars(&shown);
    list(s, ChatAvatar_NameFlags(avatar, nullptr, &key));
    done(&shown);
    done(&key);
    done(&name);
}
void addToRoom(u8 *avatar, const u8 *tokens) {
    StdWString name;
    u32 out;
    text(tokens, 1, &name);
    ChatCommand_Env()->listAdd(chars(&name), 0);
    StdMapD_InsertCopy(&at<u8>(avatar, 0xe80), nullptr, &out, &name);
    done(&name);
}
void makePlayer(u32 cell, s32 x, const StdWString *name) {
    void *const block = GameHeap_Alloc(0x3a8);
    if (block) g_env->playerCtor(block, nullptr, cell, x, name, 0);
}
u8 *findPlayer(const StdWString *name) {
    void *const world = global<void *>(0x007cf83c);
    u16 handle;
    World_FirstThing(world, nullptr, &handle);
    while (handle) {
        u8 *const thing = Thing_ByHandle(handle);
        if (at<u8>(thing, 2) == 0x12 && ChatPlayer_NameIs(thing, nullptr, reinterpret_cast<const u8 *>(name))) return thing;
        u16 next;
        handle = *World_NextThing(world, nullptr, &next, thing);
    }
    return nullptr;
}
u8 *sameThing(u8 *thing) {
    u16 handle;
    return Thing_ByHandle(*Thing_GetHandle(thing, nullptr, &handle));
}
u8 *ensureSession() {
    u8 *session = global<u8 *>(0x00f7d88c);
    if (!session) {
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x404));
        session = block ? ChatCommand_Env()->makeSession(block) : nullptr;
        global<u8 *>(0x00f7d88c) = session;
    }
    return session;
}

// 0x22: a chat player's move.
void movePlayer(u8 *avatar, const u8 *tokens) {
    StdWString name;
    text(tokens, 1, &name);
    u8 *const player = findPlayer(&name);
    done(&name);
    if (!player || player == at<u8 *>(avatar, 4)) return;
    const s32 a = word(tokens, 2), b = word(tokens, 3);
    const s32 mode = word(tokens, 4);
    const u32 x = (u16)((u8)(a / 256) << 8 | (u8)(a % 256)), y = (u16)((u8)(b / 256) << 8 | (u8)(b % 256));
    if (mode == 0) {
        u8 *const thing = sameThing(player);
        if (at<u8>(thing, 2) == 0x12) g_env->destination(thing, nullptr, x, y);
    } else if (mode == 1) {
        u8 *const thing = sameThing(player);
        if (at<u8>(thing, 2) == 0x12) g_env->placeAt(thing, nullptr, x, y);
    } else if (mode == 2) {
        g_env->releaseScripts(player);
    } else if (mode == 3) {
        g_env->placeScript(player, nullptr, 0, 0.0f, 0, 0, 3);
        g_env->setRegF(at<s32>(player, 0xc), 8, 0x3fc00000);
        u8 *const thing = sameThing(player);
        if (at<u8>(thing, 2) == 0x12) g_env->placeAt(thing, nullptr, x, y);
    } else {
        debugLine(0x0075c508);
    }
}

// 0x52: to the room of token 4, or (already there) to the cell of the person of token 1.
void goToPerson(u8 *avatar, const u8 *tokens) {
    if (at<u8>(avatar, 0xef8) == 1) {
        debugLine(0x0075c418);
        return;
    }
    StdString narrow;
    const u8 *const got = g_env->tokenNarrow(token(tokens, 4), nullptr, &narrow);
    const char *const digits = at<const char *>(got, 4);
    const s32 room = Crt().atoi(digits ? digits : reinterpret_cast<const char *>(0x006fe5b0));
    StdString_Tidy(&narrow, nullptr, 1);
    u8 *session = ensureSession();
    if (room != at<s32>(reinterpret_cast<const void *>((uintptr_t)Obj_Get3a4(session)), 8)) {
        session = ensureSession();
        g_env->join(session, nullptr, room, 0x004e5230, 0);
        at<u8>(avatar, 0xef8) = 1;
        return;
    }
    StdWString name;
    text(tokens, 1, &name);
    const u8 *const player = findPlayer(&name);
    done(&name);
    if (!player) return;
    const u32 cell = (u16)((u32)at<u8>(player, 7) * 0x80 + at<u8>(player, 5) + 1);
    u16 handle;
    if (!g_env->placeAtCell(avatar, nullptr, *Thing_GetHandle(at<u8 *>(avatar, 4), nullptr, &handle), cell)) return;
    const s16 x = (s16)(((cell - 1) & 0x7f) << 8 | 0x80), z = (s16)((u8)((cell - 1) >> 7) << 8 | 0x80);
    tell(0x006fd4cc, avatar, (s32)x, (s32)z, 1u);
    at<u8>(avatar, 0xeb1) = 0;
    tell(0x006fd4bc, avatar);
}

}  // namespace

void ChatEvents_SetEnv(const ChatEventEnv *env) { g_env = env ? env : &kGameEnv; }

void __fastcall ChatAvatar_HandleEvent(u8 *avatar, void *, const u8 *tokens) {
    const u8 *const first = at<const u8 *>(tokens, 4);
    u32 it;
    switch (at<u32>(first, 0x20)) {
    case 0xb:
    case 0xc: {
        const u32 entered = at<u32>(first, 0x20) == 0xb;
        format(avatar, entered ? 0x60 : 0x61);
        StdWString s;
        grown(avatar, &s);
        noticeOf(avatar, &s, 7);
        g_env->setC3128((u8)entered);
        done(&s);
        break;
    }
    case 0xf:
        if ((u32)count(tokens) < 2 || !at<u32>(first, 0x44)) break;
        format(avatar, 0x79);
        noticeGrown(avatar);
        break;
    case 0x10:
        if (count(tokens) != 2 || !at<u32>(first, 0x44)) break;
        format(avatar, 0x7a);
        noticeGrown(avatar);
        break;
    case 0x11:
        formatTwo(avatar, tokens, 0x58);
        noticeChars(avatar, 1);
        break;
    case 0x12:
        formatTwo(avatar, tokens, 0x5d);
        noticeChars(avatar, 4);
        break;
    case 0x13: {
        formatTwo(avatar, tokens, 0x59);
        noticeChars(avatar, 2);
        StdWString name;
        text(tokens, 1, &name);
        StdWString_AssignSub(&at<StdWString>(avatar, 0xe70), nullptr, &name, 0, global<u32>(0x006fef00));
        done(&name);
        break;
    }
    case 0x16:
    case 0x18:
    case 0x1a:
    case 0x1c:
        formatTwo(avatar, tokens, 0x5c);
        noticeChars(avatar, 3);
        break;
    case 0x1e:
        noticeTextOfName(avatar, tokens, 0xa0, 1);
        break;
    case 0x1f:
        formatName(avatar, tokens, 0x74);
        noticeGrown(avatar);
        break;
    case 0x22:
        movePlayer(avatar, tokens);
        break;
    case 0x23: {
        if (!buddyOfName(avatar, tokens, &it)) break;
        const bool here = inRoom(avatar, tokens);
        formatName(avatar, tokens, here ? 0x6f : 0x70);
        StdWString s;
        assigned(avatar, &s);
        noticeOf(avatar, &s, 7);
        debugLine(here ? 0x0075c4c8 : 0x0075c488);
        at<u8>(reinterpret_cast<const void *>((uintptr_t)it), 0x1d) = here;
        done(&s);
        at<u8>(reinterpret_cast<const void *>((uintptr_t)it), 0x1c) = 1;
        tellFlags(avatar, tokens, g_env->listSetData1);
        break;
    }
    case 0x24: {
        if (!buddyOfName(avatar, tokens, &it)) break;
        formatName(avatar, tokens, 0x71);
        StdWString s;
        assigned(avatar, &s);
        noticeOf(avatar, &s, 7);
        at<u8>(reinterpret_cast<const void *>((uintptr_t)it), 0x1d) = 0;
        at<u8>(reinterpret_cast<const void *>((uintptr_t)it), 0x1c) = 0;
        tellFlags(avatar, tokens, g_env->listSetData1);
        done(&s);
        break;
    }
    case 0x25: {
        const s32 x = word(tokens, 3);
        StdWString name;
        text(tokens, 1, &name);
        u16 entrance;
        const u32 cell = *Map_EntrancePos(&at<u8>(global<u8 *>(0x007cf83c), 0x2d8), nullptr, &entrance, 1);
        makePlayer(cell, x, &name);
        done(&name);
        addToRoom(avatar, tokens);
        if (!buddyOfName(avatar, tokens, &it)) break;
        at<u8>(reinterpret_cast<const void *>((uintptr_t)it), 0x1d) = 1;
        tellFlags(avatar, tokens, g_env->listSetData1);
        debugLine(0x0075c448);
        break;
    }
    case 0x26: {
        StdWString name;
        text(tokens, 1, &name);
        u8 *const player = ChatPlayers_FindByName(reinterpret_cast<const u8 *>(&name));
        if (player) g_env->thingDelete(player);
        done(&name);
        text(tokens, 1, &name);
        g_env->listRemove(chars(&name));
        WStringMap_EraseKey(&at<u8>(avatar, 0xe80), nullptr, reinterpret_cast<const u8 *>(&name));
        done(&name);
        if (!buddyOfName(avatar, tokens, &it)) break;
        at<u8>(reinterpret_cast<const void *>((uintptr_t)it), 0x1d) = 0;
        tellFlags(avatar, tokens, g_env->listSetData1);
        break;
    }
    case 0x27: {
        StdWString name;
        text(tokens, 1, &name);
        const bool own = WString_Compare(reinterpret_cast<const u8 *>(&name), nullptr, &at<u8>(avatar, 8)) == 0;
        done(&name);
        if (own) break;
        const s32 x = word(tokens, 3), y = word(tokens, 4), z = word(tokens, 5);
        text(tokens, 1, &name);
        u32 cell = (u32)y;   // (the word in the stack slot of y; its high half stays)
        u16 made;
        cell = cell & 0xffff0000u | (z == -1 ? *Map_EntrancePos(&at<u8>(global<u8 *>(0x007cf83c), 0x2d8), nullptr, &made, 1) : *Cell_MakeHandle(&made, y / 256, z / 256));
        makePlayer(cell, x, &name);
        done(&name);
        addToRoom(avatar, tokens);
        break;
    }
    case 0x2a:
        if ((u32)count(tokens) <= 1) break;
        formatName(avatar, tokens, 0x5e);
        noticeAssigned(avatar);
        break;
    case 0x2e:
        formatName(avatar, tokens, 0x6e);
        noticeAssigned(avatar);
        break;
    case 0x2f:
        formatName(avatar, tokens, 0x66);
        noticeAssigned(avatar);
        break;
    case 0x38: {
        u32 flags = (u32)count(tokens) >= 3 && at<u32>(first, 0x68) ? 2 : 0;
        if (inRoom(avatar, tokens)) flags |= 4;
        StdWString name;
        text(tokens, 1, &name);
        const u8 added = ChatAvatar_SetBuddy(avatar, nullptr, &name, flags | 1);
        done(&name);
        if (!added) break;
        formatName(avatar, tokens, 0x6a);
        noticeAssigned(avatar);
        break;
    }
    case 0x39: {
        StdWString name;
        text(tokens, 1, &name);
        StdWString key;
        WStdString_CtorCopyC(&key, nullptr, &name);
        ChatBuddySet_EraseKey(&at<u8>(avatar, 0xe90), nullptr, reinterpret_cast<const u8 *>(&key));
        done(&key);
        WStdString_CtorCopyC(&key, nullptr, &name);
        const u32 flags = ChatAvatar_NameFlags(avatar, nullptr, &key);
        done(&key);
        g_env->listRemove1(chars(&name));
        g_env->listSetData(chars(&name), flags);
        done(&name);
        formatName(avatar, tokens, 0x6c);
        noticeAssigned(avatar);
        break;
    }
    case 0x3a: {
        if (ignoredName(avatar, tokens, &it)) break;
        StdWString name, key;
        keyOfName(tokens, &name, &key);
        u8 *inserted;
        StdMapE_Insert(&at<u8>(avatar, 0xea0), nullptr, &inserted, &key);   // (and Pair_Assign into a local of no use)
        done(&key);
        done(&name);
        formatName(avatar, tokens, 0x62);
        StdWString s;
        assigned(avatar, &s);
        noticeOf(avatar, &s, 7);
        tellFlags(avatar, tokens, g_env->listSetData);
        tellFlags(avatar, tokens, g_env->listSetData1);
        done(&s);
        break;
    }
    case 0x3b: {
        if (!ignoredName(avatar, tokens, &it)) break;
        u8 *next;
        StdTree_EraseE(&at<u8>(avatar, 0xea0), nullptr, &next, reinterpret_cast<u8 *>((uintptr_t)it));
        formatName(avatar, tokens, 0x63);
        StdWString s;
        assigned(avatar, &s);
        noticeOf(avatar, &s, 7);
        tellFlags(avatar, tokens, g_env->listSetData);
        tellFlags(avatar, tokens, g_env->listSetData1);
        done(&s);
        break;
    }
    case 0x3c:
        noticeTextOfName(avatar, tokens, 0x73, 0);
        break;
    case 0x3d: {
        StdWString three, two, one;
        u8 arg5[16], arg4[16], arg0[16];
        text(tokens, 3, &three);
        textArg(arg5, 5, chars(&three));
        text(tokens, 2, &two);
        textArg(arg4, 4, chars(&two));
        text(tokens, 1, &one);
        textArg(arg0, 0, chars(&one));
        format(avatar, 0x98, arg0, arg4, arg5);
        done(&one);
        done(&two);
        done(&three);
        noticeAssigned(avatar);
        break;
    }
    case 0x3f:
        formatTwo(avatar, tokens, 0x5a);
        noticeAssigned(avatar);
        break;
    case 0x40:
        formatTwo(avatar, tokens, 0x5b);
        noticeAssigned(avatar);
        break;
    case 0x41:
    case 0x42:
        formatName(avatar, tokens, 0x7d);
        noticeAssigned(avatar);
        break;
    case 0x44:
        format(avatar, 0x68);
        noticeAssigned(avatar);
        break;
    case 0x45:
        formatName(avatar, tokens, 0x64);
        noticeAssigned(avatar);
        break;
    case 0x47: {
        format(avatar, 0x5f);
        StdWString s;
        grown(avatar, &s);
        ChatAvatar_NoticeText(avatar, nullptr, &s);
        CountedBuf_ReleaseAndClear(reinterpret_cast<u8 *>(&s));
        break;
    }
    case 0x4f: {
        Fn_5ce530(&at<u8>(avatar, 0xe90), nullptr, (u32)(uintptr_t)&it);
        u8 *node = reinterpret_cast<u8 *>((uintptr_t)it), *old;
        while (node != at<u8 *>(avatar, 0xe94)) {
            tell(0x006fd488, avatar, node + 0xc);
            StdTreeC_PostInc(&node, nullptr, &old, 0);
        }
        Fn_5ce530(&at<u8>(avatar, 0xea0), nullptr, (u32)(uintptr_t)&it);
        node = reinterpret_cast<u8 *>((uintptr_t)it);
        while (node != at<u8 *>(avatar, 0xea4)) {
            tell(0x006fd484, avatar, node + 0xc);
            StdTreeE_PostInc(&node, nullptr, &old, 0);
        }
        break;
    }
    case 0x51:
        noticeTextOfName(avatar, tokens, 0x67, 0);
        break;
    case 0x52:
        goToPerson(avatar, tokens);
        break;
    default:
        break;
    }
}

u8 __fastcall ChatAvatar_SetBuddy(u8 *avatar, void *, const StdWString *name, u32 flags) {
    const u8 star = (u8)(flags >> 1 & 1), online = (u8)(flags >> 2 & 1);
    StdWString key;
    u32 found;
    WStdString_CtorCopy(&key, nullptr, name);
    u8 *const set = &at<u8>(avatar, 0xe90);
    ChatBuddySet_Find(set, nullptr, &found, reinterpret_cast<const u8 *>(&key));
    done(&key);
    u8 made = 0;
    if (found != at<u32>(avatar, 0xe94)) {
        at<u8>(reinterpret_cast<const void *>((uintptr_t)found), 0x1c) = star;
        at<u8>(reinterpret_cast<const void *>((uintptr_t)found), 0x1d) = online;
    } else {
        StdWString copy;
        done(&copy, 0);
        WStdString_AssignSub(&copy, nullptr, name, 0, global<u32>(0x00700628));
        struct {
            StdWString name;
            u8 star, online;
        } value;
        WStdString_CtorCopy(&value.name, nullptr, &copy);
        value.star = star;
        value.online = online;
        u8 *result;
        StdMapC_Insert(set, nullptr, &result, &value.name);
        done(&value.name);
        done(&copy);
        made = 1;
    }
    StdWString copy;
    done(&copy, 0);
    WStdString_AssignSub(&copy, nullptr, name, 0, global<u32>(0x00700628));
    const u32 shown = ChatAvatar_NameFlags(avatar, nullptr, &copy);
    done(&copy);
    const u16 *const s = name->ptr ? name->ptr : kNullStr;
    ChatCommand_Env()->listAdd1(s, shown);
    g_env->listSetData(s, shown);
    return made;
}
