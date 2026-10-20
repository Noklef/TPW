# A person's sprite placed for a moment (0x4fa030 Person_PlaceSprite, src/game/world/person) against the original: t as person.py's Lerp
# has it (both ranges' ends, NaN, infinities, denormals; the first position is at t - 0.35), the nav's and last positions near or far
# apart; personscripts.py's script list behind the reference +0xc, the slots +0xb4 and +0x210 scripts of it or 0; +0x28 set or not, the
# timer +0x198 about the counter 0x7cdb98 (the square face: dynfaces.py's queue), the byte +2 1 or not; the bob row 0x7cedd4 (0-20 or
# any) and its time 0x7cedd8 about 1.0 (NaN too). CentreOnSprite (0x486860): the same sprite (or none) and the clock [0x8786bc] about
# the last tick's [0x878a1c] (0-300 ms on, behind, or any); the camera's point and its three cleared dwords compared.
# usage: personsprite.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=PlaceSprite,CentreOnSprite)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import Bench, Call, RAM, fb, put  # noqa: E402
from personscripts import script_list  # noqa: E402

B = Bench([(0x475480, 0x4754f0), (0x4d8c50, 0x4d8c70), (0x4f9f00, 0x4fa2b0), (0x4fe900, 0x4fe960), (0x50b350, 0x50b370), (0x510160, 0x510190),
           (0x53c8d0, 0x53c9d0), (0x5da3c0, 0x5da3c1), (0x486860, 0x4868c1), (0x42aab0, 0x42ab1d)])
CALLS = {'PlaceSprite': 0x4fa030, 'CentreOnSprite': 0x486860}
THING = RAM + 0x20000


def word(r):
    return r.choice([0, 1, 4, 0x7fffffff, 0x80000000, 0xffffffff, r.getrandbits(32)])


def case(r):
    op = r.deal(OPS)
    regions = {}
    thing = bytearray(r.bytes(0x240))
    script_list(r, regions, thing)
    for off in (0xb4, 0x210):
        put(thing, off, 'I', r.choice([0, 0, 1, 2, 3, 4]))
    put(thing, 0x28, 'I', r.choice([0, 0, 0, 1, word(r)]))
    thing[2] = r.choice([1, 1, 1, 0, 2, r.getrandbits(8)])
    for at in (0xdc, 0xe0):   # (person.py's Lerp: the nav (+0xd4)'s position, then the last one about it)
        pos = word(r) if r.random() < 0.3 else r.randrange(0x800000)
        put(thing, at, 'I', pos)
        put(thing, 0x190 + at - 0xdc, 'I', r.choice([pos, (pos + r.randrange(-0x2000, 0x2000)) & 0xffffffff, (pos ^ 0x80000000), word(r)]))
    counter = word(r)
    regions[0x7cdb98] = struct.pack('<I', counter)
    put(thing, 0x198, 'I', r.choice([0, counter, counter + 1, counter + 2, counter - 1, word(r)]) & 0xffffffff)
    regions[THING] = thing
    n = r.choice([0, 1, 2, 5, 0x806])   # (dynfaces.py's queue)
    regions.update({0x820b00: r.bytes(0xa0 * 9), 0x872168: r.bytes(0x40), 0x871ee0: struct.pack('<I', n), 0x763c98: bytes([r.choice([0, 1, 1])]) + bytes(3),
                    0x874fc4: struct.pack('<I', r.choice([0, 0, 1, 5]))})
    bob = r.choice([0.0, 0.5, 0.9, 0.9000001, 0.95, 1.0, 1.05, -1.0, 1e30, float('nan'), r.uniform(0, 1.2)])
    regions[0x7cedd4] = struct.pack('<If', r.choice([0, 1, 18, 19, 20, 21, 0xffffffff, r.getrandbits(32)]), bob)
    if op == 'CentreOnSprite':
        last = r.getrandbits(32)
        now = r.choice([last + r.randrange(0, 300), last + r.randrange(0, 300), last - r.randrange(1, 300), r.getrandbits(32)])
        regions[0x878a1c] = struct.pack('<I', last)
        regions[0x8786bc] = struct.pack('<I', now & 0xffffffff)
        regions[0x7908f0] = r.bytes(12)
        regions[0x790a88] = r.bytes(12)
        return Call(CALLS[op], [r.choice([THING] * 9 + [0])], regions, ret='v')
    t = r.choice([0.0, -0.0, 0.25, 0.35, 0.5, 1.0, 1.35, 1.5, 2.0, 2.5, -0.5, -1.0, 1e-40, float('nan'), float('inf'), float('-inf'),
                  r.uniform(-1.5, 2.5), r.uniform(0, 1), r.uniform(0, 1), struct.unpack('<f', r.bytes(4))[0]])
    return Call(CALLS[op], [THING, fb(t)], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
