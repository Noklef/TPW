# The research panel's handler (src/game/ui/levelmeter: 0x4a96b0) against the original: every call a recorder numbered as ResearchPanelEnv from 10, Window_FindChild answering objects in turn whose slot 7 is recorder 30.
# usage: researchpanel.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region, put, recorder, set_return, vtable  # noqa: E402

B = Bench([(0x4a96b0, 0x4a9bb8)],
          hooks={0x658d9f: _recorder(10, 4, cdecl=True), 0x4a9bd0: _recorder(11, 1, cdecl=True), 0x4a9c70: _recorder(12, 1, cdecl=True), 0x65ef90: _recorder(13, 2), 0x65f024: _recorder(14, 1), 0x485b00: _recorder(15, 1, cdecl=True),
                 0x485d20: _recorder(16, 3, cdecl=True), 0x51c2c0: _recorder(17, 1, cdecl=True), 0x486b40: _recorder(18, 1, cdecl=True), 0x519510: _recorder(19, 0), 0x4c9e60: _recorder(20, 0, cdecl=True), 0x519550: _recorder(21, 0),
                 0x65d7d0: _recorder(22, 1), 0x503870: _recorder(23, 2), 0x503900: _recorder(24, 1), 0x65d3a3: _recorder(25, 1), 0x51bc40: _recorder(26, 3, cdecl=True), 0x51c300: _recorder(27, 1, cdecl=True),
                 0x485aa0: _recorder(28, 1, cdecl=True), 0x488ba0: _recorder(29, 4, cdecl=True)})
WINDOW, CHILDREN, SUMS, VT, OBJECTS, LAB = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x1000, RAM + 0x2000
SLIDERS = [0xd2d0b, 0xd2d0c, 0xd2d0d, 0xd2d0e, 0xd2d0f, 0xd2d10]


def case(r):
    log = log_region()
    regions = {LOG: log}
    regions[VT] = put(vtable({}, size=0x10), 0x1c, 'I', recorder(30, 1))
    regions[OBJECTS] = struct.pack('<I', VT) * 40 * 4
    regions[CHILDREN] = struct.pack('<40I', *[OBJECTS + 0x10 * i for i in range(40)])
    answers(log, 22, CHILDREN)
    now = r.choice([0, 25, 50, 100, r.randrange(200), r.getrandbits(32)])
    regions[0x7cc1c8] = struct.pack('<I', r.choice([now, r.randrange(200), r.getrandbits(32)]))
    regions[SUMS] = struct.pack('<2I', now, r.choice([now, r.randrange(200), r.getrandbits(32)])) + bytes(152)
    answers(log, 20, SUMS)
    regions[LAB + 0x1460] = struct.pack('<I', r.choice([0x46, 100, 0, r.randrange(200), r.getrandbits(32)]))
    set_return(log, 21, LAB)
    set_return(log, 19, r.getrandbits(32))
    set_return(log, 13, r.choice([0, 0xffff0000, 1, r.getrandbits(32)]))
    for k in (15, 28, 29):
        set_return(log, k, r.getrandbits(32))
    regions[0x7cc1b4] = struct.pack('<I', r.choice([1, 2, 10, 100, 1024, r.getrandbits(32) | 1]))
    regions[0x7cc1bc] = bytes([r.choice([0, 0, 1, 0xff, r.getrandbits(8)])])
    limit = r.choice([1, 100, 512, 1024, r.randrange(1, 4000), -r.randrange(1, 4000) & 0xffffffff])
    regions[0x7cc1c0] = struct.pack('<I', limit)
    regions[0x7cc1cc] = r.bytes(4)
    regions[0x7cc1d0] = r.bytes(4)
    regions[0x7cc1d4] = struct.pack('<I', r.choice([0, r.getrandbits(32)]))
    regions[0x7cc1d8] = r.bytes(4)
    regions[0x7cf83c] = r.bytes(4)
    message = r.deal([5, 0x10, 0x14, 0x15, 0x1e, 0x1e, 0x1e, 0x100, 0x100, 0x800, 0x800, 0x800, 0x800, 0x800, 0x800, 0x801, 0x801, 0x802, 0x802, 0x13, 0x101, 0x803, r.getrandbits(32)])
    a = r.choice(SLIDERS + [0xd2d10, 0xd2d10, 0xd2d0a, 0xd2d11, 0xffffffff, 0xfffffffe, 0xfffffffd, 0, r.getrandbits(32)])
    b = r.choice([0, 1, 512, limit, limit + 1 & 0xffffffff, r.randrange(4000), -r.randrange(4000) & 0xffffffff, r.getrandbits(32) >> 8])
    return Call(0x4a96b0, [WINDOW, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
