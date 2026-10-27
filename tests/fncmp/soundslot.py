# A sound object's slot (src/game/core/soundslot) against the originals: a slot at SLOT whose +0 is a COM object at OBJ (a recorder vtable:
# slots 5, 7, 13, 14, 20, 21, 38, 40, 42, 46, 48 and 59, answering any; 21's three outs filled with any dwords) or null, its +0x24 flags any (the cached settings any, their floats now
# and then NaNs); the release list (0x6d7460's, made already: its flag at 0xfb2728 set) at LIST with its head null or a node, the heap
# failing now and then, Enter/LeaveCriticalSection recorders. The setters' vectors any floats (NaNs quiet and signalling now and then: the
# position's _isnan 0x67b410), the pitch steps -0x180 to 0x180 (0 now and then, any now and then) over a pitch table at 0x784010 of random
# floats (signalling NaNs among them: the positive steps' fld / fstp), the gain any. The guard's destructor: its flag set or not, the sound
# system at 0xfb2724 a recorder (slot 10 thiscall, its out on the stack logged as 0). The source's (0x711b08) slot position (0x6d35d0): as
# getpos's, the source's slot array (+0x28) placed so that the index (small or any) names the slot.
# usage: soundslot.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, fill, heap, import_table, log_region, set_return, strings, vtable  # noqa: E402

# name: (address, returns)
CALLS = {'init': (0x6d6dd0, 'a'), 'call7': (0x6d6df0, 'a'), 'call5': (0x6d6e10, 'a'), 'set': (0x6d6e40, 'a'), 'call59': (0x6d6e70, 'a'),
         'isset': (0x6d6eb0, 'a'), 'restart': (0x6d6ec0, 'a'), 'guard': (0x6d7060, 'v'), 'free': (0x6d7080, 'a'),
         'position': (0x6d7130, 'a'), 'pitch': (0x6d71f0, 'a'), 'distances': (0x6d7260, 'a'), 'gain': (0x6d72d0, 'a'), 'set48': (0x6d7330, 'a'),
         'getpos': (0x6d70c0, 'a'), 'srcgetpos': (0x6d35d0, 'a')}
B = Bench([(0x6d6dd0, 0x6d6dec), (0x6d6df0, 0x6d6e0b), (0x6d6e10, 0x6d6e40), (0x6d6e40, 0x6d6e6e), (0x6d6e70, 0x6d6ea8), (0x6d6eb0, 0x6d6eba),
           (0x6d6ec0, 0x6d6f6c), (0x6d7060, 0x6d707f), (0x6d7080, 0x6d70b9), (0x6d7130, 0x6d71e8), (0x6d71f0, 0x6d7256), (0x6d7260, 0x6d72ca),
           (0x6d72d0, 0x6d732b), (0x6d7330, 0x6d7369), (0x67b410, 0x67b440), (0x6d7460, 0x6d7493), (0x6d7520, 0x6d75b7), (0x6d70c0, 0x6d712d),
           (0x6bb220, 0x6bb250), (0x6d35d0, 0x6d35f6)])
SLOT, OUT, OBJ, OVT, NODE, NEW, VEC, SYSOBJ, SVT = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x500, RAM + 0x600, RAM + 0x700, RAM + 0x800, RAM + 0x900
FLAG, LIST, SYSTEM, PITCH = 0xfb2728, 0xfb2730, 0xfb2724, 0x784000
SOURCE = RAM + 0xa00   # (the 0x711b08 source: +0x28 its slots)
ENTER_K, LEAVE_K = 40, 41
COM = {5: 2, 7: 2, 13: 2, 14: 1, 20: 4, 21: 4, 38: 4, 40: 2, 42: 2, 46: 2, 48: 2, 59: 3}
NANS = [0x7fc00000, 0xffc00000, 0x7f800001, 0xff812345, 0x7fc12345]
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def odd_float(r):
    return r.choice(NANS) if r.random() < 0.15 else r.f()


