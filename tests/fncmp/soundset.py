# The sound settings' setters and flag, and the sound commands' base constructor
# (src/game/core/soundset: 0x51bb60-0x51bc20, 0x51bc40 / 0x51c270 (the manager's engine's slots 3 / 5 recorders, the position's floats hashed), 0x51e6f0, 0x51e710, 0x51c1c0, 0x51c6f0, 0x51bb40)
# against the originals.
# usage: soundset.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, RAM, blob, blob_bytes, fill, log_region, strings, vtable  # noqa: E402

B = Bench([(0x51bb40, 0x51bb52), (0x51bb60, 0x51bc34), (0x51e6f0, 0x51e704), (0x51e710, 0x51e726),
           (0x51c1c0, 0x51c1cc), (0x51b4f0, 0x51b530), (0x51b7c0, 0x51b810), (0x5da3c0, 0x5da3c1),
           (0x51bc40, 0x51bc70), (0x6b5b80, 0x6b5b9a), (0x51c270, 0x51c2bc), (0x6b5b60, 0x6b5b75),
           (0x51c0a0, 0x51c140), (0x51c2c0, 0x51c360), (0x51c640, 0x51c700), (0x5e2a30, 0x5e2a90), (0x6b58b0, 0x6b5900), (0x6b5930, 0x6b5960),
           (0x6b5bd0, 0x6b5c00), (0x6b5c70, 0x6b5ca0), (0x6b5cb0, 0x6b5d10), (0x6b6510, 0x6b6560), (0x6b65a0, 0x6b65f0), (0x6b6830, 0x6b68e0),
           (0x6b6b40, 0x6b6bb0), (0x6b7b60, 0x6b7d80), (0x6b8230, 0x6b8370), (0x6b8930, 0x6b89a0), (0x6b8a70, 0x6b8ae0), (0x6b8b40, 0x6b8b70),
           (0x6b8c70, 0x6b8cf0), (0x6b9a10, 0x6b9a70), (0x6bb7a0, 0x6bb7d0), (0x6bc900, 0x6bca20), (0x6c49b0, 0x6c4c30), (0x6c4c80, 0x6c4f70),
           (0x6c4fd0, 0x6c5000), (0x6c5130, 0x6c5160)])
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


SMGR = RAM + 0x8000   # (the senders' manager: +4 the sound object)
SENDERS = [0x51c0a0, 0x51c0e0, 0x51c2c0, 0x51c300, 0x51c640, 0x51c6b0]


def sender_case(r, fn):
    """The sound commands' senders: the sound globals (on and started, either 0 now and then; the manager at SMGR, its +4 the sound
    object); 0x51c0a0 / 0x51c0e0 / 0x51c2c0 / 0x51c300 the soundobject bench's object (a mixer with voices, ids of theirs or not; for
    0x51c0e0 the sound system [0xfb1f24] as the soundcmd bench gives it; the object's own slot 6 its out hidden); 0x51c640 / 0x51c6b0
    the soundchannel bench's occluder add / remove (cmdoccadd's values and cmdoccrem's node)."""
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    log = log_region()
    if fn in (0x51c640, 0x51c6b0):
        import soundchannel as sc
        regions = sc.voices_case(r, log, 'cmdoccadd' if fn == 0x51c640 else 'cmdoccrem').regions
        target = sc.OBJECT
    else:
        import soundobject as so
        regions = {}
        ids = so.object_regions(r, log, regions, slot6=fn in (0x51c0a0, 0x51c0e0), voices=True)
        target = so.OBJ
        pick = r.choice(ids * 3 + [0, 1, 100, r.getrandbits(32)])
        if fn in (0x51c2c0, 0x51c300):
            strings(log, 6, 0, out=1)   # (the object's own slot 6: the out the argument's stack slot)
        if fn == 0x51c0e0:
            import soundcmd as sd
            blob_bytes(log, 4, 1, 0xff, inner=(8, 0x7000, 28))
            regions.update({sd.SYSTEM: struct.pack('<I', sd.SYS), sd.SYS: struct.pack('<I', sd.SVT), sd.SVT: vtable({4: 2})})
    g = bytearray(r.bytes(0x20))
    struct.pack_into('<II', g, 8, r.choice([1] * 8 + [0, r.getrandbits(32)]), SMGR)
    struct.pack_into('<I', g, 0x14, r.choice([1] * 8 + [0, r.getrandbits(32)]))
    regions.update({0x802bc0: g, SMGR: struct.pack('<II', r.getrandbits(32), target) + r.bytes(8)})
    if fn == 0x51c640:
        import soundchannel as sc
        _, x, y, z, radius, _ = struct.unpack('<6I', regions[sc.COMMAND])
        return Call(fn, [x, r.getrandbits(32), y, radius, z], regions, ret='a')
    if fn == 0x51c6b0:
        return Call(fn, [struct.unpack('<I', regions[sc.PNODE])[0]], regions)
    if fn == 0x51c0e0:
        return Call(fn, [pick, r.getrandbits(32)], regions)
    return Call(fn, [pick if fn != 0x51c2c0 else r.getrandbits(32)], regions)


def case(r):
    if r.random() < 0.4:
        return sender_case(r, r.deal(SENDERS))
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


if __name__ == '__main__':
    B.main(case)
