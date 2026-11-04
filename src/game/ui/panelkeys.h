// A panel screen's message handler: the escape key that nothing up takes opens the panel's menu.
#pragma once

#include "../../../re/types.h"

struct PanelKeysEnv {
    void(__cdecl *open)(u32 flag);  // Panel7c2534_Open (0x48c830)
};

void PanelKeys_SetEnv(const PanelKeysEnv *env);

extern "C" {

// 0x5e41c0 (thiscall (message, a, b), pops 12; class 0x702da0; name invented, unconfirmed): on (0x1000b, 0x1b) the objects of the list +0x18 (nodes {object, next} from +4, the next read first) that answer slot 9 get slot 6;
// when none answers it, Panel7c2534_Open (1). Then the objects that answer slot 9 get slot 5 with the message.
void __fastcall PanelKeys_HandleMessage(u8 *panel, void *, u32 message, u32 a, u32 b);
}
