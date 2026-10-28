#include "viewcontrols.h"

#include <cstdint>

#include "../core/vcall.h"
#include "../ui/dialogs.h"
#include "lobbymodel.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// The view objects' shared part of their message handlers (0x5dced0, 0x5dd530): message 0x1000a's keys move the values, message 0x10006
// with 2 swaps the mode.
void view_message(u8 *self, u32 message, u32 key) {
    if (message == 0x1000a) {
        if (key == 0x2500) ViewCtl_Lower48(self);
        else if (key == 0x2600) ViewCtl_Raise50(self);
        else if (key == 0x2700) ViewCtl_Raise48(self);
        else if (key == 0x2800) ViewCtl_Lower50(self);
    } else if (message == 0x10006 && key == 2) {
        if (at<u32>(self, 0x58)) ViewCtl_UseFirstVector(self);
        else ViewCtl_UseSecondVector(self);
        Dialog7cdb0c_SetChild16006State(at<u32>(self, 0x58));
    }
}

}  // namespace

extern "C" {

void __fastcall ViewCtl_Lower48(u8 *self) { at<float>(self, 0x48) = at<float>(self, 0x48) - at<float>(self, 0x14); }

void __fastcall ViewCtl_Raise48(u8 *self) { at<float>(self, 0x48) = at<float>(self, 0x14) + at<float>(self, 0x48); }

void __fastcall ViewCtl_Raise50(u8 *self) { at<float>(self, 0x50) = at<float>(self, 0x14) + at<float>(self, 0x50); }

void __fastcall ViewCtl_Lower50(u8 *self) { at<float>(self, 0x50) = at<float>(self, 0x50) - at<float>(self, 0x14); }

void __fastcall ViewCtl_UseSecondVector(u8 *self) {
    at<u32>(self, 0x58) = 1;
    Dialog7cdb0c_Child16007Message6Off();
    Obj_ChooseVector(self);
}

void __fastcall ViewCtl_UseFirstVector(u8 *self) {
    at<u32>(self, 0x58) = 0;
    Dialog7cdb0c_Child16007Message6On();
    Obj_ChooseVector(self);
}

s32 __fastcall ViewCtl702e70_ShowDialogs(u8 *) {
    Dialog7cdb0c_Message6On();
    Dialog7cc4c4_Message6On();
    return Dialog7cdb0c_Child16007Message6On();
}

s32 __fastcall ViewCtl702e20_ShowDialogs(u8 *) {
    Dialog7cdb0c_Message6On();
    return Dialog7cc4c4_Message6On();
}

s32 __fastcall Scene702dd0_ShowDialog(u8 *self) {
    const s32 result = Dialog7cc47c_Message6On();
    at<u32>(self, 0xe8) = 1;
    return result;
}

s32 __fastcall Scene702dd0_HideDialog(u8 *) { return Dialog7cc47c_Message6Off(); }

s32 __cdecl Scene702dd0_ShowDialogCallback() { return Dialog7cc47c_Message6On(); }

s32 __cdecl Scene702dd0_DialogDone(s32 accepted, u8 *self) {
    if (accepted == 0)
        return vcall<s32>(self, 0x44);
    const u32 state = at<u32>(self, 0x14);
    if (state != 0) {
        if (state == 2)
            LobbyModel_Play(at<u8 *>(at<u8 *>(self, 0xc), 8), nullptr, 1, 0);
        at<u32>(self, 0x14) = 0;
    }
    return Dialog7cc47c_Message6On();
}

void __fastcall ViewCtl702bc8_Message(u8 *self, void *, u32 message, u32 key, u32) { view_message(self, message, key); }

void __fastcall ViewCtl702e70_Message(u8 *self, void *, u32 message, u32 key, u32) {
    view_message(self, message, key);
    if (message == 0x10005 && key == 0) {
        if (const u8 *const target = at<const u8 *>(self, 0x70)) {
            *reinterpret_cast<u32 *>((uintptr_t)0x00f82888) = at<u32>(target, 0xc);
            vcall<void>(self, 0x20);
            vcall<void>(at<u8 *>(*reinterpret_cast<u8 **>((uintptr_t)0x00f85614), 8), 0x1c);
        }
    }
}

void __fastcall Obj_ChooseVector(u8 *object) {
    const u32 base = at<u32>(object, 0x58) ? 0x28 : 0x1c;
    const u32 x = at<u32>(object, base), y = at<u32>(object, base + 4), z = at<u32>(object, base + 8);
    at<u32>(object, 0x10) = x;
    at<u32>(object, 0x14) = y;
    at<u32>(object, 0x18) = z;
}

u32 __stdcall Obj_Positive40(const u8 *object) { return at<s32>(object, 0x40) > 0 ? 1 : 0; }

u32 __stdcall Obj_Pending(const u8 *object) {
    return at<u32>(object, 0x60) == 0 && at<s32>(object, 0x40) < at<s32>(object, 0x64) ? 1 : 0;
}

void __fastcall Obj_SetOrKeepC(u8 *object, void *, u32 value) {
    if (value) {
        at<u32>(object, 0xc) = value;
        at<u32>(object, 0x14) = 0;
    }
}
}  // extern "C"
