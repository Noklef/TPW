# A person's sprite script and ride timers (src/game/world/personscripts: 0x4fdc90, 0x4fe950,
# 0x50c0d0, 0x4f9ed0, 0x4fe900, 0x50be40) against the originals, with the script list not ready (0x7b49f0 zero): script slots set
# or not; clock values on and off the 20-tick beat.
# usage: personscripts.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Tick,Jump,Kill)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x50be40, 0x50be7c), (0x4f9ed0, 0x4f9efc), (0x4fe900, 0x4fe944), (0x4d4170, 0x4d4186), (0x475550, 0x47558b), (0x4fdc90, 0x4fdcb6), (0x4fdc60, 0x4fdc90), (0x4fe950, 0x4fe977), (0x50c0d0, 0x50c0f2), (0x475010, 0x476a30),
           (0x540b90, 0x540d78), (0x541f60, 0x541f70), (0x5da3c0, 0x5da3c1)])
CALLS = ['Tick', 'Jump', 'Kill', 'Release', 'Move', 'After']
CLOCK = RAM + 0x1000


def case(r):
    op = r.choice(OPS)
    person = bytearray(r.bytes(0x400))
    regions = {RAM: person, 0x7b49f0: bytearray(16)}
    if op == 'Tick':
        regions[0x802390] = put(bytearray(r.bytes(0x20)), 0xc, 'I', CLOCK - 0x1da70c)
        regions[CLOCK] = put(bytearray(r.bytes(0x20)), 0, 'I', r.choice([0, 20, 40, 1, 19, 21, 0x7fffffff, r.getrandbits(32), 20 * r.randrange(1000)]))
        return Call(0x4fdc90, [RAM], regions, conv='t')
    if op == 'After':
        stamp = r.choice([0, 12, 13, 100, 0xfffffff0, r.getrandbits(32)])
        put(person, 0x8c, 'I', (stamp + r.choice([-13, -12, -11, 0, 5, r.getrandbits(32)])) & 0xffffffff)
        put(person, 0x84, 'I', r.choice([0, 0, 1, 2, 7]))
        regions[0x7cf6e0] = put(bytearray(r.bytes(0x20)), 0xc, 'I', RAM + 0x800)
        regions[RAM + 0x800] = put(bytearray(8), 0, 'I', RAM + 0x1000 - 0x1da70c)
        regions[RAM + 0x1000] = put(bytearray(r.bytes(0x20)), 0, 'I', stamp)
        return Call(0x50be40, [RAM], regions, conv='t')
    if op == 'Release':
        for off in (0xc, 0x30, 0x210):
            put(person, off, 'I', r.choice([0, 0, 1, 2, 7]))
        person[2] = r.choice([0, 1, 1, 2])
        return Call(0x4f9ed0, [RAM], regions, conv='t')
    if op == 'Move':
        put(person, 0x210, 'I', r.choice([0, 0, 1, 2, 7]))
        return Call(0x4fe900, [RAM] + [r.getrandbits(32) for _ in range(4)], regions, conv='t')
    if op == 'Jump':
        put(person, 0x210, 'I', r.choice([0, 0, 1, 2, 7, 0xffffffff]))
        return Call(0x4fe950, [RAM], regions, conv='t')
    put(person, 0x84, 'I', r.choice([0, 0, 1, 2, 7, 0xffffffff]))
    return Call(0x50c0d0, [RAM], regions, conv='t')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
