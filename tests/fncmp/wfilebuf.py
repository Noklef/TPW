# The stream buffer's initialiser and its locale reference (src/game/core/wfilebuf: 0x4f5990, 0x4f5a50) against the originals: the locale 0x699da0 (agent B's) the cdecl recorder 15 of 0, the
# global's counted object [0xfb1dd0] with its count about -1 (the STL lock left out of both). Init: which 0, 1 or any; with 0, the old locale reference none or a counted object at
# its last use or not (its slot 0 a recorder), the file any, the global [0xfb34b4] any.
# usage: wfilebuf.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Locale,Init)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x4f5990, 0x4f5a4f), (0x4f5a50, 0x4f5acc), (0x4ef7b0, 0x4ef800), (0x5da3c0, 0x5da3c1)],
          hooks={0x699da0: _recorder(15, 0, cdecl=True), 0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})
BUF, OLD, OLDVT, GLOBAL = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300
OPS = [n for n in ('Locale', 'Init') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def case(r):
    op = r.deal(OPS)
    log = log_region()
    set_return(log, 15, r.getrandbits(32))
    counted = bytearray(r.bytes(0x10))
    put(counted, 4, 'I', r.choice([0, 1, 2, 0xfffffffe, 0xffffffff, r.getrandbits(32)]))
    regions = {LOG: log, BUF: bytearray(r.bytes(0x60)), 0xfb1dd0: struct.pack('<I', GLOBAL), GLOBAL: counted, 0xfb34b4: r.bytes(4)}
    if op == 'Locale':
        return Call(0x4f5a50, [BUF], regions, conv='t', ret='a')
    old = bytearray(r.bytes(0x10))
    put(old, 0, 'I', OLDVT)
    put(old, 4, 'I', r.choice([0, 1, 1, 2, 3, 0xfffffffe, 0xffffffff]))
    regions.update({OLD: old, OLDVT: vtable({0: 1})})
    put(regions[BUF], 0x4c, 'I', r.choice([0, OLD, OLD, OLD]))
    set_return(log, 0, r.getrandbits(32))
    return Call(0x4f5990, [BUF, r.getrandbits(32), r.choice([0, 0, 0, 1, 1, 2, r.getrandbits(32)])], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
