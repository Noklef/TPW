# The input system (src/game/core/inputsystem: 0x617610-0x618c80, 0x5f5590-0x5f5620) against the originals: the real family, its small ctors and the heap; recorders for the library 0x631870-0x634ee0, Class703ca0_Ctor (its vtable's slot 2 is the library's),
# EventQueue_Post, App_PumpMessages, the two imports and the fake objects' slots. Recorder slots are short here (32 for 35 calls): 29 and 30 are each shared by two library calls that never meet in one call, 3 by 0x6326e0 and EventQueue_Post.
# Masked: the two allocator bytes of the 0x38-byte object (+0x14, +0x24): copies of unset stack bytes in the original, nothing reads them (the listing: Sentinel_Ctor and 0x4f0150 write them; 0x617e00 and 0x617f40 do not read them).
# usage: inputsystem.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=the names of CASES below)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, Mask, _recorder, blob, echo, fill, fill_this, heap, log_fold, log_region, put, recorder, set_return, strings, vtable  # noqa: E402

C631870, C633360, C633900, C634220, C6345D0, C634EE0, CLASSCA0 = 10, 15, 14, 6, 22, 24, 4
F631FC0, F632020, F6323E0, F6336F0, F633160, F633920, F634310 = 29, 30, 30, 20, 19, 18, 21
F633250, F632350, F6326E0, F633B90, F633340, F633EA0, F6345A0, F634A30 = 16, 11, 3, 25, 13, 5, 9, 23
F6328B0, F6328D0, F6335B0, F633560, F6341D0, F633D10, F633700, POST, PUMP = 17, 29, 27, 28, 26, 31, 12, 3, 6
THIS_SLOT2, SAY, SHOW = 14, 40, 41

B = Bench([(0x4f0150, 0x4f0180), (0x5ce530, 0x5ce550), (0x5ce910, 0x5ce930), (0x5f5590, 0x5f5640), (0x617610, 0x617a40), (0x617bd0, 0x618c90), (0x633120, 0x633150), (0x6338b0, 0x6338d0), (0x633b60, 0x633b80)],
          hooks={0x631870: _recorder(C631870, 0), 0x633360: _recorder(C633360, 0), 0x633900: _recorder(C633900, 0), 0x634220: _recorder(C634220, 0), 0x6345d0: _recorder(C6345D0, 0), 0x634ee0: _recorder(C634EE0, 0), 0x633dc0: _recorder(CLASSCA0, 0),
                 0x631fc0: _recorder(F631FC0, 0), 0x632020: _recorder(F632020, 0), 0x6323e0: _recorder(F6323E0, 0), 0x6336f0: _recorder(F6336F0, 0), 0x633160: _recorder(F633160, 0), 0x633920: _recorder(F633920, 0), 0x634310: _recorder(F634310, 0),
                 0x633250: _recorder(F633250, 1), 0x632350: _recorder(F632350, 1), 0x6326e0: _recorder(F6326E0, 1), 0x633b90: _recorder(F633B90, 1), 0x633340: _recorder(F633340, 1), 0x633ea0: _recorder(F633EA0, 1), 0x6345a0: _recorder(F6345A0, 1), 0x634a30: _recorder(F634A30, 1),
                 0x6328b0: _recorder(F6328B0, 2), 0x6328d0: _recorder(F6328D0, 2), 0x6335b0: _recorder(F6335B0, 2), 0x633560: _recorder(F633560, 2), 0x6341d0: _recorder(F6341D0, 2), 0x633d10: _recorder(F633D10, 2), 0x633700: _recorder(F633700, 2),
                 0x5fbf80: _recorder(POST, 2), 0x5f86e0: _recorder(PUMP, 0, cdecl=True)})
SYSTEM = 0xfa6e40
THIS, VT, THIS_VT, OUT, SLOT, EVENT, NODES, FAKES = RAM, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x410, RAM + 0x420, RAM + 0x500, RAM + 0x1000
FLAGS = 0x703c6c
# Where the 0x38-byte object lands: first, or after the blocks of InputSystem_Init before it (rounded to 16).
DEVICE74_ALONE, DEVICE74_IN_INIT = 0x10, 0x10 + 0x30 + 0x80 + 0x50 + 0x20 + 0x130
RESULTS = [0, 0, 0, 0, 0, 1, 0x4ffe0002, 0x80004005, 0xcffe0000]


def fake(i):
    return FAKES + 0x100 * i


def result(r):
    return r.choice(RESULTS)


def setup(r, log, regions):
    """The answers common to all: out arguments masked and filled, the ctors' answers, the fake objects."""
    log_fold(log)
    for k in (F6326E0, F633B90, F633340, F633EA0, F6345A0, F634A30, F6328B0, F6328D0, F6335B0, F633560, F6341D0, F633D10, 8):
        strings(log, k, 0, 1)
    for which, k in enumerate((F6326E0, F633B90, F633340, F633EA0, F6345A0, F634A30, F6328B0)):
        fill(log, k, 0, struct.pack('<3I', result(r), result(r), result(r)), which=which, records=3)
    echo(log, F6326E0, 0)
    echo(log, F6328B0, 0)
    for i, k in enumerate((C631870, C633360, C6345D0, C634EE0, CLASSCA0)):
        set_return(log, k, r.choice([fake(i), fake(i), fake(i), fake(i), fake(i), fake(i), fake(i), 0]))
    regions[VT] = vtable({1: 1, 2: 1, 8: 2})
    for i in range(10):
        regions[fake(i)] = struct.pack('<I', VT) + r.bytes(0xfc)


