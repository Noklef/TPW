# The rides written to a save (src/game/world/ridesload: 0x464140) against the original: ridesload.py's rides in some of the slots at 0x7a4614, the block (GlobalMem_Alloc's recorder answers it) compared whole, the stream's write a
# recorder answering what was asked or short (of the size, or of the block), the dword each write's buffer starts with logged.
# usage: ridessave.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import ridesload  # noqa: E402
from fncmp import LOG, Bench, Call, _recorder, answers, blob, log_fold, log_region, set_return, vtable  # noqa: E402
from ridesload import ALLOC, AFTER, BLOCK, FREE, RIDES, STREAM, TABLES, VT, dwords, ride_fixture  # noqa: E402

WRITE = 3
B = Bench([(0x464140, 0x464578), (0x5f5e50, 0x5f5e90), (0x5da3c0, 0x5da3c1)],
          hooks={0x45b9d0: _recorder(ALLOC, 2, cdecl=True), 0x45ba00: _recorder(FREE, 1, cdecl=True), 0x464580: _recorder(AFTER, 2, cdecl=True)})
ANSWERS = TABLES + 0xa00


def case(r):
    log = log_region()
    log_fold(log)
    regions = {LOG: log, STREAM: dwords(VT, 0, 0, 0), VT: vtable({WRITE: 2})}
    count = r.choice([0, 1, 2, 3, 5, 8])
    first = r.randrange(count + 1)
    regions[0x7a4084], regions[0x7a3ebc] = dwords(first), dwords(count - first)
    regions[0x7aedf4] = r.bytes(4)
    shapes = [ride_fixture(r, regions, i) for i in range(3)]
    slots = [r.choice([0, 1, 1, 2, 2, 3]) for _ in range(count)]   # (0 none, else that ride)
    regions[0x7a4614] = dwords(*[RIDES + 0x1000 * (s - 1) if s else 0 for s in slots], 0)
    size = 0xc
    for s in slots:
        size += 1
        if s:
            vertices, parts, layers = shapes[s - 1]
            size += 0x36 + layers * 0x2c + vertices * 8 + parts * 4
    regions[BLOCK] = r.bytes(0x2000)
    set_return(log, ALLOC, BLOCK)
    regions[ANSWERS] = dwords(r.choice([4] * 11 + [3]), r.choice([size] * 11 + [size - 1]), *[0] * 38)
    answers(log, WRITE, ANSWERS)
    blob(log, WRITE, 0, 1)
    return Call(0x464140, [STREAM], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
