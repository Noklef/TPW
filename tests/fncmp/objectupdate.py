# An object's tick (src/game/world/objectupdate: Object_Update 0x4e0b90, Object_PostUpdate 0x4e0e00) against the originals. What they call is recorders (ObjectUpdateEnv):
# Rides_FindById 1, Rides_GetElement 2 (its answers in turn), Rides_SetElement 3, Cell_MakeHandle 4 (its out the stack's, a word written: the rest of the dword is the
# object's address, sic), Map_SpreadRemove 5, Object_WearTick 6, Visitor_IsState11To14 7, Object_BeginCycle 8, Object_DismissQueueHead 9, Object_ArriveWaiting 10.
# The two floats +0x44, +0x48 about the truncation's 0, 1 and 25, the bounds of the clamp, huge and NaN; the handle +0x3c in the table at 0x7cfb90, its thing null or not.
# usage: objectupdate.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=update,post)
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, fill, log_region, put, set_return, strings  # noqa: E402

B = Bench([(0x4e0b90, 0x4e0da3), (0x4e0e00, 0x4e0e3f), (0x5da3c0, 0x5da3c1)],
          hooks={0x55a070: _recorder(1, 1, cdecl=True), 0x55a390: _recorder(2, 2, cdecl=True), 0x55a0b0: _recorder(3, 3, cdecl=True), 0x4cd4e0: _recorder(4, 3, cdecl=True),
                 0x4d8460: _recorder(5, 2), 0x4df670: _recorder(6, 0), 0x502430: _recorder(7, 0), 0x4e14e0: _recorder(8, 0), 0x4e0450: _recorder(9, 0), 0x4e1410: _recorder(10, 0)})
OBJ, RIDE, ANSWERS, THING, TICKS = RAM, RAM + 0x400, RAM + 0x500, RAM + 0x600, RAM + 0x800
GAME = TICKS - 0x1da70c
TABLE = 0x7cfb90


def number(r):
    return r.choice([r.uniform(0, 100), r.uniform(0, 1), r.uniform(1, 2), r.uniform(24, 26), 25.0, 24.999, 0.0, -0.0, 1.0, 5.0, 4.9, 5.1, 100.0, 105.0, 105.5, 106.0, r.uniform(-20, 140),
                     -0.5, 256.0, 256.5, 511.0, 1e30, -1e30, math.nan])


def case(r):
    name = r.deal(OPS)
    log = log_region()
    obj = bytearray(r.bytes(0x1a0))
    put(obj, 0x19c, 'I', r.choice([0, 0, 0, 2, 2, 2, 1, 3, 4, 5, 0xffffffff, r.getrandbits(32)]))
    regions = {LOG: log, OBJ: obj}
    if name == 'post':
        return Call(0x4e0e00, [OBJ], regions, conv='t', ret='v')
    obj[0x32] = r.choice([0, 1, 0x80, 0x81, r.getrandbits(8)])
    put(obj, 0x44, 'f', number(r))
    put(obj, 0x48, 'f', number(r))
    put(obj, 0x68, 'I', r.choice([0, 1, 1, 1, 1, r.getrandbits(32)]))
    handle = r.choice([0, 0, 1, 7, 0x15e, r.randrange(0x15f)])
    put(obj, 0x3c, 'H', handle)
    table = bytearray(r.bytes(0x14 * 0x160))
    put(table, 0x14 * handle, 'I', r.choice([0, THING, THING]))
    set_return(log, 1, RIDE)
    answers(log, 2, ANSWERS)
    set_return(log, 7, r.choice([0, 1, 0x100, r.getrandbits(32)]))
    strings(log, 4, 0, out=1)
    fill(log, 4, 0, r.bytes(2))
    regions.update({ANSWERS: struct.pack('<40I', *(r.choice([0, 0, 1, 1, 2, r.getrandbits(32)]) for _ in range(40))), TICKS: bytes([r.choice([0, 8, 0x10, 0x48, r.getrandbits(8)])]),
                    TABLE: table, 0x80239c: struct.pack('<I', GAME), 0x8023a0: r.bytes(4), RIDE: r.bytes(0x10), THING: r.bytes(0x10)})
    return Call(0x4e0b90, [OBJ], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', 'update,update,update,update,post').split(',') if k]

if __name__ == '__main__':
    B.main(case)
