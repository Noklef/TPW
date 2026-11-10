# The C library's entry point WinMainCRTStartup (src/game/core/crtstartup: 0x67ee10) against the original. Its callees are cdecl recorders at their game addresses
# (natively driver.cpp's kCrtStartupCalls, the same k), exit and fast_error_exit the driver's exit_case, WinMain 0x45a960 a COM recorder of 4; GetVersion, GetCommandLineA, GetStartupInfoA (its STARTUPINFO on the
# caller's stack: logged as 0, filled, or not) and GetModuleHandleA COM recorders in their import slots. _ismbblead answers in turn from a table. Not compared: the
# __except filter and handler (_XcptFilter 0x688800, then _exit): the emulator raises no exceptions.
# usage: crt_entry.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _exit_case, _recorder, answers, fill, import_table, log_region, set_return  # noqa: E402

HEAPINIT, MTINIT, IOINIT, MBCTABLE, ENVSTR, SETARGV, SETENVP, CINIT, LEAD, WINMAIN = 1, 2, 4, 5, 6, 7, 8, 9, 10, 12
VERSION, CMDLINE, STARTUP, MODULE = 13, 14, 15, 16
EXIT = 29   # (exit and fast_error_exit: the driver's exit_case, the case ends there; natively its exception passes the port's __except filter)
CALLEES = {0x6826b0: (HEAPINIT, 0), 0x6823e0: (MTINIT, 0), 0x687230: (IOINIT, 0), 0x68a160: (MBCTABLE, 0), 0x689b00: (ENVSTR, 0),
           0x689850: (SETARGV, 0), 0x689760: (SETENVP, 0), 0x67f8f0: (CINIT, 0), 0x689700: (LEAD, 1)}
HOOKS = {a: _recorder(k, n, cdecl=True) for a, (k, n) in CALLEES.items()}
HOOKS[0x45a960] = _recorder(WINMAIN, 4, com=True)
HOOKS[0x67eff0] = HOOKS[0x67f920] = _exit_case(EXIT)
B = Bench([(0x67ee10, 0x67ef6d), (0x67ef82, 0x67efb8)], hooks=HOOKS)
COMMAND, LEADS = RAM, RAM + 0x200
PIECES = [b'a', b'tp.exe', b' ', b'  ', b'\t', b'"', b'\x81', b'\x81"', b'-w', b'\x01', b'!']


def case(r):
    log = log_region()
    regions = {LOG: log, IMPORT_TABLE: import_table(log, {'version': (VERSION, 0x6fd240, 0, 0, 0), 'cmdline': (CMDLINE, 0x6fd1e4, 0, 0, 0),
                                                         'startup': (STARTUP, 0x6fd23c, 1, 0, 1), 'module': (MODULE, 0x6fd238, 1, 0, 0)})}
    set_return(log, VERSION, r.getrandbits(32))
    set_return(log, HEAPINIT, r.choice([1, 1, 1, 0]))
    set_return(log, MTINIT, r.choice([1, 1, 1, 0]))
    set_return(log, ENVSTR, r.choice([RAM + 0x300, RAM + 0x300, 0]))
    set_return(log, CMDLINE, r.choice([COMMAND, COMMAND, COMMAND, 0]))
    set_return(log, MODULE, r.getrandbits(32))
    set_return(log, WINMAIN, r.getrandbits(32))
    if r.random() < 0.7:   # (STARTUPINFO: else only the dwFlags the original clears)
        info = bytearray(r.bytes(0x44))
        info[0x2c] = r.choice([0, 1, 0xff, 0xfe])
        fill(log, STARTUP, 0, bytes(info))
    answers(log, LEAD, LEADS)
    regions[LEADS] = struct.pack('<40I', *[r.choice([0, 0, 1]) for _ in range(40)])
    regions[COMMAND] = b''.join(r.choice(PIECES) for _ in range(r.randrange(8))) + bytes(0x40)
    regions[0xfb1354] = r.bytes(4)
    regions[0xfb1364] = r.bytes(0x10)
    regions[0xfb3cb8] = r.bytes(4)
    return Call(0x67ee10, [], regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)
