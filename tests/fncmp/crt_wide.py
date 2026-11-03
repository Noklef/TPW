# The C library's wide string routines (src/game/core/crt) against the originals: swprintf and _snwprintf's string file, as _woutput (a recorder, which moves the file's pointer and
# count) leaves it, then one or two characters through _flsbuf (a recorder) when it has no room; _wcsicmp and _wcsnicmp in the "C" locale (A-Z folded down) and with a
# locale (the lock's Interlocked imports, or lock 0x13 when the locale is being set; _towlower_lk a stand-in on both sides that looks only at the low 16 bits); btowc over
# _Mbrtowc (a recorder that fills the answer through its stack pointers).
# usage: crt_wide.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fill, log_region, recorder, set_return, strings  # noqa: E402

LOCK, UNLOCK, WOUTPUT, FLSBUF, MBRTOWC = 7, 8, 21, 22, 23
INC, DEC = 30, 31


def towlower(e):
    """_towlower_lk's stand-in (driver.cpp): the locale's own rule is not this bench's business."""
    c = e.u32(e.r['ESP']) & 0xffff
    e.r['EAX'] = c ^ 0x20 if 0x41 <= c <= 0x5a or 0x100 <= c < 0x180 else c


B = Bench([(0x67af00, 0x67afa0), (0x67c830, 0x67c8d0), (0x67b140, 0x67b240), (0x67c8d0, 0x67c9f0), (0x693dc0, 0x693e20)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x683090: _recorder(WOUTPUT, 3, cdecl=True), 0x680ad0: _recorder(FLSBUF, 2, cdecl=True),
                 0x693bc0: _recorder(MBRTOWC, 5, cdecl=True), 0x67e670: towlower})
TO, FMT, A, Z = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300
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


def printing(r, address, head):
    log = log_region()
    if r.random() < 0.5:   # (one blob a slot: the file's pointer, count, base and flags, its other four dwords never set; or the arguments)
        blob(log, WOUTPUT, 0, 0xf)
        strings(log, WOUTPUT, 0, out=4)
    else:
        blob(log, WOUTPUT, 2, 3)
        strings(log, WOUTPUT, 0, out=1)
    blob(log, FLSBUF, 1, 0xf)
    n = r.choice([0, 1, 7, 40])
    if r.random() < 0.8:
        fill(log, WOUTPUT, 0, struct.pack('<Ii', TO + n, r.choice([5, 2, 1, 0, 0x7fffffff - n])))
    set_return(log, WOUTPUT, r.getrandbits(32))
    regions = {LOG: log, TO: r.bytes(0x60), FMT: b'%d\0\0'}
    return Call(address, [TO] + head + [FMT, r.getrandbits(32), r.getrandbits(32)], regions, conv='c', ret='a')


def swprintf(r):
    return printing(r, 0x67af00, [])


def snwprintf(r):
    return printing(r, 0x67c830, [r.choice([0, 1, 2, 0x10, 0x40000000])])


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


KINDS = {'wcsicmp': wcsicmp, 'wcsnicmp': wcsnicmp, 'swprintf': swprintf, 'snwprintf': snwprintf, 'btowc': btowc}
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]


def case(r):
    return KINDS[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)
