# A visitor's per-state steps (src/game/world/visitorsteps: 0x4ff7f0, 0x500900, 0x500ad0, 0x500bd0, 0x501160, 0x4feb10, 0x4ff520,
# 0x4ff730, 0x4ffb20, 0x500a50, 0x4fc890, 0x4fc800) against the originals. The
# calls they make through VisitorStepsEnv are thiscall recorders (driver.cpp): Sprite_WalkNav 1 of 0, Sprite_SetDestination 2 of 2 (x and y
# logged as their low words: the original's high halves are stack garbage), Sprite_SetDestinationCell 3 of 1 (a word), Sprite_SpawnParticle
# 4 of 1, Thing_Delete 5 of 0, Visitor_SetState 26 of 1, Object_BackOfQueueTo 6 of 1 (writing a cell), Object_QueuePosition 7 of 1 (a word),
# Object_AnyQueueSpot 8 of 4 (writing the cell and the spot). The park: thingenv.py's things and slots (handles 1-11, kinds 3 mostly),
# ridelist.py's ride records (the things' +0x24 their ids, element 1 the one asked for), the world [0x80239c] (its seed, +0x1da70c / 710,
# the handles +0x1da72a / 72c / 732, the count +0x2d0); the balance positions 0x78559c - 0x7855e8 (ticket booths, bus stop, entrances,
# crossings) small (near the map's top or bottom rows now and then), the visitor standing on one of them now and then (Arrive: the booths
# mostly, Go / Stop: the bus stop's four cells); the mark word of the visitor's cell ([0x8023a0] + cell * 0x44 - 0x20)
# its handle mostly.
#   Arrive (0x4ff7f0): +0x1da710 0 mostly, the ride element 1 mostly; the countdown +0x200 and +0x204 set or not.
#   Leave (0x500900): the walk's answer 0 mostly (or 2, 1); the pending +0x1de 0 or a handle (kind 3 mostly), the back of its queue 0 or a cell.
#   Go (0x500ad0) and Stop (0x500bd0): Game_PollRideElement1's and Game_TakeRideElement1's worlds (ridelist.py's), the arrival handles equal
#   or not.
#   Queue (0x501160): the object +0x1dc any handle, its queue place -1, small or negative, the spot any.
#   Kill (0x4feb10): personscripts.py's script list, the sprite script +0x210 0 or one of its slots.
#   Booth (0x4ff520) as Go; WalkIn (0x4ff730): the take's element 3 mostly, the speeds 0x75c7f0 - 0x75c7f5 any; Enter (0x4ffb20) as Arrive;
#   Out (0x500a50): +0x1bc about 0, happiness +0x19c 0-300 (its __ftol low byte 0 at 0, 256), the cell's type plain or not.
#   Again (0x4fc890): the state +0x224 any. Emote (0x4fc800): the mode 4 or 5 mostly, the handle's low 4 bits 0 half the time, Kill's script
#   list; Sprite_PlaySound a thiscall recorder (10 of 2).
# usage: visitorsteps.py [CASES [SEED]] [--wild|--both] [--show N]
#   (env OPS=Arrive,Leave,Go,Stop,Queue,Kill,Booth,WalkIn,Enter,Out,Again,Emote)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _cdecl_log2, _recorder, blob_bytes, fb, fill, log_region, put, set_return, strings  # noqa: E402
import personscripts  # noqa: E402
from thingenv import POOL, THING, park  # noqa: E402

