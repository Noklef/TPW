# A GUI window's messages (src/game/ui/window and its users) against the originals. CallHandler / SendMessage (0x65f697 / 0x658d9f): the
# window (or none) with its byte +4 now and then set, its handler +0x114 and the hook [0xfaa5fc] fncmp's four-argument cdecl logger or 0 (its
# result now and then set). CallDefault (0x65f66e): the default handler +0x118 fncmp's logger, the message 0x14 often. PostMessage
# (0x658ccc): each message kind, the window or none (its byte +4 now and then set), the two queues 1-4 entries (free or not) or none.
# CancelPosted / CancelPostedFirst / CancelTimed (0x658fe9, 0x659012, 0x65903b) and ResetQueues (0x659064): window.py's timers() (the queues
# and the timed table, entries the window's now and then, each now and then missing); the window or 0. MouseDown / MouseUp (0x658af1,
# 0x658b5b): buttons 0-7, the windows the window or none, the queues as for PostMessage.
# usage: windowmessages.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=CallHandler,SendMessage,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import CDECL_LOG4, LOG, Bench, Call, log_region, put, set_return, vtable  # noqa: E402
from window import MISC, RANGES, VTABLE, WINDOW, queues, timers  # noqa: E402

CALLS = {'CallHandler': 0x65f697, 'SendMessage': 0x658d9f, 'CallDefault': 0x65f66e, 'PostMessage': 0x658ccc,
         'CancelPosted': 0x658fe9, 'CancelPostedFirst': 0x659012, 'CancelTimed': 0x65903b, 'ResetQueues': 0x659064,
         'MouseDown': 0x658af1, 'MouseUp': 0x658b5b}
B = Bench(RANGES)


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, VTABLE: vtable({5: 1})}
    if op == 'ResetQueues':   # the queues and the timed table as for the cancels
        timers(r, regions, WINDOW)
        return Call(CALLS[op], [], regions, ret='v')
    if op.startswith('Cancel'):
        timers(r, regions, WINDOW)
        return Call(CALLS[op], [r.choice([WINDOW] * 5 + [0])], regions, ret='w')
    if op in ('MouseDown', 'MouseUp'):
        # the GUI up ([0x77c488]) or not, the hovered window and the buttons' windows (0xfaa588) the window or none, the button bits
        # [0xfaa5bc]; buttons 0-7 (their high words random), x and y anything
        win = bytearray(r.bytes(0x120))
        win[4] = r.choice([0, 0, 1])
        regions[WINDOW] = win
        queues(r, regions)
        regions[0x77c488] = struct.pack('<I', r.choice([1, 1, 1, 0, 0x100]))
        regions[0xfaa588] = struct.pack('<8I', *[r.choice([WINDOW, 0]) for _ in range(8)])
        regions[0xfaa5bc] = r.bytes(2)
        regions[0xfaa5e0] = struct.pack('<I', r.choice([WINDOW, WINDOW, 0]))
        button = r.choice([0, 1, 2, 2, 7]) | r.choice([0, 0, 0x10000, 0xffff0000])
        return Call(CALLS[op], [button, r.getrandbits(32), r.choice([0, 5, 0x8000, r.getrandbits(32)])], regions, ret='w')
    if op == 'PostMessage':   # the queues [0xfaa5f4] / [0xfaa5f0] (as msgqueue.py) or 0 (for 0x1000a-c too: a fault in both)
        win = bytearray(r.bytes(0x120))
        win[4] = r.choice([0, 0, 0, 1, 0x80])
        regions[WINDOW] = win
        queues(r, regions)
        message = r.choice([1, 2, 3, 0x1000a, 0x1000b, 0x1000c, 0, 4, 0x10001, 0x1000d, r.getrandbits(32)])
        return Call(CALLS[op], [r.choice([WINDOW] * 6 + [0]), message, r.getrandbits(32), r.getrandbits(32)], regions, ret='a')
    if op == 'CallDefault':   # the default handler +0x118 fncmp's logger (its result now and then set); the object A0[0]
        win = bytearray(r.bytes(0x120))
        put(win, 0x118, 'I', CDECL_LOG4)
        put(win, 0xa0, 'I', MISC)
        regions[WINDOW] = win
        regions[MISC] = r.bytes(0x20)
        regions[0x7cb2b8] = r.bytes(4)
        if r.random() < 0.5:
            set_return(log, 0x3d, r.choice([0, 1, -1, r.getrandbits(32)]))
        args = [r.choice([0x14, 0x14, 0x15, 0, 4, r.getrandbits(32)])] + [r.choice([0, 1, 4, r.getrandbits(32)]) for _ in range(2)]
        return Call(CALLS[op], [WINDOW] + args, regions, conv='t', ret='a')
    win = bytearray(r.bytes(0x120))   # CallHandler / SendMessage
    win[4] = r.choice([0, 0, 0, 1, 0x80])
    put(win, 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, CDECL_LOG4, 0]))
    regions[WINDOW] = win
    regions[0xfaa5fc] = struct.pack('<I', r.choice([0, 0, CDECL_LOG4]))
    if r.random() < 0.5:
        set_return(log, 0x3d, r.choice([0, 1, -1, r.getrandbits(32)]))
    window = r.choice([WINDOW] * 6 + [0])
    args = [r.choice([0, 1, 4, 6, r.getrandbits(32)]) for _ in range(3)]
    if op == 'CallHandler':
        return Call(CALLS[op], [WINDOW] + args, regions, conv='t', ret='a')
    return Call(CALLS[op], [window] + args, regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
