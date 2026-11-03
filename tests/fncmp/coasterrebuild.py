# A coaster's track rebuilt (src/game/coaster/rebuild: 0x43eb70) against the original: the type's part flags, the nodes' numbers and links, the slopes (with Vec3_Distance), the mesh's walk with its flags.
# Recorders (driver.cpp's kCoasterRebuildEnv): the pose 1, refresh 2, Coaster_BuildNodes 3, Coaster_TestRun 4, Contacts_UpdateBounds 5, 0x439ab0 6, Owner_DisposeChildren 7, TrackMesh_PlaceSections 8, CoasterSearch_Reset 9,
# Keys_MergeSamples 10, TrackMesh_EmitPiece 11, CoasterMesh_Finish 12.
# usage: coasterrebuild.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fb, log_region, put, set_return  # noqa: E402

HOOKS = {0x430ed0: _recorder(1, 0, cdecl=True), 0x432b50: _recorder(2, 0, cdecl=True), 0x440c10: _recorder(3, 1, cdecl=True), 0x43c450: _recorder(4, 1, cdecl=True), 0x439920: _recorder(5, 1, cdecl=True), 0x439ab0: _recorder(6, 1, cdecl=True),
         0x43eac0: _recorder(7, 1, cdecl=True), 0x4402d0: _recorder(8, 0, cdecl=True), 0x43ea90: _recorder(9, 3, cdecl=True), 0x43f030: _recorder(10, 5, cdecl=True), 0x43f4e0: _recorder(11, 5, cdecl=True), 0x43f150: _recorder(12, 1, cdecl=True)}
B = Bench([(0x43eb70, 0x43f02a), (0x470940, 0x4709a0)], hooks=HOOKS)
COASTER, TRACK, TYPE, TYPE2, PARTS, FLAGS, KIND, BANK = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x240, RAM + 0x280, RAM + 0x400, RAM + 0x600, RAM + 0x640
PIECES, NODES = RAM + 0x1000, RAM + 0x2000
SEEN, TABLE, KEYS = 0x7923ec, 0x7925dc, 0x792414


def f(r, nan):
    if r.random() < nan:
        return r.choice([0x7fc00000, 0xffc00000, 0x7fa00000, 0x7f800000])
    return fb(r.choice([0.0, 1.0, r.uniform(-4, 4), r.uniform(-4, 4), r.uniform(-40, 40)]))


def case(r):
    nan = r.deal([0, 0, 0, 0.03])
    log = log_region()
    set_return(log, 10, r.randrange(0, 0x100))
    regions = {LOG: log}
    # The type and its parts.
    parts = r.choice([0, 1, 2, 5])
    regions[TYPE] = put(bytearray(r.bytes(0x20)), 0x1c, 'I', TYPE2)
    regions[TYPE2] = put(put(bytearray(r.bytes(0x20)), 4, 'H', parts), 0x10, 'I', PARTS)
    regions[PARTS] = r.bytes(0x10 * 5)
    regions[FLAGS] = r.bytes(0x1c * 5)
    regions[TABLE] = struct.pack('<I', FLAGS)
    regions[SEEN] = struct.pack('<I', TYPE if r.random() < 0.3 else r.getrandbits(32))
    regions[0x792218] = r.bytes(4)
    regions[KEYS] = r.bytes(4)
    regions[0x79240c] = r.bytes(4)
    regions[0x7923e4] = r.bytes(4)
    regions[0x7925f8] = r.bytes(4)
    # The pieces: a chain of 0-4, each with 0-3 nodes.
    n = r.choice([0, 1, 2, 2, 3, 4])
    pieces, nodes = bytearray(), bytearray()
    regions[KIND] = bytearray(r.bytes(0x10))
    regions[BANK] = struct.pack('<I', r.choice([0, 1, 2, 3]))
    at = 0
    for p in range(n):
        piece = bytearray(r.bytes(0x110))
        here = PIECES + 0x110 * p
        flags = r.getrandbits(32)
        for bit, chance in ((0x2000, 0.2), (0x10000, 0.2), (0x20000, 0.2)):
            flags = flags | bit if r.random() < chance else flags & ~bit
        put(piece, 8, 'I', flags)
        put(piece, 0x4c, 'III', f(r, nan), f(r, nan), f(r, nan))
        put(piece, 0x64, 'I', here + 0x110 if p + 1 < n else 0)
        put(piece, 0x68, 'I', here - 0x110 if p else 0)
        put(piece, 0x74, 'I', BANK)
        put(piece, 0x100, 'I', KIND + r.choice([0, 8]))
        count = r.choice([0, 1, 1, 2, 3])
        put(piece, 0x90, 'I', NODES + at)
        one = bytearray(0x10 + 0x58 * 8)
        one[0x10:] = r.bytes(0x58 * 8)
        for k in range(count):
            put(one, 0x10 + 0x58 * k + 0x40, 'III', f(r, nan), f(r, nan), f(r, nan))
        put(one, 0, 'III', count, NODES + at + 0x10, NODES + at + 0x10 + 0x58 * 4)
        nodes += one
        at += len(one)
        pieces += piece
    kind = regions[KIND]
    put(kind, 4, 'B', r.getrandbits(8) & 0xfe)
    put(kind, 0xc, 'B', r.getrandbits(8) | 1)
    if n:
        regions[PIECES] = pieces
        regions[NODES] = nodes
    coaster = bytearray(r.bytes(0x60))
    put(coaster, 4, 'I', TYPE)
    put(coaster, 0x54, 'I', PIECES if n and r.random() < 0.9 else 0)
    regions[COASTER] = coaster
    track = bytearray(r.bytes(0x60))
    put(track, 0x18, 'I', PIECES if n else 0)
    regions[TRACK] = track
    regions[0x790fe4] = struct.pack('<I', TRACK)
    return Call(0x43eb70, [COASTER], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
