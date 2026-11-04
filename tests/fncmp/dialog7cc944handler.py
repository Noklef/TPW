# The handler of the dialog [0x7cc944] (src/game/ui/dialog7cc944handler: 0x4bf9f0) against the original. Window_FindChild among decoys and Online_Instance real; the others recorders as driver.cpp's kDialog7cc944HandlerEnv.
# usage: dialog7cc944handler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from dialog7cb400 import kids  # noqa: E402
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob, fill, import_table, log_region, put, recorder, set_return, strings  # noqa: E402

FOCUS, CLEAR, SEND, POST, SEARCH, CLEAR_LIST, TEXT, ADD, NTH, OPEN, STATE, SET_STATE, SELECT, DEFAULT, DONE, DEBUG = range(10, 26)
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x4bf9f0, 0x4bfc6d), (0x65d7bc, 0x65d840), (0x48b390, 0x48b3fd)],
          hooks={0x65e59b: _recorder(FOCUS, 1), 0x5d6060: _recorder(CLEAR, 0), 0x658d9f: C(SEND, 4), 0x658ccc: C(POST, 4), 0x5bcbc0: _recorder(SEARCH, 3), 0x6649d5: _recorder(CLEAR_LIST, 0), 0x485b00: C(TEXT, 1),
                 0x66403b: _recorder(ADD, 4), 0x664c71: _recorder(NTH, 2), 0x4c0ae0: C(OPEN, 5), 0x668abd: _recorder(STATE, 0), 0x668bec: _recorder(SET_STATE, 2), 0x6677ae: _recorder(SELECT, 0), 0x65f66e: _recorder(DEFAULT, 3)})
WINDOW, DIALOG, HOLDER, SCENES, SESSION = (RAM + 0x1000 * k for k in range(1, 6))
KIDS1, KIDS2 = RAM + 0x10000, RAM + 0x14000


def case(r):
    log = log_region()
    regions = {LOG: log, HOLDER: r.bytes(8), SCENES: r.bytes(0x20), SESSION: r.bytes(0x100)}
    regions[0xf82884] = struct.pack('<I', HOLDER)
    regions[0xf85614] = struct.pack('<I', SCENES)
    regions[0xf7d88c] = struct.pack('<I', SESSION)
    first, top = kids(r, regions, KIDS1, (0xe059, 0xe05a, 0x77))
    regions[WINDOW] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', first)
    first, _ = kids(r, regions, KIDS2, (0xe05b, 0xe059))
    regions[DIALOG] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', first)
    regions[0x7cc944] = struct.pack('<II', r.deal([DIALOG, DIALOG, DIALOG, 0]), r.deal([0, recorder(DONE, 0, cdecl=True)]))
    regions[IMPORT_TABLE] = import_table(log, {'debug': (DEBUG, 0x6fd1e0, 1, 1, 0)})
    message = r.deal([0x14, 0x1f, 0x100, 0x100, 0x100, 0x100, 0x100, 0x400, 0x400, 0x400, 0x802, 0x802, 0x802, 0x1000b, 0x1000b, 0x101, 0x401, 0x803, 0x1000a, r.getrandbits(32)])
    if message == 0x100:
        a, b = r.deal([0xe059, 0xe059, 0xe059, 0xfffffffe, 0xe05a, 0xffffffff, 0x77]), r.deal([1, 1, 1, 0, 0x101, 2, 0xffffffff])
    elif message == 0x1000b:
        a, b = r.deal([0x1b, 0x1b, 0x1c, 0x11b]), r.getrandbits(32)
    else:
        a, b = r.deal([0xe059, 0x77, 0x77, 0xe05a, 0x1234]), r.deal([0xffffffff, 0, 1, 5, 0x80000000])
    fill(log, NTH, 1, struct.pack('<I', r.deal([0, 0xffffffff, 1, RAM + 0x20000, RAM + 0x20000, 0x80000000])))
    strings(log, NTH, 0, 1 << 1)
    blob(log, ADD, 0, 0x3f)
    set_return(log, TEXT, r.getrandbits(32))
    set_return(log, STATE, r.deal([0, 0, 0, 1, 0x100, 0x10000, 0x10001, 0xffffffff]))
    set_return(log, DEFAULT, r.getrandbits(32))
    return Call(0x4bf9f0, [WINDOW, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
