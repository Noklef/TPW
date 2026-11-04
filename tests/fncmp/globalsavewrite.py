# The global save's file written (src/game/core/globalsave: 0x5afc60) against the original, the file's front ends and the serialise recorders (driver.cpp's kGlobalSaveCalls): the open failing, the version's write failing or short, the serialise failing, or all well.
# usage: globalsavewrite.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, echo, fill, log_region, set_return, strings  # noqa: E402

OPEN, WRITE, CLOSE, SERIALIZE = 1, 2, 3, 8
B = Bench([(0x5afc60, 0x5afd63), (0x5da3c0, 0x5da3c1)], hooks={0x5f5ad0: _recorder(OPEN, 4, cdecl=True), 0x5f5e50: _recorder(WRITE, 5, cdecl=True), 0x5f5b80: _recorder(CLOSE, 2, cdecl=True), 0x5aff50: _recorder(SERIALIZE, 3)})
SELF, PATH = RAM, RAM + 0x100
OPS = ['saved', 'saved', 'saved', 'no open', 'bad write', 'short write', 'long write', 'bad serialize', 'stream unset']


def good(r):
    return r.choice([0, 1, 0x7fffffff])


def case(r):
    op = r.deal(OPS)
    log = log_region()
    echo(log, OPEN, 0)
    echo(log, WRITE, 0)
    strings(log, OPEN, 4, 3)
    strings(log, WRITE, 0, 1 | 16)
    blob(log, WRITE, 2, 1)
    strings(log, CLOSE, 0, 1)
    fill(log, OPEN, 0, struct.pack('<i', -r.randrange(1, 1 << 31) if op == 'no open' else good(r)), which=0)
    if op != 'stream unset':
        fill(log, OPEN, 1, r.bytes(4), which=1)
    fill(log, WRITE, 0, struct.pack('<i', -r.randrange(1, 1 << 31) if op == 'bad write' else good(r)), which=2)
    fill(log, WRITE, 4, struct.pack('<I', r.randrange(0, 4) if op == 'short write' else 5 if op == 'long write' else 4), which=3)
    set_return(log, SERIALIZE, r.choice([1, 0x100, 0xffffffff]) if op == 'bad serialize' else 0)
    text = b'Save\\Global%d.sav\0' % r.randrange(100)
    regions = {LOG: log, SELF: r.bytes(0x50), PATH: text}
    return Call(0x5afc60, [SELF, PATH], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
