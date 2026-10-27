# The sound settings' setters and flag, and the sound commands' base constructor
# (src/game/core/soundset: 0x51bb60-0x51bc20, 0x51bc40 / 0x51c270 (the manager's engine's slots 3 / 5 recorders, the position's floats hashed), 0x51e6f0, 0x51e710, 0x51c1c0, 0x51c6f0, 0x51bb40)
# against the originals. The commands' senders (0x51c0a0-0x51c700, 0x51bc70-0x51bd30, 0x51c570) and the settings update (0x51bd70): see
# sender_case() and update_case().
# usage: soundset.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=51b920,...: only those senders)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, Bench, Call, RAM, blob, blob_bytes, fill, fb, heap, import_table, log_region, recorder, set_return, strings, vtable  # noqa: E402

B = Bench([(0x51b4f0, 0x51b530), (0x51b620, 0x51b670), (0x51b7c0, 0x51b913), (0x51b920, 0x51bb35), (0x51bb40, 0x51bc34), (0x51bc40, 0x51bfd0),
           (0x51c060, 0x51c140), (0x51c1c0, 0x51c1cc), (0x51c1d0, 0x51c2bc), (0x51c2c0, 0x51c360), (0x51c570, 0x51c5c2),
           (0x51c5d0, 0x51c8c6), (0x51c8d0, 0x51c930), (0x51e6f0, 0x51e704), (0x51e710, 0x51e726), (0x5da3c0, 0x5da3c1),
           (0x5e2a30, 0x5e2b80), (0x5f5f10, 0x5f5f7d), (0x5f5fa0, 0x5f5fc0), (0x632150, 0x632380), (0x6338d0, 0x6338f9),
           (0x67acb0, 0x67acb1), (0x6b55c0, 0x6b5739), (0x6b5770, 0x6b5801), (0x6b5810, 0x6b5834), (0x6b5840, 0x6b5860), (0x6b5860, 0x6b5890), (0x6b92f0, 0x6b9310), (0x6c0bd0, 0x6c0da0),
           (0x6b5890, 0x6b5a80), (0x6b5a90, 0x6b5ab0), (0x6b5b60, 0x6b5b75), (0x6b5b80, 0x6b5b9a), (0x6b5ba0, 0x6b5c00),
           (0x6b5c50, 0x6b5d10), (0x6b5e40, 0x6b5fb4), (0x6b5fc0, 0x6b603d), (0x6b6140, 0x6b6163), (0x6b62b0, 0x6b63b0),
           (0x6b6440, 0x6b65f0), (0x6b6830, 0x6b68e0), (0x6b6a50, 0x6b6a90), (0x6b6b00, 0x6b6c40), (0x6b7150, 0x6b71a0),
           (0x6b7910, 0x6b796b), (0x6b7970, 0x6b79cb), (0x6b79d0, 0x6b7a6b), (0x6b7a70, 0x6b7b5f), (0x6b7b60, 0x6b7e5b),
           (0x6b7e60, 0x6b7ea5), (0x6b7fa0, 0x6b814e), (0x6b8180, 0x6b822d), (0x6b8230, 0x6b8370), (0x6b8480, 0x6b84ec),
           (0x6b84f0, 0x6b8569), (0x6b85f0, 0x6b86a0), (0x6b8930, 0x6b89a0), (0x6b8a10, 0x6b8ae0), (0x6b8b00, 0x6b8cf0),
           (0x6b8f60, 0x6b8f90), (0x6b90d0, 0x6b9154), (0x6b9160, 0x6b91ce), (0x6b9220, 0x6b925b), (0x6b9260, 0x6b9280),
           (0x6b9660, 0x6b9760), (0x6b98d0, 0x6b9a05), (0x6b9a10, 0x6b9aa1), (0x6b9ab0, 0x6b9b26), (0x6b9b30, 0x6b9b94),
           (0x6b9c50, 0x6b9cc2), (0x6b9cd0, 0x6b9d02), (0x6b9d30, 0x6b9d43), (0x6ba1b0, 0x6ba200), (0x6baa10, 0x6baa20),
           (0x6babc0, 0x6bac04), (0x6bac30, 0x6bac5c), (0x6bb220, 0x6bb280), (0x6bb7a0, 0x6bb856), (0x6bb860, 0x6bb8c9),
           (0x6bc650, 0x6bc690), (0x6bc760, 0x6bc820), (0x6bc900, 0x6bcac0), (0x6bcb60, 0x6bcb90), (0x6bd5f0, 0x6bd7a0),
           (0x6bead0, 0x6beb03), (0x6c4670, 0x6c46a7), (0x6c46b0, 0x6c475a), (0x6c4760, 0x6c47a3), (0x6c49b0, 0x6c4c30),
           (0x6c4c80, 0x6c4f70), (0x6c4fd0, 0x6c5000), (0x6c5080, 0x6c5088), (0x6c5130, 0x6c5160), (0x6c5c50, 0x6c5c8f),
           (0x6c6000, 0x6c60a0), (0x6d1bc0, 0x6d1bca)])
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
SENDERS = [0x51c0a0, 0x51c0e0, 0x51c2c0, 0x51c300, 0x51c640, 0x51c6b0, 0x51bc70, 0x51bcb0, 0x51bcf0, 0x51bd30, 0x51bd70, 0x51c700, 0x51c570,
           0x51b810, 0x51b620, 0x51c060, 0x51c1d0, 0x51c5d0, 0x51c8d0, 0x51b920]
