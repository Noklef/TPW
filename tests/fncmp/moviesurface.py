# The movie player's DirectDraw helpers (src/game/gfx/moviesurface: 0x670300 the surface size, 0x670660 the mode from the caps, 0x671fa0
# the lost surfaces' restore) against the originals. The settings' IDirectDraw and four surfaces are objects over vtables of COM
# recorders (each surface its own IsLost / Restore recorders, so their answers differ per surface); the flags random with the bits
# tested set about half the time, the surfaces null now and then. ReadSize: GetSurfaceDesc fills the description and answers 0, or in
# 3 cases of 10 fails (the description as passed, its size dword and the cleared rest, then shows in its hash). PickMode: GetCaps fills
# the driver's caps (size and caps) and answers 0, or fails the same (the emulation's pointer logged as 0).
# Restore: IsLost answers DDERR_SURFACELOST mostly, Restore 0 mostly; the player's +0x4c 2 half the time.
# usage: moviesurface.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=ReadSize,PickMode,Restore)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, blob, fill, log_region, recorder, set_return, strings  # noqa: E402

CALLS = {'ReadSize': 0x670300, 'PickMode': 0x670660, 'Restore': 0x671fa0}
B = Bench([(0x670300, 0x670344), (0x670660, 0x670706), (0x671fa0, 0x672083)])
PLAYER, SETTINGS, DRAW, SURFACES, VTABLES = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400
DESC, CAPS, LOST, RESTORE = 22, 11, 40, 44
SURFACELOST = 0x887601c2


def vtable(i):
    slots = [recorder(k, 0) for k in range(32)]
    slots[CAPS], slots[DESC] = recorder(CAPS, 3, True), recorder(DESC, 2, True)
    slots[24], slots[27] = recorder(LOST + i, 1, True), recorder(RESTORE + i, 1, True)
    return struct.pack('<32I', *slots)


def case(r):
    op = r.deal(OPS)
    log = log_region()
    flags = r.getrandbits(32) & ~(1 << 9 | 1 << 25 | 1 << 27)
    for b in (9, 25, 27):
        if r.random() < 0.5:
            flags |= 1 << b
    surfaces = [SURFACES + 0x10 * i if r.random() < 0.85 else 0 for i in range(4)]
    if op == 'ReadSize' and r.random() < 0.97:
        surfaces[0] = SURFACES
    settings = struct.pack('<5I', r.getrandbits(32), flags, r.getrandbits(32), r.getrandbits(32), DRAW) + struct.pack('<4I', *surfaces)
    player = bytearray(r.bytes(0x60))
    struct.pack_into('<I', player, 0, SETTINGS)
    struct.pack_into('<I', player, 0x4c, r.choice([1, 2, 2, 3, r.getrandbits(32)]))
    regions = {LOG: log, PLAYER: player, SETTINGS: settings + r.bytes(0x10), DRAW: struct.pack('<I', VTABLES + 0x400),
               VTABLES + 0x400: vtable(4)}
    for i in range(4):
        regions[SURFACES + 0x10 * i] = struct.pack('<I', VTABLES + 0x100 * i) + r.bytes(0xc)
        regions[VTABLES + 0x100 * i] = vtable(i)
        set_return(log, LOST + i, r.choice([SURFACELOST, SURFACELOST, SURFACELOST, 0, r.getrandbits(32)]))
        set_return(log, RESTORE + i, r.choice([0, 0, 0, 1, SURFACELOST]))
    blob(log, DESC, 1, (1 << 27) - 1)
    if r.random() < 0.7:   # (else it fails, and the description as passed shows in the hash: a fill comes before it)
        fill(log, DESC, 1, struct.pack('<I', 0x6c) + r.bytes(0x68))
        set_return(log, DESC, 0)
    else:
        set_return(log, DESC, r.choice([1, 0x80000000, r.getrandbits(32) | 1]))
    blob(log, CAPS, 1, 0xffffffff)
    strings(log, CAPS, 0, 1 << 2)
    caps = r.getrandbits(32) & ~0x6300
    for b in (0x100, 0x200, 0x2000, 0x4000):
        if r.random() < 0.7:
            caps |= b
    if r.random() < 0.7:
        fill(log, CAPS, 1, struct.pack('<II', 0x17c, caps), 1)
        set_return(log, CAPS, 0)
    else:
        set_return(log, CAPS, r.choice([1, 0x80000000]))
    return Call(CALLS[op], [PLAYER], regions, conv='t', ret='v' if op == 'ReadSize' else 'a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
