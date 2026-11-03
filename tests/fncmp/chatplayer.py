# A chat player's constructor (ChatPlayer_Ctor 0x4f7b20) and the avatar's name compared (ChatAvatar_NameEquals 0x4ea9a0), src/game/online/chatavatar, against the originals.
# The constructor's calls are recorders (the player's base constructor, the seed, the folder's sprite and count, the script's start and register, swprintf); the sprite about the count, the name's text set or null.
# The name: std::wstrings over the heap, the avatar's +8 null or a shared block; the text the same in another case, or a near miss.
# usage: chatplayer.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=ctor,name)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, put, set_return  # noqa: E402

B = Bench([(0x4f7b20, 0x4f7c2a),(0x4ea9a0, 0x4eab33), (0x50b350, 0x50b370), (0x4f21d0, 0x4f2500), (0x4f39e0, 0x4f3b60), (0x4f6080, 0x4f6400), (0x47ba30, 0x47bc10), (0x479230, 0x479270), (0x67acd0, 0x67acee), (0x67c8d0, 0x67cb00),
           (0x405280, 0x405390), (0x45b380, 0x45b5f0)],
          hooks={0x4f8940: _recorder(6, 2), 0x516370: _recorder(7, 1), 0x541f70: _recorder(8, 1, cdecl=True), 0x541f60: _recorder(9, 1, cdecl=True), 0x4d4140: _recorder(10, 3), 0x4755d0: _recorder(11, 3, cdecl=True),
                 0x67af00: _recorder(12, 3, cdecl=True)})
OPS = [k for k in os.environ.get('OPS', 'ctor,name').split(',') if k]
PLAYER, NAME, TEXT = RAM, RAM + 0x800, RAM + 0x900


def wide(s):
    return s.encode('utf-16-le') + b'\0\0'


def ctor(r):
    log = log_region()
    player = bytearray(r.bytes(0x400))
    name = bytearray(r.bytes(0x10))
    put(name, 4, 'I', r.choice([0, TEXT]))
    count = r.choice([0, 1, 2, 3, 10, r.getrandbits(32)])
    sprite = r.choice([0, 1, count, count - 1 & 0xffffffff, count + 1 & 0xffffffff, r.randrange(16), r.getrandbits(32)])
    set_return(log, 8, r.getrandbits(32))
    set_return(log, 9, count)
    regions = {LOG: log, PLAYER: player, NAME: name, TEXT: r.bytes(0x10), 0x80239c: r.bytes(4), 0x75c7f2: r.bytes(2)}
    return Call(0x4f7b20, [PLAYER, r.getrandbits(32), sprite, NAME, r.getrandbits(32)], regions, conv='t', ret='a')


def name_equals(r):
    log = log_region()
    block = heap(0x400)
    own = r.choice(['Sam', 'Samantha', 'x', 'Theo'])
    avatar = bytearray(r.bytes(0x20))
    if r.random() < 0.85:
        at = HEAP + 0x82   # (a counted block: its count at -1, from HEAP + 0x80)
        data = wide(own)
        block[0x82:0x82 + len(data)] = data
        block[0x81] = r.choice([0, 1, 2, 0xff, 0xfe])
        struct.pack_into('<III', avatar, 0xc, at, len(own), len(own) + r.randrange(0, 8))
    else:
        own = ''
        struct.pack_into('<III', avatar, 0xc, 0, 0, 0)
    text = r.choice([own, own, own.upper(), own.lower(), own + 'y', own[:-1], 'Sam', 'Sal', ''])
    regions = {LOG: log, HEAP: block, PLAYER: avatar, TEXT: wide(text), 0x700624: bytes(2) + struct.pack('<I', 0xffffffff)}
    return Call(0x4ea9a0, [PLAYER, TEXT], regions, conv='t', ret='b')


def case(r):
    return (ctor if r.deal(OPS) == 'ctor' else name_equals)(r)


if __name__ == '__main__':
    B.main(case)
