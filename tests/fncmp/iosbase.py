# The C++ library's ios_base, _Xlen / _Xran, __Fiopen (src/game/core/iosbase) and exception's constructors (core/crt) against the originals. The locks do nothing on both sides (stack
# addresses, IosBaseEnv's lockit); _CxxThrowException is a recorder that returns, so the exception it is given (its dwords, less the allocator byte's garbage) is compared and
# the code after it runs; fopen, fseek, fclose are recorders; the locale reference's maker too. The stream's list blocks come from the heap region.
# usage: iosbase.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _free, _recorder, blob, fill, heap, log_region, recorder, set_return, strings, vtable  # noqa: E402

THROW, FOPEN, FSEEK, FCLOSE, LOCALE = 14, 16, 17, 18, 15
CALLBACK = 21
MBRTOWC = 23
SYNC = (21, 22, 24)


def lock(e):
    e.r['EAX'] = e.r['ECX']


B = Bench([(0x692650, 0x692797), (0x6930e0, 0x693227), (0x693340, 0x6934c0), (0x693830, 0x693a90), (0x6918f0, 0x691a20), (0x691ba0, 0x691cc4), (0x67d250, 0x67d340), (0x45b340, 0x45b9a0), (0x4f5a50, 0x4f5acc),
           (0x4f0100, 0x4f0138), (0x4f1620, 0x4f1640), (0x4f1820, 0x4f1830), (0x4f1870, 0x4f1890), (0x4f2180, 0x4f21a0), (0x4ef840, 0x4ef860), (0x4f1ad0, 0x4f1b30), (0x4f1ba0, 0x4f1bd0),
           (0x4f1560, 0x4f1620), (0x4f1be0, 0x4f1ca0), (0x693dc0, 0x693e20), (0x4f11e0, 0x4f1260), (0x4f12b0, 0x4f12c5), (0x4f1320, 0x4f1335), (0x4f1850, 0x4f186e), (0x4f1b80, 0x4f1b9e), (0x4f1830, 0x4f1850)],
          hooks={0x67ad90: _free, 0x693bc0: _recorder(MBRTOWC, 5, cdecl=True), 0x691de0: lock, 0x691ea0: lambda e: None, 0x67d3a0: _recorder(THROW, 2, com=True), 0x67b8a0: _recorder(FOPEN, 2, cdecl=True), 0x67c100: _recorder(FSEEK, 3, cdecl=True),
                 0x67b620: _recorder(FCLOSE, 1, cdecl=True), 0x699da0: _recorder(LOCALE, 0, cdecl=True)})
del B.emu.hooks[0x6918f0], B.emu.hooks[0x691ba0]   # (the bench's raising stand-ins: here the originals run)
OBJ, NODES, LOC, VT = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300
STREAM, VBT = RAM + 0x800, RAM + 0x900
STD = 0xfb1d60


def throw_log(log):
    blob(log, THROW, 0, 0x77)   # (vtable, text, owned, what.ptr, what.len, what.res; not what's allocator byte, stack garbage in the original)


def stream(r, size=0x40):
    return bytearray(r.bytes(size))


def clear(r):
    log = log_region()
    throw_log(log)
    obj = stream(r)
    struct.pack_into('<I', obj, 8, r.choice([0, 1, 2, 4, 7, 6, r.getrandbits(32)]))
    state = r.choice([0, 1, 2, 3, 4, 5, 6, 7, r.getrandbits(32)])
    reraise = r.choice([0, 0, 0, 1, 0x100, 0x101])
    return Call(0x693340, [OBJ, state, reraise], {LOG: log, OBJ: obj, HEAP: heap(0x400)}, conv='t', ret='v')


