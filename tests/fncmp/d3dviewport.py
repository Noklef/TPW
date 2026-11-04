# The Direct3D setup's viewport (src/game/render/d3dviewport: 0x572dd0) against the original: four COM recorders, each with results from a list (0 mostly; a failure runs the engine debug log's and the error list's real code, the log's C library calls
# recorders as in enginedebug.py), the new viewport given by the first call's fill (or left as it was), the viewport structure logged by its contents, the screen's size any.
# usage: d3dviewport.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import LOG, RAM, Bench, Call, blob, fill, log_region, put, recorder, set_return, vtable  # noqa: E402
import enginedebug  # noqa: E402

B = Bench([(0x572dd0, 0x572f64), (0x56a770, 0x56ae60), (0x56af20, 0x56af9c), enginedebug.RANGE, enginedebug.FALLBACK, (0x67a400, 0x690000)], hooks=enginedebug.HOOKS)
SELF, D3D, D3DVT, DEV, DEVVT, VIEW, VIEWVT = RAM, RAM + 0x40, RAM + 0x80, RAM + 0x200, RAM + 0x280, RAM + 0x400, RAM + 0x480
CREATE, ADD, SETVIEW, CURRENT = 6, 5, 17, 20   # (not the debug log's 11-14)


def hresult(r):
    return r.choice([0, 0, 0, 0, 0, 0, 1, 0x88760000 + r.randrange(0x300), 0x80004005, 0x80070057, 0xb0754002, r.getrandbits(32)])


def case(r):
    log = log_region()
    enginedebug.texts(log)
    if r.random() < 0.3:
        set_return(log, enginedebug.FOPEN, 0)
    for k in (CREATE, ADD, SETVIEW, CURRENT):
        set_return(log, k, hresult(r))
    me = bytearray(r.bytes(0x20))
    put(me, 4, 'I', D3D)
    put(me, 8, 'I', DEV)
    if r.random() < 0.8:
        put(me, 0xc, 'I', r.getrandbits(32))
        fill(log, CREATE, 1, struct.pack('<I', VIEW))
    else:
        put(me, 0xc, 'I', VIEW)
    blob(log, SETVIEW, 1, 0x7ff)
    devvt = vtable({}, size=16, com={ADD: 2})
    put(devvt, 12 * 4, 'I', recorder(CURRENT, 2, True))
    errors = bytearray(r.bytes(0x204))
    put(errors, 0x200, 'I', r.choice([0, 1, 0x3f, 0x40, r.randrange(0x40)]))
    size = r.choice([(640, 480), (800, 600), (1024, 768), (0, 0), (0x1000001, 0x7fffffff), (-1, -640), (r.getrandbits(32), r.getrandbits(32))])
    regions = {LOG: log, SELF: me, D3D: bytearray(struct.pack('<I', D3DVT) + r.bytes(0x10)), D3DVT: vtable({}, size=16, com={CREATE: 3}),
               DEV: bytearray(struct.pack('<I', DEVVT) + r.bytes(0x10)), DEVVT: devvt, VIEW: bytearray(struct.pack('<I', VIEWVT) + r.bytes(0x10)), VIEWVT: vtable({}, size=24, com={SETVIEW: 2}),
               0x8bcbd0: struct.pack('<ii', *[v - (1 << 32) if v >= 1 << 31 else v for v in size]), 0x879cd4: r.bytes(4), 0x879a2c: bytearray(struct.pack('<I', r.choice([0, 1]))),
               0x879ad0: errors, 0x8bd508: bytearray(4)}
    return Call(0x572dd0, [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
