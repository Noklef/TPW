#include "chatcommand.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/buffers.h"
#include "../core/countedpair.h"
#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/globalsave.h"
#include "../core/selectors.h"
#include "../core/stdtree.h"
#include "../core/textarg.h"
#include "../core/trivial.h"
#include "../core/wstringmap.h"
#include "../core/wscanner.h"
#include "../core/wtraits.h"
#include "../files/charsets.h"
#include "../ui/dialogs.h"
#include "../ui/window7cc8bc.h"
#include "itemlist.h"
#include "session.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename F> F game(u32 address) { return reinterpret_cast<F>((uintptr_t)address); }

const u16 *const kNullStr = reinterpret_cast<const u16 *>(0x006fd6c4);    // the first wide string copy's _Nullstr
const u16 *const kNullStr2 = reinterpret_cast<const u16 *>(0x00700624);   // the second copy's
u32 npos() { return global<u32>(0x006fef00); }

const ChatCommandEnv kGameEnv = {&Charset_FormatWide,
                                 game<StdWString *(__fastcall *)(StdWString *, void *, const u16 *, u32)>(0x004051d0),
                                 game<void(__fastcall *)(u8 *, void *, StdWString *, u32)>(0x005d3230),
                                 &Window7cc8bc_ChatAddStd,
                                 &Window7cc8bc_ListAddStd,
                                 &Window7cc8bc_ListAdd1Std,
                                 game<u8 *(__fastcall *)(u8 *)>(0x005b50b0),
                                 game<void(__cdecl *)(u8 *, u32, u32, u32, u32, u32, u32, u32)>(0x0047f020)};
const ChatCommandEnv *g_env = &kGameEnv;

void xlen() { reinterpret_cast<void(__cdecl *)()>(0x006918f0)(); }  // std::_Xlen: throws length_error

void reset(StdWString *s, u32 release) { CountedBuf_Reset(reinterpret_cast<u8 *>(s), nullptr, release); }

// A method of the Chat (+0) through its import slot (__thiscall), the avatar its context (the last argument).
template <typename... A> void tell(u32 slot, u8 *avatar, A... args) {
    using Method = u32(__fastcall *)(void *, void *, A..., void *);
    (*reinterpret_cast<const Method *>((uintptr_t)slot))(at<void *>(avatar, 0), nullptr, args..., avatar);
}

// OutputDebugStringA (stdcall) through its import slot.
void debugLine(u32 text) { (*reinterpret_cast<void(__stdcall *const *)(u32)>((uintptr_t)0x006fd1e0))(text); }

// The tokens vector's count ((last - first) / 0x24, signed as the original's; no first: 0) and its token i.
u32 count(const u8 *tokens) {
    const u8 *const first = at<const u8 *>(tokens, 4);
    return first ? (u32)((s32)(at<const u8 *>(tokens, 8) - first) / 0x24) : 0;
}
const u8 *token(const u8 *tokens, u32 i) { return at<const u8 *>(tokens, 4) + 0x24 * i; }

// Whether token i's text is not empty (a copy made, read and released).
bool hasText(const u8 *tokens, u32 i) {
    StdWString text;
    const u32 n = ChatToken_Text(token(tokens, i), nullptr, &text)->len;
    reset(&text, 1);
    return n != 0;
}

// The import (a method taking a const std::wstring & and the context) told token i's text (a copy, released after).
void tellText(u32 slot, u8 *avatar, const u8 *tokens, u32 i) {
    StdWString text;
    tell(slot, avatar, ChatToken_Text(token(tokens, i), nullptr, &text));
    reset(&text, 1);
}

// The same with a flag between (Say, RoomShout).
void tellTextFlag(u32 slot, u8 *avatar, const u8 *tokens, u32 i, u32 flag) {
    StdWString text;
    tell(slot, avatar, ChatToken_Text(token(tokens, i), nullptr, &text), flag);
    reset(&text, 1);
}

// How a notice's buffer text becomes a std::wstring: agent B's assign(chars, n) (0x4051d0, through the env), _Grow(n, true) and the
// characters (inlined; a terminator and the length only when there is room), or 0x4051d0 inlined (ChatCommand_AssignChars).
enum Assign { kAssignCall, kAssignGrow, kAssignInline };

