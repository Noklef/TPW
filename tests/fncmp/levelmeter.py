# The level meter's functions (src/game/ui/levelmeter: 0x4a95b0, 0x4a9fc0) against the originals. The window has random bytes with its
# dwords +0xa0: the position (16.16, often near the phases' limits 0x1f / 0x2f / 0x3f), the clock, the speed (small, negative or any),
# and the phase (0-3 mostly). Its children (+0x5c, then +0x54) are the light 0xd2d06 (now and then missing: a fault in both once it is told)
# among 0-2 decoys. The light's objects +0x98 / +0x9c are recorders of slot 3 (k 3 and 22, their event's word +4 hashed) or 0. Its
# default handler +0x118 is a cdecl recorder. The GUI clock [0x77c490]'s slot 0 is a recorder with a random result.
# SetLevel: the level 0 a third of the time, else small or any. OnMessage: the message 0x1e mostly.
# usage: levelmeter.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetLevel,OnMessage)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, blob_bytes, log_region, put, recorder, set_return, vtable  # noqa: E402

CALLS = {'SetLevel': 0x4a95b0, 'OnMessage': 0x4a9fc0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench([(0x4a95b0, 0x4a96af), (0x4a9fc0, 0x4aa0b1), (0x65e6d5, 0x65e73d), (0x65d3a3, 0x65d40e), (0x65d7bc, 0x65d832),
           (0x65f66e, 0x65f697), (0x65968e, 0x6596a2)])
WINDOW, KIDS, OBJ98, OBJ9C, VT98, VT9C, CLOCK, CLOCKVT = RAM, RAM + 0x400, RAM + 0x1000, RAM + 0x1040, RAM + 0x1080, RAM + 0x1100, RAM + 0x1200, RAM + 0x1240
KID = 0x120
LIGHT = 0xd2d06


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, VT98: put(vtable({}, size=8), 0xc, 'I', recorder(3, 2)), VT9C: put(vtable({}, size=8), 0xc, 'I', recorder(22, 2)),
               OBJ98: put(bytearray(r.bytes(0x20)), 0, 'I', VT98), OBJ9C: put(bytearray(r.bytes(0x20)), 0, 'I', VT9C),
               CLOCK: put(bytearray(r.bytes(8)), 0, 'I', CLOCKVT), CLOCKVT: vtable({0: 0}, size=4), 0x77c490: struct.pack('<I', CLOCK)}
    set_return(log, 0, r.getrandbits(32))
    blob_bytes(log, 3, 1, 0x30)
    blob_bytes(log, 22, 1, 0x30)
    w = bytearray(r.bytes(0x140))
    limit = r.choice([0x1f, 0x2f, 0x3f, 0x10, 0])
    pos = r.choice([(limit << 16) + r.randrange(-0x30000, 0x30000), r.getrandbits(22), r.getrandbits(32), 0, 0x200000, 0x300000])
    speed = r.choice([r.randrange(0, 0x40000), r.randrange(-0x40000, 0), r.getrandbits(32), 0, 0x8000])
    phase = r.choice([0, 1, 2, 3] * 4 + [4, 0xffffffff, r.getrandbits(32)])
    put(w, 0xa0, '4I', pos & 0xffffffff, r.getrandbits(32), speed & 0xffffffff, phase)
    put(w, 0x118, 'I', recorder(5, 4, cdecl=True))
    ids = ([LIGHT] if r.random() < 0.95 else []) + [r.choice([0, LIGHT + 1, LIGHT - 1, r.getrandbits(32)]) for _ in range(r.choice([0, 0, 1, 2]))]
    r.shuffle(ids)
    for i, ident in enumerate(ids):
        k = bytearray(r.bytes(KID))
        put(k, 0x8c, 'I', ident)
        put(k, 0x54, 'I', KIDS + KID * (i + 1) if i + 1 < len(ids) else 0)
        put(k, 0x98, 'I', OBJ98 if r.random() < 0.8 else 0)
        put(k, 0x9c, 'I', OBJ9C if r.random() < 0.8 else 0)
        regions[KIDS + KID * i] = k
    put(w, 0x5c, 'I', KIDS if ids else 0)
    regions[WINDOW] = w
    if op == 'SetLevel':
        level = r.choice([0] * 5 + [r.randrange(1, 101)] * 6 + [r.randrange(-40, 0), r.getrandbits(32), 0x7fff, 0x8000])
        return Call(CALLS[op], [WINDOW, level & 0xffffffff], regions, ret='v')
    message = r.choice([0x1e] * 6 + [0x1d, 0x1f, 0, r.getrandbits(32)])
    return Call(CALLS[op], [WINDOW, message, r.getrandbits(32), r.getrandbits(32)], regions)


if __name__ == '__main__':
    B.main(case)
