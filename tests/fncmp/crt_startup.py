# The C library's command line parsing and _setargv (src/game/core/crtstartup: 0x6898f0, 0x689850) against the originals. parse_cmdline: no calls, the buffers and
# counts compared. _setargv: GetModuleFileNameA a COM recorder in its slot that fills the name buffer 0xfb1558 (or not), _nh_malloc and _amsg_exit cdecl recorders
# (the CrtInnerEnv's), parse_cmdline and malloc run.
# usage: crt_startup.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, fill, import_table, log_region, set_return  # noqa: E402

AMSG, NHMALLOC, MODULENAME = 26, 28, 14
B = Bench([(0x6898f0, 0x689aff), (0x689850, 0x6898ee), (0x67dcd0, 0x67dcf0)],
          hooks={0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True), 0x67efc0: _recorder(AMSG, 1, cdecl=True)})
COMMAND, WORDS, TEXT, COUNTS = RAM, RAM + 0x400, RAM + 0x800, RAM + 0xc00
ARGV = RAM + 0x1000
PIECES = [b'a', b'bc', b' ', b'  ', b'\t', b'"', b'""', b'\\', b'\\\\', b'\\\\\\', b'\\"', b'\\\\"', b'\\\\\\"', b'\x81x', b'\x81"', b'\x81 ', b'\x81\\', b'\xe9', b'word']
OPS = [n for n in ('parse', 'setargv') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def lead_bytes(r):
    return bytes(4 if 0x81 <= i <= 0x9f else r.choice([0, 1, 2, 8]) for i in range(256))


def command(r):
    return b''.join(r.choice(PIECES) for _ in range(r.randrange(14))) + r.choice([b'', b'', b'\x81'])


def parse(r):
    regions = {COMMAND: command(r) + bytes(1) + r.choice([bytes(0x10), b'after end\0'.ljust(0x10, b'\0')]), WORDS: r.bytes(0x100), TEXT: r.bytes(0x100), COUNTS: r.bytes(8)}
    regions[0xfb1669] = lead_bytes(r)
    return Call(0x6898f0, [COMMAND, r.choice([WORDS, WORDS, 0]), r.choice([TEXT, TEXT, 0]), COUNTS, COUNTS + 4], regions, conv='c')


def setargv(r):
    log = log_region()
    regions = {LOG: log, IMPORT_TABLE: import_table(log, {'name': (MODULENAME, 0x6fd194, 3, 0, 0)})}
    set_return(log, MODULENAME, r.getrandbits(32))
    if r.random() < 0.8:   # (the program's name)
        fill(log, MODULENAME, 1, r.choice([b'C:\\TP\\tp.exe\0', b'"C:\\a b\\tp.exe\0', b'\0', command(r)[:0x40] + b'\0']))
    regions[0xfb1558] = bytes(0x110)
    regions[COMMAND] = r.choice([b'', b'', command(r)]) + bytes(0x11)   # (empty: the program's name parsed)
    regions[0xfb3cb8] = struct.pack('<I', COMMAND)
    regions[0xfb1374] = r.bytes(8)
    regions[0xfb1390] = r.bytes(4)
    regions[0xfb1669] = lead_bytes(r)
    regions[ARGV] = r.bytes(0x400)
    regions[0xfb1548] = r.bytes(4)   # (malloc's new mode)
    set_return(log, NHMALLOC, ARGV if r.random() < 0.9 else 0)   # (0: _amsg_exit returns here, and parse_cmdline writes at the count's offset: a crash in both)
    return Call(0x689850, [], regions, conv='c', ret='v')


def case(r):
    return {'parse': parse, 'setargv': setargv}[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)