void assign(StdWString *s, const u16 *chars, Assign how) {
    const u32 n = Crt().wcslen(chars);
    if (how == kAssignCall) {
        g_env->assign(s, nullptr, chars, n);
    } else if (how == kAssignInline) {
        ChatCommand_AssignChars(s, nullptr, chars, n);
    } else if (StdWString_Grow(s, nullptr, n, 1)) {
        WTraits_Copy(s->ptr, chars, n);
        ImeWString_SetLength(reinterpret_cast<u8 *>(s), nullptr, n);
    }
}

// The charset [0x78bac8]'s entry formatted into the avatar's +0x260 (0x400 characters), with a text argument when chars is given (the
// string argument: vtable 0x6ff174, tag 'MRAV', number 0, the characters at +0xc; built on TextArg_Ctor as the original mostly does).
u16 *format(u8 *avatar, u32 entry, const u16 *chars, bool withText) {
    u16 *const buffer = &at<u16>(avatar, 0x260);
    if (withText) {
        u8 arg[16];
        TextArg_Ctor(arg, nullptr, 0);
        at<u32>(arg, 0) = (u32)(uintptr_t)TextArgString_vtable;
        at<const u16 *>(arg, 0xc) = chars;
        g_env->format(global<u8 *>(0x0078bac8), buffer, 0x400, entry, arg);
    } else {
        g_env->format(global<u8 *>(0x0078bac8), buffer, 0x400, entry);
    }
    return buffer;
}

// A notice: the entry formatted, made a std::wstring and a copy of it shown in colour 7 (ChatAvatar_Notice); the string released.
void noticeWith(u8 *avatar, u32 entry, const u16 *chars, bool withText, Assign how = kAssignCall) {
    u16 *const buffer = format(avatar, entry, chars, withText);
    StdWString text;
    text.allocator = 0;   // (an uninitialised stack byte in the original)
    reset(&text, 0);
    assign(&text, buffer, how);
    StdWString copy;
    StdWString_CtorCopy(&copy, nullptr, &text);
    ChatAvatar_Notice(avatar, nullptr, copy, 7);
    reset(&text, 1);
}
void notice(u8 *avatar, u32 entry, Assign how = kAssignCall) { noticeWith(avatar, entry, nullptr, false, how); }

// A notice naming a reply target: no target (its length 0) the entry `none`, else `named` with the target's characters (null: 0x6fd6c4).
void targetNotice(u8 *avatar, u32 target, u32 none, u32 named) {
    if (!at<u32>(avatar, target + 8)) {
        notice(avatar, none);
        return;
    }
    const u16 *chars = at<const u16 *>(avatar, target + 4);
    noticeWith(avatar, named, chars ? chars : kNullStr, true);
}

// Token 1's text as the second copy's string (the copy constructor) and the player's own name (+8) the same.
void ownAndKey(u8 *avatar, const u8 *tokens, StdWString *key, StdWString *own) {
    StdWString text;
    WStdString_CtorCopy(key, nullptr, ChatToken_Text(token(tokens, 1), nullptr, &text));
    reset(&text, 1);
    WStdString_CtorCopy(own, nullptr, &at<const StdWString>(avatar, 8));
}

// compare(0, a's length, b's characters (null: 0x700624), b's length) without case, == 0.
bool sameSub(const StdWString *a, const StdWString *b) {
    return WStdString_CompareSub(a, nullptr, 0, a->len, b->ptr ? b->ptr : kNullStr2, b->len) == 0;
}

// The online session (0xf7d88c), made when missing (Mem_Alloc(0x404), 0x5b50b0; none: it stays 0).
u8 *makeSession() {
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x404));
    u8 *const session = block ? g_env->makeSession(block) : nullptr;
    gChatcommand_f7d88c = session;
    return session;
}
u8 *session() {
    u8 *const s = gChatcommand_f7d88c;
    return s ? s : makeSession();
}

// A wide string made from characters (null: 0x6fd6c4) by _Grow(n, true) and the characters (inlined), told to the import (Goto,
// Locate) and released. allocator: the string's allocator byte (garbage in the original).
void tellChars(u32 slot, u8 *avatar, const u16 *chars, u8 allocator) {
    StdWString s;
    s.allocator = allocator;
    reset(&s, 0);
    assign(&s, chars ? chars : kNullStr, kAssignGrow);
    tell(slot, avatar, &s);
    reset(&s, 1);
}

