// The online dialog's five chat rooms' head counts (names invented, unconfirmed): the room ids [0x7cdae0], the counts [0x7cdac8] (5 dwords each), the Chat [0x7cdb14] asked for each room's info, the answers counted in [0x7cdb18], the flag [0x7cdb1c] set when the counts are ready to show, and the flag
// [0x7cdb10] asking for a round. Reimplementation of 0x4c4920 and 0x4c4bb0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4c4920 (stdcall (type, unused, the event's arguments: a vector of 0x24-byte Arguments), the Chat's callback): a type -5 or -4 sets the flag [0x7cdb1c]. A type 0x64 by its first Argument's kind (+0x20): 7 (the fourth's string +0x6c through atoi as a room id: that room's count
// the fifth's kind +0xb0); 0x2c (with over one Argument, the second's string +0x24 as a room id: that room's count 0). Then, any kind, the answers counted, the flag set at 5.
void __stdcall RoomCounts_ChatEvent(s32 type, u32 unused, const u8 *arguments);

// 0x4c4bb0 (cdecl): with the flag [0x7cdb10] and the session's (Online_Instance) city's +0x28 and rooms (+0x2c's +0xc) over 0: the flag cleared, up to 5 rooms' ids (each +8) with their counts 0, a new Chat (tried twice, memory or the constructor answering 0: the Chat is 0) connected
// (Chat::Connect) to the host [0x7a1504] and port (a signed word, [0x7a1552]) with RoomCounts_ChatEvent and Global7cc53c_Pick7cc4f0; refused: OutputDebugString of 0x759924. Connected: both counters 0, and Chat::RoomInfo for each room's id as "%d". Then, whatever came before,
// with the flag [0x7cdb1c] set: Chat::Release, each count handed to the dialog [0x7cdb0c]'s (if any) child 0x16007's child 0x16008-0x1600c by its slot 7, and the flag cleared.
void __cdecl RoomCounts_Update();
}
