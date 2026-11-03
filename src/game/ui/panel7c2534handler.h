// The game's menu, the rows object at [0x7c2534]: the handler of its rows. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x48b6a0 (cdecl; window, message, a, b; name invented, unconfirmed). Closing means: unless [0x786ba4] or the byte [0x7c2518] is set, [0x786ba4] = 1, 0x409300 on 0x786b68 and 0x662420; WindowAt10_Hide; the object told (WindowAt10_Message4IfAny) and freed; [0x7c2534] = 0.
// Message 0x1000b: a = 0x1b, or a key the key map [0x78721c] does not know (the byte +2 of b its modifiers): closes (the object through its scalar destructor), 0 returned. Message 0x100001 by a: 0, 0x10, 0x15 closes, 0 returned; 1 / 0x16 then
// LoadDialog_Create (0 / 1 pushed, not read); 2 then SaveDialog_Create; 5, with no activity in the session (Obj_HasActivity), then the options dialog; 6, 0x13, 0x18 Global879088_Set_5508b0(2) ([0xf8288c] = 1 first for 0x13) and closes, 1 returned;
// 7 the session's 0x5b5e40 and closes, 1 returned; 3, 0x17, 4, 8, 0x14, 0x19 the question box 0xb, 0xb, 0xdd, 9, 9, 9 with the callback 0x48b5b0, 0x48b620, 0x48b400, 0x48b310 (two buttons). Any other, and after a box: WindowAt10_Handler, its answer returned.
s32 __cdecl Panel7c2534_Handler(u8 *window, u32 message, u32 a, u32 b);
}

// The handler's calls (the bench's recorders; their own benches have them). 0x409300, 0x47f020 and 0x5b5e40 are agent B's, unported, 0x662420 unported.
struct Panel7c2534HandlerEnv {
    void(__fastcall *controls)(u8 *object);                                     // 0x409300
    u32(__cdecl *f662420)();                                                    // 0x662420
    void(__fastcall *hide)(u8 *object);                                         // WindowAt10_Hide
    s32(__fastcall *tell)(u8 *object);                                          // WindowAt10_Message4IfAny
    u8 *(__fastcall *destroy)(u8 *object, void *, u32 flags);                   // WindowAt10_ScalarDtor
    void(__cdecl *load)(u32 unused);                                            // LoadDialog_Create
    void(__cdecl *save)(u32 unused);                                            // SaveDialog_Create
    void(__cdecl *options)();                                                   // Dialog7cb2fc_Open
    u16 *(__cdecl *text)(u32 index);                                            // Language_WideText
    void(__cdecl *box)(const u8 *record, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g);   // 0x47f020
    u8 *(__cdecl *online)();                                                    // Online_Instance
    u32(__fastcall *hasActivity)(const u8 *object);                             // Obj_HasActivity
    void(__fastcall *leave)(u8 *online);                                        // 0x5b5e40
    u32(__cdecl *setGlobal)(u32 value);                                         // Global879088_Set_5508b0
    s32(__fastcall *findKey)(const u8 *map, void *, u32 key, u32 modifiers);    // KeyMap_FindKey
    s32(__cdecl *handler)(u8 *window, u32 message, u32 a, u32 b);               // WindowAt10_Handler
};
void Panel7c2534Handler_SetEnv(const Panel7c2534HandlerEnv *env);
