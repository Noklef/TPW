# The things' release (src/game/world/things: 0x5191e0) against the original. Release: thingenv.py's park (0-6 things of kinds 1, 3-8,
# 0x12 and others, some being deleted), each thing's script slot +0xc 0 or personscripts.py's script list's slot (the list ready or not;
# its scripts freed through the heap), its ride +0x20 any; Ride_Destroy (0x463c80) a cdecl recorder (k 31, driver.cpp's kQueuePiecesEnv).
# The sweeps on the same park (ThingSweepEnv's recorders, Thing_GetHandle real): Pose 0x519060, Claim 0x519120, Clear 0x515dd0 (the mouse mode 0 or a heap block).
# usage: thingsrelease.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Release,Pose,Claim,Clear)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, put  # noqa: E402
import personscripts  # noqa: E402
from thingenv import POOL, THING, park  # noqa: E402

B = Bench([(0x5191e0, 0x5192c1), (0x50b350, 0x50b370), (0x5da3c0, 0x5da3c1), (0x4d4170, 0x4d4186), (0x475010, 0x476a30),
           (0x540b90, 0x540da0), (0x541f60, 0x541fb0), (0x4d4100, 0x4d4134), (0x5f5e20, 0x5f5e90), (0x5f5fa0, 0x5f5fc0), (0x5f5660, 0x5f56a0),
           (0x519060, 0x5191d3), (0x515dd0, 0x515f22)],
          hooks={0x463c80: _recorder(31, 1, cdecl=True), 0x46c350: _recorder(20, 1, com=True), 0x4d8330: _recorder(21, 0), 0x50b780: _recorder(22, 0),
                 0x518ea0: _recorder(23, 0, cdecl=True), 0x50f240: _recorder(24, 0, cdecl=True), 0x544450: _recorder(25, 0, cdecl=True),
                 0x546280: _recorder(26, 2, cdecl=True), 0x4dd500: _recorder(27, 1)})
CALLS = {'Release': 0x5191e0, 'Pose': 0x519060, 'Claim': 0x519120, 'Clear': 0x515dd0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
SCRIPTS = RAM + 0x50000
STATE, MODE = RAM + 0x60000, RAM + 0x61000
WORLD = STATE - 0x1da738   # (+0x1da738 the state, the one field the sweep writes)


def sweep(r, name, regions):
    """Pose, Claim, Clear: the things' fields read, the kinds as park() gave them; every thing's +0x28 0 now and then; the mouse mode null or a block of the heap."""
    things = regions[POOL]
    for i in range(len(things) // THING):
        put(things, THING * i + 0x28, 'I', r.choice([0, r.getrandbits(32), r.getrandbits(32)]))
    regions[LOG] = log_region()
    if name == 'Pose':
        return Call(CALLS[name], [r.getrandbits(32), r.getrandbits(32)], regions, conv='t')
    if name == 'Claim':
        return Call(CALLS[name], [r.getrandbits(32)], regions, conv='t')
    regions[HEAP] = heap(0x40)
    regions[0x7b05e8] = struct.pack('<I', r.choice([0, HEAP + 0x10]))
    regions[0x7cf83c] = r.bytes(4)
    regions[STATE] = r.bytes(4)
    return Call(CALLS[name], [WORLD], regions, conv='t')


def case(r):
    name = r.deal(OPS)
    count = r.choice([0, 1, 2, 3, 4, 6, 6])
    regions, handles, _ = park(r, count=count, kinds=(1, 3, 3, 4, 5, 6, 7, 8, 0x12, 0x12, 2, 0, 9, 0x13), defs=False)
    if name != 'Release':
        return sweep(r, name, regions)
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
