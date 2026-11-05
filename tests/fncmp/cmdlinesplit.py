# The command line cut in parts (src/game/core/cmdline: CommandLine_Split, 0x405630) against the original: lines of words with " /" among them, spaces and slashes alone, none, and past 0x80 parts. GetCommandLineA and
# the copy (CRT_strdup, its text hashed; it answers a copy the case holds) recorders.
# usage: cmdlinesplit.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, import_table, log_region, set_return, strings  # noqa: E402

DUP, GETLINE = 1, 42
B = Bench([(0x405630, 0x4056ce), (0x5da3c0, 0x5da3c1)], hooks={0x68b5f0: _recorder(DUP, 1, cdecl=True)})
SELF, LINE, COPY = RAM, RAM + 0x400, RAM + 0x800
WORDS = [b'tp.exe', b'/v', b' /', b' ', b'/', b'a b', b'/x y', b'  /z', b' /  /', b'-q', b' / /']


def case(r):
    log = log_region()
    table = import_table(log, {'GetCommandLineA': (GETLINE, 0x6fd1e4, 0, 0, 0)})
    kind = r.deal(['words', 'words', 'words', 'words', 'empty', 'many', 'many exact'])
    line = (b'' if kind == 'empty' else b'x /a' * (0x81 if kind == 'many' else 0x80) if kind.startswith('many') else b''.join(r.choice(WORDS) for _ in range(r.randrange(1, 9))))
    set_return(log, GETLINE, LINE)
    set_return(log, DUP, COPY)
    strings(log, DUP, 1)
    regions = {LOG: log, IMPORT_TABLE: table, SELF: r.bytes(0x208), LINE: line + b'\0', COPY: line + b'\0' + r.bytes(4)}
    return Call(0x405630, [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
