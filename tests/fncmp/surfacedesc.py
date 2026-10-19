# A surface's description into its info, and the pixel channel shifts (src/game/gfx/surface: 0x61b090-0x61b1d2, thiscall) against the originals:
# channel masks contiguous, odd or empty against any top bit; DDSURFACEDESCs of 8 bits (paletted) or more; Refresh's GetSurfaceDesc (COM slot 22 on
# the surface) a recorder that fills the description (its bits now and then null, so the info's are kept) or fails.
# Gap: the fill overwrites the size Refresh sets in the description before the recorder logs it.
# usage: surfacedesc.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Channel,Shifts,SetInfo,Refresh)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, RAM, blob, fill, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x61b090, 0x61b1d3)])
OBJ, ARG, DESC, SURF, VTS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400
GET_DESC = 22
CALLS = {'Channel': 0x61b090, 'Shifts': 0x61b0e0, 'SetInfo': 0x61b130, 'Refresh': 0x61b190}


def mask(r):
    return r.choice([0, 0xff, 0xff00, 0xff0000, 0xff000000, 0xf800, 0x7e0, 0x1f, 0x7c00, 0x3e0, 0x8000, 0x3ff00000, 1, 0x80000000, r.getrandbits(32)])


def desc(r):
    d = bytearray(r.bytes(0x6c))
    put(d, 0x54, 'I', r.choice([8, 8, 16, 24, 32, r.getrandbits(32)]))
    put(d, 0x58, '4I', mask(r), mask(r), mask(r), mask(r))
    if r.random() < 0.3:
        put(d, 0x24, 'I', 0)
    return d


def case(r):
    op = r.deal(OPS)
    obj = bytearray(r.bytes(0x100))
    regions = {OBJ: obj}
    address = CALLS[op]
    if op == 'Channel':
        return Call(address, [OBJ, mask(r), r.choice([0x1f, 0x17, 0xf, 7, 0, 32, r.getrandbits(32)])], regions, conv='t')
    if op == 'Shifts':
        regions[ARG] = struct.pack('<4I', mask(r), mask(r), mask(r), mask(r))
        return Call(address, [OBJ, ARG], regions, conv='t')
    if op == 'SetInfo':
        regions[DESC] = desc(r)
        return Call(address, [OBJ, DESC], regions, conv='t')
    log = log_region()
    blob(log, GET_DESC, 1, 1)
    fill(log, GET_DESC, 1, desc(r))
    if r.random() < 0.15:
        set_return(log, GET_DESC, r.choice([0x80000000, 0x887601c2, r.getrandbits(32)]))
    put(obj, 0x24, 'I', SURF)
    regions.update({SURF: struct.pack('<I', VTS) + r.bytes(0xc), VTS: vtable({}, com={GET_DESC: 2}), LOG: log})
    return Call(address, [OBJ], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
