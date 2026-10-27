# A channel's sound source (src/game/core/soundsource) against the originals. The rate's set (0x6d21d0): a source at SRC whose rate (+0xc)
# is 11025, 22050 or any, low 0, 1 or any; the device singleton at 0xfb2030 made already (its guard set: no atexit), its primary buffer
# at PRIM or null, whose SetFormat (a COM recorder, the format logged as a hash) succeeds or fails. The orientation's set (0x6d37b0): the
# source's COM object (+0x1c) a recorder vtable whose slot 11 takes six floats, null now and then; the two vectors' floats normal, zero,
# NaNs (quiet and signalling, either sign) or any. The events (0x6d3020): +4 small, huge or any (the heap failing now and then:
# a write through null in both), CreateEventA a recorder. The caps test (0x6d3070): +0x18 a recorder whose slot 11 fills the caps' flags +4
# (bits 8, 2 and 0x40 set mostly).
# The QSound mixer source's config (0x6d1f40), flags (0x6d20a0) and sound (0x6d2290): the config +0x20 null mostly; info's +0x40 / +0x44
# small (+0x4c's bit 8 picks), its +0x4c bit 1 either way, +0x68 0, 1 or any; the device singleton made already, its sound DS (a recorder
# vtable: AddRef 1, Release 2, GetCaps 4 filling the caps' max 3D streaming buffers, small) or null (DirectSoundCreate a recorder filling DS
# in); the cached caps (0xfb201c) done or not.
# The source's destroy (0x6c42b0): the slot's source a recorder vtable (slots 7 and 0), null now and then (a fault in both).
# usage: soundsource.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import (HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, blob_bytes, fb, fill, heap, import_table, log_region, recorder, set_return,  # noqa: E402
                   strings, vtable)

CALLS = {'setrate': 0x6d21d0, 'orient': 0x6d37b0, 'events': 0x6d3020, 'caps': 0x6d3070, 'configure': 0x6d1f40, 'flags': 0x6d20a0,
         'sound': 0x6d2290, 'destroy': 0x6c42b0}
B = Bench([(0x6d21d0, 0x6d2214), (0x6d6c70, 0x6d6cf0), (0x6c4390, 0x6c4410), (0x6c4500, 0x6c4541), (0x6d3a80, 0x6d3b04),
           (0x6d1f40, 0x6d2093), (0x6d20a0, 0x6d2110), (0x6d2290, 0x6d22a5), (0x6c4040, 0x6c418b),
           (0x6d37b0, 0x6d37fe), (0x6d3020, 0x6d306a), (0x6d3070, 0x6d30a8), (0x6c42b0, 0x6c42d2)])
EVENT_K, CAPS_K = 42, 11
SRC, OUT, PRIM, PRIM_VT, LISTENER, LVT, FRONT, TOP = (RAM + 0x100 * i for i in range(8))
DEVICE_GUARD = 0xfb2028   # (the guard's byte, then the device at 0xfb2030)
FORMAT_K = 41
DS, DS_VT, INFO = RAM + 0x800, RAM + 0x900, RAM + 0xa00
STATICS = 0xfb201c   # (Obj_CacheStatics's max and free 3D streaming buffers, then its done byte)
DSCREATE_K = 44
SLOT, DSRC, DSRC_VT = RAM + 0xb00, RAM + 0xc00, RAM + 0xd00   # (0x6c42b0: the slot, its source, a recorder vtable)
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def floats(r):
    return struct.pack('<3I', *[r.choice([fb(r.uniform(-8, 8)), 0, 0x80000000, 0x7fc00000, 0x7f800001, 0xffa00000, 0x7fbfffff,
                                          r.getrandbits(32)]) for _ in range(3)])


def mixer(r, name, log):
    caps = bytearray(r.bytes(0x40))
    struct.pack_into('<I', caps, 0x30, r.choice([0, 0, 1, 4, 30, r.getrandbits(32)]))
    fill(log, 4, 1, bytes(caps), which=1, offset=4)
    fill(log, DSCREATE_K, 1, struct.pack('<I', DS))
    for k in (DSCREATE_K, 4):
        set_return(log, k, r.choice([0, 0, 0, 1, 0x88780078]))
    for k in (1, 2):
        set_return(log, k, r.getrandbits(32))
    statics = struct.pack('<2I', r.choice([0, 0, 1, 4, r.getrandbits(32)]), r.getrandbits(32)) + bytes([r.choice([0, 1, 1, r.getrandbits(8)])]) + r.bytes(3)
    regions = {LOG: log, IMPORT_TABLE: import_table(log, {'DirectSoundCreate': (DSCREATE_K, 0x6fd028, 3, 0, 0b10)}), STATICS: statics,
               DS: struct.pack('<I', DS_VT) + r.bytes(0xc), DS_VT: vtable({}, com={1: 1, 2: 1, 4: 2}), HEAP: heap(0x400),
               DEVICE_GUARD: bytes([r.choice([1, 3, 0xff])]) + r.bytes(7) + struct.pack('<I', r.choice([DS, DS, 0])) + r.bytes(4),
               OUT: r.bytes(4)}
    src = bytearray(r.bytes(0x40))
    struct.pack_into('<I', src, 0x20, 0 if r.random() < 0.85 else r.getrandbits(32))
    regions[SRC] = src
    if name == 'sound':
        return Call(0x6d2290, [SRC, OUT], regions, conv='t', ret='b')
    if name == 'flags':
        return Call(0x6d20a0, [SRC], regions, conv='t', ret='v')
    info = bytearray(r.bytes(0x70))
    struct.pack_into('<2I', info, 0x40, *[r.choice([0, 1, 2, 3, 0x10, 0x40]) for _ in range(2)])
    struct.pack_into('<I', info, 0x68, r.choice([0, 1, r.getrandbits(32)]))
    regions[INFO] = info
    return Call(0x6d1f40, [SRC, OUT, INFO], regions, conv='t', ret='a')


