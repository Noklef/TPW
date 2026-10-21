# The GUI library's 0x705a50 window's state (src/game/ui/window705a50: 0x668820, 0x668ad3, 0x668bec) against the originals.
# Look: random windows, flag 2 of +0x48 and the bytes +0x138 / +0x139 set or not.
# NotifyLook: Look's windows with the object +0x9c a recorder (slot 3) or 0. SetState: Look's windows, the state equal to +0x138 now and
# then, notify 0 / 1 (the pushed register's upper bytes random), [0xfaa5d4] set or not, window.py's message queues (Window_PostMessage),
# the objects +0x98 / +0x9c recorders (slot 3: k 3 and 22) or 0. SetPair: Look's windows, any two values. Set14c / Set154: NotifyLook's
# windows and objects, the value 0, -1 or any. SetEnabled (0x668a23): Look's windows (flag 2 either way) with the handler +0x114 fncmp's
# logger or 0 and SetState's objects, window.py's tree and hover (Window_UpdateHover), enabled 0, 1 or any (upper bytes random); its event
# masked to the look word +4.
# Masked: both functions hand their listeners an event on the stack that they only partly set, so each recorder logs its event as a hash of
# the bytes set (fncmp.blob_bytes): 0x668bec's event its look word +4 and the dword +0x18 (1); 0x668ad3's its dword +0x18 (the code) and, by
# the pointer +0x10, the second event's words +0xa / +0xc and dword +0x10, except for look 1, where nothing there is set (the pointer itself
# is a stack address, not compared). The other bytes are stack garbage that differs between the sides.
# Repeat (0x668dd9): Look's windows with flag 0x20 of +0x48 mostly set, the phase +0x15a 1 or 2 mostly, the clock [0x77c490] a recorder
# vtable whose slot 0 answers a time near the time +0x144 (or any), the delay / interval +0x13c / +0x140 near the difference (or any),
# the window +0x60 the window itself, another or 0, and window.py's message queues.
# usage: window705a50.py [CASES [SEED]] [--wild|--both] [--show N]
# SetFlag139 (0x668c99): SetState's cases for the byte +0x139, both objects' events masked to the look and the dword +0x18.
#   (env OPS=Look,NotifyLook,SetState,SetPair,Set14c,Set154,SetEnabled,Repeat,SetFlag139)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CDECL_LOG4, LOG, Bench, Call, RAM, blob_bytes, log_region, put, recorder, set_return, vtable  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from window import RANGES as WINDOW_RANGES, hover, queues, tree  # noqa: E402

CALLS = {'Look': 0x668820, 'NotifyLook': 0x668ad3, 'SetState': 0x668bec, 'SetPair': 0x668f35, 'Set14c': 0x668f5a, 'Set154': 0x668f7b,
         'SetEnabled': 0x668a23, 'Repeat': 0x668dd9, 'SetFlag139': 0x668c99}
RANGES = [(0x668820, 0x6688b3), (0x668ad3, 0x668bd4), (0x668bec, 0x668c89), (0x668f35, 0x668f9c), (0x6693d0, 0x669407),
          (0x658ccc, 0x658d9f), (0x6697e3, 0x6698a4), (0x659710, 0x659721)]
B = Bench(sorted(set(RANGES + WINDOW_RANGES + [(0x668a23, 0x668aad), (0x65da8d, 0x65db25), (0x668dd9, 0x668ecf), (0x668c99, 0x668d5f),
                                           (0x65968e, 0x6596a2)])))
WINDOW, OBJ98, OBJ9C, VT98, VT9C = RAM, RAM + 0x200, RAM + 0x240, RAM + 0x280, RAM + 0x300
OTHER, CLOCK, CLOCK_VT = RAM + 0x380, RAM + 0x3c0, RAM + 0x400
EVENT_MASK = 0x0f000030   # 0x668bec's event: the look +4 / +5, the dword +0x18
NOTIFY_MASK = 0x0f000000  # 0x668ad3's event: the code +0x18
INNER = (0x10, 0xf3c, 8)  # its second event from +8: the words +0xa / +0xc, the dword +0x10
LOOK_MASK = 0x30          # 0x668a23's event: the look +4 / +5


def look(w):
    if w[0x48] & 2:
        return 1
    if w[0x139]:
        return 3 if w[0x138] & 1 else 2
    return 4 if w[0x138] & 2 else 5 if w[0x138] & 1 else 0


def window(r):
    w = bytearray(r.bytes(0x160))
    w[4] = r.choice([0, 0, 0, 1])
    w[0x48] = r.choice([0, 2, r.getrandbits(8)])
    w[0x138] = r.choice([0, 1, 2, 3, r.getrandbits(8)])
    w[0x139] = r.choice([0, 0, 1, r.getrandbits(8)])
    w[0x158] = r.choice([0, 0x7f, 0x80, 0xff, r.getrandbits(8)])
    return w


