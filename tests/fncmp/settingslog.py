# The render settings' lines in the debug log (src/game/render/settingslog: 0x5833c0, 0x5835c0, 0x584790) against the originals: the debug log itself and the C library's sprintf real, its file calls recorders (enginedebug.py), random settings with floats short enough for the stack's lines.
# usage: settingslog.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import enginedebug  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, log_region, put  # noqa: E402

CALLS = {'Display': (0x5833c0, 0x5834fb), 'System': (0x5835c0, 0x5836a6), 'Texturing': (0x584790, 0x5848a9)}
B = Bench(list(CALLS.values()) + [enginedebug.RANGE, (0x67a400, 0x6902c0)], hooks=enginedebug.HOOKS)
SETTINGS = RAM
OPS = list(CALLS)
FLOAT_TABLE = struct.pack('<6I', 0x680a30, 0x6805d0, 0x680660, 0x680570, 0x680640, 0x680a30)   # (the C library's float printers: its start-up at 0x67a610 sets them)


def case(r):
    op = r.deal(OPS)
    log = log_region()
    enginedebug.texts(log)
    settings = bytearray(r.bytes(0x4100))
    put(settings, 0, 'I', r.getrandbits(32) & r.choice([0, 0xffffffff, r.getrandbits(32)]))
    put(settings, 0x14, 'i', r.choice([r.randrange(-5000, 5000), r.randrange(200000, 500000), r.randrange(-1 << 31, 1 << 31)]))
    span = r.choice([1.0, 10.0, 100.0]) if op == 'Display' else r.choice([1.0, 10.0])
    for offset in (0x48, 0x4c, 0x50, 0x54, 0x8f8, 0x40d8, 0x40e0, 0x40e4, 0x40e8, 0x40ec):
        put(settings, offset, 'f', r.uniform(-span, span))
    return Call(CALLS[op][0], [SETTINGS], {LOG: log, SETTINGS: settings, 0x77eca0: FLOAT_TABLE, 0x879a2c: struct.pack('<I', r.choice([0, 1, 2]))}, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
