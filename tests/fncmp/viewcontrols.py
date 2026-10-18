# The view objects of src/game/frontend/viewcontrols against the originals: an object of random bytes, its step (+0x14) and values (+0x48,
# +0x50) now and then ordinary floats. UseSecond / UseFirst: the mode +0x58 random, the dialog [0x7cdb0c] a window whose children (0-2 others)
# now and then hold 0x16007, each child's handler (+0x114) fncmp's four-argument cdecl logger or 0, the hook [0xfaa5fc] the logger or 0.
# The dialog shows and hides (Show70 / Show20 / Show8 / Hide8 / Callback / Done): the dialogs [0x7cdb0c] (with its children), [0x7cc4c4]
# and [0x7cc47c] each a window (byte +4 now and then set, handler the logger or 0) or none, the logger's result now and then set. Done:
# accepted 0 or not; the object's vtable recorders (slot 0x11's result now and then set), its state +0x14 0-2 or random, its +0xc a holder
# whose +8 is a lobby model with no model (+4 0; a holder of 0 now and then: a fault in both when the state is 2). Choose: the mode +0x58
# 0, 1 or 7. Positive40 / Pending (slot 16s): +0x40 about 0 and the signed limits, Pending's +0x60 0 or not and +0x64 on either side of
# +0x40. SetOrKeep (slot 19): a value of 0 or not.
# usage: viewcontrols.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Lower48,Raise48,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CDECL_LOG4, LOG, Bench, Call, RAM, log_region, put, set_return, vtable  # noqa: E402

CALLS = {'Lower48': 0x5dd310, 'Raise48': 0x5dd320, 'Raise50': 0x5dd330, 'Lower50': 0x5dd340, 'UseSecond': 0x5dd350, 'UseFirst': 0x5dd390,
         'Show70': 0x5dd7a0, 'Show20': 0x5dda70, 'Show8': 0x5e2620, 'Hide8': 0x5e2640, 'Callback': 0x5e2650, 'Done': 0x5e2660,
         'Choose': 0x5dd3d0, 'Positive40': 0x5dd810, 'Pending': 0x5ddac0, 'SetOrKeep': 0x5e2750}
B = Bench([(0x5dd310, 0x5dd3fd), (0x4c57e0, 0x4c5830), (0x658d9f, 0x658e04), (0x659710, 0x659740), (0x65d7bc, 0x65d832), (0x65f697, 0x65f6e1),
           (0x5dd7a0, 0x5dd7c0), (0x5dda70, 0x5dda90), (0x5e2620, 0x5e26a0), (0x4b8810, 0x4b8850), (0x4b9c20, 0x4b9c40), (0x4c57a0, 0x4c57c0),
           (0x5d83f0, 0x5d8431), (0x5dd810, 0x5dd822), (0x5ddac0, 0x5ddae0), (0x5e2750, 0x5e2763)])
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
WINDOW, KIDS, WIN2, WIN3, HOLDER, VTABLE = RAM + 0x1000, RAM + 0x1200, RAM + 0x1800, RAM + 0x1a00, RAM + 0x2000, RAM + 0x2200
DIALOGS = {'Show70': (0x7cdb0c, 0x7cc4c4), 'Show20': (0x7cdb0c, 0x7cc4c4)}


def window(r):
    win = bytearray(r.bytes(0x120))
    win[4] = r.choice([0, 0, 0, 1])
    put(win, 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, CDECL_LOG4, 0]))
    return win


