# The C++ library's num_put::do_put of a pointer (src/game/core/numput) against the originals, over the C library's sprintf: _Mbrtowc a cdecl recorder, the buffer's overflow a thiscall recorder.
# usage: numput.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, fill, log_region, recorder, set_return, strings  # noqa: E402

LOCK, UNLOCK, GETPTD, FLSBUF, MBRTOWC, OVERFLOW = 7, 8, 2, 22, 23, 12
B = Bench([(0x694e60, 0x695030), (0x69bb70, 0x69bd2c), (0x693dc0, 0x693e20), (0x67a650, 0x67a6b8), (0x680c00, 0x681480), (0x681590, 0x6815d2), (0x6815e0, 0x681619), (0x681620, 0x68165d), (0x681660, 0x681671), (0x681680, 0x681692), (0x683aa0, 0x683ab2), (0x68c350, 0x68c3b8), (0x68c3c0, 0x68c435)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x682460: _recorder(GETPTD, 0, cdecl=True), 0x680ad0: _recorder(FLSBUF, 2, cdecl=True), 0x693bc0: _recorder(MBRTOWC, 5, cdecl=True)})
OUT, BUFFER, POINTER, COUNT, VTABLE, IOS, AREA, A_OVERFLOW, A_WIDE = RAM, RAM + 0x20, RAM + 0x80, RAM + 0x90, RAM + 0xa0, RAM + 0xc0, RAM + 0x100, RAM + 0x200, RAM + 0x300
OPS = [n for n in ('narrow', 'wide') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def case(r):
    name = r.deal(OPS)
    wide = name == 'wide'
    log = log_region()
    regions = {LOG: log}
    regions[OUT] = r.bytes(8)
    regions[0x709b50] = b'%p\0\0'
    regions[0x77ea90] = struct.pack('<I', RAM + 0x800)
    regions[RAM + 0x800] = bytes(0x200)
    room = r.choice([0, 0, 1, 2, 5, 9, 12, 30, 0xffffffff, 0x80000000])
    place = r.choice([AREA, AREA, AREA, 0])
    regions[POINTER] = struct.pack('<I', place)
    regions[COUNT] = struct.pack('<I', room if room < 0x100 else (room if place else 0))
    regions[AREA] = r.bytes(0x60)
    regions[VTABLE] = struct.pack('<2I', 0, recorder(OVERFLOW, 1))
    regions[BUFFER] = struct.pack('<I', VTABLE) + bytes(0x1c) + struct.pack('<I', POINTER) + bytes(0xc) + struct.pack('<I', COUNT)
    answers(log, OVERFLOW, A_OVERFLOW)
    regions[A_OVERFLOW] = struct.pack('<40I', *[r.choice([0, 0x41, 0x41, 0xffffffff, 0xffff, 0xffff0041]) for _ in range(40)])
    width = r.choice([0, 0, -1, 1, 7, 8, 9, 12, 20, -20 & 0xffffffff])
    regions[IOS] = bytes(0xc) + struct.pack('<3I', r.choice([0, 0x40, 0x40, 0x80, 0x100, 0x140, 0x1c0, 0xffffffff, 0xfffffe3f]), r.getrandbits(32), width & 0xffffffff)
    if wide:
        strings(log, MBRTOWC, 0, out=9)
        blob(log, MBRTOWC, 1, 1)
        fill(log, MBRTOWC, 0, r.bytes(2) + b'\0\0')
        set_return(log, MBRTOWC, r.choice([0, 1, 1, 1, 0xffffffff, 0xfffffffe]))
    return Call(0x69bb70 if wide else 0x694e60, [r.getrandbits(32), OUT, r.getrandbits(32), r.choice([BUFFER, BUFFER, BUFFER, BUFFER, 0]), IOS, r.getrandbits(32), r.choice([0, 1, 0xdeadbeef, r.getrandbits(32)])], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
