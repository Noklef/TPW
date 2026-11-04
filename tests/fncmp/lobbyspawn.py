# The lobby's flyers spawned and stepped (src/game/frontend/lobbyscene: 0x5da020) against the original: LobbyFlight_Ctor (0x5d9830), RideUser_Acquire (0x5d8130) and LobbyFlyer_Step (0x5d9b50) are recorders, each with a
# bench of its own. The count below the limit or not, the frame's seed at +0x34 giving another round or not, the block failing with the set-up flag clear (or set: a fault in both on the null flyer, rarely), 0 to 3 flyers listed.
# usage: lobbyspawn.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, put, set_return  # noqa: E402

CTOR, ACQUIRE, STEP = 13, 14, 15
B = Bench([(0x5da020, 0x5da114), (0x67a400, 0x690000)], hooks={0x5d9830: _recorder(CTOR, 7), 0x5d8130: _recorder(ACQUIRE, 0), 0x5d9b50: _recorder(STEP, 0)})
SCENE, OWNER, NODES, FLYER, FRAME = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400


def next_seed(seed):
    x = seed * 0x19660d + 0x3c6ef35f & 0xffffffff
    return (x >> 13 | x << 19) & 0xffffffff


def case(r):
    log = log_region()
    set_return(log, CTOR, FLYER)
    count = r.choice([0, 0, 1, 3, r.getrandbits(32)])
    limit = r.choice([count, count + 1, count + 1, count + 2, count + 5, count - 1, 0, r.getrandbits(32)]) & 0xffffffff
    fail = r.choice([0] * 9 + [1])
    scene = bytearray(r.bytes(0x60))
    put(scene, 4, '3I', r.choice([0, 0, r.getrandbits(32)] if not fail else [0] * 9 + [1]), limit, count)
    put(scene, 0x58, 'I', OWNER)
    n = r.choice([0, 1, 2, 3])
    nodes = bytearray(r.bytes(0x20))
    for i in range(n):
        put(nodes, 8 * i, '2I', RAM + 0x500 + 4 * i, NODES + 8 * (i + 1) if i + 1 < n else 0)
    seed = r.getrandbits(32)
    while r.random() < 0.6 and next_seed(seed) & 7 != 1:   # (often stopping at once)
        seed = r.getrandbits(32)
    frame = bytearray(r.bytes(0x40))
    put(frame, 0x34, 'I', seed)
    flyer = bytearray(r.bytes(0x10))
    put(flyer, 0xc, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    regions = {LOG: log, SCENE: scene, OWNER: struct.pack('<II', r.getrandbits(32), NODES if n else 0), NODES: nodes, FLYER: flyer, FRAME: frame, 0xf82884: struct.pack('<I', FRAME), HEAP: heap(fail=fail)}
    return Call(0x5da020, [SCENE], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
