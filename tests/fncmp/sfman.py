# The SoundFont synthesiser (src/game/core/sfman) against the originals: the object at SELF (random bytes; the MIDI out handle 0 now and
# then, the programs' count small or any), its API table at API (cdecl recorders by slot), the WINMM and KERNEL32 imports recorders
# (midiOutShortMsg answering 0, failing, or failing on a pattern of calls: fail_mask()); the device word logged by its low word (the pushed
# register's high half is not the callee's), the stack word pair of slot 14 by its bytes. The system-exclusive messages' header on the stack
# is logged by its bytes 4-31 and the message it points to by the bytes the case's type sends (midiOutPrepareHeader the first 15,
# midiOutLongMsg bytes 4-18). midiOutGetDevCapsA fills the devices' names at +8 in turn (records: "SoundFont", near misses, others),
# midiOutOpen the handle. Slot 16's names land in the buffer its third argument's structure points to (fill(through=4); per call for
# the presets' count, some of them the string at 0x786de4 that ends it), the sink's slot 5 logs the name by its text; slots 11 and 16
# log their word pairs by bytes or as 0, the bank's {0x80, path} by its dwords.
# usage: sfman.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import (IMPORT_TABLE, LOG, LOG_BYTES, RAM, Bench, Call, blob, blob_bytes, fail_mask, fill, import_table, log_region, recorder,  # noqa: E402
                   set_return, strings, vtable)

B = Bench([(0x6d3f80, 0x6d3fa1), (0x6d4530, 0x6d4565), (0x6d4570, 0x6d4595), (0x6d45a0, 0x6d46da), (0x6d3fb0, 0x6d41a8), (0x6d4240, 0x6d4339),
           (0x67c250, 0x67c2a0), (0x6d4340, 0x6d43b2), (0x6d43c0, 0x6d449c), (0x6d44a0, 0x6d4522),
           (0x6d3bb0, 0x6d3bb5), (0x6d3bc0, 0x6d3c82), (0x6d3c90, 0x6d3f1e), (0x6d3f20, 0x6d3f80)])
SELF, API, OUT, MSG = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300
SHORT_K, PREP_K, LONG_K, CLOSE_K, OPEN_K, CAPS_K, DEVS_K, LOAD_K, PROC_K, FREE_K = 40, 41, 42, 43, 44, 45, 46, 47, 48, 49
IMPORTS = {'midiOutShortMsg': (SHORT_K, 0x6fd3cc, 2, 0, 0), 'midiOutPrepareHeader': (PREP_K, 0x6fd3b8, 3, 0, 0),
           'midiOutLongMsg': (LONG_K, 0x6fd3d0, 3, 0, 0), 'midiOutOpen': (OPEN_K, 0x6fd3c0, 5, 0, 0),
           'midiOutGetDevCapsA': (CAPS_K, 0x6fd3c4, 3, 0, 2), 'midiOutGetNumDevs': (DEVS_K, 0x6fd3c8, 0, 0, 0),
           'midiOutClose': (CLOSE_K, 0x6fd3bc, 1, 0, 0), 'LoadLibraryA': (LOAD_K, 0x6fd0b4, 1, 1, 0), 'GetProcAddress': (PROC_K, 0x6fd0b0, 2, 2, 0),
           'FreeLibrary': (FREE_K, 0x6fd0a8, 1, 0, 0)}
NAMES = [b'SoundFont\0\0\0', b'SoundFont 2\0', b'Soundfont\0\0\0', b'SoundFon\0\0\0\0', b'MPU-401\0\0\0\0\0', b'SoundFontXYZ']
LENGTHS = {0: 0xc, 2: 0xd, 4: 0x13, 0x7f: 8}
API_ARGS = {0: 1, 1: 2, 2: 2, 4: 1, 5: 1, 7: 3, 11: 3, 14: 2, 16: 3}
SINK, EMPTY, PATH = RAM + 0x400, 0x786de4, RAM + 0x500
SINK_K = 25   # (the sink's slot 5)
MANAGER, MANAGER_K = RAM + 0x600, 20   # the "SFManager" export: a structure whose first dword is the query (cdecl)
FLAGS = [0x40000000, 0x80000000, 0xc0000000, 0, 0, 0, 0x3fffffff]
CALLS = {'pack': 0x6d3f80, 'unload': 0x6d4530, 'notesoff': 0x6d4570, 'send': 0x6d45a0, 'sysex': 0x6d3fb0, 'openout': 0x6d4240,
         'getname': 0x6d4340, 'count': 0x6d43c0, 'loadbank': 0x6d44a0, 'dtor': 0x6d3bb0, 'open': 0x6d3bc0,
         'select': 0x6d3c90, 'close': 0x6d3f20}
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


def openout(r, log, me, unset=0.85):
    """midiOutGetNumDevs' count, midiOutGetDevCapsA's names (per call), midiOutOpen's handle and answers, the message's."""
    if r.random() < unset:
        struct.pack_into('<I', me, 0x10, 0)
    set_return(log, DEVS_K, r.choice([0, 0x10000, r.getrandbits(16) << 16 | r.randrange(1, 40)] + [r.randrange(1, 12)] * 6 + [0x10000 | r.randrange(1, 5)]))
    names = [r.choice(NAMES + [r.bytes(12)]) for _ in range(r.randrange(1, 10))]
    fill(log, CAPS_K, 1, b''.join(names), 0, offset=8, records=len(names))
    fill(log, OPEN_K, 0, struct.pack('<I', r.choice([0, r.getrandbits(32), r.getrandbits(32)])), 1)
    answers(r, log, OPEN_K)
    answers(r, log, PREP_K)
    answers(r, log, LONG_K)


