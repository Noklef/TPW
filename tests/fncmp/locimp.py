# The C++ library's std::locale::_Locimp and _Nomemory (src/game/core/locimp) against the originals, over the C library's setlocale and the strings' ports: the locks do nothing, _CxxThrowException a COM recorder (it returns, so what follows a throw runs), _Makeloc, atexit, realloc and _nh_malloc cdecl recorders, the facets' destructors thiscall recorders.
# usage: locimp.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
import locinfo  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _free, _recorder, answers, blob, heap, import_table, log_region, recorder, strings  # noqa: E402

REALLOC, LOCK, UNLOCK, FACET_DTOR, INIT, NHMALLOC = 6, 7, 8, 12, 9, 28
THROW, ATEXIT, MAKELOC = 14, 20, 21
LCID, VERSION, VALIDCP, VALIDLOCALE, ENUM, INFO = 22, 23, 24, 25, 26, 27


def lock(e):
    e.r['EAX'] = e.r['ECX']


B = Bench([(0x6a6680, 0x6a66a0), (0x6a66a0, 0x6a66be), (0x699f70, 0x69a079), (0x69a080, 0x69a179), (0x699ee0, 0x699f70), (0x6a65f0, 0x6a6644), (0x6a6650, 0x6a6669), (0x6a66c0, 0x6a66d9), (0x6a6670, 0x6a667c), (0x698ea0, 0x698fc0), (0x698c70, 0x698e7e), (0x693f50, 0x69409a), (0x67d250, 0x67d340), (0x45b340, 0x45b9a0), (0x4f1400, 0x4f1511),
           (0x698fc0, 0x6990fa), (0x699100, 0x6991e2), (0x683ac0, 0x683d88), (0x683d90, 0x683ee5), (0x683ef0, 0x68400b), (0x684010, 0x684202), (0x684210, 0x684254), (0x684260, 0x684367), (0x684370, 0x6843da), (0x68db70, 0x68dbae),
           (0x67c250, 0x67c288), (0x68dbb0, 0x68dd41), (0x68dd50, 0x68ddad), (0x68ddb0, 0x68de3a), (0x68e120, 0x68e17e), (0x68e260, 0x68e29e), (0x68e340, 0x68e35b), (0x68e360, 0x68e443), (0x68e510, 0x68e549), (0x68e6c0, 0x68e6e1),
           (0x67d7f0, 0x67d82a), (0x67dcd0, 0x67dcf0), (0x67df40, 0x67e010), (0x67a480, 0x67a519), (0x688730, 0x68876d), (0x688770, 0x6887d1), (0x67ae00, 0x67aefe)],
          hooks={0x67ad90: _free, 0x691de0: lock, 0x691ea0: lambda e: None, 0x67d3a0: _recorder(THROW, 2, com=True), 0x67a7d0: _recorder(ATEXIT, 1, cdecl=True), 0x694120: _recorder(MAKELOC, 4, cdecl=True),
                 0x681740: _recorder(REALLOC, 2, cdecl=True), 0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True), 0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True)})
