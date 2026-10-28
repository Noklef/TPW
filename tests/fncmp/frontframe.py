# The front-end frame's close (src/game/frontend/frame: 0x5d5cf0 Class7029f0_Close) against the original, over soundcats.py's RelSfx world
# (Sound_ReleaseLobbySfx: the categories, the sound on and started or not, the manager's object's slot 8 a recorder): the frame random bytes,
# its list at +0x18 (a sentinel whose +4 is the first of 0-4 nodes {object, next}) of recorder objects (slot 4), the objects [0xf83784],
# [0xf83778], [0xf83774], [0xf85614] and +0x10 each a recorder object (slot 0 with one argument) or 0, the window +4 viewcontrols.py's (byte
# +4 now and then set, its handler the four-argument logger or 0) or 0, the hook [0xfaa5fc] the logger or 0. The result (+0x14) compared.
# usage: frontframe.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import CDECL_LOG4, RAM, Bench, Call, log_region, put, vtable  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import soundcats  # noqa: E402
import viewcontrols  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

B = Bench(soundcats.merged(soundcats.RANGES + [(0x5d5cf0, 0x5d5d95), (0x658d9f, 0x658e04), (0x659710, 0x659740), (0x65d7bc, 0x65d832),
                                               (0x65f697, 0x65f6e1)]))
FRAME, SENT, NODES, OBJS, OVT, WIN = RAM + 0xd000, RAM + 0xd100, RAM + 0xd200, RAM + 0xd400, RAM + 0xd800, RAM + 0xe000
GLOBALS = [0xf83784, 0xf83778, 0xf83774, 0xf85614]


def case(r):
    log = log_region()
    regions = soundcats.release_case(r, 'RelSfx', log).regions
    objs = bytearray(r.bytes(0x20 * 10))
    for k in range(10):
        put(objs, 0x20 * k, 'I', OVT)
    regions.update({OBJS: objs, OVT: vtable({0: 1})})
    n = r.choice([0, 1, 1, 2, 3, 4])
    nodes = bytearray(r.bytes(8 * max(n, 1)))
    for k in range(n):
        put(nodes, 8 * k, 'II', OBJS + 0x20 * k, NODES + 8 * (k + 1) if k + 1 < n else 0)
    sent = bytearray(r.bytes(0x10))
    put(sent, 4, 'I', NODES if n else 0)
    frame = bytearray(r.bytes(0x40))
    put(frame, 0x18, 'I', SENT)
    put(frame, 0x10, 'I', r.choice([OBJS + 0x20 * 9, OBJS + 0x20 * 9, 0]))
    put(frame, 4, 'I', r.choice([WIN, WIN, WIN, 0]))
    for k, g in enumerate(GLOBALS):
        regions[g] = struct.pack('<I', r.choice([OBJS + 0x20 * (5 + k), OBJS + 0x20 * (5 + k), 0]))
    regions.update({FRAME: frame, SENT: sent, NODES: nodes, WIN: viewcontrols.window(r),
                    0xfaa5fc: struct.pack('<I', r.choice([0, 0, CDECL_LOG4]))})
    return Call(0x5d5cf0, [FRAME], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
