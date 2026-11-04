# The park map window's handler (src/game/world/parkmapwindow: 0x5f1130) against the original: the list's pop real, the rest recorders.
# usage: parkmapwindow.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, global_heap, heap, log_region, put, set_return, vtable  # noqa: E402

DELETE, SCROLL, PICTURE, DRAW, UNCACHE, VIEW, DEFAULT = 0, 8, 10, 11, 12, 17, 19
LAYOUT, REMOVE, RELEASE, RELEASE_ALL, FREE, FLAGS, RESTORE, TIMERS, SEND, ADVISOR, FOCUS, RESTORE_FOCUS, HELP = 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 7  # (a cdecl recorder's k is under 32)
B = Bench([(0x5f1130, 0x5f181c), (0x5f46a0, 0x5f46d6), (0x67acb0, 0x67acc0)],
          hooks={0x5f35b0: _recorder(SCROLL, 2), 0x5f1ea0: _recorder(PICTURE, 1), 0x5f2380: _recorder(DRAW, 0), 0x577990: _recorder(UNCACHE, 4), 0x5f1050: _recorder(VIEW, 0), 0x65f66e: _recorder(DEFAULT, 3),
                 0x5f3230: _recorder(LAYOUT, 0), 0x56b450: _recorder(REMOVE, 1), 0x56b340: _recorder(RELEASE, 0), 0x56b210: _recorder(RELEASE_ALL, 0), 0x5fa740: _recorder(FREE, 0), 0x5763f0: _recorder(FLAGS, 1),
                 0x409300: _recorder(RESTORE, 0), 0x662420: _recorder(TIMERS, 0, cdecl=True), 0x658d9f: _recorder(SEND, 4, cdecl=True), 0x486b40: _recorder(ADVISOR, 1, cdecl=True), 0x485b70: _recorder(FOCUS, 1, cdecl=True),
                 0x4862a0: _recorder(RESTORE_FOCUS, 0, cdecl=True), 0x492530: _recorder(HELP, 2, cdecl=True)})
MAP, WINDOW, LIST, NODES, OBJECTS, VT = RAM, RAM + 0x100, RAM + 0x140, RAM + 0x180, RAM + 0x200, RAM + 0x300
TAIL = MAP + 0x20060
COMMANDS = [0x982, 0x983, 0x984, 0x985, 0x986, 0x987, 0x988, 0x989, 0x98a, 0x98b, 0x993, 0x994, 0x995, 0xffffffff, 0xfffffffe]
BUTTONS = [0x98c, 0x98d, 0x98e, 0x98f, 0x990, 0x997, 0x998, 0x999, 0x99a, 0x98b]
MOVES = [0x981, 0x982, 0x984, 0x988, 0x98a, 0x98b]
KEYS = [0x2500, 0x2600, 0x2700, 0x2800, 0x6b00, 0x6d00, 0x2400, 0x6c00]
ZOOMS = [0.5, 1.0, 1.25, 1.5, 2.0, 2.5, 2.75, 3.0, 3.25, 3.5, 4.5, float('nan'), -1.0, 1e30]
EDGES = [0, 5, 10, 20, 21, 1024, 1500, 2027, 2028, 2030, 2040, 2047, 2048, 3000, 1530, 1535, 1536, 1537, 0xfffffffb]


def case(r):
    log = log_region()
    table = bytearray(0x600)
    for at, stub in global_heap().items():
        table[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(0x1000)}
    regions[0x78d910] = struct.pack('<B', r.choice([0, 0, 0, 1]))
    regions[0x7cb2ac] = struct.pack('<I', r.getrandbits(32))
    regions[0x87a6d0] = struct.pack('<I', r.getrandbits(32))
    regions[0x8bd47c] = struct.pack('<I', r.getrandbits(32))
    regions[0x786ba4] = struct.pack('<I', r.getrandbits(32))
    # The map's list of items: 0 to 3 objects, some null; the list object itself perhaps none.
    count = r.choice([0, 1, 2, 3])
    nodes = bytearray(0x60)
    for i in range(count):
        put(nodes, 0x10 * i, '3I', r.choice([OBJECTS + 0x10 * i, OBJECTS + 0x10 * i, 0]), NODES + 0x10 * (i + 1) if i + 1 < count else 0, NODES + 0x10 * (i - 1) if i else 0)
    regions[NODES] = nodes
    regions[OBJECTS] = struct.pack('<I12x', VT) * 3
    regions[VT] = vtable({0: 1})
    regions[LIST] = struct.pack('<4I', VT, NODES if count else 0, NODES + 0x10 * (count - 1) if count else 0, count)
    head = bytearray(r.bytes(0x80))
    put(head, 0x10, '2I', r.choice(EDGES), r.choice(EDGES))
    put(head, 0x5c, 'f', r.choice(ZOOMS))
    regions[MAP] = head
    tail = bytearray(r.bytes(0x30))
    put(tail, 0x20, 'I', LIST)
    regions[TAIL] = tail
    regions[WINDOW] = bytes(0x10)
    set_return(log, DEFAULT, r.getrandbits(32))
    message = r.choice([0x14, 0x15, 0x1e, 0x1e, 0x1e, 0x100, 0x100, 0x100, 0x101, 0x101, 0x10001, 0x10003, 0x10004, 0x10008, 0x10009, 0x1000a, 0x1000a, 0x1000b, 0x1000b, 0x1000c, 0x10002, 0x1f, 0x16, r.getrandbits(32)])
    a, b = r.getrandbits(32), r.getrandbits(32)
    if message == 0x100:
        a = r.choice(COMMANDS)
    elif message == 0x101:
        a, b = r.choice(BUTTONS), r.choice([0, 1, 1, 2])
    elif message in (0x10008, 0x10009):
        a = r.choice(MOVES)
    elif message == 0x1000a:
        a = r.choice(KEYS)
    elif message == 0x1000b:
        a, b = r.choice([0x1b, 0x1b, 0x1a]), r.choice([0, 0xff00ff, 0x10000, 0xff00ffff, 0x1234])
    return Call(0x5f1130, [MAP, WINDOW, message, a, b], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
