# A GUI window's messages (src/game/ui/window and its users) against the originals. CallHandler / SendMessage (0x65f697 / 0x658d9f): the
# window (or none) with its byte +4 now and then set, its handler +0x114 and the hook [0xfaa5fc] fncmp's four-argument cdecl logger or 0 (its
# result now and then set). CallDefault (0x65f66e): the default handler +0x118 fncmp's logger, the message 0x14 often. PostMessage
# (0x658ccc): each message kind, the window or none (its byte +4 now and then set), the two queues 1-4 entries (free or not) or none.
# CancelPosted / CancelPostedFirst / CancelTimed (0x658fe9, 0x659012, 0x65903b) and ResetQueues (0x659064): window.py's timers() (the queues
# and the timed table, entries the window's now and then, each now and then missing); the window or 0. MouseDown / MouseUp (0x658af1,
# 0x658b5b): buttons 0-7, the windows the window or none, the queues as for PostMessage.
# PostIfHit (0x65dcaf): the rectangle +0x2c a GUI one or a recorder (its answer's upper half set or not), the point in it or near, the press
# points [0xfaa598] small or negative words. Activate (0x65dd18): the active window [0xfaa5dc] the window, another or none.
# usage: windowmessages.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=CallHandler,SendMessage,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import CDECL_LOG4, LOG, Bench, Call, log_region, put, set_return, vtable  # noqa: E402
from window import MISC, RANGES, VTABLE, WINDOW, WINDOW2, queues, timers  # noqa: E402

CALLS = {'CallHandler': 0x65f697, 'SendMessage': 0x658d9f, 'CallDefault': 0x65f66e, 'PostMessage': 0x658ccc,
         'CancelPosted': 0x658fe9, 'CancelPostedFirst': 0x659012, 'CancelTimed': 0x65903b, 'ResetQueues': 0x659064,
         'MouseDown': 0x658af1, 'MouseUp': 0x658b5b, 'PostIfHit': 0x65dcaf, 'Activate': 0x65dd18}
B = Bench(RANGES + [(0x65dcaf, 0x65dd18), (0x65dd18, 0x65dd5f)])


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
    if op == 'PostIfHit':   # the rectangle +0x2c a GUI one (small, empty now and then); the point in it or near (upper halves random);
        # the press points [0xfaa598] (four entries of 0xc bytes) small or negative words; buttons 0-3; the queues
        win = bytearray(r.bytes(0x120))
        win[4] = r.choice([0, 0, 0, 1])
        left, top = r.randrange(-30, 20), r.randrange(-30, 20)
        right, bottom = left + r.choice([0, 1, 10, 30]), top + r.choice([0, 1, 10, 30])
        put(win, 0x2c, 'Ihhhh', 0x705920, left, top, right, bottom)
        if r.random() < 0.25:   # (a recorder rectangle whose slot 0 answers a word with the upper half set or not)
            put(win, 0x2c, 'I', VTABLE)
            regions[VTABLE] = vtable({5: 1, 0: 2})
            set_return(log, 0, r.choice([0, 1, 0x10000, 0xffff0001, 0x8000]))
        regions[WINDOW] = win
        queues(r, regions)
        regions[0xfaa598] = bytes(b for _ in range(0x18) for b in struct.pack('<h', r.choice([0, 1, 7, -1, -300, 0x7fff, r.randrange(-0x8000, 0x8000)])))
        x = r.choice([left, right - 1, r.randrange(left - 5, right + 5), r.randrange(-0x8000, 0x8000)]) & 0xffff
        y = r.choice([top, bottom - 1, r.randrange(top - 5, bottom + 5), r.randrange(-0x8000, 0x8000)]) & 0xffff
        point = x << 16 | y if r.random() < 0.9 else x << 16 | y | r.getrandbits(16) << 16
        message = r.choice([0x10005, 0x10006, 0x10007, 0, r.getrandbits(32)])
        return Call(CALLS[op], [WINDOW, message, r.randrange(4), point], regions, conv='t', ret='v')
    if op == 'Activate':   # the active window [0xfaa5dc] the window, another or none; the queues
        for w in (WINDOW, WINDOW2):
            win = bytearray(r.bytes(0x120))
            win[4] = r.choice([0, 0, 0, 1])
            regions[w] = win
        queues(r, regions)
        regions[0xfaa5dc] = struct.pack('<I', r.choice([WINDOW, WINDOW2, WINDOW2, 0]))
        return Call(CALLS[op], [WINDOW], regions, conv='t', ret='v')
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
