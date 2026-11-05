# The C library's locale information calls (src/game/core/crtlocaleinfo) against the originals: the imports COM recorders, _nh_malloc a cdecl recorder; which call works, the answers and the digits varied.
# usage: crt_localeinfo.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _free, _recorder, answers, fill, heap, import_table, log_region, set_return  # noqa: E402

NHMALLOC, INFOW, INFOA, TOWIDE, TOMULTI, LASTERROR = 28, 14, 15, 16, 17, 18
B = Bench([(0x691380, 0x69152c), (0x691680, 0x6917b0), (0x6917b0, 0x6918ef), (0x67dcd0, 0x67dcf0), (0x67ae00, 0x67aefe), (0x680430, 0x6804c5)],
          hooks={0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True), 0x67ad90: _free})
A_MALLOC, A_INFOW, A_INFOA, A_WIDE, A_MULTI, PCTYPE, BUF, OUT = (RAM + 0x100 * i for i in range(8))
BLOCKS = [HEAP + 0x100 + 0x180 * i for i in range(10)]
OPS = [n for n in ('wide', 'multi', 'text', 'text', 'nocopy', 'digits', 'digits', 'other') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
SIZES = [0, 0, 1, 2, 3, 8, 0x10, 0x40, 0x70]


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x1000),
               IMPORT_TABLE: import_table(log, {'infow': (INFOW, 0x6fd204, 4, 0, 0b100), 'infoa': (INFOA, 0x6fd28c, 4, 0, 0b100), 'towide': (TOWIDE, 0x6fd174, 6, 0b100, 0),
                                                'tomulti': (TOMULTI, 0x6fd1dc, 8, 0b100, 0b10000), 'lasterror': (LASTERROR, 0x6fd138, 0, 0, 0)})}
    for k, at, values in ((NHMALLOC, A_MALLOC, [BLOCKS[i % 10] if r.random() < 0.9 else 0 for i in range(40)]), (INFOW, A_INFOW, [r.choice(SIZES) for _ in range(40)]), (INFOA, A_INFOA, [r.choice(SIZES) for _ in range(40)]),
                          (TOWIDE, A_WIDE, [r.choice([0, 1, 7, 0x20]) for _ in range(40)]), (TOMULTI, A_MULTI, [r.choice([0, 1, 7, 0x20]) for _ in range(40)])):
        answers(log, k, at)
        regions[at] = struct.pack('<40I', *values)
    set_return(log, LASTERROR, r.choice([0x7a, 0x7a, 0x7a, 0, 0x57]))
    digits = bytes(b for _ in range(4) for b in (r.choice(b'0123456789' * 3 + b'x '), r.choice([0, 0x30])))
    fill(log, INFOW, 2, digits if name == 'digits' else r.bytes(4) + b'\0\0')
    text = r.choice([b'', b'a', b'abc', b'a longer text of the locale']) + b'\0'
    fill(log, INFOA, 2, text + r.bytes(0x70 - len(text)), which=1)   # (the whole of what an answer of up to 0x70 says was written: what was not is the stack's)
    fill(log, TOMULTI, 4, text + r.bytes(0x70 - len(text)), which=2)
    pctype = bytearray(0x200)
    for c in range(0x100):
        pctype[c * 2] = (4 if 0x30 <= c <= 0x39 else 0) | r.choice([0, 1, 2, 8, 0x80])
        pctype[c * 2 + 1] = r.choice([0, 4, 0xff])
    regions[PCTYPE] = pctype
    regions[0x77ea90] = struct.pack('<I', PCTYPE)
    regions[0x77e7dc] = struct.pack('<I', r.choice([1, 1, 1, 2, 0, 0xffffffff]))
    regions[0xfb1450] = struct.pack('<I', r.getrandbits(32))
    modes = [0, 0, 1, 1, 2, 2, 3] if name in ('wide', 'multi') else [0, 0, 1, 1, 2, 2]   # (mode 3, no call made and 3 answered, has __getlocaleinfo copy the stack's garbage: not comparable)
    regions[0xfb1938] = bytearray(r.bytes(8) + bytes(4) + struct.pack('<2I', r.choice(modes), r.choice(modes)))
    regions[BUF] = r.bytes(0x40)
    regions[OUT] = r.bytes(8)
    if name == 'nocopy':   # (no room on the stack, the text in a block, none for the copy: the block goes back)
        regions[A_INFOA] = struct.pack('<40I', 0, *[8] * 39)
        regions[A_MALLOC] = struct.pack('<40I', BLOCKS[0], *[0] * 39)
        set_return(log, LASTERROR, 0x7a)
        regions[0xfb1938][0x10:0x14] = struct.pack('<I', 2)
    locale, kind = r.getrandbits(32), r.getrandbits(32)
    if name in ('wide', 'multi'):
        return Call(0x691680 if name == 'wide' else 0x6917b0, [locale, kind, r.choice([BUF, BUF, 0]), r.choice([0, 0, 4, 0x20]), r.choice([0, 0, 0x4e4])], regions, ret='a')
    return Call(0x691380, [{'text': 1, 'nocopy': 1, 'digits': 0}.get(name, r.choice([2, 0xffffffff, r.getrandbits(32)])), locale, kind, OUT], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
