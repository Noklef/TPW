#include "dxerrors.h"

#include <cstdint>

namespace {

const char *Text(u32 address) { return reinterpret_cast<const char *>((uintptr_t)address); }

}  // namespace

extern "C" {

const char *__cdecl DxResult_Message(u32 result) {
    switch (result) {
    case 0x00000000u: return Text(0x7663c0);
    case 0x80004001u: return Text(0x7682f4);
    case 0x80004005u: return Text(0x7682e0);
    case 0x8007000eu: return Text(0x76829c);
    case 0x80070057u: return Text(0x768254);
    case 0x88760005u: return Text(0x76822c);
    case 0x8876000au: return Text(0x7681f0);
    case 0x88760014u: return Text(0x7681b0);
    case 0x88760028u: return Text(0x768188);
    case 0x88760037u: return Text(0x768140);
    case 0x8876005au: return Text(0x7680fc);
    case 0x8876005fu: return Text(0x7680a8);
    case 0x88760064u: return Text(0x768064);
    case 0x8876006eu: return Text(0x768030);
    case 0x88760078u: return Text(0x767ffc);
    case 0x88760082u: return Text(0x767fb4);
    case 0x88760091u: return Text(0x767f88);
    case 0x88760096u: return Text(0x767f64);
    case 0x887600a0u: return Text(0x767f14);
    case 0x887600aau: return Text(0x767ef8);
    case 0x887600b4u: return Text(0x767e8c);
    case 0x887600cdu: return Text(0x767e74);
    case 0x887600d2u: return Text(0x767e0c);
    case 0x887600d4u: return Text(0x767db0);
    case 0x887600d7u: return Text(0x767d84);
    case 0x887600dcu: return Text(0x767d1c);
    case 0x887600e1u: return Text(0x767cac);
    case 0x887600e6u: return Text(0x767c7c);
    case 0x887600f0u: return Text(0x767c60);
    case 0x887600fau: return Text(0x767c08);
    case 0x887600ffu: return Text(0x767be8);
    case 0x88760104u: return Text(0x767b88);
    case 0x88760118u: return Text(0x767b18);
    case 0x88760122u: return Text(0x767ab8);
    case 0x88760136u: return Text(0x767a5c);
    case 0x8876013cu: return Text(0x767918);
    case 0x8876013du: return Text(0x767984);
    case 0x88760140u: return Text(0x7679fc);
    case 0x8876014au: return Text(0x7678b0);
    case 0x8876014fu: return Text(0x767838);
    case 0x88760154u: return Text(0x7677d8);
    case 0x8876015eu: return Text(0x767758);
    case 0x88760168u: return Text(0x767708);
    case 0x8876017cu: return Text(0x76829c);
    case 0x8876017eu: return Text(0x7676d4);
    case 0x88760180u: return Text(0x767694);
    case 0x88760183u: return Text(0x767630);
    case 0x88760190u: return Text(0x767600);
    case 0x8876019au: return Text(0x7675b4);
    case 0x887601a4u: return Text(0x76755c);
    case 0x887601aeu: return Text(0x7674f8);
    case 0x887601b8u: return Text(0x7674bc);
    case 0x887601c2u: return Text(0x767414);
    case 0x887601ccu: return Text(0x7673ec);
    case 0x887601d6u: return Text(0x7673bc);
    case 0x887601e0u: return Text(0x767364);
    case 0x887601eau: return Text(0x767334);
    case 0x887601feu: return Text(0x7672fc);
    case 0x88760208u: return Text(0x7672b4);
    case 0x88760219u: return Text(0x767294);
    case 0x8876021cu: return Text(0x767220);
    case 0x88760230u: return Text(0x7671d8);
    case 0x88760231u: return Text(0x766e60);
    case 0x88760232u: return Text(0x767048);
    case 0x88760233u: return Text(0x766cf4);
    case 0x88760234u: return Text(0x7669cc);
    case 0x88760235u: return Text(0x766cd0);
    case 0x88760236u: return Text(0x766994);
    case 0x88760237u: return Text(0x7670a4);
    case 0x88760238u: return Text(0x766da8);
    case 0x88760239u: return Text(0x766c68);
    case 0x8876023au: return Text(0x766f04);
    case 0x8876023bu: return Text(0x766f74);
    case 0x8876023cu: return Text(0x766bb8);
    case 0x8876023du: return Text(0x766b84);
    case 0x8876023eu: return Text(0x767180);
    case 0x8876023fu: return Text(0x766dd8);
    case 0x88760240u: return Text(0x766d5c);
    case 0x88760241u: return Text(0x766a04);
    case 0x88760242u: return Text(0x766be8);
    case 0x88760243u: return Text(0x766df8);
    case 0x88760244u: return Text(0x766b40);
    case 0x88760245u: return Text(0x766ff0);
    case 0x88760246u: return Text(0x766afc);
    case 0x88760247u: return Text(0x767104);
    case 0x88760248u: return Text(0x766a7c);
    case 0x88760249u: return Text(0x767158);
    case 0x8876024au: return Text(0x766d7c);
    case 0x8876024bu: return Text(0x766944);
    case 0x8876024cu: return Text(0x766eb4);
    case 0x8876024du: return Text(0x766a44);
    case 0x887602bcu: return Text(0x76692c);
    case 0x887602bdu: return Text(0x766914);
    case 0x887602c6u: return Text(0x7668f4);
    case 0x887602c7u: return Text(0x7668a0);
    case 0x887602c8u: return Text(0x766884);
    case 0x887602c9u: return Text(0x766864);
    case 0x887602cau: return Text(0x7668dc);
    case 0x887602cbu: return Text(0x7668c0);
    case 0x887602ccu: return Text(0x76684c);
    case 0x887602cdu: return Text(0x76682c);
    case 0x887602d0u: return Text(0x766810);
    case 0x887602d1u: return Text(0x7667bc);
    case 0x887602d2u: return Text(0x76679c);
    case 0x887602d3u: return Text(0x766780);
    case 0x887602d4u: return Text(0x766760);
    case 0x887602d5u: return Text(0x766744);
    case 0x887602d7u: return Text(0x7667dc);
    case 0x887602d8u: return Text(0x7667f4);
    case 0x887602dau: return Text(0x766724);
    case 0x887602dbu: return Text(0x766704);
    case 0x887602dcu: return Text(0x7666e4);
    case 0x887602deu: return Text(0x7666c4);
    case 0x887602e4u: return Text(0x7666a4);
    case 0x887602e5u: return Text(0x766684);
    case 0x887602e6u: return Text(0x766664);
    case 0x887602eeu: return Text(0x766648);
    case 0xb0754000u: return Text(0x766628);
    case 0xb0754001u: return Text(0x766610);
    case 0xb0754002u: return Text(0x7665f8);
    case 0xb0754003u: return Text(0x7665e0);
    case 0xb0754004u: return Text(0x7665bc);
    case 0xb0754005u: return Text(0x766598);
    case 0xb0754006u: return Text(0x766580);
    case 0xb0754007u: return Text(0x766564);
    case 0xb0754008u: return Text(0x76653c);
    case 0xb0754009u: return Text(0x766524);
    case 0xb075400au: return Text(0x766514);
    case 0xb075400bu: return Text(0x7664fc);
    case 0xb075400cu: return Text(0x7664e0);
    case 0xb075400du: return Text(0x7664c4);
    case 0xb075400eu: return Text(0x7664b0);
    case 0xb075400fu: return Text(0x7664a0);
    case 0xb0754010u: return Text(0x76647c);
    case 0xb0754011u: return Text(0x766458);
    case 0xb0754012u: return Text(0x766448);
    case 0xb0754013u: return Text(0x766434);
    case 0xb0754014u: return Text(0x766420);
    case 0xb0754015u: return Text(0x766408);
    case 0xb0754016u: return Text(0x7663f0);
    case 0xb0754017u: return Text(0x7663e4);
    case 0xb0754018u: return Text(0x7663cc);
    default: return Text(0x7663a8);
    }
}
}
