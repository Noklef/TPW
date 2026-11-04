# The game's asynchronous pointer made (src/game/gfx/pointer: 0x47e7d0) against the original: every callee a recorder, the block or the pointer failing now and then, the video card and the flag either way; what is on the stack logged by the fields it sets.
# usage: pointermake.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, blob_bytes, blob_when, fill, log_region, set_return, strings  # noqa: E402

POST, RELEASE, CTOR, ATTACH, PAUSE, KEY, NEAREST, CLIP, POSITION, ALLOC = 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
TH = {0x5fbf80: (POST, 2), 0x47e460: (RELEASE, 0), 0x5fb8e0: (CTOR, 1), 0x5fbbd0: (ATTACH, 2), 0x5fbc80: (PAUSE, 2), 0x5fb7b0: (KEY, 1)}
CD = {0x5f73a0: (NEAREST, 4), 0x5f5650: (CLIP, 1), 0x5f5640: (POSITION, 0), 0x67acc0: (ALLOC, 1)}
HOOKS = {a: _recorder(k, n) for a, (k, n) in TH.items()}
HOOKS.update({a: _recorder(k, n, cdecl=True) for a, (k, n) in CD.items()})
B = Bench([(0x47e7d0, 0x47e982)], hooks=HOOKS)
BLOCK, POINTER, CARD = RAM, RAM + 0x40, RAM + 0x1000


def case(r):
    log = log_region()
    blob_bytes(log, POST, 1, 0xff)
    blob_when(log, POST, 1, 6, 0xff, entry=0, deref=True)
    blob_when(log, POST, 1, 7, 0xf1fff, entry=1, deref=True)
    blob(log, CLIP, 0, 0xf)
    strings(log, ATTACH, 0, 1)
    fill(log, ATTACH, 0, r.bytes(8))
    blob(log, PAUSE, 0, 3)
    blob_bytes(log, KEY, 0, 0x1f)
    set_return(log, ALLOC, r.choice([BLOCK, BLOCK, BLOCK, BLOCK, 0]))
    set_return(log, CTOR, r.choice([POINTER, POINTER, POINTER, POINTER, 0]))
    set_return(log, POSITION, r.getrandbits(32))
    set_return(log, NEAREST, r.getrandbits(32))
    card = bytearray(r.bytes(0x430))
    struct.pack_into('<II', card, 0x428, r.choice([0x121a, 0x121a, r.getrandbits(32)]), r.choice([0, 4, 5, 6, 0xffffffff]))
    regions = {LOG: log, BLOCK: r.bytes(0x24), POINTER: r.bytes(0x20), CARD: card, 0x879a24: struct.pack('<I', CARD), 0x78826c: struct.pack('<I', r.choice([0, 0, 1, r.getrandbits(32)])), 0x7a1968: r.bytes(4), 0x7a1930: r.bytes(4), 0x7b4b84: r.bytes(8),
               0x8bd510: r.bytes(8)}
    return Call(0x47e7d0, [], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
