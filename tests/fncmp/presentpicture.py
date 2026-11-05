# The picture copied to the back surface and shown (src/game/render/present: 0x573cc0, 0x573890) against the originals: each display mode, lock and unlock answering 0, a positive or a failure. The surface's slots, the four blits, Device_Present and the error's calls recorders; the lock's description (its size and the zeroed rest) hashed, the pitch and pixels written.
# usage: presentpicture.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob_bytes, fill, log_region, recorder, set_return, strings  # noqa: E402

MESSAGE, LOGGED, PUSH, PRESENT, COPY16, DITHER, COPY32, CONVERT, LOCK, UNLOCK = 1, 2, 3, 4, 5, 6, 7, 8, 40, 41
B = Bench([(0x573cc0, 0x573e23), (0x573890, 0x57389b)], hooks={0x56a770: _recorder(MESSAGE, 1, cdecl=True), 0x564650: _recorder(LOGGED, 1, cdecl=True), 0x56af20: _recorder(PUSH, 2), 0x564210: _recorder(PRESENT, 0),
                                                              0x678790: _recorder(COPY16, 6, cdecl=True), 0x678d70: _recorder(DITHER, 6, cdecl=True), 0x678c20: _recorder(COPY32, 10, cdecl=True), 0x6787f0: _recorder(CONVERT, 10, cdecl=True)})
ENGINE, SURFACE, VT, PICTURE = RAM, RAM + 0x100, RAM + 0x180, RAM + 0x300
ANSWERS = [0, 0, 0, 1, 0x80000001, 0x887601c2]
OPS = ['Picture'] * 9 + ['Engine']


def case(r):
    op = r.deal(OPS)
    log = log_region()
    engine = bytearray(r.bytes(0x40))
    struct.pack_into('<I', engine, 0x14, SURFACE)
    vt = bytearray(0x100)
    struct.pack_into('<I', vt, 0x64, recorder(LOCK, 5, True))
    struct.pack_into('<I', vt, 0x80, recorder(UNLOCK, 2, True))
    regions = {LOG: log, ENGINE: engine, SURFACE: struct.pack('<I', VT), VT: vt, PICTURE: r.bytes(0x20), 0x879a24: struct.pack('<I', ENGINE), 0x879cd4: r.bytes(4), 0x8bcbd0: struct.pack('<II', r.randrange(1, 800), r.randrange(1, 600)),
               0x8bcbc8: struct.pack('<I', r.choice([0, 0x40000000, 0x40000000, 0x2000, 0x4000000, 0x4002000, 1]) | (r.getrandbits(8) << 16 & 0xff0000)), 0x8bcbe4: struct.pack('<I', r.choice([0x20, 0x20, 0x10, 0x18])), 0x8bd47c: r.bytes(4)}
    set_return(log, PRESENT, r.getrandbits(32))
    if op == 'Engine':
        return Call(0x573890, [], regions, ret='a')
    set_return(log, LOCK, r.choice(ANSWERS))
    set_return(log, UNLOCK, r.choice(ANSWERS))
    set_return(log, MESSAGE, r.getrandbits(32))
    blob_bytes(log, LOCK, 2, 0xffffffff)
    fill(log, LOCK, 2, struct.pack('<I', r.getrandbits(32)), offset=0x10)
    fill(log, LOCK, 2, struct.pack('<I', r.getrandbits(32)), which=1, offset=0x24)
    return Call(0x573cc0, [PICTURE + 0x10], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
