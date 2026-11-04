# The mailing lists' marks acted on, changes sent and waiting box (src/game/ui/dialog7cc654lists: Dialog7cc654_ApplyMarks 0x4bc430, _SendChanges 0x4bc500, _WaitHandler 0x4bc660) against the original. Window_FindChild among decoys; the rest recorded.
# usage: dialog7cc654lists.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from dialog7cb400 import kids  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob_bytes, fill, log_region, put, set_return, strings  # noqa: E402

NTH, FIRST, NEXT, ERASE68, ERASE74, MARK1, MARK2, SOURCE, TEXT, PREPARE, CONNECT, PICK, BOX, HANDLER, SHOWN, POST, MESSAGE, BASE = range(10, 28)
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x4bc430, 0x4bc4ff), (0x4bc500, 0x4bc637), (0x4bc660, 0x4bc704), (0x65d7bc, 0x65d840)],
          hooks={0x664c71: _recorder(NTH, 2), 0x5cc5e0: _recorder(FIRST, 0), 0x5cc600: _recorder(NEXT, 0), 0x5cc700: _recorder(ERASE68, 0), 0x5cc830: _recorder(ERASE74, 0), 0x5cc640: _recorder(MARK1, 0),
                 0x5cc770: _recorder(MARK2, 0), 0x48bcd0: C(SOURCE, 0), 0x485b00: C(TEXT, 1), 0x5cd0a0: _recorder(PREPARE, 0), 0x5cc8a0: _recorder(CONNECT, 3), 0x4bae30: C(PICK, 0), 0x47f020: C(BOX, 8),
                 0x65e526: _recorder(HANDLER, 1), 0x65d9dd: _recorder(SHOWN, 1), 0x47f410: C(POST, 2), 0x658ccc: C(MESSAGE, 4), 0x47eed0: C(BASE, 4)})
WINDOW, OTHER, HOLDER, OBJ1, OBJ2, OBJ3, TABLE, HOST = (RAM + 0x1000 * k for k in range(1, 9))
KIDS1, KIDS2 = RAM + 0x10000, RAM + 0x18000
FLAGS = [0, 0x1000000, 0x2000000, 0x3000000]
OBJS = (OBJ1, OBJ2, OBJ3)


def apply_marks(r, log, regions):
    data = b''.join(struct.pack('<I', r.deal([0, 1, 2, 3]) | r.deal(FLAGS)) for _ in range(4))
    fill(log, NTH, 1, data, records=4)
    strings(log, NTH, 0, 2)   # (the out, on the stack, logged as 0)
    return Call(0x4bc430, [], regions, ret='a')


def send_changes(r, log, regions):
    regions[0x7a14f4] = struct.pack('<I', r.deal([0, 0, HOST, HOST, HOST]))
    regions[HOST] = r.deal([b'\0', b'mail.host\0', b'x\0']).ljust(0x20, b'\0')
    regions[0x6fe5b0] = r.deal([b'\0\0\0\0', b'\0\0\0\0', b'z\0\0\0'])
    regions[0x7a1550] = struct.pack('<h', r.deal([0, 25, -1, -0x7fff, 0x7fff]))
    set_return(log, TEXT, r.getrandbits(32))
    set_return(log, PICK, r.getrandbits(32))
    set_return(log, CONNECT, r.deal([0, 0, 1, 1, 1, 0x100, 0xffffff01, 0xffffff00]))
    set_return(log, BOX, RAM + 0x20000 + 16 * r.randrange(100))
    set_return(log, HANDLER, r.getrandbits(32))
    blob_bytes(log, BOX, 0, 0xff7f)
    return Call(0x4bc500, [r.deal([1, 1, 1, 1, 0, 0, 0x100, 0xffffff01, 0xffffff00])], regions, ret='v')


def wait_handler(r, log, regions):
    message = r.deal([0x14, 0x14, 0x14, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1f, 0x13, 0x15, 0x1d, 0x114, 0x11e, r.getrandbits(32)])
    same = r.getrandbits(32)
    cells = bytearray(regions[0x7cc650])
    put(cells, 0x38, 'I', r.deal([0, RAM + 0x20000, RAM + 0x20000, RAM + 0x20000]))
    pair = r.deal([0, 0, 1, 2, 3])
    put(cells, 0x8, 'I', same)
    put(cells, 0x34, 'I', same ^ (pair & 1))
    put(cells, 0x14, 'I', same + 7)
    put(cells, 0xc, 'I', same + 7 + (pair >> 1))
    regions[0x7cc650] = cells
    set_return(log, BASE, r.getrandbits(32))
    set_return(log, MESSAGE, r.getrandbits(32))
    return Call(0x4bc660, [r.getrandbits(32), message, r.getrandbits(32), r.getrandbits(32)], regions, ret='a')


def case(r):
    log = log_region()
    regions = {LOG: log}
    first, top = kids(r, regions, KIDS1, (0xee8,))
    second, _ = kids(r, regions, KIDS2, (0xbf432,))
    regions[WINDOW] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', first)
    regions[OTHER] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', second)
    regions[HOLDER] = put(bytearray(r.bytes(8)), 4, 'I', OTHER)
    regions[0xf82884] = struct.pack('<I', HOLDER)
    regions[0x7cc654 & ~0xf] = put(bytearray(r.bytes(0x40)), 4, 'I', WINDOW)
    for obj in OBJS:
        regions[obj] = put(bytearray(r.bytes(0x100)), 0xc4, 'I', r.deal([0, 0, 1, 0x80000000]))
    regions[TABLE] = b''.join(struct.pack('<I', r.choice(OBJS)) for _ in range(40))
    answers(log, SOURCE, TABLE)
    put(regions[top[0xee8]], 0x150, 'i', r.deal([0, 1, 2, 3, 4, -1]))
    make = r.deal([apply_marks, apply_marks, send_changes, send_changes, send_changes, wait_handler, wait_handler])
    return make(r, log, regions)


if __name__ == '__main__':
    B.main(case)
