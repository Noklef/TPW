// The DirectDraw / Direct3D result messages (0x56a770): reimplementation.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x56a770 (cdecl; an HRESULT; name unconfirmed): the message for a DirectDraw / Direct3D result (and the renderer's own 0xb0754016-0xb0754018,
// "DirectX6 not found." and so on), a string in the exe from 0x7663a8 on; for any other code "Mommy! Where's fluffy?" (0x7663a8). The original
// finds it by a tree of compares and three jump tables; the codes are the 142 the listing reaches.
const char *__cdecl DxResult_Message(u32 result);
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[24], gText_DxResult_Message_7663a8, 0x007663a8, 0x18);
GAME_DATA(char[12], gText_DxResult_Message_7663c0, 0x007663c0, 0xc);
GAME_DATA(char[24], gText_DxResult_Message_7663cc, 0x007663cc, 0x18);
GAME_DATA(char[12], gText_DxResult_Message_7663e4, 0x007663e4, 0xc);
GAME_DATA(char[24], gText_DxResult_Message_7663f0, 0x007663f0, 0x18);
GAME_DATA(char[24], gText_DxResult_Message_766408, 0x00766408, 0x18);
GAME_DATA(char[20], gText_DxResult_Message_766420, 0x00766420, 0x14);
GAME_DATA(char[20], gText_DxResult_Message_766434, 0x00766434, 0x14);
GAME_DATA(char[16], gText_DxResult_Message_766448, 0x00766448, 0x10);
GAME_DATA(char[36], gText_DxResult_Message_766458, 0x00766458, 0x24);
GAME_DATA(char[36], gText_DxResult_Message_76647c, 0x0076647c, 0x24);
GAME_DATA(char[16], gText_DxResult_Message_7664a0, 0x007664a0, 0x10);
GAME_DATA(char[20], gText_DxResult_Message_7664b0, 0x007664b0, 0x14);
GAME_DATA(char[28], gText_DxResult_Message_7664c4, 0x007664c4, 0x1c);
GAME_DATA(char[28], gText_DxResult_Message_7664e0, 0x007664e0, 0x1c);
GAME_DATA(char[24], gText_DxResult_Message_7664fc, 0x007664fc, 0x18);
GAME_DATA(char[16], gText_DxResult_Message_766514, 0x00766514, 0x10);
GAME_DATA(char[24], gText_DxResult_Message_766524, 0x00766524, 0x18);
GAME_DATA(char[40], gText_DxResult_Message_76653c, 0x0076653c, 0x28);
GAME_DATA(char[28], gText_DxResult_Message_766564, 0x00766564, 0x1c);
GAME_DATA(char[24], gText_DxResult_Message_766580, 0x00766580, 0x18);
GAME_DATA(char[36], gText_DxResult_Message_766598, 0x00766598, 0x24);
GAME_DATA(char[36], gText_DxResult_Message_7665bc, 0x007665bc, 0x24);
GAME_DATA(char[24], gText_DxResult_Message_7665e0, 0x007665e0, 0x18);
GAME_DATA(char[24], gText_DxResult_Message_7665f8, 0x007665f8, 0x18);
GAME_DATA(char[24], gText_DxResult_Message_766610, 0x00766610, 0x18);
GAME_DATA(char[32], gText_DxResult_Message_766628, 0x00766628, 0x20);
GAME_DATA(char[28], gText_DxResult_Message_766648, 0x00766648, 0x1c);
GAME_DATA(char[32], gText_DxResult_Message_766664, 0x00766664, 0x20);
GAME_DATA(char[32], gText_DxResult_Message_766684, 0x00766684, 0x20);
GAME_DATA(char[32], gText_DxResult_Message_7666a4, 0x007666a4, 0x20);
GAME_DATA(char[32], gText_DxResult_Message_7666c4, 0x007666c4, 0x20);
GAME_DATA(char[32], gText_DxResult_Message_7666e4, 0x007666e4, 0x20);
GAME_DATA(char[32], gText_DxResult_Message_766704, 0x00766704, 0x20);
GAME_DATA(char[32], gText_DxResult_Message_766724, 0x00766724, 0x20);
GAME_DATA(char[28], gText_DxResult_Message_766744, 0x00766744, 0x1c);
GAME_DATA(char[32], gText_DxResult_Message_766760, 0x00766760, 0x20);
GAME_DATA(char[28], gText_DxResult_Message_766780, 0x00766780, 0x1c);
GAME_DATA(char[32], gText_DxResult_Message_76679c, 0x0076679c, 0x20);
GAME_DATA(char[32], gText_DxResult_Message_7667bc, 0x007667bc, 0x20);
GAME_DATA(char[24], gText_DxResult_Message_7667dc, 0x007667dc, 0x18);
GAME_DATA(char[28], gText_DxResult_Message_7667f4, 0x007667f4, 0x1c);
GAME_DATA(char[28], gText_DxResult_Message_766810, 0x00766810, 0x1c);
GAME_DATA(char[32], gText_DxResult_Message_76682c, 0x0076682c, 0x20);
GAME_DATA(char[24], gText_DxResult_Message_76684c, 0x0076684c, 0x18);
GAME_DATA(char[32], gText_DxResult_Message_766864, 0x00766864, 0x20);
GAME_DATA(char[28], gText_DxResult_Message_766884, 0x00766884, 0x1c);
GAME_DATA(char[32], gText_DxResult_Message_7668a0, 0x007668a0, 0x20);
GAME_DATA(char[28], gText_DxResult_Message_7668c0, 0x007668c0, 0x1c);
GAME_DATA(char[24], gText_DxResult_Message_7668dc, 0x007668dc, 0x18);
GAME_DATA(char[32], gText_DxResult_Message_7668f4, 0x007668f4, 0x20);
GAME_DATA(char[24], gText_DxResult_Message_766914, 0x00766914, 0x18);
GAME_DATA(char[24], gText_DxResult_Message_76692c, 0x0076692c, 0x18);
GAME_DATA(char[80], gText_DxResult_Message_766944, 0x00766944, 0x50);
GAME_DATA(char[56], gText_DxResult_Message_766994, 0x00766994, 0x38);
GAME_DATA(char[56], gText_DxResult_Message_7669cc, 0x007669cc, 0x38);
GAME_DATA(char[64], gText_DxResult_Message_766a04, 0x00766a04, 0x40);
GAME_DATA(char[56], gText_DxResult_Message_766a44, 0x00766a44, 0x38);
GAME_DATA(char[128], gText_DxResult_Message_766a7c, 0x00766a7c, 0x80);
GAME_DATA(char[68], gText_DxResult_Message_766afc, 0x00766afc, 0x44);
GAME_DATA(char[68], gText_DxResult_Message_766b40, 0x00766b40, 0x44);
GAME_DATA(char[52], gText_DxResult_Message_766b84, 0x00766b84, 0x34);
GAME_DATA(char[48], gText_DxResult_Message_766bb8, 0x00766bb8, 0x30);
GAME_DATA(char[128], gText_DxResult_Message_766be8, 0x00766be8, 0x80);
GAME_DATA(char[104], gText_DxResult_Message_766c68, 0x00766c68, 0x68);
GAME_DATA(char[36], gText_DxResult_Message_766cd0, 0x00766cd0, 0x24);
GAME_DATA(char[104], gText_DxResult_Message_766cf4, 0x00766cf4, 0x68);
GAME_DATA(char[32], gText_DxResult_Message_766d5c, 0x00766d5c, 0x20);
GAME_DATA(char[44], gText_DxResult_Message_766d7c, 0x00766d7c, 0x2c);
GAME_DATA(char[48], gText_DxResult_Message_766da8, 0x00766da8, 0x30);
GAME_DATA(char[32], gText_DxResult_Message_766dd8, 0x00766dd8, 0x20);
GAME_DATA(char[104], gText_DxResult_Message_766df8, 0x00766df8, 0x68);
GAME_DATA(char[84], gText_DxResult_Message_766e60, 0x00766e60, 0x54);
GAME_DATA(char[80], gText_DxResult_Message_766eb4, 0x00766eb4, 0x50);
GAME_DATA(char[124], gText_DxResult_Message_766f74, 0x00766f74, 0x7c);
GAME_DATA(char[88], gText_DxResult_Message_766ff0, 0x00766ff0, 0x58);
GAME_DATA(char[92], gText_DxResult_Message_767048, 0x00767048, 0x5c);
GAME_DATA(char[96], gText_DxResult_Message_7670a4, 0x007670a4, 0x60);
GAME_DATA(char[84], gText_DxResult_Message_767104, 0x00767104, 0x54);
GAME_DATA(char[40], gText_DxResult_Message_767158, 0x00767158, 0x28);
GAME_DATA(char[88], gText_DxResult_Message_767180, 0x00767180, 0x58);
GAME_DATA(char[72], gText_DxResult_Message_7671d8, 0x007671d8, 0x48);
GAME_DATA(char[116], gText_DxResult_Message_767220, 0x00767220, 0x74);
GAME_DATA(char[32], gText_DxResult_Message_767294, 0x00767294, 0x20);
GAME_DATA(char[72], gText_DxResult_Message_7672b4, 0x007672b4, 0x48);
GAME_DATA(char[56], gText_DxResult_Message_7672fc, 0x007672fc, 0x38);
GAME_DATA(char[48], gText_DxResult_Message_767334, 0x00767334, 0x30);
GAME_DATA(char[88], gText_DxResult_Message_767364, 0x00767364, 0x58);
GAME_DATA(char[48], gText_DxResult_Message_7673bc, 0x007673bc, 0x30);
GAME_DATA(char[40], gText_DxResult_Message_7673ec, 0x007673ec, 0x28);
GAME_DATA(char[168], gText_DxResult_Message_767414, 0x00767414, 0xa8);
GAME_DATA(char[60], gText_DxResult_Message_7674bc, 0x007674bc, 0x3c);
GAME_DATA(char[100], gText_DxResult_Message_7674f8, 0x007674f8, 0x64);
GAME_DATA(char[88], gText_DxResult_Message_76755c, 0x0076755c, 0x58);
GAME_DATA(char[76], gText_DxResult_Message_7675b4, 0x007675b4, 0x4c);
GAME_DATA(char[48], gText_DxResult_Message_767600, 0x00767600, 0x30);
GAME_DATA(char[100], gText_DxResult_Message_767630, 0x00767630, 0x64);
GAME_DATA(char[64], gText_DxResult_Message_767694, 0x00767694, 0x40);
GAME_DATA(char[52], gText_DxResult_Message_7676d4, 0x007676d4, 0x34);
GAME_DATA(char[80], gText_DxResult_Message_767708, 0x00767708, 0x50);
GAME_DATA(char[128], gText_DxResult_Message_767758, 0x00767758, 0x80);
GAME_DATA(char[96], gText_DxResult_Message_7677d8, 0x007677d8, 0x60);
GAME_DATA(char[120], gText_DxResult_Message_767838, 0x00767838, 0x78);
GAME_DATA(char[104], gText_DxResult_Message_7678b0, 0x007678b0, 0x68);
GAME_DATA(char[108], gText_DxResult_Message_767918, 0x00767918, 0x6c);
GAME_DATA(char[120], gText_DxResult_Message_767984, 0x00767984, 0x78);
GAME_DATA(char[96], gText_DxResult_Message_7679fc, 0x007679fc, 0x60);
GAME_DATA(char[92], gText_DxResult_Message_767a5c, 0x00767a5c, 0x5c);
GAME_DATA(char[96], gText_DxResult_Message_767ab8, 0x00767ab8, 0x60);
GAME_DATA(char[112], gText_DxResult_Message_767b18, 0x00767b18, 0x70);
GAME_DATA(char[96], gText_DxResult_Message_767b88, 0x00767b88, 0x60);
GAME_DATA(char[32], gText_DxResult_Message_767be8, 0x00767be8, 0x20);
GAME_DATA(char[88], gText_DxResult_Message_767c08, 0x00767c08, 0x58);
GAME_DATA(char[28], gText_DxResult_Message_767c60, 0x00767c60, 0x1c);
GAME_DATA(char[48], gText_DxResult_Message_767c7c, 0x00767c7c, 0x30);
GAME_DATA(char[112], gText_DxResult_Message_767cac, 0x00767cac, 0x70);
GAME_DATA(char[104], gText_DxResult_Message_767d1c, 0x00767d1c, 0x68);
GAME_DATA(char[44], gText_DxResult_Message_767d84, 0x00767d84, 0x2c);
GAME_DATA(char[92], gText_DxResult_Message_767db0, 0x00767db0, 0x5c);
GAME_DATA(char[104], gText_DxResult_Message_767e0c, 0x00767e0c, 0x68);
GAME_DATA(char[24], gText_DxResult_Message_767e74, 0x00767e74, 0x18);
GAME_DATA(char[108], gText_DxResult_Message_767e8c, 0x00767e8c, 0x6c);
GAME_DATA(char[28], gText_DxResult_Message_767ef8, 0x00767ef8, 0x1c);
GAME_DATA(char[80], gText_DxResult_Message_767f14, 0x00767f14, 0x50);
GAME_DATA(char[36], gText_DxResult_Message_767f64, 0x00767f64, 0x24);
GAME_DATA(char[44], gText_DxResult_Message_767f88, 0x00767f88, 0x2c);
GAME_DATA(char[72], gText_DxResult_Message_767fb4, 0x00767fb4, 0x48);
GAME_DATA(char[52], gText_DxResult_Message_767ffc, 0x00767ffc, 0x34);
GAME_DATA(char[52], gText_DxResult_Message_768030, 0x00768030, 0x34);
GAME_DATA(char[68], gText_DxResult_Message_768064, 0x00768064, 0x44);
GAME_DATA(char[84], gText_DxResult_Message_7680a8, 0x007680a8, 0x54);
GAME_DATA(char[68], gText_DxResult_Message_7680fc, 0x007680fc, 0x44);
GAME_DATA(char[72], gText_DxResult_Message_768140, 0x00768140, 0x48);
GAME_DATA(char[40], gText_DxResult_Message_768188, 0x00768188, 0x28);
GAME_DATA(char[64], gText_DxResult_Message_7681b0, 0x007681b0, 0x40);
GAME_DATA(char[60], gText_DxResult_Message_7681f0, 0x007681f0, 0x3c);
GAME_DATA(char[40], gText_DxResult_Message_76822c, 0x0076822c, 0x28);
GAME_DATA(char[72], gText_DxResult_Message_768254, 0x00768254, 0x48);
GAME_DATA(char[68], gText_DxResult_Message_76829c, 0x0076829c, 0x44);
GAME_DATA(char[20], gText_DxResult_Message_7682e0, 0x007682e0, 0x14);
GAME_DATA(char[24], gText_DxResult_Message_7682f4, 0x007682f4, 0x18);
