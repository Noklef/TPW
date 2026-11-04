# A model's tree of parts added to a batch (src/game/render/batch: 0x57b610) against the original, the transformer's slot 6 (it writes part of the local matrix) and Model_AddTriangles recorders: trees of up to 3 levels and 3 parts a level,
# parts skipped by their flags (0x8210) or not gone into (0x20), no child, no first part, the triangles' call failing now and then.
# usage: modeltree.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fail_mask, fill, log_fold, log_region, put, strings, vtable  # noqa: E402

TRANSFORM, TRIANGLES = 6, 7
B = Bench([(0x57b610, 0x57b6df)], hooks={0x581a00: _recorder(TRIANGLES, 3)})
MODEL, MATRIX, TRANSFORMER, VT, NODES = RAM, RAM + 0x40, RAM + 0x80, RAM + 0xa0, RAM + 0x200
NODE = 0x20


def case(r):
    log = log_region()
    log_fold(log)
    blob(log, TRANSFORM, 2, 0xffff)
    strings(log, TRANSFORM, 0, 8)
    fill(log, TRANSFORM, 3, r.bytes(0x20))
    blob(log, TRIANGLES, 1, 0xffff)
    fail_mask(log, TRIANGLES, r.choice([0xffffffff, 0xffffffff, 0xffffffff, r.getrandbits(32), r.getrandbits(32) | 0xff]))
    nodes = bytearray(r.bytes(NODE * 40))
    used = 0

    def level(depth):
        nonlocal used
        first = 0
        for _ in range(r.choice([1, 1, 2, 3])):
            if used == 40:
                break
            at = NODE * used
            address = NODES + at
            used += 1
            flags = r.getrandbits(32) & ~0x8230
            flags |= r.choice([0, 0, 0, 0x10, 0x200, 0x8000]) | r.choice([0, 0, 0x20])
            put(nodes, at, 'I', flags)
            put(nodes, at + 8, 'I', first)
            child = level(depth - 1) if depth and r.random() < 0.6 else 0
            put(nodes, at + 0xc, 'I', child)
            first = address
        return first

    regions = {LOG: log, MODEL: put(bytearray(r.bytes(0x20)), 4, 'I', 0 if r.random() < 0.1 else level(2)), MATRIX: r.bytes(0x40), TRANSFORMER: struct.pack('<I', VT) + r.bytes(4), VT: vtable({}, 8, cdecl={TRANSFORM: 4}),
               NODES: nodes, 0x87b1d8: struct.pack('<I', TRANSFORMER)}
    return Call(0x57b610, [r.getrandbits(32), MODEL, MATRIX, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
