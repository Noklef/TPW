# The online mouse mode handlers on the cell under the cursor (src/game/online/chatcursor: 0x46c610, 0x46cee0, 0x46cf80) against the originals: the async modifiers (k 1), GameMode_MapClick (k 2), OnlineChat_Instance (k 3), ChatAvatar_GoToCell (k 4)
# and ChatAvatar_MoveToCell (k 5) recorders; any ECX (its high half reaches the cell's dword, sic), any cell id, the avatar calls answering 0 or 1.
# usage: chatcursor.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, _recorder, log_region, set_return  # noqa: E402

B = Bench([(0x46c610, 0x46c64e), (0x46cee0, 0x46cf01), (0x46cf80, 0x46cfa1)],
          hooks={0x4815c0: _recorder(1, 0, cdecl=True), 0x524960: _recorder(2, 4, cdecl=True), 0x46cf10: _recorder(3, 0, cdecl=True), 0x4eac30: _recorder(4, 1), 0x4eab40: _recorder(5, 1)})


def case(r):
    log = log_region()
    set_return(log, 1, r.getrandbits(32))
    set_return(log, 3, r.getrandbits(32))
    set_return(log, 4, r.choice([0, 1]))
    set_return(log, 5, r.choice([0, 1]))
    regions = {LOG: log, 0x7b05cc: struct.pack('<H', r.choice([0, 1, 0x80, 0x81, r.getrandbits(16)]))}
    address, ret = r.choice([(0x46c610, 'v'), (0x46cee0, 'a'), (0x46cf80, 'a')])
    return Call(address, [r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret=ret)


if __name__ == '__main__':
    B.main(case)
