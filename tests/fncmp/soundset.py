# The sound settings' setters and flag, and the sound commands' base constructor
# (src/game/core/soundset: 0x51bb60-0x51bc20, 0x51bc40 / 0x51c270 (the manager's engine's slots 3 / 5 recorders, the position's floats hashed), 0x51e6f0, 0x51e710, 0x51c1c0, 0x51c6f0, 0x51bb40)
# against the originals. Place (0x51c1d0): soundchannel.py's placement surroundings (its voices_case() for the manager's forward 0x6b5ba0),
# the manager [0x802bcc] its handle, on / started as manager()'s; the nine dwords its three vectors (the object's own mostly, the channel's
# scale 1.0, one changed now and then), the level now and then a signalling NaN, the u16 0x803ad2 (fixed level) set now and then.
# usage: soundset.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Setters,Place: Setters the old cases)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, RAM, blob, fill, log_region, strings, vtable  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import soundchannel  # noqa: E402

# (soundchannel.py's: the placement's forward, the object's and the channel's setters)
PLACE_RANGES = [
    (0x6b8b40, 0x6b8b60), (0x6b7d70, 0x6b7e5b), (0x6c4760, 0x6c47a3), (0x6b5e40, 0x6b5e59), (0x6b7910, 0x6b796b), (0x6b7970, 0x6b79cb),
    (0x6c4670, 0x6c46a7), (0x6c46b0, 0x6c475a), (0x6b7b60, 0x6b7d6e), (0x6b79d0, 0x6b7a6b), (0x6b5e60, 0x6b5fb4), (0x6c4c80, 0x6c4eff),
    (0x6c4f00, 0x6c4f54), (0x6b8330, 0x6b8358), (0x6bb7a0, 0x6bb7bd), (0x6bb7c0, 0x6bb856), (0x6babc0, 0x6bac04), (0x6bac30, 0x6bac5c),
    (0x6b7e60, 0x6b7ea5), (0x6c5080, 0x6c5088), (0x6bb240, 0x6bb265), (0x6baa10, 0x6baa20), (0x6d1bc0, 0x6d1bca), (0x6b90d0, 0x6b9154),
    (0x6b9930, 0x6b99ba), (0x6b84f0, 0x6b8569), (0x6b57b0, 0x6b5801), (0x6b5720, 0x6b5739), (0x6b5810, 0x6b5834), (0x6b9c50, 0x6b9c6a),
    (0x6b9c70, 0x6b9c82), (0x6b9c90, 0x6b9ca9), (0x6b9cf0, 0x6b9d02), (0x6b9d30, 0x6b9d43), (0x6b6140, 0x6b6163), (0x6b8b60, 0x6b8c63),
    (0x6b7fa0, 0x6b814e), (0x6b7a70, 0x6b7b5f), (0x6b9a60, 0x6b9aa1), (0x6b9ab0, 0x6b9b26), (0x6b8180, 0x6b822d), (0x6b99c0, 0x6b9a05),
    (0x6b9a10, 0x6b9a5b), (0x6b8230, 0x6b82b1), (0x6c49b0, 0x6c4b93), (0x6c4fd0, 0x6c4fe3), (0x5e2a30, 0x5e2a80), (0x5e2a80, 0x5e2b80),
    (0x67acb0, 0x67acb1), (0x6b82c0, 0x6b832e), (0x6c4ba0, 0x6c4c1e), (0x6b9160, 0x6b91ce), (0x6b9660, 0x6b9760), (0x6bead0, 0x6beb03),
    (0x632150, 0x632380), (0x6b9b30, 0x6b9b94), (0x6b8480, 0x6b84ec), (0x6b8c70, 0x6b8cda), (0x6b8f60, 0x6b8f90), (0x6b5fc0, 0x6b603d),
    (0x6bb860, 0x6bb8c9), (0x6b5ba0, 0x6b5bc3)]


B = Bench([(0x51bb40, 0x51bb52), (0x51bb60, 0x51bc34), (0x51e6f0, 0x51e704), (0x51e710, 0x51e726),
           (0x51c1c0, 0x51c1cc), (0x51c6f0, 0x51c6f6), (0x51b4f0, 0x51b530), (0x51b7c0, 0x51b810), (0x5da3c0, 0x5da3c1),
           (0x51bc40, 0x51bc70), (0x6b5b80, 0x6b5b9a), (0x51c270, 0x51c2bc), (0x6b5b60, 0x6b5b75)] + PLACE_RANGES + [(0x51c1d0, 0x51c266)])
MANAGER, VT, ENGINE, EVT = RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400


