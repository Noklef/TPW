# The help background (src/game/frontend/helpbg: 0x4922d0, 0x491fe0) against the original. Clear: any object bytes. Handler: the window's
# default handler +0x118 fncmp's logger (its result now and then set), its object A0[0] a recorder (slot 15), the message 0x14 often.
# SetInterval / FrameSetInterval (0x492160, src/game/ui/frame's 0x66a084): the words +4 / +0xc set or not, slot 0 and the clock recorders. (Its HelpBg_Refresh, 0x492320, is
# benched with the virtual call forwarders in vcall_forwards.py.)
# usage: helpbg.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Clear,Handler,SetInterval,FrameSetInterval)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CDECL_LOG4, LOG, Bench, Call, RAM, log_region, put, set_return, vtable  # noqa: E402

CALLS = {'Clear': 0x4922d0, 'Handler': 0x491fe0, 'SetInterval': 0x492160, 'FrameSetInterval': 0x66a084}
B = Bench([(0x4922d0, 0x4922e0), (0x491fe0, 0x492014), (0x65e705, 0x65e73d), (0x65f66e, 0x65f697), (0x492160, 0x49217c), (0x66a084, 0x66a0b9),
           (0x65968e, 0x6596a2)])
WINDOW, OBJECT, VTABLE = RAM + 0x100, RAM + 0x300, RAM + 0x400


def case(r):
    op = r.deal(OPS)
    if op in ('SetInterval', 'FrameSetInterval'):   # the words +4 / +0xc 0 or not (upper bytes too), slot 0 and the clock [0x77c490] recorders
        log = log_region()
        obj = bytearray(r.bytes(0x20))
        put(obj, 0, 'I', VTABLE)
        put(obj, 4, 'H', r.choice([0, 1, 0x100, 0x8000, r.getrandbits(16)]))
        put(obj, 0xc, 'H', r.choice([0, 0, 1, 0x100, 0x8000, r.getrandbits(16)]))
        if r.random() < 0.7:
            set_return(log, 0, r.getrandbits(32))
        regions = {LOG: log, OBJECT: obj, VTABLE: vtable({}), 0x77c490: struct.pack('<I', OBJECT + 0x40), OBJECT + 0x40: struct.pack('<I', VTABLE)}
        return Call(CALLS[op], [OBJECT, r.getrandbits(32)], regions, conv='t', ret='v')
    if op == 'Handler':
        log = log_region()
        win = bytearray(r.bytes(0x120))
        put(win, 0x118, 'I', CDECL_LOG4)
        put(win, 0xa0, 'I', OBJECT)
        if r.random() < 0.5:
            set_return(log, 0x3d, r.choice([0, 1, -1, r.getrandbits(32)]))
        if r.random() < 0.5:
            set_return(log, 15, r.getrandbits(32))
        regions = {LOG: log, WINDOW: win, OBJECT: struct.pack('<I', VTABLE) + r.bytes(0x1c), VTABLE: vtable({})}
        args = [r.choice([0x14, 0x14, 0x15, 0, 4, r.getrandbits(32)])] + [r.choice([0, 1, 4, r.getrandbits(32)]) for _ in range(2)]
        return Call(CALLS[op], [WINDOW] + args, regions, ret='a')
    return Call(CALLS[op], [RAM + 0x20], {RAM: r.bytes(0x60)}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
