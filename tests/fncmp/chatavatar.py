# The chat avatar's moves of src/game/online/chatavatar (0x4ea370, 0x4ea3f0, 0x4ea410, 0x4ebfb0, 0x4eab40) against the originals:
# things_methods.py's SetPos world (a park whose things sit in a cell grid), the avatar's thing now and then of kind 0x12 (so it is placed);
# Chat::MovePlayer and Chat::NotAFK (imports 0x6fd4cc, 0x6fd4bc) recorders; the cell moves with cells in the grid's four rows and random high words.
# usage: chatavatar.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Move,Say,Move3,Cell,CellMove)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import things_methods  # noqa: E402
from fncmp import IMPORT_TABLE, LOG, Bench, Call, RAM, log_region, recorder  # noqa: E402
from thingenv import POOL  # noqa: E402

B = Bench([(0x4eab40, 0x4eac22), (0x4ea370, 0x4ea3e5),(0x4ea3f0, 0x4ea409), (0x4ea410, 0x4ea485), (0x4ebfb0, 0x4ec021), (0x4f7e20, 0x4f7e4c), (0x4217f0, 0x421800),
           (0x4d91f0, 0x4d93c0), (0x4fa930, 0x4fa9a0), (0x50b350, 0x50b370), (0x50b6a0, 0x50b790), (0x5100e0, 0x510110), (0x510160, 0x510190),
           (0x412e90, 0x412f00), (0x516270, 0x516320), (0x516330, 0x516390), (0x541f60, 0x541fb0), (0x6ace60, 0x6aceb0), (0x5da3c0, 0x5da3c1)])
CALLS = {'Move': 0x4ea370, 'Say': 0x4ea3f0, 'Move3': 0x4ea410, 'Cell': 0x4ebfb0, 'CellMove': 0x4eab40}
AVATAR, CHAT = RAM + 0x20000, RAM + 0x21000


def case(r):
    op = r.deal(OPS)
    c = things_methods.setpos(r)
    regions = dict(c.regions)
    thing, x, y = c.args
    pool = bytearray(regions[POOL])
    if r.random() < 0.6:
        pool[thing - POOL + 2] = 0x12
    regions[POOL] = pool
    log = log_region()
    table = bytearray(0x600)
    struct.pack_into('<I', table, 0x6fd4cc - IMPORT_TABLE, recorder(1, 4))
    struct.pack_into('<I', table, 0x6fd4bc - IMPORT_TABLE, recorder(2, 1))
    avatar = bytearray(r.bytes(0xec0))
    struct.pack_into('<II', avatar, 0, CHAT, thing)
    regions.update({LOG: log, IMPORT_TABLE: table, AVATAR: avatar, CHAT: r.bytes(0x20)})
    cell = r.choice([1, 2, 128, 129, 512, r.randrange(1, 513)]) | r.choice([0, 0, 0x10000, r.getrandbits(16) << 16])
    if op == 'CellMove':
        return Call(CALLS[op], [AVATAR, cell], regions, conv='t', ret='b')
    if op == 'Cell':
        handle = struct.unpack_from('<H', pool, thing - POOL)[0]
        return Call(CALLS[op], [r.getrandbits(32), handle | r.choice([0, 0, r.getrandbits(16) << 16]), cell], regions, conv='t', ret='b')
    if op == 'Say':
        return Call(CALLS[op], [AVATAR], regions, conv='t', ret='a')
    return Call(CALLS[op], [AVATAR, x, y], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