def init_dtor(r):
    """ios_base::Init's destructors: the counter at 1 mostly; three streams with their ios at +8 or +0xc, in one region (some only 0x38 apart); each sync its recorder's answer by the case."""
    log = log_region()
    throw_log(log)
    address, counter, streams = r.choice([(0x692650, 0x7822b0, (0xfb1a00, 0xfb1b28, 0xfb1970)), (0x6930e0, 0x78240c, (0xfb1cc8, 0xfb1b60, 0xfb1ba0))])
    area = bytearray(r.bytes(0x3c0))
    table = bytearray(0x200)
    for i, s in enumerate(streams):
        vb = r.choice([8, 0xc])
        at = s - 0xfb1970
        struct.pack_into('<I', area, at, VBT + 8 * i)
        struct.pack_into('<I', table, 8 * i + 4, vb)
        struct.pack_into('<I', area, at + vb + 4, r.choice([0, 0, 0, 1, 2, 4, 6, 0x100]))
        struct.pack_into('<I', area, at + vb + 8, r.choice([0, 0, 0, 4, 7]))
        struct.pack_into('<I', area, at + vb + 0x28, OBJ + 0x10 * i)
    buffers = bytearray(r.bytes(0x30))
    methods = bytearray(0x30 * 3)
    for i in range(3):
        struct.pack_into('<I', buffers, 0x10 * i, VT + 0x30 * i)
        struct.pack_into('<I', methods, 0x30 * i + 0x2c, recorder(SYNC[i], 0))
        set_return(log, SYNC[i], r.choice([0, -1 & 0xffffffff, -1 & 0xffffffff, 1]))
    regions = {LOG: log, 0xfb1970: area, VBT: table, OBJ: buffers, VT: methods, HEAP: heap(0x400), counter: struct.pack('<I', r.choice([1, 1, 1, 1, 0, 2, 5]))}
    return Call(address, [r.getrandbits(32)], regions, conv='t', ret='v')


def xlen(r):
    log = log_region()
    throw_log(log)
    return Call(r.deal([0x6918f0, 0x691ba0]), [], {LOG: log, HEAP: heap(0x400)}, conv='c', ret='v')


def callback_nodes(r, base, n):
    """A list of n nodes (next, index, fn) in `base`, as bytes, and its head."""
    data = bytearray()
    for i in range(n):
        data += struct.pack('<III', base + 0x10 * (i + 1) if i + 1 < n else 0, r.getrandbits(32), recorder(CALLBACK, 3, cdecl=True)) + b'\0' * 4
    return data or bytearray(4), base if n else 0


def callfns(r):
    log = log_region()
    data, head = callback_nodes(r, NODES, r.choice([0, 1, 2, 5]))
    obj = stream(r)
    struct.pack_into('<I', obj, 0x1c, head)
    return Call(0x6938e0, [OBJ, r.getrandbits(32)], {LOG: log, OBJ: obj, NODES: data}, conv='t', ret='v')


def addstd(r):
    obj = stream(r)
    index = r.choice([0, 1, 3, 7, 8, 9])
    struct.pack_into('<I', obj, 0x24, index)
    slots = [r.choice([0, OBJ, r.getrandbits(32) | 1, r.getrandbits(32) | 1]) for _ in range(10)]
    table = struct.pack('<10I', *slots) + r.bytes(0x10)
    return Call(0x693910, [OBJ], {OBJ: obj, STD: table}, conv='t', ret='v')


def init(r):
    log = log_region()
    set_return(log, LOCALE, LOC)
    return Call(0x693960, [OBJ], {LOG: log, OBJ: stream(r), LOC: r.bytes(8), 0xfb1dd0: struct.pack('<I', LOC)}, conv='t', ret='v')


def heap_lists(r, obj, h):
    """Two lists of blocks in the heap region, for +0x18 and +0x1c."""
    at = 0x100
    for off in (0x18, 0x1c):
        head = 0
        for _ in range(r.choice([0, 1, 3])):
            struct.pack_into('<III', h, at, head, r.getrandbits(32), recorder(CALLBACK, 3, cdecl=True))   # (the callback for the list that Callfns walks)
            head, at = HEAP + at, at + 0x10
        struct.pack_into('<I', obj, off, head)


def tidy(r):
    log = log_region()
    h = heap(0x400)
    obj = stream(r)
    heap_lists(r, obj, h)
    return Call(0x6939a0, [OBJ], {LOG: log, OBJ: obj, HEAP: h}, conv='t', ret='v')


def dtor(r):
    log = log_region()
    h = heap(0x400)
    obj = stream(r)
    heap_lists(r, obj, h)
    index = r.choice([0, 0, 1, 2, 7])
    struct.pack_into('<II', obj, 0x20, r.choice([LOC, LOC, LOC, 0]), index)
    loc = struct.pack('<II', VT, r.choice([0, 1, 1, 2, 0xffffffff, 0xfffffffe])) + r.bytes(8)
    opens = bytes(r.choice([0, 1, 1, 2, 0x7f, 0x80, 0xff]) for _ in range(8))
    regions = {LOG: log, OBJ: obj, HEAP: h, LOC: loc, VT: vtable({0: 1}, size=2), 0xfb1d88: opens}
    return Call(0x693830, [OBJ], regions, conv='t', ret='v')


