# The park map's mouse handler (src/game/world/parkmapwindow: 0x5f18d0) against the original: the strings real, the rest recorders.
# usage: parkmapmouse.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob, blob_when, fill, global_heap, heap, log_region, put, set_return, vtable  # noqa: E402

SCROLL, TEXT_SET, PICTURE, DRAW, UNCACHE, STAFF, FORMAT, SIGN, FIND, VIEW, PANEL, DEFAULT = 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19
B = Bench([(0x5f18d0, 0x5f1ea0), (0x5da3c0, 0x5da3c1), (0x65d55a, 0x65d56b)] + gamestring.B_RANGES,
          hooks={0x5f35b0: _recorder(SCROLL, 2), 0x5f1ea0: _recorder(PICTURE, 1), 0x5f2380: _recorder(DRAW, 0), 0x577990: _recorder(UNCACHE, 4), 0x506860: _recorder(STAFF, 0),
                 0x6acd60: _recorder(FORMAT, 5, cdecl=True), 0x4e2780: _recorder(SIGN, 2), 0x65d7d0: _recorder(FIND, 1), 0x5f1050: _recorder(VIEW, 0), 0x486920: _recorder(PANEL, 1, cdecl=True),
                 0x65f66e: _recorder(DEFAULT, 3)})
WINDOW, PARENT, CHILD, VT, LIST, NODES, ITEMS, THINGS, NAME, MAP = RAM, RAM + 0x100, RAM + 0x180, RAM + 0x1c0, RAM + 0x240, RAM + 0x280, RAM + 0x300, RAM + 0x800, RAM + 0x2000, RAM + 0x4000


def wide(text):
    return text.encode('utf-16-le') + bytes(2)


def case(r):
    log = log_region()
    table = bytearray(0x600)
    for at, stub in global_heap().items():
        table[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(0x4000)}
    width, height = r.choice([(640, 480), (800, 600), (1024, 768), (r.getrandbits(32), r.getrandbits(32))])
    regions[0x7a1968] = struct.pack('<I', width)
    regions[0x7a1930] = struct.pack('<I', height)
    regions[0x78d910] = struct.pack('<B', r.choice([0, 1]))
    regions[0x78ba94] = struct.pack('<I', 0x1234500)
    regions[0x7ba058] = bytes(0x100)
    # Things of all kinds; the slots of the thing table name them.
    things = bytearray(r.bytes(0x200 * 4))
    for i in range(4):
        put(things, 0x200 * i + 2, 'B', r.choice([0, 1, 1, 2, 4, 5, 6, 7, 8, 9]))
        put(things, 0x200 * i + 0x1d8, 'I', r.choice([0, r.getrandbits(32)]))
    regions[THINGS] = things
    slots = bytearray()
    for i in range(8):
        slots += struct.pack('<I', THINGS + 0x200 * r.randrange(4)) + r.bytes(16)
    put(slots, 20 * r.randrange(8), 'I', 0)
    regions[0x7cfb90] = slots
    # The items, some of them near the mouse.
    a = r.randrange(0x800) << 16 | r.randrange(0x600)
    x, y = (a >> 16) * (width & 0xffff) / 2048, (0x600 - (a & 0xffff)) * (height & 0xffff) / 1536
    count = r.choice([0, 1, 2, 3])
    nodes, items = bytearray(0x40), bytearray(r.bytes(0x40 * 4))
    for i in range(4):
        put(items, 0x40 * i + 0xc, '2f', x + r.uniform(-8, 8), y + r.uniform(-8, 8))
        put(items, 0x40 * i + 0x18, 'I', r.choice([0, THINGS, THINGS + 0x200, THINGS + 0x400, THINGS + 0x600]))
        put(items, 0x40 * i + 0x2c, 'I', r.choice([0, 1, 1, 1]))
    for i in range(count):
        put(nodes, 8 * i, '2I', ITEMS + 0x40 * i, NODES + 8 * (i + 1) if i + 1 < count else 0)
    regions[NODES] = nodes
    regions[ITEMS] = items
    regions[LIST] = struct.pack('<2I', r.getrandbits(32), NODES if count else 0)
    # The map: its squares' words small enough for the thing table.
    pattern = b''.join(r.bytes(4) + struct.pack('<H', r.randrange(8)) + r.bytes(2) for _ in range(8))
    m = bytearray(pattern * (0x20088 // len(pattern) + 1))[:0x20088]
    put(m, 0, '24s', r.bytes(24))
    put(m, 0x10, '2I', r.getrandbits(11), r.getrandbits(11))
    put(m, 0x24, '2i', r.choice([128, 128, 64, 0]), r.choice([128, 128, 64, 0]))
    put(m, 0x48, '2I', r.getrandbits(32), r.getrandbits(32))
    put(m, 0x54, '3f', r.uniform(0, 128), r.uniform(0, 128), r.choice([1.0, 0.5, 2.0, 0.25]))
    put(m, 0x20060, 'I', r.choice([0, ITEMS, ITEMS + 0x40, ITEMS + 0xc0]))
    put(m, 0x20064, 'I', r.randrange(8))
    put(m, 0x20080, '2I', LIST, r.choice([0, 1]))
    regions[MAP] = m
    regions[WINDOW] = struct.pack('<I', 0) * 0x18 + struct.pack('<I', PARENT)
    regions[PARENT] = bytes(0x40)
    regions[CHILD] = struct.pack('<I', VT)
    regions[VT] = vtable({9: 1})
    set_return(log, FIND, CHILD)
    regions[NAME] = wide(r.choice(['', 'B', 'Bob']))
    set_return(log, STAFF, NAME)
    fill(log, FORMAT, 1, wide(r.choice(['', 'F', 'Fmt'])), 0)
    fill(log, SIGN, 0, wide(r.choice(['', 'S', 'Sgn'])), 1)
    blob(log, TEXT_SET, 0, 3)
    blob(log, FORMAT, 4, 0x1f, only_when=True)
    blob_when(log, FORMAT, 3, 0x1c8, 0x1f)
    set_return(log, DEFAULT, r.getrandbits(32))
    message = r.choice([0x1e, 0x1e, 0x10001, 0x10001, 0x10001, 0x10003, 0x10003, 0x10004, 0x10005, 0x10006, 0x10006, 0x10006, 0x10002, 0x10007, 0x1d, r.getrandbits(32)])
    if message in (0x10004, 0x10005, 0x10006):
        a = r.choice([0, 1, a])
    return Call(0x5f18d0, [MAP, WINDOW, message, a, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
