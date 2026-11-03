# The chat client object (src/game/online/chatclient: 0x4e5950, 0x4e5be0, 0x4e5ef0) against the original: its members' destructors, the other functions and the library's imports recorders, the counted strings real on the bench's heap.
# usage: chatclient.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, LOG_BYTES, RAM, Bench, Call, _recorder, answers, blob_bytes, echo, fill, heap, import_table, log_region, mask_this, recorder, set_return, strings  # noqa: E402

SAVEB, SAVEI, CTOR, SEND, ELEM, NEXT, FIND, ERASE, D40, D30, D20, LIST, CHECK, FORMAT, ENTRANCE, PLAYER, HANDLE, BYHANDLE, PLACE, MODE, BASE, SETMODE = range(1, 23)
ONLINE = D30   # (Online_Instance's recorder in the driver; only the destructor calls D30)
START, STOP, NEW, CONNECT, LOGIN, DEBUG = range(23, 29)
PLACED, JOIN, SET, GO = START, STOP, NEW, CONNECT   # (another op's)
HOOKS = {0x4ef010: (SAVEB, 1), 0x4ef880: (SAVEI, 1), 0x5d29b0: (CTOR, 0), 0x5d2a60: (SEND, 0), 0x4f7430: (ELEM, 1), 0x6c2910: (NEXT, 0), 0x5ce530: (FIND, 2), 0x4f3400: (ERASE, 2), 0x4e58e0: (D40, 0), 0x4e5870: (D30, 0), 0x4e5800: (D20, 0), 0x5c8ba0: (LIST, 0),
         0x5d2a20: (CHECK, 0), 0x5b50b0: (ONLINE, 0), 0x4d8690: (ENTRANCE, 2), 0x4f7b20: (PLAYER, 4), 0x50b350: (HANDLE, 1), 0x4cd300: (BYHANDLE, 1), 0x4f7e20: (PLACE, 2), 0x46ce80: (MODE, 0), 0x46c6a0: (BASE, 0), 0x46c350: (SETMODE, 1)}
hooks = {at: _recorder(k, n) for at, (k, n) in HOOKS.items()}
hooks[0x67af00] = _recorder(FORMAT, 3, cdecl=True)
B = Bench([(0x4e5950, 0x4e5b82), (0x4e5be0, 0x4e5eeb), (0x4e5ef0, 0x4e6128), (0x4f23f0, 0x4f2437), (0x4bae30, 0x4bae3f), (0x41ce80, 0x41ce84), (0x41ce90, 0x41ce94), (0x5b6480, 0x5b6484), (0x5b6490, 0x5b6494), (0x5b64c0, 0x5b64c4), (0x405280, 0x405390), (0x45b590, 0x45b8c5), (0x479150, 0x4791b0), (0x479230, 0x479256), (0x47b750, 0x47bcf0), (0x67acd0, 0x67acee)],
          hooks=hooks)
SELF = RAM + 0x1000
DEQUE = RAM + 0x3000
SESSION = RAM + 0x4000
HOSTNAME = RAM + 0x5000
TEXT = RAM + 0x5100
OTHER = RAM + 0x5200
WORLD = RAM + 0x6000
THING = RAM + 0x7000
PLAYER_AT = RAM + 0x8000
MODE_AT = RAM + 0x8100
HANDLE_AT = RAM + 0x8200
OPS = ['dtor', 'connect', 'enter']


def text(r, heap_bytes, at, count):
    """A string's text in the heap region: its count byte just before it."""
    if r.random() < 0.3:
        return 0
    heap_bytes[at + count - 1] = r.choice([0, 0xff, 1, 2, 200])
    return HEAP + at + count


def wide(r, most=12):
    return ''.join(chr(r.randrange(0x21, 0x7f)) for _ in range(r.randrange(most))).encode('utf-16-le') + b'\0\0'


def imports(log, slots):
    table = import_table(log, {})
    table.extend(bytes(0x100))   # (past import_table()'s 0x400 bytes)
    for slot, (k, n) in slots.items():
        struct.pack_into('<I', table, slot - IMPORT_TABLE, recorder(k, n))
    return table


def dtor(r, log):
    table = imports(log, {0x6fd47c: (START, 0), 0x6fd4d8: (STOP, 0)})
    h = bytearray(heap(0x4000))
    self_bytes = bytearray(r.randbytes(0x1000))
    struct.pack_into('<I', self_bytes, 0, r.choice([0, HEAP + 0x100, HEAP + 0x100]))
    self_bytes[0x2b] = r.choice([1, 1, 0, 2, 0x81])
    struct.pack_into('<I', self_bytes, 0xf00, text(r, h, 0x200, 2))
    struct.pack_into('<I', self_bytes, 0x1c, text(r, h, 0x300, 1))
    struct.pack_into('<I', self_bytes, 0xc, text(r, h, 0x400, 2))
    end = DEQUE + 0x10 * r.randrange(1, 4)
    struct.pack_into('<I', self_bytes, 0xec8 + 8, end)
    struct.pack_into('<I', self_bytes, 0xec8 + 0xc, DEQUE)
    struct.pack_into('<I', self_bytes, 0xec8 + 0x2c, r.choice([0, 0, 1, 2, 5]))
    struct.pack_into('<I', self_bytes, 0xe60 + 4, r.choice([0, HEAP + 0x500]))      # (CountedBuf_Reset's text: its count byte)
    h[0x4ff] = r.choice([0, 0xff, 1])
    struct.pack_into('<I', self_bytes, 0xe70 + 4, r.choice([0, HEAP + 0x600]))
    h[0x5ff] = r.choice([0, 0xff, 1])
    struct.pack_into('<2I', self_bytes, 0xe60 + 0x58, HEAP + 0x700, r.getrandbits(32))
    fill(log, FIND, 0, struct.pack('<I', r.getrandbits(32)))
    echo(log, FIND, 0)
    strings(log, FIND, 0, out=1)
    strings(log, ERASE, 0, out=1)
    return Call(0x4e5950, [SELF], {LOG: log, IMPORT_TABLE: table, SELF: self_bytes, DEQUE: bytes(0x100), HEAP: h, 0x786de4: b'name\0'}, conv='t', ret='v')


