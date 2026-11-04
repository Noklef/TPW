# The game owner's pausing and its end (src/game/core/gameowner: 0x4092a0, 0x409300, 0x409350, 0x409180) against the originals, all callees recorders: +0x3c 1 or not, +0x1c 1 or not, the flags' bytes either (a flag's upper bytes any), the mode 1 or not; the
# level [0x7cf83c], the message centre [0x788d3c] and the list +0x38 there or not, flag 0x400000 of 0x8bcbc8 either.
# usage: gameowner.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

TH = {0x481440: 1, 0x402d90: 2, 0x481450: 3, 0x402db0: 4, 0x481460: 5, 0x402dd0: 6, 0x515dd0: 7, 0x409240: 8, 0x412d50: 9, 0x585e20: 10, 0x418d70: 11, 0x409250: 12, 0x408f00: 13}
CD = {0x51bcf0: (14, 0), 0x598960: (15, 0), 0x51c1c0: (16, 1), 0x51bd30: (17, 0), 0x598990: (18, 0), 0x67acb0: (19, 1)}
HOOKS = {a: _recorder(k, 0) for a, k in TH.items()}
HOOKS.update({a: _recorder(k, n, cdecl=True) for a, (k, n) in CD.items()})
B = Bench([(0x4092a0, 0x4092f4), (0x409300, 0x409343), (0x409350, 0x4093cb), (0x409180, 0x40923f), (0x5da3c0, 0x5da3c1)], hooks=HOOKS)
SELF = RAM
OPS = ['Pause', 'Resume', 'Toggle', 'Shutdown']


def case(r):
    regions = {LOG: log_region()}
    me = bytearray(r.bytes(0x40))
    put(me, 0x3c, 'I', r.choice([1, 1, 1, 1, 0, 2, 0x101]))
    put(me, 0x1c, 'I', r.choice([0, 1, 1, 2]))
    me[0x20] = r.choice([0, 1, 0x80])
    flag = r.choice([0, 1, 0x80]) | r.choice([0, 0, 0x100, r.getrandbits(24) << 8])
    op = r.deal(OPS)
    regions[SELF] = me
    if op == 'Pause':
        call = Call(0x4092a0, [SELF, r.choice([0, 1, 1, 2, 0x101, -1]), flag], regions, conv='t', ret='v')
    elif op == 'Resume':
        call = Call(0x409300, [SELF], regions, conv='t', ret='v')
    elif op == 'Toggle':
        call = Call(0x409350, [SELF, flag], regions, conv='t', ret='v')
    else:
        put(me, 0x38, 'I', r.choice([0, RAM + 0x100]))
        regions[0x7cf83c] = r.choice([bytes(4), (RAM + 0x200).to_bytes(4, 'little')])
        regions[0x788d3c] = r.choice([bytes(4), (RAM + 0x300).to_bytes(4, 'little')])
        regions[0x8bcbc8] = r.choice([bytes(4), (0x400000).to_bytes(4, 'little'), r.bytes(4)])
        call = Call(0x409180, [SELF], regions, conv='t', ret='a')
    return call


if __name__ == '__main__':
    B.main(case)
