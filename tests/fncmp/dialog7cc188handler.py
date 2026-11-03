# The address book's handler (src/game/ui/dialog7cc188handler: Dialog7cc188_Handler 0x4a73d0, Dialog7cc188_EraseName 0x4a7bc0) against the originals. Window_FindChild among decoys, the real list<string> (0-4 nodes) and the session, the CRT's text functions real; recorders as driver.cpp's kDialog7cc188HandlerEnv, slot 9 of a child 17.
# The row's data (the recorder fills it): negative for the session's name, or a string past the list's end. The recipients with the name in them or not, the text 0xd3 or not; the names empty now and then.
# usage: dialog7cc188handler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from dialog7cb400 import VT, kids  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob, blob_bytes, fill, heap, log_region, put, recorder, set_return, strings, vtable  # noqa: E402
from window705950draw import INTERLOCKED, LOCALE, interlocked  # noqa: E402

SOURCE, NTH, SEND, CANCEL, WIDE_TEXT, FORMAT, BOX, DEFAULT, ERASE, FILL_LIST, SLOT9 = range(10, 21)
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
HOOKS = {0x48bcd0: C(SOURCE, 0), 0x664c71: _recorder(NTH, 2), 0x658d9f: C(SEND, 4), 0x486b40: C(CANCEL, 1), 0x485b00: C(WIDE_TEXT, 1), 0x6acd60: C(FORMAT, 5), 0x47f020: C(BOX, 8), 0x65f66e: _recorder(DEFAULT, 3), 0x5cd660: _recorder(ERASE, 1), 0x4a7a80: C(FILL_LIST, 1)}
HOOKS.update({at: interlocked(step) for at, (_, step) in INTERLOCKED.items()})
B = Bench([(0x4a73d0, 0x4a77a0), (0x4a77a0, 0x4a77b9), (0x4a7bc0, 0x4a7be6), (0x5da3c0, 0x5da3c1), (0x5cd430, 0x5cd43e), (0x5cd440, 0x5cd509), (0x45b340, 0x45b9a0), (0x65d7bc, 0x65d840), (0x48b390, 0x48b3fd), (0x5b96b0, 0x5b96c0), (0x67a400, 0x690000), (0x6918f0, 0x691a30), (0x691ba0, 0x691ce0)], hooks=HOOKS)
OBJ, NODES, TEXT, WINDOW, DIALOG, SESSION, HOLDER, WIDE, PLACE, KIDS = (RAM + 0x1000 * k for k in range(1, 11))
NAMES = ['a@b.c', 'name@host.org', 'x@yy.z', 'some.long.name@some.host.example.org', 'p' * 70 + '@q.c']
PLACEHOLDER = 'to whom'


def wide(text):
    return text.encode('utf-16le') + b'\0\0'


def names(r, regions):
    """the list<string> at OBJ + 0x100"""
    n = r.deal([0, 1, 2, 3, 4])
    at = [NODES + 0x20 * i for i in range(n + 1)]
    nodes, texts = bytearray(r.bytes(0x20 * (n + 1))), bytearray()
    for i in range(n + 1):
        put(nodes, 0x20 * i, 'II', at[(i + 1) % (n + 1)], at[i - 1])
    for i in range(1, n + 1):
        form = r.choice(['text'] * 8 + ['null', 'empty'])
        put(nodes, 0x20 * i + 0xc, 'I', 0 if form == 'null' else TEXT + len(texts))
        texts += (r.choice(NAMES).encode() if form == 'text' else b'') + b'\0'
    regions[TEXT] = bytes(texts) or b'\0'
    regions[NODES] = nodes
    regions[OBJ] = put(bytearray(r.bytes(0x120)), 0x104, 'II', at[0], n)
    regions[0x786de4] = b'\0'
    return n


def case(r):
    log = log_region()
    imports = bytearray(0x400)
    for at, (slot, _) in INTERLOCKED.items():
        put(imports, slot - IMPORT_TABLE, 'I', at)
    regions = {LOG: log, HEAP: heap(), 0x78ba94: r.bytes(4), LOCALE: bytes(4), IMPORT_TABLE: imports, 0xfb3ca4: bytes(8)}  # (the CRT's wctomb: see window705950draw.py)
    set_return(log, SOURCE, OBJ)
    n = names(r, regions)
    if r.deal([0, 0, 0, 0, 1]):
        set_return(log, ERASE, r.getrandbits(32))
        return Call(0x4a7bc0, [r.deal([0, 1, 2, r.getrandbits(32)])], regions, ret='a')

    first, top = kids(r, regions, KIDS, (0x25, 0x26))
    regions[WINDOW] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', first)
    put(regions[top[0x26]], 0x14c, 'i', r.deal([0, 1, 5, 0x7fffffff, -1, -0x80000000]))
    inner, _ = kids(r, regions, KIDS + 0x1000, (0x5da5, 0x5da4))
    regions[DIALOG] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', inner)
    regions[VT] = put(vtable({}, size=0x10), 0x24, 'I', recorder(SLOT9, 1))
    regions[0x7cc184] = struct.pack('<2I', DIALOG, WINDOW)
    message = r.deal([0x14, 0x100, 0x100, 0x100, 0x400, 0x400, 0x400, 0x400, 0x401, 0x401, 0x401, 5, 0x101, 0xff, 0x3ff, 0x402, 0x80000100, r.getrandbits(32)])
    a = r.deal([0xffffffff, 0x24, 0x24, 0x24, 0x25, 0]) if message == 0x100 else r.deal([0x26, 0x26, 0x26, 0x25, 0x77])
    b = r.deal([0xffffffff, 0, 1, 2, r.getrandbits(32)])

    session_name = r.deal(['', 'me@here.org', NAMES[0], NAMES[1]])
    regions[WIDE] = wide(session_name)
    regions[0xf7d88c] = struct.pack('<I', SESSION)
    regions[SESSION] = put(bytearray(r.bytes(0x400)), 0x3a4, 'I', HOLDER)
    regions[HOLDER] = put(bytearray(r.bytes(0x100)), 0x9c, 'I', 0 if message != 0x400 and r.deal([0, 0, 0, 1]) else WIDE)
    fill(log, NTH, 1, struct.pack('<i', r.deal([0, 1, 2, 3, n, n + 2, -1, -0x80000000])))
    strings(log, NTH, 0, 1 << 1)

    regions[PLACE] = wide(PLACEHOLDER)
    set_return(log, WIDE_TEXT, PLACE)
    regions[0x7cbeb8] = r.bytes(0x100)
    recipients = r.deal([PLACEHOLDER, PLACEHOLDER, '', ';other@x.y', ';' + NAMES[0], ';' + NAMES[1] + ';' + NAMES[2], session_name, PLACEHOLDER + 'z', NAMES[3][:9]])
    regions[0x7cb4d0] = bytearray(wide(recipients)).ljust(0x400, b'\0')
    fill(log, FORMAT, 1, wide('are you sure'), which=1)
    strings(log, FORMAT, 0, 1 << 1)
    blob(log, FORMAT, 4, 0xf)
    blob_bytes(log, BOX, 0, 0xff70, (0, 0x7fff))
    set_return(log, DEFAULT, r.getrandbits(32))
    set_return(log, SEND, r.getrandbits(32))
    return Call(0x4a73d0, [WINDOW, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
