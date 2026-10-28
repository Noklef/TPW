# The things' release (src/game/world/things: 0x5191e0) against the original. Release: thingenv.py's park (0-6 things of kinds 1, 3-8,
# 0x12 and others, some being deleted), each thing's script slot +0xc 0 or personscripts.py's script list's slot (the list ready or not;
# its scripts freed through the heap), its ride +0x20 any; Ride_Destroy (0x463c80) a cdecl recorder (k 31, driver.cpp's kQueuePiecesEnv).
# usage: thingsrelease.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Release)
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402
import personscripts  # noqa: E402
from thingenv import POOL, THING, park  # noqa: E402

B = Bench([(0x5191e0, 0x5192c1), (0x50b350, 0x50b370), (0x5da3c0, 0x5da3c1), (0x4d4170, 0x4d4186), (0x475010, 0x476a30),
           (0x540b90, 0x540da0), (0x541f60, 0x541fb0), (0x4d4100, 0x4d4134), (0x5f5e20, 0x5f5e90), (0x5f5fa0, 0x5f5fc0), (0x5f5660, 0x5f56a0)],
          hooks={0x463c80: _recorder(31, 1, cdecl=True)})
CALLS = {'Release': 0x5191e0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
SCRIPTS = RAM + 0x50000


def case(r):
    r.deal(OPS)
    count = r.choice([0, 1, 2, 3, 4, 6, 6])
    regions, handles, _ = park(r, count=count, kinds=(1, 3, 3, 4, 5, 6, 7, 8, 0x12, 0x12, 2, 0, 9, 0x13), defs=False)
    holder = bytearray(r.bytes(0x40))
    personscripts.script_list(r, regions, holder, base=SCRIPTS)
    slot = holder[0xc]
    things = regions[POOL]
    for i in range(count):
        put(things, THING * i + 0xc, 'I', r.choice([0, slot, slot, 1, 2, 3]))
        put(things, THING * i + 0x20, 'I', r.getrandbits(32))
    regions[LOG] = log_region()
    return Call(CALLS['Release'], [RAM], regions, conv='t')


if __name__ == '__main__':
    B.main(case)
