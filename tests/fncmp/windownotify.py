# A GUI window's notifications and its object +0x9c (src/game/ui/window) against the originals. Set4c / NotifyC00 / SetImpl (0x65c388,
# 0x65c428, 0x65d1c8) and Notify3000 / Notify20000 (0x65c4b9, 0x65c54a): window.py's notify_env tree with +0x4c look objects or none, the
# objects' events hashed. ImplRelease (0x6616b0): a recorder object, its words +0x10 / +0x12 small or edges. ImplSlot7 / ImplSlot6 / DwordA0
# (0x65d6db, 0x65d745, 0x65e705): +0x9c a recorder object or 0; +0xa0 sixteen dwords (an index reads before them too).
# usage: windownotify.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Set4c,NotifyC00,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, Bench, Call, log_region, put, set_return, vtable  # noqa: E402
from window import IMPL, LOOK, MISC, RANGES, VTABLE, WINDOW, notify_blobs, notify_env  # noqa: E402

CALLS = {'Set4c': 0x65c388, 'NotifyC00': 0x65c428, 'Notify3000': 0x65c4b9, 'Notify20000': 0x65c54a, 'SetImpl': 0x65d1c8,
         'ImplRelease': 0x6616b0, 'ImplSlot7': 0x65d6db, 'ImplSlot6': 0x65d745, 'DwordA0': 0x65e705}
B = Bench(RANGES)


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, VTABLE: vtable({5: 1})}
    if op in ('ImplSlot7', 'ImplSlot6', 'DwordA0'):   # +0x9c a recorder object or 0; +0xa0 sixteen dwords (an index reads before them too)
        win = bytearray(r.bytes(0x120))
        put(win, 0x9c, 'I', r.choice([MISC, MISC, 0]))
        regions[WINDOW] = win
        regions[MISC] = struct.pack('<I', VTABLE)
        for k in (6, 7):
            if r.random() < 0.5:
                set_return(log, k, r.choice([0, 1, r.getrandbits(32)]))
        regions[VTABLE] = vtable({7: 2})
        if op == 'ImplSlot7':
            return Call(CALLS[op], [WINDOW, r.getrandbits(32), r.choice([0, 1, r.getrandbits(32)])], regions, conv='t', ret='a')
        if op == 'ImplSlot6':
            return Call(CALLS[op], [WINDOW], regions, conv='t', ret='a')
        index = r.choice([0, 1, 7, 15, 16, 17, 0x7fff, 0xffff, 0xffe8, 0x10005, 0xfffe000f, r.getrandbits(32) & 0xffff000f, r.getrandbits(32)])
        return Call(CALLS[op], [WINDOW, index], regions, conv='t', ret='a')
    if op == 'ImplRelease':   # the object: a recorder vtable (slot 5 takes one argument), its words +0x10 and +0x12 small or edges
        impl = bytearray(r.bytes(0x20))
        put(impl, 0, 'I', VTABLE)
        for off in (0x10, 0x12):
            put(impl, off, 'H', r.choice([0, 1, 2, 3, 0x7fff, 0x8000, 0xffff, r.getrandbits(16)]))
        regions[MISC] = impl
        return Call(CALLS[op], [MISC], regions, conv='t', ret='a')
    wins = notify_env(r, regions, log)   # see notify_env()
    window = r.choice(wins)
    if op == 'Set4c':
        notify_blobs(log, 0, 1 << 8)
        return Call(CALLS[op], [window, r.choice([LOOK, LOOK + 0x40, 0])], regions, conv='t', ret='v')
    if op in ('NotifyC00', 'Notify3000', 'Notify20000'):   # (the bytes +4 / +5, +6 / +7 or +9 of the second event)
        notify_blobs(log, 0, {'NotifyC00': 0x30, 'Notify3000': 0xc0, 'Notify20000': 0x200}[op])
        args = [r.getrandbits(32) for _ in range(1 if op == 'Notify20000' else 2)]
        return Call(CALLS[op], [window] + args, regions, conv='t', ret='v')
    look = struct.unpack_from('<I', regions[window], 0x4c)[0]
    notify_blobs(log, 0x0ff0f3f0, 1 << 8 if look else 0)
    return Call(CALLS[op], [window, r.choice([IMPL, IMPL, MISC + 0x40, 0])], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