def objects(r, regions, log, w, which):
    """+0x98 / +0x9c recorder objects (slot 3 of two arguments: k 3 / 22) or 0"""
    vt9c = vtable({}, size=8)
    put(vt9c, 0xc, 'I', recorder(22, 2))
    regions[VT98], regions[VT9C] = vtable({3: 2}, size=8), vt9c
    regions[OBJ98] = put(bytearray(r.bytes(0x20)), 0, 'I', VT98)
    regions[OBJ9C] = put(bytearray(r.bytes(0x20)), 0, 'I', VT9C)
    for off, obj in which:
        put(w, off, 'I', obj if r.random() < 0.8 else 0)
    inner = None if look(w) == 1 else INNER
    blob_bytes(log, 22, 1, NOTIFY_MASK, inner)


def repeat(r, regions, log, w):
    w[0x48] = w[0x48] | 0x20 if r.random() < 0.85 else w[0x48] & ~0x20
    w[0x15a] = r.choice([1, 1, 1, 2, 2, 2, 0, r.getrandbits(8)])
    start = r.getrandbits(32)
    near = lambda: r.choice([0, 1, 2, 124, 125, 126, 499, 500, 501, r.getrandbits(32)])  # noqa: E731
    gap = near() if r.random() < 0.8 else r.randrange(0x7ffffff0, 0x80000010)
    regions[CLOCK_VT], regions[CLOCK] = vtable({}), struct.pack('<I', CLOCK_VT)
    regions[0x77c490] = struct.pack('<I', CLOCK)
    set_return(log, 0, start + gap & 0xffffffff)
    put(w, 0x144, 'I', start)
    for off in (0x13c, 0x140):
        put(w, off, 'I', r.choice([gap, gap - 1, gap + 1, near(), r.getrandbits(32)]) & 0xffffffff)
    put(w, 0x60, 'I', r.choice([WINDOW, OTHER, OTHER, 0]))
    regions[OTHER] = put(bytearray(r.bytes(0x10)), 4, 'B', r.choice([0, 0, 0, 1]))
    queues(r, regions)
    return Call(CALLS['Repeat'], [WINDOW], regions, conv='t', ret='v')


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log}
    w = window(r)
    regions[WINDOW] = w
    if op == 'Look':
        return Call(CALLS[op], [WINDOW], regions, conv='t', ret='w')
    if op == 'NotifyLook':
        objects(r, regions, log, w, [(0x9c, OBJ9C)])
        return Call(CALLS[op], [WINDOW], regions, conv='t', ret='v')
    if op == 'SetEnabled':
        w[0x48] = (w[0x48] & 0xfd) | r.choice([0, 2])
        put(w, 0x114, 'I', r.choice([CDECL_LOG4, 0]))
        objects(r, regions, log, w, [(0x98, OBJ98), (0x9c, OBJ9C)])
        blob_bytes(log, 22, 1, LOOK_MASK)
        blob_bytes(log, 3, 1, LOOK_MASK)
        hover(r, regions, tree(r, regions))
        enabled = r.choice([0, 0, 1, 1, 0x8000, r.getrandbits(16)]) | r.choice([0, r.getrandbits(16)]) << 16
        return Call(CALLS[op], [WINDOW, enabled], regions, conv='t', ret='w')
    if op == 'Repeat':
        return repeat(r, regions, log, w)
    if op == 'SetPair':
        return Call(CALLS[op], [WINDOW, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')
    if op in ('Set14c', 'Set154'):   # (NotifyLook's, the value it hands on for some looks set first)
        objects(r, regions, log, w, [(0x9c, OBJ9C)])
        return Call(CALLS[op], [WINDOW, r.choice([0, 0xffffffff, r.getrandbits(32)])], regions, conv='t', ret='v')
    at = 0x139 if op == 'SetFlag139' else 0x138
    state = w[at] if r.random() < 0.25 else r.choice([0, 1, 2, 3, r.getrandbits(8)])
    objects(r, regions, log, w, [(0x98, OBJ98), (0x9c, OBJ9C)])
    w2 = bytearray(w)   # the look the notification sees: after the state is set
    w2[0x138] = state
    blob_bytes(log, 22, 1, EVENT_MASK if op == 'SetFlag139' else NOTIFY_MASK, None if look(w2) == 1 or op == 'SetFlag139' else INNER)
    blob_bytes(log, 3, 1, EVENT_MASK)
    queues(r, regions)
    regions[0xfaa5d4] = put(bytearray(4), 0, 'I', r.choice([0, 1, 0x80000000]))
    notify = r.choice([0, 1, 0xff]) | r.choice([0, r.getrandbits(24)]) << 8
    return Call(CALLS[op], [WINDOW, state | r.choice([0, r.getrandbits(24)]) << 8, notify], regions, conv='t', ret='w')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
