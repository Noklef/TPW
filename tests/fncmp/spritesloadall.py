# All the sprite sets loaded (src/game/sprite/loading: 0x541d90) against the original, the Strings real, the find and Sprites_LoadFolder recorders: the folder not found, all 14 sets or only the last, the count moving under each load (the found folder being the count's own address, written through by the recorder).
# usage: spritesloadall.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, answers, blob, fill, heap, log_region, put, set_return, strings  # noqa: E402

LOAD, FIND = 8, 9
COUNT = 0x87626c
B = Bench([(0x541d90, 0x541ec5), (0x5f5fc0, 0x5f7f00), (0x409cd0, 0x409cdc), (0x67a400, 0x6902c0)], hooks={0x541310: _recorder(LOAD, 2, cdecl=True), 0x421770: _recorder(FIND, 2)})
FOLDER, TABLE, NAMES = RAM, RAM + 0x100, RAM + 0x400
OPS = ['all', 'all', 'all', 'last', 'last', 'not found', 'found twice', 'other folder']


def case(r):
    op = r.deal(OPS)
    log = log_region()
    flag = r.getrandbits(24) << 8 | r.randrange(1, 0x100) if op == 'last' else r.getrandbits(24) << 8
    strings(log, FIND, 0, 2)
    set_return(log, FIND, 0 if op == 'not found' else 2 if op == 'found twice' else 1)
    fill(log, FIND, 1, struct.pack('<I', FOLDER if op == 'other folder' else COUNT), which=0)
    start = r.randrange(0, 0x100)
    fill(log, LOAD, 0, struct.pack('<14I', *[start + r.randrange(0, 5) + 6 * i for i in range(14)]), which=1, records=14)
    blob(log, LOAD, 1, 2)   # (the name String by its length)
    answers(log, LOAD, TABLE)
    names = bytearray(0x20 * 14)
    sets = bytearray(r.bytes(0xa8))
    for i, length in enumerate(r.sample(range(1, 0x1f), 14)):
        names[0x20 * i:0x20 * i + length] = bytes(r.randrange(0x41, 0x5b) for _ in range(length))
        put(sets, 0xc * i, 'I', NAMES + 0x20 * i)
    regions = {LOG: log, FOLDER: r.bytes(0x10), TABLE: r.bytes(0xa0), NAMES: names, HEAP: heap(0x4000), 0x786b9c: r.bytes(4), COUNT: r.bytes(4), 0x876b28: r.bytes(4), 0x763f88: sets}
    return Call(0x541d90, [flag], regions, conv='c', ret='b')


if __name__ == '__main__':
    B.main(case)
