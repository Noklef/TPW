# ios_base::Init's two constructors (src/game/core/iosbase: 0x6921d0, 0x692c60) against the originals: the counter below 0 (the standard streams built in their fixed .bss places, the FILEs
# 0x77e800.. only pointed at and their fields kept), or not (only the count). The locale 0x699da0 is a recorder; _Mbrtowc a recorder that fills the wide fill character.
# usage: iosinit.py [CASES [SEED]] [--wild|--both] [--show N]
import struct
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, fill, heap, log_region, set_return, strings, vtable  # noqa: E402

LOCALE, MBRTOWC = 15, 23
B = Bench([(0x6921d0, 0x692517), (0x692c60, 0x692f9b), (0x692520, 0x692565), (0x692fa0, 0x692ff1), (0x692570, 0x69264a), (0x693000, 0x6930da), (0x693340, 0x6934c0), (0x693830, 0x693a90), (0x693dc0, 0x693e20),
           (0x5cfd80, 0x5cfd90), (0x4ef830, 0x4ef840), (0x4f1bd0, 0x4f1be0), (0x5d0a90, 0x5d0b72), (0x4f5990, 0x4f5a4f), (0x4f5a50, 0x4f5acc), (0x4ef7b0, 0x4ef800), (0x4f6020, 0x4f6080), (0x5d06c0, 0x5d08e0),
           (0x5d0e00, 0x5d0e70), (0x5cefe0, 0x5cf1c0), (0x45b340, 0x45b9a0)],
          hooks={0x699da0: _recorder(LOCALE, 0, cdecl=True), 0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None, 0x693bc0: _recorder(MBRTOWC, 5, cdecl=True)})
OBJ, LOC, VT = RAM, RAM + 0x100, RAM + 0x200


def case(r):
    log = log_region()
    set_return(log, LOCALE, LOC)
    strings(log, MBRTOWC, 0, out=0xb)
    fill(log, MBRTOWC, 0, r.bytes(4))
    set_return(log, MBRTOWC, r.choice([1, 1, 1, 0, 0xffffffff]))
    regions = {LOG: log, OBJ: r.bytes(4), LOC: struct.pack('<II', VT, r.choice([0x100, 0x1000, 0xffffffff])) + r.bytes(8), VT: vtable({0: 1}), 0xfb1dd0: struct.pack('<I', LOC), HEAP: heap(0x200),
               0xfb1970: r.bytes(0x428), 0x77e800: r.bytes(0x60), 0xfb2760: r.bytes(4), 0xfb34b4: r.bytes(4)}
    address, counter = r.deal([(0x6921d0, 0x7822b0), (0x692c60, 0x78240c)])
    regions[counter] = struct.pack('<i', r.choice([-1, -1, -1, -1, -5, 0, 1, 7, -0x80000000, 0x7fffffff]))
    return Call(address, [OBJ], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
