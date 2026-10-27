# The sound settings' setters and flag, and the sound commands' base constructor
# (src/game/core/soundset: 0x51bb60-0x51bc20, 0x51bc40 / 0x51c270 (the manager's engine's slots 3 / 5 recorders, the position's floats hashed), 0x51e6f0, 0x51e710, 0x51c1c0, 0x51c6f0, 0x51bb40)
# against the originals. The commands' senders (0x51c0a0-0x51c6b0, 0x51bc70-0x51bd30) and the settings update (0x51bd70): see
# sender_case() and update_case().
# usage: soundset.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, Bench, Call, RAM, blob, blob_bytes, fill, import_table, log_region, recorder, set_return, strings, vtable  # noqa: E402

B = Bench([(0x51b4f0, 0x51b530), (0x51b7c0, 0x51b810), (0x51bb40, 0x51bc34), (0x51bc40, 0x51bfd0), (0x51c0a0, 0x51c140), (0x51c1c0, 0x51c1cc),
           (0x51c270, 0x51c2bc), (0x51c2c0, 0x51c360), (0x51c640, 0x51c700), (0x51e6f0, 0x51e704), (0x51e710, 0x51e726), (0x5da3c0, 0x5da3c1),
           (0x5e2a30, 0x5e2b80), (0x5f5f10, 0x5f5f7d), (0x5f5fa0, 0x5f5fc0), (0x632150, 0x632380), (0x6338d0, 0x6338f9), (0x67acb0, 0x67acb1),
           (0x6b55c0, 0x6b5739), (0x6b5770, 0x6b5801), (0x6b5810, 0x6b5834), (0x6b5840, 0x6b5860), (0x6b5890, 0x6b5a80), (0x6b5a90, 0x6b5ab0),
           (0x6b5b60, 0x6b5b75), (0x6b5b80, 0x6b5b9a), (0x6b5ba0, 0x6b5bc3), (0x6b5bd0, 0x6b5c00), (0x6b5c70, 0x6b5ca9), (0x6b5cb0, 0x6b5d10),
           (0x6b5e40, 0x6b5fb4), (0x6b5fc0, 0x6b603d), (0x6b6140, 0x6b6163), (0x6b62b0, 0x6b63b0), (0x6b6440, 0x6b65f0), (0x6b6830, 0x6b68e0),
           (0x6b6a50, 0x6b6a90), (0x6b6b00, 0x6b6c40), (0x6b7150, 0x6b7190), (0x6b7910, 0x6b796b), (0x6b7970, 0x6b79cb), (0x6b79d0, 0x6b7a6b),
           (0x6b7a70, 0x6b7b5f), (0x6b7b60, 0x6b7e5b), (0x6b7e60, 0x6b7ea5), (0x6b7fa0, 0x6b814e), (0x6b8180, 0x6b822d), (0x6b8230, 0x6b8370),
           (0x6b8480, 0x6b84ec), (0x6b84f0, 0x6b8569), (0x6b85f0, 0x6b86a0), (0x6b8930, 0x6b89a0), (0x6b8a70, 0x6b8ae0), (0x6b8b00, 0x6b8c63),
           (0x6b8c70, 0x6b8cf0), (0x6b8f60, 0x6b8f90), (0x6b90d0, 0x6b9154), (0x6b9160, 0x6b91ce), (0x6b9220, 0x6b925b), (0x6b9260, 0x6b926e),
           (0x6b9660, 0x6b9760), (0x6b98d0, 0x6b99ba), (0x6b99c0, 0x6b9a05), (0x6b9a10, 0x6b9aa1), (0x6b9ab0, 0x6b9b26), (0x6b9b30, 0x6b9b94),
           (0x6b9c50, 0x6b9c6a), (0x6b9c70, 0x6b9c82), (0x6b9c90, 0x6b9ca9), (0x6b9cb0, 0x6b9cc2), (0x6b9cd0, 0x6b9d02), (0x6b9d30, 0x6b9d43),
           (0x6ba1b0, 0x6ba1e7), (0x6baa10, 0x6baa20), (0x6babc0, 0x6bac04), (0x6bac30, 0x6bac5c), (0x6bb220, 0x6bb265), (0x6bb7a0, 0x6bb856),
           (0x6bb860, 0x6bb8c9), (0x6bc650, 0x6bc690), (0x6bc760, 0x6bc820), (0x6bc900, 0x6bcac0), (0x6bcb60, 0x6bcb90), (0x6bd5f0, 0x6bd7a0),
           (0x6bead0, 0x6beb03), (0x6c4670, 0x6c46a7), (0x6c46b0, 0x6c475a), (0x6c4760, 0x6c47a3), (0x6c49b0, 0x6c4c30), (0x6c4c80, 0x6c4f70),
           (0x6c4fd0, 0x6c5000), (0x6c5080, 0x6c5088), (0x6c5130, 0x6c5160), (0x6c5c50, 0x6c5c8f), (0x6c6000, 0x6c6084), (0x6d1bc0, 0x6d1bca)])
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
SENDERS = [0x51c0a0, 0x51c0e0, 0x51c2c0, 0x51c300, 0x51c640, 0x51c6b0, 0x51bc70, 0x51bcb0, 0x51bcf0, 0x51bd30, 0x51bd70]
MIXB, TOPB = RAM + 0x9000, RAM + 0x9100   # (the settings update's mixer and its loudest array)


