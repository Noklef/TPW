# The C++ library's conversions under the C library's locale (src/game/core/crt: _Getcvt 0x69a560, _Getctype 0x699bd0, _Wcrtomb 0x69a460 and 0x69a4e0, _Mbrtowc 0x693bc0 and 0x693c40) against the originals:
# the Win32 calls (the interlocked pair, MultiByteToWideChar, WideCharToMultiByte) COM recorders in their import slots, the lock and _nh_malloc cdecl recorders, the locale's globals in regions.
# usage: crt_convert.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, blob, blob_bytes, fill, import_table, log_region, set_return, strings  # noqa: E402

GETPTD, LOCK, UNLOCK, NHMALLOC = 2, 7, 8, 28
INC, DEC, M2W, W2M = 14, 15, 16, 17
B = Bench([(0x69a460, 0x69a4d3), (0x69a4e0, 0x69a55e), (0x69a560, 0x69a5c8), (0x693bc0, 0x693c3b), (0x693c40, 0x693db7), (0x699bd0, 0x699c89), (0x686a00, 0x686a10), (0x67dcd0, 0x67dcf0), (0x4f5fb0, 0x4f6006), (0x69a5d0, 0x69a5fb), (0x4f1b30, 0x4f1b40), (0x4f1b40, 0x4f1b71)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x682460: _recorder(GETPTD, 0, cdecl=True), 0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True)})
PTD, TABLE, CONV, TEXT, STATE, UNIT, OUT, ANSWERS, PCTYPE = (RAM + 0x100 * i for i in range(9))
COPY, RESULT = RAM + 0x1000, RAM + 0x1400
OPS = [n for n in ('getcvt', 'getctype', 'wcrtomb', 'wcrtomb_lk', 'mbrtowc', 'mbrtowc_lk', 'length', 'wctob', 'narrow', 'narrow_range') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, PTD: bytearray(0x20), IMPORT_TABLE: import_table(log, {'inc': (INC, 0x6fd0c0, 1, 0, 0), 'dec': (DEC, 0x6fd12c, 1, 0, 0), 'm2w': (M2W, 0x6fd174, 6, 0, 0), 'w2m': (W2M, 0x6fd1dc, 8, 0, 0b10000000)})}
    set_return(log, GETPTD, PTD)
    regions[0xfb3ca4] = struct.pack('<2I', r.choice([0, 0, 1]), r.randrange(4))
    current = r.choice([0, 0x409, 0x411])
    regions[0xfb143c] = struct.pack('<I', r.getrandbits(32)) + struct.pack('<I', current) + bytes(0xc) + struct.pack('<I', r.choice([932, 1252, 0]))
    regions[0x77e7dc] = struct.pack('<i', r.choice([1, 2, 2, 2, 3, 0, -1]))
    regions[0x77ea90] = struct.pack('<I', PCTYPE)
    pctype = bytearray(0x200)
    for i in range(0x100):
        put_lead = r.random() < 0.4
        struct.pack_into('<H', pctype, i * 2, (r.getrandbits(15)) | (0x8000 if put_lead else 0))
    regions[PCTYPE] = pctype
    regions[ANSWERS] = struct.pack('<40I', *[r.choice([0, 1, 1, 2, r.getrandbits(32)]) for _ in range(40)])
    answers(log, M2W, ANSWERS)
    answers(log, W2M, ANSWERS)
    conversion = r.choice([0, 0, CONV])
    regions[CONV] = struct.pack('<2I', r.choice([0, 0x409, 0x411]), r.choice([932, 1252]))
    if name == 'getcvt':
        return Call(0x69a560, [], regions, conv='c', ret='a')
    if name == 'getctype':
        regions[RESULT] = r.bytes(0x10)
        regions[COPY] = r.bytes(0x200)
        set_return(log, NHMALLOC, r.choice([COPY, COPY, 0]))
        regions[0xfb1548] = struct.pack('<I', r.randrange(2))
        return Call(0x699bd0, [RESULT], regions, conv='c', ret='a')
    if name in ('wcrtomb', 'wcrtomb_lk'):
        regions[OUT] = r.bytes(0x10)
        blob(log, W2M, 2, 1)
        fill(log, W2M, 7, struct.pack('<I', r.choice([0, 0, 0, 1])))
        unit = r.choice([0, 0x41, 0xff, 0x100, 0x3042, 0x10041, r.getrandbits(32), r.getrandbits(16)])
        return Call(0x69a460 if name == 'wcrtomb' else 0x69a4e0, [OUT, unit, r.getrandbits(32), conversion], regions, conv='c', ret='a')
    if name == 'length':   # (codecvt::do_length 0x4f5fb0 on the real _Wcrtomb: the bytes go to its own stack, the unit's high half is the caller's EDX, so only the low half is seen.)
        strings(log, W2M, 0, out=0b10010000)
        blob_bytes(log, W2M, 2, 0b11)
        fill(log, W2M, 7, struct.pack('<I', r.choice([0, 0, 0, 0, 1])))
        regions[ANSWERS] = struct.pack('<40I', *[r.choice([1, 1, 1, 2, 2, 3, 0, 0xffffffff]) for _ in range(40)])
        self_ = bytearray(r.bytes(0x10))
        struct.pack_into('<2I', self_, 8, r.choice([0, 0x409, 0x409]), r.choice([932, 1252]))
        regions[RESULT] = self_
        count = r.choice([0, 1, 2, 3, 5, 8])
        regions[TEXT] = struct.pack('<16H', *[r.choice([0x41, 0xff, 0x100, 0x3042, r.getrandbits(16)]) for _ in range(16)])
        return Call(0x4f5fb0, [RESULT, r.getrandbits(32), TEXT, TEXT + 2 * count, r.choice([0, 1, 2, 3, 4, 5, 8, 0x100, 0xffffffff])], regions, conv='t', ret='a')
    if name in ('wctob', 'narrow', 'narrow_range'):   # (_Wctob 0x69a5d0, ctype<wchar_t>'s do_narrow 0x4f1b30, 0x4f1b40: the byte written to _Wctob's own argument slot, the high half of the unit the caller's EAX for the range.)
        strings(log, W2M, 0, out=0b10010000)
        blob_bytes(log, W2M, 2, 0b11)
        fill(log, W2M, 7, struct.pack('<I', r.choice([0, 0, 0, 0, 1])))
        fill(log, W2M, 4, bytes([r.choice([0x41, 0x80, 0xe9, r.getrandbits(8)])]), which=1)
        regions[ANSWERS] = struct.pack('<40I', *[r.choice([1, 1, 1, 1, 2, 0]) for _ in range(40)])
        unit = r.choice([0xffff, 0x1ffff, 0x41, 0xe9, 0xff, 0x100, 0x3042, r.getrandbits(32)])
        if name == 'wctob':
            return Call(0x69a5d0, [unit], regions, conv='c', ret='a')
        if name == 'narrow':
            return Call(0x4f1b30, [RESULT, unit, r.getrandbits(32)], regions, conv='t', ret='a')
        regions[TEXT] = struct.pack('<8H', *[r.choice([0xffff, 0x41, 0xe9, 0x100, 0x3042, r.getrandbits(16)]) for _ in range(8)])
        regions[OUT] = r.bytes(0x10)
        return Call(0x4f1b40, [RESULT, TEXT, TEXT + 2 * r.choice([0, 1, 2, 3, 6]), r.getrandbits(32), OUT], regions, conv='t', ret='a')
    regions[TEXT] = bytes(r.choice([0, 0x41, 0x81, 0xff, r.randrange(1, 256), r.randrange(1, 256)]) for _ in range(4)) + bytes(4)
    regions[STATE] = struct.pack('<I', r.choice([0, 0, 0, 1, r.getrandbits(32)]))
    regions[UNIT] = r.bytes(4)
    return Call(0x693bc0 if name == 'mbrtowc' else 0x693c40, [r.choice([UNIT, UNIT, UNIT, 0]), r.choice([TEXT, TEXT, TEXT, TEXT, 0]), r.choice([0, 1, 1, 2, 3, 4, 0xffffffff]), STATE, conversion], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
