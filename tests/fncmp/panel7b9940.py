# The 0x7b9940 object's detach (src/game/ui/panel7b9940: 0x48a3d0, and the jump 0x481410 to it) against the original: the GUI manager
# [0x77c48c] with a recorder vtable (slot 4 logged), the object's vtable giving its virtual base an offset of 0, 8 or 0x20, the renderer
# [0x87a6d0] whose tag 0x20 list (+0x18) holds 0-4 nodes keyed by that base or not, its free list and count, the scene flags 0x8bd508.
# Ctor (0x4855d0): a random object, complete (the real vbtables) or not (its +0 the real vbtable or one with other offsets, the
# second base's now and then over the first's fields), the scene flags and the device generation 0x8bcc40.
# Close (0x48a290, and the jump 0x4813f0 to it): the same, with the ten 0x7c26b0 records, the released globals, the dialog [0x7ca098]
# and the windows [0x7cb2ac] / [0x7cb2b0] each 0, a window (recorder vtable, closed or not, fncmp's logger the handler) or an object;
# the message hook [0xfaa5fc] set or not. Show (0x48a310): the second base 0x10-0x40 in, the GUI's two queues (two entries) set or
# not, the timed table empty or none, the byte 0x78d90e 0 or set (the manager's slot 3 or 4).
# usage: panel7b9940.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Detach,Jump,Ctor,Close,CloseJump,Show)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CDECL_LOG4, LOG, RAM, Bench, Call, log_region, put, set_return, vtable  # noqa: E402

CALLS = {'Detach': 0x48a3d0, 'Jump': 0x481410, 'Ctor': 0x4855d0, 'Close': 0x48a290, 'CloseJump': 0x4813f0, 'Show': 0x48a310}
B = Bench([(0x481410, 0x481415), (0x48a3d0, 0x48a40a), (0x659607, 0x65961c), (0x576540, 0x5765d0), (0x4855d0, 0x4856bf),
           (0x5836b0, 0x5836c2), (0x4813f0, 0x4813f5), (0x48a290, 0x48a310), (0x48ee20, 0x48ee4e), (0x498b90, 0x498ba2),
           (0x4b3f20, 0x4b3f54), (0x497b60, 0x497b72), (0x48d990, 0x48d9b3), (0x4987d0, 0x4987e2), (0x658d9f, 0x658df4),
           (0x659710, 0x659721), (0x65f697, 0x65f6d1), (0x48eaf0, 0x48eb20), (0x5da3c0, 0x5da3c1),
           (0x48a310, 0x48a3c3), (0x659064, 0x6590a5), (0x669eb3, 0x669efb), (0x669c62, 0x669c8d), (0x669bdb, 0x669c30),
           (0x67fff0, 0x680048), (0x576410, 0x576550), (0x65961c, 0x659631), (0x576270, 0x576320)])
MANAGER, VTABLE, TABLE, RENDERER, NODES, SELF = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500
WINDOW, WINDOW2, THING = RAM + 0x600, RAM + 0x800, RAM + 0xa00
OBJECT = 0x7b9940


