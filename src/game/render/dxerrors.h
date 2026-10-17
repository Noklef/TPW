// The DirectDraw / Direct3D result messages (0x56a770): reimplementation.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x56a770 (cdecl; an HRESULT; name unconfirmed): the message for a DirectDraw / Direct3D result (and the renderer's own 0xb0754016-0xb0754018,
// "DirectX6 not found." and so on), a string in the exe from 0x7663a8 on; for any other code "Mommy! Where's fluffy?" (0x7663a8). The original
// finds it by a tree of compares and three jump tables; the codes are the 142 the listing reaches.
const char *__cdecl DxResult_Message(u32 result);
}
