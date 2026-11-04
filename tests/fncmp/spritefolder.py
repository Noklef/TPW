# A folder's sprites loaded (src/game/sprite/loading: Sprites_LoadFolder 0x541310) against the original: the name one of those with a limit or another, the two listings finding files or not, the files' names the four avatars' or others', the loads answering by their low byte, the sprites' index near its end, the handle's allocation failing, and now and then more files than the table holds. The strings, the sort and the sprite's constructor real; the listing, the handles and the load recorders.
# usage: spritefolder.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import gamestring  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, answers, blob_bytes, fail_after, fill, heap, log_region, put, set_return, vtable  # noqa: E402

LEVEL, TIER, FIND, NEXT, HANDLE, ASSIGN, LOAD = 1, 2, 3, 4, 5, 6, 7
DELETE = 11   # the handle's vtable +0x2c
B = Bench([(0x541310, 0x541d83), (0x541210, 0x541302), (0x424cc0, 0x424d10), (0x5f5660, 0x5f5690), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000), (0x5c5480, 0x5c5523), (0x41ce80, 0x41ce84), (0x540bd0, 0x540c5c), (0x5da3c0, 0x5da3c1)],
          hooks={0x41a9d0: _recorder(LEVEL, 0), 0x41aa40: _recorder(TIER, 0), 0x41d570: _recorder(FIND, 2), 0x41d710: _recorder(NEXT, 1), 0x5f8240: _recorder(HANDLE, 0), 0x5f82b0: _recorder(ASSIGN, 1), 0x540d90: _recorder(LOAD, 2)})
FOLDER, NAME, NAME_TEXT, ROOT_TEXT, TAIL_TEXT, SLASH_TEXT, HANDLE_OBJECT, FINDS, NEXTS, LOADS = RAM, RAM + 0x100, RAM + 0x180, RAM + 0x200, RAM + 0x280, RAM + 0x300, RAM + 0x400, RAM + 0x500, RAM + 0x600, RAM + 0x700
FOUND = 0x875038
NAMES = [b'kids', b'KidsHeads', b'handymen', b'mechanics', b'Guards', b'researchers', b'kids', b'guards', b'other', b'', b'kid']
FILES = [b'SPR_BI.ESP', b'spr_ki.esp', b'SPR_TA.ESP', b'SPR_SU.ESP', b'zed.esp', b'abc.esp', b'SPR_BI.ES']


def names_record(r, count):
    return b''.join(r.choice(FILES).ljust(0x10, b'\0') for _ in range(count))


def case(r):
    log = log_region()
    blob_bytes(log, FIND, 0, 0xf0, inner=(8, (1 << 47) - 1))   # (the path, a String on the stack: its length and text)
    many = r.random() < 0.015
    name = r.choice([b'other', b'handymen', b'zoo']) if many else r.choice(NAMES)
    set_return(log, LEVEL, r.choice([0, 1, 1, 2, 3, 100, 0xffffffff]))
    set_return(log, TIER, r.choice([0, 1, 2, 2, 3, 100, 0xffffffff]))
    set_return(log, HANDLE, HANDLE_OBJECT)
    answers(log, FIND, FINDS)
    answers(log, LOAD, LOADS)
    if many:
        fail_after(log, NEXT, 0x85, 0, counted=True)
        fill(log, FIND, 1, b''.join(b'same.esp'.ljust(0x10, b'\0') for _ in range(2)), which=1, offset=0x24, records=2)
        finds = [1, 0] + [0] * 38
        loads = [1] * 40
    else:
        answers(log, NEXT, NEXTS)
        fill(log, NEXT, 0, names_record(r, 7), which=0, offset=0x24, records=7)
        fill(log, FIND, 1, names_record(r, 2), which=1, offset=0x24, records=2)
        finds = [r.choice([1, 1, 1, 1, 0, 2]), r.choice([1, 1, 1, 0, 3])] + [0] * 38
        loads = [r.choice([1, 1, 1, 0, 0x100, 0x101, 0xff00, 0xff]) for _ in range(40)]
    a, b = r.choice([0, 0, 1, 2, 3, 4]), r.choice([0, 0, 1, 2, 3])
    nexts = [r.choice([1, 2, 0x100, 0x80000000]) for _ in range(a)] + [0] + [r.choice([1, 2, 0x100]) for _ in range(b)] + [0]
    found = bytearray(r.bytes(0x128))
    put(found, 0x20, 'I', 0x87505c + r.choice([0, 0, 0, 0, 1, 4]))
    found[0x24:0x34] = r.choice(FILES).ljust(0x10, b'\0')
    table = {}
    regions = {LOG: log, HEAP: heap(0x20000 if many else 0x4000, fail=r.choice([0] * 26 + [9, 10, 11, 12, 13, 14, 15])), FOLDER: r.bytes(0x40), FOUND: found,
               FINDS: struct.pack('<40I', *finds), NEXTS: struct.pack('<40I', *(nexts + [0] * (40 - len(nexts)))), LOADS: struct.pack('<40I', *loads),
               HANDLE_OBJECT: struct.pack('<I', HANDLE_OBJECT + 0x40) + bytes(0x3c) + vtable({DELETE: 1}),
               0x875430: r.bytes(4), 0x875970: r.bytes(0x604), 0x876668: r.bytes(0x100),
               0x87626c: struct.pack('<I', 0x1ff if many else r.choice([0, 0, 1, 5, 0x1fe, 0x1ff, 0x200])), 0x876b48: r.bytes(0x810)}
    gamestring.string(r, 0, NAME, NAME_TEXT, regions, name)
    gamestring.string(r, 0, 0x8757f0, TAIL_TEXT, regions, r.choice([b'sprites', b'', b'a\\b']))
    gamestring.string(r, 0, 0xf80be0, SLASH_TEXT, regions, b'\\')
    gamestring.string(r, 1, 0x786b68, ROOT_TEXT, regions, r.choice([b'wad', b'', b'c:\\data']).decode().encode('utf-16le'))
    return Call(0x541310, [FOLDER, NAME], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
