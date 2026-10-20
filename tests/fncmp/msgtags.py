# The message tags' windows (src/game/ui/msgtags: 0x48ea00, 0x48eaa0) against the originals. NewWindow: window.py's tree and queues
# (Window_Ctor's surroundings, as listrows.py's NewRow), the heap failing now and then, the id any; the GUI's registered windows of
# listrows.py's register() with "msgtag" among them or not (the object +0x98's slot 3 event: its words and the window pointer hashed).
# Create: the same, the ten records [0x7c26b0] random bytes, an allocation of the ten (or none) failing.
# usage: msgtags.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=NewWindow,Create)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import listrows  # noqa: E402
from fncmp import HEAP, LOG, Bench, Call, blob_bytes, heap, log_region, put  # noqa: E402
from window import RANGES, queues, tree  # noqa: E402

CALLS = {'NewWindow': 0x48ea00, 'Create': 0x48eaa0}
B = Bench(sorted(set(RANGES + listrows.REG_RANGES + [(0x48ea00, 0x48ea9b), (0x48eaa0, 0x48eae5), (0x65e526, 0x65e549),
                                                     (0x65e6d5, 0x65e705)])))


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log}
    wins = tree(r, regions)
    queues(r, regions)
    regions[0xfaa540] = struct.pack('<II', r.randrange(-10, 120) & 0xffffffff, r.randrange(-10, 40) & 0xffffffff)
    regions[0xfaa5d8] = struct.pack('<5I', r.choice([wins[0], 0]), r.getrandbits(32), r.choice(wins + [0, 0]), r.getrandbits(32),
                                    r.choice([0] * 6 + [1]))
    listrows.register(r, regions, 'msgtag')
    regions[listrows.OBJ98] = put(bytearray(r.bytes(0x20)), 0, 'I', listrows.VT98)   # (register()'s other object)
    blob_bytes(log, 3, 1, 0x0ff0f3f0)   # (the event's words +4-+9 and +0xc-+0xf and the window pointer +0x14, its flags +0x18)
    if op == 'NewWindow':
        regions[HEAP] = heap(0x400, r.choice([0] * 5 + [1]))
        return Call(CALLS[op], [r.choice([0, 900, 909, r.getrandbits(16), r.getrandbits(32)])], regions, ret='a')
    regions[HEAP] = heap(0x2000, r.choice([0] * 4 + list(range(1, 11))))
    regions[0x7c26b0] = r.bytes(0x208 * 10)
    return Call(CALLS[op], [], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
