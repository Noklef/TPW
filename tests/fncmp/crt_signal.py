# The C library's raise and abort (src/game/core/crtsignal) against the originals: the signal lock and _getptd cdecl recorders; the handlers (the console ones and the thread's
# exception-action table) cdecl recorders or SIG_IGN; abort's message through _NMSG_WRITE (the Win32 calls COM recorders, the message table made here) and its _exit through doexit
# (the terminators cdecl recorders, ExitProcess a COM recorder that returns, as crt_exit.py has it).
# Bench limit: no handler is SIG_DFL, whose _exit returns here and leaves raise calling a null handler (a crash in both).
# usage: crt_signal.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, import_table, log_region, recorder, set_return, strings  # noqa: E402

GETPTD, LOCK, UNLOCK, HANDLER, FPE_HANDLER, TERM, BOX = 2, 7, 8, 9, 10, 11, 12
WRITE, STD, MODULE, EXITP, LOAD, FIND, TERMINATE, CURRENT = 14, 15, 16, 17, 18, 19, 20, 21
B = Bench([(0x68f780, 0x68f958), (0x68f990, 0x68f9cf), (0x68cc30, 0x68cc4f),
           (0x68a1b0, 0x68a386), (0x68efb0, 0x68f03f), (0x67ae00, 0x67aefe), (0x68a390, 0x68a437), (0x67f020, 0x67f191),
           (0x67f940, 0x67f952), (0x67f960, 0x67fa12), (0x67fa20, 0x67fa2b), (0x67fa30, 0x67fa3b), (0x67fa40, 0x67fa60)],
          hooks={0x682460: _recorder(GETPTD, 0, cdecl=True), 0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True)})
# (the listing stops at the calls that never return in the game: _exit's in raise and abort, doexit's ExitProcess; the exe's bytes after them given to the emulator)
B.emu.code.update({0x68f880: ('ADD', 'ESP,0x4'), 0x68cc4b: ('ADD', 'ESP,0x4'), 0x68cc4e: ('RET', ''),
                   0x67fa0e: ('POP', 'ESI'), 0x67fa0f: ('POP', 'EBP'), 0x67fa10: ('POP', 'EBX'), 0x67fa11: ('RET', '')})
B.emu.order = sorted(set(B.emu.order) | {0x68f880, 0x68cc4b, 0x68cc4e, 0x67fa0e, 0x67fa0f, 0x67fa10, 0x67fa11})
B.emu.next = dict(zip(B.emu.order, B.emu.order[1:]))
PTD, TABLE, TEXTS = RAM, RAM + 0x100, RAM + 0x400
OPS = [n for n in ('raise', 'raise', 'raise', 'raise', 'abort') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
SIGNALS = [2, 4, 8, 8, 11, 15, 21, 22, 0, 1, 3, 5, 23, 0xffffffff, 0x80000002]


def handler(r, sig):
    return 1 if r.random() < 0.3 else recorder(FPE_HANDLER, 2, cdecl=True) if sig == 8 else recorder(HANDLER, 1, cdecl=True)


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, IMPORT_TABLE: import_table(log, {'write': (WRITE, 0x6fd0e0, 5, 0b10, 0b1000), 'std': (STD, 0x6fd1ac, 1, 0, 0), 'module': (MODULE, 0x6fd194, 3, 0, 0b10), 'exit': (EXITP, 0x6fd244, 1, 0, 0),
                                                         'load': (LOAD, 0x6fd0b4, 1, 0, 0), 'find': (FIND, 0x6fd0b0, 2, 0, 0), 'terminate': (TERMINATE, 0x6fd24c, 2, 0, 0), 'current': (CURRENT, 0x6fd250, 0, 0, 0)})}
    set_return(log, GETPTD, PTD)
    # The thread's exception-action table (12-byte entries: code, signal, action), its count and the floating point entries' first and number.
    count = r.choice([3, 6, 9])
    entries = []
    for i in range(count):
        sig = r.choice([4, 8, 8, 11, 2]) if i >= 3 or r.random() < 0.1 else (4, 8, 11)[i]
        entries += [r.getrandbits(32), sig, handler(r, sig)]
    regions[TABLE] = struct.pack('<%dI' % (3 * count), *entries)
    first = r.randrange(count)
    regions[0x781508] = struct.pack('<4I', first, r.randrange(count - first + 1), r.getrandbits(32), count)
    regions[PTD] = r.bytes(0x50) + struct.pack('<3I', TABLE, r.getrandbits(32), r.choice([0x81, 0x8c, r.getrandbits(32)])) + r.bytes(4)
    regions[0xfb1918] = struct.pack('<4I', *[handler(r, 0) for _ in range(4)])
    if name == 'raise':
        return Call(0x68f780, [r.choice(SIGNALS)], regions, conv='c', ret='a')
    # abort: the message (as crt_message.py makes the table), then _exit's terminators.
    strings(log, BOX, 0b10)
    texts = bytearray(0x400)
    table = []
    for i, code in enumerate([2, 8, 9, 0xa, 0x10, 0x11, 0x12, 0x13, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0xfc, 0xff]):
        text = b'message %d\r\n' % code
        texts[0x20 * i:0x20 * i + len(text)] = text
        table += [code, TEXTS + 0x20 * i]
    regions[TEXTS] = texts
    regions[0x781680] = struct.pack('<38I', *table, 0xd2d0920, TEXTS)
    regions[0xfb135c] = struct.pack('<I', r.choice([0, 0, 1, 1, 2]))
    regions[0x77ea84] = struct.pack('<I', r.choice([1, 1, 2]))
    regions[0xfb1398] = r.bytes(8) + struct.pack('<I', r.choice([0, 0, 1, 2]))
    regions[0x72e0e8] = r.bytes(4) + bytes(0x14) + bytes(0x10) + struct.pack('<2I', *[r.choice([0, recorder(TERM, 0, cdecl=True)]) for _ in range(2)])
    return Call(0x68cc30, [], regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)