void vote(u8 *avatar) {
    const u32 key = at<u32>(reinterpret_cast<u8 *>((uintptr_t)Obj_Get3a4(session())), 0x48);
    if (at<u32>(Owner2c_FindByKey(at<u8 *>(session(), 0xe0), nullptr, key), 0x60))
        return;
    if (at<u32>(avatar, 0xec4)) {
        debugLine(reinterpret_cast<uintptr_t>(&gChatcommand_75c61c));
        return;
    }
    u32 guid[4];
    for (u32 k = 0; k < 4; ++k) guid[k] = gChatcommand_7005d8[k];
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0xc));
    using Ctor = u32(__fastcall *)(u8 *, void *);
    const u32 made = block ? (*reinterpret_cast<const Ctor *>((uintptr_t)0x006fd5f4))(block, nullptr) : 0;
    at<u32>(avatar, 0xec4) = made;
    if (!made)
        return;
    debugLine(reinterpret_cast<uintptr_t>(&gChatcommand_75c600));
    Window7cc8bc_SetC3129(1);
    const char *host = global<const char *>(0x007a1544);
    if (!host) host = reinterpret_cast<const char *>(0x006fe5b0);
    const u32 port = global<u16>(0x007a155a);
    const u32 pick = (u32)(uintptr_t)Global7cc53c_Pick7cc4f0();
    using Connect = u32(__fastcall *)(u32, void *, const char *, u32, u32, u32);
    if ((*reinterpret_cast<const Connect *>((uintptr_t)0x006fd5f8))(at<u32>(avatar, 0xec4), nullptr, host, pick, port, 0x004e4fc0) != 1)
        return;
    debugLine(reinterpret_cast<uintptr_t>(&gChatcommand_75c5f0));
    u8 *s = gChatcommand_f7d88c;   // (made up to three times: twice, then once more for the third getter)
    if (!s && !(s = makeSession()))
        s = makeSession();
    u8 *const first = s;
    if (!s)
        s = makeSession();
    const u32 name = (u32)(uintptr_t)Online_String28(s);
    const u32 b = Get38_41ce90(first);
    const u32 c = Get34_41ce80(s);
    using Info = u32(__fastcall *)(u32, void *, u32, u32, u32, u32);
    (*reinterpret_cast<const Info *>((uintptr_t)0x006fd5fc))(at<u32>(avatar, 0xec4), nullptr, 9, c, b, name);
    const u32 y = at<u32>(reinterpret_cast<u8 *>((uintptr_t)Obj_Get3a4(session())), 8);
    using Send = u32(__fastcall *)(u32, void *, u32, u32 *, u32, u32);
    (*reinterpret_cast<const Send *>((uintptr_t)0x006fd600))(at<u32>(avatar, 0xec4), nullptr, y, guid, 1, 0);
    debugLine(reinterpret_cast<uintptr_t>(&gChatcommand_75c5e4));
}

// 0x13: Goto.
void gotoName(u8 *avatar, const u8 *tokens) {
    if (at<u8>(avatar, 0xef8) == 1) {
        debugLine(reinterpret_cast<uintptr_t>(&gChatcommand_75c418));
        return;
    }
    if (count(tokens) <= 1) {
        notice(avatar, 0x72, kAssignGrow);
        return;
    }
    StdWString key, own;
    ownAndKey(avatar, tokens, &key, &own);
    if (sameSub(&key, &own)) {
        notice(avatar, 0x9c);
    } else if (hasText(tokens, 1)) {
        StdWString text;
        StdWString_AssignSub(&at<StdWString>(avatar, 0xefc), nullptr, ChatToken_Text(token(tokens, 1), nullptr, &text), 0, npos());
        reset(&text, 1);
        u8 found = 0;
        StdWString name;
        WStdString_CtorCopy(&name, nullptr, ChatToken_Text(token(tokens, 1), nullptr, &text));
        reset(&text, 1);
        for (u8 *it = at<u8 *>(at<u8 *>(avatar, 0xe84), 0); it != at<u8 *>(avatar, 0xe84);) {
            StdWString other;
            WStdString_CtorCopy(&other, nullptr, &at<const StdWString>(it, 0xc));
            const bool same = sameSub(&name, &other);
            reset(&other, 1);
            if (same) {
                found = 1;
                break;
            }
            StdTree_IncD(&it);
        }
        const u16 *chars = ChatToken_Text(token(tokens, 1), nullptr, &text)->ptr;
        if (found) {
            tellChars(0x006fd490, avatar, chars, found);
            reset(&text, 1);
        } else {
            format(avatar, 0x7f, chars ? chars : kNullStr, true);   // (the argument built inline in the original: the same bytes)
            reset(&text, 1);
            u8 message[16];
            at<u16 *>(message, 0) = &at<u16>(avatar, 0x260);
            message[4] = message[5] = message[6] = 0xff;   // (byte 7 left as it is: an uninitialised stack byte, sic)
            at<u32>(message, 8) = 4;
            at<u32>(message, 0xc) = 0;
            g_env->dialog(message, 2, 0, 1, 0x004e5210, 0, 0, 0);
        }
        reset(&name, 1);
    }
    reset(&own, 1);
    reset(&key, 1);
}

