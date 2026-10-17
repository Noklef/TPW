# The holder functions of src/game/model/holder that go through the renderer (0x45bdd0 Holder_Sleep, 0x4254c0 Animator_Sleep, 0x45bc60 Holder_Destroy) against the originals: a
# holder whose part sits in a renderer's slot lists (tags 0x10, 0x20, 0x70) or not; an animator holding such a holder; and a holder whose part has a vtable (a recorder) giving the
# virtual base offset; a user of the holder behind [0x79fcb4] taking it out by its rectangle (misc.py's grid) or by sleeping.
# usage: holder.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Sleep,Animator,Destroy,Release)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import misc  # noqa: E402
from fncmp import Bench, Call, LOG, RAM, log_region, put, vtable  # noqa: E402

B = Bench([(0x4254c0, 0x4254f0), (0x45bc60, 0x45bcb0), (0x45bdd0, 0x45be90), (0x45be80, 0x45bed0), (0x576540, 0x5765d0), (0x5766e0, 0x57681a), (0x5d8820, 0x5d8870), (0x5da3c0, 0x5da3c1)])
ANIM, OWNER, TARGET, USER = RAM + 0x2000, RAM + 0x2500, RAM + 0x3000, RAM + 0x3200


def sleep(r):
    obj = bytearray(r.bytes(0x400))
    put(obj, 0, 'I', r.getrandbits(32))
    put(obj, 0x1c, 'I', r.choice([0, RAM + 0x100, RAM + 0x100]))
    put(obj, 0x100, 'I', RAM + 0x300)
    offset = r.choice([0, 0x10, 0x20])
    put(obj, 0x304, 'I', offset)
    renderer = bytearray(r.bytes(0x100))
    part = RAM + 0x100 + offset
    nodes = bytearray(0x40)
    for k, slot in enumerate((0x14, 0x18, 0x2c)):
        put(nodes, 8 * k, 'II', part if r.random() < 0.6 else r.getrandbits(32), 0)
        put(renderer, slot, 'I', RAM + 0x1400 + 8 * k if r.random() < 0.8 else 0)
    put(renderer, 4, 'I', 0)
    put(renderer, 0xc, 'I', 5)
    regions = {0x87a6d0: put(bytearray(r.bytes(0x10)), 0, 'I', RAM + 0x1000), RAM: obj, RAM + 0x1000: renderer, RAM + 0x1400: nodes, 0x8bd500: r.bytes(0x20)}
    return Call(0x45bdd0, [RAM], regions, conv='c', ret='v')


def animator(r):
    c = sleep(r)
    c.regions[ANIM] = put(put(bytearray(r.bytes(0x40)), 0, 'I', r.getrandbits(32)), 0x24, 'I', RAM)
    return Call(0x4254c0, [ANIM], c.regions, conv='c', ret='a')


def destroy(r):
    c = sleep(r)
    obj = c.regions[RAM]
    offset = struct.unpack_from('<I', obj, 0x304)[0]
    vt = vtable({0: 1})
    put(vt, 4, 'I', offset)
    obj[0x300:0x300 + len(vt)] = vt
    put(obj, 0x100 + offset, 'I', RAM + 0x300)
    put(obj, 0x8, 'I', r.choice([0, OWNER]))
    c.regions[OWNER] = r.bytes(0x40)
    c.regions[LOG] = log_region()
    return Call(0x45bc60, [RAM], c.regions, conv='c', ret='v')


def release(r):
    if r.random() < 0.5:   # the rectangle path: misc.py's renderer grid
        misc.OPS = ['RemRect']
        c = misc.case(r)
        regions, lo, hi, flags = dict(c.regions), c.args[1], c.args[2], 0x4000 | r.getrandbits(32)
    else:                  # the sleep path: a holder in the renderer's slot lists
        c = sleep(r)
        regions, lo, hi, flags = dict(c.regions), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32) & ~0x4000
    regions[TARGET] = put(put(put(put(bytearray(r.bytes(0x100)), 4, 'I', flags), 8, 'I', RAM), 0xc0, 'I', lo), 0xc4, 'I', hi)
    regions[USER] = put(bytearray(r.bytes(0x20)), 4, 'I', r.choice([0, 1, 5, 0xffffffff]))
    regions[0x79fcb4] = struct.pack('<I', TARGET)
    return Call(0x5d8820, [USER], regions, conv='t', ret='v')


CASES = {'Sleep': sleep, 'Animator': animator, 'Destroy': destroy, 'Release': release}


def case(r):
    return CASES[r.choice(OPS)](r)


OPS = [k for k in os.environ.get('OPS', ','.join(CASES)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