def fiopen(r):
    log = log_region()
    strings(log, FOPEN, 3)
    set_return(log, FOPEN, r.choice([NODES, NODES, NODES, 0]))
    set_return(log, FSEEK, r.choice([0, 0, 1, 0xffffffff]))
    modes = [0x1, 0x2, 0x12, 0x9, 0x21, 0x22, 0x32, 0x29, 0x3, 0x23, 0x33, 0x2b]
    mode = r.choice(modes + [m | 4 for m in modes] + [0, 4, r.getrandbits(32) & 0xff, r.getrandbits(32)])
    return Call(r.deal([0x6939f0, 0x693a70]), [LOC, mode], {LOG: log, LOC: b'name.txt\0'}, conv='c', ret='a')


def exception_ctor(r):
    text = b'x' * r.choice([0, 1, 3, 4, 7, 30]) + b'\0'
    regions = {HEAP: heap(0x200, fail=r.choice([0, 0, 0, 1])), OBJ: r.bytes(0xc), NODES: struct.pack('<I', LOC), LOC: text}
    return Call(0x67d250, [OBJ, NODES], regions, conv='t', ret='a')


def exception_copy(r):
    text = b'y' * r.choice([0, 1, 3, 4, 7, 30]) + b'\0'
    regions = {HEAP: heap(0x200, fail=r.choice([0, 0, 0, 1])), OBJ: r.bytes(0xc), NODES: struct.pack('<III', r.getrandbits(32), LOC, r.choice([0, 1, r.getrandbits(32)])), LOC: text}
    return Call(0x67d2b0, [OBJ, NODES], regions, conv='t', ret='a')


def ios_clears(r):
    log = log_region()
    throw_log(log)
    obj = stream(r)
    struct.pack_into('<I', obj, 8, r.choice([0, 0, 4, 7, r.getrandbits(32)]))
    struct.pack_into('<I', obj, 0x28, r.choice([0, 1, r.getrandbits(32)]))
    state = r.choice([0, 0, 1, 2, 4, r.getrandbits(32)])
    return Call(r.deal([0x4f0110, 0x4f1620]), [OBJ, state, r.choice([0, 0, 1, 0x100])], {LOG: log, OBJ: obj, HEAP: heap(0x400)}, conv='t', ret='v')


def ios_dtors(r):
    regions = dtor(r).regions
    if r.random() < 0.5:
        return Call(0x4f0100, [OBJ], regions, conv='t', ret='v')
    return Call(0x4ef840, [OBJ, r.choice([0, 1, 2, 3, 0x100, 0x101, r.getrandbits(32)])], regions, conv='t', ret='a')


def deleting(r):
    """The deleting destructors. A stream: its vbtable first, then the virtual base (dtor's object moved behind it), the vtable word stored at the vbtable's offset."""
    regions = dtor(r).regions
    flags = r.choice([0, 1, 2, 3, 0x100, 0x101, r.getrandbits(32)])
    which = r.deal(['ios', 'ostream', 'istream', 'ostream_del', 'istream_del', 'bad_cast', 'ctype'])
    if which == 'ios':
        return Call(0x4f1210, [OBJ, flags], regions, conv='t', ret='a')
    if which == 'bad_cast':
        regions[OBJ] = struct.pack('<III', r.getrandbits(32), r.getrandbits(32), r.choice([0, 1, r.getrandbits(32)]))
        return Call(0x4f1850, [OBJ, flags], regions, conv='t', ret='a')
    if which == 'ctype':
        obj = bytearray(r.bytes(0x18))
        struct.pack_into('<I', obj, 0x14, r.choice([0, 1, r.getrandbits(32)]))
        regions[OBJ] = obj
        return Call(0x4f1b80, [OBJ, flags], regions, conv='t', ret='a')
    base = 4 if which.startswith('ostream') else 8
    obj = regions.pop(OBJ)
    stream = bytearray(struct.pack('<I', VBT)) + bytearray(r.bytes(base - 4)) + bytearray(obj) + bytearray(r.bytes(0x10))
    regions[STREAM] = stream
    regions[VBT] = struct.pack('<II', r.getrandbits(32), base + r.choice([0, 0x40, 0x44]))   # (the word, in the game the stream's +base; elsewhere here so that it shows)
    if which.endswith('_del'):
        return Call(0x4f11e0 if base == 4 else 0x4f1230, [STREAM + base, flags], regions, conv='t', ret='a')
    return Call(0x4f12b0 if base == 4 else 0x4f1320, [STREAM], regions, conv='t', ret='v')