B = Bench([(0x4ff7f0, 0x4ff9ca), (0x500900, 0x500a4e), (0x500ad0, 0x500bc2), (0x500bd0, 0x500db5), (0x501160, 0x501282),
           (0x51a280, 0x51a2f0), (0x51a690, 0x51a760), (0x51a760, 0x51a9c3), (0x51aad0, 0x51aaee), (0x516330, 0x516370),
           (0x4d8610, 0x4d8750), (0x4cd4e0, 0x4cd4f7), (0x50b350, 0x50b370), (0x5da3c0, 0x5da3c1), (0x55a070, 0x55a0b0),
           (0x55a0b0, 0x55a100), (0x55a390, 0x55a3e0), (0x412e90, 0x412eea), (0x4cd300, 0x4cd331), (0x4dd7f0, 0x4dd930),
           (0x4dd4e0, 0x4dd510), (0x4feb10, 0x4feb43), (0x4ff520, 0x4ff5b0), (0x4ff730, 0x4ff7e1), (0x4ffb20, 0x4ffbbe),
           (0x500a50, 0x500acc), (0x4fc890, 0x4fc89d), (0x4fc800, 0x4fc88b), (0x4217f0, 0x421800), (0x51aaf0, 0x51ab1a), (0x67a830, 0x67a860), (0x4fa990, 0x4fa9fa), (0x536300, 0x53644c),
           # (personscripts.py's script list: SpriteScript_Kill, the heap)
           (0x475010, 0x476a30), (0x540b90, 0x540da0), (0x541f60, 0x541fb0), (0x4d4100, 0x4d4134), (0x4d4170, 0x4d4186),
           (0x5f5e20, 0x5f5e90), (0x5f5fa0, 0x5f5fc0), (0x5f5660, 0x5f56a0),
           (0x4fec90, 0x4ff4e0), (0x50c0d0, 0x50c1a0), (0x4fea70, 0x4feb10), (0x519510, 0x519560), (0x4fe980, 0x4fe9a0)],
          hooks={0x4fa2a0: _recorder(1, 0), 0x4fa5f0: _recorder(2, 2), 0x4fa530: _recorder(3, 1), 0x4fa470: _recorder(4, 1),
                 0x50b780: _recorder(5, 0), 0x501db0: _recorder(26, 1), 0x4de110: _recorder(6, 1), 0x4ddf50: _recorder(7, 1),
                 0x4de7e0: _recorder(8, 4), 0x4faa00: _recorder(10, 2),
                 0x4d93b0: _recorder(18, 1), 0x6e7074: _cdecl_log2, 0x500dc0: _recorder(19, 2), 0x501020: _recorder(20, 1), 0x4c8eb0: _recorder(21, 3),
                 0x500f00: _recorder(22, 1), 0x4fe950: _recorder(23, 0), 0x4f9490: _recorder(24, 0), 0x4fcb10: _recorder(25, 0), 0x40fb10: _recorder(27, 1)})
CALLS = {'Arrive': 0x4ff7f0, 'Leave': 0x500900, 'Go': 0x500ad0, 'Stop': 0x500bd0, 'Queue': 0x501160, 'Kill': 0x4feb10, 'Booth': 0x4ff520,
         'WalkIn': 0x4ff730, 'Enter': 0x4ffb20, 'Out': 0x500a50, 'Again': 0x4fc890,
         'Emote': 0x4fc800, 'Idle': 0x4fec90}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
WORLD = RAM   # (thingenv.py's world pointers)
NODES, ARRAYS, VIS = RAM + 0x1000, RAM + 0x4000, RAM + 0x9000
MAPC = 0x31000000
SCRIPTS = RAM + 0x50000   # (personscripts.py's script list)


def rides(r, regions, handles, elements):
    """ridelist.py's records (up to four ids, element 1 from elements), the things naming them."""
    count = r.choice([1, 2, 3, 4, 4, 0])
    ids = r.sample(range(1, 9), count)
    nodes = bytearray(r.bytes(0x100 * 4))
    arrays = bytearray(r.bytes(0x40 * 4))
    for i, rid in enumerate(ids):
        o = 0x100 * i
        put(nodes, o, 'I', NODES + 0x100 * (i + 1) if i + 1 < count else 0)
        put(nodes, o + 8, 'I', rid)
        put(nodes, o + 0x14, 'I', r.choice(ids + [99]))
        put(nodes, o + 0x1c, 'I', ARRAYS + 0x40 * i)
        put(nodes, o + 0x8c, 'i', r.choice([2, 4, 4, 4, 1]))
        put(arrays, 0x40 * i + 4, 'I', r.choice(elements))
    regions.update({NODES: nodes, ARRAYS: arrays, 0x8791a0: struct.pack('<I', r.choice([0, 1, 1, 1, 1, 1])),
                    0x8791b0: struct.pack('<I', NODES if count else 0)})
    pool = bytearray(regions[POOL])
    for i in range(len(handles)):
        put(pool, THING * i + 0x24, 'I', r.choice(ids * 3 + [0, 99]) if ids else r.choice([0, 99]))
    regions[POOL] = pool


