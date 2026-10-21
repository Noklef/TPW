# The dialog [0x7cc1f8]'s stats shown or hidden (src/game/ui/dialog7cc1f8: 0x4abfb0) against the original. ShowStats: window.py's shown_env
# tree, its root the stats child (id 0x1ed mostly, else another) and its root's children given the eight stats ids mostly (0x1ed, 0x1f4, a
# high bit or any now and then; repeats and misses); deeper windows any id or one of them. The dialog's children: the root, a decoy (id any or
# 0x1ed, childless) then the root, the decoy alone or none. The mode 0-3 mostly, 4, -1 or any.
# usage: dialog7cc1f8.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=ShowStats)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, log_region, put, vtable  # noqa: E402
from window import RANGES, VTABLE, shown_env  # noqa: E402

CALLS = {'ShowStats': 0x4abfb0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench(RANGES + [(0x4abfb0, 0x4ac25e)])
DIALOG, DECOY = RAM + 0xb000, RAM + 0xb200
IDS = [0x1f3, 0x1f1, 0x1f6, 0x1f7, 0x1ee, 0x1f2, 0x1ef, 0x1f0]


def case(r):
    r.deal(OPS)
    log = log_region()
    regions = {LOG: log, VTABLE: vtable({5: 1})}
    wins = shown_env(r, regions, log)
    root = wins[0]
    put(regions[root], 0x8c, 'I', 0x1ed if r.random() < 0.9 else r.choice([0x1ec, 0x101ed, r.getrandbits(32)]))
    c = struct.unpack_from('<I', regions[root], 0x5c)[0]
    while c:
        put(regions[c], 0x8c, 'I', r.choice(IDS * 3 + [0x1ed, 0x1f4, 0x101f3, r.getrandbits(32)]))
        c = struct.unpack_from('<I', regions[c], 0x54)[0]
    for w in wins[1:]:
        if r.random() < 0.3:
            put(regions[w], 0x8c, 'I', r.choice(IDS))
    decoy = put(put(put(bytearray(r.bytes(0x120)), 0x8c, 'I', r.choice([0x1ed, r.getrandbits(32)])), 0x5c, 'I', 0), 0x54, 'I', root)
    shape = r.choice(['root'] * 6 + ['decoy'] * 2 + ['lone', 'none'])
    if shape == 'lone':
        put(decoy, 0x54, 'I', 0)
    regions[DECOY] = decoy
    dialog = bytearray(r.bytes(0x120))
    put(dialog, 0x5c, 'I', {'root': root, 'decoy': DECOY, 'lone': DECOY, 'none': 0}[shape])
    regions[DIALOG] = dialog
    regions[0x7cc1f8] = struct.pack('<I', DIALOG)
    mode = r.choice([0, 1, 2, 3] * 3 + [4, 0xffffffff, r.getrandbits(32)])
    return Call(CALLS['ShowStats'], [mode], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
