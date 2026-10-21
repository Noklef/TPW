# The SoundFont synthesiser (src/game/core/sfman) against the originals: the object at SELF (random bytes; the MIDI out handle 0 now and
# then, the programs' count small or any), its API table at API (cdecl recorders by slot), the WINMM and KERNEL32 imports recorders
# (midiOutShortMsg answering 0, failing, or failing on a pattern of calls: fail_mask()); the device word logged by its low word (the pushed
# register's high half is not the callee's), the stack word pair of slot 14 by its bytes.
# usage: sfman.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import (IMPORT_TABLE, LOG, LOG_BYTES, RAM, Bench, Call, blob_bytes, fail_mask, import_table, log_region, set_return,  # noqa: E402
                   vtable)

B = Bench([(0x6d3f80, 0x6d3fa1), (0x6d4530, 0x6d4565), (0x6d4570, 0x6d4595), (0x6d45a0, 0x6d46da)])
SELF, API, OUT = RAM, RAM + 0x100, RAM + 0x200
SHORT_K = 40
IMPORTS = {'midiOutShortMsg': (SHORT_K, 0x6fd3cc, 2, 0, 0)}
API_ARGS = {0: 1, 1: 2, 2: 2, 4: 1, 5: 1, 7: 3, 14: 2}
CALLS = {'pack': 0x6d3f80, 'setload': 0x6d4530, 'notesoff': 0x6d4570, 'send': 0x6d45a0}
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def word(log, k, arg):
    struct.pack_into('<I', log, LOG_BYTES + 4 * k, 1 << (16 + arg) | struct.unpack_from('<I', log, LOG_BYTES + 4 * k)[0])


def env(r):
    log = log_region()
    me = bytearray(r.bytes(0x20))
    struct.pack_into('<I', me, 8, API if r.random() < 0.9 else 0)
    struct.pack_into('<I', me, 0x10, r.choice([0, r.getrandbits(32), r.getrandbits(32)]))
    struct.pack_into('<I', me, 0x18, r.choice([r.randrange(0, 0x100), r.getrandbits(32)]))
    how = r.randrange(4)
    if how == 0:
        set_return(log, SHORT_K, 0)
    elif how == 1:
        fail_mask(log, SHORT_K, r.getrandbits(5))
    elif how == 2:
        fail_mask(log, SHORT_K, 1 << r.randrange(5))
    word(log, 14, 0)
    blob_bytes(log, 14, 1, 0xf)
    if r.random() < 0.5:
        set_return(log, 14, r.choice([0, 1, 0xffffffff]))
    regions = {SELF: me, API: vtable({}, cdecl=API_ARGS), OUT: r.bytes(4), LOG: log, IMPORT_TABLE: import_table(log, IMPORTS)}
    return log, me, regions


def case(r):
    name = r.deal(OPS)
    log, me, regions = env(r)
    if name == 'pack':
        a = [r.choice([r.getrandbits(8), r.getrandbits(32), r.randrange(0x80, 0x100)]) for _ in range(3)]
        return Call(0x6d3f80, a, {OUT: r.bytes(4)}, conv='s', ret='a')
    if name == 'send':
        program = r.choice([r.randrange(0, 0x100), r.getrandbits(32), struct.unpack_from('<I', me, 0x18)[0]])
        flags = r.choice([1 << r.randrange(7), r.getrandbits(8), r.getrandbits(32), 0])
        return Call(0x6d45a0, [SELF, OUT, program, flags, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')
    return Call(CALLS[name], [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