def cell_of(x, y):
    return ((y << 7) + x + 1) & 0xffff


def idle(r, regions, log, v, handles, w):
    """Idle (0x4fec90): the visitor's row 0x40 up, so that the world's cell record (RAM + cell * 0x44) is clear of the park; the clocks about their
    limits, the moods about theirs, one recorder answer or another; the chain of the world's cell through the park's things."""
    pool = bytearray(regions[POOL])
    v[7] = r.randrange(0x40, 0x80)
    own = cell_of(v[5], v[7])
    now = r.getrandbits(32)
    put(w, 0xc, 'I', now)
    put(w, 0x10, 'I', r.choice([0, 0, 0, 0, 0, 0, 1]))
    put(w, 0x20, 'H', r.choice(handles))
    regions[WORLD + 0x1da700] = w
    put(v, 0x208, 'I', (now - r.choice([0, 0, 0, 0x64, 0x64, 0x65])) & 0xffffffff)
    put(v, 0x1fc, 'I', (now - r.choice([0, 0x1e, 0x1f, 0x1f, 0x1f, 1000])) & 0xffffffff)
    put(v, 0x19c, 'I', fb(r.choice([0.0, 1.0, 14.0, 14.9, 14.9, 14.9, 15.0, 80.9, 81.0, 200.0, 256.0, r.uniform(1, 15), r.uniform(1, 15)])))
    put(v, 0x1b0, 'I', fb(r.choice([0.0, 50.0, 80.0, 100.0, 100.5, 101.0, 356.0, r.uniform(0, 100)])))
    put(v, 0x1b4, 'I', fb(r.choice([0.0, 50.0, 89.9, 90.0, 91.0, 255.0, 256.0, r.uniform(0, 100)])))
    put(v, 0x1bc, 'I', r.choice([1, 1, 1, 1, 1, 1, 0, 0x100]))
    put(v, 0x1c0, 'B', r.choice([0x64, 0x65, 0x66, 0x64, 0x65, 0x66, 0x63, 0x67, 0, 0xff]))
    put(v, 0x1dc, 'H', r.choice([0, 1, 0x1234]))
    put(v, 0x220, 'I', r.choice([6, 7, 0xa, 6, 7, 0xa, 5, 8, 9, 0xb]))
    put(v, 0x30 + 0x88, 'I', r.randrange(33))
    regions[VIS] = v
    # (the recorders' answers and fills)
    head = r.choice(handles + [0])
    strings(log, 19, 0, out=1)
    fill(log, 19, 0, struct.pack('<H', r.choice([0, 0, 0, head, r.getrandbits(16)])), which=4)
    strings(log, 20, 0, out=1)
    fill(log, 20, 0, struct.pack('<H', r.choice([0, 0, 0, 0, r.choice(handles)])), which=5)
    struct.pack_into('<I', log, LOG_BYTES + 4 * 21, 0x20000)   # (the cell: a low word)
    set_return(log, 21, r.choice([0, 0, 1, 2]))
    staffAt = RAM + 0x8000
    strings(log, 22, 0, out=1)
    set_return(log, 22, staffAt)
    regions[staffAt] = struct.pack('<H', r.choice(handles))
    set_return(log, 24, r.choice([0, 1]))
    set_return(log, 3, r.choice([0, 0, 0, 1]))
    regions[0x803a24] = struct.pack('<I', r.getrandbits(32))
    set_return(log, 10, r.getrandbits(32))
    blob_bytes(log, 27, 0, 0xf3ff)
    regions[0x788d3c] = struct.pack('<I', RAM + 0x8800)
    regions[0x785058] = r.bytes(3)
    regions[0x70048c] = struct.pack('<I', r.choice([2, 3, 5, 10, 0x20]))
    regions[0x700794] = struct.pack('<3I', fb(r.choice([5.0, 20.0, 1.5, r.uniform(0, 20)])), fb(r.choice([100.0, 10.0, 12.0])), fb(r.choice([0.0, 0.0, 5.0, 10.0])))
    regions[0x7850e0] = struct.pack('<I', r.choice([0, 1, 50, 99, 100]))
    # (the chain of the world's cell, things of the park linked by +0xa)
    chain = r.sample(handles, r.randrange(min(len(handles), 3) + 1))
    for i, h in enumerate(chain):
        put(pool, THING * (handles.index(h)) + 0xa, 'H', chain[i + 1] if i + 1 < len(chain) else 0)
        put(pool, THING * (handles.index(h)) + 2, 'B', r.choice([1, 1, 3]))
        put(pool, THING * (handles.index(h)) + 0x210, 'I', r.choice([0, 0, 1]))
        put(pool, THING * (handles.index(h)) + 0x19c, 'I', r.choice([0xe, 0xe, 3]))
    if r.random() < 0.3 and handles:
        put(v, 0, 'H', r.choice(chain + handles))
    cell = bytearray(0x44)
    put(cell, 0x24, 'H', chain[0] if chain else 0)
    regions[WORLD + 0x294 + own * 0x44] = cell
    regions[POOL] = pool


