# The engine's error list (src/game/render/engineerrors: 0x56af20) against the original: a list with a count of 0 to 0x40 mostly (now and then past
# it; a negative one writes before the list), kind 3 often with the debug flag (0x1000000 at 0x8bd508) set half the time, codes the result messages
# know mostly (as dxerrors.py's) or random. The debug log 0x564650 is fncmp's one-argument cdecl logger (hooked in the emulator, the driver's EngineErrors env natively), its text
# hashed; the message goes through the CRT's sprintf in the original (the codes whose message overruns its buffer are not logged).
# usage: engineerrors.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import LOG, RAM, Bench, Call, _cdecl_log, log_region, strings  # noqa: E402

B = Bench([(0x56af20, 0x56af9c), (0x56a770, 0x56ae60), (0x67a400, 0x690000)], hooks={0x564650: _cdecl_log})
LIST = RAM
CODES = [0x88760000 + k for k in range(0x300)] + [(0xb0754000 + k) & 0xffffffff for k in range(-2, 0x1c)] * 8 + [0, 0x80004005, 0x8007000e, 0x80070057]
# The codes whose message is 100 characters or more (the longest 165): never logged here.
LONG = {0x887600b4, 0x887600d2, 0x887600dc, 0x887600e1, 0x88760118, 0x8876013c, 0x8876013d, 0x8876014a, 0x8876014f, 0x8876015e, 0x887601c2, 0x8876021c,
        0x88760233, 0x88760239, 0x8876023a, 0x8876023b, 0x88760242, 0x88760243, 0x88760248}


def case(r):
    log = log_region()
    strings(log, 0x3f, 1)
    data = bytearray(r.bytes(0x204))
    struct.pack_into('<i', data, 0x200, r.choice([r.randrange(0x41), r.randrange(0x41), 0, 0x3f, 0x40, 0x41, 0x7fffffff]))
    code = r.choice(CODES) if r.random() < 0.9 else r.getrandbits(32)
    flags = r.getrandbits(32) & ~0x1000000 | (0x1000000 if r.random() < 0.5 else 0)
    kind = r.choice([3, 3, 3, 2, 1, 0, 4, r.getrandbits(32)])
    if code in LONG:   # (its message overruns the 100-byte buffer in both: logged, it would smash the stack)
        flags &= ~0x1000000
    return Call(0x56af20, [LIST, kind, code], {LIST: data, LOG: log, 0x8bd508: struct.pack('<I', flags)}, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
