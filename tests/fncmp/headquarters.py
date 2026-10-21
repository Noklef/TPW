# The mechanics' HQ (src/game/world/headquarters: 0x4dadc0) against the original. HandleMessage: an event (slot 0 a recorder answering 7
# mostly) for a mechanic (+8) among 1-6 things in the game table 0x7cfb90 (their +0xe, +0x64 0 or not, +0x19c 0-8, +0x210 the next in a
# chain of them or 0, +0x218 the mechanic, 0 or another); the first HQ (the world's +0x1da73e) one of them or 0. The rest is advisor.py's
# PostId set-up (the world [0x80239c] as its time's, the advisor the table entry of its +0x1da71c), the definitions of the messages
# 0x44-0x46 its own now and then and their records not yet posted. The stack message's unwritten bytes are masked in the slots and the
# copy at +0x16d0 always (advisor.py leaves the copy unmasked for its id 0x15f, whose record it is; here only 0x44-0x46 are posted).
# usage: headquarters.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=HandleMessage)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, Mask, _cdecl_log, put, set_return, vtable  # noqa: E402
from advisor import ADVISOR, NOW, queue  # noqa: E402

CALLS = {'HandleMessage': 0x4dadc0}
B = Bench([(0x5997b0, 0x5997f8), (0x429fe0, 0x42a010), (0x44b2e0, 0x44b3d0), (0x474070, 0x4740e0), (0x5442e0, 0x544340), (0x5989c0, 0x598ae0),
           (0x5ed580, 0x5ed5b0), (0x67a400, 0x6902c0), (0x59a940, 0x59aa63), (0x59abc0, 0x59ae1c), (0x59c2b0, 0x59c680), (0x41a980, 0x41a986),
           (0x41a990, 0x41a9a7), (0x5da3c0, 0x5da3c1), (0x59afc0, 0x59b049), (0x409cd0, 0x409cdd), (0x59b590, 0x59b611), (0x59bf20, 0x59c093),
           (0x550ca0, 0x550d90), (0x5d1750, 0x5d1751), (0x5194d0, 0x51950d), (0x4dadc0, 0x4db011), (0x5064f0, 0x506523), (0x4e03d0, 0x4e0400)],
          hooks={0x67a7d0: _cdecl_log})
EVENT, EVT, THINGS = RAM + 0x20000, RAM + 0x20100, RAM + 0x21000
THING = 0x220


def case(r):
    r.deal(OPS)
    c = queue(r, 'PostId')
    regions = c.regions
    for code in (0x44, 0x45, 0x46):
        if r.random() < 0.6:
            put(regions[0x76e300], 0x38 * code + 4, 'I', code)
        if r.random() < 0.6:
            put(regions[ADVISOR], 0xe0 + 0x10 * code, 'I', 0)
    n = r.randrange(1, 7)
    h = n + 1   # (the advisor's entry)
    table = bytearray(r.bytes(20 * (h + 1)))
    put(table, 20 * h, 'I', ADVISOR)
    order = r.sample(range(1, n + 1), n)
    links = {a: (order[i + 1] if i + 1 < len(order) else 0) for i, a in enumerate(order)}
    who = r.choice(range(1, n + 1))
    for k in range(1, n + 1):
        t = bytearray(r.bytes(THING))
        put(t, 0x64, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
        put(t, 0x19c, 'I', r.choice([0, 1, 2, 3, 4, 5, 6, 7, 8, 3, 4]))
        put(t, 0x210, 'H', links[k] if r.random() < 0.9 else 0)
        put(t, 0x218, 'H', r.choice([who, 0, 0, r.randrange(1, n + 1), r.getrandbits(16)]))
        regions[THINGS + THING * k] = t
        put(table, 20 * k, 'I', THINGS + THING * k)
    for k in [k for k in regions if 0x7cfb90 <= k < 0x7cfb90 + 0x1000]:   # (PostId's own entry for the advisor)
        del regions[k]
    regions[0x7cfb90] = table
    world = bytearray(r.bytes(0x40))
    world[0:4] = regions[NOW][0:4]
    put(world, 0x10, 'H', h)
    put(world, 0x32, 'H', r.choice([order[0], order[0], r.randrange(1, n + 1), 0]))
    regions[NOW] = world
    regions[EVT] = vtable({0: 0})
    regions[EVENT] = put(put(bytearray(r.bytes(0x10)), 0, 'I', EVT), 8, 'H', who)
    set_return(regions[LOG], 0, r.choice([7, 7, 7, 7, 6, 0, r.getrandbits(32)]))
    offsets = sorted([0x16d0 + k for k in (2, 3, 0x17)] + [0x14 + 0x18 * s + k for s in range(8) for k in (2, 3, 0x17)])
    mask = Mask(ADVISOR, offsets, "the stack message's bytes +2, +3, +0x17, never written (0x4dadc0), copied into a slot or +0x16d0")
    return Call(CALLS['HandleMessage'], [r.getrandbits(32), EVENT], regions, conv='t', masks=[mask])


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
