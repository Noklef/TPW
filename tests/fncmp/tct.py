# A level's terrain texture sets read from its .tct file (src/game/terrain/tct: 0x427620) against the original. The sets: 0-3 groups named
# from a few (QueueTex and PathTex among them), their counts and pointers any, the bytes needed (+0xc) mostly 0 to start. The file through
# vfs.py's mounts and memory source, read twice (the memory source holds it twice over, so that each pass gets it whole; now and then the
# second copy cut short): lines naming groups (any case, now and then one not there or too long), tiles (a number, then a name), comments
# ('#', ';'), other punctuation, blank runs and random bytes, the last line with or without its line end.
# usage: tct.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, RAM, Bench, Call, global_heap, heap, log_region, put, set_return  # noqa: E402
import vfs  # noqa: E402
from gamestring import LOCALE  # noqa: E402
from texdirs import merged  # noqa: E402

B = Bench(merged(vfs.RANGES + [(0x427620, 0x427ac2), (0x4273c0, 0x427615), (0x5c4d50, 0x5c5020), (0x45b9d0, 0x45ba30), (0x67a400, 0x690000)]))
SETS, GROUPS, NAMES = RAM + 0x60000, RAM + 0x60100, RAM + 0x60200
GROUP_NAMES = [b'QueueTex', b'PathTex', b'Grass', b'Rock2']


def flip(r, text):
    return bytes(c ^ 0x20 if (0x41 <= c <= 0x5a or 0x61 <= c <= 0x7a) and r.random() < 0.3 else c for c in text)


def line(r, names):
    k = r.random()
    if k < 0.2:
        return flip(r, r.choice(names + [b'Nothing', b'Path', b'A' * 300, b'Queue9Tex'])) + r.choice([b'\n', b'\r\n', b'  # x\n'])
    if k < 0.65:
        number = r.choice([b'1', b'12', b'0', b'99999999999999', b'3.5', b'7-1', str(r.randrange(1000)).encode()])
        name = r.choice([b'grass01', b'Tile_a.tga', b'x', b'rock-2', b'b' * 300, b'a;b', b'#c'])
        return number + r.choice([b' ', b'\t', b'   ', b'', b' \x80 ']) + name + r.choice([b'\n', b'\r\n', b' trailing\n', b''])
    if k < 0.8:
        return r.choice([b'#', b';']) + r.choice([b' comment', b'', b'1 a']) + b'\n'
    if k < 0.9:
        return r.choice([b'-', b'!', b'[', b'~']) + r.choice([b'5 tile', b'', b'x']) + r.choice([b'\n', b''])
    return bytes(r.randrange(256) for _ in range(r.randrange(8)))


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x8000), LOCALE: bytes(4), **global_heap()}
    root = vfs.mounts(r, log, regions)
    n = r.choice([0, 1, 2, 2, 3])
    names = r.sample(GROUP_NAMES, n)
    groups = bytearray(r.bytes(0xc * 3))
    text = bytearray()
    for i, name in enumerate(names):
        put(groups, 0xc * i, 'I', NAMES + 0x10 * i)
        text += name + b'\0'
        text += bytes(0x10 - len(name) - 1)
    regions[NAMES] = bytes(text) + bytes(0x10)
    regions[GROUPS] = bytes(groups)
    sets = bytearray(r.bytes(0x18))
    put(sets, 0, 'II', n, GROUPS)
    put(sets, 0xc, 'I', r.choice([0, 0, 0, 0x10, 3]))
    regions[SETS] = sets
    data = b''.join(line(r, names) for _ in range(r.choice([0, 1, 3, 8, 16])))
    if r.random() < 0.1:
        data = data.rstrip(b'\n')
    second = data if r.random() < 0.9 else data[:r.randrange(len(data) + 1)] + bytes(len(data))
    vfs.memory_source(r, log, regions, data + second[:len(data)])
    set_return(log, 8, len(data))
    regions[vfs.NAME] = vfs.name_under(r, root) + b'.tct\0' + r.bytes(7)
    return Call(0x427620, [vfs.NAME, SETS], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