def fields(r, regions, where, devices=None):
    """The system: its vtable a fake's with slot 2 taking an out argument, each field null at times."""
    o = bytearray(r.bytes(0x100))
    put(o, 0, 'I', THIS_VT)
    table = vtable({1: 1, 8: 2})
    put(table, 8, 'I', recorder(THIS_SLOT2, 1))
    regions[THIS_VT] = table
    put(o, 0x54, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    for i, offset in enumerate((0x58, 0x5c, 0x60, 0x64, 0x68, 0x6c, 0x70, 0x74, 0x78, 0x80)):
        put(o, offset, 'I', r.choice([fake(i), fake(i), fake(i), 0]))
    if devices is not None:
        put(o, 0x58, 'I', r.choice([fake(0), 0]) if devices else fake(0))
        put(o, 0x5c, 'I', r.choice([fake(1), 0]) if devices else fake(1))
    regions[where] = o
    return o


def device74(r, regions):
    """A built 0x38-byte object: the list at +0x14 (its sentinel at +0x18, its count at +0x1c) with 0-3 nodes."""
    n = r.choice([0, 1, 1, 2, 3])
    o = bytearray(r.bytes(0x38))
    put(o, 0x18, 'I', NODES)
    put(o, 0x1c, 'I', n)
    ring = [NODES + 0x20 * i for i in range(n + 1)]
    place = bytearray(0x20 * (n + 1))
    for i, a in enumerate(ring):
        struct.pack_into('<3I', place, a - NODES, ring[(i + 1) % len(ring)], ring[i - 1], r.getrandbits(32))
    regions[THIS] = o
    regions[NODES] = place


def create(address, offset=None):
    def run(r, log, regions):
        regions[SLOT] = r.bytes(4)
        masks = [Mask(HEAP, [offset + 0x14, offset + 0x24], 'allocator bytes: unset stack bytes copied, never read')] if offset else []
        return Call(address, [OUT, r.choice([SLOT] * 7 + [0])], regions, conv='c', ret='a', masks=masks)
    return run


def on_system(address, ret, *args):
    def run(r, log, regions):
        fields(r, regions, THIS)
        return Call(address, [THIS, *args], regions, conv='t', ret=ret)
    return run


def opening(address, offset, slot2=True):
    def run(r, log, regions):
        o = fields(r, regions, THIS)
        if r.randrange(4):
            put(o, offset, 'I', 0)
        fill(log, THIS_SLOT2, 0, struct.pack('<I', result(r)), which=7)
        strings(log, THIS_SLOT2, 0, 1)
        echo(log, THIS_SLOT2, 0)
        return Call(address, [THIS, OUT], regions, conv='t', ret='a')
    return run


def wrapper_open(address, offset):
    def run(r, log, regions):
        o = fields(r, regions, SYSTEM)
        if r.randrange(4):
            put(o, offset, 'I', 0)
        fill(log, THIS_SLOT2, 0, struct.pack('<I', result(r)), which=7)
        strings(log, THIS_SLOT2, 0, 1)
        echo(log, THIS_SLOT2, 0)
        return Call(address, [OUT], regions, conv='c', ret='a')
    return run


def closing(address, offset, deleting=False, where=THIS):
    def run(r, log, regions):
        fields(r, regions, where)
        if r.randrange(3):   # (the library's own removal clears it at times)
            fill_this(log, F6328D0, bytes(4), offset - 0x10, 0)
        if where == SYSTEM:
            return Call(address, [], regions, conv='c', ret='v')
        return Call(address, [THIS, OUT], regions, conv='t', ret='a')
    return run


def destroying(address, deleting):
    def run(r, log, regions):
        fields(r, regions, THIS)
        if r.randrange(3):
            fill_this(log, F6328D0, bytes(4), r.choice([0x48, 0x4c]), 0)
        args = [THIS, r.choice([0, 1, 2, 3, 0xfffffffe, r.getrandbits(32)])] if deleting else [THIS]
        return Call(address, args, regions, conv='t', ret='a' if deleting else 'v')
    return run


def update(r, log, regions):
    fields(r, regions, THIS)
    set_return(log, 7, r.choice([0, 1, 2, 3, 5, 12]))
    return Call(0x6176e0, [THIS, r.getrandbits(32), r.choice([0, 0, 1, 2, 4, 100, r.getrandbits(32)]), r.getrandbits(32)], regions, conv='t', ret='a')


def wrapper_update(r, log, regions):
    fields(r, regions, SYSTEM)
    set_return(log, 7, r.choice([0, 1, 2, 3, 5]))
    return Call(0x5f5590, [r.getrandbits(32), r.choice([0, 0, 1, 2, 4, r.getrandbits(32)]), r.getrandbits(32), r.choice([0, 0, 1, r.getrandbits(32)])], regions, conv='c', ret='a')


def init(r, log, regions):
    fields(r, regions, THIS)
    return Call(0x617760, [THIS, OUT], regions, conv='t', ret='a', masks=[Mask(HEAP, [DEVICE74_IN_INIT + 0x14, DEVICE74_IN_INIT + 0x24], 'allocator bytes: unset stack bytes copied, never read')])


def posting(log):
    strings(log, POST, 0, 0)
    blob(log, POST, 1, 0b11)


def value(r, log, regions):
    posting(log)
    fields(r, regions, THIS, devices=False)
    return Call(0x6185b0, [THIS, OUT, r.getrandbits(32)], regions, conv='t', ret='a')


def event(r, log, regions):
    posting(log)
    kind = r.choice([4, 4, 4, 6, 7, 7, 7, 0, 5, 8, r.getrandbits(32)])
    fields(r, regions, THIS, devices=kind == 7)
    regions[EVENT] = struct.pack('<5I', kind, r.choice([0, r.getrandbits(32)]), r.getrandbits(32), r.getrandbits(32), r.choice([0, 0, 1, r.getrandbits(32)]))
    table = bytearray(0x400)
    put(table, 0x6fd1e0 - IMPORT_TABLE, 'I', recorder(SAY, 1, True))
    put(table, 0x6fd30c - IMPORT_TABLE, 'I', recorder(SHOW, 1, True))
    strings(log, SAY, 1)
    regions[IMPORT_TABLE] = table
    regions[FLAGS] = r.bytes(16)
    return Call(0x618610, [THIS, r.getrandbits(32), EVENT], regions, conv='t', ret='v')


def poll_all(r, log, regions):
    posting(log)
    fields(r, regions, THIS, devices=True)
    return Call(0x618730, [THIS, OUT], regions, conv='t', ret='a')


def poll(r, log, regions):
    posting(log)
    fields(r, regions, THIS)
    return Call(0x618780, [THIS, OUT, r.choice([fake(3) + 0x10, fake(3) + 0x10, fake(3) + 0x10, 0])], regions, conv='t', ret='a')


def device74_dtor(address, deleting, base=0):
    def run(r, log, regions):
        device74(r, regions)
        args = [THIS + base, r.choice([0, 1, 2, 3, r.getrandbits(32)])] if deleting else [THIS]
        return Call(address, args, regions, conv='t', ret='a' if deleting else 'v')
    return run


def chain64_dtor(address, deleting, base=0):
    def run(r, log, regions):
        regions[THIS] = r.bytes(0x74)
        args = [THIS + base, r.choice([0, 1, 2, 3, r.getrandbits(32)])] if deleting else [THIS]
        return Call(address, args, regions, conv='t', ret='a' if deleting else 'v')
    return run


CASES = {'update': update, 'wupdate': wrapper_update, 'init': init, 'baseinit': on_system(0x618330, 'a', OUT),
         'open5c': opening(0x617860, 0x5c), 'open58': opening(0x6178c0, 0x58), 'bopen5c': opening(0x618400, 0x5c), 'bopen58': opening(0x618510, 0x58), 'wopen58': wrapper_open(0x5f55c0, 0x58), 'wopen5c': wrapper_open(0x5f5600, 0x5c),
         'close5c': closing(0x618490, 0x5c), 'close58': closing(0x6184d0, 0x58), 'wclose58': closing(0x5f55e0, 0x58, where=SYSTEM), 'wclose5c': closing(0x5f5620, 0x5c, where=SYSTEM),
         'dtor': destroying(0x617630, False), 'del': destroying(0x617610, True), 'bdtor': destroying(0x618260, False), 'bdel': destroying(0x618240, True), 'del744': destroying(0x617950, True),
         'value': value, 'event': event, 'pollall': poll_all, 'poll': poll,
         'c70': create(0x617bd0), 'c74': create(0x617c70, DEVICE74_ALONE), 'c60': create(0x6187f0), 'c64': create(0x6188c0), 'c68': create(0x618a40), 'c5c': create(0x618ae0), 'c58': create(0x618bb0),
         'd74': device74_dtor(0x617e00, False), 'del74': device74_dtor(0x617de0, True), 'thunk74': device74_dtor(0x6181b0, True, 0x10),
         'd64': chain64_dtor(0x6189d0, False), 'del64': chain64_dtor(0x6189b0, True), 'thunk64': chain64_dtor(0x618c80, True, 0x64)}
OPS = [n for n in CASES if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x1000, fail=r.choice([0] * 12 + [1, 2, 3, 4, 5, 6])), OUT: r.bytes(4)}
    setup(r, log, regions)
    return CASES[name](r, log, regions)


if __name__ == '__main__':
    B.main(case)