// The command run alone (name null) or on a name, its tokens made as the callbacks 0x4ead00-0x4eb5d0 make them (see the header).
void runOn(u8 *avatar, u32 command, const StdWString *name, u8 allocator) {
    alignas(4) u8 tokens[16] = {};               // a vector of CountedPairs: the allocator byte, first, last, end
    tokens[0] = allocator;
    CountedPair t;
    t.narrow.allocator = allocator;
    StdString_Tidy(&t.narrow, nullptr, 0);
    t.wide.allocator = allocator;
    reset(&t.wide, 0);
    t.value = command;
    CountedPairVector_InsertN(tokens, nullptr, at<u32>(tokens, 8), 1, &t);
    if (name) {
        reset(&t.wide, 1);
        StdString_Tidy(&t.narrow, nullptr, 1);
        StdWString copy;
        copy.allocator = *reinterpret_cast<const u8 *>(name);
        reset(&copy, 0);
        StdWString_AssignSub(&copy, nullptr, name, 0, npos());
        CountedPair_CtorWide(&t, nullptr, copy);
        CountedPairVector_InsertN(tokens, nullptr, at<u32>(tokens, 8), 1, &t);
    }
    reset(&t.wide, 1);   // (the original's releases, some inlined: the same)
    StdString_Tidy(&t.narrow, nullptr, 1);
    ChatAvatar_RunCommand(avatar, nullptr, tokens);
    for (u8 *e = at<u8 *>(tokens, 4); e != at<u8 *>(tokens, 8); e += 0x24)
        CountedPair_Clear(e);
    GameHeap_Free(at<u8 *>(tokens, 4));
}

void runOn(u8 *avatar, u32 command, const StdWString *name) {
    runOn(avatar, command, name, (u8)(uintptr_t)name);   // (the name's address's low byte, sic; alone: stack garbage in the original)
}

// The buddy set's node of the key by its lower bound (out: that bound, a node or the head), else the head.
u32 findBuddy(const u8 *avatar, const StdWString *key, u32 &bound) {
    u8 *const set = const_cast<u8 *>(avatar) + 0xe90;
    bound = 0;
    ChatBuddySet_LowerBoundOut(set, nullptr, &bound, reinterpret_cast<const u8 *>(key));
    const u32 head = at<u32>(set, 4);
    if (bound == head || WStdString_Compare(key, nullptr, reinterpret_cast<const StdWString *>((uintptr_t)bound + 0xc)) < 0)
        return head;
    return bound;
}

void buddyCommand(u8 *avatar, const StdWString *name, bool whenBuddy) {
    StdWString copy;
    copy.allocator = *reinterpret_cast<const u8 *>(name);
    reset(&copy, 0);
    WStdString_AssignSub(&copy, nullptr, name, 0, global<u32>(0x00700628));
    u32 bound;
    const bool buddy = findBuddy(avatar, &copy, bound) != at<u32>(avatar, 0xe94);
    reset(&copy, 1);
    if (buddy == whenBuddy)
        runOn(avatar, 0xf, name, (u8)bound);   // (the allocator bytes the bound's low byte, sic: its slot was the name's)
}

void ignoreCommand(u8 *avatar, const StdWString *name, bool whenIgnored) {
    StdWString copy;
    copy.allocator = *reinterpret_cast<const u8 *>(name);
    reset(&copy, 0);
    WStdString_AssignSub(&copy, nullptr, name, 0, global<u32>(0x00700628));
    const u8 *const bound = ChatIgnoreSet_LowerBound(avatar + 0xea0, nullptr, reinterpret_cast<const u8 *>(&copy));
    const bool ignored = (uintptr_t)bound != at<u32>(avatar, 0xea4)
        && WStdString_Compare(&copy, nullptr, reinterpret_cast<const StdWString *>(bound + 0xc)) >= 0;
    reset(&copy, 1);
    if (ignored == whenIgnored)
        runOn(avatar, 0xd, name, (u8)(uintptr_t)bound);   // (the bound's low byte, sic, as buddyCommand's)
}

}  // namespace

