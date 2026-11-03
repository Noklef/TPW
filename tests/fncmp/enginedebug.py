# The engine's debug log (src/game/render/enginedebug: EngineDebug_Log 0x564650) against the original: the C library's file calls recorders (no file is written), fopen's answer a file or none,
# the count 0x879a2c 0 or not. Its callers' benches take RANGE and HOOKS and call texts(log), for a text in a stack buffer.
# usage: enginedebug.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, set_return, strings  # noqa: E402

FOPEN, FPRINTF, FFLUSH, FCLOSE = 28, 29, 30, 31   # (driver.cpp's kEngineDebugEnv)
RANGE = (0x564650, 0x5646ba)
HOOKS = {0x67b8a0: _recorder(FOPEN, 2, cdecl=True), 0x67dad0: _recorder(FPRINTF, 3, cdecl=True), 0x67d070: _recorder(FFLUSH, 1, cdecl=True), 0x67b620: _recorder(FCLOSE, 1, cdecl=True)}
TEXT = RAM + 0x100
B = Bench([RANGE], hooks=HOOKS)


def texts(log):
    """The text is logged by what it says, not its address."""
    strings(log, FPRINTF, 1 << 2)


def case(r):
    log = log_region()
    texts(log)
    if r.random() < 0.3:
        set_return(log, FOPEN, 0)
    count = r.choice([0, 0, 1, 2, r.getrandbits(32)])
    return Call(0x564650, [TEXT], {LOG: log, 0x879a2c: count.to_bytes(4, 'little'), TEXT: r.bytes(r.randrange(1, 40)) + b'\0'})


if __name__ == '__main__':
    B.main(case)
