# The class 0x6fdaa4's three finds (src/game/core/class6fd9d8: 0x41f420) against the original, an old skip: the handle's find (slot 1 at +0x2c, as HasEmt's in class6fdaa4.py) logs each pattern's text and writes a result (0, 1 or negative); the first negative ends it with 0. Each find also dirties the record: "XY" through its name pointer, the pointer set to a sink. A find that then sees a name of "XY" (the name's text not emptied, or the pointer not reset) answers a negative, so the reset before the next find shows. The record's 0x20 bytes copied from a local never written (sic) are not compared.
# usage: class6fdaacheck.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, echo, fill, log_region, named, put, strings, vtable  # noqa: E402

FIND = 1
B = Bench([(0x41f420, 0x41f630)])
OBJECT, VT, TEXTS, NEGATIVE, SINK = RAM, RAM + 0x100, RAM + 0x140, RAM + 0x1a0, RAM + 0x1c0
PATTERNS = {0x78c440: b'*.a', 0x78c248: b'*.bb', 0x78c4e8: b'*.ccc'}


def case(r):
    log = log_region()
    obj = bytearray(r.bytes(0x60))
    put(obj, 0x2c, 'I', VT)
    strings(log, FIND, 1 << 1, 0b101)   # (the pattern's text; the status and record pointers as 0)
    echo(log, FIND, 0)
    named(log, FIND, 2, {b'XY': (NEGATIVE, b'')}, through=0x20)
    fill(log, FIND, 0, struct.pack('<3i', *[r.choice([0, 0, 0, 1, 0x7fffffff, -1, -0x80000000]) for _ in range(3)]), records=3)
    fill(log, FIND, 2, b'XY\0', which=1, through=0x20)
    fill(log, FIND, 2, struct.pack('<I', SINK), which=2, offset=0x20)
    regions = {LOG: log, OBJECT: obj, VT: vtable({FIND: 3}), NEGATIVE: struct.pack('<i', -1), SINK: b'XY\0' + r.bytes(5)}
    for k, (at, text) in enumerate(PATTERNS.items()):
        regions[TEXTS + 0x10 * k] = text + b'\0'
        regions[at] = struct.pack('<I', TEXTS + 0x10 * k)
    return Call(0x41f420, [OBJECT], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
