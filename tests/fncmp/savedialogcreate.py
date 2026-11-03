# The save and load dialogs' making (src/game/ui/savedialogcreate: LoadDialog_Create 0x49efb0, SaveDialog_Create 0x49f280) against the originals. Window_FindChild real on children with random ids, the heap heap()'s; everything else a recorder (driver.cpp's kSaveDialogCreateEnv, in its order from 10; slot 9 of the windows 31).
# usage: savedialogcreate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from dialog7cb400 import VT, kids  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, put, recorder, set_return, vtable  # noqa: E402

C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x49efb0, 0x49f0a5), (0x49f280, 0x49f41c), (0x65d7bc, 0x65d840)],
          hooks={0x4092a0: _recorder(10, 2), 0x485b40: C(11, 0), 0x47ed80: C(12, 3), 0x65e59b: _recorder(13, 1), 0x485b00: C(14, 1), 0x485d20: C(15, 3), 0x65d9dd: _recorder(16, 1), 0x6639cb: _recorder(17, 2), 0x6636b2: _recorder(18, 2), 0x659bc9: C(19, 1), 0x662eee: _recorder(20, 1),
                 0x49ec80: C(21, 1), 0x49edf0: C(22, 1), 0x491170: _recorder(23, 1), 0x65d1c8: _recorder(24, 1), 0x65c388: _recorder(25, 1), 0x485a70: C(26, 1), 0x6662b7: _recorder(27, 2), 0x66656c: _recorder(28, 1), 0x66781a: _recorder(29, 1), 0x486b70: C(30, 0)})
DIALOG, KIDS = RAM + 0x100, RAM + 0x1000


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(fail=r.choice([0, 0, 0, 1])), 0x786ba4: r.bytes(4), 0x7cb244: r.bytes(8)}
    save = r.deal([0, 1])
    ids = [0x23bce2] if save else r.sample([0xffffffff, 0x23bce2, 0x23bce0 ^ 1], r.randrange(1, 4))
    if r.random() < 0.8:
        ids.append(0x23bce0)
    first, _ = kids(r, regions, KIDS, ids)
    regions[DIALOG] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', first)
    regions[VT] = put(vtable({}, size=0x10), 0x24, 'I', recorder(31, 1))
    set_return(log, 12, r.choice([DIALOG, DIALOG, DIALOG, DIALOG, 0]))
    for k in (14, 19, 23, 26):
        set_return(log, k, r.getrandbits(32))
    return Call(0x49f280 if save else 0x49efb0, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
