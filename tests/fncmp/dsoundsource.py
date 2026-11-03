# The DirectSound source (src/game/core/dsoundsource: 0x6d2d60, 0x6d2de0, 0x6d30b0, 0x6d3190-0x6d3a20) against the originals: its COM interfaces and Win32 imports recorders answering 0 mostly, the slots real (as soundslot.py) on a recorder buffer,
# the release list made and empty. The thread start, the vector iterators and the refill recorders through the env.
# usage: dsoundsource.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, blob, fb, fill, heap, import_table, log_region, recorder, set_return, strings, vtable  # noqa: E402

BEGIN, WAITLOOP, REFILL, VCTOR, VDTOR = 17, 18, 21, 60, 61
COCREATE, WINDOW, PRIORITY, ENTER, LEAVE, DELETE, SETEVENT, CLOSEH, CREATEEVENT = 22, 23, 24, 25, 26, 27, 28, 29, 30
QI, RELEASE, S13, S15, S16, S17, S19, S23, CAPS, LS3, S35, X3, X12, X10, S36, I14, I12 = 50, 51, 52, 53, 54, 55, 56, 57, 49, 58, 43, 44, 45, 47, 39, 41, 37
B = Bench([(0x6d2d60, 0x6d2d70), (0x6d2de0, 0x6d3b73), (0x6d6c70, 0x6d6cf0), (0x6d6db0, 0x6d7690), (0x67b410, 0x67b440), (0x6bb220, 0x6bb250), (0x6b9d50, 0x6b9d63)],
          hooks={0x67e050: _recorder(BEGIN, 3, cdecl=True), 0x6d3800: _recorder(WAITLOOP, 0), 0x6d3850: _recorder(REFILL, 1), 0x67acf0: _recorder(VCTOR, 5, com=True), 0x67afa0: _recorder(VDTOR, 4, com=True)})
IMPORTS = {'window': (WINDOW, 0x6fd370, 0, 0, 0), 'priority': (PRIORITY, 0x6fd1fc, 2, 0, 0), 'enter': (ENTER, 0x6fd140, 1, 0, 0), 'leave': (LEAVE, 0x6fd130, 1, 0, 0), 'delete': (DELETE, 0x6fd0d0, 1, 0, 0),
           'SetEvent': (SETEVENT, 0x6fd124, 1, 0, 0), 'CloseHandle': (CLOSEH, 0x6fd13c, 1, 0, 0), 'CreateEvent': (CREATEEVENT, 0x6fd0d4, 4, 0, 0)}
OPS_TABLE = {'thread': (0x6d2d60, 'c', 'v'), 'open': (0x6d2de0, 't', 'a'), 'close': (0x6d30b0, 't', 'v'), 'distances': (0x6d3190, 't', 'a'), 'isset': (0x6d3230, 't', 'a'), 'free': (0x6d32a0, 't', 'a'), 'freeall': (0x6d3310, 't', 'a'),
             'flush': (0x6d33c0, 't', 'v'), 'play': (0x6d3430, 't', 'a'), 'position': (0x6d3600, 't', 'a'), 'pitch': (0x6d3680, 't', 'a'), 'gain': (0x6d3900, 't', 'a'), 'set48': (0x6d3980, 't', 'a'), 'deldtor': (0x6d3a00, 't', 'a'), 'dtor': (0x6d3a20, 't', 'v')}
