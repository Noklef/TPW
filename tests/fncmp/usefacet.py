# More of the C++ library's use_facet and the time facets' constructors (src/game/core/localefacet) against the originals: localefacet.py's world (the locks, the throw and atexit
# recorders, the locale at HANDLE with its ids and caches new or set), _Mbrtowc a recorder as moneypunct.py has it, a localeconv and the C library's day and month names made here.
# usage: usefacet.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
import localefacet as lf  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, blob, fill, heap, import_table, log_region, set_return, strings  # noqa: E402

# k 29 for GetVersionEx: k 23 is _Mbrtowc's in the driver, as moneypunct.py has it.
MBRTOWC, VERSION = 23, 29
B = Bench(lf.RANGES + [(0x691530, 0x691536), (0x693dc0, 0x693e20), (0x6a4f20, 0x6a5172), (0x6ab880, 0x6aba77), (0x6902b0, 0x690394), (0x6903a0, 0x69048a),
                       (0x6a4d70, 0x6a4f13), (0x6a5180, 0x6a531a), (0x6a5320, 0x6a549f), (0x6a58d0, 0x6a5a5a), (0x6ab6d0, 0x6ab873), (0x6aba80, 0x6abc1a), (0x6abc20, 0x6abe79),
                       (0x6a3470, 0x6a35fd), (0x6aa200, 0x6aa365)],
          hooks={**lf.HOOKS, 0x693bc0: _recorder(MBRTOWC, 5, cdecl=True)})
# name: the address, then for use_facet the id, the cache and the holder
USES = {'moneywintl': (0x6a4d70, 0xfb1de8, 0xfb1e2c, 0xfb1e34), 'moneyw': (0x6a5180, 0xfb1e0c, 0xfb1e28, 0xfb1e38), 'moneycintl': (0x6ab6d0, 0xfb1e80, 0xfb1eb0, 0xfb1eb4),
        'moneyc': (0x6aba80, 0xfb1e90, 0xfb1eac, 0xfb1eb8), 'f709bec': (0x6a5320, 0xfb1e08, 0xfb1e40, 0xfb1e3c), 'timec': (0x6abc20, 0xfb1e84, 0xfb1ec0, 0xfb1ebc),
        'codecvtw': (0x6a58d0, 0xfb1e00, 0x7ced40, 0x7ced48)}
OPS = [n for n in list(USES) + ['ctorc', 'ctorw'] if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
CONV, TEXTS, NAMES, NAME_TEXTS, INFO = RAM + 0x3000, RAM + 0x3100, RAM + 0x3200, RAM + 0x3300, RAM + 0x3800
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
    """The ids, caches and holders (the ids apart from localefacet.py's: lf.statics has 1-0x17; a facet made and replaced would be deleted by the exe's own vtable)."""
    area = bytearray(r.bytes(0xf0))
    extra = bytearray(r.bytes(0xc))
    for (_address, id_, cache, _hold) in USES.values():
        if id_ >= 0xfb1de0:
            struct.pack_into('<I', area, id_ - 0xfb1de0, r.choice([0, 0, r.randrange(1, 0x18)]))
        where, at = (area, cache - 0xfb1de0) if cache >= 0xfb1de0 else (extra, cache - 0x7ced40)
        struct.pack_into('<I', where, at, lf.facet_or_null(r))
    regions[0xfb1de0] = area
    regions[0x7ced40] = extra


def string(r, held, k):
    """A std::string of the _Locinfo: unset, or a counted block in the heap."""
    if r.random() < 0.4:
        return (r.getrandbits(32), 0, 0, 0)
    at = 0xc00 + 0x40 * k
    text = r.choice([b'', b'x', b'old text'])
    held[at - 1] = r.choice([0, 0, 1, 2, 0xfd, 0xfe, 0xff])
    held[at:at + len(text) + 1] = text + b'\0'
    return (r.getrandbits(32), HEAP + at, len(text), max(len(text), r.choice([1, 0x1f, 0x30])))


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x8000, fail=r.choice([0] * 30 + [1, 2, 3, 5])),
               IMPORT_TABLE: import_table(log, {'lcid': (lf.LCID, 0x6fd260, 0, 0, 0), 'version': (VERSION, 0x6fd264, 1, 0, 1), 'validcp': (lf.VALIDCP, 0x6fd288, 1, 0, 0), 'validlocale': (lf.VALIDLOCALE, 0x6fd284, 2, 0, 0),
                                                'enum': (lf.ENUM, 0x6fd1f8, 2, 0, 0), 'info': (lf.INFO, 0x6fd28c, 4, 0, 0b100), 'inc': (lf.INC, 0x6fd0c0, 1, 0, 0), 'dec': (lf.DEC, 0x6fd12c, 1, 0, 0)})}
    lf.state(r, regions, log)
    money(r, regions, log)
    names(r, regions)
    statics(r, regions)
    regions[lf.OUT] = r.bytes(0x20)
    if name in USES:
        return Call(USES[name][0], [lf.HANDLE, r.getrandbits(32), r.choice([0, 1, 1, 1, 0x100, 0x101])], regions, conv='c', ret='a')
    regions[INFO] = struct.pack('<8I', *string(r, regions[HEAP], 0), *string(r, regions[HEAP], 1))
    return Call(0x6aa200 if name == 'ctorc' else 0x6a3470, [lf.OUT, INFO, r.choice([0, 1, 0xffffffff])], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
