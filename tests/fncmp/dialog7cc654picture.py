# The picture in the online dialog drawn (src/game/ui/dialog7cc654picture: 0x4bc270) against the original. Recorders (their own benches have them), the stack places logged as 0 and the methods on locals
# without their this. Array3D_Init is a stand-in as driver.cpp's dialog_picture_array: it sets the array's width, height and pixels.
# usage: dialog7cc654picture.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fill, log_region, mask_this, put, set_return, strings  # noqa: E402

CTOR, FIND, LOCK, INFO, ASK, LOAD, SHRINK, UNLOCK, FREE = range(10, 19)
FROM, TO, OTHER = RAM + 0x1000, RAM + 0x3000, RAM + 0x5000


def array(e):  # thiscall (width, height, depth), pops 12: as driver.cpp's dialog_picture_array
    sp = e.r['ESP'] + (4 if e.tail else 0)
    width, height = e.u32(sp), e.u32(sp + 4)
    e.write(e.r['ECX'], struct.pack('<I', width))
    e.write(e.r['ECX'] + 4, struct.pack('<I', height))
    e.write(e.r['ECX'] + 0xc, struct.pack('<I', TO))
    if e.tail:
        ret = e.u32(e.r['ESP'])
        e.r['ESP'] += 12
        e.write(e.r['ESP'], struct.pack('<I', ret))
    else:
        e.r['ESP'] += 12


B = Bench([(0x4bc270, 0x4bc41c)], hooks={0x5fa680: _recorder(CTOR, 0), 0x65d7d0: _recorder(FIND, 1), 0x65d6db: _recorder(LOCK, 2), 0x4925d0: _recorder(INFO, 4, cdecl=True), 0x5fa8a0: _recorder(ASK, 4), 0x5faa00: _recorder(LOAD, 5),
                                         0x5d3830: array, 0x5d4260: _recorder(SHRINK, 4), 0x65d745: _recorder(UNLOCK, 0), 0x5d3860: _recorder(FREE, 0)})


def case(r):
    log = log_region()
    for k in (CTOR, ASK, LOAD, SHRINK, FREE):
        mask_this(log, k)
    strings(log, LOCK, 0, 3)
    strings(log, INFO, 0, 2)
    strings(log, ASK, 0, 1)
    strings(log, LOAD, 0, 1)
    fill(log, LOCK, 0, struct.pack('<I', r.getrandbits(16)))
    fill(log, LOCK, 1, struct.pack('<I', r.getrandbits(16)), which=1)
    set_return(log, FIND, r.getrandbits(32))
    set_return(log, LOCK, r.deal([0, r.getrandbits(32) | 1, r.getrandbits(32) | 1, r.getrandbits(32) | 1]))
    width, height = r.deal([0, 1, 2, 5, 8]), r.deal([0, 1, 2, 3, 8])
    regions = {LOG: log, FROM: r.bytes(width * height * 4 + 8), TO: b'\xcd' * (width * height * 4 + 8), OTHER: put(bytearray(0x40), 0x30, 'I', r.deal([0, r.getrandbits(32) | 1])), 0x7cc654: r.bytes(4), 0x7cc660: struct.pack('<I', OTHER),
               0x7cc640: struct.pack('<III', FROM, width, height), 0x8bd554: struct.pack('<I', r.deal([0, 1, 2, 2, 3, 3, 4]))}
    return Call(0x4bc270, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
