# The terrain tile cache's .lnd read (src/game/terrain/texcache: 0x5723f0) against the original: the land's calls and the error list's recorders, Lnd_Read's answer with its low byte 0 or not and any upper bytes, the options 0x8bd550 any.
# usage: terrainreadlnd.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, set_return  # noqa: E402

SIZE, READ, PUSH = 8, 9, 10
B = Bench([(0x5723f0, 0x57243a)], hooks={0x55f720: _recorder(SIZE, 1), 0x560ff0: _recorder(READ, 1), 0x56af20: _recorder(PUSH, 2)})
CACHE, NAME = RAM, RAM + 0x100


def case(r):
    log = log_region()
    set_return(log, READ, r.choice([0, 0, 1, 1, 0x100, 0xffffff00, 0x80, r.getrandbits(32)]))
    return Call(0x5723f0, [CACHE, NAME], {LOG: log, CACHE: r.bytes(8), NAME: b'base.lnd\0', 0x8bd550: r.bytes(4)}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