OBJECT, FROM, FACETS, VTABLE, A_MALLOC, A_REALLOC, HANDLE = (RAM + 0x1000 + 0x100 * i for i in (0, 1, 2, 4, 5, 6, 7))   # (past locinfo.py's own)
SELF, SOURCE, FACET = RAM + 0x1800, RAM + 0x1900, RAM + 0x1a00
TEXT = HEAP + 0x800
BLOCKS = [HEAP + 0x5000 + 0x100 * i for i in range(40)]
OPS = [n for n in ('ctor', 'dtor', 'dtor', 'tidyatexit', 'badctor', 'badcopy', 'baddtor', 'baddelete', 'badraise', 'nomemory', 'nomemory', 'addfac', 'addfac', 'addfac', 'addfac', 'copy', 'copy', 'copy', 'copy', 'copy', 'localeadd', 'localeadd', 'localeadd', 'localeadd')
       if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def facets(r, regions, count):
    """An array of count facets at FACETS, some null, of FACET's with their counts."""
    regions[VTABLE] = struct.pack('<I', recorder(FACET_DTOR, 1))
    data = bytearray()
    for i in range(0x10):
        data += struct.pack('<II', VTABLE, r.choice([0, 1, 1, 2, 2, 5, 0xfffffffe, 0xffffffff]))
    regions[FACET] = data
    regions[FACETS] = struct.pack('<%dI' % 0x40, *[r.choice([FACET + 8 * (i & 0xf), FACET + 8 * (i & 0xf), 0]) for i in range(0x40)])


def name_string(r, regions):
    """A name string's dwords: its allocator's byte in the garbage, and its block in the heap."""
    held = regions[HEAP]
    text = r.choice([b'', b'C', b'French', b'a longer name of the locale'])
    owners = r.choice([0, 0, 1, 2, 0xfd, 0xfe, 0xff])
    held[TEXT - HEAP - 1] = owners
    held[TEXT - HEAP:TEXT - HEAP + len(text) + 1] = text + b'\0'
    kind = r.random()
    if kind < 0.15:
        return r.getrandbits(32), 0, 0, 0
    if kind < 0.25:
        return r.getrandbits(32), TEXT, 0, r.choice([0, 1, 0x1f, 0x40])
    return r.getrandbits(32), TEXT, len(text), r.choice([max(len(text), 1), 0x1f, 0x20, 0x40])


def imp_bytes(r, regions, refs, count):
    return struct.pack('<5I', 0x709bac, refs, FACETS, count, r.getrandbits(32)) + r.bytes(4) + struct.pack('<4I', *name_string(r, regions))


def locale_state(r, regions, log):
    locinfo.locale_regions(r, regions, log)
    regions[0xfb18fc] = struct.pack('<I', recorder(INFO, 4, True))
    regions[0x75c74c] = b'C\0\0\0'
    del regions[0x6fe5b0]
    regions[0x6fe5ac] = struct.pack('<I', 0xffffffff) + bytes(0x10)   # (npos, _Nullstr)


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x8000),
               IMPORT_TABLE: import_table(log, {'lcid': (LCID, 0x6fd260, 0, 0, 0), 'version': (VERSION, 0x6fd264, 1, 0, 1), 'validcp': (VALIDCP, 0x6fd288, 1, 0, 0), 'validlocale': (VALIDLOCALE, 0x6fd284, 2, 0, 0), 'enum': (ENUM, 0x6fd1f8, 2, 0, 0), 'info': (INFO, 0x6fd28c, 4, 0, 0b100)})}
    answers(log, REALLOC, A_REALLOC)
    regions[A_REALLOC] = struct.pack('<40I', *[BLOCKS[i] for i in range(40)])
    blob(log, THROW, 0, 0x7)   # (vtable, text, owned)
    strings(log, MAKELOC, 0, 1)
    regions[0x709f08] = b'bad allocation\0\0'
    regions[0xfb1e68] = bytes(0x18)
    if name in ('badctor', 'badcopy', 'baddtor', 'baddelete', 'badraise', 'nomemory'):
        answers(log, NHMALLOC, A_MALLOC)
        regions[A_MALLOC] = struct.pack('<40I', *[BLOCKS[i] if r.random() < 0.9 else 0 for i in range(40)])
        regions[HEAP][0x800:0x803] = b'ab\0'
        object_ = struct.pack('<3I', r.getrandbits(32), TEXT, r.choice([0, 1, 1])) if r.random() < 0.9 else struct.pack('<3I', r.getrandbits(32), 0, 0)
        regions[OBJECT] = object_
        regions[FROM] = object_
        if name == 'badctor':
            return Call(0x6a6650, [OBJECT, r.choice([0x709f08, TEXT])], regions, conv='t', ret='a')
        if name == 'badcopy':
            return Call(0x6a66c0, [OBJECT, FROM], regions, conv='t', ret='a')
        if name == 'baddelete':
            return Call(0x6a66a0, [OBJECT, r.choice([0, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')
        if name == 'badraise':
            return Call(0x6a6680, [FROM], regions, conv='t')
        if name == 'baddtor':
            return Call(0x6a6670, [OBJECT], regions, conv='t')
        if r.random() < 0.5:
            regions[0xfb1e68] = struct.pack('<I', r.choice([1, 3])) + bytes(4) + object_
        else:
            regions[0xfb1e68] = struct.pack('<I', r.choice([0, 2])) + bytes(4) + r.bytes(0xc)
        return Call(0x6a65f0, [], regions, conv='c')
    facets(r, regions, 0x40)
    if name == 'ctor':   # (the name's text read where it lies)
        regions[SELF] = r.bytes(0x28)
        regions[0x747828] = r.choice([b'*\0', b'*\0', b'\0', b'a longer name than the smallest block\0'])
        return Call(0x699f70, [SELF, r.getrandbits(32)], regions, conv='t', ret='a')
    if name == 'dtor':
        regions[SELF] = imp_bytes(r, regions, r.choice([0, 1, 2]), r.choice([0, 1, 2, 7, 0x10, 0x40]))
        return Call(0x69a080, [SELF], regions, conv='t')
    if name == 'tidyatexit':
        regions[0xfb1dd0] = struct.pack('<I', r.choice([0, FACET + 8 * r.randrange(0x10)]))
        return Call(0x699ee0, [], regions, conv='c')
    if name == 'addfac':
        count = r.choice([0, 1, 4, 0x20, 0x40])
        regions[SELF] = imp_bytes(r, regions, 1, count)
        regions[A_REALLOC] = struct.pack('<40I', *[FACETS + 0x100 * 0 for _ in range(40)])
        return Call(0x698ea0, [SELF, FACET + 8 * r.randrange(0x10), r.choice([0, 1, 3, 4, 0x1e, 0x1f, 0x20, 0x21, 0x3f])], regions, conv='t')
    locale_state(r, regions, log)
    if name == 'copy':
        count = r.choice([0, 0, 1, 2, 7, 0x10])
        regions[SOURCE] = imp_bytes(r, regions, r.choice([1, 2]), count)
        regions[SELF] = r.bytes(0x28)
        regions[0xfb1dcc] = struct.pack('<I', SOURCE if r.random() < 0.3 else 0)
        return Call(0x698c70, [SELF, SOURCE], regions, conv='t', ret='a')
    regions[SELF] = imp_bytes(r, regions, r.choice([0, 1, 1, 2, 3, 0xffffffff]), r.choice([1, 4, 0x10, 0x20]))
    regions[HANDLE] = struct.pack('<I', SELF)
    regions[0xfb1dcc] = struct.pack('<I', SELF if r.random() < 0.15 else 0)
    regions[A_REALLOC] = struct.pack('<40I', *[FACETS for _ in range(40)])
    return Call(0x693f50, [HANDLE, FACET + 8 * r.randrange(0x10), r.choice([0, 1, 3, 0x1f]), r.choice([0, 0, 1, 0x3f])], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
