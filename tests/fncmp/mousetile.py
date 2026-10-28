# The tile modes' Assign and the online chat's instance (src/game/world/mousemode: 0x46d5a0 MouseModeTile_Assign, 0x46cf10
# OnlineChat_Instance) against the originals.
# Tile: a tile mode (+8 a kind: 4, 0x3b or another; +0xc the flag 0, 1 or a dword; +0x10 an id the definitions table finds, as
# recorded_actions.py's Set8186e0 sets it up, with 0x300-byte definitions: +0x288 1 or not, +0x40 0 or not); the tile [0x7b05cc] 0, 1,
# 0x80, 0x81 or any word; the env's 0x5234d0 / 0x52f200 cdecl recorders (30, 24); recorded_actions.py's recorder and globals.
# Chat: [0x7cddec] a chat (returned) or none: made in onlinechat.py's Ctor world (the new chat at HEAP + 0x10, its allocator bytes masked),
# or the heap's first block failing (0).
# usage: mousetile.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Tile,Chat)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, put  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import onlinechat  # noqa: E402
import recorded_actions as ra  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops


B = Bench([(0x46d5a0, 0x46d65f), (0x46cf10, 0x46cf7d),
           # recorded_actions.py's
           (0x403be0, 0x403f00), (0x52f880, 0x52f8e0), (0x52fa90, 0x52fac0), (0x52fad0, 0x52fb00), (0x51ab20, 0x51ab70),
           (0x412e60, 0x412e6d), (0x412e70, 0x412e8f), (0x412e90, 0x412f00), (0x5da3c0, 0x5da3c1),
           # onlinechat.py's (the constructor's)
           (0x5c8b90, 0x5c8be0), (0x479230, 0x479270), (0x4e6410, 0x4e6570), (0x4f5560, 0x4f59a0), (0x4f6080, 0x4f60a0),
           (0x4f6e70, 0x4f6f00), (0x4f7480, 0x4f74b0), (0x6c1940, 0x6c1aa0), (0x405280, 0x405390), (0x45b380, 0x45b5f0),
           (0x45b680, 0x45b8e0), (0x47bad0, 0x47bc10), (0x4e5260, 0x4e54d0), (0x4f0010, 0x4f0100), (0x4f1400, 0x4f1530),
           (0x4f23f0, 0x4f2450), (0x4f4120, 0x4f41e0), (0x4f4b70, 0x4f4c30), (0x4f5080, 0x4f5140), (0x5ec210, 0x5ec230),
           (0x67a400, 0x6902c0), (0x6918f0, 0x691a30), (0x691ba0, 0x691ce0)],
          hooks={0x687690: onlinechat._mbtowc, 0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None,
                 0x5234d0: _recorder(30, 3, cdecl=True), 0x52f200: _recorder(24, 2, cdecl=True)})
MODE, NDEFS, STRIDE = RAM + 0x30000, 24, 0x300
ITEMS = RAM + 0x31000


def tile(r):
    regions = ra.recorder(r)
    for address, size in {0x81b0c8: 0xc, 0x81eca8: 0xc, 0x8186d8: 0x20, 0x8187f0: 0x10}.items():
        regions[address] = r.bytes(size)
    defs = bytearray(r.bytes(0x740))
    items = bytearray(r.bytes(NDEFS * STRIDE))
    count = r.choice([1, r.randrange(1, NDEFS + 1), NDEFS])
    for i in range(NDEFS):
        base = i * STRIDE
        put(defs, i * 4, 'I', ITEMS + base)
        put(items, base, 'H', i + 1)
        put(items, base + 0x94, 'I', r.choice([0, 1, 8, 9, 256, r.randrange(1, 257), r.randrange(1, 17)]))
        put(items, base + 0x288, 'I', r.choice([1, 1, 0, 2, r.getrandbits(32)]))
        put(items, base + 0x40, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    put(defs, 0x528, 'i', count)
    put(defs, 0x730, 'HxxI', 0, 0)   # (no cache hit)
    mode = bytearray(r.bytes(0x20))
    flag = r.choice([0, 1, 1, r.getrandbits(32) | 1])
    put(mode, 8, 'I', r.choice([4, 4, 0x3b, 0x3b, 0, 1, 3, 5, 0x3a, r.getrandbits(32)]))
    put(mode, 0xc, 'I', flag)
    put(mode, 0x10, 'H', r.randrange(1, count + 1))   # (found: the original reads through the definition)
    regions.update({ra.DEFS: defs, ITEMS: items, ra.BUILT: r.bytes(0x20), 0x7cf83c: r.bytes(4), MODE: mode, LOG: log_region(),
                    0x7b05cc: struct.pack('<H', r.choice([0, 1, 0x80, 0x81, r.getrandbits(16)]))})
    return Call(0x46d5a0, [MODE], regions, conv='t', ret='v')


def chat(r):
    regions, mask = onlinechat.ctor(r, HEAP + 0x10)
    k = r.random()
    if k < 0.3:
        regions[0x7cddec] = struct.pack('<I', r.choice([onlinechat.CHAT, RAM + 0x123]))
        return Call(0x46cf10, [], regions, ret='a')
    regions[0x7cddec] = struct.pack('<I', 0)
    if k < 0.45:
        regions[HEAP] = heap(0x6000, fail=1)
        return Call(0x46cf10, [], regions, ret='a')
    return Call(0x46cf10, [], regions, ret='a', masks=[mask])


CALLS = {'Tile': tile, 'Chat': chat}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def case(r):
    return CALLS[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)
