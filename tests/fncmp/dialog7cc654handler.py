# The mailing lists' handler (src/game/ui/dialog7cc654handler: Dialog7cc654_Handler 0x4bbc20) against the original. Window_FindChild among decoys, Online_Instance, wcscpy and wcscat real; recorders as driver.cpp's kDialog7cc654HandlerEnv, the rows' data filled in turn, the nodes a small ring.
# usage: dialog7cc654handler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from dialog7cb400 import kids  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, blob, fill, heap, log_region, put, set_return, strings  # noqa: E402

MARKED, RESTRICTED, POST, SEND, LEAVE, CLOSE, NTH, TEXTS, DATA, REFRESH, FREE, FOCUS, SOURCE, FIRST, NEXT, WIDE, FIT, SHOW, BASE = range(10, 29)
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x4bbc20, 0x4bc0fd), (0x65d7bc, 0x65d840), (0x48b390, 0x48b3fd), (0x67a400, 0x690000)],
          hooks={0x4bc430: C(MARKED, 0), 0x5b6460: _recorder(RESTRICTED, 0), 0x47f410: C(POST, 2), 0x658d9f: C(SEND, 4), 0x4bc500: C(LEAVE, 1), 0x5b6660: _recorder(CLOSE, 0), 0x664c71: _recorder(NTH, 2), 0x6644ea: _recorder(TEXTS, 5), 0x664b68: _recorder(DATA, 2), 0x663324: _recorder(REFRESH, 0),
                 0x5fa740: _recorder(FREE, 0), 0x4862a0: C(FOCUS, 0), 0x48bcd0: C(SOURCE, 0), 0x5cc5e0: _recorder(FIRST, 0), 0x5cc600: _recorder(NEXT, 0), 0x4bbb80: C(WIDE, 3), 0x4bc100: C(FIT, 0), 0x4bc270: C(SHOW, 0), 0x47ee90: C(BASE, 4)})
WINDOW, OTHER, HOLDER, OBJ, NODES, TEXT, SESSION, WORD, KIDS = (RAM + 0x1000 * k for k in range(1, 10))
FLAGS = [0, 0x1000000, 0x2000000, 0x3000000]


def wide(text):
    return text.encode('utf-16le') + b'\0\0'


def nodes(r, regions):
    n = r.deal([0, 1, 2, 3])
    at = [NODES + 0x20 * i for i in range(n + 1)]
    data, texts = bytearray(r.bytes(0x20 * (n + 1))), bytearray()
    for i in range(n + 1):
        put(data, 0x20 * i, 'I', at[(i + 1) % (n + 1)])
    for i in range(1, n + 1):
        form = r.choice(['text'] * 5 + ['null'])
        put(data, 0x20 * i + 0xc, 'I', 0 if form == 'null' else TEXT + len(texts))
        texts += (b'' if form == 'null' else b'list%d' % r.randrange(100)) + b'\0'
    regions[TEXT] = bytes(texts) or b'\0'
    regions[NODES] = data
    regions[OBJ] = put(bytearray(r.bytes(0x100)), 0x98, 'I', at[0])


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(), HOLDER: r.bytes(8), SESSION: r.bytes(0x10)}
    regions[0xf82884] = struct.pack('<I', HOLDER)
    regions[0xf7d88c] = struct.pack('<I', SESSION)
    first, top = kids(r, regions, KIDS, (0xee8, 0xee9))
    regions[WINDOW] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', first)
    other = r.bytes(0x100)
    regions[OTHER] = other
    regions[0x7cc654] = struct.pack('<I', WINDOW)
    put(regions[top[0xee8]], 0x150, 'i', r.deal([0, 1, 2, 3, -1]))
    put(regions[top[0xee8]], 0x14c, 'i', r.deal([0, 1, 7, -1]))
    message = r.deal([0x14, 0x15, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x404, 0x404, 0x404, 0x401, 0x401, 0x401, 0x401, 5, 0xff, 0x402, 0x403, 0x405, r.getrandbits(32)])
    a = r.deal([0xffffffff, 0xffffffff, 0xee5, 0xee6, 0xee7, 0xee4, 0xee8, 0]) if message == 0x100 else r.deal([0xee8, 0xee8, 0xee8, 0xee9, 0x77])
    b = r.deal([2, 2, 3, 1, 0, 0x7fffffff, -1 & 0xffffffff, 0x80000000]) if message == 0x404 else r.deal([0xffffffff, 0, 1, 2, r.getrandbits(32)])

    set_return(log, MARKED, struct.unpack('<I', struct.pack('<i', r.deal([0, 0, 1, 5, -1])))[0])
    set_return(log, RESTRICTED, r.deal([0, 0, 1, 0x100]))
    set_return(log, SOURCE, OBJ)
    set_return(log, BASE, r.getrandbits(32))
    data = bytearray()
    for _ in range(4):
        low = r.randrange(4) if message == 0x401 else r.deal([0, 1, r.getrandbits(24)])
        data += struct.pack('<I', low | r.deal(FLAGS) | (0 if message == 0x401 else r.deal([0, 0xf4000000, 0x80000000])))
    fill(log, NTH, 1, bytes(data), records=4)
    strings(log, NTH, 0, 1 << 1)
    blob(log, TEXTS, 1, 0xc)
    nodes(r, regions)
    regions[WORD] = wide(r.deal(['word', '', 'a longer word']))
    set_return(log, WIDE, WORD)
    strings(log, WIDE, 1)
    regions[0x78a374] = wide(r.deal(['', 'to:', 'the head of it']))
    regions[0x7cb4d0] = r.bytes(0x400)
    regions[0x7cc660] = r.bytes(4)
    return Call(0x4bbc20, [WINDOW, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