def connect(r, log):
    table = imports(log, {0x6fd474: (NEW, 0), 0x6fd498: (CONNECT, 4), 0x6fd494: (LOGIN, 5)})
    struct.pack_into('<I', table, 0x6fd1e0 - IMPORT_TABLE, recorder(DEBUG, 1, True))
    h = bytearray(heap(0x4000, r.choice([0] * 12 + [1, 2, 3, 4])))
    self_bytes = bytearray(r.randbytes(0x1000))
    struct.pack_into('<I', self_bytes, 0, r.choice([0, SESSION + 0x100]))
    struct.pack_into('<I', self_bytes, 0x1c, r.choice([0, HOSTNAME]))
    struct.pack_into('<3I', self_bytes, 8 + 4, 0, 0, 0)   # (+8: an empty wide string)
    self_bytes[8] = r.getrandbits(8)
    session = bytearray(r.randbytes(0x100))
    struct.pack_into('<I', session, 0x14, HOSTNAME)
    struct.pack_into('<I', session, 0x28, OTHER)
    made = r.choice([SESSION] * 7 + [0])
    set_return(log, CHECK, r.choice([0, 1]))
    set_return(log, NEW, SESSION + 0x200)
    set_return(log, CONNECT, r.choice([0, 1, 1, 1]))
    answer_at = RAM + 0x9000
    answers(log, ONLINE, answer_at)
    fill(log, FORMAT, 0, wide(r))
    for k in (CTOR, CHECK):
        mask_this(log, k)
    blob_bytes(log, LOGIN, 0, 0xfff0fff0)   # (both strings' pointers, lengths and capacities: their flag bytes unset; the second's address masked)
    strings(log, LOGIN, 0, out=8)
    strings(log, CONNECT, 1)
    regions = {LOG: log, IMPORT_TABLE: table, SELF: self_bytes, SESSION: session, HOSTNAME: b'host\0', OTHER: wide(r), HEAP: h, answer_at: struct.pack('<40I', *([made, SESSION, 0, SESSION] * 10)), 0x75c0cc: b'fmt\0', 0x75c0fc: b'debug\0',
               0x6fe5b0: b'\0', 0x6fd6c4: b'\0\0', 0xf7d88c: struct.pack('<I', r.choice([0, SESSION])), 0x7a1552: r.randbytes(2), 0x7cc53c: bytes([r.choice([0, 1])])}
    return Call(0x4e5be0, [SELF], regions, conv='t', ret='v')


def enter(r, log):
    table = imports(log, {0x6fd4cc: (PLACED, 4), 0x6fd4bc: (JOIN, 1), 0x6fd4d4: (SET, 2), 0x6fd4c0: (GO, 1)})
    self_bytes = bytearray(r.randbytes(0x1000))
    struct.pack_into('<I', self_bytes, 0, SESSION)
    cell = r.choice([0, 1, 0x80, 0x81, 0x100, 0xff00, 0xffff, r.getrandbits(16), r.getrandbits(16)]) | r.getrandbits(16) << 16
    fill(log, ENTRANCE, 0, struct.pack('<I', cell))
    strings(log, ENTRANCE, 0, out=1)
    set_return(log, PLAYER, PLAYER_AT)
    fill(log, HANDLE, 0, r.randbytes(2))
    echo(log, HANDLE, 0)
    strings(log, HANDLE, 0, out=1)
    set_return(log, BYHANDLE, THING)
    set_return(log, MODE, MODE_AT)
    set_return(log, BASE, MODE_AT + 0x10)
    for k in (BYHANDLE, PLACE):
        struct.pack_into('<I', log, LOG_BYTES + 4 * k, 3 << 16)   # (words: the rest of each dword is unset on the original's stack)
    thing = bytearray(r.randbytes(0x10))
    thing[2] = r.choice([0x12, 0x12, 0x11, 0x92])
    session = bytearray(r.randbytes(0x100))
    set_return(log, ONLINE, SESSION)
    h = bytearray(heap(0x4000, r.choice([0, 0, 0, 2, 3, 4, 5])))   # (not the first: a null player's handle; the recorder's own answer)
    regions = {LOG: log, IMPORT_TABLE: table, SELF: self_bytes, SESSION: session, WORLD: bytes(0x400), THING: thing, HEAP: h, 0x7cf83c: struct.pack('<I', WORLD), 0xf7d88c: struct.pack('<I', r.choice([0, SESSION])),
               0x7b05e8: struct.pack('<I', r.choice([0, HEAP + 0x3000])), 0x7b05e0: r.randbytes(4), 0x7b05d4: r.randbytes(4)}
    return Call(0x4e5ef0, [SELF], regions, conv='t', ret='v')


def case(r):
    log = log_region()
    op = r.choice(OPS)
    for k in (CTOR, SEND):
        mask_this(log, k)
    return {'dtor': dtor, 'connect': connect, 'enter': enter}[op](r, log)


if __name__ == '__main__':
    B.main(case)