def select(r, log, length):
    """The API's slots for the device selection: 1 the capabilities' flags (per call), 4 the opens' answers, 7 the free memory (per call), 2
    the version; the message logged by its first `length` bytes."""
    strings(log, 1, 0, out=2)
    flags = [r.choice(FLAGS + [r.getrandbits(32)]) for _ in range(r.randrange(1, 17))]
    fill(log, 1, 1, struct.pack('<%dI' % len(flags), *flags), 4, offset=6, records=len(flags))
    how = r.randrange(4)
    if how == 0:
        set_return(log, 4, 0)
    elif how == 1:
        fail_mask(log, 4, r.getrandbits(20))
    elif how == 2:
        fail_mask(log, 4, (1 << r.randrange(20)) - 1)
    strings(log, 7, 0, out=6)
    free = [r.choice([r.getrandbits(32), r.getrandbits(19), r.getrandbits(24), 0x60 << 19 | r.getrandbits(19)]) for _ in range(r.randrange(1, 17))]
    fill(log, 7, 2, struct.pack('<%dI' % len(free), *free), 5, records=len(free))
    strings(log, 2, 0, out=2)
    fill(log, 2, 1, struct.pack('<I', r.getrandbits(32)), 6)
    sysex_log(log, length)


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
        openout(r, log, me)
        sysex_log(log, 8)
        return Call(0x6d4240, [SELF, OUT], regions, conv='t', ret='a')
    if name == 'select':
        select(r, log, 0xc)
        count = r.choice([0, 0x10000, r.getrandbits(16) << 16 | r.randrange(1, 17)] + [r.randrange(1, 17)] * 8)
        return Call(0x6d3c90, [SELF, OUT, count], regions, conv='t', ret='a')
    if name == 'open':
        me[0x14] = r.choice([0, 0, 0, 0, 0, 1, r.getrandbits(8)])
        set_return(log, LOAD_K, r.choice([0] + [r.getrandbits(32)] * 6))
        set_return(log, PROC_K, r.choice([0] + [MANAGER] * 8))
        regions[MANAGER] = struct.pack('<I', recorder(MANAGER_K, 2, cdecl=True))
        fill(log, MANAGER_K, 1, struct.pack('<I', API if r.random() < 0.97 else 0), 2)
        set_return(log, MANAGER_K, r.choice([0] * 8 + [1, r.getrandbits(32)]))
        strings(log, 0, 0, out=1)
        fill(log, 0, 0, struct.pack('<I', r.choice([0, r.getrandbits(16) << 16 | r.randrange(1, 17)] + [r.randrange(1, 17)] * 6)), 3)
        openout(r, log, me, 0.97)
        select(r, log, 8)
        return Call(0x6d3bc0, [SELF, OUT], regions, conv='t', ret='a')
    if name in ('close', 'dtor'):
        me[0x14] = r.choice([1, 1, 1, 0, r.getrandbits(8)])
        word(log, 5, 0)
        return Call(CALLS[name], [SELF], regions, conv='t', ret='a')
    if name in ('getname', 'count', 'loadbank'):
        word(log, 16, 0)
        blob_bytes(log, 16, 1, 0xf)
        strings(log, 16, 0, out=4)
        if name == 'getname':
            data = r.bytes(r.randrange(0, 0x3f)).replace(b'\0', b'a') + b'\0'
            fill(log, 16, 2, data[:0x40], 2, through=4)
            set_return(log, 16, r.choice([0, 0, 0, 1, r.getrandbits(32)]))
            sink = bytearray(vtable({}))
            struct.pack_into('<I', sink, 0x14, recorder(SINK_K, 1))
            strings(log, SINK_K, 1)
            regions.update({SINK: struct.pack('<I', SINK + 4) + sink})
            return Call(0x6d4340, [SELF, OUT, r.getrandbits(32), SINK], regions, conv='t', ret='a')
        empty = r.choice([b'', b'Empty', b'none', r.bytes(r.randrange(1, 8)).replace(b'\0', b'x')]) + b'\0'
        regions[EMPTY] = empty + r.bytes(0x40 - len(empty))
        names = [r.choice([empty, empty, b'Piano\0', b'Organ 2\0', b'Empty\0', b'\0']) for _ in range(r.randrange(1, 15))]
        names = [(n + bytes(8))[:8] for n in names]
        fill(log, 16, 2, b''.join(names), 2, records=len(names), through=4)
        how = r.randrange(3)
        if how == 0:
            set_return(log, 16, 0)
        elif how == 1:
            fail_mask(log, 16, 1 << r.randrange(40))
        if name == 'count':
            return Call(0x6d43c0, [SELF], regions, conv='t', ret='a')
        word(log, 11, 0)
        strings(log, 11, 0, out=2)
        blob(log, 11, 2, 3)
        set_return(log, 11, r.choice([0, 0, 0, 1, r.getrandbits(32)]))
        me[0x14] = r.choice([1, 1, 1, 0, r.getrandbits(8)])
        regions[PATH] = r.bytes(0x10)
        return Call(0x6d44a0, [SELF, OUT, PATH, r.getrandbits(32)], regions, conv='t', ret='a')
    if name == 'send':
        program = r.choice([r.randrange(0, 0x100), r.getrandbits(32), struct.unpack_from('<I', me, 0x18)[0]])
        flags = r.choice([1 << r.randrange(7), r.getrandbits(8), r.getrandbits(32), 0])
        return Call(0x6d45a0, [SELF, OUT, program, flags, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')
    return Call(CALLS[name], [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
