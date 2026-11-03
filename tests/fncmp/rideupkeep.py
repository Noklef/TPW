# RideUpkeep_Total (src/game/world/rideupkeep, 0x4c8240) against the original. Thiscall recorders (their own benches have them; driver.cpp's kRideUpkeepEnv): ObjectDefs_Find 10 (the id's word logged: the original's upper half is dirty; answering from a table of three definitions),
# Object_QueueOpen 11, ObjectControls_FindUncached 12 and ObjectControls_Find 13 (the words so), FunnyTime_SpanSince 14 (its out the stack's), Object_AgeDays 15. _alldiv runs. The original's debug print (the empty 0x5da3c0) is left out by the port.
# usage: rideupkeep.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, answers, log_region, put, set_return, strings  # noqa: E402

B = Bench([(0x4c8240, 0x4c8424), (0x5da3c0, 0x5da3c1), (0x67c320, 0x67c3e0)],
          hooks={0x412e90: _recorder(10, 1), 0x4dd920: _recorder(11, 0), 0x4d3d80: _recorder(12, 2), 0x4d3d10: _recorder(13, 2), 0x4f88b0: _recorder(14, 2), 0x4dd670: _recorder(15, 0)})
GAME, THINGS, DEFS, CONTROLS, TIME, TABLE, OPENS = RAM + 0x10000, RAM + 0x100, RAM + 0x1000, RAM + 0x4000, RAM + 0x4100, RAM + 0x4200, RAM + 0x4300
DAY = 0xc92a69c000


def definition(r):
    d = bytearray(r.bytes(0x500))
    put(d, 0x4ac, 'I', r.choice([0, 0, 2, 2, 1, 3, r.getrandbits(32)]))
    put(d, 0x9c, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    put(d, 0x5c, 'i', r.choice([1, 2, 5, 30, 100, -3, r.randrange(1, 1000)]))
    put(d, 0x58, 'i', r.choice([0, 100, -100, r.randrange(-1 << 31, 1 << 31)]))
    return d


def case(r):
    log = log_region()
    for k in (10, 12, 13):
        struct.pack_into('<I', log, LOG_BYTES + 4 * k, 1 << 16)
    strings(log, 14, 0, out=0b1)
    answers(log, 10, TABLE)
    answers(log, 11, OPENS)
    set_return(log, 12, CONTROLS)
    set_return(log, 13, CONTROLS)
    set_return(log, 14, TIME)
    set_return(log, 15, r.choice([0, 1, 3, 10, 29, 100, -1, r.randrange(-1000, 1000)]))
    count = r.choice([0, 1, 1, 2, 3])
    slots = bytearray(r.bytes(20 * 5))
    things = bytearray(r.bytes(0x10 * 4))
    for i in range(1, count + 1):
        put(slots, 20 * i, 'I', THINGS + 0x10 * i - 0x10)
        put(things, 0x10 * i - 0x10 + 0xc, 'H', i + 1 if i < count else 0)
    defs = [DEFS + 0x1000 * k for k in range(3)]
    controls = bytearray(r.bytes(0x20))
    put(controls, 0x18, 'i', r.choice([1, 1, 2, 5, 0, -1]))
    days = r.choice([0, 1, 5, 30, 100, r.randrange(1000)])
    regions = {LOG: log, 0x80239c: struct.pack('<I', GAME), GAME + 0x1da746: struct.pack('<H', count and 1), 0x7cfb90: slots, THINGS: things, CONTROLS: controls,
               TIME: struct.pack('<Q', (days * DAY + r.choice([0, 1, DAY - 1, r.randrange(DAY)])) & 0xffffffffffffffff if r.random() < 0.9 else r.getrandbits(64)),
               TABLE: struct.pack('<40I', *[r.choice(defs) for _ in range(40)]), OPENS: struct.pack('<40I', *[r.choice([0, 1, 1, 1, r.getrandbits(32)]) for _ in range(40)])}
    for k, at in enumerate(defs):
        regions[at] = definition(r)
    return Call(0x4c8240, [RAM], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
