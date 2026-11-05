# The game's end (src/game/core/gameshutdown: 0x5503f0, 0x550aa0, 0x40f230, 0x44e560) against the originals: started or not, at state 0xa or another; the modules' shut downs recorders in their order; the temp folder's String real (path wide,
# name narrow, ended in "\" or not), the tree's removal a recorder hashing the String's bytes.
# usage: gameshutdown.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import gamestring  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, blob_bytes, heap, log_region, set_return  # noqa: E402

CDECL = {0x54e360: 1, 0x40d070: 2, 0x51b3e0: 3, 0x51bc70: 4, 0x51eda0: 5, 0x51ee40: 6, 0x51b7c0: 7, 0x53e930: 8, 0x51fd20: 9, 0x5584b0: 10, 0x55d4d0: 11, 0x598ad0: 12, 0x572b60: 13, 0x583700: 14, 0x5633a0: 15}
REMOVE, OWNER, GUI = 16, 17, 18
HOOKS = {addr: _recorder(k, 0, cdecl=True) for addr, k in CDECL.items()}
HOOKS.update({0x5c5250: _recorder(REMOVE, 1, cdecl=True), 0x409150: _recorder(OWNER, 0), 0x4813d0: _recorder(GUI, 0)})
B = Bench([(0x5503f0, 0x55045d), (0x550aa0, 0x550b2d), (0x40f230, 0x40f23b), (0x44e560, 0x44e570), (0x619240, 0x619250), (0x5c5480, 0x5c5510)] + gamestring.B_RANGES, hooks=HOOKS)
OPS = ['End', 'End', 'End', 'End', 'Temp', 'Owner', 'Engine']


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, HEAP: heap(), gamestring.LOCALE: bytes(4)}
    blob_bytes(log, REMOVE, 0, 0xfffff)
    gamestring.string(r, 1, 0xf816c0 + 0x28, RAM + 0x100, regions, text=b''.join(bytes([c, 0]) for c in r.choice([b'c:\\temp', b'c:\\temp\\', b'x', b''])))
    gamestring.string(r, 0, 0xf80be0, RAM + 0x200, regions, text=b'\\')
    gamestring.string(r, 0, 0x878eb8, RAM + 0x300, regions, text=r.choice([b'park', b'tmp\\a', b'']))
    if op == 'Owner':
        set_return(log, OWNER, r.getrandbits(32))
        return Call(0x40f230, [], regions, ret='a')
    if op == 'Engine':
        set_return(log, CDECL[0x5633a0], r.getrandbits(32))
        return Call(0x44e560, [], regions, ret='a')
    if op == 'Temp':
        return Call(0x550aa0, [], regions, ret='v')
    regions[0x879068] = struct.pack('<II', r.choice([1, 1, 1, 1, 0, 0x100]), r.choice([0xa, 0xa, 0xb, 0, 9, 0x10a]))
    return Call(0x5503f0, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
