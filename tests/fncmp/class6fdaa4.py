# The class 0x6fdaa4's .sam / .txt opens of src/game/core/class6fd9d8 against the originals (0x41e5d0-0x41e720, 0x41f0d0-0x41f350): a file name is built in game strings
# from the object's name (+4), a given String or the language buffer, with the prefixes / suffixes of the game's global strings ("Easy_", "Online_", "coaster", ".sam",
# ".txt"), and opened through an object's slot 3 (the handle at +0x2c, or one given): a recorder that logs the name's text (the result pointer, a stack address, as 0),
# writes a result (0, 1 or negative) through its first argument and returns that argument. The strings are of the small alphabet of gamestring.py; the heap region gives
# their blocks.
# usage: class6fdaa4.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import gamestring  # noqa: E402
from fncmp import Bench, Call, HEAP, LOG, RAM, echo, fill, heap, log_region, put, strings, vtable  # noqa: E402

# name: (address, convention, arguments: o the object, b its name (+4) as a base, h an opener object, n a name String, s the stream argument; pops)
CALLS = {'SamAt': (0x41e5d0, 's', 'bhs'), 'Sam': (0x41e660, 't', 'ohs'), 'OnlineSamAt': (0x41e680, 's', 'bhs'), 'OnlineSam': (0x41e720, 't', 'ohs'),
         'OpenSam': (0x41f0d0, 't', 'ons'), 'OpenOwnSam': (0x41f170, 't', 'os'), 'OpenCoasterSam': (0x41f330, 't', 'os'),
         'OpenEasySam': (0x41f190, 't', 'os'), 'OpenOnlineSam': (0x41f260, 't', 'os'), 'OpenLanguageTxt': (0x41f350, 't', 'os')}
B = Bench([(0x41e5d0, 0x41e7c0), (0x41f0d0, 0x41f420), (0x41a820, 0x41a830), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000)])
OBJ, OPENER, NAME, VT, TEXTS = RAM, RAM + 0x40, RAM + 0x80, RAM + 0xc0, RAM + 0x1000
# The game's global strings (String objects in .bss), their texts: ".sam" twice (the second another text here, so that the two are told apart), "Easy_", "Online_", "coaster", ".txt".
GLOBALS = {0x78c780: b'.sam', 0x78c738: b'.SaM2', 0x78bb98: b'Easy_', 0x78c990: b'Online_', 0x78c708: b'coaster', 0x78bec8: b'.txt'}
LANGUAGE = 0xf7f230   # Language_Buffer's (0x41a820) text
K_OPEN = 3


def case(r):
    name = r.choice(OPS)
    address, conv, kinds = CALLS[name]
    log = log_region()
    strings(log, K_OPEN, 1 << 2, 1 << 0)   # (the name's text; the result pointer as 0)
    fill(log, K_OPEN, 0, struct.pack('<i', r.choice([0, 0, 1, -1, -2, 0x7fffffff, -0x80000000])))
    echo(log, K_OPEN, 0)
    regions = {LOG: log, HEAP: heap(0x2000), VT: vtable({K_OPEN: 5})}
    obj = bytearray(r.bytes(0x40))
    k = 0
    for off in (4, 0x18):   # the object's two strings (+4 its name)
        gamestring.string(r, 0, RAM + 0x800, TEXTS + 0x100 * k, regions)
        obj[off:off + 0x14] = regions.pop(RAM + 0x800)[:0x14]
        k += 1
    put(obj, 0x2c, 'II', VT, r.getrandbits(32))   # (the handle: a recorder vtable)
    regions[OBJ] = obj
    regions[OPENER] = struct.pack('<I', VT) + r.bytes(0x20)
    for at, text in GLOBALS.items():
        regions[TEXTS + 0x100 * k] = text + b'\0' + r.bytes(8)
        regions[at] = struct.pack('<5I', gamestring.VT[0][0], len(text), TEXTS + 0x100 * k, (len(text) & ~7) + 8, 8)
        k += 1
    regions[LANGUAGE] = gamestring.units(r, 0, r.choice([0, 1, 3, 9])) + b'\0' + r.bytes(8)
    args = []
    for x in kinds:
        if x == 'o':
            args.append(OBJ)
        elif x == 'b':
            args.append(OBJ + 4)
        elif x == 'h':
            args.append(OPENER)
        elif x == 'n':
            gamestring.string(r, 0, NAME, TEXTS + 0x100 * k, regions)
            args.append(NAME)
        elif x == 's':
            args.append(r.choice([0, 1, 0x80000001, r.getrandbits(32)]))
    return Call(address, args, regions, conv=conv, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
