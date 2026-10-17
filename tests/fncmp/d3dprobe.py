# The Direct3D driver probes (src/game/render/d3dprobe) against the originals: a probe whose virtual base (at a random offset from its base table) has a
# vtable of recorders (slot 2 flags, 3 clear, 0x10 the read back: its pixel out logged as 0 and filled, 0x11 draw) and a device of COM recorders
# (SetRenderState 22, SetTextureStageState 40); the caps bits at [0x879da8] + 0xa4 / + 0xa8, the render caps 0x8bcbc8 and the pixel mask 0x8bd4dc random,
# the pixel 0, the mask, or random bits; the read back fails now and then.
# Gap: one fill answers both of the fog test's reads, so its second read never fails nor differs from the first.
# usage: d3dprobe.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, fill, log_region, put, set_return, strings, vtable  # noqa: E402

CALLS = {'AlphaTest': (0x567270, 0x5673c5), 'FogTest': (0x5673d0, 0x5675b4)}
B = Bench(list(CALLS.values()))
PROBE, VBT, VT, DEV, DVT, CAPS = RAM, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500, RAM + 0x600


def case(r):
    name = r.deal(OPS)
    probe = bytearray(r.bytes(0x100))
    off = r.choice([0x30, 0x40, 0x80])
    put(probe, 0, 'I', VBT)
    put(probe, off, 'I', VT)
    put(probe, 0x24, 'I', DEV)
    log = log_region()
    strings(log, 16, 0, out=0b100)
    mask = r.choice([0xffffff, 0xffff, 0xf800, r.getrandbits(32)])
    fill(log, 16, 2, struct.pack('<I', r.choice([0, mask, r.getrandbits(32), r.getrandbits(32) & mask, mask | 0x1000000])))
    if r.random() < 0.25:
        set_return(log, 16, 0)
    caps = bytearray(r.bytes(0x100))
    if r.random() < 0.7:
        put(caps, 0xa4, 'B', caps[0xa4] | 0x40)
        put(caps, 0xa8, 'I', struct.unpack_from('<I', caps, 0xa8)[0] | 0x80000)
    regions = {PROBE: probe, VBT: bytearray(struct.pack('<II', r.getrandbits(32), off)), VT: vtable({2: 1, 3: 2, 16: 3, 17: 7}, size=24),
               DEV: bytearray(struct.pack('<I', DVT) + r.bytes(0x1c)), DVT: vtable({}, size=48, com={22: 3, 40: 4}), LOG: log,
               CAPS: caps, 0x879da8: bytearray(struct.pack('<I', CAPS)), 0x8bcbc8: r.bytes(4), 0x8bd4dc: bytearray(struct.pack('<I', mask))}
    return Call(CALLS[name][0], [PROBE], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
