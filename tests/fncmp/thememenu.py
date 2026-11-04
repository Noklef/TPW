# The lobby's menus of themes (src/game/frontend/thememenu: 0x5e2010, 0x5e27b0) against the original: the strings real, the singletons, the theme records, the table and the items recorders; the fonts of a fake vector.
# usage: thememenu.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob, fill, global_heap, heap, log_region, put, recorder, set_return, strings, vtable  # noqa: E402

FONT_DTOR, REMOVE, M2W = 3, 4, 5
SLOTS, SAVE, GET, NAME, SUFFIX, LOCALISE, TEXTS, DELETE, ONLINE, VALUE, LOAD = 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18
B = Bench([(0x5e2010, 0x5e22d0), (0x5e27b0, 0x5e2a20), (0x424db0, 0x424e6e), (0x5c5480, 0x5c5510), (0x5c5520, 0x5c5530), (0x5f5660, 0x5f5690), (0x5f56f0, 0x5f5710)] + gamestring.B_RANGES,
          hooks={0x5c7420: _recorder(SLOTS, 0), 0x5aef50: _recorder(SAVE, 0), 0x5b09c0: _recorder(GET, 1), 0x5b1410: _recorder(NAME, 2), 0x5b1520: _recorder(SUFFIX, 2), 0x409b10: _recorder(LOCALISE, 3),
                 0x5d8470: _recorder(TEXTS, 2), 0x5f5d60: _recorder(DELETE, 2, cdecl=True), 0x5b50b0: _recorder(ONLINE, 0), 0x5bd270: _recorder(VALUE, 1), 0x5e3210: _recorder(LOAD, 5)})
SELF, OWNER, NODES, ITEMS, USERSLOTS, SESSION, FONTS, VT, TEXT = RAM, RAM + 0x100, RAM + 0x180, RAM + 0x200, RAM + 0x400, RAM + 0x600, RAM + 0x800, RAM + 0x900, RAM + 0xa00


def wide(text):
    return text.encode('utf-16-le') + bytes(2)


def case(r):
    log = log_region()
    table = bytearray(0x600)
    put(table, 0x6fd058 - IMPORT_TABLE, 'I', recorder(REMOVE, 1, True))
    put(table, 0x6fd174 - IMPORT_TABLE, 'I', recorder(M2W, 6, True))
    for at, stub in global_heap().items():
        table[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(0x4000)}
    back = 'back\0'.encode('utf-16-le')
    set_return(log, M2W, len(back) // 2)
    fill(log, M2W, 4, back, 2)
    strings(log, M2W, 4, 0b10000)
    strings(log, REMOVE, 1)
    strings(log, DELETE, 2, 1)
    # The list: 0 to 3 items.
    n = r.choice([0, 1, 2, 3])
    nodes = bytearray(r.bytes(0x40))
    items = bytearray(r.bytes(0x200))
    for i in range(n):
        put(nodes, 8 * i, '2I', ITEMS + 0x80 * i, NODES + 8 * (i + 1) if i + 1 < n else 0)
        put(items, 0x80 * i + 0x30, 'I', TEXT + 0x40 * i)
    regions[NODES] = nodes
    regions[ITEMS] = items
    regions[TEXT] = b''.join(b'item%d\0' % i + bytes(0x40 - 6) for i in range(4))
    regions[OWNER] = struct.pack('<II', r.getrandbits(32), NODES if n else 0)
    self = bytearray(r.bytes(0x100))
    put(self, 8, 'I', OWNER)
    regions[SELF] = self
    # The singletons, there or made.
    userslots = bytearray(r.bytes(0x110))
    put(userslots, 0x60, 'i', r.choice([-1, -1, 0, 1]))
    regions[USERSLOTS] = userslots
    set_return(log, SLOTS, USERSLOTS)
    session = bytearray(r.bytes(0x110))
    regions[SESSION] = session
    set_return(log, ONLINE, SESSION)
    set_return(log, SAVE, 0x1234500)
    set_return(log, GET, 0x1234600)
    regions[0xf81ebc] = struct.pack('<I', r.choice([0, USERSLOTS]))
    regions[0xf7d88c] = struct.pack('<I', r.choice([0, SESSION]))
    regions[0xf7c58c] = struct.pack('<I', r.choice([0, 0x1234500]))
    blob(log, GET, 0, 0xf)
    fill(log, NAME, 0, wide('nm'), 0)
    fill(log, SUFFIX, 0, wide('sfx'), 1)
    strings(log, NAME, 0, 1)
    strings(log, SUFFIX, 0, 1)
    strings(log, LOCALISE, 0, 0b110)
    blob(log, LOCALISE, 0, 0xf)
    blob(log, TEXTS, 0, 3)
    blob(log, TEXTS, 1, 3)
    set_return(log, VALUE, TEXT + 0x80)
    # The fonts: 0 to 2 strings of 0x14 bytes.
    count = r.choice([0, 0, 1, 2])
    fonts = bytearray(r.bytes(0x40))
    for i in range(count):
        put(fonts, 0x14 * i, 'I', VT)
        put(fonts, 0x14 * i + 8, 'I', TEXT + 0x40 * i)
    regions[FONTS] = fonts
    regions[VT] = vtable({3: 1})
    regions[0x789c1c] = struct.pack('<2I', FONTS, FONTS + 0x14 * count)
    if r.random() < 0.5:
        return Call(0x5e2010, [SELF], regions, conv='t', ret='v')
    # (+0xc is the first item of the list at +8, another list.)
    put(self, 8, 'I', RAM + 0x1e0)
    regions[RAM + 0x1e0] = struct.pack('<2I', r.getrandbits(32), RAM + 0x1f0)
    regions[RAM + 0x1f0] = struct.pack('<2I', r.getrandbits(32), r.getrandbits(32))
    return Call(0x5e27b0, [SELF, OWNER], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
