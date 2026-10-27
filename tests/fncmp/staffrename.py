# The staff member's rename dialog's handler (src/game/ui/staffrename: 0x4b6320) against the original. Handler: focus.py's FocusRestore
# set-up (a tree of windows, the message queues, the focus globals) with the five key maps as keymap.py's (KeyMaps_EnableAllAndClearFlag;
# Focus_Restore's three among them), one of the tree's windows called with its handler +0x114 fncmp's logger or none (Window_SendMessage),
# its default handler +0x118 the logger (its result set now and then), the hook [0xfaa5fc] the logger or none; its dword +0xa0 a staff
# member (random bytes: the name +0x1a0 written), +0xa4 an object whose vtable is a recorder (slot 1) and whose +0x24 the clear writes;
# the name buffer 0x7cc360 random wide characters, terminated within 32 or not. The message 0x14, 0x100 (a -1, -2 or other), 0x802, 0x804
# or another.
# usage: staffrename.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Handler)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import focus  # noqa: E402
import keymap  # noqa: E402
import window  # noqa: E402
from fncmp import CDECL_LOG4, LOG, RAM, Bench, Call, put, set_return, vtable  # noqa: E402

CALLS = {'Handler': 0x4b6320}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench(sorted(set(focus.RANGES + focus.MODES + [(0x4b6320, 0x4b6424), (0x486b60, 0x486b70), (0x40c870, 0x40d06a), (0x641910, 0x641920),
                                                   (0x506830, 0x506860), (0x67b0d0, 0x67b110), (0x65e705, 0x65e73d), (0x65f66e, 0x65f6e1)])))
STAFF, SOURCE, KEYS = RAM + 0x60000, RAM + 0x60400, RAM + 0x70000


def case(r):
    r.deal(OPS)
    c = focus.case(r, ['FocusRestore'])
    regions = c.regions
    log = regions[LOG]
    wins = sorted(k for k in regions if window.TREE <= k < window.RECTS)
    w = r.choice(wins)
    for k in wins:
        put(regions[k], 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
    put(regions[w], 0x118, 'I', CDECL_LOG4)
    put(regions[w], 0xa0, 'II', STAFF, SOURCE)
    regions[0xfaa5fc] = struct.pack('<I', r.choice([0, 0, CDECL_LOG4]))
    if r.random() < 0.5:
        set_return(log, 0x3d, r.choice([0, 1, -1, r.getrandbits(32)]))
    regions[STAFF] = r.bytes(0x200)
    regions[SOURCE] = put(bytearray(r.bytes(0x40)), 0, 'I', SOURCE + 0x40) + vtable({}, size=4)
    for k, g in enumerate(keymap.GLOBALS):
        keymap.keymap(r, KEYS + k * 0x2000 + 0x40, regions)
        regions[g] = struct.pack('<I', KEYS + k * 0x2000 + 0x40)
    regions[0x7c24d0] = r.bytes(0x10)
    name = [r.choice([r.randrange(0x20, 0x7f), r.getrandbits(16)]) for _ in range(40)]
    if r.random() < 0.8:
        name[r.randrange(0, 32)] = 0
    regions[0x7cc360] = struct.pack('<40H', *name)
    message = r.choice([0x14] * 3 + [0x100] * 6 + [0x802, 0x804, 0x803, 0x101, 0x15, 0, 4, r.getrandbits(32)])
    a = r.choice([-1, -1, -1, -2, -2, 0, 1, r.getrandbits(31)]) if message == 0x100 else r.choice([-1, -2, r.getrandbits(31)])
    return Call(CALLS['Handler'], [w, message, a & 0xffffffff, r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