extern "C" {

void ChatCommand_SetEnv(const ChatCommandEnv *env) { g_env = env ? env : &kGameEnv; }
const ChatCommandEnv *ChatCommand_Env() { return g_env; }

StdWString *__fastcall ChatToken_Text(const u8 *token, void *, StdWString *out) {
    return StdWString_CtorCopy(out, nullptr, &at<const StdWString>(token, 0x10));
}

void __fastcall ChatAvatar_Notice(u8 *, void *, StdWString text, u32 color) {
    if (GlobalSave_Byte25(GlobalSave_Instance()))
        g_env->censor(GlobalSave_Instance(), nullptr, &text, 1);
    g_env->chatAdd(const_cast<u16 *>(text.ptr ? text.ptr : kNullStr), color);
    if (text.ptr) {  // _Tidy(true), inlined
        u8 &refs = *reinterpret_cast<u8 *>(reinterpret_cast<uintptr_t>(text.ptr) - 1);
        if (refs != 0 && refs != 0xff)
            --refs;
        else
            GameHeap_Free(reinterpret_cast<u8 *>(text.ptr) - 2);
    }
}

void __fastcall ChatAvatar_ParseCommand(u8 *avatar, void *, StdWString text) {
    if (!text.len) {
        reset(&text, 1);
        return;
    }
    alignas(4) u8 tokens[16] = {};               // a vector of CountedPairs: the allocator byte, first, last, end
    StdWString out;
    out.allocator = 0;
    reset(&out, 0);
    alignas(4) u8 scanner[0x58];
    WScanner_Construct(scanner, nullptr, tokens, &out);
    StdWString line;
    line.allocator = text.allocator;
    reset(&line, 0);
    StdWString_AssignSub(&line, nullptr, &text, 0, npos());
    WScanner_Parse(scanner, nullptr, line);
    const u8 *const first = at<const u8 *>(tokens, 4);
    if (!first || (at<const u8 *>(tokens, 8) - first) / 0x24 == 0) {
        CountedPair t;
        t.narrow.allocator = 0;
        StdString_Tidy(&t.narrow, nullptr, 0);
        t.wide.allocator = 0;
        reset(&t.wide, 0);
        t.value = 0x17;
        CountedPairVector_InsertN(tokens, nullptr, at<u32>(tokens, 8), 1, &t);
        reset(&t.wide, 1);
        StdString_Tidy(&t.narrow, nullptr, 1);
        StdWString copy;
        copy.allocator = out.allocator;
        reset(&copy, 0);
        StdWString_AssignSub(&copy, nullptr, &out, 0, npos());
        CountedPair_CtorWide(&t, nullptr, copy);
        CountedPairVector_InsertN(tokens, nullptr, at<u32>(tokens, 8), 1, &t);
        reset(&t.wide, 1);
        StdString_Tidy(&t.narrow, nullptr, 1);
    }
    ChatAvatar_RunCommand(avatar, nullptr, tokens);
    WScanner_Destruct(scanner);   // (the original: 0x4f04d0 on +0x44, then 0x5e6a00)
    reset(&out, 1);
    for (u8 *e = at<u8 *>(tokens, 4); e != at<u8 *>(tokens, 8); e += 0x24)
        CountedPair_Clear(e);
    GameHeap_Free(at<u8 *>(tokens, 4));
    if (text.ptr) {  // _Tidy(true), inlined
        u8 &refs = *reinterpret_cast<u8 *>(reinterpret_cast<uintptr_t>(text.ptr) - 1);
        if (refs != 0 && refs != 0xff)
            --refs;
        else
            GameHeap_Free(reinterpret_cast<u8 *>(text.ptr) - 2);
    }
}

void __fastcall ChatAvatar_Blackmark(u8 *avatar, void *, const StdWString *name) { runOn(avatar, 0xe, name); }
void __fastcall ChatAvatar_Page(u8 *avatar, void *, const StdWString *name) { runOn(avatar, 0x11, name); }
void __fastcall ChatAvatar_Locate(u8 *avatar, void *, const StdWString *name) { runOn(avatar, 0x14, name); }
void __fastcall ChatAvatar_Goto(u8 *avatar, void *, const StdWString *name) { runOn(avatar, 0x13, name); }
void __fastcall ChatAvatar_ToggleIgnore(u8 *avatar, void *, const StdWString *name) { runOn(avatar, 0xd, name); }
void __fastcall ChatAvatar_Vote(u8 *avatar) { runOn(avatar, 0x15, nullptr); }
void __fastcall ChatAvatar_ToggleEarmuffs(u8 *avatar) { runOn(avatar, 0xc, nullptr); }
void __fastcall ChatAvatar_AddBuddy(u8 *avatar, void *, const StdWString *name) { buddyCommand(avatar, name, false); }
void __fastcall ChatAvatar_RemoveBuddy(u8 *avatar, void *, const StdWString *name) { buddyCommand(avatar, name, true); }
void __fastcall ChatAvatar_AddIgnore(u8 *avatar, void *, const StdWString *name) { ignoreCommand(avatar, name, false); }
void __fastcall ChatAvatar_RemoveIgnore(u8 *avatar, void *, const StdWString *name) { ignoreCommand(avatar, name, true); }

void __fastcall ChatAvatar_EarmuffsOn(u8 *avatar) {
    if (!at<u8>(avatar, 0xeb0)) ChatAvatar_ToggleEarmuffs(avatar);
}

void __fastcall ChatAvatar_EarmuffsOff(u8 *avatar) {
    if (at<u8>(avatar, 0xeb0)) ChatAvatar_ToggleEarmuffs(avatar);
}

u32 __fastcall ChatAvatar_NameFlags(const u8 *avatar, void *, const StdWString *name) {
    u32 flags = 0;
    u32 bound;
    const u32 buddy = findBuddy(avatar, name, bound);
    if (buddy != at<u32>(avatar, 0xe94)) {
        const u8 *const node = reinterpret_cast<const u8 *>((uintptr_t)buddy);
        flags = at<u8>(node, 0x1c) ? 3 : 1;
        if (at<u8>(node, 0x1d)) flags |= 4;
    }
    u32 ignored;
    ChatIgnoreSet_Find(const_cast<u8 *>(avatar) + 0xea0, nullptr, &ignored, reinterpret_cast<const u8 *>(name));
    if (ignored != at<u32>(avatar, 0xea4)) flags |= 8;
    return flags;
}

void __fastcall ChatAvatar_NoticeText(u8 *avatar, void *, const StdWString *text) {
    StdWString copy;
    StdWString_CtorCopy(&copy, nullptr, text);
    ChatAvatar_Notice(avatar, nullptr, copy, 7);
}

void __fastcall ChatAvatar_GotoDone(u8 *avatar) {
    notice(avatar, 0x9d, kAssignGrow);
    at<u8>(avatar, 0xef8) = 0;
}

void __fastcall ChatAvatar_FillRoomList(u8 *avatar) {
    u8 *it = *at<u8 **>(avatar, 0xe84);
    while (it != at<u8 *>(avatar, 0xe84)) {
        const StdWString *const name = &at<const StdWString>(it, 0xc);
        StdWString copy;
        copy.allocator = *reinterpret_cast<const u8 *>(name);
        reset(&copy, 0);
        WStdString_AssignSub(&copy, nullptr, name, 0, global<u32>(0x00700628));
        const u32 flags = ChatAvatar_NameFlags(avatar, nullptr, &copy);
        reset(&copy, 1);
        const u16 *const chars = at<const u16 *>(it, 0x10);
        g_env->listAdd(chars ? chars : reinterpret_cast<const u16 *>(0x006fd6c4), flags);
        StdTree_IncD(&it);
    }
}

void __fastcall ChatAvatar_FillBuddyList(u8 *avatar) {
    u8 *it = *at<u8 **>(avatar, 0xe94);
    while (it != at<u8 *>(avatar, 0xe94)) {
        const u32 flags = ChatAvatar_NameFlags(avatar, nullptr, &at<const StdWString>(it, 0xc));
        const u16 *const chars = at<const u16 *>(it, 0x10);
        g_env->listAdd1(chars ? chars : reinterpret_cast<const u16 *>(0x00700624), flags);
        StdTree_IncC(&it);
    }
}

StdWString *__fastcall ChatCommand_AssignChars(StdWString *s, void *, const u16 *chars, u32 n) {
    if (n > Const7ffffffd_4f6080())
        xlen();
    if (s->ptr) {
        u8 &refs = *reinterpret_cast<u8 *>(reinterpret_cast<uintptr_t>(s->ptr) - 1);
        if (refs != 0 && refs != 0xff) {
            if (!n) {
                --refs;
                s->ptr = nullptr;
                s->len = 0;
                s->res = 0;
                return s;
            }
            StdWString_Copy(s, nullptr, n);
            goto copy;
        }
    }
    if (!n) {
        reset(s, 1);
        return s;
    }
    if (s->res > 0x1f || s->res < n) {
        reset(s, 1);
        StdWString_Copy(s, nullptr, n);
    }
copy:
    WTraits_Copy(s->ptr, chars, n);
    ImeWString_SetLength(reinterpret_cast<u8 *>(s), nullptr, n);
    return s;
}

void __fastcall ChatAvatar_RunCommand(u8 *avatar, void *, const u8 *tokens) {
    if (!count(tokens))
        return;
    if (at<u32>(token(tokens, 0), 0x20) != 0x12) {
        at<u8>(avatar, 0xeb1) = 0;
        tell(0x006fd4bc, avatar);   // Chat::NotAFK
    }
    switch (at<u32>(token(tokens, 0), 0x20)) {
    case 0:   // Say
        if (count(tokens) > 1 && hasText(tokens, 1))
            tellTextFlag(0x006fd4b8, avatar, tokens, 1, 0);
        break;
    case 1:   // Tell, keeping the target
        if (count(tokens) > 1) {
            StdWString message, to;
            ChatToken_Text(token(tokens, 2), nullptr, &message);
            tell(0x006fd4b4, avatar, ChatToken_Text(token(tokens, 1), nullptr, &to), &message, 0u);
            reset(&to, 1);
            reset(&message, 1);
            StdWString_AssignSub(&at<StdWString>(avatar, 0xe60), nullptr, ChatToken_Text(token(tokens, 1), nullptr, &to), 0, npos());
            reset(&to, 1);
        } else {
            targetNotice(avatar, 0xe60, 0xb3, 0x56);
        }
        break;
    case 2:   // reply (the last who told)
    case 3: { // tell the last told again
        const u32 target = at<u32>(token(tokens, 0), 0x20) == 2 ? 0xe70 : 0xe60;
        const u32 none = target == 0xe70 ? 0xb2 : 0xb3, named = target == 0xe70 ? 0x57 : 0x56;
        if (count(tokens) > 1) {
            if (!at<u32>(avatar, target + 8)) {
                notice(avatar, none);
            } else {
                StdWString text;
                tell(0x006fd4b4, avatar, &at<StdWString>(avatar, target), ChatToken_Text(token(tokens, 1), nullptr, &text), 0u);
                reset(&text, 1);
            }
        } else {
            targetNotice(avatar, target, none, named);
        }
        break;
    }
    case 4:   // RoomShout
    case 7:
        if (at<u8>(avatar, 0xeb0))
            notice(avatar, 0x5f);
        else if (hasText(tokens, 1))
            tellTextFlag(0x006fd4b0, avatar, tokens, 1, at<u32>(token(tokens, 0), 0x20) == 7 ? 1u : 0u);
        break;
    case 5:   // Say (emote)
        if (hasText(tokens, 1))
            tellTextFlag(0x006fd4b8, avatar, tokens, 1, 1);
        break;
    case 6:   // Tell (emote)
        if (count(tokens) > 1) {
            StdWString message, to;
            ChatToken_Text(token(tokens, 2), nullptr, &message);
            tell(0x006fd4b4, avatar, ChatToken_Text(token(tokens, 1), nullptr, &to), &message, 1u);
            reset(&to, 1);
            reset(&message, 1);
        } else {
            notice(avatar, 0x72);
        }
        break;
    case 0xa:   // SetHearingRange (the second token read whatever the count, sic)
        tell(0x006fd4d4, avatar, at<u32>(token(tokens, 1), 0x20));
        break;
    case 0xc:   // the earmuffs toggled
        if (at<u8>(avatar, 0xeb0)) {
            at<u8>(avatar, 0xeb0) = 0;
            tell(0x006fd4ac, avatar);
        } else {
            at<u8>(avatar, 0xeb0) = 1;
            tell(0x006fd4a8, avatar);
        }
        break;
    case 0xd:   // UnIgnore / Ignore
        if (count(tokens) > 1) {
            StdWString text, key;
            WStdString_CtorCopy(&key, nullptr, ChatToken_Text(token(tokens, 1), nullptr, &text));
            u32 it;
            ChatIgnoreSet_Find(&at<u8>(avatar, 0xea0), nullptr, &it, reinterpret_cast<const u8 *>(&key));
            reset(&key, 1);
            reset(&text, 1);
            if (it != at<u32>(avatar, 0xea4)) {
                tellText(0x006fd4a4, avatar, tokens, 1);
                break;
            }
            StdWString own;
            ownAndKey(avatar, tokens, &key, &own);
            if (WStdString_Compare(&key, nullptr, &own) == 0) {
                debugLine(reinterpret_cast<uintptr_t>(&gChatAvatar_RunCommand_75c680));
                notice(avatar, 0xb1);
            } else if (hasText(tokens, 1)) {
                tellText(0x006fd484, avatar, tokens, 1);
            }
            reset(&own, 1);
            reset(&key, 1);
        } else {
            notice(avatar, 0x72);
        }
        break;
    case 0xe:   // Blackmark
        if (count(tokens) > 1) {
            StdWString key, own;
            ownAndKey(avatar, tokens, &key, &own);
            if (WStdString_Compare(&key, nullptr, &own) == 0) {
                debugLine(reinterpret_cast<uintptr_t>(&gChatAvatar_RunCommand_75c660));
                notice(avatar, 0xb0);
            } else if (hasText(tokens, 1)) {
                tellText(0x006fd4a0, avatar, tokens, 1);
            }
            reset(&own, 1);
            reset(&key, 1);
        } else {
            notice(avatar, 0x69);
        }
        break;
    case 0xf:   // RemoveBuddy / AddBuddy
        if (count(tokens) > 1) {
            if (!hasText(tokens, 1)) {
                notice(avatar, 0x72);
                break;
            }
            StdWString text, key;
            WStdString_CtorCopy(&key, nullptr, ChatToken_Text(token(tokens, 1), nullptr, &text));
            u32 it;
            ChatBuddySet_Find(&at<u8>(avatar, 0xe90), nullptr, &it, reinterpret_cast<const u8 *>(&key));
            reset(&key, 1);
            reset(&text, 1);
            if (it != at<u32>(avatar, 0xe94)) {
                tellText(0x006fd49c, avatar, tokens, 1);
                break;
            }
            StdWString own;
            ownAndKey(avatar, tokens, &key, &own);
            if (sameSub(&key, &own))
                notice(avatar, 0x9b);
            else
                tellText(0x006fd488, avatar, tokens, 1);
            reset(&own, 1);
            reset(&key, 1);
        } else {
            notice(avatar, 0x72);
        }
        break;
    case 0x11:   // Page
        if (count(tokens) > 1)
            tellText(0x006fd4d0, avatar, tokens, 1);
        else
            notice(avatar, 0x72);
        break;
    case 0x12: {   // away toggled: the notice's text said
        const bool away = at<u8>(avatar, 0xeb1) != 0;
        if (away)
            at<u8>(avatar, 0xeb1) = 0;
        u16 *const buffer = format(avatar, away ? 0x7c : 0x7b, nullptr, false);
        StdWString text;
        text.allocator = 0;
        reset(&text, 0);
        assign(&text, buffer, kAssignCall);
        tell(0x006fd4b8, avatar, &text, 0u);
        if (!away) {
            at<u8>(avatar, 0xeb1) = 1;
            tell(0x006fd4c8, avatar);   // Chat::AFK
        }
        reset(&text, 1);
        break;
    }
    case 0x13:
        gotoName(avatar, tokens);
        break;
    case 0x14:   // Locate
        if (count(tokens) > 1 && hasText(tokens, 1)) {
            StdWString text;
            tellChars(0x006fd48c, avatar, ChatToken_Text(token(tokens, 1), nullptr, &text)->ptr, 0);
            reset(&text, 1);
        } else {
            notice(avatar, 0x72, kAssignGrow);
        }
        break;
    case 0x15:
        vote(avatar);
        break;
    case 0x16: {   // the global save's byte +0x25 toggled
        const bool on = GlobalSave_Byte25(GlobalSave_Instance()) != 0;
        GlobalSave_SetByte25(GlobalSave_Instance(), nullptr, on ? 0 : 1);
        notice(avatar, on ? 0x85 : 0x84);
        break;
    }
    case 0x17:   // Say
        if (count(tokens) > 1) {
            if (hasText(tokens, 1))
                tellTextFlag(0x006fd4b8, avatar, tokens, 1, 0);
        } else {
            notice(avatar, 0xa1, kAssignInline);
        }
        break;
    case 0x18:
        notice(avatar, 0x9a);
        break;
    default:
        break;
    }
}
}