def case(r):
    name = r.deal(OPS)
    log = log_region()
    if name in ('configure', 'flags', 'sound'):
        return mixer(r, name, log)
    if name == 'destroy':   # (the slot's source a recorder vtable, null now and then: a fault in both)
        regions = {LOG: log, SLOT: struct.pack('<I', DSRC if r.random() < 0.95 else 0), DSRC: struct.pack('<I', DSRC_VT) + r.bytes(0xc),
                   DSRC_VT: vtable({0: 1})}
        return Call(0x6c42b0, [r.getrandbits(32), SLOT], regions, conv='t', ret='v')
    if name == 'setrate':
        prim_vt = b''.join(struct.pack('<I', recorder(FORMAT_K if i == 14 else 48 + i, 2 if i == 14 else 0, True)) for i in range(16))
        blob_bytes(log, FORMAT_K, 1, 0x3ffff)
        set_return(log, FORMAT_K, r.choice([0, 0, 1, 0x88780032, 0x80000000]))
        src = bytearray(r.bytes(0x10))
        struct.pack_into('<I', src, 0xc, r.choice([11025, 22050, 11025, 22050, r.getrandbits(32)]))
        device = struct.pack('<II', r.getrandbits(32), r.choice([PRIM, PRIM, 0]))
        regions = {LOG: log, SRC: src, OUT: r.bytes(4), PRIM: struct.pack('<I', PRIM_VT) + r.bytes(0xc), PRIM_VT: prim_vt,
                   DEVICE_GUARD: bytes([r.choice([1, 3, 0xff])]) + r.bytes(7) + device}
        return Call(0x6d21d0, [SRC, OUT, r.choice([0, 1, r.getrandbits(32)])], regions, conv='t', ret='a')
    if name == 'events':
        imports = import_table(log, {'event': (EVENT_K, 0x6fd0d4, 4, 0, 0)})
        set_return(log, EVENT_K, r.getrandbits(32))
        src = bytearray(r.bytes(0x34))
        struct.pack_into('<I', src, 4, r.choice([0, 1, 2, 3, 6, 0, 1, 2, 0x80000000, 0x80000003, 0x7fffffff]))
        regions = {LOG: log, SRC: src, IMPORT_TABLE: imports, HEAP: heap(0x80, fail=r.choice([0] * 7 + [1]))}
        return Call(0x6d3020, [SRC], regions, conv='t', ret='v')
    if name == 'caps':   # (slot 11's caps are the caller's stack: logged as 0, their flags +4 filled)
        strings(log, CAPS_K, 0, out=0b10)
        flags = r.choice([0x4a, 0x4a, 0xff, 0x42, 0x48, 0x0a, 0, r.getrandbits(8)]) | r.getrandbits(24) << 8
        fill(log, CAPS_K, 1, struct.pack('<I', flags), offset=4)
        set_return(log, CAPS_K, r.getrandbits(32))
        src = bytearray(r.bytes(0x20))
        struct.pack_into('<I', src, 0x18, LISTENER)
        regions = {LOG: log, SRC: src, LISTENER: struct.pack('<I', LVT) + r.bytes(0xc), LVT: vtable({}, com={CAPS_K: 2})}
        return Call(0x6d3070, [SRC], regions, conv='t', ret='a')
    set_return(log, 11, r.getrandbits(32))
    src = bytearray(r.bytes(0x20))
    struct.pack_into('<I', src, 0x1c, r.choice([LISTENER] * 6 + [0]))
    regions = {LOG: log, SRC: src, OUT: r.bytes(4), LISTENER: struct.pack('<I', LVT) + r.bytes(0xc), LVT: vtable({}, com={11: 7}),
               FRONT: floats(r), TOP: floats(r)}
    return Call(0x6d37b0, [SRC, OUT, FRONT, TOP], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
