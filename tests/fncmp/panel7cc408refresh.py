# Panel7cc408_Refresh (src/game/ui/panel7cc408, 0x4b75a0) against the original. Window_FindChild, Language_Font real, the heap heap()'s (failing now and then), the other calls recorders (kPanel7cc408Env, numbered as there from 10): their own benches have them. The window +0x10 with the child 0x3734 among decoys (missing
# now and then: the recorders get a null, nothing faults).
# usage: panel7cc408refresh.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialog7cb400  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, put, set_return  # noqa: E402

B = Bench([(0x4b75a0, 0x4b7660), (0x65d7bc, 0x65d840), (0x485a70, 0x485ab0)],
          hooks={0x48ca00: _recorder(10, 1), 0x48f6a0: _recorder(11, 1), 0x65d1c8: _recorder(12, 1), 0x65c428: _recorder(13, 2), 0x65c4b9: _recorder(14, 2), 0x65c388: _recorder(15, 1), 0x65c5d5: _recorder(16, 4), 0x65f5b7: _recorder(17, 1)})
SELF, WINDOW, KIDS = RAM, RAM + 0x100, RAM + 0x1000


def case(r):
    log = log_region()
    regions = {LOG: log}
    first, _ = dialog7cb400.kids(r, regions, KIDS, [0x3734] if r.random() < 0.9 else [0x3735])
    window = bytearray(r.bytes(0x200))
    put(window, 0x5c, 'I', first)
    regions[WINDOW] = window
    regions[SELF] = put(bytearray(r.bytes(0x40)), 0x10, 'I', WINDOW)
    regions[0x7cc430] = r.bytes(4)
    regions[0x78b534] = struct.pack('<I', r.randrange(4))
    regions[HEAP] = heap(fail=r.choice([0] * 7 + [1]))
    set_return(log, 11, r.getrandbits(32))
    return Call(0x4b75a0, [SELF], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