def update_case(r, log):
    """The settings update (0x51bd70): the settings 0x803ab8-0x803ae0 (changed or not; volumes, rates and flags about their limits, the
    scale [0x785914] 0-100 or any), the target soundchannel.py's rates channel (its pool, voices and source: the rate commands; its +0x44
    bytes: 0x700b70-0x700b78; its source's slot 1: 0x700b6c's functor) and a mixer with no active voices (0x700b6c's update; timeGetTime
    a recorder, the clock disabled)."""
    import soundchannel as sc
    regions = sc.rates_case(r, log, 'cmdb80').regions
    target = bytearray(r.bytes(0x30))
    struct.pack_into('<II', target, 0x20, MIXB, sc.CHAN)
    mixer = bytearray(r.bytes(0x60))
    struct.pack_into('<I', mixer, 0x24, 0)
    struct.pack_into('<I', mixer, 0x44, TOPB)
    table = import_table(log, {})
    struct.pack_into('<I', table, 0x3ec, recorder(12, 0, True))   # timeGetTime
    set_return(log, 12, r.getrandbits(32))
    settings = bytearray(r.bytes(0x30))
    struct.pack_into('<I', settings, 0, r.choice([0, 1, 50, 100, 150, 200, r.getrandbits(32)]))
    for k in (4, 8):
        struct.pack_into('<I', settings, k, r.choice([0, 1, 50, 100, 127, 255, 0x7fffffff, r.getrandbits(32)]))
    struct.pack_into('<I', settings, 0xc, r.choice([0, 0, 0, 1, 100, 100, 0xffffffff, 0x80000000, r.getrandbits(32)]))
    struct.pack_into('<I', settings, 0x10, r.choice([1, 1, 1, 1, 0, r.getrandbits(32)]))
    struct.pack_into('<I', settings, 0x14, r.choice([0, 1, 1, r.getrandbits(32)]))
    struct.pack_into('<H', settings, 0x18, r.choice([0, 0, 1, 0x100, r.getrandbits(16)]))
    for k in (0x20, 0x24, 0x28):   # (+0x28, 0x803ae0, also one of the scaling's conditions)
        struct.pack_into('<I', settings, k, r.choice([0, 0, 1, 1, 2, 0xff, r.getrandbits(32)]))
    g = bytearray(r.bytes(0x20))
    struct.pack_into('<II', g, 8, r.choice([1] * 8 + [0, r.getrandbits(32)]), SMGR)
    struct.pack_into('<I', g, 0x14, r.choice([1] * 8 + [0, r.getrandbits(32)]))
    regions.update({sc.OBJECT: target, MIXB: mixer, TOPB: r.bytes(0x80), IMPORT_TABLE: table, 0xfa71e0: bytes(0x20), 0x803ab8: settings,
                    0x785914: struct.pack('<I', r.choice([0, 1, 50, 100, 100, -100 & 0xffffffff, r.getrandbits(32)])), 0x802bc0: g,
                    SMGR: struct.pack('<II', r.getrandbits(32), sc.OBJECT) + r.bytes(8)})
    return Call(0x51bd70, [], regions)


def sender_case(r, fn):
    """The sound commands' senders: the sound globals (on and started, either 0 now and then; the manager at SMGR, its +4 the sound
    object); 0x51c0a0 / 0x51c0e0 / 0x51c2c0 / 0x51c300 the soundobject bench's object (a mixer with voices, ids of theirs or not; for
    0x51c0e0 the sound system [0xfb1f24] as the soundcmd bench gives it; the object's own slot 6 its out hidden); 0x51c640 / 0x51c6b0
    the soundchannel bench's occluder add / remove (cmdoccadd's values and cmdoccrem's node)."""
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    log = log_region()
    if fn == 0x51bd70:
        return update_case(r, log)
    if fn in (0x51c640, 0x51c6b0):
        import soundchannel as sc
        regions = sc.voices_case(r, log, 'cmdoccadd' if fn == 0x51c640 else 'cmdoccrem').regions
        target = sc.OBJECT
    else:
        import soundobject as so
        regions = {}
        ids = so.object_regions(r, log, regions, slot6=fn in (0x51c0a0, 0x51c0e0, 0x51bc70, 0x51bcb0), voices=True)
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
    if fn in (0x51bc70, 0x51bcb0, 0x51bcf0, 0x51bd30):
        return Call(fn, [], regions)
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