SRC, OUT, DS, DS_VT, LIS, LIS_VT, EXTRA, EXTRA_VT, INNER, INNER_VT, OBJ, OBJ_VT, SLOTS, VEC, INFO, EVENTS, ANSWERS, NEW = (RAM + 0x400 * i for i in range(18))
FLAG, LIST, PITCH = 0xfb2728, 0xfb2730, 0x784000
COM = {5: 2, 7: 2, 13: 2, 14: 1, 20: 4, 21: 4, 38: 4, 40: 2, 42: 2, 46: 2, 48: 2, 59: 3}
OPS = [n for n in OPS_TABLE if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
NANS = [0x7fc00000, 0xffc00000, 0x7f800001, 0xff812345, 0x7fc12345]


def odd_float(r):
    return r.choice(NANS) if r.random() < 0.15 else fb(r.uniform(-8, 8)) if r.random() < 0.7 else r.getrandbits(32)


def com_table(size, slots):
    table = bytearray(4 * size)
    for slot, (k, n) in slots.items():
        struct.pack_into('<I', table, 4 * slot, recorder(k, n, True))
    return table


def answer(r):
    return r.choice([0, 0, 0, 0, 0, 1, 0x80004005, r.getrandbits(32)])


def slots(r):
    table = bytearray(r.bytes(0x2c * 8))
    for i in range(8):
        for at in (4, 8, 0xc, 0x14, 0x18, 0x1c):
            struct.pack_into('<I', table, 0x2c * i + at, odd_float(r))
        struct.pack_into('<I', table, 0x2c * i, r.choice([OBJ, OBJ, OBJ, 0]))
        struct.pack_into('<I', table, 0x2c * i + 0x10, r.choice([0, r.randrange(-0x180, 0x181) & 0xffffffff]))
    return table


def case(r):
    name = r.deal(OPS)
    at, conv, ret = OPS_TABLE[name]
    log = log_region()
    table = import_table(log, IMPORTS)
    table.extend(bytes(0x100))   # (CoCreateInstance, 0x6fd454, is past import_table()'s 0x400 bytes)
    struct.pack_into('<I', table, 0x6fd454 - IMPORT_TABLE, recorder(COCREATE, 5, True))
    for k in [v[0] for v in IMPORTS.values()] + [COCREATE, QI, S15, S16, S17, S19, S23, S35]:
        set_return(log, k, answer(r))
    for k in COM:
        set_return(log, k, r.choice([0, 0, 0, 1, 0x80004005, r.getrandbits(32)]))
    source = bytearray(r.bytes(0x60))
    channels = r.choice([1, 2, 4, 8])
    struct.pack_into('<II', source, 0, 0x711b08, channels)
    struct.pack_into('<I', source, 8, r.getrandbits(32) & ~0x80)
    struct.pack_into('<3I', source, 0x18, r.choice([DS, DS, DS, 0]), r.choice([LIS, 0]), r.choice([EXTRA, 0]))
    struct.pack_into('<3I', source, 0x24, r.choice([INNER, 0]), SLOTS, EVENTS)
    struct.pack_into('<I', source, 0x30, 2 * channels + 1)
    lst = bytearray(r.bytes(0x30))
    struct.pack_into('<4I', lst, 0, r.getrandbits(32), 0, 0, 0)
    flag = bytearray(r.bytes(4))
    flag[0] |= 1
    regions = {LOG: log, IMPORT_TABLE: table, SRC: source, OUT: r.bytes(4), DS: struct.pack('<I', DS_VT), LIS: struct.pack('<I', LIS_VT), EXTRA: struct.pack('<I', EXTRA_VT), INNER: struct.pack('<I', INNER_VT), OBJ: struct.pack('<I', OBJ_VT) + r.bytes(0xc),
               DS_VT: com_table(24, {0: (QI, 3), 2: (RELEASE, 1), 11: (CAPS, 2), 13: (S13, 1), 15: (S15, 4), 16: (S16, 2), 17: (S17, 3), 19: (S19, 3), 23: (S23, 2)}),
               LIS_VT: com_table(4, {2: (RELEASE, 1), 3: (LS3, 4)}), EXTRA_VT: com_table(37, {2: (RELEASE, 1), 3: (X3, 2), 10: (X10, 2), 12: (X12, 2), 35: (S35, 2), 36: (S36, 2)}), INNER_VT: com_table(15, {2: (RELEASE, 1), 12: (I12, 3), 14: (I14, 3)}),
               OBJ_VT: vtable({}, size=64, com=COM), SLOTS: slots(r), VEC: struct.pack('<4I', *[odd_float(r) for _ in range(4)]), EVENTS: struct.pack('<24I', *[r.getrandbits(32) for _ in range(24)]), FLAG: flag, LIST: lst, HEAP: heap(0x4000, fail=r.choice([0, 0, 0, 0, 1])), 0xfb1f04: struct.pack('<I', 0)}
    index = r.randrange(8)
    value = r.getrandbits(32)
    if name == 'open':
        return open_case(r, log, regions, source)
    if name == 'close':
        h = bytearray(regions[HEAP])
        struct.pack_into('<I', h, 0x100, r.randrange(1, 9))
        regions[HEAP] = h
        h[0x200:0x220] = r.bytes(0x20)
        events = HEAP + 0x200 if struct.unpack_from('<I', source, 8)[0] & 0x40 else r.choice([0, HEAP + 0x200])
        struct.pack_into('<3I', source, 0x28, r.choice([0, HEAP + 0x104]), events, r.randrange(1, 9))
        return Call(at, [SRC], regions, conv=conv, ret=ret)
    if name == 'distances':
        return Call(at, [SRC, OUT, index, VEC], regions, conv=conv, ret=ret)
    if name == 'isset':
        return Call(at, [SRC, index], regions, conv=conv, ret=ret)
    if name == 'free':
        return Call(at, [SRC, OUT, index], regions, conv=conv, ret=ret)
    if name == 'freeall':
        return Call(at, [SRC, OUT], regions, conv=conv, ret=ret)
    if name == 'flush':
        return Call(at, [SRC], regions, conv=conv, ret=ret)
    if name == 'play':
        return play_case(r, log, regions, source, channels)
    if name == 'position':
        return Call(at, [SRC, OUT, index, VEC, value], regions, conv=conv, ret=ret)
    if name == 'pitch':
        regions[PITCH] = struct.pack('<%dI' % 0x308, *[odd_float(r) for _ in range(0x308)])
        return Call(at, [SRC, OUT, value, index, r.randrange(-0x180, 0x181) & 0xffffffff], regions, conv=conv, ret=ret)
    if name in ('gain', 'set48'):
        return Call(at, [SRC, OUT, index, value], regions, conv=conv, ret=ret)
    if name == 'deldtor':
        h = bytearray(regions[HEAP])
        h[0x10:0x10 + len(source)] = source
        regions[HEAP] = h
        return Call(at, [HEAP + 0x10, r.choice([0, 1, 2, 3, r.getrandbits(32)])], regions, conv=conv, ret=ret)
    return Call(at, [SRC], regions, conv=conv, ret=ret)


def open_case(r, log, regions, source):
    set_return(log, COCREATE, r.choice([0, 0, 0, 0, 0, 0, 0x80004005]))
    fill(log, COCREATE, 4, struct.pack('<I', DS))
    fill(log, QI, 2, struct.pack('<2I', EXTRA if r.random() < 0.9 else 0, LIS), records=2, which=1)
    fill(log, S35, 1, struct.pack('<I', INNER), which=2)
    for k in (S16, S19, S15):
        set_return(log, k, r.choice([1, 1, 1, 1, 1, 0] if k == S16 else [0, 0, 0, 0, 0, 0x80004005]))
    set_return(log, QI, r.choice([0, 0, 0, 0, 0x80004005]))
    regions[ANSWERS] = struct.pack('<40I', *[r.choice([1, 1, 1, 1, 1, 0]) for _ in range(40)])
    answers(log, S16, ANSWERS)
    caps = bytearray(r.bytes(0x24))
    struct.pack_into('<I', caps, 4, r.choice([0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0xff, 0x42, r.getrandbits(32)]))
    strings(log, CAPS, 0, out=0b10)
    fill(log, CAPS, 1, bytes(caps), which=3)
    regions[0xfb2720] = bytes([r.getrandbits(8) | 1])
    set_return(log, CAPS, r.choice([0, 0, 1]))
    set_return(log, BEGIN, r.getrandbits(32))
    set_return(log, WINDOW, r.getrandbits(32))
    set_return(log, PRIORITY, r.choice([1, 1, 1, 0, 0x80000000]))
    info = bytearray(r.bytes(0x70))
    struct.pack_into('<2I', info, 0x40, r.randrange(1, 9), r.randrange(1, 9))
    struct.pack_into('<I', info, 0x4c, r.getrandbits(32))
    regions[INFO] = info
    return Call(0x6d2de0, [SRC, OUT, INFO], regions, conv='t', ret='a')


def play_case(r, log, regions, source, channels):
    struct.pack_into('<I', source, 0x18, DS)
    shift = r.choice([0, 0, 1, 2, 33])
    info = bytearray(r.bytes(0x70))
    index = r.randrange(8)
    struct.pack_into('<I', info, 0x2c, index << (shift & 31))
    struct.pack_into('<I', info, 0x20, r.choice([0, r.getrandbits(16)]))
    regions[INFO] = info
    regions[0x782db8] = bytes([shift])
    strings(log, S17, 0, out=0b100)
    set_return(log, S17, r.choice([0, 0, 0, 0, 0x80004005]))
    fill(log, S17, 2, struct.pack('<I', r.choice([NEW, NEW, 0])))
    regions[NEW] = struct.pack('<I', OBJ_VT)
    for k in (5, 59):
        set_return(log, k, r.choice([0, 0, 0, 0, 0, 0x80004005]))
    blob(log, 7, 1, 15)   # (the wave format on the stack)
    regions[PITCH] = struct.pack('<%dI' % 0x308, *[odd_float(r) for _ in range(0x308)])
    return Call(0x6d3430, [SRC, OUT, INFO], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
