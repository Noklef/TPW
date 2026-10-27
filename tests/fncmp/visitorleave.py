# A visitor leaving an object's queue (src/game/world/person: 0x500870, 0x5012f0) against the originals. 0x501db0 (agent C's, unported) is
# the thiscall recorder 26 (driver.cpp's kPersonEnv). Quit (0x500870): objectmsg.py's queue (Remove's environment: the object's head +0x3c,
# a ride record whose element 0 is a person or any), the object at the thing slot 13, the visitor one of the persons (its +0x1dc the object's
# handle 13 mostly, else another's). Away (0x5012f0): a visitor of random bytes, its handle word +0 low 3 bits 0 or not, the ring at +0x30
# (position 0-31, its last pair the one pushed now and then), happiness +0x19c about 0-100 and the bytes 0x785058 (Visitor_Lower19c); the
# sounds on or off (soundplay.py's globals, the manager's slot 2 a recorder of the command's 7 dwords), the sprite script list not ready.
# usage: visitorleave.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Quit,Away)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import objectmsg  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fb, log_region, put, set_return, vtable  # noqa: E402
from thingenv import POOL, THING  # noqa: E402

B = Bench([(0x500870, 0x5008f8), (0x5012f0, 0x50138d), (0x4dd4e0, 0x4dd510), (0x4ddd20, 0x4ddf50), (0x4e0a70, 0x4e0ab0), (0x4e2670, 0x4e2720),
           (0x4faa00, 0x4faa70), (0x4754e0, 0x475560), (0x4fea70, 0x4feb20), (0x50b350, 0x50b370), (0x50c0d0, 0x50c1a0), (0x519610, 0x519660),
           (0x51bfc0, 0x51c060), (0x55a070, 0x55a110), (0x55a390, 0x55a3f0), (0x5da3c0, 0x5da3c1)],
          hooks={0x501db0: _recorder(26, 1)})
CALLS = {'Quit': 0x500870, 'Away': 0x5012f0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
OBJ = objectmsg.OBJ
VISITOR, MANAGER, VT = RAM + 0x40000, RAM + 0x41000, RAM + 0x41100


def quit_case(r, log):
    c = objectmsg.queue(r, 'Remove')
    regions = dict(c.regions)
    slots = bytearray(regions[0x7cfb90]) + bytearray(r.bytes(20 * 4))
    put(slots, 20 * 13, 'IIIII', OBJ, 13, 0, 0, 0)
    regions[0x7cfb90] = slots
    pool = bytearray(regions[POOL])
    live = [i for i in range(len(pool) // THING)   # (the things in the park: their handle's slot names them)
            if struct.unpack_from('<I', slots, 20 * (struct.unpack_from('<H', pool, THING * i)[0] % 16))[0] == POOL + THING * i]
    i = r.choice(live)
    other = struct.unpack_from('<H', pool, THING * r.choice(live))[0]
    put(pool, THING * i + 0x1dc, 'H', 13 if r.random() < 0.9 else other)
    regions[POOL] = pool
    return Call(0x500870, [POOL + THING * i], regions, conv='t', ret='v')


def away_case(r, log):
    v = bytearray(r.bytes(0x240))
    put(v, 0, 'H', r.choice([8, 16, 0x18, 1, 3, 5, r.getrandbits(16)]))
    pos = r.randrange(32)
    put(v, 0x30 + 0x88, 'I', pos)
    obj = r.choice([1, 13, r.getrandbits(16)])
    put(v, 0x1dc, 'H', obj)
    if r.random() < 0.3:   # the last pair pushed already (6, the object)
        put(v, 0x30 + (pos * 4 if pos else 0x80), 'HH', 6, obj)
    put(v, 0x19c, 'I', fb(r.choice([0.0, 1.0, 50.0, 100.0, r.uniform(-10, 110)])))
    put(v, 0xc, 'I', r.choice([0, 1, 5]))
    g = bytearray(r.bytes(0x20))   # 0x802bc0.. (soundplay.py's)
    put(g, 8, 'I', r.choice([0, 1, 1, 1]))
    put(g, 0xc, 'I', MANAGER)
    put(g, 0x14, 'I', r.choice([0, 1, 1, 1]))
    blob(log, 2, 1, 0x7f)
    if r.random() < 0.5:
        set_return(log, 2, r.getrandbits(32))
    script = bytearray(r.bytes(0x2c))
    put(script, 0, 'Ii', 0, 0)
    put(script, 0x24, 'I', 0)
    return Call(0x5012f0, [VISITOR], {VISITOR: v, 0x802bc0: g, MANAGER: struct.pack('<I', VT) + r.bytes(0xc), VT: vtable({2: 2}, 4),
                                      0x803a24: r.bytes(4), 0x785058: bytes([r.choice([0, 1, 5, 10, 255])]) + r.bytes(3),
                                      0x7b49f0: script}, conv='t', ret='v')


def case(r):
    op = r.deal(OPS)
    log = log_region()
    c = quit_case(r, log) if op == 'Quit' else away_case(r, log)
    c.regions[LOG] = log
    return c


if __name__ == '__main__':
    B.main(case)
