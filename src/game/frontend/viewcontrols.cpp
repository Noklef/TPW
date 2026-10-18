#include "viewcontrols.h"

#include <cstdint>

#include "../core/misc.h"
#include "../core/vcall.h"
#include "../ui/dialogs.h"
#include "lobbymodel.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

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
}  // extern "C"
