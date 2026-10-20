# The application globals and message pump (src/game/core/app: 0x5f86a0-0x5f8780) against the originals: the getters and setters over any
# state at 0xfa7608 and any argument; the pump with PeekMessageA (a recorder: the MSG on the stack it fills, which TranslateMessage and
# DispatchMessage are logged by, hashed) answering none, or a message every time (the 2000-message limit) with any result; TranslateMessage
# now and then fncmp's COUNTER, which counts the messages in the log (past the 40 entries it holds).
# usage: app.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=GetInstance,... to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import COUNTER, IMPORT_TABLE, LOG, Bench, Call, blob, counter, fill, import_table, log_region, set_return  # noqa: E402

B = Bench([(0x5f86a0, 0x5f8786)])
# name: (address, stack arguments, returns)
CALLS = {'GetInstance': (0x5f86a0, 0, 'a'), 'SetInstance': (0x5f86b0, 1, 'v'), 'GetWindow': (0x5f86c0, 0, 'a'), 'SetWindow': (0x5f86d0, 1, 'v'),
         'PumpMessages': (0x5f86e0, 0, 'a'), 'SetQuit': (0x5f8750, 1, 'v'), 'GetWndProc': (0x5f8760, 0, 'a'), 'GetIconId': (0x5f8770, 0, 'w'),
         'GetName': (0x5f8780, 0, 'a')}
PEEK, TRANSLATE, DISPATCH = 40, 41, 42
# name: (recorder slot, import slot, stack arguments, string arguments, out arguments)
IMPORTS = {'PeekMessageA': (PEEK, 0x6fd354, 5, 0, 1), 'TranslateMessage': (TRANSLATE, 0x6fd358, 1, 0, 0),
           'DispatchMessageA': (DISPATCH, 0x6fd35c, 1, 0, 0)}


def case(r):
    op = r.deal(OPS)
    address, n, ret = CALLS[op]
    regions = {0xfa7600: r.bytes(0x20)}
    if op == 'PumpMessages':
        log = log_region()
        table = import_table(log, IMPORTS)
        if r.random() < 0.5:   # (it writes the count through the MSG pointer: the first 8 bytes)
            table[0x358:0x35c] = COUNTER.to_bytes(4, 'little')
            counter(log, r.getrandbits(64), r.choice([1, r.getrandbits(32)]), r.getrandbits(32))
        regions.update({LOG: log, IMPORT_TABLE: table})
        set_return(log, PEEK, r.choice([0, 0, 1, r.getrandbits(32)]))
        set_return(log, TRANSLATE, r.getrandbits(32))
        set_return(log, DISPATCH, r.getrandbits(32))
        fill(log, PEEK, 0, r.bytes(0x1c))
        for k in (TRANSLATE, DISPATCH):
            blob(log, k, 0, 0x7f)
    args = [r.choice([0, 1, 0xffffffff, r.getrandbits(32)]) for _ in range(n)]
    return Call(address, args, regions, conv='c', ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
