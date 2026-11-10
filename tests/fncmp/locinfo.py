# The C++ library's std::_Lockit and std::_Locinfo (src/game/core/locinfo) against the originals, over the C library's setlocale and the strings' ports: the Win32 calls COM recorders in their import slots, the locks, the exit table's routines, _nh_malloc and the categories' init routines cdecl recorders, the heap's Mem_Alloc and free.
# Bench limit: [0xfb1968] is not 1 on entry, as no recorder can end _Lockit's wait.
# usage: locinfo.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _free, _recorder, answers, heap, import_table, log_region, recorder, set_return  # noqa: E402

LOCKEXIT, UNLOCKEXIT, MSIZE, REALLOC, LOCK, UNLOCK, INIT, NHMALLOC = 3, 4, 5, 6, 7, 8, 9, 28
EXCHANGE, INITCS, ENTERCS, LEAVECS, ENUM, INFO, VALIDCP, VALIDLOCALE, LCID, VERSION = 14, 15, 16, 17, 18, 19, 20, 21, 22, 23
DELETECS = 24
B = Bench([(0x691de0, 0x691e66), (0x691e70, 0x691e97),(0x691ea0, 0x691eb5), (0x698fc0, 0x6990fa), (0x699100, 0x6991e2), (0x45b590, 0x45b5e0), (0x45b680, 0x45b6a0), (0x45b6a0, 0x45b7c0), (0x45b7d0, 0x45b8c5),
           (0x683ac0, 0x683d88), (0x683d90, 0x683ee5), (0x683ef0, 0x68400b), (0x684010, 0x684202), (0x684210, 0x684254), (0x684260, 0x684367), (0x684370, 0x6843da), (0x68db70, 0x68dbae), (0x67c250, 0x67c288), (0x68dbb0, 0x68dd41),
           (0x68dd50, 0x68ddad), (0x68ddb0, 0x68de3a), (0x68e120, 0x68e17e), (0x68e260, 0x68e29e), (0x68e340, 0x68e35b), (0x68e360, 0x68e443), (0x68e510, 0x68e549), (0x68e6c0, 0x68e6e1),
           (0x67d7f0, 0x67d82a), (0x67dcd0, 0x67dcf0), (0x67df40, 0x67e010), (0x67a480, 0x67a519), (0x688730, 0x68876d), (0x688770, 0x6887d1), (0x67ae00, 0x67aefe), (0x67a740, 0x67a7f0)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True), 0x67ad90: _free, 0x67fa20: _recorder(LOCKEXIT, 0, cdecl=True), 0x67fa30: _recorder(UNLOCKEXIT, 0, cdecl=True),
                 0x6818f0: _recorder(MSIZE, 1, cdecl=True), 0x681740: _recorder(REALLOC, 2, cdecl=True)})
