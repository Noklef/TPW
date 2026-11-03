# A visitor whose queue was edited under it (src/game/world/person: Visitor_QueueEdited, 0x501390) against the original: objectmsg.py's queue
# (Remove's environment) and visitorleave.py's Visitor_SendAway fixture on the same person, its object the thing slot 13. The limit +0x40 small
# or wrapping when multiplied by 4 against the person's place, which is -1 (past every limit) when the walk does not find it; state 0xe or not;
# a handle dividing by 3 or not (the thought 0xd). Thoughts_Show's and Visitor_SetState's calls are recorders, as thoughtshow.py and visitorleave.py.
# usage: visitorqueueedited.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import visitorleave  # noqa: E402
from fncmp import LOG, RAM, Bench, _recorder, log_region, put  # noqa: E402
from thingenv import POOL  # noqa: E402

POINTER, WORLD = RAM + 0x42000, RAM + 0x42100

B = Bench([(0x501390, 0x5014d2), (0x5012f0, 0x50138d), (0x4dd4e0, 0x4dd510), (0x4ddd20, 0x4ddf50), (0x4ddf50, 0x4de040), (0x4e0a70, 0x4e0ab0),
           (0x4e2670, 0x4e2720), (0x4faa00, 0x4faa70), (0x4754e0, 0x475550), (0x4fea70, 0x4feb20), (0x502430, 0x502460), (0x50b350, 0x50b370),
           (0x50be80, 0x50c075), (0x50c0d0, 0x50c1a0), (0x519610, 0x519660), (0x51bfc0, 0x51c060), (0x55a070, 0x55a110), (0x55a390, 0x55a3f0),
           (0x5da3c0, 0x5da3c1)],
          hooks={0x501db0: _recorder(26, 1), 0x550ca0: _recorder(0, 0), 0x67a7d0: _recorder(1, 1, cdecl=True), 0x475550: _recorder(2, 1, cdecl=True),
                 0x475a10: _recorder(3, 7, cdecl=True)})


def case(r):
    log = log_region()
    c = visitorleave.quit_case(r, log)
    away = visitorleave.away_case(r, log)
    v = away.regions.pop(visitorleave.VISITOR)
    c.regions.update(away.regions)
    c.regions[LOG] = log
    t = c.args[0] - POOL
    pool = c.regions[POOL]
    pool[t + 0x30:t + 0x1dc] = v[0x30:0x1dc]   # (the ring, the thoughts' words, the happiness)
    pool[t + 0xc:t + 0x10] = v[0xc:0x10]
    put(pool, t + 0x1dc, 'H', 13)   # (always the object: another thing's random queue head may be a slot that is empty, a walk that does not end in both)
    pos = struct.unpack_from('<I', pool, t + 0x30 + 0x88)[0]
    if r.random() < 0.3:   # (the ring's last pair pushed already)
        put(pool, t + 0x30 + (pos * 4 if pos else 0x80), 'HH', 6, 13)
    h, chain, slots = struct.unpack_from('<H', c.regions[visitorleave.OBJ], 0x3c)[0], [], c.regions[0x7cfb90]
    while h:
        chain.append(struct.unpack_from('<I', slots, 20 * h)[0] - POOL)
        h = struct.unpack_from('<H', pool, chain[-1] + 0x228)[0]
    if t not in chain:   # (random links of a person outside the queue fault in both, at once)
        put(pool, t + 0x228, 'HH', 0, 0)
    put(pool, t + 0x220, 'i', r.choice([0xe, 0xe, 11, 12, 13, 8, 3, 15]))
    put(c.regions[visitorleave.OBJ], 0x40, 'I', r.choice([0, 0, 1, 1, 1, 2, 0x40000000, 0x40000001, 0xc0000000, r.getrandbits(32)]))
    put(pool, t + 0xb4, 'I', r.choice([0, 0, r.randrange(1, 64)]))   # (the thoughts' bubble script)
    c.regions.update({0x790ab0: bytes([r.choice([0, 0, 0, 0, 1, 2, 4, 0x10, 0x20, 0xe9])]), 0xfb34aa: b'\x01', 0xfb3b7c: struct.pack('<I', r.choice([0, 1, 1, 2])),
                      0x7cf6ec: struct.pack('<I', POINTER), POINTER: struct.pack('<I', WORLD - 0x1da70c), WORLD: r.bytes(4)})
    c.address = 0x501390
    return c


if __name__ == '__main__':
    B.main(case)
