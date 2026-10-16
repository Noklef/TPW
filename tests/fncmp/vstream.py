# The stream buffer reads and two virtual call sequences (src/game/core/vstream) against the originals: the get area at the start, inside and at the end of its
# count, a null position, the refill answering as it likes (see tools/fncmp.py set_return()), tables of character classes, all through recorders for the virtual slots.
# usage: vstream.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, LOG, RAM, log_region, put, set_return, vtable  # noqa: E402

# name: (address, convention, returns, size)
CALLS = {'WP': (0x4f1eb0, 't', 'a', 0x30), 'BP': (0x5d0050, 't', 'a', 0x30), 'WN': (0x4f06c0, 't', 'a', 0x50), 'BN': (0x5ceab0, 't', 'a', 0x50),
         'Is': (0x4f18d0, 't', 'a', 0x30), 'Ref': (0x497610, 't', 'v', 0x30), 'SI': (0x4f1950, 't', 'a', 0x40), 'SN': (0x4f1990, 't', 'a', 0x40),
         'Map': (0x4f1900, 't', 'a', 0x50), 'WPC': (0x4f0e30, 't', 'a', 0x30), 'BPC': (0x5cf1c0, 't', 'a', 0x40)}
B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()])
OBJ, MEM, GP, CNT, BUF, VT, VTM, TBL = RAM, RAM + 0x200, RAM + 0x300, RAM + 0x340, RAM + 0x400, RAM + 0x1000, RAM + 0x1100, RAM + 0x2000


def case(r):
    name = r.choice(OPS)
    address, conv, ret, _n = CALLS[name]
    obj = bytearray(r.bytes(0x80))
    log = log_region()
    regions = {OBJ: obj, LOG: log}
    put(obj, 0, 'I', VT)
    if name in ('SI', 'SN', 'Map'):
        n = r.choice([0, 1, 2, 3, 5])
        regions[BUF] = bytearray(r.choice([0x41, 0x42, 0x20, 0x7f, 0x100, 0xffff, r.getrandbits(16)]).to_bytes(2, 'little')[k % 2] for k in range(0x40))
        regions[BUF + 0x100] = r.bytes(0x40)
        put(obj, 0x10, 'I', TBL + 0x100)
        regions[TBL] = r.bytes(0x200)
        regions[VT] = vtable({2: 2, 12: 2})
        if r.random() < 0.5:
            set_return(log, 2, r.choice([0, 1, 0xff00, 0x100, 0xff]))
        if name == 'Map':
            return Call(address, [OBJ, BUF, BUF + 2 * n, BUF + 0x100], regions, conv=conv, ret=ret)
        return Call(address, [OBJ, r.choice([0, 1, 0x20, 0xffff]), BUF, BUF + 2 * n], regions, conv=conv, ret=ret)
    if name in ('WPC', 'BPC'):
        put(obj, 0x1c, 'I', GP)
        put(obj, 0x2c, 'I', CNT)
        regions[GP] = struct.pack('<I', r.choice([BUF, BUF + 2, 0]))
        regions[CNT] = struct.pack('<I', r.choice([0, 1, 2, 5]))
        regions[BUF] = r.bytes(0x40)
        regions[VT] = vtable({5: 0, 2: 1})
        return Call(address, [OBJ], regions, conv=conv, ret=ret)
    if name in ('WP', 'BP', 'WN', 'BN'):
        wide = name[0] == 'W'
        w = 2 if wide else 1
        put(obj, 0x1c, 'I', GP)
        put(obj, 0x2c, 'I', CNT)
        n = r.choice([0, 1, 2, 5])
        pos = r.choice([BUF, BUF + w, BUF + 0x10000 * 0, 0, 0x12345678 if False else BUF + 2 * w])
        regions[GP] = struct.pack('<I', pos)
        regions[CNT] = struct.pack('<I', n)
        regions[BUF] = r.bytes(0x40)
        regions[VT] = vtable({4: 0})
        if name in ('WN', 'BN'):
            if r.random() < 0.4:
                set_return(log, 4, r.choice([0xffff, 0xffffffff, 0x1234ffff, 0x41, 0]))
        elif r.random() < 0.3:
            set_return(log, 4, r.choice([0xffff, 0x41, 0xffffffff]))
        return Call(address, [OBJ], regions, conv=conv, ret=ret)
    if name == 'Is':
        put(obj, 0x10, 'I', TBL + 0x100)
        regions[TBL] = r.bytes(0x200)
        regions[VT] = vtable({12: 2})
        return Call(address, [OBJ, r.choice([0, 1, 0x80, 0x8000, 0xffff, 0x1ffff, r.getrandbits(32)]), r.choice([0, 0x41, 0xff, r.getrandbits(32)])], regions, conv=conv, ret=ret)
    put(obj, 0x20, 'I', MEM)
    regions[MEM] = struct.pack('<I', VTM) + r.bytes(0x20)
    regions[VT] = vtable({13: 0, 6: 0, 5: 0})
    regions[VTM] = vtable({9: 1})
    return Call(address, [OBJ], regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
