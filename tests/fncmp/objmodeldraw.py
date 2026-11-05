# An object's model drawn or queued (src/game/model/objmodel: 0x57a750 ObjModel_DrawOrQueue) against the original: recorders for ObjModel_DrawMeshTree (k 1; its first matrix hashed), Object_UpdateShadow (k 2), TexCache_TouchItem (k 3), MeshQueue2_Add (k 4), the transformer's slot 6
# (cdecl, k 6; its input matrix hashed, its result filled in) and the object's +0x140 (cdecl, k 7). All the globals read vary: the stamps, the queue's count, the mode, the render flags, the two matrices' pointer, the guard of the static matrix.
# usage: objmodeldraw.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _cdecl_log, _recorder, blob, fill, log_region, put, recorder, set_return, vtable  # noqa: E402

B = Bench([(0x57a750, 0x57aa01)], hooks={0x57a530: _recorder(1, 3), 0x57b8a0: _recorder(2, 0), 0x58c9f0: _recorder(3, 2), 0x57c270: _recorder(4, 1), 0x67a7d0: _cdecl_log})
OBJECT, MODEL, SHADOW, XF, XFVT, MATRIX, CACHE = RAM, RAM + 0x200, RAM + 0x300, RAM + 0x340, RAM + 0x400, RAM + 0x500, RAM + 0x600


def chance(r, bit, p=0.5):
    return bit if r.random() < p else 0


def case(r):
    log = log_region()
    set_return(log, 1, r.choice([0, 1, r.getrandbits(32)]))
    set_return(log, 2, r.choice([0, 1, 1, r.getrandbits(32)]))
    blob(log, 1, 1, 0xffff)
    blob(log, 6, 2, 0xffff)
    fill(log, 6, 3, r.bytes(0x40))

    stamps = r.bytes(8)
    obj = bytearray(r.bytes(0x160))
    put(obj, 4, 'I', r.getrandbits(32) & ~0x15 | chance(r, 1) | chance(r, 4, 0.2) | chance(r, 0x10))
    put(obj, 8, 'I', MODEL)
    obj[0x90:0x98] = r.choice([stamps, stamps, stamps[:4] + r.bytes(4), r.bytes(4) + stamps[4:], r.bytes(8)]) if r.random() < 0.5 else r.bytes(8)
    put(obj, 0x13c, 'I', r.choice([0, SHADOW, SHADOW]))
    put(obj, 0x140, 'I', r.choice([0, recorder(7, 1, cdecl=True)]))

    model = bytearray(r.bytes(0x80))
    flags = r.getrandbits(32) & ~0xe00a02
    flags |= chance(r, 0x200000, 0.3) | chance(r, 0x400000) | chance(r, 0x800000) | chance(r, 0x200, 0.3) | chance(r, 2, 0.3) | chance(r, 0x800)
    put(model, 0x30, 'I', flags)

    queue = bytearray(r.bytes(0x408))
    put(queue, 0x400, 'I', r.choice([0, 1, 5, 0xff, 0x100, 0x200]))
    render = r.getrandbits(32) & ~0x200800 | chance(r, 0x800) | chance(r, 0x200000, 0.7)
    regions = {LOG: log, OBJECT: obj, MODEL: model, SHADOW: r.bytes(0x20), XF: struct.pack('<I', XFVT), XFVT: vtable({}, cdecl={6: 4, 7: 1}), MATRIX: r.bytes(0x40), CACHE: r.bytes(4),
               0x8bcc40: stamps, 0x8bb220: queue, 0x8bd524: struct.pack('<I', r.choice([4, 4, 1, r.getrandbits(32)])), 0x8bcbc8: struct.pack('<I', render),
               0x87b088: struct.pack('<I', MATRIX), 0x87b1d8: struct.pack('<I', XF), 0x8c4b44: struct.pack('<I', CACHE),
               0xfb33e0: bytes([r.getrandbits(8)]), 0xfb3400: r.bytes(0x40)}
    return Call(0x57a750, [OBJECT + 0x154], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
