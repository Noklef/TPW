# The text layout engine (src/game/gfx/textlayout: 0x6ad5d0-0x6ad840, 0x6ade00-0x6adeb0, 0x6b0230-0x6b0270) against the originals: an
# object of random bytes at RAM (top byte 0x30) or in the game's data (top byte 0: the layout's constructor stores the top byte of `this`),
# its font (+4 of the style) 0 or one of two objects with recorder vtables (slot 9, the flags, answering a value with bit 1 set or clear),
# the vectors' blocks heap blocks, random or 0 (Mem_Free logs them), the deleting destructors' flags 0, 1 or random.
# usage: textlayout.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x5fa680, 0x5fa6b0), (0x6ad5d0, 0x6ad740), (0x6ad7c0, 0x6ad860), (0x6ade00, 0x6adf30), (0x6aeab0, 0x6aeb10), (0x6aeda0, 0x6aedf0),
           (0x6b0230, 0x6b0340)])
SOURCE = RAM + 0x400   # Assign's and SetStyle's source style
TABLE = 0x782d18       # SetStyle's renderers (0x782d38) and the dwords before them
RENDERER, VTABLE2 = RAM + 0x500, RAM + 0x480   # Render's renderer: slot 1 takes 5 arguments
ELEMS = RAM + 0x600    # Erase12's elements (12 bytes each)
DATA = 0x7d0000   # an object in the game's data (top byte 0)
FONTS, VTABLE, COLOUR, STATUS = RAM + 0x200, RAM + 0x300, RAM + 0x380, RAM + 0x3c0
# op: (address, the style's offset in the object, extra arguments)
CALLS = {'StyleCtor': (0x6ad5d0, 0), 'StyleDtor': (0x6ad640, 0), 'StyleDel': (0x6ad620, 0), 'SetFont': (0x6ad7c0, 0),
         'SetColour': (0x6ad6e0, 0), 'HolderCtor': (0x6b0230, 4), 'HolderDtor': (0x6b0270, 4), 'HolderDel': (0x6b0250, 4),
         'LayoutCtor': (0x6ade00, 8), 'LayoutDtor': (0x6adeb0, 8), 'LayoutDel': (0x6ade90, 8),
         'Assign': (0x6ad660, 0), 'GetFont': (0x6ad820, 0), 'FontKind': (0x6ad840, 0),
         'SetStyle': (0x6b0280, 4), 'Render': (0x6b0300, 4), 'CharClass': (0x6aeab0, 0), 'Erase12': (0x6aeda0, 0)}
RET = {'StyleCtor': 'a', 'StyleDel': 'a', 'SetFont': 'a', 'SetColour': 'a', 'HolderCtor': 'a', 'HolderDel': 'a', 'LayoutCtor': 'a',
       'LayoutDel': 'a', 'Assign': 'a', 'GetFont': 'a', 'FontKind': 'a',
       'SetStyle': 'a', 'Render': 'a', 'CharClass': 'b', 'Erase12': 'a'}


def case(r):
    op = r.deal(OPS)
    address, style = CALLS[op]
    obj = r.choice([RAM, DATA])
    log = log_region()
    set_return(log, 9, r.getrandbits(32) & r.choice([~2, 0xffffffff, 2]))
    set_return(log, 10, r.choice([0, 1, 2, 3, 4, 5, 6, -1, r.getrandbits(32)]))
    body = bytearray(r.bytes(0xc0))
    fonts = [FONTS, FONTS + 0x40]
    put(body, style + 4, 'I', r.choice(fonts + [0]))
    put(body, style + 8, 'I', r.getrandbits(32) & r.choice([~2, 0xffffffff, 2]))
    for off in (0x60, 0x74, 0x88):
        put(body, off, 'I', r.choice([0, HEAP + 0x40, r.getrandbits(32)]))
    regions = {obj: body, LOG: log, HEAP: heap(0x200), VTABLE: vtable({}),
               FONTS: b''.join(struct.pack('<I', VTABLE) + r.bytes(0x3c) for _ in fonts),
               COLOUR: r.bytes(8), STATUS: r.bytes(4)}
    args = [obj]
    if op == 'SetFont':
        args += [STATUS, r.choice(fonts + [0])]
    elif op == 'SetColour':
        args += [STATUS, COLOUR]
    elif op.endswith('Del'):
        args.append(r.choice([0, 1, r.getrandbits(32)]))
    elif op == 'Assign':
        source = bytearray(r.bytes(0x50))
        put(source, 4, 'I', r.choice(fonts * 6 + [0]))   # a source without a font crashes in both
        regions[SOURCE] = source
        args.append(r.choice([SOURCE] * 5 + [obj]))
    elif op == 'SetStyle':
        source = bytearray(r.bytes(0x50))
        put(source, 4, 'I', r.choice(fonts * 6 + [0]))
        put(source, 0xc, 'i', r.choice([0, 1, 2, 0, 1, 2, -1, 3, r.getrandbits(31)]))
        put(source, 0x10, 'I', r.choice([0, 1, 2]))
        set_return(log, 10, r.choice([0, 1, 2, 3, 4, 5, 6, -1, -2]))   # the index stays in TABLE (a wild read faults only in the emulator)
        regions[SOURCE] = source
        regions[TABLE] = b''.join(struct.pack('<I', r.choice([0, r.getrandbits(32)])) for _ in range(0x40))
        args += [STATUS, SOURCE]
    elif op == 'Render':
        put(body, 0x54, 'I', r.choice([0, RENDERER, RENDERER]))
        regions[VTABLE2] = vtable({1: 5})
        regions[RENDERER] = struct.pack('<I', VTABLE2)
        args += [STATUS, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)]
    elif op == 'CharClass':
        ch = r.choice([9, 10, 13, 0x20, r.randrange(0, 0x30), r.randrange(0, 0x30), r.getrandbits(16)]) | r.choice([0, r.getrandbits(16) << 16])
        return Call(address, [ch], regions, conv='s', ret='b')
    elif op == 'Erase12':
        n = r.randrange(0, 9)
        j = r.randrange(0, n + 1)
        i = r.randrange(0, j + 1)
        put(body, 4, 'II', ELEMS, ELEMS + 12 * n)
        regions[ELEMS] = r.bytes(12 * 8)
        args += [ELEMS + 12 * i, ELEMS + 12 * j]
    return Call(address, args, regions, conv='t', ret=RET.get(op, 'v'))


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
