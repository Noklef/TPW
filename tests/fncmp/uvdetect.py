# The texture coordinate offset's choice (src/game/render/uvprobe: 0x583cc0) against the original: each card by its ids and ones near them, the driver kind, [0x8bcbc8]'s bits, the TNT's caps call failing or not and their flag; the probes and the log recorders.
# usage: uvdetect.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fill, log_region, put, set_return, strings, vtable  # noqa: E402

TEXT, ERRTEXT, LOGGED, ACROSS, DOWN, CAPS = 1, 2, 4, 28, 29, 3
B = Bench([(0x583cc0, 0x583e90)], hooks={0x5645e0: _recorder(TEXT, 1, cdecl=True), 0x56a770: _recorder(ERRTEXT, 1, cdecl=True), 0x564650: _recorder(LOGGED, 1, cdecl=True), 0x584310: _recorder(ACROSS, 0), 0x583e90: _recorder(DOWN, 0)})
CARDS = [(0x104c, 0x3d07), (0x104c, 0x3d07), (0x10de, 0x20), (0x10de, 0x20), (0x10de, 0x20), (0x1002, 0x4c42), (0x1002, 0x524c), (0x1234, 0x20), (0x104c, 0x20), (0x10de, 0x3d07), (0x1002, 0x20), (0x1002, 0x4c43), (0x1234, 0x5678), (0x1234, 0x5678)]
SETTINGS, DIRECT, HOLDER, SURFACE, VT = RAM, RAM + 0x5000, RAM + 0x5500, RAM + 0x5600, RAM + 0x5700


def case(r):
    log = log_region()
    strings(log, TEXT, 1)
    blob(log, CAPS, 1, 1)
    strings(log, CAPS, 0, 6)
    set_return(log, CAPS, r.choice([0, 0, 1, -1, 0x80000000]))
    set_return(log, ERRTEXT, RAM + 0x5800)
    fill(log, CAPS, 1, struct.pack('<I', r.choice([0, 0x10, r.getrandbits(32) | 0x10, r.getrandbits(32) & ~0x10])), offset=0xf4)
    vendor, device = r.deal(CARDS)
    flags = r.getrandbits(32) & ~0x8000002 | r.choice([0, 0, 0, 2, 0x8000000])
    regions = {LOG: log, SETTINGS: r.bytes(0x40f0), DIRECT: put(bytearray(r.bytes(0x430)), 0x428, 'II', vendor, device), HOLDER: put(bytearray(r.bytes(0x10)), 8, 'I', SURFACE), SURFACE: struct.pack('<I', VT), VT: vtable({}, 8, com={CAPS: 3}), RAM + 0x5800: b'text\0',
               0x879a24: struct.pack('<I', DIRECT), 0x879da8: struct.pack('<I', HOLDER), 0x8bcbc8: struct.pack('<I', flags), 0x8bd508: struct.pack('<I', r.getrandbits(32)), 0x8bd524: struct.pack('<I', r.choice([1, 1, 1, 1, 1, 4, 0]))}
    return Call(0x583cc0, [SETTINGS], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
