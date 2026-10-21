# The SoundFont synthesiser (src/game/core/sfman) against the originals: the object at SELF (random bytes; the MIDI out handle 0 now and
# then, the programs' count small or any), its API table at API (cdecl recorders by slot), the WINMM and KERNEL32 imports recorders
# (midiOutShortMsg answering 0, failing, or failing on a pattern of calls: fail_mask()); the device word logged by its low word (the pushed
# register's high half is not the callee's), the stack word pair of slot 14 by its bytes. The system-exclusive messages' header on the stack
# is logged by its bytes 4-31 and the message it points to by the bytes the case's type sends (midiOutPrepareHeader the first 15,
# midiOutLongMsg bytes 4-18). midiOutGetDevCapsA fills the devices' names at +8 in turn (records: "SoundFont", near misses, others),
# midiOutOpen the handle.
# usage: sfman.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import (IMPORT_TABLE, LOG, LOG_BYTES, RAM, Bench, Call, blob_bytes, fail_mask, fill, import_table, log_region, set_return,  # noqa: E402
                   vtable)

B = Bench([(0x6d3f80, 0x6d3fa1), (0x6d4530, 0x6d4565), (0x6d4570, 0x6d4595), (0x6d45a0, 0x6d46da), (0x6d3fb0, 0x6d41a8), (0x6d4240, 0x6d4339),
           (0x67c250, 0x67c2a0)])
SELF, API, OUT, MSG = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300
SHORT_K, PREP_K, LONG_K, OPEN_K, CAPS_K, DEVS_K = 40, 41, 42, 44, 45, 46
IMPORTS = {'midiOutShortMsg': (SHORT_K, 0x6fd3cc, 2, 0, 0), 'midiOutPrepareHeader': (PREP_K, 0x6fd3b8, 3, 0, 0),
           'midiOutLongMsg': (LONG_K, 0x6fd3d0, 3, 0, 0), 'midiOutOpen': (OPEN_K, 0x6fd3c0, 5, 0, 0),
           'midiOutGetDevCapsA': (CAPS_K, 0x6fd3c4, 3, 0, 2), 'midiOutGetNumDevs': (DEVS_K, 0x6fd3c8, 0, 0, 0)}
NAMES = [b'SoundFont\0\0\0', b'SoundFont 2\0', b'Soundfont\0\0\0', b'SoundFon\0\0\0\0', b'MPU-401\0\0\0\0\0', b'SoundFontXYZ']
LENGTHS = {0: 0xc, 2: 0xd, 4: 0x13, 0x7f: 8}
API_ARGS = {0: 1, 1: 2, 2: 2, 4: 1, 5: 1, 7: 3, 14: 2}
CALLS = {'pack': 0x6d3f80, 'setload': 0x6d4530, 'notesoff': 0x6d4570, 'send': 0x6d45a0, 'sysex': 0x6d3fb0, 'openout': 0x6d4240}
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def word(log, k, arg):
    struct.pack_into('<I', log, LOG_BYTES + 4 * k, 1 << (16 + arg) | struct.unpack_from('<I', log, LOG_BYTES + 4 * k)[0])


def sysex_log(log, length):
    """The header's bytes 4-31 and the message's bytes up to its length (two windows of 15)."""
    blob_bytes(log, PREP_K, 1, 0xfffffff0, (0, (1 << min(length, 15)) - 1))
    blob_bytes(log, LONG_K, 1, 0xfffffff0, (0, (1 << max(0, min(length - 4, 15))) - 1, 4))


def answers(r, log, k):
    how = r.randrange(4)
    if how == 0:
        set_return(log, k, 0)
    elif how == 1:
        fail_mask(log, k, r.getrandbits(5))
    elif how == 2:
        fail_mask(log, k, 1 << r.randrange(5))


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
    if name == 'sysex':
        msg = bytearray(r.bytes(0x10))
        kind = r.choice([0, 2, 4, 0x7f] * 3 + [r.randrange(0x80), r.getrandbits(32), 1, 0x80])
        struct.pack_into('<I', msg, 0, kind)
        regions[MSG] = msg
        sysex_log(log, LENGTHS.get(kind, 0))
        answers(r, log, PREP_K)
        answers(r, log, LONG_K)
        return Call(0x6d3fb0, [SELF, OUT, r.getrandbits(32), MSG if r.random() < 0.95 else 0], regions, conv='t', ret='a')
    if name == 'openout':
        if r.random() < 0.85:
            struct.pack_into('<I', me, 0x10, 0)
        set_return(log, DEVS_K, r.choice([0, 0x10000, r.getrandbits(32)] + [r.randrange(1, 12)] * 6 + [0x10000 | r.randrange(1, 5)]))
        names = [r.choice(NAMES + [r.bytes(12)]) for _ in range(r.randrange(1, 10))]
        fill(log, CAPS_K, 1, b''.join(names), 0, offset=8, records=len(names))
        fill(log, OPEN_K, 0, struct.pack('<I', r.choice([0, r.getrandbits(32), r.getrandbits(32)])), 1)
        answers(r, log, OPEN_K)
        sysex_log(log, 8)
        answers(r, log, PREP_K)
        answers(r, log, LONG_K)
        return Call(0x6d4240, [SELF, OUT], regions, conv='t', ret='a')
    if name == 'send':
        program = r.choice([r.randrange(0, 0x100), r.getrandbits(32), struct.unpack_from('<I', me, 0x18)[0]])
        flags = r.choice([1 << r.randrange(7), r.getrandbits(8), r.getrandbits(32), 0])
        return Call(0x6d45a0, [SELF, OUT, program, flags, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')
    return Call(CALLS[name], [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
