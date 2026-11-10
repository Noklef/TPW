# More of the C++ library's use_facet, the time and collate facets' constructors, _Getcoll and _Locimp::_Makexloc / _Makewloc (src/game/core/localefacet, crt, moneypunct) against the originals:
# localefacet.py's world (the locks, the throw and atexit recorders, the locale at HANDLE with its ids and caches new or set), _Mbrtowc a recorder as moneypunct.py has it, a localeconv and the
# C library's day and month names made here. The shared globals' setters log their atexit through fncmp's cdecl log, as the port's env has it.
# usage: usefacet.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
import localefacet as lf  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _cdecl_log, _recorder, answers, blob, fill, heap, import_table, log_region, set_return, strings  # noqa: E402

# k 29 for GetVersionEx: k 23 is _Mbrtowc's in the driver, as moneypunct.py has it.
MBRTOWC, VERSION = 23, 29
# The setters' releases (sharedglobals): fncmp's cdecl log, as SharedGlobals' env in the driver.
SET_RELEASES = lf.SET_RELEASES + (0x6ac0b0, 0x6ac410, 0x6ac530, 0x6ac650, 0x4f7970, 0x6a5ae0, 0x6a5c00, 0x6a5d20, 0x6a6080, 0x6a61a0, 0x6a62c0)
SETTERS = (0x4f7770, 0x6a5a60, 0x6a5b80, 0x6a5ca0, 0x6a6000, 0x6a6120, 0x6a6240, 0x6ac030, 0x6ac390, 0x6ac4b0, 0x6ac5d0)


def at_exit(e):
    # (use_facet of numpunct<wchar_t> registers a setter's release itself, 0x6a4966: through the env's atexit natively, as the other use_facets)
    direct = not e.tail and getattr(e, 'return_to', 0) == 0x6a496b
    (_cdecl_log if e.u32(e.r['ESP'] + (4 if e.tail else 0)) in SET_RELEASES and not direct else lf._at_exit)(e)


B = Bench(lf.RANGES + [(0x691530, 0x691536), (0x693dc0, 0x693e20), (0x6a4f20, 0x6a5172), (0x6ab880, 0x6aba77), (0x6902b0, 0x690394), (0x6903a0, 0x69048a),
                       (0x6a4d70, 0x6a4f13), (0x6a5180, 0x6a531a), (0x6a5320, 0x6a549f), (0x6a58d0, 0x6a5a5a), (0x6ab6d0, 0x6ab873), (0x6aba80, 0x6abc1a), (0x6abc20, 0x6abe79),
                       (0x6a3470, 0x6a35fd), (0x6aa200, 0x6aa365),
                       (0x6a67f0, 0x6a6fc0), (0x69a7a0, 0x69b460), (0x6ac9a0, 0x6aca10), (0x6a6fc0, 0x6a7030), (0x69ff10, 0x69ff80), (0x6ab0e0, 0x6ab160), (0x6a4490, 0x6a4510),
                       (0x6abe80, 0x6ac030), (0x6a5720, 0x6a58d0), (0x6a54a0, 0x6a5720), (0x6a1ee0, 0x6a2150), (0x690490, 0x690800), (0x6a4520, 0x6a4540), (0x4f7620, 0x4f76b0),
                       (0x69fef0, 0x69ff10), (0x6a8830, 0x6a8a50), (0x6a18a0, 0x6a1b30), (0x6a1e70, 0x6a1ee0), (0x6a4790, 0x6a49da)] + [(a, a + 0x80) for a in SETTERS],
          hooks={**{k: v for k, v in lf.HOOKS.items() if k not in (0x6a67f0, 0x69a7a0)}, 0x693bc0: _recorder(MBRTOWC, 5, cdecl=True), 0x67a7d0: at_exit})
# name: the address, then for use_facet the id, the cache and the holder
USES = {'moneywintl': (0x6a4d70, 0xfb1de8, 0xfb1e2c, 0xfb1e34), 'moneyw': (0x6a5180, 0xfb1e0c, 0xfb1e28, 0xfb1e38), 'moneycintl': (0x6ab6d0, 0xfb1e80, 0xfb1eb0, 0xfb1eb4),
        'moneyc': (0x6aba80, 0xfb1e90, 0xfb1eac, 0xfb1eb8), 'f709bec': (0x6a5320, 0xfb1e08, 0xfb1e40, 0xfb1e3c), 'timec': (0x6abc20, 0xfb1e84, 0xfb1ec0, 0xfb1ebc),
        'codecvtw': (0x6a58d0, 0xfb1e00, 0x7ced40, 0x7ced48), 'timew': (0x6a54a0, 0xfb1df4, 0xfb1e48, 0xfb1e44), 'f70a044': (0x6abe80, 0xfb1e88, 0xfb1ec8, 0xfb1ec4),
        'f709ef4': (0x6a5720, 0xfb1e04, 0xfb1e50, 0xfb1e4c), 'wnumpunct': (0x6a4790, 0xfb1ddc, 0xfb1e1c, 0xfb1e30)}
