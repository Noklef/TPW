# The C++ library's facets of char and _Locimp::_Makeloc (src/game/core/localefacet) against the originals, over the C library's setlocale and the strings' ports: the locks do nothing, _CxxThrowException a COM recorder (it returns, so what follows a throw runs), atexit, realloc, _nh_malloc, _Makexloc and _Makewloc cdecl recorders, ctype's constructor and the facets' destructors thiscall recorders.
# Bench limit: _Makeloc's throw is followed by a facet added at null, which faults in both: its log is not compared. The two use_facet routines return null after theirs, so bad_cast's text and info are compared there.
# usage: localefacet.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from locimp import A_REALLOC, FACET, FACETS, HANDLE, SELF, SOURCE, facets, imp_bytes, locale_state  # noqa: E402
from locinfo import A_MALLOC, BLOCKS, LOCK, NHMALLOC, REALLOC, UNLOCK  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _cdecl_log, _free, _recorder, answers, blob, fill, heap, import_table, log_region, set_return, strings  # noqa: E402

MAKEXLOC, MAKEWLOC, CTYPE, THROW, ATEXIT, MBRTOWC = 0, 1, 18, 14, 20, 23
LCID, VERSION, VALIDCP, VALIDLOCALE, ENUM, INFO, INC, DEC = 22, 23, 24, 25, 26, 27, 2, 3
SET_RELEASES = (0x5d15d0, 0x699650, 0x699890)
_at_exit = _recorder(ATEXIT, 1, cdecl=True)


def at_exit(e):
    """The shared globals' setters have their own atexit recorder."""
    (_cdecl_log if e.u32(e.r['ESP'] + (4 if e.tail else 0)) in SET_RELEASES else _at_exit)(e)


def lock(e):
    e.r['EAX'] = e.r['ECX']


RANGES = [(0x6a4460, 0x6a4489), (0x6a74c0, 0x6a74e9), (0x6ab0b0, 0x6ab0d9), (0x6a4770, 0x6a478d), (0x6ab390, 0x6ab3ad), (0x6a4510, 0x6a451c), (0x67acb0, 0x67acbe), (0x6a7290, 0x6a7367), (0x6a7370, 0x6a73e1), (0x6a74f0, 0x6a754f), (0x6a9200, 0x6a925f), (0x6a49e0, 0x6a4a28), (0x6a2400, 0x6a2441), (0x6a91b0, 0x6a91f1), (0x6a01d0, 0x6a02cf), (0x6a02d0, 0x6a036b), (0x6a0480, 0x6a04a9), (0x6a04b0, 0x6a050f), (0x69b460, 0x69b481), (0x69b490, 0x69b4ef), (0x69cb00, 0x69cb5f), (0x695910, 0x695a2d), (0x69c680, 0x69c7db), (0x69c7e0, 0x69c8d8), (0x695c00, 0x695c35), (0x69cac0, 0x69caf5), (0x693dc0, 0x693e20), (0x694710, 0x69477b), (0x694780, 0x6947df), (0x695c40, 0x695c9f), (0x695a30, 0x695ae6), (0x699410, 0x699440), (0x698c40, 0x698c61), (0x6991f0, 0x699408), (0x699440, 0x6995ca), (0x693e20, 0x693f4b), (0x694120, 0x69470a),
           (0x699bd0, 0x699c89), (0x69a560, 0x69a5c8), (0x699ce0, 0x699d47), (0x699d50, 0x699d9b), (0x4f21a0, 0x4f21cf), (0x4f1890, 0x4f18a2), (0x4f2180, 0x4f2199), (0x691530, 0x691536), (0x5d1460, 0x5d14e0), (0x6995d0, 0x699650), (0x699810, 0x699890),
           (0x698ea0, 0x698fc0), (0x6a65f0, 0x6a6644), (0x6a6650, 0x6a6669), (0x6a66c0, 0x6a66d9), (0x6a6670, 0x6a667c), (0x67d250, 0x67d340), (0x45b340, 0x45b9a0), (0x4f1400, 0x4f1511),
           (0x698fc0, 0x6990fa), (0x699100, 0x6991e2), (0x683ac0, 0x683d88), (0x683d90, 0x683ee5), (0x683ef0, 0x68400b), (0x684010, 0x684202), (0x684210, 0x684254), (0x684260, 0x684367), (0x684370, 0x6843da), (0x68db70, 0x68dbae),
           (0x67c250, 0x67c288), (0x68dbb0, 0x68dd41), (0x68dd50, 0x68ddad), (0x68ddb0, 0x68de3a), (0x68e120, 0x68e17e), (0x68e260, 0x68e29e), (0x68e340, 0x68e35b), (0x68e360, 0x68e443), (0x68e510, 0x68e549), (0x68e6c0, 0x68e6e1),
           (0x67d7f0, 0x67d82a), (0x67dcd0, 0x67dcf0), (0x67df40, 0x67e010), (0x67a480, 0x67a519), (0x688730, 0x68876d), (0x688770, 0x6887d1), (0x67ae00, 0x67aefe)]
