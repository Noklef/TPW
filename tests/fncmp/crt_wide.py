# The C library's wide string routines (src/game/core/crt) against the originals: swprintf and _snwprintf's string file (and _snprintf, vsprintf, _vsnprintf's over _output, with their one zero byte), as _woutput (a recorder, which moves the file's pointer and
# count) leaves it, then one or two characters through _flsbuf (a recorder) when it has no room; _wcsicmp and _wcsnicmp in the "C" locale (A-Z folded down) and with a
# locale (the lock's Interlocked imports, or lock 0x13 when the locale is being set; _towlower_lk a stand-in on both sides that looks only at the low 16 bits); btowc over
# _Mbrtowc (a recorder that fills the answer through its stack pointers); vswprintf over _woutput as swprintf; _itow and _wtoi over their import (a recorder) and _itoa / atol.
# usage: crt_wide.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fill, log_region, recorder, set_return, strings  # noqa: E402

LOCK, UNLOCK, WOUTPUT, FLSBUF, MBRTOWC, OUTPUT = 7, 8, 21, 22, 23, 1
INC, DEC = 30, 31
M2W, W2M = 14, 15


def towlower(e):
    """_towlower_lk's stand-in (driver.cpp): the locale's own rule is not this bench's business."""
    c = e.u32(e.r['ESP']) & 0xffff
    e.r['EAX'] = c ^ 0x20 if 0x41 <= c <= 0x5a or 0x100 <= c < 0x180 else c


B = Bench([(0x67bf40, 0x67bfa8), (0x67c1e0, 0x67c248), (0x67f3a0, 0x67f408), (0x67af00, 0x67afa0), (0x67c830, 0x67c8d0), (0x67b140, 0x67b240), (0x67c8d0, 0x67c9f0), (0x693dc0, 0x693e20), (0x67dda0, 0x67df3e), (0x67acd0, 0x67acee), (0x67b0d0, 0x67b10f),
           (0x67e010, 0x67e04a), (0x67f4a0, 0x67f4d0), (0x67f4d0, 0x67f564), (0x688730, 0x68876d), (0x688770, 0x6887d1), (0x67a480, 0x67a519)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x683090: _recorder(WOUTPUT, 3, cdecl=True), 0x680c00: _recorder(OUTPUT, 3, cdecl=True), 0x680ad0: _recorder(FLSBUF, 2, cdecl=True),
                 0x693bc0: _recorder(MBRTOWC, 5, cdecl=True), 0x67e670: towlower})
TO, FMT, A, Z = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300
PATH = RAM + 0x1000
UNITS = [0, 0x41, 0x5a, 0x61, 0x7a, 0x40, 0x5b, 0x60, 0x7b, 0x100, 0x130, 0xffff, 0x8041]


def wide(r, base=None):
    n = r.choice([0, 1, 2, 5, 12])
    units = list(base) if base is not None and r.random() < 0.7 else [r.choice(UNITS[1:]) for _ in range(n)]
    if base is not None:
        for i in range(len(units)):
            if 0x41 <= units[i] <= 0x5a or 0x61 <= units[i] <= 0x7a:
                if r.random() < 0.5:
                    units[i] ^= 0x20
        if r.random() < 0.4:
            units = units[:r.randrange(len(units) + 1)] + [r.choice(UNITS[1:])]
    return units


def pair(r):
    a = wide(r)
    b = wide(r, a)
    return a, b, struct.pack('<%dH' % (len(a) + 1), *a, 0), struct.pack('<%dH' % (len(b) + 1), *b, 0)


def locale_regions(r, log):
    # (the handle: 0, the "C" locale's path, or set; the setter's flag and the readers' count)
    regions = {LOG: log, 0xfb1440: struct.pack('<I', r.choice([0, 1, r.getrandbits(32)])), 0xfb3ca4: struct.pack('<II', r.choice([0, 0, 1]), r.getrandbits(32)),
               0x6fd0c0: struct.pack('<I', recorder(INC, 1, True)), 0x6fd12c: struct.pack('<I', recorder(DEC, 1, True))}
    return regions


def wcsicmp(r):
    log = log_region()
    regions = locale_regions(r, log)
    _, _, x, y = pair(r)
    regions[A], regions[Z] = x, y
    return Call(0x67b140, [A, Z], regions, conv='c', ret='a')


def wcsnicmp(r):
    log = log_region()
    regions = locale_regions(r, log)
    _, _, x, y = pair(r)
    regions[A], regions[Z] = x, y
    return Call(0x67c8d0, [A, Z, r.choice([0, 1, 2, 3, 8, 0x7fffffff])], regions, conv='c', ret='a')


def printing(r, address, head, k=WOUTPUT, args=None):
    log = log_region()
    if r.random() < 0.5:   # (one blob a slot: the file's pointer, count, base and flags, its other four dwords never set; or the arguments)
        blob(log, k, 0, 0xf)
        strings(log, k, 0, out=4)
    else:
        blob(log, k, 2, 3)
        strings(log, k, 0, out=1)
    blob(log, FLSBUF, 1, 0xf)
    n = r.choice([0, 1, 7, 40])
    if r.random() < 0.8:
        fill(log, k, 0, struct.pack('<Ii', TO + n, r.choice([5, 2, 1, 0, 0x7fffffff - n])))
    set_return(log, k, r.getrandbits(32))
    regions = {LOG: log, TO: r.bytes(0x60), FMT: b'%d\0\0'}
    if args:   # (a va_list: the pointer itself is the last argument)
        regions[args] = r.bytes(0x10)
        return Call(address, [TO] + head + [FMT, args], regions, conv='c', ret='a')
    return Call(address, [TO] + head + [FMT, r.getrandbits(32), r.getrandbits(32)], regions, conv='c', ret='a')


