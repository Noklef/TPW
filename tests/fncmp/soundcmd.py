# The sound commands' runs (src/game/core/soundcmd) against the originals: a command at CMD (its value random, a float of each kind for
# 0x6b56a0: signalling and quiet NaNs, infinities, denormals), the out dword at OUT, the target at TARGET (a recorder vtable, its +0x24 the
# part at PART, another recorder vtable, or random now and then: a fault in both). The recorders log the stack functor's vtable, not
# its address. The commands on the sound object's mixer and channel (OBJECT) get the soundobject bench's object (object_regions: a mixer
# with voices, a channel; 0x6b5c70's functor 3 send and 0x6b5c50's functor 2 send too), ids of its voices or not, and for 0x6b58d0 the sound system [0xfb1f24] (a recorder vtable). 0x6b55c0: the
# channel's source a recorder vtable (slot 1), the mixer's update with no active voices (the soundmixer bench runs them: the loudest's
# array, cleared first, stays empty), timeGetTime a recorder (the clock disabled).
# usage: soundcmd.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, blob_bytes, heap, log_region, recorder, set_return, vtable  # noqa: E402

CALLS = {'b2c': 0x6b5770, 'b34': 0x6b56a0, 'b48': 0x6b5790, 'b50': 0x6b5840, 'b64': 0x6b5640, 'b68': 0x6b5670, 'b70': 0x6b59f0,
         'b74': 0x6b5a50, 'b78': 0x6b5990, 'ba8': 0x6b58b0, 'b5c': 0x6b5600, 'b60': 0x6b5620, 'bbc': 0x6b56c0, 'ba4': 0x6b58d0, 'bac': 0x6b5930,
         'bb8': 0x6b5a90, 'b6c': 0x6b55c0, 'ba0': 0x6b5c70, 'b9c': 0x6b5c50}
OBJECT = ('b5c', 'b60', 'bbc', 'ba4', 'bac', 'bb8', 'b6c', 'ba0', 'b9c')
B = Bench([(a, a + (0x30 if a in (0x6b5640, 0x6b5670) else 0x20)) for a in CALLS.values() if a != 0x6b56c0] + [(0x6b9220, 0x6b9230), (0x6b9900, 0x6b9930), (0x6b56c0, 0x6b5720),
           (0x6b85f0, 0x6b86a0), (0x6b8b00, 0x6b8b70), (0x6b8930, 0x6b8980), (0x6b6550, 0x6b65e0), (0x6b6830, 0x6b68e0), (0x6b6b40, 0x6b6bb0),
           (0x6b64b0, 0x6b6550), (0x6b7150, 0x6b7190), (0x6bc900, 0x6bca10), (0x6c5130, 0x6c5150), (0x6b55c0, 0x6b55f7), (0x6b5e40, 0x6b5e59),
           (0x6b62b0, 0x6b6396), (0x6b6ba0, 0x6b6bb4), (0x5f5f10, 0x5f5f7d), (0x5f5fa0, 0x5f5faa), (0x6b8a10, 0x6b8ac5)])
CMD, OUT, TARGET, PART, VT1, VT2 = RAM, RAM + 0x10, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400
FLOATS = [0x7f800001, 0x7fbfffff, 0xff800001, 0x7fc00000, 0xffc00000, 0x7f800000, 0xff800000, 0x00000001, 0x807fffff, 0, 0x80000000,
          0x3f800000]
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


CMDX, SYS, SVT = RAM + 0xb00, RAM + 0xc00, RAM + 0xd00
SYSTEM = 0xfb1f24


SRC, SRCV, TOP = RAM + 0xe00, RAM + 0xe40, RAM + 0xe80


def b6c_regions(r, log, regions, so):
    """0x6b55c0's: the channel's source; no active voices; timeGetTime."""
    struct.pack_into('<I', regions[so.CHAN], 0x18, SRC)
    struct.pack_into('<I', regions[so.MIX], 0x24, 0)
    struct.pack_into('<I', regions[so.MIX], 0x44, TOP)
    set_return(log, 1, r.getrandbits(32))
    set_return(log, 12, r.getrandbits(32))
    struct.pack_into('<I', regions[IMPORT_TABLE], 0x3ec, recorder(12, 0, True))   # timeGetTime
    regions.update({SRC: struct.pack('<I', SRCV) + r.bytes(0xc), SRCV: vtable({}), TOP: r.bytes(0x80),
                    0xfa71e0: bytes(0x20)})


def object_case(r, name):
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    import soundobject as so
    log = log_region()
    regions = {}
    ids = so.object_regions(r, log, regions, slot6=name != 'bac', voices=True)
    pick = r.choice(ids * 3 + [0, 1, 100, r.getrandbits(32)])
    a, b = {'b60': (r.choice([0, 0, 1, r.getrandbits(32)]), 0), 'bbc': (r.choice([pick, pick, 0]), 0), 'ba4': (r.getrandbits(32), pick),
            'bac': (pick, r.getrandbits(32)), 'ba0': (pick, r.getrandbits(32)), 'b9c': (pick, r.getrandbits(32)), 'bb8': (so.RESULT, pick)}.get(name, (r.getrandbits(32), r.getrandbits(32)))
    regions[CMDX] = struct.pack('<III', r.getrandbits(32), a, b) + r.bytes(0xc)
    if name == 'b6c':
        b6c_regions(r, log, regions, so)
    if name == 'bbc':   # (the voice's parameter block, or a new one)
        regions.update({so.PARAMS: r.bytes(0x20), HEAP: heap(0x40, fail=r.choice([0, 0, 0, 1]))})
    if name == 'ba4':   # the sound system's functor: its vtable, the sound; the record's +0x28-+0x2a
        blob_bytes(log, 4, 1, 0xff, inner=(8, 0x7000, 28))
        regions.update({SYSTEM: struct.pack('<I', SYS), SYS: struct.pack('<I', SVT), SVT: vtable({4: 2})})
    return Call(CALLS[name], [CMDX, so.OUT, so.OBJ], regions, conv='t', ret='a')


def case(r):
    name = r.deal(OPS)
    if name in OBJECT:
        return object_case(r, name)
    log = log_region()
    blob_bytes(log, 4, 1, 0xf)   # the functor (on the stack): its vtable
    value = r.choice(FLOATS + [r.getrandbits(32)]) if name == 'b34' else r.choice([0, 1, 2, 0xff, 0x100, r.getrandbits(32)])
    target = bytearray(r.bytes(0x40))
    struct.pack_into('<I', target, 0, VT1)
    struct.pack_into('<I', target, 0x24, PART if r.random() < 0.97 else r.getrandbits(32))
    part = bytearray(r.bytes(0x60))
    struct.pack_into('<I', part, 0, VT2)
    slots = {k: 2 for k in (4, 6, 7, 8, 9)}
    regions = {CMD: struct.pack('<II', r.getrandbits(32), value), OUT: r.bytes(4), TARGET: target, PART: part, VT1: vtable(slots),
               VT2: vtable(slots), LOG: log}
    return Call(CALLS[name], [CMD, OUT, TARGET], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
