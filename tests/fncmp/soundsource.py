# A channel's sound source (src/game/core/soundsource) against the originals. The rate's set (0x6d21d0): a source at SRC whose rate (+0xc)
# is 11025, 22050 or any, low 0, 1 or any; the device singleton at 0xfb2030 made already (its guard set: no atexit), its primary buffer
# at PRIM or null, whose SetFormat (a COM recorder, the format logged as a hash) succeeds or fails. The orientation's set (0x6d37b0): the
# source's COM object (+0x1c) a recorder vtable whose slot 11 takes six floats, null now and then; the two vectors' floats normal, zero,
# NaNs (quiet and signalling, either sign) or any.
# usage: soundsource.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, blob_bytes, fb, log_region, recorder, set_return, vtable  # noqa: E402

CALLS = {'setrate': 0x6d21d0, 'orient': 0x6d37b0}
B = Bench([(0x6d21d0, 0x6d2214), (0x6d6cb0, 0x6d6cf0), (0x6c4390, 0x6c4410), (0x6c4500, 0x6c4541), (0x6d3a80, 0x6d3b04),
           (0x6d37b0, 0x6d37fe)])
SRC, OUT, PRIM, PRIM_VT, LISTENER, LVT, FRONT, TOP = (RAM + 0x100 * i for i in range(8))
DEVICE_GUARD = 0xfb2028   # (the guard's byte, then the device at 0xfb2030)
FORMAT_K = 41
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def floats(r):
    return struct.pack('<3I', *[r.choice([fb(r.uniform(-8, 8)), 0, 0x80000000, 0x7fc00000, 0x7f800001, 0xffa00000, 0x7fbfffff,
                                          r.getrandbits(32)]) for _ in range(3)])


def case(r):
    name = r.deal(OPS)
    log = log_region()
    if name == 'setrate':
        prim_vt = b''.join(struct.pack('<I', recorder(FORMAT_K if i == 14 else 48 + i, 2 if i == 14 else 0, True)) for i in range(16))
        blob_bytes(log, FORMAT_K, 1, 0x3ffff)
        set_return(log, FORMAT_K, r.choice([0, 0, 1, 0x88780032, 0x80000000]))
        src = bytearray(r.bytes(0x10))
        struct.pack_into('<I', src, 0xc, r.choice([11025, 22050, 11025, 22050, r.getrandbits(32)]))
        device = struct.pack('<II', r.getrandbits(32), r.choice([PRIM, PRIM, 0]))
        regions = {LOG: log, SRC: src, OUT: r.bytes(4), PRIM: struct.pack('<I', PRIM_VT) + r.bytes(0xc), PRIM_VT: prim_vt,
                   DEVICE_GUARD: bytes([r.choice([1, 3, 0xff])]) + r.bytes(7) + device}
        return Call(0x6d21d0, [SRC, OUT, r.choice([0, 1, r.getrandbits(32)])], regions, conv='t', ret='a')
    set_return(log, 11, r.getrandbits(32))
    src = bytearray(r.bytes(0x20))
    struct.pack_into('<I', src, 0x1c, r.choice([LISTENER] * 6 + [0]))
    regions = {LOG: log, SRC: src, OUT: r.bytes(4), LISTENER: struct.pack('<I', LVT) + r.bytes(0xc), LVT: vtable({}, com={11: 7}),
               FRONT: floats(r), TOP: floats(r)}
    return Call(0x6d37b0, [SRC, OUT, FRONT, TOP], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
