#include "viewcontrols.h"

#include <cstdint>

#include "../core/misc.h"
#include "../ui/dialogs.h"

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
}  // extern "C"
