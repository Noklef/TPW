# A coaster link's cars set up from its train (src/game/coaster/registry: CoasterLink_SetUpCars 0x436d10) against the original. The
# train: 0-3 cars (+0x2e8) of 0-4 segments (+0x2f4), its first, middle and last segment types (+0x2fc, +0x300, +0x304: one of three, or
# the same) with 0-3 slot runs (+0x30) and a ride type (+0xc) or none, its slot count (+0x318). The blocks through the GlobalAlloc import
# (global_heap). The rides from agent B's unported 0x463060, a cdecl recorder (30 of 10, driver.cpp's kCoasterLinksEnv) answering a
# slot of the holder table 0x7a4610 that holds holder.py's Release target (its renderer either way: by the rectangle or Holder_Sleep),
# its instance's model (+8 -> +4) set playing. quiet 0 mostly, 1 or any.
# usage: linkcars.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, global_heap, heap, log_region, put, set_return  # noqa: E402
import holder  # noqa: E402

B = Bench(holder.RANGES + [(0x436d10, 0x436f5b), (0x472f40, 0x472f59), (0x45b9d0, 0x45ba00)],
          hooks={**holder.HOOKS, 0x463060: _recorder(30, 10, cdecl=True)})
TRAIN, LINK, TYPES, MODEL = 0x3c000000, 0x3c001000, 0x3c002000, RAM + 0x9000


def case(r):
    regions = dict(holder.release(r).regions)
    regions.setdefault(LOG, log_region())
    regions.update({HEAP: heap(), **global_heap()})
    put(regions[RAM], 4, 'I', MODEL)   # (the Release target's instance (+8): its model, set playing)
    regions[MODEL] = r.bytes(0x40)
    log = regions[LOG]
    slot = r.randrange(8)
    set_return(log, 30, slot)
    regions[0x7a4610] = b''.join(struct.pack('<I', holder.TARGET if k == slot or r.random() < 0.3 else 0) for k in range(8))
    types = [TYPES + k * 0x40 for k in range(3)]
    for t in types:
        data = bytearray(r.bytes(0x40))
        put(data, 0x30, 'I', r.choice([0, 1, 2, 3]))
        put(data, 0xc, 'I', r.choice([0, 0, 1, 0x33, r.getrandbits(32)]))
        regions[t] = data
    train = bytearray(r.bytes(0x320))
    put(train, 0x2e8, 'I', r.choice([0, 1, 2, 3]))
    put(train, 0x2f4, 'I', r.choice([0, 1, 2, 3, 4]))
    first = r.choice(types)
    put(train, 0x2fc, 'I', first)
    put(train, 0x300, 'I', r.choice([first] + types))
    put(train, 0x304, 'I', r.choice([first] + types))
    put(train, 0x318, 'I', r.choice([0, 1, 4, 12]))
    regions.update({TRAIN: train, LINK: r.bytes(0x150)})
    return Call(0x436d10, [TRAIN, LINK, r.choice([0, 0, 0, 1, r.getrandbits(32)])], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