ONLY = [int(k, 16) for k in os.environ.get('OPS', '').split(',') if k]
MIXB, TOPB = RAM + 0x9000, RAM + 0x9100   # (the settings update's mixer and its loudest array)
CVT, OVT = RAM + 0x9200, RAM + 0x9300   # (the camera level's channel and object vtables)


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
    the soundchannel bench's occluder add / remove (cmdoccadd's values and cmdoccrem's node); 0x51b920 b54's (the camera level: below)."""
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    log = log_region()
    if fn == 0x51bd70:
        return update_case(r, log)
    if fn in (0x51c640, 0x51c6b0, 0x51c700, 0x51b810, 0x51c1d0, 0x51c8d0, 0x51b920):   # (0x51b810: the voice count's, as soundchannel.py's b3c;
        # 0x51c1d0: the manager's placement, its three vectors the arguments; 0x51c8d0: the command's EAX record, its index the argument)
        import soundchannel as sc
        regions = sc.voices_case(r, log, {0x51c640: 'cmdoccadd', 0x51c6b0: 'cmdoccrem', 0x51c700: 'cmdoccswitch', 0x51b810: 'b3c',
                                          0x51c1d0: 'mgrplace', 0x51c8d0: 'cmdeaxrecord', 0x51b920: 'b54'}[fn]).regions
        target = sc.OBJECT
    else:
        import soundobject as so
        regions = {}
        ids = so.object_regions(r, log, regions, slot6=fn in (0x51c0a0, 0x51c0e0, 0x51bc70, 0x51bcb0, 0x51c570, 0x51c060, 0x51c5d0), voices=True)
        target = so.OBJ
        pick = r.choice(ids * 3 + [0, 1, 100, r.getrandbits(32)])
        if fn in (0x51c2c0, 0x51c300):
            strings(log, 6, 0, out=1)   # (the object's own slot 6: the out the argument's stack slot)
        if fn == 0x51b620:   # (the object's slot 8: (out, the value), the out the argument's slot)
            regions[so.OVT] = vtable({6: 2, 8: 2})
            strings(log, 8, 0, out=1)
        if fn == 0x51c5d0:   # (the voice's parameter block, or a new one)
            regions.update({so.PARAMS: r.bytes(0x20), HEAP: heap(0x40, fail=r.choice([0, 0, 0, 1]))})
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
    if fn == 0x51b920:   # (the camera level: the globals 0x7858fc-0x785970 one block, the thresholds about the level (each one now and then
        # random); 0x51c700's four levels rising from about it and its radii; 0x51b810's voice levels as its case's, [0x78594c] at or above the
        # level; the kept EAX record 0x803ab4 0 or not (the channel's records +0x68 none: 0x700b30 answers -1); 0x700b50's channel slot 9 and
        # 0x700b48's object slot 7 recorders (0x18, 0x17); the channel's hidden byte +0x64 the flag 0x700b44 gives it mostly, else 0, 1 or any,
        # over a category (0xfb1f28, constructed: its guard 0xfb1ff8 bit 0) of no records (+0x64 0)
        import soundchannel as sc
        lv0 = r.choice([0, 10, 100, r.randrange(-50, 400), r.getrandbits(32) - (1 << 31)])
        clamp = lambda v: max(-0x80000000, min(0x7fffffff, v))
        near = lambda: clamp(lv0 + r.choice([-1, 0, 0, 1, -100, 100, r.randrange(-500, 500)])) if r.random() < 0.9 else r.getrandbits(32) - (1 << 31)
        block = bytearray(r.bytes(0x74))
        put = lambda addr, v: struct.pack_into('<i' if v < 0 else '<I', block, addr - 0x7858fc, v)
        for addr in (0x7858fc, 0x785900, 0x785904, 0x785908, 0x785910, 0x785918, 0x785920):
            put(addr, near())
        levels = [near()]
        for _ in range(3):
            levels.append(clamp(levels[-1] + r.choice([0, 1, 50, 100, 300])) if r.random() < 0.8 else near())
        for addr, v in zip((0x78590c, 0x785958, 0x785960, 0x785968), levels):
            put(addr, v)
        for addr in (0x785954, 0x78595c, 0x785964):
            put(addr, r.choice([fb(1.0), fb(2.5), fb(5.0), 0, 0x7fc00000, r.getrandbits(32)]))
        c3c = near()
        c44 = clamp(c3c + r.choice([1, 10, 50, 100, 0, r.randrange(1000)]))
        level = r.choice([lv0, lv0, lv0 + 1, lv0 - 1, near(), c3c, c44, r.getrandbits(32) - (1 << 31)])
        level = clamp(level)
        put(0x785938, r.choice([0, 4, 8, r.randrange(32)]))
        put(0x78593c, c3c)
        put(0x785940, r.choice([16, 32, r.randrange(32)]))
        put(0x785944, c44)
        put(0x78594c, r.choice([0x7fffffff, level, min(0x7fffffff, level + r.randrange(100))]))
        chan = bytearray(regions[sc.CHAN]) + bytearray(r.bytes(0x10))
        hide = 0 if level > struct.unpack_from('<i', block, 0x785918 - 0x7858fc)[0] else 1
        struct.pack_into('<I', chan, 0, CVT)
        chan[0x64] = r.choice([hide, hide, hide, 0, 1, r.getrandbits(8)])
        struct.pack_into('<I', chan, 0x68, 0)
        obj = bytearray(regions[sc.OBJECT])
        struct.pack_into('<I', obj, 0, OVT)
        cvt, ovt = bytearray(0x80), bytearray(0x80)
        struct.pack_into('<I', cvt, 4 * 9, recorder(0x18, 2))
        struct.pack_into('<I', ovt, 4 * 7, recorder(0x17, 2))
        for k in (0x17, 0x18):
            strings(log, k, 0, out=1)   # (the out the level's slot)
        category = bytearray(r.bytes(0xd4))
        struct.pack_into('<I', category, 0x64, 0)
        category[0xd0] |= 1
        regions.update({0x7858fc: block, 0x790ab0: bytes([r.choice([0x16, 2, 0, r.getrandbits(8)])]), sc.CHAN: chan, sc.OBJECT: obj, CVT: cvt,
                        OVT: ovt, 0x803ab4: struct.pack('<I', r.choice([0, 0, 1, 5, r.getrandbits(32)])), 0xfb1f28: category})
        return Call(fn, [level & 0xffffffff], regions)
    if fn == 0x51c700:   # (the camera's flags; the four levels mostly rising, sometimes equal or any; the three radii any floats)
        ints = lambda: r.choice([0, 1, -1, 100, 0x7fffffff, -0x80000000, r.randrange(-1000, 1000), r.getrandbits(32) - (1 << 31)])
        low0 = ints() if r.random() < 0.3 else r.choice([0, 0, 10, -5])
        levels = [low0]
        for _ in range(3):
            levels.append(ints() if r.random() < 0.2 else levels[-1] + r.choice([0, 1, 50, 100, 300, 1000]))
        radii = [r.choice([fb(1.0), fb(2.5), fb(5.0), fb(10.0), 0, 0x7f800001, 0x7fc00000, 0x7f800000, r.getrandbits(32)]) for _ in range(3)]
        if r.random() < 0.15:   # (NaNs of each kind: the one the sums keep)
            radii = [r.choice([0x7f800001, 0x7fc00000, 0x7fa00000, 0xffc00000, 0x7fc00001]) for _ in range(3)]
        block = bytearray(r.bytes(0x70))
        struct.pack_into('<i', block, 0xc, max(-0x80000000, min(0x7fffffff, levels[0])))
        for k in range(3):
            struct.pack_into('<Ii', block, 0x54 + 8 * k, radii[k], max(-0x80000000, min(0x7fffffff, levels[k + 1])))
        regions.update({0x785900: block, 0x790ab0: bytes([r.choice([0x16, 2, 4, 0x10, 0, 0, 0x29, r.getrandbits(8)])])})
        level = r.choice(levels + [v + 1 for v in levels] + [v - 1 for v in levels] + [r.randrange(-100, 2000), r.getrandbits(32) - (1 << 31)])
        return Call(fn, [max(-0x80000000, min(0x7fffffff, level)) & 0xffffffff], regions)
    if fn == 0x51c1d0:   # (the word 0x803ad2: the position's y the float [0x700b28] instead)
        regions[0x803ad0] = r.bytes(2) + struct.pack('<H', r.choice([0, 0, 0, 1, 0x100, r.getrandbits(16)])) + r.bytes(4)
        args = [v for p in (sc.P1, sc.P2, sc.P3) for v in struct.unpack('<3I', regions[p])]
        if r.random() < 0.2:   # (y a signalling NaN: the x87's load and store quiet it)
            args[1] = r.choice([0x7f800001, 0xffbfffff, 0x7fa00000])
        return Call(fn, args, regions)
    if fn == 0x51c8d0:   # (the kept record 0x803ab4: the index's now and then)
        index = struct.unpack_from('<I', regions[sc.COMMAND], 4)[0]
        regions[0x803ab0] = r.bytes(4) + struct.pack('<I', r.choice([index, r.getrandbits(32), 0, 1])) + r.bytes(4)
        return Call(fn, [index], regions)
    if fn == 0x51c5d0:   # (the sound, then an area: x, z, w, d)
        return Call(fn, [r.choice([pick, pick, 0]), *[r.choice([0, 1, 100, r.getrandbits(32)]) for _ in range(4)]], regions)
    if fn == 0x51c6b0:
        return Call(fn, [struct.unpack('<I', regions[sc.PNODE])[0]], regions)
    if fn == 0x51c0e0:
        return Call(fn, [pick, r.getrandbits(32)], regions)
    if fn == 0x51b810:   # (the voice levels 0x785938-0x78594c: the level at or under [0x78594c] (above it 0x700b40's query runs 0x6b9060,
        # whose settings record's asked flags (+8) are never set: not comparable), above [0x785944] or interpolated (a zero span now and then)
        c3c = r.choice([0, 1, 10, r.randrange(-50, 50), r.getrandbits(32) - (1 << 31)])
        c44 = (c3c + r.choice([1, 10, 50, 100, 0, r.randrange(1000)])) & 0xffffffff
        c44 -= (c44 >> 31) << 32
        level = r.choice([c3c, c44, c44 + 1, c3c - 1, (c3c + c44) // 2, r.randrange(-100, 200), r.getrandbits(32) - (1 << 31)])
        level = max(-0x80000000, min(0x7fffffff, level))
        c4c = r.choice([0x7fffffff, level, level + r.randrange(100)])
        regions[0x785938] = struct.pack('<6I', r.choice([0, 4, 8, r.randrange(32), r.getrandbits(32)]), c3c & 0xffffffff,
                                        r.choice([16, 32, r.randrange(32), r.getrandbits(32)]), c44 & 0xffffffff, r.getrandbits(32),
                                        min(0x7fffffff, c4c) & 0xffffffff)
        return Call(fn, [level & 0xffffffff], regions)
    if fn == 0x51c570:   # (the result on the stack, returned)
        return Call(fn, [pick], regions, ret='a')
    if fn in (0x51bc70, 0x51bcb0, 0x51bcf0, 0x51bd30):
        return Call(fn, [], regions)
    return Call(fn, [pick if fn != 0x51c2c0 else r.getrandbits(32)], regions)


def case(r):
    if ONLY:
        return sender_case(r, r.deal(ONLY))
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
