# A theme's record found or made in the global save info's theme map, and all destroyed (src/game/core/globalsave: 0x5b0a70, 0x5b09c0, 0x5b0bc0, 0x5b08a0) against the originals: string_map.py's
# trees and keys, the name there or new, the multi byte mostly clear, the heap failing now and then (the record's block: none is made and a null goes in the map; a string's block: a
# fault in both). ThemeRecord_LoadGlobalSam (0x5b11c0, tests/fncmp/globalsave.py's) a recorder whose answer has AL set or clear. Debug_Printf (0x5da3c0) is left out of the port, with
# the lookup in 0x5b0a70 that only chose whether it prints. Clear: each record a string at +0x20 with no global.sam, or a null at the node.
# usage: thememap.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Make,Get,Clear,Select)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import gamestring  # noqa: E402
import string_map  # noqa: E402
import stdtree  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, set_return  # noqa: E402

LOAD = 8
CALLS = {'Make': 0x5b0a70, 'Get': 0x5b09c0, 'Clear': 0x5b0bc0, 'Select': 0x5b08a0}
B = Bench([(0x5b08a0, 0x5b093a), (0x5b09c0, 0x5b0a6c), (0x5b0a70, 0x5b0bb2), (0x5b0bc0, 0x5b0c31), (0x5b0cd0, 0x5b0d70), (0x5b17c0, 0x5b1925), (0x5b1b80, 0x5b20c0), (0x5b20c0, 0x5b20e0), (0x5b20e0, 0x5b2169), (0x5b2f40, 0x5b2fd0), (0x5b2480, 0x5b2530),
           (0x5b16b0, 0x5b17c0), (0x5b2170, 0x5b2480), (0x5b2ff0, 0x5b30b0), (0x5b31f0, 0x5b3203), (0x5b3230, 0x5b324b), (0x45b340, 0x45b9a0), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None, 0x5da3c0: lambda e: None, 0x5b11c0: _recorder(LOAD, 0)})
NAME = RAM + 0x800
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
SPARE, BLOCKS = RAM + 0x20000, RAM + 0x28000


def case(r):
    op = r.deal(OPS)
    string_map.OPS = ['Lower']
    regions = dict(string_map.case(r).regions)
    head = struct.unpack_from('<III', regions[stdtree.HEAD])
    n = 0 if head[1] == stdtree.NIL else len(regions[stdtree.NODES]) // 0x40
    tree = bytearray(regions[stdtree.TREE])
    struct.pack_into('<I', tree, 8, r.choice([0, 0, 0, 1]))
    struct.pack_into('<I', tree, 0xc, n)
    regions[stdtree.TREE] = tree
    regions[HEAP] = heap(fail=r.choice([0] * 24 + [1, 1, 1, 1, 2, 3, 4]) if op != 'Clear' else 0)
    log = log_region()
    set_return(log, LOAD, r.choice([0, 1, 1, 0x100, 0xffffff00, r.getrandbits(32)]))
    regions[LOG] = log
    if op == 'Clear':
        blob, objs = bytearray(regions[stdtree.NODES]), bytearray(regions[string_map.OBJS])
        for i in range(n):
            if r.random() < 0.2:
                struct.pack_into('<I', blob, 0x40 * i + 0x20, 0)
                continue
            extra = {}
            gamestring.string(r, 0, SPARE, BLOCKS + 0x80 * i, extra)
            objs[0x100 * i + 0x20:0x100 * i + 0x34] = extra[SPARE][:0x14]
            regions[BLOCKS + 0x80 * i] = extra[BLOCKS + 0x80 * i]
        regions[stdtree.NODES], regions[string_map.OBJS] = blob, objs
        return Call(CALLS[op], [stdtree.TREE - 0x38], regions, conv='t', ret='v')
    return Call(CALLS[op], [stdtree.TREE - 0x38, NAME], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
