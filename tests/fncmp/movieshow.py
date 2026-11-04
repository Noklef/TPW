# A movie's frame shown (src/game/gfx/movieshow: 0x670c20) against the original. The surfaces are over a vtable of COM recorders: Lock fills the description's pitch and pixels and logs the rest of
# it, Unlock logs. The callees that take only the player are recorders as driver.cpp's kMovieShowEnv; those with a register argument are stand-ins as driver.cpp's movie_show_N: each hashes its
# number, EDX and its stack arguments into the player's +0x66c.
# usage: movieshow.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fill, log_region, put, recorder, set_return  # noqa: E402

PLACE, CREATE, RESTORE, PIXELS, TGQ, TQI, LOCK, UNLOCK = range(10, 18)
PLAYER, SETTINGS, ENTRY, VTABLE, OBJECTS, DATA = RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x4000, RAM + 0x5000, RAM + 0x123450


def stand_in(k, n):
    def hook(e):  # fastcall (player, edx, n stack arguments): as driver.cpp's movie_show_N
        sp = e.r['ESP'] + (4 if e.tail else 0)
        player = e.r['ECX']
        h = e.u32(player + 0x66c)
        for value in [k, e.r['EDX']] + [e.u32(sp + 4 * i) for i in range(n)]:
            h = (h * 33 ^ value) & 0xffffffff
        e.write(player + 0x66c, struct.pack('<I', h))
        if e.tail:
            ret = e.u32(e.r['ESP'])
            e.r['ESP'] += 4 * n
            e.write(e.r['ESP'], struct.pack('<I', ret))
        else:
            e.r['ESP'] += 4 * n
    return hook


B = Bench([(0x670c20, 0x670fd8)],
          hooks={0x670890: stand_in(1, 0), 0x6711a0: _recorder(PLACE, 0), 0x6719e0: _recorder(CREATE, 0), 0x671fa0: _recorder(RESTORE, 0), 0x671000: _recorder(TGQ, 1, cdecl=True), 0x671540: stand_in(2, 3),
                 0x671230: stand_in(3, 3), 0x6710c0: _recorder(TQI, 2, cdecl=True), 0x671940: stand_in(4, 3), 0x671800: stand_in(5, 3), 0x6718a0: stand_in(6, 3), 0x6777d0: stand_in(7, 0), 0x677930: _recorder(PIXELS, 0),
                 0x677310: stand_in(8, 0), 0x6779e0: stand_in(9, 2), 0x6771a0: stand_in(10, 3)})


def case(r):
    log = log_region()
    table = bytearray(0x90)
    put(table, 0x64, 'I', recorder(LOCK, 5, True))
    put(table, 0x80, 'I', recorder(UNLOCK, 2, True))
    player = bytearray(r.bytes(0x670))
    put(player, 0, 'I', SETTINGS)
    put(player, 0x4c, 'I', r.deal([0, 1, 1, 1, 2, 2, 3, 4, 4]))
    put(player, 0x54, 'I', r.deal([0, 1]))
    put(player, 0x14, 'I', r.deal([0, 1, 7, r.getrandbits(16)]))
    put(player, 0x18, 'I', r.deal([0, 1, 9, r.getrandbits(16)]))
    settings = bytearray(r.bytes(0x100))
    flags = r.getrandbits(32) & ~0x8000c70
    for bit in (0x10, 0x20, 0x40, 0x8000000):
        flags |= r.deal([0, bit])
    flags |= r.deal([0, 0x400, 0x400])   # (one deck per list: two deals of [0, 0x400] in a case are always one of each)
    flags |= r.deal([0, 0, 0, 0, 0x800])
    put(settings, 4, 'I', flags)
    put(settings, 0x48, 'I', r.deal([8, 8, 0x10, 0x11, 0x20]))
    for k, offset in enumerate((0x14, 0x18, 0x1c, 0x20)):
        put(settings, offset, 'I', OBJECTS + 0x10 * k)
    pitch, pixels = r.deal([0x100, 0x280, 0x281, r.getrandbits(32)]), r.getrandbits(32)
    fill(log, LOCK, 2, struct.pack('<6I', pitch, *[r.getrandbits(32) for _ in range(4)], pixels), offset=0x10)
    blob(log, LOCK, 2, ((1 << 27) - 1) & ~(0x3f << 4))
    set_return(log, LOCK, r.deal([0, 0, 0, 0, 0, 0x88760096]))
    set_return(log, UNLOCK, r.deal([0, 0, 0x88760096]))
    set_return(log, RESTORE, r.deal([0, 0, 0, 0, 0, 1]))
    entry = bytearray(r.bytes(0x10))
    put(entry, 0, 'I', DATA)
    put(entry, 0xc, 'I', r.deal([0, 1, 1, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 6, 7, 8]))
    regions = {LOG: log, PLAYER: player, SETTINGS: settings, ENTRY: entry, VTABLE: table, OBJECTS: b''.join(struct.pack('<I', VTABLE) + r.bytes(0xc) for _ in range(4))}
    return Call(0x670c20, [PLAYER, ENTRY], regions, conv='f', ret='v')


if __name__ == '__main__':
    B.main(case)
