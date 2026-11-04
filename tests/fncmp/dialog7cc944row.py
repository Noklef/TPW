# The online game's list's waiting row (src/game/ui/dialog7cc944row: 0x4c0660) against the original. Every call a recorder (their own benches have them): the row's data filled, the texts' second logged.
# usage: dialog7cc944row.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, _recorder, blob, fill, log_region, set_return, strings  # noqa: E402

FIND, DATA, PRINT, TEXTS = range(10, 14)
B = Bench([(0x4c0660, 0x4c06c7)], hooks={0x65d7d0: _recorder(FIND, 1), 0x664c71: _recorder(DATA, 2), 0x67af00: _recorder(PRINT, 3, cdecl=True), 0x6644ea: _recorder(TEXTS, 5)})


def case(r):
    log = log_region()
    set_return(log, FIND, r.getrandbits(32))
    strings(log, DATA, 0, 2)
    fill(log, DATA, 1, struct.pack('<I', r.deal([0xffffffff, 0xffffffff, 0, r.getrandbits(32)])))
    strings(log, TEXTS, 0, 2)
    blob(log, TEXTS, 1, 0b10)
    regions = {LOG: log, 0x7cc944: struct.pack('<I', r.deal([0, r.getrandbits(32) | 1, r.getrandbits(32) | 1, r.getrandbits(32) | 1]))}
    return Call(0x4c0660, [r.getrandbits(32)], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
