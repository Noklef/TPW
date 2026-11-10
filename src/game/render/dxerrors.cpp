#include "dxerrors.h"

#include <cstdint>

namespace {

const char *Text(u32 address) { return reinterpret_cast<const char *>((uintptr_t)address); }

}  // namespace

extern "C" {

const char *__cdecl DxResult_Message(u32 result) {
    switch (result) {
    case 0x00000000u: return gText_DxResult_Message_7663c0;
    case 0x80004001u: return gText_DxResult_Message_7682f4;
    case 0x80004005u: return gText_DxResult_Message_7682e0;
    case 0x8007000eu: return gText_DxResult_Message_76829c;
    case 0x80070057u: return gText_DxResult_Message_768254;
    case 0x88760005u: return gText_DxResult_Message_76822c;
    case 0x8876000au: return gText_DxResult_Message_7681f0;
    case 0x88760014u: return gText_DxResult_Message_7681b0;
    case 0x88760028u: return gText_DxResult_Message_768188;
    case 0x88760037u: return gText_DxResult_Message_768140;
    case 0x8876005au: return gText_DxResult_Message_7680fc;
    case 0x8876005fu: return gText_DxResult_Message_7680a8;
    case 0x88760064u: return gText_DxResult_Message_768064;
    case 0x8876006eu: return gText_DxResult_Message_768030;
    case 0x88760078u: return gText_DxResult_Message_767ffc;
    case 0x88760082u: return gText_DxResult_Message_767fb4;
    case 0x88760091u: return gText_DxResult_Message_767f88;
    case 0x88760096u: return gText_DxResult_Message_767f64;
    case 0x887600a0u: return gText_DxResult_Message_767f14;
    case 0x887600aau: return gText_DxResult_Message_767ef8;
    case 0x887600b4u: return gText_DxResult_Message_767e8c;
    case 0x887600cdu: return gText_DxResult_Message_767e74;
    case 0x887600d2u: return gText_DxResult_Message_767e0c;
    case 0x887600d4u: return gText_DxResult_Message_767db0;
    case 0x887600d7u: return gText_DxResult_Message_767d84;
    case 0x887600dcu: return gText_DxResult_Message_767d1c;
    case 0x887600e1u: return gText_DxResult_Message_767cac;
    case 0x887600e6u: return gText_DxResult_Message_767c7c;
    case 0x887600f0u: return gText_DxResult_Message_767c60;
    case 0x887600fau: return gText_DxResult_Message_767c08;
    case 0x887600ffu: return gText_DxResult_Message_767be8;
    case 0x88760104u: return gText_DxResult_Message_767b88;
    case 0x88760118u: return gText_DxResult_Message_767b18;
    case 0x88760122u: return gText_DxResult_Message_767ab8;
    case 0x88760136u: return gText_DxResult_Message_767a5c;
    case 0x8876013cu: return gText_DxResult_Message_767918;
    case 0x8876013du: return gText_DxResult_Message_767984;
    case 0x88760140u: return gText_DxResult_Message_7679fc;
    case 0x8876014au: return gText_DxResult_Message_7678b0;
    case 0x8876014fu: return gText_DxResult_Message_767838;
    case 0x88760154u: return gText_DxResult_Message_7677d8;
    case 0x8876015eu: return gText_DxResult_Message_767758;
    case 0x88760168u: return gText_DxResult_Message_767708;
    case 0x8876017cu: return gText_DxResult_Message_76829c;
    case 0x8876017eu: return gText_DxResult_Message_7676d4;
    case 0x88760180u: return gText_DxResult_Message_767694;
    case 0x88760183u: return gText_DxResult_Message_767630;
    case 0x88760190u: return gText_DxResult_Message_767600;
    case 0x8876019au: return gText_DxResult_Message_7675b4;
    case 0x887601a4u: return gText_DxResult_Message_76755c;
    case 0x887601aeu: return gText_DxResult_Message_7674f8;
    case 0x887601b8u: return gText_DxResult_Message_7674bc;
    case 0x887601c2u: return gText_DxResult_Message_767414;
    case 0x887601ccu: return gText_DxResult_Message_7673ec;
    case 0x887601d6u: return gText_DxResult_Message_7673bc;
    case 0x887601e0u: return gText_DxResult_Message_767364;
    case 0x887601eau: return gText_DxResult_Message_767334;
    case 0x887601feu: return gText_DxResult_Message_7672fc;
    case 0x88760208u: return gText_DxResult_Message_7672b4;
    case 0x88760219u: return gText_DxResult_Message_767294;
    case 0x8876021cu: return gText_DxResult_Message_767220;
    case 0x88760230u: return gText_DxResult_Message_7671d8;
    case 0x88760231u: return gText_DxResult_Message_766e60;
    case 0x88760232u: return gText_DxResult_Message_767048;
    case 0x88760233u: return gText_DxResult_Message_766cf4;
    case 0x88760234u: return gText_DxResult_Message_7669cc;
    case 0x88760235u: return gText_DxResult_Message_766cd0;
    case 0x88760236u: return gText_DxResult_Message_766994;
    case 0x88760237u: return gText_DxResult_Message_7670a4;
    case 0x88760238u: return gText_DxResult_Message_766da8;
    case 0x88760239u: return gText_DxResult_Message_766c68;
    case 0x8876023au: return Text((u32)(uintptr_t)&gText_DxResult_Message_766f04);
    case 0x8876023bu: return gText_DxResult_Message_766f74;
    case 0x8876023cu: return gText_DxResult_Message_766bb8;
    case 0x8876023du: return gText_DxResult_Message_766b84;
    case 0x8876023eu: return gText_DxResult_Message_767180;
    case 0x8876023fu: return gText_DxResult_Message_766dd8;
    case 0x88760240u: return gText_DxResult_Message_766d5c;
    case 0x88760241u: return gText_DxResult_Message_766a04;
    case 0x88760242u: return gText_DxResult_Message_766be8;
    case 0x88760243u: return gText_DxResult_Message_766df8;
    case 0x88760244u: return gText_DxResult_Message_766b40;
    case 0x88760245u: return gText_DxResult_Message_766ff0;
    case 0x88760246u: return gText_DxResult_Message_766afc;
    case 0x88760247u: return gText_DxResult_Message_767104;
    case 0x88760248u: return gText_DxResult_Message_766a7c;
    case 0x88760249u: return gText_DxResult_Message_767158;
    case 0x8876024au: return gText_DxResult_Message_766d7c;
    case 0x8876024bu: return gText_DxResult_Message_766944;
    case 0x8876024cu: return gText_DxResult_Message_766eb4;
    case 0x8876024du: return gText_DxResult_Message_766a44;
    case 0x887602bcu: return gText_DxResult_Message_76692c;
    case 0x887602bdu: return gText_DxResult_Message_766914;
    case 0x887602c6u: return gText_DxResult_Message_7668f4;
    case 0x887602c7u: return gText_DxResult_Message_7668a0;
    case 0x887602c8u: return gText_DxResult_Message_766884;
    case 0x887602c9u: return gText_DxResult_Message_766864;
    case 0x887602cau: return gText_DxResult_Message_7668dc;
    case 0x887602cbu: return gText_DxResult_Message_7668c0;
    case 0x887602ccu: return gText_DxResult_Message_76684c;
    case 0x887602cdu: return gText_DxResult_Message_76682c;
    case 0x887602d0u: return gText_DxResult_Message_766810;
    case 0x887602d1u: return gText_DxResult_Message_7667bc;
    case 0x887602d2u: return gText_DxResult_Message_76679c;
    case 0x887602d3u: return gText_DxResult_Message_766780;
    case 0x887602d4u: return gText_DxResult_Message_766760;
    case 0x887602d5u: return gText_DxResult_Message_766744;
    case 0x887602d7u: return gText_DxResult_Message_7667dc;
    case 0x887602d8u: return gText_DxResult_Message_7667f4;
    case 0x887602dau: return gText_DxResult_Message_766724;
    case 0x887602dbu: return gText_DxResult_Message_766704;
    case 0x887602dcu: return gText_DxResult_Message_7666e4;
    case 0x887602deu: return gText_DxResult_Message_7666c4;
    case 0x887602e4u: return gText_DxResult_Message_7666a4;
    case 0x887602e5u: return gText_DxResult_Message_766684;
    case 0x887602e6u: return gText_DxResult_Message_766664;
    case 0x887602eeu: return gText_DxResult_Message_766648;
    case 0xb0754000u: return gText_DxResult_Message_766628;
    case 0xb0754001u: return gText_DxResult_Message_766610;
    case 0xb0754002u: return gText_DxResult_Message_7665f8;
    case 0xb0754003u: return gText_DxResult_Message_7665e0;
    case 0xb0754004u: return gText_DxResult_Message_7665bc;
    case 0xb0754005u: return gText_DxResult_Message_766598;
    case 0xb0754006u: return gText_DxResult_Message_766580;
    case 0xb0754007u: return gText_DxResult_Message_766564;
    case 0xb0754008u: return gText_DxResult_Message_76653c;
    case 0xb0754009u: return gText_DxResult_Message_766524;
    case 0xb075400au: return gText_DxResult_Message_766514;
    case 0xb075400bu: return gText_DxResult_Message_7664fc;
    case 0xb075400cu: return gText_DxResult_Message_7664e0;
    case 0xb075400du: return gText_DxResult_Message_7664c4;
    case 0xb075400eu: return gText_DxResult_Message_7664b0;
    case 0xb075400fu: return gText_DxResult_Message_7664a0;
    case 0xb0754010u: return gText_DxResult_Message_76647c;
    case 0xb0754011u: return gText_DxResult_Message_766458;
    case 0xb0754012u: return gText_DxResult_Message_766448;
    case 0xb0754013u: return gText_DxResult_Message_766434;
    case 0xb0754014u: return gText_DxResult_Message_766420;
    case 0xb0754015u: return gText_DxResult_Message_766408;
    case 0xb0754016u: return gText_DxResult_Message_7663f0;
    case 0xb0754017u: return gText_DxResult_Message_7663e4;
    case 0xb0754018u: return gText_DxResult_Message_7663cc;
    default: return gText_DxResult_Message_7663a8;
    }
}
}
