# The 0x7ca1c8 dialog's button selection (src/game/ui/dialog7ca1c8: 0x4981d0, 0x498a30, 0x4819f0) against the originals. The dialog
# [0x7ca1c8] with its child 0x18 a radiogroup.py group (children of the ids 0x19-0x21 and a few others) among another child or two, now and
# then missing (a fault in both); [0x7ca1cc] set or 0 (the IfUp guard; IfUp's dialog now and then 0); flags with one or a few of bits 0-8
# set, none, or any. GameCursor_Set's window [0x7cb2ac] 0 (the cursor id is still stored at 0x7cb2d8). The events are masked as
# radiogroup.py / window705a50.py say.
# usage: dialog7ca1c8.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SelectButton,SelectButtonIfUp,SelectButtonIfUpThis)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, RAM, log_region, put  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from radiogroup import GROUP, RANGES as GROUP_RANGES, group_env, masks, select_states  # noqa: E402

CALLS = {'SelectButton': 0x4981d0, 'SelectButtonIfUp': 0x498a30, 'SelectButtonIfUpThis': 0x4819f0}
RANGES = [(0x4981d0, 0x49829c), (0x498a30, 0x498a50), (0x4819f0, 0x481a00), (0x4a2aa0, 0x4a2ac0)] + GROUP_RANGES
B = Bench(RANGES)
DIALOG, OTHER = RAM + 0x3000, RAM + 0x3200
IDS = list(range(0x19, 0x22)) + [0x18, 2]
PICKS = [(4, 0x1b), (1, 0x1f), (2, 0x20), (8, 0x19), (0x10, 0x1a), (0x20, 0x1c), (0x40, 0x21), (0x80, 0x1e), (0x100, 0x1d)]


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log}
    g = bytearray(r.bytes(0x160))
    g[0x48] = 0
    put(g, 0x8c, 'I', r.choice([0x18] * 12 + [0x17]))
    g, kids = group_env(r, regions, log, g, IDS)
    other = bytearray(r.bytes(0x90))
    put(other, 0x8c, 'I', r.choice([3, 0x19]))
    put(other, 0x54, 'I', 0)
    regions[OTHER] = other
    if r.random() < 0.5:   # the group first or second
        put(g, 0x54, 'I', OTHER)
        first = GROUP
    else:
        put(g, 0x54, 'I', 0)
        put(other, 0x54, 'I', GROUP)
        first = OTHER
    regions[DIALOG] = put(bytearray(r.bytes(0x60)), 0x5c, 'I', first)
    up = r.choice([1, 1, 1, 0]) if op != 'SelectButton' else 1
    regions[0x7ca1c8] = put(bytearray(8), 0, 'II', DIALOG if op == 'SelectButton' or r.random() < 0.9 else 0, up)
    regions[0x7cb2ac] = bytearray(4)
    regions[0x7cb2d8] = put(bytearray(4), 0, 'I', r.getrandbits(32))
    flags = r.choice([1 << r.randrange(9), (1 << r.randrange(9)) | (1 << r.randrange(9)), 0, r.getrandbits(9), r.getrandbits(32)])
    ident = next((i for bit, i in PICKS if flags & bit), 0xffffffff)
    selected = int.from_bytes(g[0x138:0x13c], 'little')
    masks(log, kids, select_states(kids, selected, ident))
    args = [0x7b51f0, flags] if op == 'SelectButtonIfUpThis' else [flags]
    return Call(CALLS[op], args, regions, conv='t' if op == 'SelectButtonIfUpThis' else 'c', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
