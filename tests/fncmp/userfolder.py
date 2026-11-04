# The user slots' folders (src/game/files/userslots: 0x5c7d30, 0x5c7de0, 0x5c8870, 0x5c8890, 0x5b8160) against the originals: gamepaths.py's users folder world, five records of 24 bytes (the one before the
# object for a slot of -1) with short wide names, now and then none (a null string appended: a fault in both), the slot chosen any of them, the theme's label any text, the three globals of the path any text.
# _itow (0x67e010, unported) a recorder writing up to three digits. 0x5b8160's instance: there, or made: 0x5c7420 a recorder answering it.
# usage: userfolder.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,Folder,Chosen,Theme,Path)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, RAM, _recorder, blob, fill, put, set_return  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamepaths  # noqa: E402
import gamestring  # noqa: E402

CTOR, ITOW = 27, 28
B = Bench([(0x5b8160, 0x5b8260), (0x5c7d30, 0x5c7e90), (0x5c8870, 0x5c8980), (0x5c7c70, 0x5c7cfd), (0x5c7bd0, 0x5c7c64), (0x424e10, 0x424e6e), (0x5c5520, 0x5c5530), (0x5f5660, 0x5f5690), (0x5f56f0, 0x5f5710)] + gamestring.B_RANGES,
          hooks={0x5c7420: _recorder(CTOR, 0), 0x67e010: _recorder(ITOW, 3, cdecl=True)})
OPS = [k for k in os.environ.get('OPS', 'Name,Folder,Chosen,Theme,Path').split(',') if k]
BEFORE = RAM + 0x6000
SLOTS = BEFORE + 0x18
LETTERS = 'abZ_1 '


def text(r, wide=True):
    t = ''.join(r.choice(LETTERS) for _ in range(r.choice([0, 1, 2, 3, 8, 17])))
    return t.encode('utf-16-le' if wide else 'latin1')


def world(r):
    c = gamepaths.save_folder(r, 0x5c7c70)
    regions = c.regions
    obj, part = bytearray(r.bytes(0x18 + 0x110)), {}
    for i in range(5):
        name = (text(r) or b'x\0') if r.random() < 0.95 else b''
        gamestring.string(r, 1, BEFORE + 0x18 * i, RAM + 0x6200 + 0x80 * i, part, name)
        obj[0x18 * i:0x18 * i + 0x14] = part.pop(BEFORE + 0x18 * i)[:0x14]
    regions.update(part)
    regions[BEFORE] = obj
    slot = r.choice([-1, 0, 1, 2, 3, 3])
    put(obj, 0x18 + 0x60, 'i', slot)
    log = regions[LOG]
    fill(log, ITOW, 1, str(slot + 1 if r.random() < 0.8 else r.randrange(1000)).encode('utf-16-le') + bytes(2), 1)   # (fill 0 is MultiByteToWideChar's)
    blob(log, ITOW, 1, 0)   # (the stack's address)
    set_return(log, CTOR, SLOTS)
    gamestring.string(r, 0, 0x786b68, RAM + 0x6500, regions, text(r, False))
    for i, address in enumerate((0xf7d740, 0xf7d668, 0xf7cf30)):
        gamestring.string(r, 1, address, RAM + 0x6600 + 0x80 * i, regions, text(r))
    return regions, slot


def case(r):
    regions, slot = world(r)
    op = r.deal(OPS)
    out = gamepaths.OUT_W
    if op == 'Name':
        return Call(0x5c7d30, [SLOTS, out, slot], regions, conv='t', ret='a')
    if op == 'Folder':
        return Call(0x5c7de0, [SLOTS, out, slot], regions, conv='t', ret='a')
    if op == 'Chosen':
        return Call(0x5c8870, [SLOTS, out], regions, conv='t', ret='a')
    if op == 'Theme':
        return Call(0x5c8890, [SLOTS, out], regions, conv='t', ret='a')
    regions[0xf81ebc] = r.choice([SLOTS, SLOTS, 0]).to_bytes(4, 'little')
    return Call(0x5b8160, [out], regions, conv='s', ret='a')


if __name__ == '__main__':
    B.main(case)