def manager(r):
    """The sound globals (on 0x802bc8, the manager 0x802bcc, started 0x802bd4, the allocation count 0x802bd8) and a manager
    whose vtable is recorders: +0 (&result), +4 (&a, a, b, &b), the pointers logged as 0; +4 now and then writes b."""
    g = bytearray(r.bytes(0x20))
    g[8:12] = r.choice([0, 1, 1, 1, r.getrandbits(32)]).to_bytes(4, 'little')
    g[0xc:0x10] = MANAGER.to_bytes(4, 'little')
    g[0x14:0x18] = r.choice([0, 1, 1, 1, r.getrandbits(32)]).to_bytes(4, 'little')
    g[0x18:0x1c] = r.choice([0, 0, 1, r.getrandbits(32)]).to_bytes(4, 'little')
    log = log_region()
    strings(log, 0, 0, 1)
    strings(log, 1, 0, 9)
    if r.random() < 0.6:
        fill(log, 1, 3, r.bytes(4))
    return {0x802bc0: g, MANAGER: VT.to_bytes(4, 'little') + r.bytes(0xc), VT: vtable({0: 1, 1: 4}, 4), LOG: log}


def place_case(r):
    """Place (0x51c1d0): see the top"""
    c = soundchannel.voices_case(r, log_region(), 'mgrplace')
    regions = c.regions
    g = manager(r)
    regions[0x802bc0] = bytearray(g[0x802bc0])
    regions[0x802bc0][0xc:0x10] = soundchannel.MGRH.to_bytes(4, 'little')
    regions[0x803ad0] = struct.pack('<HH', r.getrandbits(16), r.choice([0] * 8 + [1, 0x8000]))
    words = list(struct.unpack('<3I', regions[soundchannel.P1]) + struct.unpack('<3I', regions[soundchannel.P2]) +
                 struct.unpack('<3I', regions[soundchannel.P3]))
    if r.random() < 0.1:
        words[1] = r.choice([0x7f800001, 0xff812345, 0x7fbfffff])
    return Call(0x51c1d0, words, regions, ret='v')


def case(r):
    if r.deal(OPS) == 'Place':
        return place_case(r)
    value = r.choice([0, 1, 100, 0xffff, 0x10000, r.getrandbits(32)])
    fn = r.choice([0x51bb60, 0x51bb80, 0x51bba0, 0x51bbc0, 0x51bbe0, 0x51bc00, 0x51bc20, 0x51e6f0, 0x51e710,
                   0x51c1c0, 0x51c6f0, 0x51bb40, 0x51b4f0, 0x51b4f0, 0x51b7c0, 0x51b7c0, 0x51bc40, 0x51bc40, 0x51c270, 0x51c270])
    if fn == 0x51b4f0:
        return Call(fn, [value, r.getrandbits(32)], manager(r), ret='a')
    if fn == 0x51bc40:   # (the manager's engine +4: its slot 3 a recorder of three)
        regions = manager(r)
        regions[MANAGER] = VT.to_bytes(4, 'little') + ENGINE.to_bytes(4, 'little') + r.bytes(8)
        regions[ENGINE] = EVT.to_bytes(4, 'little') + r.bytes(0x30)
        regions[EVT] = vtable({3: 3})
        return Call(fn, [r.getrandbits(32), r.choice([2, 4, 0xc, 0xd, r.getrandbits(32)]), value], regions, ret='a')
    if fn == 0x51c270:   # (the engine's slot 5 a recorder of two, the position (three floats on the stack) hashed)
        regions = manager(r)
        regions[MANAGER] = VT.to_bytes(4, 'little') + ENGINE.to_bytes(4, 'little') + r.bytes(8)
        regions[ENGINE] = EVT.to_bytes(4, 'little') + r.bytes(0x30)
        regions[EVT] = vtable({5: 2})
        blob(regions[LOG], 5, 1, 7)
        xyz = [r.choice([0, 1, -1, r.randrange(-0x8000, 0x8000), 0x1000001, -0x7fffffff, r.getrandbits(32)]) & 0xffffffff for _ in range(3)]
        return Call(fn, [r.getrandbits(32)] + xyz, regions, ret='a')
    if fn == 0x51b7c0:
        return Call(fn, [], manager(r))
    if fn == 0x51c6f0:
        return Call(fn, [], {0x802bc0: r.bytes(0x10)}, ret='a')
    if fn == 0x51bb40:
        return Call(fn, [RAM + 4, value], {RAM: r.bytes(0x10)}, conv='t', ret='a')
    return Call(fn, [value], {0x803ab0: r.bytes(0x40)})


OPS = [k for k in os.environ.get('OPS', 'Setters,Place').split(',') if k]

if __name__ == '__main__':
    B.main(case)
