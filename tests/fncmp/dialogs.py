# The dialog wrappers (src/game/ui/dialogs, frontend/viewcontrols) against the originals. The sends to a dialog's global (Dialog7cc47c_Message4
# and the like): the window (or none) with its byte +4 now and then set, its handler +0x114 and the hook [0xfaa5fc] fncmp's four-argument
# cdecl logger or 0 (its result now and then set); the Child ones send to the dialog's child (from 0-3 others, now and then missing).
# WindowAt10_* : a holder with the window at +0x10 (ScalarDtor frees it for flags bit 0). Windows7c26b0_Message5: ten entries of 0x208.
# Windows7cc8bc_Message6Pair / Global7cc698_Choose: the pair [0x7cc8bc] / [0x7cc8c0] and 0x7cc698, 0x7cc8b8 / 0x7cc8b4.
# Dialog7cb2b8_Child21Post (0x4a2590): the dialog or none, its child 0x21 or none among others, the child's dword +0xa8 set or not, the
# value 0 or not, window.py's queues. WindowAt10_Handler / Dialog7cb2b8_Handler / DefaultHandler (0x492e00, 0x4a0a20, 0x4a0a90; [0x7cb2b8]
# random): the default handler +0x118 fncmp's logger, the window's object A0[0] random bytes, the message 0x14 often.
# Dialog7cb2b8_ResetChild21 / ResetChild21Only / Obj7b51f0_ResetDialogChild21 (0x4a25f0, 0x4b4470, 0x4816b0): see reset_case.
# usage: dialogs.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Dialog7cc47c_Message4,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import CDECL_LOG4, HEAP, LOG, RAM, Bench, Call, heap, log_region, put, set_return, vtable  # noqa: E402
from window import HOLDER, KIDS, MISC, RANGES, VTABLE, WINDOW, WINDOW2, queues  # noqa: E402
from window705a50 import window as window705a50  # noqa: E402

CALLS = {'Dialog7cc47c_Message4': 0x4b87f0, 'Dialog7cc47c_Message6On': 0x4b8810, 'Dialog7cc47c_Message6Off': 0x4b8830,
         'Dialog7cc4b4_Message4': 0x4b8e80, 'Dialog7cc4b4_Message6Off': 0x4b8ec0, 'Dialog7cc4c4_Message4': 0x4b9c00, 'Dialog7cc4c4_Message6On': 0x4b9c20,
         'Dialog7cdb0c_Message4': 0x4c5780, 'Dialog7cdb0c_Message6On': 0x4c57a0, 'Dialog7cdb0c_Message6Off': 0x4c57c0,
         'Dialog7cdb0c_Child16007Message6On': 0x4c57e0, 'Dialog7cdb0c_Child16007Message6Off': 0x4c5800,
         'Dialog7ca098_Message4IfUp': 0x48d990, 'Dialog7cca94_Message4IfUp': 0x4c25e0, 'WindowAt10_Message4': 0x48cf70,
         'WindowAt10_Message4IfAny': 0x492e60, 'Windows7c26b0_Message5': 0x48ee20, 'WindowAt10_ScalarDtor': 0x48c8b0,
         'Windows7cc8bc_Message6Pair': 0x4bcef0, 'Global7cc698_Choose': 0x4bcec0, 'Dialog7cb2b8_Child21Post': 0x4a2590,
         'WindowAt10_Handler': 0x492e00, 'Dialog7cb2b8_Handler': 0x4a0a20, 'DefaultHandler': 0x4a0a90,
         'Dialog7cb2b8_ResetChild21': 0x4a25f0, 'Dialog7cb2b8_ResetChild21Only': 0x4b4470, 'Obj7b51f0_ResetDialogChild21': 0x4816b0}
SENDS = {k: int(k[6:12], 16) for k in CALLS if k.startswith('Dialog7') and 'Handler' not in k and 'Post' not in k and 'Reset' not in k}   # the window's global
RESET_KIDS, BUTTONS = RAM + 0x9000, RAM + 0xa000   # (0x160 bytes each: the kind byte +0x134, the 0x705a50 window's state +0x138)
B = Bench(sorted(set(RANGES + [(0x4816b0, 0x4816e0), (0x4a25f0, 0x4a2690), (0x4b4470, 0x4b44a0), (0x658ccc, 0x658e04), (0x659710, 0x659740),
                               (0x65e6d5, 0x65e73d), (0x65f697, 0x65f6e1), (0x668820, 0x6688c3), (0x668ad3, 0x668c99), (0x6697e3, 0x6698b4)])))


