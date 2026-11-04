# The small methods that touch textures, each in its subject's module, against the originals with TexCache_TouchItem a recorder (k 14):
# 0x577a30 TileGrid_TouchVisible (render/tilegrid: the cells the rectangle covers), 0x57dc80 WeatherFx_TouchTextures (render/weatherfx),
# 0x58a630 SpriteQueue_Clear, 0x58a720 SpriteQueue_ShowAll (render/spritequeue), 0x58ac70 Global8c47b0_Touch (render/spritequeue).
# usage: texturetouches.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

TOUCH, SHOW = 14, 15
B = Bench([(0x577a30, 0x577ae0), (0x57dc80, 0x57dd02), (0x58a630, 0x58a658), (0x58a720, 0x58a76b), (0x58ac70, 0x58ac8a), (0x67a830, 0x67a860)],
          hooks={0x58c9f0: _recorder(TOUCH, 2), 0x588ef0: _recorder(SHOW, 0)})
SELF, LISTS = RAM, RAM + 0x4000
OPS = [k for k in os.environ.get('OPS', 'tiles,weather,clear,show,touch').split(',') if k]


def tiles(r, regions):
    m = bytearray(r.bytes(0x60))
    box = [r.choice([0.0, 63.9, 64.0, 100.5, 128.0, 200.0, 256.0, -64.0, r.uniform(0, 320)]) for _ in range(4)]    # (64 to a cell)
    put(m, 0x20, '4f', *box)
    put(m, 0x1c, 'I', r.choice([0, RAM + 0x1000, RAM + 0x1000, RAM + 0x1000]))
    put(m, 0x10, 'I', r.choice([1, 4, 16, r.randrange(100)]))
    put(m, 0x48, 'I', r.randrange(100))
    regions[SELF] = bytes(m)
    return 0x577a30


def weather(r, regions):
    m = bytearray(r.bytes(0x3360))
    put(m, 0, 'I', r.choice([0, 1, 1, 2, 2, 3]))
    for off in (0x3308, 0x3318, 0x3348, 0x334c, 0x3350):
        put(m, off, 'I', r.choice([0, r.getrandbits(32) | 1, r.getrandbits(32) | 1]))
    regions[SELF] = bytes(m)
    return 0x57dc80


def clear(r, regions):
    regions[SELF] = r.bytes(0x2410)
    return 0x58a630


def show(r, regions):
    m = bytearray(r.bytes(0x2410))
    count = r.randrange(0, 6)
    put(m, 0x240c, 'I', count)
    for i in range(count):
        at = LISTS + 0x400 * i
        put(m, 0x2200 + 4 * i, 'I', at)
        n = r.randrange(0, 5)
        q = bytearray(r.bytes(0x20))
        put(q, 4, 'I', n)
        put(q, 0x14, 'I', at + 0x40)
        regions[at] = bytes(q)
        regions[at + 0x40] = struct.pack('<4I', *[r.getrandbits(32) for _ in range(4)])
    regions[SELF] = bytes(m)
    return 0x58a720


def touch(r, regions):
    regions[SELF] = r.bytes(0x10)
    return 0x58ac70


def case(r):
    regions = {LOG: log_region(), 0x8c4b44: r.bytes(4)}
    address = {'tiles': tiles, 'weather': weather, 'clear': clear, 'show': show, 'touch': touch}[r.deal(OPS)](r, regions)
    return Call(address, [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