HOOKS = {0x693bc0: _recorder(MBRTOWC, 5, cdecl=True), 0x67ad90: _free, 0x691de0: lock, 0x691ea0: lambda e: None, 0x67d3a0: _recorder(THROW, 2, com=True), 0x67a7d0: at_exit, 0x6a67f0: _recorder(MAKEXLOC, 4, cdecl=True), 0x69a7a0: _recorder(MAKEWLOC, 4, cdecl=True), 0x5cfb60: _recorder(CTYPE, 3),
                 0x681740: _recorder(REALLOC, 2, cdecl=True), 0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True), 0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True)}
B = Bench(RANGES, hooks=HOOKS)
OTHER_FACETS, LCONV, PCTYPE, LOCINFO, OUT, DEC_TEXT = RAM + 0x2000, RAM + 0x2200, RAM + 0x2400, RAM + 0x2800, RAM + 0x2900, RAM + 0x2a00
NAME = HEAP + 0xa00
OPS = [n for n in ('fdtor2', 'fdtor2', 'fdtor2', 'fdtor1', 'fdtor1', 'timevec', 'f7fac', 'f7facinit', 'f79c', 'f78c', 'widecopy', 'mptidyw', 'mptidyc', 'pairctor', 'pairinit', 'pairtidy', 'wbfc', 'numpunct', 'wnumpunct', 'wnumpunctinit', 'wnumget', 'wnumput', 'wcodecvt', 'tidy', 'wtidy', 'ctypeinit', 'numput', 'numget', 'numpunctinit', 'numpunctcopy', 'codecvt', 'usenumpunct', 'usenumpunct', 'usecodecvt', 'usecodecvt', 'getname', 'getname', 'makeloc', 'makeloc', 'makeloc', 'makeloc', 'makeloc')
       if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def facet_or_null(r):
    return r.choice([FACET + 8 * r.randrange(0x10), 0])


def statics(r, regions):
    """The ids, the classic facets' caches and the counter: each one new or set."""
    ids = [r.choice([0, 0, id_]) for id_ in r.sample(range(1, 0x18), 5)]    # (none the same: a facet made here and replaced would be deleted by the exe's own vtable)
    area = bytearray(0x40)
    struct.pack_into('<4I', area, 0, *[ids[i] for i in (0, 1, 2, 3)])    # (0xfb1da0 num_get, 0xfb1da4 ctype, 0xfb1da8 numpunct, 0xfb1dac num_put)
    struct.pack_into('<3I', area, 0x10, *[facet_or_null(r) for _ in range(3)])    # (0xfb1db0 num_put, 0xfb1db4 num_get, 0xfb1db8 numpunct)
    struct.pack_into('<3I', area, 0x1c, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32))    # (0xfb1dbc, 0xfb1dc0, 0xfb1dc4)
    struct.pack_into('<3I', area, 0x2c, SOURCE if r.random() < 0.85 else 0, SOURCE, r.randrange(0x18, 0x20))    # (0xfb1dcc classic, 0xfb1dd0 global, 0xfb1dd4 counter)
    struct.pack_into('<I', area, 0x38, r.choice([0, 0x1234]))    # (0xfb1dd8)
    regions[0xfb1da0] = area
    regions[0xf827dc] = struct.pack('<4I', r.getrandbits(32), facet_or_null(r), facet_or_null(r), r.getrandbits(32))
    regions[0xfb275c] = struct.pack('<I', ids[4])


def other_locale(r, regions):
    """The locale at HANDLE: its _Locimp at SOURCE with few facets, some null."""
    count = r.choice([0, 1, 3, 8, 0x20])
    regions[OTHER_FACETS] = struct.pack('<%dI' % 0x20, *[facet_or_null(r) for _ in range(0x20)])
    regions[SOURCE] = struct.pack('<5I', 0x709bac, 1, OTHER_FACETS, count, 0) + bytes([r.choice([0, 0, 1])]) + bytes(3) + bytes(0x10)
    regions[HANDLE] = struct.pack('<I', SOURCE)