def ctor(r):
    self = bytearray(r.bytes(0x100))
    first, second = r.choice([(8, 0xc), (8, 0xc), (0x10, 0x20), (0x20, 0x10), (0x40, 0x40), (8, 0)])
    put(self, 0, 'I', r.choice([0x6fdd00, TABLE]))
    regions = {SELF: self, TABLE: struct.pack('<III', r.getrandbits(32), first, second), 0x8bd508: r.bytes(8),
               0x8bcc40: r.bytes(4)}
    return Call(CALLS['Ctor'], [SELF, r.choice([0, 1, 1, 0x100, r.getrandbits(32)])], regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS)
    if op == 'Ctor':
        return ctor(r)
    log = log_region()
    if r.random() < 0.5:
        set_return(log, 4, r.getrandbits(32))
    d = r.choice([0, 8, 0x20])
    base = OBJECT + d
    renderer = bytearray(r.bytes(0x40))
    nodes = bytearray(r.bytes(0x40))
    head = 0
    for i in range(r.choice([0, 1, 2, 3, 4])):
        put(nodes, 8 * i, 'II', r.choice([base, base, OBJECT, r.getrandbits(32)]), head)
        head = NODES + 8 * i
    put(renderer, 0x18, 'I', head)
    put(renderer, 4, 'I', r.choice([0, NODES + 0x30]))
    put(renderer, 0xc, 'I', r.randrange(8))
    regions = {LOG: log, VTABLE: vtable({}), MANAGER: put(bytearray(r.bytes(0x10)), 0, 'I', VTABLE),
               0x77c48c: struct.pack('<II', MANAGER, MANAGER), TABLE: struct.pack('<II', r.getrandbits(32), d),
               OBJECT: put(bytearray(r.bytes(0x30)), 0, 'I', TABLE), RENDERER: renderer, NODES: nodes,
               0x87a6d0: struct.pack('<I', RENDERER), 0x8bd508: r.bytes(8)}
    if op == 'Show':
        # the object's two vbtable offsets (the first base's vtable the recorder), the renderer's free nodes and its tag 0x20 list (of
        # recorder objects or the base); the two queues (2 entries each) and an empty timed table, or none
        d, second = r.choice([(8, 0x10), (8, 0x20), (8, 0x40), (0x20, 0x40)])
        regions[TABLE] = struct.pack('<III', r.getrandbits(32), d, second)
        obj = put(bytearray(r.bytes(0x100)), 0, 'I', TABLE)
        regions[OBJECT] = put(obj, d, 'I', VTABLE)
        regions[THING] = put(put(bytearray(0x20), 0, 'I', VTABLE), 0x10, 'I', VTABLE)
        head = free = 0
        for i in range(r.choice([0, 1, 2, 3])):
            put(nodes, 8 * i, 'II', r.choice([THING, THING + 0x10, OBJECT + d]), head)
            head = NODES + 8 * i
        for i in range(r.choice([0, 1, 1, 2])):
            put(nodes, 0x20 + 8 * i, 'II', r.getrandbits(32), free)
            free = NODES + 0x20 + 8 * i
        put(renderer, 0x18, 'I', head)
        put(renderer, 4, 'I', free)
        if r.random() < 0.5:
            set_return(log, 2, r.choice([0, 1, 1, 2]))
        for i, g in enumerate((0xfaa5f4, 0xfaa5f0)):
            q = THING + 0x100 + 0x100 * i
            regions[q] = struct.pack('<5I', q + 0x40, 2, 0, r.randrange(2), r.getrandbits(8))
            regions[q + 0x40] = r.bytes(0x20)
            regions[g] = struct.pack('<I', r.choice([q, q, 0]))
        regions[WINDOW] = struct.pack('<II', 0, WINDOW + 0x20)
        regions[0xfaa5f8] = struct.pack('<I', r.choice([WINDOW, 0]))
        regions[0x78d90e] = bytes([r.choice([0, 1, 0x80])])
        for k in (3, 4):
            if r.random() < 0.5:
                set_return(log, k, r.getrandbits(32))
        return Call(CALLS[op], [], regions, ret='a')
    if op in ('Close', 'CloseJump'):   # the windows (+0 the recorder vtable, byte +4 the closed flag, +0x114 fncmp's logger) and the objects
        regions[VTABLE] = vtable({2: 1})
        for w in (WINDOW, WINDOW2):
            win = bytearray(r.bytes(0x120))
            put(win, 0, 'I', VTABLE)
            win[4] = r.choice([0, 0, 1])
            put(win, 0x114, 'I', CDECL_LOG4)
            regions[w] = win
        regions[THING] = put(bytearray(r.bytes(0x10)), 0, 'I', VTABLE)
        some = [0, WINDOW, WINDOW2, THING]
        for i in range(10):
            regions[0x7c26b0 + 0x208 * i] = struct.pack('<IH', r.choice(some), r.choice([0, 0, 1, 0x100]))
        for g in (0x7ca1e0, 0x7cc2fc, 0x7cc300, 0x7cc304, 0x7ca1bc, 0x7ca1c8, 0x7ca098, 0x7ca090, 0x7cb2ac, 0x7cb2b0):
            regions[g] = struct.pack('<I', r.choice(some + [0]))
        regions[0xfaa5fc] = struct.pack('<I', r.choice([0, CDECL_LOG4]))
        if r.random() < 0.5:
            set_return(log, 0x3d, r.getrandbits(32))
        return Call(CALLS[op], [], regions, ret='v')
    return Call(CALLS[op], [], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
