# The key actions that only call on (src/game/core/keyactions: 0x40bf70, 0x40c3a0, 0x40c3f0, 0x40c410, 0x40c800, 0x424c10, and the shortcut
# table's 0x40c4c0-0x40c5c0) against the originals, what they call being recorders (the shortcut entries' thiscall ones: ECX compared);
# each returns 1.
# usage: keyactioncalls.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, _recorder, log_region, set_return  # noqa: E402

CALLS = {'Pause': (0x40bf70, 0x40bf82), 'Hide': (0x40c3a0, 0x40c3b0), 'OpenNew': (0x40c3f0, 0x40c405), 'OpenLast': (0x40c410, 0x40c425),
         'Post': (0x40c800, 0x40c818), 'Clear': (0x424c10, 0x424c20)}
# The shortcut entries (16 bytes each) and the entry point each calls, recorder k as driver.cpp's kKeyActionCalls.
SHORTCUTS = {'Postcard': (0x40c4c0, 0x4a9380, 6), 'Menu': (0x40c4d0, 0x4816d0, 7), 'Buy': (0x40c4e0, 0x4816e0, 8), 'Hire': (0x40c4f0, 0x481700, 9),
             'ParkStatus': (0x40c500, 0x481710, 10), 'AllItems': (0x40c510, 0x481720, 11), 'AllStaff': (0x40c520, 0x481730, 12),
             'AllPeeps': (0x40c530, 0x481740, 13), 'FinanceInfo': (0x40c540, 0x481750, 14), 'Loans': (0x40c550, 0x481760, 15),
             'StaffCosts': (0x40c560, 0x481770, 16), 'EntryPrice': (0x40c570, 0x481780, 17), 'Map': (0x40c580, 0x481510, 18),
             'StaffLoc': (0x40c590, 0x481790, 19), 'PeepLoc': (0x40c5a0, 0x4817c0, 20), 'Research': (0x40c5b0, 0x4817e0, 21),
             'Camcorder': (0x40c5c0, 0x481a10, 22)}
CALLS.update({name: (a, a + 0x10) for name, (a, _, _) in SHORTCUTS.items()})
hooks = {0x409350: _recorder(1, 1), 0x4819e0: _recorder(2, 0), 0x519ef0: _recorder(3, 2), 0x5194d0: _recorder(4, 0), 0x59ab50: _recorder(5, 0)}
hooks.update({callee: _recorder(k, 0) for _, callee, k in SHORTCUTS.values()})
B = Bench(list(CALLS.values()) + [(0x481500, 0x481505)], hooks=hooks)   # (0x481500: Postcard's JMP to 0x4a9380)
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def case(r):
    log = log_region()
    set_return(log, 4, r.getrandbits(32))
    regions = {LOG: log, 0x7cf83c: r.bytes(4), 0x78d96c: r.bytes(4)}
    return Call(CALLS[r.deal(OPS)][0], [], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
