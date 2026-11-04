# The mouse modes' handlers that only call on, against the originals: 0x46c5d0 ChatCursor_Press, 0x46c660 ChatCursor_Preview, 0x46c6b0
# ChatCursor_ShowHelp (src/game/online/chatcursor), 0x46cfd0 MouseMode6feaa0_Press, 0x46cfe0 MouseMode6feaa0_Release (src/game/world/mousemode).
# usage: chatcursorcalls.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, _recorder, log_region, set_return  # noqa: E402

MODIFIERS, PRESS, PREVIEW, CLICK, HELP = 1, 6, 7, 8, 9
B = Bench([(0x46c5d0, 0x46c60e), (0x46c660, 0x46c691), (0x46c6b0, 0x46c6cc), (0x46cfd0, 0x46cfdd), (0x46cfe0, 0x46cfed), (0x5da3c0, 0x5da3c1)],
          hooks={0x4815c0: _recorder(MODIFIERS, 0, cdecl=True), 0x524790: _recorder(PRESS, 4, cdecl=True), 0x5234d0: _recorder(PREVIEW, 3, cdecl=True),
                 0x446300: _recorder(CLICK, 1, cdecl=True), 0x4814f0: _recorder(HELP, 1)})
CALLS = {'press': (0x46c5d0, 2, 'a'), 'preview': (0x46c660, 4, 'a'), 'help': (0x46c6b0, 0, 'v'), 'down': (0x46cfd0, 2, 'a'), 'up': (0x46cfe0, 2, 'a')}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def case(r):
    log = log_region()
    for k in (MODIFIERS, PRESS, PREVIEW, CLICK):
        set_return(log, k, r.getrandbits(32))
    regions = {LOG: log, 0x7b05cc: struct.pack('<H', r.choice([0, 1, 0x80, 0x81, r.getrandbits(16)]))}
    address, args, ret = CALLS[r.deal(OPS)]
    if not args:
        return Call(address, [], regions, ret=ret)
    return Call(address, [r.getrandbits(32)] + [r.getrandbits(32) for _ in range(args)], regions, conv='t', ret=ret)


if __name__ == '__main__':
    B.main(case)
