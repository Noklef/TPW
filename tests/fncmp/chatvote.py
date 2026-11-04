# The online chat's frame and goto callback (src/game/online/chatevents: ChatAvatar_Update 0x4e6170, ChatAvatar_GotoFtpDone 0x4e5230) on chatcommand.py's world (GotoDone's).
# Update: the queue empty (chatqueue.py has it full), the vote +0xec4 and the byte +0xec0 set or not; Vote::Release and ~Vote recorders in their import slots; Window7cc8bc_SetC3129 a
# recorder (dialogbuttons.py has it). Release cannot clear +0xec4 here (a fill goes through stack arguments only, and it has none), so the check that follows it sees the vote.
# GotoFtpDone: failed zero or not; the online chat [0x7cddec] the avatar.
# usage: chatvote.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Update,Goto)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, put, recorder, set_return, strings  # noqa: E402
import chatcommand as cc  # noqa: E402
from chatcommand import AVATAR  # noqa: E402

B = Bench(cc.RANGES + [(0x4e5230, 0x4e525b), (0x4e6170, 0x4e61e8), (0x4e6560, 0x4e6785), (0x46cf10, 0x46cf7d), (0x5508b0, 0x5508ba), (0x5c8b90, 0x5c8be0)],
          hooks={**cc.HOOKS, 0x4bd450: _recorder(12, 1, cdecl=True)})
VOTE = RAM + 0x8000
ENTER, LEAVE, DEBUG, RELEASE, DTOR = 0, 5, 23, 30, 31
OPS = [k for k in os.environ.get('OPS', 'Update,Update,Goto').split(',') if k]


def case(r):
    op = r.deal(OPS)
    regions = cc.run_case(r, 'GotoDone').regions
    log, avatar = regions[LOG], regions[AVATAR]
    table = regions[IMPORT_TABLE] = bytearray(regions[IMPORT_TABLE]) + bytes(8)
    put(table, 0x6fd140 - IMPORT_TABLE, 'I', recorder(ENTER, 1, com=True))
    put(table, 0x6fd130 - IMPORT_TABLE, 'I', recorder(LEAVE, 1, com=True))
    put(table, 0x6fd604 - IMPORT_TABLE, 'I', recorder(RELEASE, 0))
    put(table, 0x6fd608 - IMPORT_TABLE, 'I', recorder(DTOR, 0))
    strings(log, DEBUG, 0)   # (the text's address: the driver has not the exe's strings)
    set_return(log, DEBUG, r.getrandbits(32))
    regions[0x7cddec] = struct.pack('<I', AVATAR)
    regions[0x879088] = r.bytes(4)
    regions[VOTE] = r.bytes(0x20)
    put(avatar, 0xef4, 'I', 0)
    put(avatar, 0xec4, 'I', r.choice([0, VOTE, VOTE, VOTE]))
    avatar[0xec0] = r.choice([0, 1, 1, 0xff])
    if op == 'Update':
        return Call(0x4e6170, [AVATAR], regions, conv='t', ret='v')
    return Call(0x4e5230, [r.choice([0, 0, 1, 0x100, 0x80000000, r.getrandbits(32)])], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
