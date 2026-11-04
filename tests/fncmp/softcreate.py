# The software renderer's object made (src/game/render/devicecreate: 0x573960) against the original, its open a recorder (devicecreate.py compares that): there already or not, the allocation failing, the open failing.
# usage: softcreate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from devicecreate import ERROR, OPENSOFT, tables  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, set_return  # noqa: E402

B = Bench([(0x573960, 0x573a30)], hooks={0x573b90: _recorder(OPENSOFT, 1), 0x56af20: _recorder(ERROR, 2)})
OPS = ['opens', 'opens', 'fails', 'there', 'no memory']


def case(r):
    op = r.deal(OPS)
    log = log_region()
    set_return(log, OPENSOFT, 0 if op == 'fails' else r.choice([1, 2, -1]))
    regions = {LOG: log, HEAP: heap(0x200, 1 if op == 'no memory' else 0), **tables(), 0x879dac: struct.pack('<I', RAM + 0xf00 if op == 'there' else 0), 0x8bd52c: r.bytes(4)}
    return Call(0x573960, [r.getrandbits(32)], regions)


if __name__ == '__main__':
    B.main(case)
