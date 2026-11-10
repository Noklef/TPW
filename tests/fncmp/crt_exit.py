# The C library's _cinit, exit, _exit, doexit and the exit lock (src/game/core/crtstartup) against the originals. lock / unlock are cdecl recorders at their game
# addresses; TerminateProcess, GetCurrentProcess and ExitProcess COM recorders in their import slots (ExitProcess returns: the original then returns too). The
# onexit table, the initialiser and terminator tables and the floating point pointer 0x77e718 hold cdecl recorders of their own k, or 0, so the log has their order.
# usage: crt_exit.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, import_table, log_region, recorder  # noqa: E402

LOCK, UNLOCK = 7, 8
ONEXIT, PRETERM, TERM, CINIT, CXXINIT, FPINIT = (14, 15, 16), 17, 18, 19, 23, 24
TERMINATE, CURRENT, EXITPROCESS = 20, 21, 22
B = Bench([(0x67f8f0, 0x67f920), (0x67f920, 0x67f932), (0x67f940, 0x67f952), (0x67f960, 0x67fa12), (0x67fa20, 0x67fa2b), (0x67fa30, 0x67fa3b), (0x67fa40, 0x67fa60)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True)})
# (the listing stops at doexit's CALL [ExitProcess] 0x67fa08, taken for no-return: its tail, the exe's 5e 5d 5b c3, given to the emulator here)
B.emu.code.update({0x67fa0e: ('POP', 'ESI'), 0x67fa0f: ('POP', 'EBP'), 0x67fa10: ('POP', 'EBX'), 0x67fa11: ('RET', '')})
B.emu.next.update({0x67fa08: 0x67fa0e, 0x67fa0e: 0x67fa0f, 0x67fa0f: 0x67fa10, 0x67fa10: 0x67fa11, 0x67fa11: 0x67fa12})
OPS = [n for n in ('doexit', 'doexit', 'doexit', 'exit', '_exit', 'cinit', 'lockexit', 'unlockexit') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
TABLE = RAM
ADDRESS = {'cinit': 0x67f8f0, 'exit': 0x67f920, '_exit': 0x67f940, 'doexit': 0x67f960, 'lockexit': 0x67fa20, 'unlockexit': 0x67fa30}


def entries(r, n, ks):
    """n table entries: each 0 or a cdecl recorder of one of ks."""
    return struct.pack('<%dI' % n, *[0 if r.random() < 0.3 else recorder(r.choice(ks), 0, cdecl=True) for _ in range(n)])


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, IMPORT_TABLE: import_table(log, {'terminate': (TERMINATE, 0x6fd24c, 2, 0, 0), 'current': (CURRENT, 0x6fd250, 0, 0, 0), 'exitprocess': (EXITPROCESS, 0x6fd244, 1, 0, 0)})}
    # (the second-exit flag at 0xfb13a0, 0xfb139c and the byte 0xfb1398 the routine sets)
    regions[0xfb1398] = r.bytes(8) + struct.pack('<I', r.choice([0, 0, 1, 2]))
    used = r.choice([0, 1, 3, 6])
    begin = r.choice([TABLE, TABLE, TABLE, 0])
    regions[0xfb3cb0] = struct.pack('<II', TABLE + used * 4, begin)
    regions[TABLE] = entries(r, 8, ONEXIT)
    # (0x72e0e8: the C++ table's end; the C initialisers 0x72e0ec-0x72e0fc, pre-terminators 0x72e100-0x72e10c, terminators 0x72e110-0x72e118)
    regions[0x72e0e8] = r.bytes(4) + entries(r, 4, (CINIT,)) + bytes(4) + entries(r, 3, (PRETERM,)) + bytes(4) + entries(r, 2, (TERM,))
    regions[0x77e718] = struct.pack('<I', r.choice([0, recorder(FPINIT, 0, cdecl=True)]))
    if name == 'cinit':
        table = bytearray(0x72e0e8 - 0x727000)
        for _ in range(r.choice([0, 1, 3])):
            struct.pack_into('<I', table, 4 * r.choice([0, len(table) // 4 - 1, r.randrange(len(table) // 4)]), recorder(CXXINIT, 0, cdecl=True))
        regions[0x727000] = table
    code = r.choice([0, 1, 3, r.getrandbits(32)])
    args = [] if name in ('cinit', 'lockexit', 'unlockexit') else [code] if name in ('exit', '_exit') else \
        [code, r.choice([0, 0, 1, 0x100]), r.choice([0, 0, 1, 0x100, 0xff00])]
    return Call(ADDRESS[name], args, regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)
