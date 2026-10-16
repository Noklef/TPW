# A person's sprite script and ride timers (src/game/world/personscripts: 0x4fdc90, 0x4fe950,
# 0x50c0d0) against the originals, with the script list not ready (0x7b49f0 zero): script slots set
# or not; clock values on and off the 20-tick beat.
# usage: personscripts.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Tick,Jump,Kill)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x4fdc90, 0x4fdcb6), (0x4fdc60, 0x4fdc90), (0x4fe950, 0x4fe977), (0x50c0d0, 0x50c0f2), (0x475010, 0x476a30),
           (0x540b90, 0x540d78), (0x541f60, 0x541f70), (0x5da3c0, 0x5da3c1)])
CALLS = ['Tick', 'Jump', 'Kill']
CLOCK = RAM + 0x1000


def case(r):
    op = r.choice(OPS)
    person = bytearray(r.bytes(0x400))
    regions = {RAM: person, 0x7b49f0: bytearray(16)}
    if op == 'Tick':
        regions[0x802390] = put(bytearray(r.bytes(0x20)), 0xc, 'I', CLOCK - 0x1da70c)
        regions[CLOCK] = put(bytearray(r.bytes(0x20)), 0, 'I', r.choice([0, 20, 40, 1, 19, 21, 0x7fffffff, r.getrandbits(32), 20 * r.randrange(1000)]))
        return Call(0x4fdc90, [RAM], regions, conv='t')
    if op == 'Jump':
        put(person, 0x210, 'I', r.choice([0, 0, 1, 2, 7, 0xffffffff]))
        return Call(0x4fe950, [RAM], regions, conv='t')
    put(person, 0x84, 'I', r.choice([0, 0, 1, 2, 7, 0xffffffff]))
    return Call(0x50c0d0, [RAM], regions, conv='t')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
