# A sprite's file read (src/game/sprite: Sprite_LoadEsp 0x540d90) against the original: the file system a recorder whose opens answer in turn (ok or failing) and attach a memory source to the real stream, the
# first over the .esp's bytes (full, or ending before the actions), the second over the frames (spritesload.py's kinds, loaded for real); the path, the suffixes and the settings that choose among them; the heap failing the stream or the frame set now and then.
# usage: spriteloadesp.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import gamestring  # noqa: E402
import spritesload  # noqa: E402
from fncmp import HEAP, LOG, RAM, SEEK, STUBS, Bench, Call, _recorder, answers, fill, global_heap, heap, log_region, put, recorder, set_return, strings, vtable  # noqa: E402
from gamestring import string  # noqa: E402
from texdirs import merged  # noqa: E402

OPEN = 3
B = Bench(merged(gamestring.B_RANGES + [(0x41a840, 0x41a860), (0x540d90, 0x541220), (0x5646c0, 0x564720), (0x564730, 0x5647a0), (0x564a80, 0x564af0), (0x564ba0, 0x564c10), (0x564d00, 0x564e30), (0x564ff0, 0x565070), (0x56af20, 0x56afb0),
                                         (0x579b40, 0x579c30), (0x583720, 0x583740), (0x5839e0, 0x583a00), (0x587130, 0x587390), (0x587db0, 0x588330), (0x588320, 0x588350), (0x588350, 0x58850d), (0x588410, 0x588830), (0x588ba0, 0x588d6e),
                                         (0x58dc50, 0x58dd70), (0x590a00, 0x590a75), (0x590b90, 0x590c50), (0x590d40, 0x590f7e), (0x5c4f60, 0x5c4fb0), (0x618d30, 0x618d90), (0x4036a0, 0x4036e0), (0x639100, 0x639110), (0x5da3c0, 0x5da3c1)]),
          hooks={0x587c80: _recorder(spritesload.TICK, 0)})
SPRITE, ENTRY, FILES, ANSWERS, YES, NO = RAM, RAM + 0x400, RAM + 0x500, RAM + 0x600, RAM + 0x700, RAM + 0x704
SOURCE1, SOURCE2, VT_MEM = RAM + 0x800, RAM + 0x820, RAM + 0x840
DATA1, DATA2, STRINGS = RAM + 0x1000, RAM + 0x2000, RAM + 0x8000
GLOBALS = (0x8757a8, 0x875fe8, 0x876270, 0x875598)   # (the first's length is the path's suffix)
HEADER = 0x10e + 0x40


def source(at, data, size):
    return struct.pack('<5I', VT_MEM, data, size, data, size)


def esp_file(r):
    anims = bytearray(r.bytes(0x40))
    for i in range(16):
        anims[i * 4 + 2] = r.choice([0, 0, 1, 4, r.getrandbits(8)])
    actions = bytearray(r.bytes(16))
    for i in (0, 4, 8, 12):
        actions[i] = r.choice([0, 1, 7])
    data = r.bytes(0x10c) + bytes(r.choice([0, 1]) for _ in range(2)) + bytes(anims) + bytes(actions)
    return data[:r.choice([len(data)] * 3 + [HEADER, HEADER + 1, HEADER + 4, HEADER + 15, 0x10, HEADER - 5])]


def frames_file(r):
    kind = r.choice([3, 3, 2, 2, 0, 1, 0x103])
    count = r.choice([0, 1, 1, 2])
    data = struct.pack('<HHI', kind, r.getrandbits(16), count)
    if kind == 3:
        data += r.bytes(0x400)
    for _ in range(count):
        data += spritesload.rle(r) if kind == 3 else spritesload.wct(r) if kind == 2 else b''
    return data


def case(r):
    log = log_region()
    alphabet = b'abcdefgh\\.'
    path = bytes(r.choice(alphabet) for _ in range(r.choice([0, 4, 8, 20, 60]))) + r.choice([b'.esp', b'.ESP', b'x'])
    regions = {LOG: log}
    at = STRINGS
    for i, base in enumerate(GLOBALS):
        text = r.choice([b'.esp', b'.fpc', b'', b'\\x.tpc']) if i else r.choice([b'.esp', b'.esp', b'.esp', b'.x', b'', b'longer than the shortest of the paths'])
        string(r, False, base, at, regions, text)
        at += 0x80
    entry = bytearray(r.bytes(0x24)) + path + b'\0' + r.bytes(4)
    regions[ENTRY] = entry
    regions[SPRITE] = r.bytes(0x260)
    files = bytearray(r.bytes(0x10))
    put(files, 0, 'I', FILES + 0x10)
    regions[FILES] = bytes(files) + bytes(vtable({OPEN: 5}, size=8))
    strings(log, OPEN, 1 << 2, out=1 << 0)
    first, second = r.choice([YES] * 9 + [NO]), r.choice([YES] * 9 + [NO])
    regions[ANSWERS] = struct.pack('<40I', first, second, *[NO] * 38)
    answers(log, OPEN, ANSWERS)
    regions[YES] = struct.pack('<ii', r.choice([0, 1, 0x7fffffff]), r.choice([-1, -0x80000000]))
    fill(log, OPEN, 1, struct.pack('<4I', 0x703100, SOURCE1, 0x703100, SOURCE2), records=2)
    esp, frames = esp_file(r), frames_file(r)
    regions[DATA1], regions[DATA2] = esp + r.bytes(4), frames + r.bytes(4)
    regions[SOURCE1] = source(SOURCE1, DATA1, len(esp))
    regions[SOURCE2] = source(SOURCE2, DATA2, len(frames))
    vt = bytearray(r.bytes(0x28))
    put(vt, 0, '6I', recorder(10, 1), STUBS, STUBS + 0x10, SEEK, recorder(8, 0), recorder(9, 0))
    put(vt, 0x24, 'I', recorder(10, 0))
    regions[VT_MEM] = vt
    for k in (8, 9):
        set_return(log, k, r.getrandbits(32))
    regions[0x6fd12c] = struct.pack('<I', recorder(11, 1, True))   # InterlockedDecrement: 1 (the close's Release frees nothing)
    set_return(log, 11, 1)
    regions[HEAP] = heap(0x20000, fail=r.choice([0] * 18 + [2, 5, 6, 7]))
    regions.update(global_heap())
    settings = bytearray(r.bytes(0x4088))
    put(settings, 4, 'I', r.choice([2, 2, 2, 0, 1, 3]))
    regions.update({0x8bd550: settings, 0x8bd508: r.bytes(4), 0x879ad0: put(bytearray(r.bytes(0x204)), 0x200, 'i', r.choice([r.randrange(0x40), 0, 0x3f, 0x40])), 0x8798c8: bytes(4), 0x879a38: bytearray(r.bytes(0x2c)),
                    spritesload.TABLE: bytearray(8 * 8), spritesload.COUNT: bytes(4), 0x763f28: r.choice([b'\0', b'\1']), 0x8768fc: struct.pack('<I', r.choice([0, 1])), 0x877354: r.choice([b'\0', b'\1']), 0x87626c: r.bytes(4)})
    return Call(0x540d90, [SPRITE, ENTRY, FILES], regions, conv='t', ret='b')


if __name__ == '__main__':
    B.main(case)
