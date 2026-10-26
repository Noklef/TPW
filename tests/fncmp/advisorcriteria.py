# The advisor scoring's park criteria (src/game/core/advisorscoring: 0x59c8d0-0x59fb10) against the originals, each on parkenv.py's
# joint park (things, chains, definitions and control records, the analyser, the research lab), the scoring's bounds small (about the
# counts the park can give: -1 to 5, the percentages and half counts about 30 and 100) or anything, its weights anything.
# usage: advisorcriteria.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Booth,No02,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import Bench, Call, RAM, put  # noqa: E402
import parkenv  # noqa: E402

# name: (address, the scoring's bounds: offset -> the values about them)
PCT = [0, 25, 33, 50, 50, 66, 100, 100, 150, -1]
CALLS = {'Booth': (0x59c8d0, {}), 'No02': (0x59c960, {}), 'Obj0': (0x59ca80, {}), 'Tal7': (0x59cd30, {}),
         'Low6': (0x59ce30, {0x10c: [0, 1, 2, 30, 100, -1]}), 'Share8': (0x59cea0, {0x114: PCT}), 'All8': (0x59cf50, {}),
         'Tal7b': (0x59d3b0, {}), 'Res0': (0x59da30, {}), 'C14': (0x59dbf0, {}), 'Res1': (0x59dd40, {}), 'Res2': (0x59dfd0, {})}
B = Bench([(0x412e60, 0x412f00), (0x41a960, 0x41a980), (0x41a990, 0x41a9c0), (0x4c7fa0, 0x4c8080), (0x4c81e0, 0x4c8250), (0x4c9050, 0x4c9080),
           (0x4c91c0, 0x4c92a0), (0x4c94a0, 0x4c94e0), (0x4c9530, 0x4c9680), (0x4c9750, 0x4c9840), (0x4c9960, 0x4c9ab0), (0x4c9b40, 0x4c9c10),
           (0x4c9e60, 0x4c9ef0), (0x4ca310, 0x4ca3b0), (0x4ca450, 0x4ca6c0), (0x4ca7b0, 0x4ca830), (0x4ca8e0, 0x4ca9c0), (0x4d3d80, 0x4d3df0),
           (0x4d8610, 0x4d8660), (0x4e0370, 0x4e0390), (0x4fa990, 0x4faa10), (0x503980, 0x503a00), (0x503b30, 0x503b90), (0x506eb0, 0x506ee0),
           (0x50b350, 0x50b370), (0x516130, 0x5161c0), (0x519510, 0x5195a0), (0x51a280, 0x51a2a0), (0x536300, 0x53644c),
           (0x59c8d0, 0x59c9e0), (0x59ca80, 0x59cad0), (0x59cd30, 0x59cda0), (0x59ce30, 0x59cff0), (0x59d3b0, 0x59d410), (0x59da30, 0x59dae0),
           (0x59dbf0, 0x59dc70), (0x59dd40, 0x59ddf0), (0x59dfd0, 0x59e080), (0x59e260, 0x59e2f0), (0x59e840, 0x59e8a0), (0x59f940, 0x59f9b0),
           (0x59fa60, 0x59fb20), (0x5da3c0, 0x5da3c1)])
NONE = {'Obj0': 4, 'Tal7': 7, 'Low6': 6, 'Share8': 8, 'All8': 8}   # (the kind of staff a criterion asks to be absent)
UNFLAGGED = {'Tal7', 'Low6', 'Share8', 'All8'}
SCORING = RAM + 0x50000
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def case(r):
    name = r.deal(OPS)
    address, bounds = CALLS[name]
    deep = r.random() < 0.6   # (the criteria that ask for no staff of a kind and the flag clear: mostly so, to reach their last tests)
    regions, _info = parkenv.park(r, kinds=tuple(k for k in parkenv.KINDS if k != NONE.get(name)) if deep else parkenv.KINDS,
                                  flag=0 if deep and name in UNFLAGGED else None)
    s = bytearray(r.bytes(0x4d0))
    for off in range(0xa0, 0x4d0, 4):   # (most dwords small, so a bound lands about the counts; the weights small or anything)
        if r.random() < 0.7:
            put(s, off, 'I', parkenv.small(r))
    for off, values in bounds.items():
        if r.random() < 0.8:
            put(s, off, 'I', r.choice(values) & 0xffffffff)
    regions[SCORING] = s
    return Call(address, [SCORING], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