def widen_setup(r, log):
    strings(log, MBRTOWC, 0, out=0xb)
    fill(log, MBRTOWC, 0, r.bytes(4))
    set_return(log, MBRTOWC, r.choice([1, 1, 1, 0, 0xffffffff]))


def streams(r):
    log = log_region()
    throw_log(log)
    widen_setup(r, log)
    set_return(log, LOCALE, LOC)
    obj = bytearray(r.bytes(0x60))
    derived = r.choice([0, 1, 1, 1, r.getrandbits(32) | 1])
    ostream = r.random() < 0.5
    struct.pack_into('<I', obj, 0, VT)
    regions = {LOG: log, OBJ: obj, LOC: r.bytes(8), 0xfb1dd0: struct.pack('<I', LOC), VT: struct.pack('<II', r.getrandbits(32), 4 if ostream else 8), HEAP: heap(0x400),
               STD: r.bytes(0x28), 0xfb1d88: r.bytes(0x10)}
    buffer = r.choice([0, 1, r.getrandbits(32)])
    isstd = r.choice([0, 1, 0x100, 0x101])
    if ostream:
        return Call(0x4f1560, [OBJ, buffer, isstd, r.choice([0, 1, 0x100, 0x101]), derived], regions, conv='t', ret='a')
    return Call(0x4f1be0, [OBJ, buffer, isstd, derived], regions, conv='t', ret='a')


def bad_cast(r):
    text = b'z' * r.choice([0, 1, 7, 30]) + b'\0'
    regions = {HEAP: heap(0x200, fail=r.choice([0, 0, 0, 1])), OBJ: r.bytes(0xc), NODES: struct.pack('<III', r.getrandbits(32), LOC, r.choice([0, 1, r.getrandbits(32)])), LOC: text}
    which = r.deal(['ctor', 'copy', 'dtor', 'throw'])
    if which == 'throw':
        log = log_region()
        blob(log, THROW, 0, 0x7)   # (vtable, text, owned)
        regions[LOG] = log
        return Call(0x4f1830, [NODES], regions, conv='t', ret='v')
    if which == 'ctor':
        return Call(0x4f2180, [OBJ, LOC], regions, conv='t', ret='a')
    if which == 'copy':
        return Call(0x4f1870, [OBJ, NODES], regions, conv='t', ret='a')
    regions[OBJ] = struct.pack('<III', r.getrandbits(32), r.getrandbits(32), r.choice([0, 1, r.getrandbits(32)]))
    return Call(0x4f1820, [OBJ], regions, conv='t', ret='v')


def widen(r):
    log = log_region()
    widen_setup(r, log)
    set_return(log, MBRTOWC, r.choice([1, 1, 0, 0xffffffff]))
    if r.random() < 0.5:
        # (A low byte 0xff sign-extends to the end of file, which never reaches mbrtowc.)
        return Call(0x4f1ad0, [OBJ, r.getrandbits(24) << 8 | r.choice([0xff, 0x80, r.getrandbits(8)])], {LOG: log}, conv='t', ret='a')
    n = r.choice([0, 1, 2, 9])
    regions = {LOG: log, LOC: bytes(r.choice([0xff, 0x80, r.getrandbits(8)]) for _ in range(0x10)), NODES: r.bytes(0x20)}
    return Call(0x4f1af0, [OBJ, LOC, LOC + n, NODES], regions, conv='t', ret='a')


def ctype_dtor(r):
    obj = bytearray(r.bytes(0x18))
    struct.pack_into('<I', obj, 0x14, r.choice([0, 1, r.getrandbits(32)]))
    return Call(0x4f1ba0, [OBJ], {OBJ: obj, HEAP: heap(0x40)}, conv='t', ret='v')


KINDS = {'init_dtor': init_dtor, 'clear': clear, 'xlen': xlen, 'callfns': callfns, 'addstd': addstd, 'init': init, 'tidy': tidy, 'dtor': dtor, 'fiopen': fiopen, 'exception_ctor': exception_ctor,
         'exception_copy': exception_copy, 'ios_clears': ios_clears, 'ios_dtors': ios_dtors, 'streams': streams, 'bad_cast': bad_cast, 'widen': widen, 'ctype_dtor': ctype_dtor, 'deleting': deleting}
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]


def case(r):
    return KINDS[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)
