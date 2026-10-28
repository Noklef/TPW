# The movie player's DirectSound helpers (src/game/gfx/moviesound: 0x672950 a buffer's size, 0x6729e0 a release, 0x672bf0 the stream's
# release, 0x672730 the play distance, 0x672990 the primary buffer's and device's release) against the originals. The objects are over a vtable of COM recorders. Bytes: GetCaps fills the
# DSBCAPS (size, flags, bytes) or, in 3 cases of 10, writes nothing (the caps as passed then show in its hash) and answers anything.
# Release / ReleaseStream: the place or the settings' +0x5c the object or null (a fifth of cases). Distance: the simulated flag +0x3cc
# set half the time; the size 0x100-0x10000 mostly (else any), the last write and the position 0 to a little past the size (else any);
# GetCurrentPosition fills the play cursor (its two pointers logged as 0). ReleaseAll (0x672990): the settings' flag 0x800000 of +4 set
# mostly, the player's +0x3d8 / +0x3dc set or not, the primary buffer +0x58 and the device +0x54 objects or null.
# usage: moviesound.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Bytes,Release,ReleaseStream,Distance,ReleaseAll)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, blob, fill, log_region, set_return, strings, vtable  # noqa: E402

CALLS = {'Bytes': 0x672950, 'Release': 0x6729e0, 'ReleaseStream': 0x672bf0, 'Distance': 0x672730, 'ReleaseAll': 0x672990}
B = Bench([(0x672950, 0x672984), (0x6729e0, 0x6729f9), (0x672bf0, 0x672c0f), (0x672730, 0x6727be), (0x672990, 0x6729d6)])
PLAYER, SETTINGS, OBJECT, TABLE, PLACE = RAM, RAM + 0x400, RAM + 0x500, RAM + 0x600, RAM + 0x700
OBJECT2 = RAM + 0x540
RELEASE, CAPS, POSITION = 2, 3, 4


def case(r):
    op = r.deal(OPS)
    log = log_region()
    obj = OBJECT if r.random() < 0.8 else 0
    regions = {LOG: log, OBJECT: struct.pack('<I', TABLE) + r.bytes(0xc), TABLE: vtable({}, com={RELEASE: 1, CAPS: 2, POSITION: 3})}
    if op == 'Bytes':
        blob(log, CAPS, 1, 0x1f)
        if r.random() < 0.7:
            fill(log, CAPS, 1, struct.pack('<3I', 0x14, r.getrandbits(32), r.getrandbits(32)))
        set_return(log, CAPS, r.choice([0, 0, 1, 0x88780096]))
        return Call(CALLS[op], [OBJECT], regions, conv='t', ret='a')
    if op == 'Release':
        regions[PLACE] = struct.pack('<I', obj) + r.bytes(4)
        return Call(CALLS[op], [PLACE], regions, conv='t', ret='a')
    if op == 'ReleaseAll':   # (the settings' flag 0x800000 of +4 set mostly, the player's +0x3d8 / +0x3dc set or not, +0x54 / +0x58 objects or null)
        player = bytearray(r.bytes(0x3e0))
        struct.pack_into('<II', player, 0x3d8, r.choice([0, 1, r.getrandbits(32)]), r.choice([0, 1, r.getrandbits(32)]))
        struct.pack_into('<I', player, 0, SETTINGS)
        settings = bytearray(r.bytes(0x60))
        struct.pack_into('<I', settings, 4, r.getrandbits(32) & ~0x800000 | r.choice([0x800000] * 4 + [0]))
        struct.pack_into('<II', settings, 0x54, OBJECT2 if r.random() < 0.8 else 0, obj)
        regions.update({PLAYER: player, SETTINGS: settings, OBJECT2: struct.pack('<I', TABLE) + r.bytes(0xc)})
        return Call(CALLS[op], [PLAYER], regions, conv='t', ret='a')
    size = r.randrange(0x100, 0x10001) if r.random() < 0.9 else r.getrandbits(32)
    near = lambda: r.randrange(0, (size & 0xffff) + 0x400) if r.random() < 0.9 else r.getrandbits(32)  # noqa: E731
    player = bytearray(r.bytes(0x3d0))
    struct.pack_into('<I', player, 0, SETTINGS)
    struct.pack_into('<II', player, 0x36c, size, near())
    struct.pack_into('<II', player, 0x3c8, near(), r.getrandbits(32) if r.random() < 0.5 else 0)
    regions.update({PLAYER: player, SETTINGS: r.bytes(0x5c) + struct.pack('<I', obj if op == 'ReleaseStream' else OBJECT) + r.bytes(4)})
    if op == 'Distance':
        fill(log, POSITION, 1, struct.pack('<I', near()))
        strings(log, POSITION, 0, 1 << 1 | 1 << 2)
    return Call(CALLS[op], [PLAYER], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