def case(r):
    name = r.deal(OPS)
    address, ret = CALLS[name]
    log = log_region()
    imports = import_table(log, {'enter': (ENTER_K, 0x6fd140, 1, 0, 0), 'leave': (LEAVE_K, 0x6fd130, 1, 0, 0)})
    for k in COM:
        if r.random() < 0.8:
            set_return(log, k, r.choice([0, 1, 0x80004005, r.getrandbits(32)]))
    slot = bytearray(r.bytes(0x2c))
    struct.pack_into('<I', slot, 0, r.choice([OBJ, OBJ, OBJ, 0]))
    for at in (4, 8, 0xc, 0x14, 0x18, 0x1c):
        struct.pack_into('<I', slot, at, odd_float(r))
    struct.pack_into('<I', slot, 0x10, r.choice([0, r.randrange(-0x180, 0x181) & 0xffffffff]))
    head = r.choice([0, NODE])
    lst = bytearray(r.bytes(0x30))
    struct.pack_into('<4I', lst, 0, r.getrandbits(32), head, r.choice([NODE, 0]) if head else 0, r.randrange(0, 5))
    flag = bytearray(r.bytes(4))
    flag[0] |= 1
    pitch = bytearray(r.bytes(0xc20))
    for at in range(0, len(pitch), 4):
        struct.pack_into('<I', pitch, at, odd_float(r) if r.random() < 0.3 else struct.unpack_from('<I', pitch, at)[0])
    regions = {SLOT: slot, OUT: r.bytes(4), OBJ: struct.pack('<I', OVT) + r.bytes(0xc), OVT: vtable({}, size=64, com=COM),
               NODE: r.bytes(0xc), FLAG: flag, LIST: lst, IMPORT_TABLE: imports, LOG: log, HEAP: heap(0x40, fail=r.choice([0, 0, 0, 1]))}
    value = r.getrandbits(32)
    if name in ('init', 'isset'):
        return Call(address, [SLOT], regions, conv='t', ret=ret)
    if name == 'call7':
        return Call(address, [SLOT, value], regions, conv='t', ret=ret)
    if name == 'set':
        return Call(address, [SLOT, r.choice([NEW, 0, value])], regions, conv='t', ret=ret)
    if name == 'call5':
        return Call(address, [SLOT, OUT, value], regions, conv='t', ret=ret)
    if name == 'call59':
        return Call(address, [SLOT, OUT, value, r.getrandbits(32)], regions, conv='t', ret=ret)
    if name in ('restart', 'pitch'):
        regions[PITCH] = pitch
    if name in ('restart', 'free'):
        return Call(address, [SLOT, OUT], regions, conv='t', ret=ret)
    if name == 'guard':
        guard = bytearray(r.bytes(0xc))
        guard[4] = r.choice([0, 1, r.getrandbits(8)])
        strings(log, 10, 0, out=1)
        regions.update({SLOT: guard, SYSTEM: struct.pack('<I', SYSOBJ), SYSOBJ: struct.pack('<I', SVT) + r.bytes(0xc),
                        SVT: vtable({10: 2}, size=16)})
        return Call(address, [SLOT], regions, conv='t', ret=ret)
    if name in ('getpos', 'srcgetpos'):   # (slot 21's three outs are the caller's stack: logged as 0, each filled with any dword)
        strings(log, 21, 0, out=0b1110)
        for which in range(3):
            fill(log, 21, which + 1, r.bytes(4), which=which)
        regions[VEC] = r.bytes(0xc)
        if name == 'srcgetpos':
            index = r.choice([0, 0, 1, 2, r.getrandbits(32)])
            regions[SOURCE] = r.bytes(0x28) + struct.pack('<I', (SLOT - 0x2c * index) & 0xffffffff) + r.bytes(0x14)
            return Call(address, [SOURCE, OUT, index, VEC], regions, conv='t', ret=ret)
        return Call(address, [SLOT, OUT, VEC], regions, conv='t', ret=ret)
    if name in ('position', 'distances'):
        regions[VEC] = struct.pack('<3I', *[odd_float(r) for _ in range(3)])
        return Call(address, [SLOT, OUT, VEC], regions, conv='t', ret=ret)
    if name == 'pitch':
        steps = r.choice([0, r.randrange(-0x180, 0x181), r.randrange(-0x180, 0x181), r.randrange(-0x180, 0x181)])
        if r.random() < 0.3:   # (the step's own entry a NaN: the positive steps' fld / fstp quietens a signalling one)
            struct.pack_into('<I', regions[PITCH], 0x10 + 4 * steps if steps >= 0 else 0x610 + 4 * -steps, r.choice(NANS))
        return Call(address, [SLOT, OUT, steps & 0xffffffff], regions, conv='t', ret=ret)
    return Call(address, [SLOT, OUT, r.choice([r.randrange(0, 0x80), r.randrange(0, 0x80), 0x7f, r.getrandbits(32)])], regions, conv='t', ret=ret)


if __name__ == '__main__':
    B.main(case)
