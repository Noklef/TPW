# The online dialog's chat rooms' head counts (src/game/online/roomcounts: RoomCounts_ChatEvent 0x4c4920, RoomCounts_Update 0x4c4bb0) against the originals. Real strings (shared or, with a full owners' count, copied), the CRT's atoi and the game's String real; the Chat's imports recorders: constructor 11, Connect 12, RoomInfo 13 (the string's text hashed), Release 14, OutputDebugString 15; the session's constructor 10 (driver.cpp's kOnlineInstanceEnv), slot 7 of the children 16.
# usage: roomcounts.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from dialog7cb400 import VT, kids  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob_bytes, heap, log_region, put, recorder, set_return, vtable  # noqa: E402
from window705950draw import INTERLOCKED, LOCALE, interlocked  # noqa: E402

HOOKS = {0x5b50b0: _recorder(10, 0)}
HOOKS.update({at: interlocked(step) for at, (_, step) in INTERLOCKED.items()})
B = Bench([(0x4c4920, 0x4c4fb0), (0x4bae30, 0x4bae50), (0x5bcb60, 0x5bcb8b), (0x45b340, 0x45b9a0), (0x5f5fc0, 0x5f7f00), (0x65d7bc, 0x65d840), (0x67a400, 0x690000), (0x6918f0, 0x691a30), (0x691ba0, 0x691ce0), (0x5da3c0, 0x5da3c1)], hooks=HOOKS)
ARGS, FIRST, TEXT, SESSION, CITY, LIST, NODES, ROOMS, HOST, CHAT, DIALOG, GROUP = (RAM + 0x400 * k for k in range(12))
KIDS = RAM + 0x8000
IDS = [0, 1, 7, 100, 12345, -3, 0x7fffffff]


def common(r, log):
    imports = bytearray(0x500)
    for at, (slot, _) in INTERLOCKED.items():
        put(imports, slot - IMPORT_TABLE, 'I', at)
    put(imports, 0x6fd474 - IMPORT_TABLE, 'I', recorder(11, 0))
    put(imports, 0x6fd498 - IMPORT_TABLE, 'I', recorder(12, 4))
    put(imports, 0x6fd478 - IMPORT_TABLE, 'I', recorder(13, 2))
    put(imports, 0x6fd47c - IMPORT_TABLE, 'I', recorder(14, 0))
    put(imports, 0x6fd1e0 - IMPORT_TABLE, 'I', recorder(15, 1, True))
    state = bytearray(r.bytes(0x60))
    for k in range(5):
        put(state, 0x18 + 4 * k, 'i', r.choice(IDS))
        put(state, 4 * k, 'i', r.choice([0, 1, 5, 0x123456]))
    put(state, 0x48, 'I', r.choice([0, 1, 1]))                 # [0x7cdb10]
    put(state, 0x50, 'i', r.choice([0, 3, 4, 4, 5, 100, -1]))  # [0x7cdb18]
    put(state, 0x54, 'I', r.choice([0, 0, 1]))                 # [0x7cdb1c]
    return {LOG: log, HEAP: heap(), IMPORT_TABLE: imports, LOCALE: bytes(4), 0xfb3ca4: bytes(8), 0x7cdac8: state}


def string(r, regions, record, offset, text, owners):
    block = len(regions[TEXT])
    regions[TEXT] += bytes([owners]) + text + b'\0'
    put(record, offset + 4, 'I', TEXT + block + 1)
    put(record, offset + 8, 'I', len(text))
    put(record, offset + 0xc, 'I', max(len(text), 31))


