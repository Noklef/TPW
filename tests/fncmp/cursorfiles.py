# The game's cursor files loaded (src/game/ui/cursorfiles: 0x4895b0, 0x489670) against the originals. Every call a recorder (their own benches have them): sprintf (its name filled, the folder a number and the name logged),
# Quickload_Locate, Mem_Alloc, the two constructors, 0x47ec80, LoadCursorFromFileA and the cursor's slot 1.
# usage: cursorfiles.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, fill, import_table, log_region, put, recorder, set_return, strings  # noqa: E402

FORMAT, LOCATE, ALLOC, SYSTEM, PICTURE, LOAD, SLOT1, CURSOR_FILE = range(10, 18)
NAME, CURSOR, TABLE = RAM + 0x1000, RAM + 0x2000, RAM + 0x3000

B = Bench([(0x4895b0, 0x489666), (0x489670, 0x489719)], hooks={0x67a650: _recorder(FORMAT, 4, cdecl=True), 0x44a280: _recorder(LOCATE, 3, cdecl=True), 0x67acc0: _recorder(ALLOC, 1, cdecl=True), 0x661935: _recorder(SYSTEM, 0),
                                                              0x47eab0: _recorder(PICTURE, 0), 0x47ec80: _recorder(LOAD, 1)})


def case(r):
    log = log_region()
    strings(log, FORMAT, 0b1000, 1)
    fill(log, FORMAT, 0, bytes(r.randrange(0x61, 0x7b) for _ in range(r.randrange(1, 12))) + b'\0')
    strings(log, LOCATE, 1, 0)
    set_return(log, LOCATE, r.deal([0, r.getrandbits(32) | 1, r.getrandbits(32) | 1, r.getrandbits(32) | 1]))
    set_return(log, ALLOC, r.deal([0, r.getrandbits(32) | 1, r.getrandbits(32) | 1, r.getrandbits(32) | 1]))
    made = r.deal([0, CURSOR, CURSOR, CURSOR])
    set_return(log, SYSTEM, made)
    set_return(log, PICTURE, made)
    imports = import_table(log, {'LoadCursorFromFileA': (CURSOR_FILE, 0x6fd314, 1, 0, 0)})
    set_return(log, CURSOR_FILE, r.deal([0, r.getrandbits(32) | 1, r.getrandbits(32) | 1]))
    regions = {LOG: log, IMPORT_TABLE: imports, NAME: bytes(r.randrange(0x61, 0x7b) for _ in range(6)) + b'\0', CURSOR: struct.pack('<I', TABLE), TABLE: put(bytearray(8), 4, 'I', recorder(SLOT1, 2)), 0x74fc30: r.bytes(8)}
    return Call(r.deal([0x4895b0, 0x489670]), [NAME], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
