# The disc checked (src/game/files/gamepaths: 0x45f7a0) against the original: its statics made at first or there, GamePaths_FindDisc run on a copy (kept here by its recorder's stand-in, so it compares), its answer 1 or not, the copy's list freed. The callees are recorders.
# usage: gamepathscheckdisc.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fill_this, log_region, mask_this, put, set_return  # noqa: E402

ATEXIT, MAKE, FIND, NAME_END, FREE = 2, 3, 4, 5, 6
NODES, OUT = RAM + 0x100, RAM + 0x800
_find_record = _recorder(FIND, 0)
OPS = ['found', 'found', 'found', 'missing', 'missing', 'other', 'first', 'first', 'no list', 'one']


def _find(e):
    """GamePaths_FindDisc's recorder (tools/fncmp/driver.cpp's findDiscStandIn, the same): the copy kept first."""
    e.write(OUT, e.read(e.r['ECX'], 0x35))
    _find_record(e)


B = Bench([(0x45f7a0, 0x45f8df)], hooks={0x67a7d0: _recorder(ATEXIT, 1, cdecl=True), 0x5f8790: _recorder(MAKE, 0), 0x5aa500: _find, 0x5f61e0: _recorder(NAME_END, 0), 0x67acb0: _recorder(FREE, 1, cdecl=True)})


def case(r):
    op = r.deal(OPS)
    log = log_region()
    count = 0 if op == 'no list' else 1 if op == 'one' else r.randrange(2, 6)
    nodes = bytearray(r.bytes(0x20 * 6))
    for i in range(count):
        put(nodes, 0x20 * i + 0x14, 'I', NODES + 0x20 * (i + 1) if i + 1 < count else 0)
    mask_this(log, FIND)
    fill_this(log, FIND, struct.pack('<I', NODES if count else 0), 4)
    set_return(log, FIND, {'missing': 0, 'other': r.choice([2, 0x100, 0x101, 0xffffffff])}.get(op, 1))
    guard = r.getrandbits(8)
    regions = {LOG: log, NODES: nodes, OUT: bytes(0x38), 0xfb34ca: bytes([guard & 0xfe if op == 'first' else guard | 1]), 0xfb34e0: r.bytes(0x35)}
    return Call(0x45f7a0, [], regions, conv='c', ret='b')


if __name__ == '__main__':
    B.main(case)