def case(r):
    op = r.deal(OPS)
    obj = bytearray(r.bytes(0x100))
    regions = {RAM: obj}
    for off in (0x14, 0x48, 0x50):
        if r.random() < 0.7:
            put(obj, off, 'I', r.f())
    if op == 'Choose':
        put(obj, 0x58, 'I', r.choice([0, 0, 1, 7]))
        return Call(CALLS[op], [RAM], regions, conv='t')
    if op == 'Positive40':
        put(obj, 0x40, 'I', r.choice([0, 1, 0xffffffff, 0x80000000, 0x7fffffff, r.getrandbits(32)]))
        return Call(CALLS[op], [RAM], regions, conv='s', ret='a')
    if op == 'Pending':
        put(obj, 0x60, 'I', r.choice([0, 0, 1]))
        put(obj, 0x40, 'i', r.choice([0, 1, 5, -1, -5, 0x7fffffff, -0x80000000]))
        put(obj, 0x64, 'i', r.choice([0, 1, 5, -1, 6, 0x7fffffff, -0x80000000]))
        return Call(CALLS[op], [RAM], regions, conv='s', ret='a')
    if op == 'SetOrKeep':
        return Call(CALLS[op], [RAM, r.choice([0, 0, 1, r.getrandbits(32)])], regions, conv='t')
    if op.startswith('Use') or op == 'Show70':
        put(obj, 0x58, 'I', r.choice([0, 1, 2, r.getrandbits(32)]))
        regions[LOG] = log = log_region()
        win = window(r) if op == 'Show70' else bytearray(r.bytes(0x90))
        ids = [r.choice([0x16006, 0x16008, 1]) for _ in range(r.choice([0, 1, 2]))]
        if r.random() < 0.85:
            ids.insert(r.randrange(len(ids) + 1), 0x16007)
        for i, ident in enumerate(ids):
            kid = bytearray(r.bytes(0x120))
            kid[4] = r.choice([0, 0, 0, 1])
            put(kid, 0x54, 'I', KIDS + 0x120 * (i + 1) if i + 1 < len(ids) else 0)
            put(kid, 0x8c, 'I', ident)
            put(kid, 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
            regions[KIDS + 0x120 * i] = kid
        put(win, 0x5c, 'I', KIDS if ids else 0)
        regions[WINDOW] = win
        regions[0x7cdb0c] = struct.pack('<I', WINDOW)
        regions[0xfaa5fc] = struct.pack('<I', r.choice([0, 0, CDECL_LOG4]))
        if op == 'Show70':
            regions[WIN2] = window(r)
            regions[0x7cc4c4] = struct.pack('<I', r.choice([WIN2, WIN2, WIN2, 0]))
            if r.random() < 0.5:
                set_return(log, 0x3d, r.choice([0, 1, -1, r.getrandbits(32)]))
            return Call(CALLS[op], [RAM], regions, conv='t', ret='a')
    elif op in ('Show20', 'Show8', 'Hide8', 'Callback', 'Done'):
        regions[LOG] = log = log_region()
        if r.random() < 0.5:
            set_return(log, 0x3d, r.choice([0, 1, -1, r.getrandbits(32)]))
        regions[0xfaa5fc] = struct.pack('<I', r.choice([0, 0, CDECL_LOG4]))
        for at, win in ((0x7cdb0c, WINDOW), (0x7cc4c4, WIN2), (0x7cc47c, WIN3)):
            regions[win] = window(r)
            regions[at] = struct.pack('<I', r.choice([win, win, win, 0]))
        if op == 'Callback':
            return Call(CALLS[op], [], regions, ret='a')
        if op == 'Done':
            put(obj, 0, 'I', VTABLE)
            regions[VTABLE] = vtable({})
            if r.random() < 0.5:
                set_return(log, 0x11, r.getrandbits(32))
            put(obj, 0x14, 'I', r.choice([0, 1, 2, 2, r.getrandbits(32)]))
            put(obj, 0xc, 'I', r.choice([HOLDER] * 8 + [0]))
            holder = bytearray(r.bytes(0x100))
            put(holder, 8, 'I', HOLDER + 0x80)
            put(holder, 0x84, 'I', 0)
            regions[HOLDER] = holder
            return Call(CALLS[op], [r.choice([0, 0, 1, r.getrandbits(32)]), RAM], regions, ret='a')
        return Call(CALLS[op], [RAM], regions, conv='t', ret='a')
    return Call(CALLS[op], [RAM], regions, conv='t')


if __name__ == '__main__':
    B.main(case)