def state(r, regions, log):
    locale_state(r, regions, log)
    regions[0xfb1438] = struct.pack('<6I', 0, r.choice([0, 0x409, 0x40c]), r.choice([0, 0x409, 0x40c]), 0, 0, 0)
    regions[0x709b60] = b'true\0\0\0\0false\0\0\0'
    regions[0x75c734] = b'missing locale facet\0\0\0\0'
    regions[PCTYPE] = r.bytes(0x200)
    regions[0x77ea90] = struct.pack('<I', PCTYPE)
    regions[DEC_TEXT] = r.choice([b'.', b',', b'\0']) + b'\0\0\0' + r.choice([b',', b'\0', b'.']) + b'\0\0\0' + r.choice([b'', b'\3', b'\3\3\0x', b'a longer grouping of bytes']) + bytes(0x20)
    regions[LCONV] = struct.pack('<3I', DEC_TEXT, DEC_TEXT + 4, DEC_TEXT + 8)
    regions[0x781fc8] = struct.pack('<I', LCONV)
    blob(log, THROW, 0, 0x7)   # (vtable, text, owned)
    answers(log, REALLOC, A_REALLOC)
    regions[A_REALLOC] = struct.pack('<40I', *[FACETS for _ in range(40)])
    regions[A_MALLOC] = struct.pack('<40I', *[BLOCKS[i] if r.random() < 0.995 else 0 for i in range(40)])
    facets(r, regions, 0x40)
    statics(r, regions)
    other_locale(r, regions)