def case(r):
    name = r.choice(OPS)
    log = log_region()
    struct.pack_into('<I', log, LOG_BYTES + 4 * 2, 0x30000)   # (x, y: low words)
    struct.pack_into('<I', log, LOG_BYTES + 4 * 3, 0x10000)
    struct.pack_into('<I', log, LOG_BYTES + 4 * 7, 0x10000)
    regions, handles, _a = park(r, count=r.choice([1, 2, 3, 4, 6, 6]), kinds=(3, 3, 3, 3, 1, 2, 0xff), defs=False)
    regions[LOG] = log
    h = lambda: r.choice(handles) if handles and r.random() < 0.85 else r.randrange(12)  # noqa: E731
    # (the world: seed, +0x1da70c - +0x1da740, +0x2d0)
    w = bytearray(r.bytes(0x40))
    put(w, 0x10, 'I', r.choice([0, 0, 0, 0, 1, r.getrandbits(32)]))   # +0x1da710
    a = h() if r.random() < 0.85 else 0
    put(w, 0x2a, 'HH', a, a if r.random() < 0.6 else r.choice([0, h()]))
    put(w, 0x32, 'H', h() if r.random() < 0.9 else 0)
    regions[WORLD + 0x1da700] = w
    regions[WORLD + 0x2d0] = struct.pack('<i', r.choice([0, 9, 10, 11, 5]))
    one = [1, 1, 1, 1, 0, 2, 6, 0xffffffff]
    poll = [6, 3, 1, 2, 6, 3, 1, 2, 0, 0xffffffff]
    elements = {'Arrive': one, 'Enter': one, 'Go': poll, 'Booth': poll, 'Stop': [4, 4, 4, 1, 2, 3, 6, 0, 0xffffffff],
                'WalkIn': [3, 3, 3, 1, 6, 0]}.get(name, [1, 6])
    rides(r, regions, handles, elements)
    # (the balance positions: small, near one another)
    bx, by = r.randrange(2, 120), r.choice([r.randrange(2, 120), r.randrange(2, 120), 0, 1, 126, 127])
    pos = [(bx + r.randrange(-2, 3), by + r.randrange(-2, 3)) for _ in range(10)]
    regions[0x78559c] = struct.pack('<20i', *[v for p in pos for v in p])
    # (the visitor: on a position now and then (the bus stop's neighbours too), its handle word one of the park's)
    v = bytearray(r.bytes(0x240))
    me = r.choice(handles) if r.random() < 0.7 else r.getrandbits(16)
    put(v, 0, 'H', me)
    stop = cell_of(*pos[2])
    spots = {'Arrive': [cell_of(*pos[0]), cell_of(*pos[1])] * 3 + [cell_of(*p) for p in pos],
             'Go': [stop, stop + 1, stop - 0x100, stop - 0xff], 'Stop': [stop, stop + 1, stop - 0x100, stop - 0xff]}.get(name, [stop])
    if r.random() < 0.6:
        c = r.choice(spots) - 1
        v[5], v[7] = c & 0x7f, (c >> 7) & 0xff
    else:
        v[5], v[7] = r.randrange(128), r.randrange(128)
    own = ((v[7] << 7) + v[5] + 1) & 0xffff
    cell = bytearray(r.bytes(0x44))   # (the visitor's cell: its type, its mark word +0x24)
    put(cell, 8, 'I', r.choice([0, 1, 3, 9, 0xa, 2, 4, 0xb]))
    put(cell, 0x24, 'H', me if r.random() < 0.7 else r.getrandbits(16))
    regions[MAPC + (own - 1) * 0x44] = cell
    regions[0x8023a0] = struct.pack('<I', MAPC)
    put(v, 0x200, 'I', r.choice([0, 0, 1, 5]))
    put(v, 0x204, 'I', r.choice([0, 1, 0x1234]))
    put(v, 0x1dc, 'H', h())
    put(v, 0x1de, 'H', r.choice([0, r.choice(handles), r.choice(handles)]))
    put(v, 0x1bc, 'i', r.choice([0, 1, 5, -1, 100]))
    put(v, 0x19c, 'I', fb(r.choice([0.0, 0.5, 1.0, 50.0, 100.0, 256.0, 300.0, r.uniform(-10, 110)])))
    put(v, 0x210, 'I', r.choice([0, 0, 1, 2, 3, 4]))
    personscripts.script_list(r, regions, v, base=SCRIPTS)
    regions[0x75c7f0] = r.bytes(6)
    regions[VIS] = v
    # (the recorders' answers and fills)
    set_return(log, 1, r.choice([0, 0, 0, 2, 1, 3]))
    set_return(log, 2, r.choice([0, 1, 1]))
    set_return(log, 3, r.choice([0, 1, 1]))
    set_return(log, 7, r.choice([0xffffffff, 0, 1, 2, 5, 0x100, 0xfffffffe, 0x80000000]))
    strings(log, 6, 0, out=1)
    fill(log, 6, 0, struct.pack('<H', r.choice([0, r.getrandbits(16), r.getrandbits(16)])), which=0)
    strings(log, 8, 0, out=0xe)
    fill(log, 8, 1, struct.pack('<H', r.getrandbits(16)), which=1)
    fill(log, 8, 2, bytes([r.getrandbits(8)]), which=2)
    fill(log, 8, 3, bytes([r.getrandbits(8)]), which=3)
    if name == 'Emote':   # (the handle's low 4 bits 0 half the time)
        put(v, 0, 'H', r.choice([0x10, 0x20, 0x100, 0, me, me | 0x10]) & 0xffff)
        regions[0x803a24] = struct.pack('<I', r.getrandbits(32))
        set_return(log, 10, r.getrandbits(32))
        return Call(CALLS[name], [VIS, r.choice([4, 4, 4, 5, 5, 0, r.getrandbits(32)])], regions, conv='t')
    if name == 'Idle':
        idle(r, regions, log, v, handles, w)
    return Call(CALLS[name], [VIS], regions, conv='t', ret='a' if name == 'Queue' else 'v')


if __name__ == '__main__':
    B.main(case)
