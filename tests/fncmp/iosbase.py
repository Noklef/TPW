# The C++ library's ios_base, _Xlen / _Xran, __Fiopen (src/game/core/iosbase) and exception's constructors (core/crt) against the originals. The locks do nothing on both sides (stack
# addresses, IosBaseEnv's lockit); _CxxThrowException is a recorder that returns, so the exception it is given (its dwords, less the allocator byte's garbage) is compared and
# the code after it runs; fopen, fseek, fclose are recorders; the locale reference's maker too. The stream's list blocks come from the heap region.
# usage: iosbase.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, blob, heap, log_region, recorder, set_return, strings, vtable  # noqa: E402

THROW, FOPEN, FSEEK, FCLOSE, LOCALE = 14, 16, 17, 18, 15
CALLBACK = 21


def lock(e):
    e.r['EAX'] = e.r['ECX']


B = Bench([(0x693340, 0x6934c0), (0x693830, 0x693a90), (0x6918f0, 0x691a20), (0x691ba0, 0x691cc4), (0x67d250, 0x67d320), (0x45b340, 0x45b9a0), (0x4f5a50, 0x4f5acc)],
          hooks={0x691de0: lock, 0x691ea0: lambda e: None, 0x67d3a0: _recorder(THROW, 2, com=True), 0x67b8a0: _recorder(FOPEN, 2, cdecl=True), 0x67c100: _recorder(FSEEK, 3, cdecl=True),
                 0x67b620: _recorder(FCLOSE, 1, cdecl=True), 0x699da0: _recorder(LOCALE, 0, cdecl=True)})
del B.emu.hooks[0x6918f0], B.emu.hooks[0x691ba0]   # (the bench's raising stand-ins: here the originals run)
OBJ, NODES, LOC, VT = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300
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


KINDS = {'clear': clear, 'xlen': xlen, 'callfns': callfns, 'addstd': addstd, 'init': init, 'tidy': tidy, 'dtor': dtor, 'fiopen': fiopen, 'exception_ctor': exception_ctor,
         'exception_copy': exception_copy}
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]


def case(r):
    return KINDS[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)
