# The dialog [0x7cc944]'s list filled (src/game/ui/dialog7cc944list: 0x4bfc70) against the original. Every callee a recorder as driver.cpp's kDialog7cc944ListEnv; Window_FindChild, the
# wide string's assign (0x4051d0 in the original, its parts in the port), the String and WString code real; the rows hashed, the theme's text through its pointer.
# usage: dialog7cc944list.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, blob, blob_bytes, fill, heap, log_fold, log_region, put, set_return, strings, vtable  # noqa: E402

CLEAR, WIDE, ADD, SAVECTOR, BYTE25, FILTER, ONLINECTOR, VALUE, LABEL, FIND, STATE = range(10, 21)
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x4bfc70, 0x4bff6a), (0x65d7bc, 0x65d832), (0x4051d0, 0x405280), (0x405280, 0x405390), (0x479230, 0x479270), (0x4f23f0, 0x4f2450), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000), (0x6918f0, 0x691a30), (0x691ba0, 0x691ce0), (0x5da3c0, 0x5da3c1)],
          hooks={0x6649d5: _recorder(CLEAR, 0), 0x485b00: C(WIDE, 1), 0x66403b: _recorder(ADD, 4), 0x5aef50: _recorder(SAVECTOR, 0), 0x5b09b0: _recorder(BYTE25, 0), 0x5d3230: _recorder(FILTER, 2),
                 0x5b50b0: _recorder(ONLINECTOR, 0), 0x5bd270: _recorder(VALUE, 1), 0x4099b0: _recorder(LABEL, 2), 0x5bc9c0: _recorder(FIND, 1), 0x668bec: _recorder(STATE, 2)})
KVTABLE, DIALOG, LISTW, STATEW, REPLY, NODES, RECORDS, TEXTS, THEME, ENTRY, ONLINE, SAVE = RAM + 0x1000, RAM + 0x1100, RAM + 0x1300, RAM + 0x1500, RAM + 0x1700, RAM + 0x1800, RAM + 0x2000, RAM + 0x3000, RAM + 0x3800, RAM + 0x3900, RAM + 0x4000, RAM + 0x4400
NODE = 0x180
FROZEN = RAM + 0x5000


def node(r, ident, nxt, child=0):
    buf = bytearray(r.bytes(NODE))
    put(buf, 0, 'I', KVTABLE)
    put(buf, 0x54, 'I', nxt)
    put(buf, 0x5c, 'I', child)
    put(buf, 0x8c, 'I', ident)
    return buf


def case(r):
    log = log_region()
    log_fold(log)
    regions = {LOG: log, KVTABLE: vtable({}), HEAP: heap(0x4000, r.choice([0] * 12 + [r.randrange(3, 12)]))}
    regions[DIALOG] = node(r, 1, 0, LISTW)
    regions[LISTW] = node(r, 0xe05b, STATEW)
    regions[STATEW] = node(r, 0xe059, 0)
    regions[0x7cc944] = struct.pack('<I', r.choice([DIALOG] * 15 + [0]))
    n = r.choice([0, 1, 1, 2, 3])
    count = r.choice([n, n, n, n, n + 1, 0])
    reply = bytearray(r.bytes(0x10))
    put(reply, 4, 'I', NODES if n else 0)
    put(reply, 0xc, 'I', count)
    regions[REPLY] = reply
    nodes = bytearray(8 * max(n, 1))
    records = bytearray(r.bytes(0x100 * max(n, 1)))
    texts = bytearray(0x80 * max(n, 1))
    for i in range(n):
        put(nodes, 8 * i, 'II', RECORDS + 0x100 * i, NODES + 8 * (i + 1) if i + 1 < n else 0)
        put(records, 0x100 * i + 0x14, 'I', TEXTS + 0x80 * i)
        length = r.randrange(0, 30)
        for j in range(length):
            put(texts, 0x80 * i + 2 * j, 'H', r.randrange(0x21, 0x7e) | (r.getrandbits(8) << 8) | 0x100)
    regions.update({NODES: nodes, RECORDS: records, TEXTS: texts})
    regions[THEME] = b'theme of the park 123456\0'
    regions[ENTRY] = r.bytes(0x20)
    regions[0xf7c58c] = struct.pack('<I', r.choice([SAVE, SAVE, 0]))
    regions[0xf7d88c] = struct.pack('<I', r.choice([ONLINE, ONLINE, 0]))
    regions[0x786b68] = r.bytes(4)
    online = bytearray(r.bytes(0x100))
    put(online, 0xe0, 'I', RAM + 0x4200)
    put(online, 0xf8, 'I', RAM + 0x4300)
    regions[ONLINE] = online
    set_return(log, WIDE, RAM + 0x3a00)
    set_return(log, SAVECTOR, SAVE)
    set_return(log, ONLINECTOR, ONLINE)
    set_return(log, BYTE25, r.choice([0, 0, 1, 0x100, r.getrandbits(32)]))
    set_return(log, VALUE, THEME)
    set_return(log, FIND, ENTRY)
    blob(log, ADD, 0, 0x3f)
    strings(log, FILTER, 0, out=1)
    regions[FROZEN] = bytes(0x100)
    if r.choice([False, False, True]):  # (the filter leaves a frozen string: count 0xff)
        regions[FROZEN] = b'\0\xff' + bytes(0xfe)
        fill(log, FILTER, 0, struct.pack('<I', FROZEN + 2), offset=4)
    strings(log, LABEL, 0, out=2)
    blob_bytes(log, LABEL, 0, 0, (8, 0xffff))
    return Call(0x4bfc70, [REPLY], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