def event(r):
    log = log_region()
    regions = common(r, log)
    state = regions[0x7cdac8]
    records = bytearray(r.bytes(0x24 * 6))
    regions[TEXT] = bytearray()
    ids = struct.unpack_from('<5i', state, 0x18)
    number = r.choice(list(ids) * 3 + [99, 4242])
    text = r.choice([str(number).encode()] * 8 + [b'', b'  %d' % number, b'%dx' % number, b'abc'])
    kind = r.choice([7, 7, 7, 7, 0x2c, 0x2c, 0x2c, 0x2c, 6, 8, 0x2d, 0])
    put(records, 0x20, 'i', kind)
    put(records, 0xb0, 'i', r.choice([0, 1, 50, r.getrandbits(31)]))
    string(r, regions, records, 0x6c, text, r.choice([0, 0, 1, 2, 0xfd, 0xfe, 0xff]))
    string(r, regions, records, 0x24, text, r.choice([0, 0, 1, 2, 0xfd, 0xfe, 0xff]))
    size = r.choice([0, 1, 2, 2, 2, 3, 6, -1])
    regions[FIRST] = records
    regions[ARGS] = struct.pack('<4I', r.getrandbits(32), FIRST, FIRST + (0x24 * size if r.random() < 0.9 else 0x24 * size + r.randrange(1, 0x24)) & 0xffffffff, r.getrandbits(32))
    regions[TEXT] = bytes(regions[TEXT]) or b'\0'
    typ = r.choice([0x64] * 14 + [-6, -5, -4, -3, 0x63, 0x65, 0])
    return Call(0x4c4920, [typ, r.getrandbits(32), ARGS], regions, conv='s', ret='v')


def update(r):
    log = log_region()
    regions = common(r, log)
    state = regions[0x7cdac8]
    if r.random() < 0.85:
        put(state, 0x48, 'I', 1)
    n = r.choice([1, 2, 4, 5, 5, 6, 7, 0, -1])
    rows = max(n, 1)
    nodes, rooms = bytearray(r.bytes(8 * rows)), bytearray(r.bytes(0x10 * rows))
    for k in range(rows):
        put(nodes, 8 * k, 'II', ROOMS + 0x10 * k, NODES + 8 * (k + 1) if k + 1 < rows else 0)
        put(rooms, 0x10 * k + 8, 'i', r.choice([0, 1, 42, -7, 2147483647, r.getrandbits(31)]))
    made = r.random() < 0.15
    if not made:
        regions[HEAP] = heap(fail=r.choice([0] * 6 + [1, 2]))
    regions[0xf7d88c] = struct.pack('<I', 0 if made else SESSION)
    set_return(log, 10, SESSION)
    regions[SESSION] = put(bytearray(r.bytes(0x100)), 0xf4, 'I', CITY)
    regions[CITY] = put(put(bytearray(r.bytes(0x40)), 0x28, 'I', r.choice([1, 1, 1, 1, 1, 1, 0, 0x100])), 0x2c, 'I', LIST)
    regions[LIST] = put(put(bytearray(r.bytes(0x10)), 4, 'I', NODES), 0xc, 'i', n)
    regions[NODES], regions[ROOMS] = nodes, rooms
    regions[HOST] = b'chat.example\0'
    regions[0x7a1504] = struct.pack('<I', r.choice([HOST, HOST, 0])) + r.bytes(0x4a) + struct.pack('<H', r.choice([0, 80, 0x7fff, 0x8000, 0xfffe]))
    regions[0x7cc53c] = struct.pack('<I', r.choice([0, 1]))
    set_return(log, 11, r.choice([CHAT] * 6 + [0]))
    set_return(log, 12, r.choice([1] * 6 + [0, 0x100]))
    blob_bytes(log, 13, 0, 0, inner=(4, 0x3f))
    regions[VT] = put(vtable({}, size=0x10), 0x1c, 'I', recorder(16, 1))
    first, top = kids(r, regions, KIDS, (0x16007,))
    regions[DIALOG] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', first)
    inner, _ = kids(r, regions, KIDS + 0x1000, (0x16008, 0x16009, 0x1600a, 0x1600b, 0x1600c))
    put(regions[top[0x16007]], 0x5c, 'I', inner)
    regions[0x7cdb0c] = struct.pack('<I', r.choice([DIALOG] * 5 + [0]))
    return Call(0x4c4bb0, [], regions, ret='v')


def case(r):
    return (event if r.random() < 0.5 else update)(r)


if __name__ == '__main__':
    B.main(case)
