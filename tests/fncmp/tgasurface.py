# A TGA read into a new surface (src/game/gfx/imageops 0x47e5a0, src/game/gfx/pointer 0x47ec80) against the originals: the codec's two calls and the surface's calls recorders, each answering through its out
# slot (a status any at each step), the codec's vtable logged as `this`, the descriptions by what the calls were given.
# usage: tgasurface.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, echo, fill, log_region, put, strings, this_first  # noqa: E402

INFO, LOAD, CREATE, LOCK, GETINFO, UNLOCK, RELEASE = 1, 2, 5, 6, 7, 8, 9
B = Bench([(0x47e5a0, 0x47e721), (0x47ec80, 0x47ec9f), (0x5fa680, 0x5fa6a0), (0x5da3c0, 0x5da3c1)],
          hooks={0x5fa8a0: _recorder(INFO, 4), 0x5faa00: _recorder(LOAD, 5), 0x5f9070: _recorder(CREATE, 5, cdecl=True), 0x5f9550: _recorder(LOCK, 3, cdecl=True),
                 0x5fad40: _recorder(GETINFO, 2, cdecl=True), 0x5f9600: _recorder(UNLOCK, 1, cdecl=True), 0x5f9130: _recorder(RELEASE, 2, cdecl=True)})
SELF, NAME, OUT = RAM, RAM + 0x100, RAM + 0x140


def description(r):
    d = bytearray(r.bytes(0x28))
    put(d, 0x1c, '2I', r.choice([0, 31, 32, 33, 320, 0x10000, r.getrandbits(32)]), r.choice([0, 32, r.getrandbits(32)]))
    return bytes(d)


def status(r):
    return struct.pack('<i', r.choice([0, 0, 0, 0, 0, 1, -1, -0x7fffffff]))


def case(r):
    log = log_region()
    for k in INFO, LOCK, LOAD:
        echo(log, k, 0)
    this_first(log, INFO)
    this_first(log, LOAD)
    strings(log, INFO, 0, 1)
    strings(log, CREATE, 0, 1)
    strings(log, LOCK, 0, 1 | 4)
    strings(log, GETINFO, 0, 1)
    strings(log, LOAD, 0, 1)
    strings(log, RELEASE, 0, 1)
    blob(log, INFO, 3, 0x3ff)
    blob(log, GETINFO, 0, 0x3ff)
    blob(log, LOAD, 1, 0x3ff)
    fill(log, INFO, 3, description(r), which=0)
    fill(log, INFO, 0, status(r), which=1)
    fill(log, LOCK, 0, status(r), which=2)
    fill(log, GETINFO, 0, description(r), which=3)
    fill(log, LOAD, 0, status(r), which=4)
    me = bytearray(r.bytes(0x80))
    regions = {LOG: log, SELF: me, NAME: r.bytes(0x40), OUT: r.bytes(4)}
    if r.randrange(4):
        return Call(0x47e5a0, [NAME, r.getrandbits(32), OUT], regions, conv='c', ret='a')
    return Call(0x47ec80, [SELF, NAME], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
