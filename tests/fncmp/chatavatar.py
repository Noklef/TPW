# The chat avatar's moves of src/game/online/chatavatar (0x4ea370, 0x4ea3f0, 0x4ea410, 0x4ebfb0, 0x4eab40, 0x4eac30) against the originals:
# things_methods.py's SetPos world (a park whose things sit in a cell grid), the avatar's thing now and then of kind 0x12 (so it is placed);
# Chat::MovePlayer and Chat::NotAFK (imports 0x6fd4cc, 0x6fd4bc) recorders; the cell moves with cells in the grid's four rows and random high
# words. GoTo: movingperson.py's person sent to a cell. Arrive (0x4ea240): the thing's script slots as personscripts.py's release.
# usage: chatavatar.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Move,Say,Move3,Cell,CellMove,GoTo,Arrive)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import things_methods  # noqa: E402
from fncmp import IMPORT_TABLE, LOG, Bench, Call, RAM, log_region, recorder  # noqa: E402
from thingenv import POOL, SLOTS  # noqa: E402

_ops = os.environ.pop('OPS', None)   # (movingperson.py reads its own OPS, as hex addresses)
import movingperson  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

B = Bench(movingperson.RANGES + [(0x4ea240, 0x4ea2c4), (0x4f9ed0, 0x4f9efc), (0x4d4170, 0x4d4186), (0x475550, 0x47558b), (0x4fe950, 0x4fe977),
                                 (0x475010, 0x476a30), (0x540b90, 0x540d78), (0x50c0d0, 0x50c0f2), (0x50bab0, 0x50bac4), (0x50be40, 0x50be7c),
(0x4eac30, 0x4eacdc),(0x4eab40, 0x4eac22),(0x4ea370, 0x4ea3e5),(0x4ea3f0, 0x4ea409), (0x4ea410, 0x4ea485), (0x4ebfb0, 0x4ec021), (0x4f7e20, 0x4f7e4c), (0x4217f0, 0x421800),
           (0x4d91f0, 0x4d93c0), (0x4fa930, 0x4fa9a0), (0x50b350, 0x50b370), (0x50b6a0, 0x50b790), (0x5100e0, 0x510110), (0x510160, 0x510190),
           (0x412e90, 0x412f00), (0x516270, 0x516320), (0x516330, 0x516390), (0x541f60, 0x541fb0), (0x6ace60, 0x6aceb0), (0x5da3c0, 0x5da3c1)])
CALLS = {'Move': 0x4ea370, 'Say': 0x4ea3f0, 'Move3': 0x4ea410, 'Cell': 0x4ebfb0, 'CellMove': 0x4eab40, 'GoTo': 0x4eac30, 'Arrive': 0x4ea240}
AVATAR, CHAT = RAM + 0x20000, RAM + 0x21000


def imports(regions):
    table = bytearray(0x600)
    struct.pack_into('<I', table, 0x6fd4cc - IMPORT_TABLE, recorder(1, 4))
    struct.pack_into('<I', table, 0x6fd4bc - IMPORT_TABLE, recorder(2, 1))
    regions.update({LOG: log_region(), IMPORT_TABLE: table})


def goto(r):
    """GoTo: movingperson.py's person (at RAM) sent to a cell (ids over the map, 0 and past it), its handle's slot it, of kind 0x12 or not."""
    c = movingperson.cell(r, 0x4f7da0)
    regions = dict(c.regions)
    person = bytearray(regions[RAM])
    handle = r.randrange(1, 12)
    struct.pack_into('<H', person, 0, handle)
    if r.random() < 0.7:
        person[2] = 0x12
    slots = bytearray(r.bytes(20 * 12))
    struct.pack_into('<I', slots, 20 * handle, RAM)
    avatar = bytearray(r.bytes(0xec0))
    struct.pack_into('<II', avatar, 0, CHAT, RAM)
    regions.update({RAM: person, SLOTS: slots, AVATAR: avatar, CHAT: r.bytes(0x20)})
    imports(regions)
    return Call(CALLS['GoTo'], [AVATAR, c.args[1]], regions, conv='t', ret='b')


def case(r):
    op = r.deal(OPS)
    if op == 'GoTo':
        return goto(r)
    c = things_methods.setpos(r)
    regions = dict(c.regions)
    thing, x, y = c.args
    pool = bytearray(regions[POOL])
    if r.random() < 0.6:
        pool[thing - POOL + 2] = 0x12
    if op == 'Arrive':   # (personscripts.py's release: script slots set or not, the script list not ready)
        for off in (0xc, 0x30, 0x210):
            struct.pack_into('<I', pool, thing - POOL + off, r.choice([0, 0, 1, 2, 7]))
        if r.random() < 0.2:
            pool[thing - POOL + 2] = 1
        regions[0x7b49f0] = bytes(16)
    regions[POOL] = pool
    avatar = bytearray(r.bytes(0xec0))
    struct.pack_into('<II', avatar, 0, CHAT, thing)
    regions.update({AVATAR: avatar, CHAT: r.bytes(0x20)})
    imports(regions)
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
