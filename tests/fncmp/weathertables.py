# The weather renderer made and its tables filled (src/game/render/weathertables: 0x57d100, 0x57d300) against the originals. Rand the bench's generator from a dealt seed, timeGetTime a recorder; Render3d_Init and EngineErrors_Push recorders.
# A filling runs some 100k instructions under the emulator: a few dozen cases show it, every table entry being a fresh value.
# usage: weathertables.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, RAND_SEED, Bench, Call, _recorder, heap, import_table, log_region, mask_this, set_return  # noqa: E402

INIT, PUSH, TIME = 1, 2, 3
B = Bench([(0x57d100, 0x57d195), (0x57d300, 0x57d717)], hooks={0x57d1d0: _recorder(INIT, 0), 0x56af20: _recorder(PUSH, 2)})
FX = RAM
OPS = ['Fill', 'Create', 'Null', 'Refused', 'Already', 'Fill']


def case(r):
    op = r.deal(OPS)
    log = log_region()
    table = import_table(log, {'timeGetTime': (TIME, 0x6fd1d0, 0, 0, 0)})
    set_return(log, TIME, r.getrandbits(32))
    regions = {LOG: log, IMPORT_TABLE: table, RAND_SEED: r.bytes(4)}
    if op == 'Fill':
        regions[FX] = bytes(0xa1cc)
        return Call(0x57d300, [FX], regions, conv='t', ret='a')
    regions[0x8bcbac] = struct.pack('<I', FX if op == 'Already' else 0)
    regions[HEAP] = heap(0x10000, 1 if op == 'Null' else 0)
    mask_this(log, INIT)
    set_return(log, INIT, 0 if op == 'Refused' else FX)
    if op == 'Create':
        regions[FX] = bytes(0xa1cc)
    return Call(0x57d100, [], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