A_MALLOC, A_INIT, EXIT_TABLE, SELF, STRINGS, TEXT, OWNED = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x400, RAM + 0x500, RAM + 0x700, RAM + 0x800
BLOCKS = [HEAP + 0x2000 + 0x100 * i for i in range(40)]
C = 0x780dc8
CATEGORIES = [b'LC_ALL', b'LC_COLLATE', b'LC_CTYPE', b'LC_MONETARY', b'LC_NUMERIC', b'LC_TIME']
OPS = [n for n in ('lockit', 'lockit', 'unlockit', 'term', 'ctor', 'ctor', 'ctor', 'dtor', 'dtor', 'dtor') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def locale_regions(r, regions, log):
    """The C library's locale state: every category "C", or one other for some."""
    answers(log, NHMALLOC, A_MALLOC)
    answers(log, INIT, A_INIT)
    regions[A_MALLOC] = struct.pack('<40I', *[BLOCKS[i] if r.random() < 0.97 else 0 for i in range(40)])
    regions[A_INIT] = struct.pack('<40I', *[0 if r.random() < 0.95 else 1 for _ in range(40)])
    regions[0xfb1438] = struct.pack('<6I', 0, 0, 0, 0, 0, 0)
    regions[0xfb1450] = struct.pack('<I', 0x4e4)
    regions[0xfb3ca4] = struct.pack('<2I', 0, 0)
    regions[0x708be0] = b';\0\0\0=;\0\0=\0\0\0_.,\0'
    regions[0x74c77c] = b'_\0\0\0'
    regions[0x747800] = b'.\0\0\0'
    regions[0x747828] = b'*\0\0\0'
    regions[0x7096ec] = b'OCP\0ACP\0'
    regions[0x6fe5b0] = bytes(0x10)   # (_Nullstr)
    strings = bytearray(0x200)
    pointers = []
    for i, text in enumerate(CATEGORIES):
        strings[i * 0x20:i * 0x20 + len(text)] = text
        pointers.append(STRINGS + i * 0x20)
    regions[STRINGS] = strings
    table = bytearray(0x48)
    other = HEAP + 0x1000
    regions[HEAP][0x1000:0x1007] = b'French\0'
    mixed = r.random() < 0.3
    init = recorder(INIT, 0, cdecl=True)
    for i in range(6):
        locale = 0 if i == 0 else (other if mixed and i == 3 else C)
        struct.pack_into('<3I', table, i * 12, pointers[i], locale, init)
    regions[C] = b'C\0\0\0\0\0\0\0' + bytes(table)
    cache = bytearray(0x110)
    cache[0:3] = b'fra'
    cache[0x88:0x88 + 6] = b'French'
    regions[0x780e18] = cache
    keys = bytearray(0x208)
    regions[0x781760] = keys
    regions[0x781968] = bytes(0xb8)


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x8000),
               IMPORT_TABLE: import_table(log, {'exchange': (EXCHANGE, 0x6fd200, 2, 0, 0), 'initcs': (INITCS, 0x6fd144, 1, 0, 0), 'entercs': (ENTERCS, 0x6fd140, 1, 0, 0), 'leavecs': (LEAVECS, 0x6fd130, 1, 0, 0), 'enum': (ENUM, 0x6fd1f8, 2, 0, 0),
                                                'info': (INFO, 0x6fd28c, 4, 0, 0b100), 'validcp': (VALIDCP, 0x6fd288, 1, 0, 0), 'validlocale': (VALIDLOCALE, 0x6fd284, 2, 0, 0), 'lcid': (LCID, 0x6fd260, 0, 0, 0), 'version': (VERSION, 0x6fd264, 1, 0, 1),
                                                'deletecs': (DELETECS, 0x6fd0d0, 1, 0, 0)})}
    regions[0xfb1950] = bytes(0x18) + struct.pack('<I', r.choice([0, 2, 2, 7]))
    set_return(log, EXCHANGE, r.choice([0, 0, 2, 1]))
    if name == 'term':
        set_return(log, EXCHANGE, r.choice([0, 1, 2, 2, 3]))
        return Call(0x691e70, [], regions, conv='c', ret='v')
    if name in ('lockit', 'unlockit'):
        regions[0xfb3cb0] = struct.pack('<II', EXIT_TABLE, EXIT_TABLE)
        regions[EXIT_TABLE] = r.bytes(0x100)
        set_return(log, MSIZE, r.choice([0x20, 0x20, 0]))
        set_return(log, REALLOC, r.choice([0, EXIT_TABLE]))
        return Call(0x691de0 if name == 'lockit' else 0x691ea0, [SELF], regions, conv='t', ret='a' if name == 'lockit' else 'v')
    locale_regions(r, regions, log)
    regions[0xfb18fc] = struct.pack('<I', recorder(INFO, 4, True))
    if name == 'ctor':
        regions[SELF] = r.bytes(0x40)
        regions[TEXT] = r.choice([b'C\0', b'fra\0', b'French\0', b'x\0', b'x_y_z_w\0']) + bytes(0x20)
        return Call(0x698fc0, [SELF, r.choice([TEXT, TEXT, TEXT, 0, TEXT + 0x100])], regions, conv='t', ret='a')
    self_ = bytearray(0x40)
    block_text = r.choice([b'C', b'French', b'x', b'fra'])
    owned = bytearray(0x100)
    pointers = []
    for i in range(4):
        at = OWNED + 0x40 * i
        owners = r.choice([0, 0, 1, 2, 0xfe, 0xff])
        owned[0x40 * i] = owners
        owned[0x40 * i + 1:0x40 * i + 2 + len(block_text)] = block_text + b'\0'
        ptr = r.choice([at + 1, at + 1, at + 1, 0])
        length = r.choice([len(block_text), len(block_text), 0]) if i == 2 else 3
        struct.pack_into('<4I', self_, i * 0x10, r.getrandbits(32), ptr, length, 0x1f)
    regions[OWNED] = owned
    regions[SELF] = self_
    return Call(0x699100, [SELF], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