def chain(r, regions, base, want, others, fill):
    """0-2 windows of 0x160 (ids from others) with want among them (now and then missing), linked by +0x54; returns the first or 0"""
    ids = [r.choice(others) for _ in range(r.choice([0, 1, 2]))]
    if r.random() < 0.9:
        ids.insert(r.randrange(len(ids) + 1), want)
    for i, ident in enumerate(ids):
        kid = window705a50(r)
        put(kid, 0x54, 'I', base + 0x160 * (i + 1) if i + 1 < len(ids) else 0)
        put(kid, 0x8c, 'I', ident)
        put(kid, 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
        fill(kid, ident)
        regions[base + 0x160 * i] = kid
    return base if ids else 0


def reset_case(r, op, regions):
    """Dialog7cb2b8_ResetChild21 and its callers: [0x7cb2ac] set or not, the dialog [0x7cb2b8] or none, its child 0x21 (kind +0x134 3, 4
    or other; the dword +0xa8 set or not) and child 0x25 among others, 0x25's child 0x27 (a 0x705a50 window without listeners) among
    others; the flag 0, 1 or with only upper bits; [0x7cc2f0] set or not; window.py's queues, [0xfaa5d4] / [0xfaa5fc]"""
    queues(r, regions)
    regions[0xfaa5fc] = struct.pack('<I', r.choice([0, CDECL_LOG4]))
    regions[0xfaa5d4] = struct.pack('<I', r.choice([0, 1, 0x80000000]))

    def button(kid, ident):
        put(kid, 0x98, 'II', 0, 0)

    def kid(w, ident):
        w[0x134] = r.choice([3, 4, 3, 4, 0, 2, 5, r.getrandbits(8)])
        put(w, 0xa8, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
        if ident == 0x25:
            put(w, 0x5c, 'I', chain(r, regions, BUTTONS, 0x27, [0x26, 0x28, 1], button))
    first = chain(r, regions, RESET_KIDS, 0x21, [0x25, 0x25, 0x20, 1], kid)
    if not any(struct.unpack_from('<I', w, 0x8c)[0] == 0x25 for a, w in regions.items() if RESET_KIDS <= a < BUTTONS) and r.random() < 0.8:
        w = window705a50(r)   # (child 0x25 with its 0x27 after the rest)
        put(w, 0x54, 'I', first)
        put(w, 0x8c, 'I', 0x25)
        put(w, 0x114, 'I', CDECL_LOG4)
        kid(w, 0x25)
        regions[RESET_KIDS + 0x600] = w
        first = RESET_KIDS + 0x600
    dialog = bytearray(r.bytes(0x120))
    put(dialog, 0x5c, 'I', first)
    regions[WINDOW2] = dialog
    regions[0x7cb2b8] = struct.pack('<I', r.choice([WINDOW2] * 6 + [0]))
    regions[0x7cb2ac] = struct.pack('<I', r.choice([1, 1, 1, 0, r.getrandbits(32)]))
    regions[0x7cc2f0] = struct.pack('<I', r.choice([1, 1, 0, r.getrandbits(32)]))
    if op == 'Dialog7cb2b8_ResetChild21':
        flag = r.choice([0, 1, 1, 0x10000, r.getrandbits(32)])
        return Call(CALLS[op], [flag], regions)
    if op == 'Obj7b51f0_ResetDialogChild21':
        return Call(CALLS[op], [0x7b51f0], regions, conv='t', ret='a')
    return Call(CALLS[op], [], regions)


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, VTABLE: vtable({5: 1})}
    if 'Reset' in op:
        return reset_case(r, op, regions)
    if op == 'Global7cc698_Choose':   # 0x7cc8b8 set or not, the byte 0x7cc8b4 set or not, any windows
        g = bytearray(r.bytes(0x40))
        put(g, 0x28, 'I', r.choice([0, 0, 7, 0x80000000]))  # 0x7cc8b8 (region base 0x7cc890)
        g[0x24] = r.choice([0, 0, 1, 0xff])  # 0x7cc8b4
        return Call(CALLS[op], [], {0x7cc890: g, 0x7cc690: r.bytes(0x20)}, ret='a')
    if op == 'Dialog7cb2b8_Child21Post':   # the dialog [0x7cb2b8] or none; its child 0x21 among 0-2 others (or missing), its dword +0xa8 0 or not
        regions[0xfaa5fc] = struct.pack('<I', r.choice([0, CDECL_LOG4]))
        queues(r, regions)
        ids = [r.choice([0x20, 0x22, 1]) for _ in range(r.choice([0, 1, 2]))]
        if r.random() < 0.85:
            ids.insert(r.randrange(len(ids) + 1), 0x21)
        for i, ident in enumerate(ids):
            kid = bytearray(r.bytes(0x120))
            kid[4] = r.choice([0, 0, 0, 1])
            put(kid, 0x54, 'I', KIDS + 0x120 * (i + 1) if i + 1 < len(ids) else 0)
            put(kid, 0x8c, 'I', ident)
            put(kid, 0xa8, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
            put(kid, 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
            regions[KIDS + 0x120 * i] = kid
        dialog = bytearray(r.bytes(0x120))
        put(dialog, 0x5c, 'I', KIDS if ids else 0)
        regions[WINDOW2] = dialog
        regions[0x7cb2b8] = struct.pack('<I', r.choice([WINDOW2] * 6 + [0]))
        return Call(CALLS[op], [r.choice([0, 1, r.getrandbits(32), r.getrandbits(32)])], regions, ret='a')
    if op in ('WindowAt10_Handler', 'Dialog7cb2b8_Handler', 'DefaultHandler'):   # the default handler +0x118 fncmp's logger (its result now and then set); the object A0[0]
        win = bytearray(r.bytes(0x120))
        put(win, 0x118, 'I', CDECL_LOG4)
        put(win, 0xa0, 'I', MISC)
        regions[WINDOW] = win
        regions[MISC] = r.bytes(0x20)
        regions[0x7cb2b8] = r.bytes(4)
        if r.random() < 0.5:
            set_return(log, 0x3d, r.choice([0, 1, -1, r.getrandbits(32)]))
        args = [r.choice([0x14, 0x14, 0x15, 0, 4, r.getrandbits(32)])] + [r.choice([0, 1, 4, r.getrandbits(32)]) for _ in range(2)]
        return Call(CALLS[op], [WINDOW] + args, regions, ret='a')
    win = bytearray(r.bytes(0x120))
    win[4] = r.choice([0, 0, 0, 1, 0x80])
    put(win, 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, CDECL_LOG4, 0]))
    regions[WINDOW] = win
    regions[0xfaa5fc] = struct.pack('<I', r.choice([0, 0, CDECL_LOG4]))
    if r.random() < 0.5:
        set_return(log, 0x3d, r.choice([0, 1, -1, r.getrandbits(32)]))
    window = r.choice([WINDOW] * 6 + [0])
    if op.startswith('WindowAt10'):
        holder = bytearray(r.bytes(0x20))
        put(holder, 0x10, 'I', window)
        regions[HOLDER] = holder
        if op == 'WindowAt10_ScalarDtor':   # (freed for flags bit 0: the heap's free count and hash)
            regions[HEAP] = heap(0x20)
            return Call(CALLS[op], [HOLDER, r.choice([0, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')
        return Call(CALLS[op], [HOLDER], regions, conv='t', ret='a')
    if op == 'Windows7c26b0_Message5':   # ten entries of 0x208: +0 the window, +4 a word (0: skipped)
        other = bytearray(r.bytes(0x120))
        other[4] = r.choice([0, 0, 1])
        put(other, 0x114, 'I', r.choice([CDECL_LOG4, 0]))
        regions[WINDOW2] = other
        for i in range(10):
            regions[0x7c26b0 + 0x208 * i] = struct.pack('<IH', r.choice([WINDOW, WINDOW2, 0]), r.choice([0, 0, 1, 0x100, 0xffff]))
        return Call(CALLS[op], [], regions)
    if op == 'Windows7cc8bc_Message6Pair':   # (0x7cc698 the first window or not, then cleared)
        other = bytearray(r.bytes(0x120))
        other[4] = r.choice([0, 0, 1])
        put(other, 0x114, 'I', r.choice([CDECL_LOG4, 0]))
        regions[WINDOW2] = other
        first = r.choice([WINDOW, WINDOW2, 0])
        regions[0x7cc8bc] = struct.pack('<II', first, r.choice([WINDOW, WINDOW2, 0]))
        regions[0x7cc698] = struct.pack('<I', r.choice([first, first, WINDOW, WINDOW2, 0, r.getrandbits(32)]))
        return Call(CALLS[op], [], regions, ret='a')
    if op == 'Dialog7ca098_Message4IfUp':   # (the dword 0x7ca090 is cleared)
        regions[0x7ca090] = r.bytes(8) + struct.pack('<I', window)
        return Call(CALLS[op], [], regions, ret='a')
    if 'Child' in op:   # the dialog's child (from 0-3 others, now and then missing: the send gets no window) sent to instead
        want = int(op.split('Child')[1][:5], 16)
        ids = [r.choice([want - 1, want + 1, 1]) for _ in range(r.choice([0, 1, 2]))]
        if r.random() < 0.9:
            ids.insert(r.randrange(len(ids) + 1), want)
        for i, ident in enumerate(ids):
            kid = bytearray(r.bytes(0x120))
            kid[4] = r.choice([0, 0, 0, 1])
            put(kid, 0x54, 'I', KIDS + 0x120 * (i + 1) if i + 1 < len(ids) else 0)
            put(kid, 0x8c, 'I', ident)
            put(kid, 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
            regions[KIDS + 0x120 * i] = kid
        put(win, 0x5c, 'I', KIDS if ids else 0)
        window = WINDOW
    regions[SENDS[op]] = struct.pack('<I', window)
    return Call(CALLS[op], [], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
