# The lobby island's camera settings read (0x5e2cc0 IslandCamera_LoadSettings, src/game/frontend/islandcamera) against the original: the
# path the two global Strings (0xf855b8 + 0xf855d0) under vfs.py's mounts, the file from its memory source (particlelib.py's way): lines of
# the seven keys (case varied now and then, other lines between) with numbers, odd text, missing ',' or ')' and camera indexes 0-9 or past
# them, mostly '\n' ended ("\r\n" now and then); the size answer the data's length mostly, else short or into the source's random tail.
# usage: islandcamera.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import gamestring  # noqa: E402
import vfs  # noqa: E402
import wctimage  # noqa: E402
from fncmp import HEAP, LOG, Bench, Call, RAM, global_heap, heap, log_region, set_return  # noqa: E402
from gamestring import LOCALE  # noqa: E402

B = Bench(wctimage.merged(vfs.RANGES + [
    (0x5e2cc0, 0x5e3166), (0x5e3170, 0x5e3208), (0x5c4d50, 0x5c4ff4), (0x5c5000, 0x5c5020), (0x5f5660, 0x5f5690), (0x67a400, 0x6902c0),
    (0x5da3c0, 0x5da3c1)]))
SETTINGS, PATH = RAM + 0x60000, RAM + 0x61000
KEYS = [b'ISLANDFOV(', b'SPINSPEED(', b'SPINRADIUS(', b'VERTICALOFFSET(', b'GLOBERADIUSOUT(', b'GLOBERADIUSIN(']
CAMERA = b'ISLANDCAMERAPOSITION('


def number(r):
    return r.choice([b'1', b'0', b'-2.5', b'45.0', b'0.0004', b'1e10', b'-1e-3', b' 7.25', b'abc', b'', b'3.', b'.5', b'1e400',
                     str(r.uniform(-1000, 1000)).encode()])


def line(r):
    k = r.random()
    if k < 0.45:
        key = r.choice(KEYS)
        tail = number(r) + r.choice([b')', b')', b')', b'', b',1)'])
    elif k < 0.85:
        key = CAMERA
        index = r.choice([b'0', b'1', b'5', b'9', b'10', b'-1', b'11', b'x', b' 3', str(r.randrange(10)).encode()])
        tail = index + b',' + number(r) + r.choice([b',', b',', b')']) + number(r) + r.choice([b')', b')', b''])
    else:
        return r.choice([b'', b'// comment', b'ISLAND', b'SPIN(1)', b'islandfov(3)', b'  ISLANDFOV(2)', r.bytes(r.randrange(8)).replace(b'\n', b'')])
    if r.random() < 0.1:
        key = vfs.flip(r, key)
    return key + tail + r.choice([b'', b'', b'', b' ;x', b'  '])


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x10000), LOCALE: bytes(4), **global_heap()}
    root = vfs.mounts(r, log, regions)
    lines = [line(r) for _ in range(r.choice([0, 1, 2, 4, 8, 16, 30]))]
    data = b''.join(x + (b'\r\n' if r.random() < 0.1 else b'\n') for x in lines)
    if lines and r.random() < 0.15:   # (a last line without '\n': read on past the block, sic)
        data = data.rstrip(b'\n')
    vfs.memory_source(r, log, regions, data)
    set_return(log, 8, r.choice([len(data)] * 8 + [len(data) // 2, len(data) + r.randrange(1, 0x40)]))
    name = vfs.name_under(r, root) + b'.txt'
    cut = r.randrange(len(name) + 1)
    strings = {}   # (the two String objects 0x18 bytes apart: one region)
    gamestring.string(r, 0, 0xf855b8, PATH, strings, name[:cut])
    gamestring.string(r, 0, 0xf855d0, PATH + 0x200, strings, name[cut:])
    regions[0xf855b8] = strings.pop(0xf855b8)[:0x18] + strings.pop(0xf855d0)
    regions.update(strings)
    regions[SETTINGS] = r.bytes(0x6c)
    return Call(0x5e2cc0, [SETTINGS], regions, conv='t')


if __name__ == '__main__':
    B.main(case)
