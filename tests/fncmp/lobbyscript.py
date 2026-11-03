# The lobby scene's script read (src/game/frontend/lobbyscript: 0x5e3210) against the original. The strings and the CRT's strncmp, atoi, atof real; the file calls, the block, the objects' constructors and model loads recorders (driver.cpp's kLobbyScriptCalls).
# usage: lobbyscript.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, blob, heap, log_region, set_return, strings  # noqa: E402
import gamestring  # noqa: E402
from gamestring import LOCALE  # noqa: E402

OPEN, SEEK, SIZE, READ, FINISH, ALLOC, FREE, ISLAND, ISLAND_MODELS, FLY, FLY_MODELS = 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
CDECL = {0x5c4d50: (OPEN, 2), 0x5c4fa0: (SEEK, 3), 0x5c5000: (SIZE, 1), 0x5c4f60: (READ, 4), 0x5c4f80: (FINISH, 1), 0x5f5660: (ALLOC, 1), 0x5f5680: (FREE, 1)}
THISCALL = {0x5dfa60: (ISLAND, 12), 0x5dfca0: (ISLAND_MODELS, 0), 0x5d9db0: (FLY, 7), 0x5da120: (FLY_MODELS, 0)}
HOOKS = {addr: _recorder(k, n, cdecl=True) for addr, (k, n) in CDECL.items()}
HOOKS.update({addr: _recorder(k, n) for addr, (k, n) in THISCALL.items()})
B = Bench([(0x5e3210, 0x5e3a77), (0x5e3170, 0x5e3208), (0x5e3a80, 0x5e3b24), (0x690000, 0x6902c0), (0x5da3c0, 0x5da3c1)] + gamestring.B_RANGES, hooks=HOOKS)
SELF, NAME, PATH, TEXT, WORLD, HOLDER, TABLE = RAM, RAM + 0x40, RAM + 0x80, RAM + 0x1000, RAM + 0x3000, RAM + 0x3100, RAM + 0x3200
PATHS = 0xf82884
ISLAND_OBJECT, FLY_OBJECT = 0x770000, 0x770100


def number(r):
    return r.choice([b'1', b'0', b'-2.5', b'45.0', b'0.0004', b'1e10', b'-1e-3', b' 7.25', b'abc', b'', b'3.', b'1e400', str(r.uniform(-1000, 1000)).encode()])


def integer(r):
    return r.choice([b'0', b'1', b'7', b'-3', b'0x10', b'abc', b'', b'65535', b'40000', str(r.randrange(100000)).encode()])


def word(r):
    return r.choice([b'', b'a', b'tree', b'isle01', b'longer name', b'seven..', b'abc(d,e)'])


def line(r):
    kind = r.deal(['Island', 'Fly', 'Lightning', 'Rainy', 'Sky', 'Other'])
    if kind == 'Island':
        return b'ISLAND(%s,"%s","%s","%s","%s",%s,%s)' % (r.choice([b'0', b'1', b'5', b'9', b'x', b'-1']), word(r), word(r), word(r), word(r), number(r), number(r))
    if kind == 'Fly':
        return b'FLYINGMESH("%s","%s",%s,%s,%s,%s,%s)' % (word(r), word(r), integer(r), number(r), number(r), number(r), number(r))
    if kind == 'Lightning':
        return b'LIGHTNING(%s)' % integer(r)
    if kind == 'Rainy':
        return b'RAINY(%s)' % integer(r)
    if kind == 'Sky':
        return b'SKYCOLOUR(%s,%s,%s)' % (integer(r), integer(r), integer(r))
    return r.choice([b'', b'// comment', b'ISLAN', b'ISLANDFOV(3)', b'FLYINGMESH', b'RAINY', b'rainy(3)', b' RAINY(3)', b'LIGHTNIN(1)', b'SKYCOLOUR', b'SKY(1,2,3)'])


def case(r):
    log = log_region()
    lines = [line(r) for _ in range(r.choice([0, 1, 1, 2, 3, 6]))]
    data = b''.join(x + r.choice([b'\n', b'\n', b'\n', b'\r\n', b'\n\n']) for x in lines)
    if lines and r.random() < 0.05:   # (no last '\n': on past the size to the NUL)
        data = data.rstrip(b'\n')
    if lines and r.random() < 0.02:   # (a missing delimiter: on to the next, or the end of the block)
        cut = r.randrange(len(data))
        data = data[:cut] + data[cut + 1:]
    size = r.choice([len(data)] * 12 + [len(data) // 2, len(data) + r.randrange(1, 8)])
    regions = {LOG: log, LOCALE: bytes(4), HEAP: heap(0x4000, fail=r.choice([0] * 24 + list(range(1, 16))) if r.random() < 0.2 else 0)}
    regions[TEXT] = data + bytes(0x10)
    regions[NAME] = r.choice([b'lobby', b'', b'x\\y']) + bytes(4)
    gamestring.string(r, 0, 0xf855b8, PATH, regions, r.choice([b'', b'data\\', b'c:\\tpw\\lobby\\']))
    regions[SELF] = struct.pack('<I', 0) * 4 + struct.pack('<I', WORLD) + r.bytes(0x10)
    regions[WORLD] = bytes(r.bytes(0x3c)) + struct.pack('<3I', r.f(), r.f(), r.f()) + r.bytes(0x60)
    regions[PATHS] = struct.pack('<I', HOLDER)
    regions[HOLDER] = bytes(0x10) + struct.pack('<I', TABLE)
    regions[TABLE] = b''.join(struct.pack('<I', r.f()) for _ in range(0x80 // 4))
    set_return(log, OPEN, r.choice([0x771000, 0]))
    set_return(log, SIZE, size)
    set_return(log, ALLOC, TEXT)
    set_return(log, ISLAND, ISLAND_OBJECT)
    set_return(log, FLY, FLY_OBJECT)
    strings(log, OPEN, 1)
    strings(log, ISLAND, 0x3c, out=1 << 7)
    blob(log, ISLAND, 7, 7)
    strings(log, FLY, 6, out=1 << 4)
    blob(log, FLY, 3, 0x7007)
    index = r.choice([-1, -1, -1, 0, 3, 9])
    return Call(0x5e3210, [SELF, NAME, r.getrandbits(32), index, r.getrandbits(32), r.getrandbits(32)], regions, conv='t')


if __name__ == '__main__':
    if '--table' in sys.argv:
        for addr, (k, n) in sorted(CDECL.items()):
            print('    {0x%06x, jpeg_recorder(%d, %d)},' % (addr, k, n))
        for addr, (k, n) in sorted(THISCALL.items()):
            print('    {0x%06x, recorder(%d, %d)},' % (addr, k, n))
        sys.exit(0)
    B.main(case)
