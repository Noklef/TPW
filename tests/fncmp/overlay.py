# The overlay base (src/game/gfx/overlay: 0x476e80) against the original: the shown node picked
# by base + frame from a model's node list (short, ending early, or empty), with flags taking
# either from a descriptor or neither, and the old node apart, null, among the list or the
# overlay itself.
# usage: overlay.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x476e80, 0x476f04)])
OV, DESC, HOLDER, MODEL, NODES = (RAM + k * 0x1000 for k in range(5))


def case(r):
    count = r.choice([0, 1, r.randrange(2, 12)])
    nodes = bytearray(r.bytes(12 * 0x10))
    for i in range(count):
        put(nodes, i * 0x10, 'I', r.choice([r.getrandbits(32), 0, 0x8010, 0xffffffff]))
        put(nodes, i * 0x10 + 0xc, 'I', NODES + (i + 1) * 0x10 if i + 1 < count else 0)
    model = put(bytearray(r.bytes(0x80)), 0x78, 'I', NODES if count else 0)
    holder = put(bytearray(r.bytes(0x10)), 4, 'I', MODEL)
    ov = bytearray(r.bytes(0x80))
    put(ov, 4, 'I', HOLDER)
    small = lambda: r.choice([r.randrange(4), r.randrange(count + 2), r.getrandbits(16)])  # noqa: E731
    put(ov, 0xa, '2H', small(), small())
    old = r.choice([0, NODES + 0x10 * r.randrange(max(count, 1)), OV, RAM + 0x5000])
    put(ov, 0x7c, 'I', old)
    desc = bytearray(r.bytes(0x10))
    put(desc, 4, 'H', small())
    put(desc, 0xa, 'H', small())
    flags = r.choice([0, 1, 0x80000, 0x80001, r.getrandbits(32), r.getrandbits(32) & ~0x80001])
    regs = {OV: ov, DESC: desc, HOLDER: holder, MODEL: model, NODES: nodes, RAM + 0x5000: r.bytes(4)}
    return Call(0x476e80, [OV, flags, DESC], regs, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