def info_name(r, regions):
    """A _Locinfo with a name at +0x30: its text in the heap with an owners' byte."""
    held = regions[HEAP]
    text = r.choice([b'', b'C', b'French', b'LC_COLLATE=C;LC_CTYPE=German; and more, longer than a small string'])
    held[NAME - HEAP - 1] = r.choice([0, 0, 1, 2, 0xfd, 0xfe, 0xff])
    held[NAME - HEAP:NAME - HEAP + len(text) + 1] = text + b'\0'
    kind = r.random()
    if kind < 0.15:
        fields = (r.getrandbits(32), 0, 0, 0)
    else:
        fields = (r.getrandbits(32), NAME, len(text), max(len(text), r.choice([1, 0x1f, 0x40])))
    regions[LOCINFO] = r.bytes(0x30) + struct.pack('<4I', *fields)


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x8000, fail=r.choice([0] * 30 + [1, 2, 3, 5])),
               IMPORT_TABLE: import_table(log, {'lcid': (LCID, 0x6fd260, 0, 0, 0), 'version': (VERSION, 0x6fd264, 1, 0, 1), 'validcp': (VALIDCP, 0x6fd288, 1, 0, 0), 'validlocale': (VALIDLOCALE, 0x6fd284, 2, 0, 0), 'enum': (ENUM, 0x6fd1f8, 2, 0, 0), 'info': (INFO, 0x6fd28c, 4, 0, 0b100),
                                                'inc': (INC, 0x6fd0c0, 1, 0, 0), 'dec': (DEC, 0x6fd12c, 1, 0, 0)})}
    state(r, regions, log)
    regions[OUT] = r.bytes(0x20)
    if name in ('numput', 'numget', 'wnumget', 'wnumput', 'numpunct', 'wnumpunct', 'pairctor', 'wbfc', 'f7fac', 'f79c', 'f78c'):   # (the name read where it lies)
        regions[0x75c74c] = r.choice([b'C\0', b'C\0', b'C\0', b'x\0'])
    if name in ('wnumpunct', 'wnumpunctinit', 'pairctor', 'pairinit', 'widecopy'):   # (the character fetched or not; the slots out of the recorder's log, as in crt_wide.py: they are stack addresses)
        strings(log, MBRTOWC, 0, out=9)
        blob(log, MBRTOWC, 1, 1)
        set_return(log, MBRTOWC, r.choice([0, 1, 1, 0xffffffff]))
        fill(log, MBRTOWC, 0, r.bytes(4))
    if name in ('f7fac', 'f7facinit', 'widecopy'):
        regions[0x709d08] = r.choice([b'ab\0\0cd\0\0', b'\0\0\0\0abc\0', b'a\0\0\0\0\0\0\0'])
        if name == 'widecopy':
            return Call(0x6a49e0, [r.choice([0x709d08, 0x709d0c])], regions, conv='c', ret='a')
        if name == 'f7facinit':
            return Call(0x6a7370, [OUT, 0], regions, conv='t')
        return Call(0x6a7290, [OUT, r.choice([0, 1, 0xffffffff])], regions, conv='t', ret='a')
    if name in ('f79c', 'f78c'):
        return Call(0x6a74f0 if name == 'f79c' else 0x6a9200, [OUT, r.choice([0, 1, 0xffffffff])], regions, conv='t', ret='a')
    if name in ('fdtor2', 'fdtor1', 'timevec'):
        regions[OUT] = struct.pack('<4I', r.choice([0, HEAP + 0x100]), r.getrandbits(32), r.choice([0, HEAP + 0x200]), r.choice([0, HEAP + 0x300]))
        return Call(r.choice({'fdtor2': [0x6a4460, 0x6a74c0, 0x6ab0b0], 'fdtor1': [0x6a4770, 0x6ab390], 'timevec': [0x6a4510]}[name]), [OUT], regions, conv='t')
    if name in ('mptidyw', 'mptidyc'):
        regions[OUT] = struct.pack('<8I', *[r.getrandbits(32) if i not in (2, 4, 5, 6) else r.choice([0, HEAP + 0x100 * i]) for i in range(8)])
        return Call(0x6a2400 if name == 'mptidyw' else 0x6a91b0, [OUT], regions, conv='t')
    if name in ('pairctor', 'pairinit'):
        regions[0x709d08] = r.choice([b'ab\0\0cd\0\0', b'\0\0\0\0abc\0', b'a\0\0\0\0\0\0\0'])
        if name == 'pairinit':
            return Call(0x6a02d0, [OUT, 0], regions, conv='t')
        return Call(0x6a01d0, [OUT, r.choice([0, 1, 0xffffffff])], regions, conv='t', ret='a')
    if name == 'pairtidy':
        regions[OUT] = struct.pack('<8I', *[r.getrandbits(32) if i not in (2, 3) else r.choice([0, HEAP + 0x100 * i]) for i in range(8)])
        return Call(0x6a0480, [OUT], regions, conv='t')
    if name == 'wbfc':
        return Call(0x6a04b0, [OUT, r.choice([0, 1, 0xffffffff])], regions, conv='t', ret='a')
    if name in ('numpunct', 'wnumpunct'):
        return Call(0x695910 if name == 'numpunct' else 0x69c680, [OUT, r.choice([0, 1, 0xffffffff])], regions, conv='t', ret='a')
    if name == 'wnumpunctinit':
        return Call(0x69c7e0, [OUT, 0], regions, conv='t')
    if name in ('wnumget', 'wnumput'):
        return Call(0x69b490 if name == 'wnumget' else 0x69cb00, [OUT, r.choice([0, 1, 0xffffffff])], regions, conv='t', ret='a')
    if name == 'wcodecvt':
        return Call(0x69b460, [OUT, 0, r.choice([0, 1, 0xffffffff])], regions, conv='t', ret='a')
    if name in ('tidy', 'wtidy'):
        regions[OUT] = struct.pack('<8I', *[r.getrandbits(32) if i not in (2, 4, 5) else r.choice([0, HEAP + 0x100 * i]) for i in range(8)])
        return Call(0x695c00 if name == 'tidy' else 0x69cac0, [OUT], regions, conv='t')
    if name == 'ctypeinit':
        return Call(0x694710, [OUT, 0], regions, conv='t')
    if name in ('numput', 'numget'):
        return Call(0x694780 if name == 'numput' else 0x695c40, [OUT, r.choice([0, 1, 0xffffffff])], regions, conv='t', ret='a')
    if name == 'numpunctinit':
        return Call(0x695a30, [OUT, 0], regions, conv='t')
    if name == 'numpunctcopy':
        return Call(0x699410, [DEC_TEXT + r.choice([0, 4, 8]), r.getrandbits(32)], regions, conv='c', ret='a')
    if name == 'codecvt':
        return Call(0x698c40, [OUT, 0, r.choice([0, 1, 0xffffffff])], regions, conv='t', ret='a')
    if name in ('usenumpunct', 'usecodecvt'):
        return Call(0x6991f0 if name == 'usenumpunct' else 0x699440, [HANDLE, r.getrandbits(32), r.choice([0, 1, 1, 1, 0x100, 0x101])], regions, conv='c', ret='a')
    info_name(r, regions)
    if name == 'getname':
        return Call(0x693e20, [LOCINFO, r.choice([OUT, OUT, OUT, LOCINFO + 0x30])], regions, conv='t', ret='a')
    regions[SELF] = imp_bytes(r, regions, 1, r.choice([0, 1, 4, 0x20, 0x40]))
    set_return(log, CTYPE, FACET + 8 * r.randrange(0x10))
    return Call(0x694120, [LOCINFO, r.choice([0, 2, 8, 0xa, 0xa, 0xa, 0x3f, 0x3f, 0xfffffff5]), SELF, r.choice([0, 0, HANDLE, HANDLE, HANDLE])], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
