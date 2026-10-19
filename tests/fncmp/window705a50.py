# The GUI library's 0x705a50 window's state (src/game/ui/window705a50: 0x668820, 0x668ad3, 0x668bec) against the originals.
# Look: random windows, flag 2 of +0x48 and the bytes +0x138 / +0x139 set or not.
# NotifyLook: Look's windows with the object +0x9c a recorder (slot 3) or 0. SetState: Look's windows, the state equal to +0x138 now and
# then, notify 0 / 1 (the pushed register's upper bytes random), [0xfaa5d4] set or not, window.py's message queues (Window_PostMessage),
# the objects +0x98 / +0x9c recorders (slot 3: k 3 and 22) or 0.
# Masked: both functions hand their listeners an event on the stack that they only partly set, so each recorder logs its event as a hash of
# the bytes set (fncmp.blob_bytes): 0x668bec's event its look word +4 and the dword +0x18 (1); 0x668ad3's its dword +0x18 (the code) and, by
# the pointer +0x10, the second event's words +0xa / +0xc and dword +0x10, except for look 1, where nothing there is set (the pointer itself
# is a stack address, not compared). The other bytes are stack garbage that differs between the sides.
# usage: window705a50.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Look,NotifyLook,SetState)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, RAM, blob_bytes, log_region, put, recorder, vtable  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from window import queues  # noqa: E402

CALLS = {'Look': 0x668820, 'NotifyLook': 0x668ad3, 'SetState': 0x668bec}
RANGES = [(0x668820, 0x6688b3), (0x668ad3, 0x668bd4), (0x668bec, 0x668c89),
          (0x658ccc, 0x658d9f), (0x6697e3, 0x6698a4), (0x659710, 0x659721)]
B = Bench(RANGES)
WINDOW, OBJ98, OBJ9C, VT98, VT9C = RAM, RAM + 0x200, RAM + 0x240, RAM + 0x280, RAM + 0x300
EVENT_MASK = 0x0f000030   # 0x668bec's event: the look +4 / +5, the dword +0x18
NOTIFY_MASK = 0x0f000000  # 0x668ad3's event: the code +0x18
INNER = (0x10, 0xf3c, 8)  # its second event from +8: the words +0xa / +0xc, the dword +0x10


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
    state = w[0x138] if r.random() < 0.25 else r.choice([0, 1, 2, 3, r.getrandbits(8)])
    objects(r, regions, log, w, [(0x98, OBJ98), (0x9c, OBJ9C)])
    w2 = bytearray(w)   # the look the notification sees: after the state is set
    w2[0x138] = state
    blob_bytes(log, 22, 1, NOTIFY_MASK, None if look(w2) == 1 else INNER)
    blob_bytes(log, 3, 1, EVENT_MASK)
    queues(r, regions)
    regions[0xfaa5d4] = put(bytearray(4), 0, 'I', r.choice([0, 1, 0x80000000]))
    notify = r.choice([0, 1, 0xff]) | r.choice([0, r.getrandbits(24)]) << 8
    return Call(CALLS[op], [WINDOW, state | r.choice([0, r.getrandbits(24)]) << 8, notify], regions, conv='t', ret='w')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