def swprintf(r):
    return printing(r, 0x67af00, [])


def snprintf(r):
    return printing(r, 0x67bf40, [r.choice([0, 1, 2, 0x10, 0x40000000, 0x80000000])], OUTPUT)


def vsprintf(r):
    return printing(r, 0x67c1e0, [], OUTPUT, A)


def vsnprintf(r):
    return printing(r, 0x67f3a0, [r.choice([0, 1, 2, 0x10, 0x40000000, 0x80000000])], OUTPUT, A)


def snwprintf(r):
    return printing(r, 0x67c830, [r.choice([0, 1, 2, 0x10, 0x40000000])])


def vswprintf(r):
    return printing(r, 0x67f4d0, [], WOUTPUT, A)


def itow(r):
    """_itow: _itoa into its 0x14-byte local (the digits logged as text), MultiByteToWideChar (a recorder) through to the caller's buffer. Radix 2 only for
    values short enough for the local (a longer one overruns it on both sides)."""
    log = log_region()
    strings(log, M2W, 0b100, out=0)
    fill(log, M2W, 4, r.bytes(r.choice([2, 8, 0x28])))
    set_return(log, M2W, r.choice([0, 1, 5]))
    radix = r.choice([2, 8, 10, 10, 16, 36])
    value = r.choice([0, 1, 9, 0x7fffffff, 0x80000000, 0xffffffff, r.getrandbits(32)])
    return Call(0x67e010, [value & 0x3ffff if radix == 2 else value, TO, radix], {LOG: log, TO: r.bytes(0x30), 0x6fd174: struct.pack('<I', recorder(M2W, 6, True))}, conv='c', ret='a')


def wtoi(r):
    """_wtoi: WideCharToMultiByte (a recorder) into its 0x14-byte local (filled with the bytes atol then reads), atol on them (the "C" table)."""
    log = log_region()
    strings(log, W2M, 0, out=0b10000)
    fill(log, W2M, 4, r.choice([b'0', b'12', b'-7', b'+42', b' 99', b'\t-0x1', b'2147483648', b'4294967296', b'abc', b'', b'1 2']) + b'\0')
    set_return(log, W2M, r.choice([0, 1, 3]))
    table = bytearray(0x200)
    for c in range(0x30, 0x3a):
        table[2 * c] = 4
    for c in (0x20, 9):
        table[2 * c] = 8
    regions = {LOG: log, A: struct.pack('<4H', 0x31, 0x32, 0, 0), 0x6fd1dc: struct.pack('<I', recorder(W2M, 8, True)), 0x77e7dc: struct.pack('<I', 1), 0x77ea90: struct.pack('<I', Z), Z: bytes(table)}
    return Call(0x67f4a0, [A], regions, conv='c', ret='a')


def btowc(r):
    log = log_region()
    if r.random() < 0.5:   # (one blob a slot: the argument's slot, the byte over it; or the state, zeroed)
        strings(log, MBRTOWC, 0, out=9)
        blob(log, MBRTOWC, 1, 1)
    else:
        strings(log, MBRTOWC, 0, out=3)
        blob(log, MBRTOWC, 3, 1)
    fill(log, MBRTOWC, 0, r.bytes(4))
    set_return(log, MBRTOWC, r.choice([0, 1, 0xffffffff, 0xfffffffe, r.getrandbits(32)]))
    return Call(0x693dc0, [r.choice([0xffffffff, 0, 0x41, 0xe9, r.getrandbits(32)])], {LOG: log}, conv='c', ret='a')


def splitpath(r):
    """_wsplitpath: a path of few kinds of character, a long one now and then (past the 0xff cut); any output null."""
    n = r.choice([0, 1, 2, 3, 5, 8, 12, 20, 0x100, 0x101, 0x180])
    chars = [r.choice([0x61, 0x62, 0x2f, 0x5c, 0x2e, 0x3a, 0x3a, 0x100]) if n < 0x100 or r.random() < 0.02 else 0x61 for _ in range(n)]
    if n and r.random() < 0.4:
        chars[r.randrange(n)] = r.choice([0x2f, 0x2e])
    if n > 1 and r.random() < 0.3:
        chars[1] = 0x3a
    outs = [RAM + 0x1000 * (i + 2) if r.random() < 0.8 else 0 for i in range(4)]
    regions = {PATH: struct.pack('<%dH' % (n + 1), *chars, 0)}
    for out in outs:
        if out:
            regions[out] = r.bytes(0x400)
    return Call(0x67dda0, [PATH, *outs], regions, conv='c', ret='v')


KINDS = {'splitpath': splitpath, 'wcsicmp': wcsicmp, 'wcsnicmp': wcsnicmp, 'swprintf': swprintf, 'snwprintf': snwprintf, 'snprintf': snprintf, 'vsprintf': vsprintf, 'vsnprintf': vsnprintf, 'btowc': btowc,
         'vswprintf': vswprintf, 'itow': itow, 'wtoi': wtoi}
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]


def case(r):
    return KINDS[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)