# _Makexloc's and _Makewloc's other ids and classic caches
IDS = sorted({u[1] for u in USES.values()} | {0xfb1e98, 0xfb1e8c, 0xfb1e94, 0xfb1e7c, 0xfb1df8, 0xfb1dfc, 0xfb1dec, 0xfb1ddc, 0xfb1df0, 0xfb1de4, 0xfb1de0})
CACHES = sorted({u[2] for u in USES.values()} | {0xfb1ea8, 0xfb1ea4, 0xfb1ea0, 0xfb1e9c, 0x7ced3c, 0xfb1e24, 0xfb1e20, 0xfb1e1c, 0xfb1e18, 0xfb1e14, 0xfb1e10})
OPS = [n for n in list(USES) + ['ctorc', 'ctorw', 'collc', 'collw', 'tnx', 'tnw', 'getcoll', 'mpwintl'] + ['makex'] * 5 + ['makew'] * 5
       if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
CONV, TEXTS, NAMES, NAME_TEXTS, INFO, FOUT = RAM + 0x3000, RAM + 0x3100, RAM + 0x3200, RAM + 0x3300, RAM + 0x3800, RAM + 0x3900
WORDS = [b'', b'$', b'.', b',', b'\x03\x03', b'USD ', b'\xa3', b'-', b'+', b'long text']


def money(r, regions, log):
    """moneypunct.py's localeconv and _Mbrtowc."""
    pointers, texts = [], bytearray()
    for _ in range(10):
        pointers.append(TEXTS + len(texts))
        texts += r.choice(WORDS) + b'\0'
    lconv = struct.pack('<10I', *pointers) + bytes(r.choice([0, 0, 0, 1, 1, 2, 0x7f, 0x7e, 0x80, 0xff, 0x10]) for _ in range(2))
    lconv += bytes(r.choice([0, 0, 1, 1, 1, 2, 0x81, 0xfe, 0xff]) for _ in range(4)) + bytes(r.choice([0, 1, 2, 3, 4, 4, 5, 0x7f, 0x80, 0xff]) for _ in range(2))
    regions.update({CONV: lconv, TEXTS: bytes(texts), 0x781fc8: struct.pack('<I', CONV)})
    strings(log, MBRTOWC, 0, out=9)
    blob(log, MBRTOWC, 1, 1)
    fill(log, MBRTOWC, 0, struct.pack('<8I', *[r.getrandbits(32) for _ in range(8)]), records=8)
    set_return(log, MBRTOWC, r.choice([0, 1, 1, 0xffffffff, r.getrandbits(32)]))


def names(r, regions):
    """_Getdays' and _Getmonths' table: empty names, short ones, a bad byte for a wide copy to pass on."""
    texts, pointers = bytearray(), []
    for i in range(43):
        pointers.append(NAME_TEXTS + len(texts))
        texts += r.choice([b'', b'n', b'Sunday', b'\xe9t\xe9', b'name%d' % i]) + b'\0'
    regions.update({NAME_TEXTS: bytes(texts), NAMES: struct.pack('<43I', *pointers), 0x781f8c: struct.pack('<I', NAMES), 0x709e78: b'Sun:Sunday\0\0', 0x709df0: b'Jan:January\0'})


def statics(r, regions):
    """The ids, caches, holders and setters' globals. The ids none the same, nor as localefacet.py's: a facet made here and replaced would be deleted by the exe's own vtable."""
    area = bytearray(r.bytes(0x100))    # 0xfb1de0-0xfb1ee0
    extra = bytearray(r.bytes(0x10))    # 0x7ced3c-0x7ced4c
    low = regions[0xfb1da0]             # (localefacet.py's, to 0xfb1de0)
    used = set(struct.unpack_from('<4I', low)) | set(struct.unpack('<I', regions[0xfb275c]))
    pool = [i for i in range(1, 0x18) if i not in used]
    r.shuffle(pool)

    def put(address, value):
        where, at = (low, address - 0xfb1da0) if address < 0xfb1de0 and address >= 0xfb1da0 else (area, address - 0xfb1de0) if address >= 0xfb1de0 else (extra, address - 0x7ced3c)
        struct.pack_into('<I', where, at, value)
    for id_ in IDS:
        put(id_, pool.pop() if pool and r.random() < 0.6 else 0)
    for cache in CACHES:
        put(cache, lf.facet_or_null(r))
    regions[0xfb1de0] = area
    regions[0x7ced3c] = extra
    regions[0xfb1450] = struct.pack('<I', r.choice([0, 0x4e4, r.getrandbits(32)]))


def string(r, held, k):
    """A std::string of the _Locinfo: unset, or a counted block in the heap."""
    if r.random() < 0.4:
        return (r.getrandbits(32), 0, 0, 0)
    if r.random() < 0.2:   # (counted but null: _Nullstr's)
        return (r.getrandbits(32), 0, 5, 0x1f)
    at = 0xc00 + 0x40 * k
    text = r.choice([b'', b'x', b'old text'])
    held[at - 1] = r.choice([0, 0, 1, 2, 0xfd, 0xfe, 0xff])
    held[at:at + len(text) + 1] = text + b'\0'
    return (r.getrandbits(32), HEAP + at, len(text), max(len(text), r.choice([1, 0x1f, 0x30])))


def case(r):
    name = r.deal(OPS)
    log = log_region()
    fails = [0, 0, 1, 1, 2, 3] if name in ('ctorc', 'ctorw') else [0] * 60 + [1, 2, 3, 5, 9] if name in ('makex', 'makew') else [0] * 30 + [1, 2, 3, 5]
    regions = {LOG: log, HEAP: heap(0x8000, fail=r.choice(fails)),
               IMPORT_TABLE: import_table(log, {'lcid': (lf.LCID, 0x6fd260, 0, 0, 0), 'version': (VERSION, 0x6fd264, 1, 0, 1), 'validcp': (lf.VALIDCP, 0x6fd288, 1, 0, 0), 'validlocale': (lf.VALIDLOCALE, 0x6fd284, 2, 0, 0),
                                                'enum': (lf.ENUM, 0x6fd1f8, 2, 0, 0), 'info': (lf.INFO, 0x6fd28c, 4, 0, 0b100), 'inc': (lf.INC, 0x6fd0c0, 1, 0, 0), 'dec': (lf.DEC, 0x6fd12c, 1, 0, 0)})}
    lf.state(r, regions, log)
    money(r, regions, log)
    names(r, regions)
    statics(r, regions)
    regions[lf.OUT] = r.bytes(0x20)
    if name in USES:
        return Call(USES[name][0], [lf.HANDLE, r.getrandbits(32), r.choice([0, 1, 1, 1, 0x100, 0x101])], regions, conv='c', ret='a')
    if name == 'getcoll':
        return Call(0x6ac9a0, [], regions, conv='c', ret='i')
    refs = r.choice([0, 1, 0xffffffff, r.getrandbits(32)])
    if name in ('collc', 'collw'):
        return Call(0x6a6fc0 if name == 'collc' else 0x69ff10, [lf.OUT, refs], regions, conv='t', ret='a')
    regions[INFO] = struct.pack('<8I', *string(r, regions[HEAP], 0), *string(r, regions[HEAP], 1))
    if name in ('tnx', 'tnw'):
        return Call(0x6ab0e0 if name == 'tnx' else 0x6a4490, [lf.OUT, INFO, refs], regions, conv='t', ret='a')
    if name == 'mpwintl':
        regions[FOUT] = r.bytes(0x30)
        return Call(0x6a1ee0, [FOUT, INFO, refs], regions, conv='t', ret='a')
    if name in ('makex', 'makew'):
        regions[0x7ced3c][0:4] = struct.pack('<I', lf.FACET)   # (bench limit: ctype<wchar_t>'s constructor takes the driver's WStream recorders, so the classic one is never made here)
        regions[lf.SELF] = lf.imp_bytes(r, regions, 1, r.choice([0, 1, 4, 0x20, 0x40]))
        cats = r.choice([0x3f, 0x3f, 0x3f, 1, 2, 4, 8, 0x10, 0x20, 0, r.getrandbits(32)])
        return Call(0x6a67f0 if name == 'makex' else 0x69a7a0, [INFO, cats, lf.SELF, r.choice([0, 0, lf.HANDLE, lf.HANDLE, lf.HANDLE])], regions, conv='c', ret='v')
    return Call(0x6aa200 if name == 'ctorc' else 0x6a3470, [lf.OUT, INFO, r.choice([0, 1, 0xffffffff])], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
