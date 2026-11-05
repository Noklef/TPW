# The lobby's entry (src/game/frontend/thememenu: 0x5e18d0) against the original: the wide strings, the stricmp and the small render setters real; the rest recorders; the live globals varied.
# usage: thememenuenter.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, blob, fill, global_heap, heap, log_region, put, recorder, set_return, strings, vtable  # noqa: E402

FONT_DTOR, REMOVE, M2W = 3, 4, 5
IS_ON, AFTER = 9, 10   # the vtable's +0x24 and +0x28
LOCALISE, DELETE, LOAD = 13, 15, 18
HOLDER, AMBIENT, LIGHT, INSTANCE, RESET, OPEN, SHOW, TIME, REFRESH, PLAY, APPLY = range(19, 30)
B = Bench([(0x5e18d0, 0x5e1bb0), (0x5e1bb0, 0x5e1bd0), (0x5d9680, 0x5d96c4), (0x424db0, 0x424e6e), (0x5c5480, 0x5c5510), (0x5c5520, 0x5c5530), (0x5f5660, 0x5f5690), (0x5f56f0, 0x5f5710), (0x67df40, 0x67e010)] + gamestring.B_RANGES,
          hooks={0x409b10: _recorder(LOCALISE, 3), 0x5f5d60: _recorder(DELETE, 2, cdecl=True), 0x5e3210: _recorder(LOAD, 5), 0x5d87d0: _recorder(HOLDER, 0), 0x5d9230: _recorder(AMBIENT, 2), 0x5d92e0: _recorder(LIGHT, 4),
                 0x48b390: _recorder(INSTANCE, 0, cdecl=True), 0x5b6070: _recorder(RESET, 0), 0x4b8ca0: _recorder(OPEN, 0, cdecl=True), 0x4b90f0: _recorder(SHOW, 0, cdecl=True), 0x5f5fa0: _recorder(TIME, 0, cdecl=True), 0x5e2010: _recorder(REFRESH, 0),
                 0x51bfc0: _recorder(PLAY, 6, cdecl=True), 0x51bd70: _recorder(APPLY, 0, cdecl=True)})
SELF, OWNER, NODES, THEMES, SCENE, ITEMS, FONTS, VT, TEXT, VIEW, TABLE, TABLE2, SOUNDS = RAM, RAM + 0x100, RAM + 0x180, RAM + 0x1c0, RAM + 0x200, RAM + 0x400, RAM + 0x800, RAM + 0x900, RAM + 0xa00, RAM + 0xc00, RAM + 0xd00, RAM + 0xe00, RAM + 0xf00
NAMES = [b'one', b'Two', b'three', b'ONE']


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
    strings(log, LOCALISE, 0, 0b110)
    blob(log, LOCALISE, 0, 0xf)
    blob(log, LIGHT, 1, 1)
    # The texts.
    regions[TEXT] = b''.join(name + bytes(0x40 - len(name)) for name in NAMES) + b'font\0' + bytes(0x3b)
    # The scene's list: strings, 0 to 3 of them.
    scene = r.choice([0, 1, 2, 3])
    strings_at = bytearray(0x80)
    nodes = bytearray(r.bytes(0x40))
    for i in range(scene):
        put(strings_at, 0x20 * i, '3I', r.getrandbits(32), len(NAMES[i]), TEXT + 0x40 * i)
        put(nodes, 8 * i, '2I', SCENE + 0x20 * i, NODES + 8 * (i + 1) if i + 1 < scene else 0)
    regions[SCENE] = strings_at
    regions[NODES] = nodes
    regions[0x786b90] = struct.pack('<I', NODES if scene else 0)
    # The menu's list: 1 to 3 items, one perhaps the chosen name's.
    count = r.choice([1, 2, 3])
    themes = bytearray(r.bytes(0x40))
    items = bytearray(r.bytes(0x200))
    for i in range(count):
        put(themes, 8 * i, '2I', ITEMS + 0x80 * i, THEMES + 8 * (i + 1) if i + 1 < count else 0)
        put(items, 0x80 * i + 0x30, 'I', TEXT + 0x40 * r.choice([0, 1, 2]))
        put(items, 0x80 * i + 0x18, 'I', r.choice([0, 1, 2, 3]))
    regions[THEMES] = themes
    regions[ITEMS] = items
    regions[OWNER] = struct.pack('<II', r.getrandbits(32), THEMES)
    regions[0xf85488] = struct.pack('<I', TEXT + 0x40 * r.choice([0, 1, 2, 3]))
    regions[0x803a4c] = struct.pack('<I', SOUNDS)
    regions[SOUNDS] = r.bytes(0x20)
    # The object.
    self = bytearray(r.bytes(0x100))
    put(self, 0, 'I', VT)
    put(self, 8, 'I', OWNER)
    regions[SELF] = self
    regions[VT] = vtable({3: 1})
    regions[TABLE] = struct.pack('<2I', *r.choice([(0, 0), (0, 0), (0, 0), (0, 1), (1, 0), (2, 2)])) + bytes(0x98)
    answers(log, IS_ON, TABLE)
    regions[TABLE2] = r.bytes(8) + bytes(0x98)
    answers(log, PLAY, TABLE2)
    set_return(log, TIME, r.getrandbits(32))
    set_return(log, INSTANCE, r.getrandbits(32))
    # The globals it reads live and the objects behind them.
    regions[0xf83774] = struct.pack('<I', r.getrandbits(32))
    regions[0xf83778] = struct.pack('<I', VIEW)
    regions[VIEW] = r.bytes(0x40)
    regions[0xf82884] = struct.pack('<I', VIEW + 0x40)
    regions[VIEW + 0x40] = struct.pack('<5I', 0, 0, 0, 0, VIEW + 0x60)
    regions[VIEW + 0x60] = struct.pack('<If', 0, r.choice([0.5, 1.0, r.uniform(-100, 100)]))
    regions[0x8bcbc8] = r.bytes(4)
    regions[0x8bd47c] = r.bytes(4)
    regions[0x8bd490] = r.bytes(8)
    # The fonts: 0 to 2 strings of 0x14 bytes.
    fonts_count = r.choice([0, 0, 1, 2])
    fonts = bytearray(r.bytes(0x40))
    for i in range(fonts_count):
        put(fonts, 0x14 * i, 'I', VT)
        put(fonts, 0x14 * i + 8, 'I', TEXT + 0x100)
    regions[FONTS] = fonts
    regions[0x789c1c] = struct.pack('<2I', FONTS, FONTS + 0x14 * fonts_count)
    return Call(0x5e18d0, [SELF], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
