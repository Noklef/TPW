# The online chat's flags under its lock (src/game/online/chat: 0x4e6130, 0x4e6150) against the originals: a chat of random bytes, its
# critical section (+0x48) entered and left through recording imports.
# usage: onlinechat.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Set,Get)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, import_table, log_region  # noqa: E402

B = Bench([(0x4e6130, 0x4e6150), (0x4e6150, 0x4e616f), (0x5c8bb0, 0x5c8bd0)])
CALLS = {'Set': 0x4e6130, 'Get': 0x4e6150}
IMPORTS = {'EnterCriticalSection': (1, 0x6fd140, 1, 0, 0), 'LeaveCriticalSection': (2, 0x6fd130, 1, 0, 0)}


def case(r):
    op = r.choice(OPS)
    log = log_region()
    regions = {RAM: r.bytes(0x100), LOG: log, IMPORT_TABLE: import_table(log, IMPORTS)}
    return Call(CALLS[op], [RAM], regions, conv='t', ret='b' if op == 'Get' else 'v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
