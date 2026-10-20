# The sprite draw lists (src/game/sprite/draw: 0x540ab0, 0x542010, 0x5408e0/0x5408f0) against
# the original: the camera heading, and a script's draw record appended to its map cell or the
# overlay list. Scripts always carry flag 0x1000000 (their own height: without it the terrain
# height is called, out of range). SpriteDraw_End (0x540b30): the thing under the cursor (0x532bd0) is cdecl recorder 1, the
# pick's hand-over (0x423b50, thiscall on 0x7b05a8) cdecl recorder 2 popping its argument (driver.cpp: the env's).
# usage: spritedraw.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, RAM, _recorder, log_region, put, set_return  # noqa: E402

_pick = _recorder(1, 0, cdecl=True)


def _set_picked(e):  # thiscall (thing) on 0x7b05a8: logged as the driver's cdecl one, then pops it
    _recorder(2, 1, cdecl=True)(e)
    e.r['ESP'] = e.r['ESP'] + 4


B = Bench([(0x46f150, 0x475010), (0x67a400, 0x690000), (0x5408e0, 0x540900), (0x540ab0, 0x540bd0),
           (0x542010, 0x5423a0)], hooks={0x532bd0: _pick, 0x423b50: _set_picked})
SCRIPT, SPRITES, LISTS, MATRIX = (RAM + 0x1000 * k for k in range(4))
FRAMES = RAM + 0x10000  # the sprites' frame tables (+4: the frame records)
CAMERA = 0x790a30   # camera angle (+8), matrix pointer (+0x68), flags (+0x80)
BSS = 0x875500      # 0x875528 lists, 0x875864 overlay count, 0x87614c / 0x8762b4 cell sizes, 0x876490
                    # counts, 0x87692c overlay list, 0x876b24 heading, 0x876b48 gSprites, 0x877348-50
ENABLED = 0x763f84  # the draw switch (byte); gSpriteFolders follow at 0x763f88
NSPRITES = 4


def f32(x):
    return struct.unpack('<I', struct.pack('<f', x))[0]


def nan(r):
    return 0x7f800000 | (r.getrandbits(23) or 1) | r.getrandbits(1) << 31


def snan(r):
    return 0x7f800000 | (r.getrandbits(22) or 1) | r.getrandbits(1) << 31


def angle(r):
    return r.choice([f32(r.uniform(-7, 7)), f32(r.uniform(0, 6.3)), f32(r.uniform(-100, 100)), f32(6.2831855),
                     f32(-6.2831855), f32(0.0), f32(-0.0), f32(r.uniform(-1e6, 1e6)), f32(3e38), nan(r), 0x7f800000,
                     f32(1e-40), f32(-1e-40), f32(-0.3926991), f32(5.8904862)])


def position(r):
    return r.choice([f32(r.uniform(0, 200)), f32(r.uniform(0, 200)), f32(r.uniform(-50, 300)), f32(r.uniform(-1e10, 1e10)),
                     f32(3e38), nan(r), snan(r), f32(-0.0), f32(39.999996), f32(40.0)])


def case(r):
    op = r.deal(OPS)
    bss = bytearray(r.bytes(0x1e60))
    o = lambda a: a - BSS  # noqa: E731
    lists = [LISTS + 0x100 * k for k in range(16)]
    put(bss, o(0x875528), '16I', *lists)
    put(bss, o(0x875864), 'i', r.choice([0, 1, 5, r.randrange(0x30)]))
    put(bss, o(0x87614c), 'i', r.choice([1, 2, 5, 10, 16, 64, r.randrange(1, 100), -3]))
    put(bss, o(0x8762b4), 'i', r.choice([1, 2, 5, 10, 16, 64, r.randrange(1, 100), -3]))
    put(bss, o(0x876490), '16i', *(r.choice([0, 1, 3, r.randrange(0x30), 0x7ff, 0x800]) for _ in range(16)))
    put(bss, o(0x87692c), 'I', LISTS + 0x1000 - 0x100 if r.random() < 0.95 else 0)
    put(bss, o(0x876b24), 'I', angle(r))
    put(bss, o(0x876b48), '64I', *(SPRITES + 0x260 * r.randrange(NSPRITES) for _ in range(64)))
    sprites = bytearray()
    for k in range(NSPRITES):
        s = bytearray(r.bytes(0x260))
        put(s, 0, 'I', FRAMES + 0x10 * k)
        for a in range(16):  # first frame, frames per direction, directions
            put(s, 0x220 + 4 * a, 'HBB', r.randrange(200), r.choice([1, 2, 4, 8, r.randrange(256)]), r.choice([0, 1, 4, 5, 8, r.randrange(256)]))
        sprites += s
    frames = bytearray(r.bytes(0x40))
    camera = bytearray(r.bytes(0x90))
    put(camera, 8, 'I', angle(r))
    put(camera, 0x68, 'I', MATRIX)
    put(camera, 0x80, 'B', r.choice([0, 0, 0x16, 2, 4, 0x10, 1, 0xe9, r.randrange(256)]))
    matrix = bytearray(r.bytes(0x40))
    for i in range(12):
        put(matrix, 4 * i, 'f', r.uniform(-1, 1))
    folders = bytearray(r.bytes(4 + 14 * 12))
    put(folders, 0, 'B', r.choice([1, 1, 1, 0, r.randrange(256)]))
    for f in range(14):
        put(folders, 4 + 12 * f + 4, 'I', r.randrange(40))
    s = bytearray(r.bytes(0x118))
    put(s, 0x114, 'I', r.choice([1, 1, 1, 0, r.getrandbits(32)]))
    put(s, 0x88, 'III', position(r), r.choice([position(r), f32(r.uniform(0, 30)), snan(r)]), position(r))
    put(s, 0xac, 'II', r.choice([r.randrange(14), 5, 5]), r.randrange(8))
    put(s, 0xb4, 'i', r.choice([r.randrange(0, 16), r.randrange(-20, 40), r.randrange(-20, 40)]))
    put(s, 0xb8, 'I', r.choice([0, 1, 2, r.randrange(30), 0xffffffff]))
    put(s, 0xc0, 'i', r.choice([r.randrange(8), r.randrange(-3, 12)]))
    flags = r.getrandbits(32) | 0x1000000
    for bit in (0x40000, 0x2000000, 0x200):
        flags = flags & ~bit | (bit if r.random() < 0.3 else 0)
    put(s, 0xc4, 'I', flags)
    regions = {SCRIPT: s, SPRITES: sprites, LISTS: bytearray(r.bytes(0x1000)), MATRIX: matrix, FRAMES: frames,
               BSS: bss, CAMERA: camera, ENABLED: folders}
    if op == 'Begin':
        return Call(0x540ab0, [], regions)
    if op == 'End':
        put(bss, o(0x877348), 'I', r.choice([0, 1, 0x100, r.getrandbits(32)]))
        regions[0x8751e0] = r.bytes(0x260)   # (the renderer's copies of the counts, 0x8751f4 + 0x24 * cell)
        log = log_region()
        if r.random() < 0.8:
            set_return(log, 1, r.choice([0, r.getrandbits(32)]))
        regions[LOG] = log
        return Call(0x540b30, [], regions)
    if op == 'Enable':
        return Call(0x5408e0, [], regions)
    if op == 'Disable':
        return Call(0x5408f0, [], regions)
    return Call(0x542010, [SCRIPT], regions)


OPS_ALL = ['Begin', 'End', 'Draw', 'Draw', 'Draw', 'Enable', 'Disable']
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
