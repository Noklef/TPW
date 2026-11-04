# A movie's display made ready (src/game/gfx/moviedisplay: 0x6700d0) against the original. Recorders: the mode, the size, malloc, the video tables, and the COM methods. The two fastcalls with a
# register argument, MoviePixels_ReadFormat and MoviePixels_BuildTables (their own benches have them), are stand-ins as driver.cpp's movie_display_stand_in: each hashes its number and arguments into HASH.
# usage: moviedisplay.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fill, log_region, put, recorder, set_return  # noqa: E402

PICK, SIZE, MALLOC, TABLES, QUERY, MAKE, SET = range(10, 17)
PLAYER, SETTINGS, HASH, VTABLE, OBJECTS, ROOM = RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x4000, RAM + 0x5000, RAM + 0x6000


def stand_in(k, n):
    def hook(e):  # fastcall (ecx, edx, n stack arguments): as driver.cpp's movie_display_stand_in
        sp = e.r['ESP'] + (4 if e.tail else 0)
        h = e.u32(HASH)
        for value in [k, e.r['ECX'], e.r['EDX']] + [e.u32(sp + 4 * i) for i in range(n)]:
            h = (h * 33 ^ value) & 0xffffffff
        e.write(HASH, struct.pack('<I', h))
        if e.tail:
            ret = e.u32(e.r['ESP'])
            e.r['ESP'] += 4 * n
            e.write(e.r['ESP'], struct.pack('<I', ret))
        else:
            e.r['ESP'] += 4 * n
    return hook


B = Bench([(0x6700d0, 0x670206)], hooks={0x670660: _recorder(PICK, 0), 0x670300: _recorder(SIZE, 0), 0x670210: stand_in(1, 1), 0x67dcd0: _recorder(MALLOC, 1, cdecl=True), 0x66fbd0: _recorder(TABLES, 0), 0x670350: stand_in(2, 0)})


def case(r):
    log = log_region()
    table = bytearray(0x90)
    put(table, 0, 'I', recorder(QUERY, 3, True))
    put(table, 0x14, 'I', recorder(MAKE, 5, True))
    put(table, 0x7c, 'I', recorder(SET, 2, True))
    player = bytearray(r.bytes(0x670))
    put(player, 0, 'I', SETTINGS)
    settings = bytearray(r.bytes(0x100))
    flags = r.getrandbits(32) & ~0x4100800
    for bit in (0x800, 0x100000, 0x4000000):
        flags |= r.deal([0, bit])
    put(settings, 4, 'I', flags)
    put(settings, 0x88, 'I', r.deal([0, 0, 0, 0, 1]))
    put(settings, 0xc, 'I', r.deal([0, OBJECTS]))
    device = r.deal([0, OBJECTS + 0x10])
    put(settings, 0x10, 'I', device)
    put(settings, 0x14, 'I', OBJECTS + 0x20)
    put(settings, 0x48, 'I', r.deal([8, 8, 8, 0x10, 0x20]))
    set_return(log, PICK, r.getrandbits(32))
    set_return(log, MALLOC, ROOM)
    fill(log, QUERY, 2, struct.pack('<I', OBJECTS + 0x10))
    regions = {LOG: log, PLAYER: player, SETTINGS: settings, HASH: r.bytes(4), VTABLE: table, OBJECTS: b''.join(struct.pack('<I', VTABLE) + r.bytes(0xc) for _ in range(3)), ROOM: b'\xcd' * 0x408, 0xfb7f30: r.bytes(0x1004)}
    return Call(0x6700d0, [PLAYER], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
