# The online chat's flags under its lock (src/game/online/chat: 0x4e6130, 0x4e6150) against the originals: a chat of random bytes, its
# critical section (+0x48) entered and left through recording imports.
# Ctor (0x4e5260, the constructor): a chat of random bytes; its critical sections made through a recording import; stringmapsrecord.py's
# trees' nil nodes made already (any counter) or not; rand() the fixed stand-in (fncmp.RAND) through the CRT's swprintf; the global string
# 0x7a1500 null, empty or 1-0x7f characters in a block whose count byte is 0-2 or 0xfd-0xff (shared or copied), npos (0x6fe5ac) now and then
# small; mbtowc (0x687690, which swprintf's %08x goes through) the C locale's in Python; the four allocator bytes the original copies from an uninitialised stack byte (and the record's) masked.
# usage: onlinechat.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Set,Get,Ctor)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, Mask, heap, import_table, log_region, put  # noqa: E402



def _mbtowc(e):   # cdecl (to, from, n) -> the C locale's mbtowc (0x687690; the original takes the locale lock through InterlockedIncrement,
    sp = e.r['ESP']   # which the driver's own swprintf (crt.cpp) never does): _woutput writes %x's digits through it
    to, frm, n = e.u32(sp), e.u32(sp + 4), e.u32(sp + 8)
    c = e.read(frm, 1)[0] if frm and n else None
    if c is not None and to:
        e.write(to, struct.pack('<H', c))
    e.r['EAX'] = 0 if not frm or not n or c == 0 else 1


B = Bench([(0x4e6130, 0x4e6150), (0x4e6150, 0x4e616f), (0x5c8bb0, 0x5c8bd0),
           (0x405280, 0x405390), (0x45b590, 0x45b5f0), (0x45b680, 0x45b8e0), (0x47bad0, 0x47bc10), (0x4e5260, 0x4e54d0), (0x4f0010, 0x4f0100),
           (0x4f1400, 0x4f1530), (0x4f23f0, 0x4f2450), (0x4f4120, 0x4f41e0), (0x4f4b70, 0x4f4c30), (0x4f5080, 0x4f5140), (0x5c8b90, 0x5c8bb0),
           (0x5ec210, 0x5ec230), (0x67a400, 0x6902c0), (0x6918f0, 0x691a30), (0x691ba0, 0x691ce0)],
          hooks={0x687690: _mbtowc, 0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})   # (the STL lock, as stringmapsrecord.py's)
CALLS = {'Set': 0x4e6130, 'Get': 0x4e6150, 'Ctor': 0x4e5260}
IMPORTS = {'EnterCriticalSection': (1, 0x6fd140, 1, 0, 0), 'LeaveCriticalSection': (2, 0x6fd130, 1, 0, 0)}
CHAT, NILS, BLOCK = RAM + 0x10000, RAM + 0x400, RAM + 0x800
TREES = [(0x7ced24, 0x7ced28), (0x7ced2c, 0x7ced30), (0x7ced34, 0x7ced38)]   # (stringmapsrecord.py's kinds D, C, E: nil global, map counter)
MASKED = [8, 0x18, 0xec8, 0xefc] + [0xe60 + k for k in (0, 0x10, 0x20, 0x21, 0x30, 0x31, 0x40, 0x41, 0x54)]


def ctor(r):
    log = log_region()
    regions = {CHAT: r.bytes(0xf0c), HEAP: heap(), LOG: log,
               IMPORT_TABLE: import_table(log, {'InitializeCriticalSection': (3, 0x6fd144, 1, 0, 0)})}
    glob = bytearray(r.bytes(0x20))
    for t, (nil_at, counter) in enumerate(TREES):
        nil = NILS + 0x40 * t
        regions[nil] = r.bytes(0x40)
        struct.pack_into('<II', glob, nil_at - 0x7ced20, r.choice([nil, nil, 0]), r.choice([0, 1, 2, 5, r.getrandbits(32)]))
    regions[0x7ced20] = glob
    n = r.choice([0, 1, 5, 31, 32, r.randrange(1, 0x80)])
    block = bytearray(r.bytes(0x100))
    block[0] = r.choice([0, 0, 1, 2, 0xfd, 0xfe, 0xff])
    block[1:1 + n] = bytes(r.randrange(1, 256) for _ in range(n))
    block[1 + n] = 0
    ptr = 0 if n == 0 and r.random() < 0.5 else BLOCK + 1
    regions[BLOCK] = block
    regions[0x7a1500] = struct.pack('<4I', r.getrandbits(32), ptr, n, r.choice([n, max(n, 31), n + r.randrange(40)]))
    if r.random() < 0.2:   # (npos small: the copy cut short)
        regions[0x6fe5ac] = put(bytearray(4), 0, 'I', r.choice([0, 1, 5, 0x20]))
    return Call(0x4e5260, [CHAT], regions, conv='t', ret='a', masks=[Mask(CHAT, MASKED, 'the allocator bytes the original copies from uninitialised stack (sic)')])


def case(r):
    op = r.choice(OPS)
    if op == 'Ctor':
        return ctor(r)
    log = log_region()
    regions = {RAM: r.bytes(0x100), LOG: log, IMPORT_TABLE: import_table(log, IMPORTS)}
    return Call(CALLS[op], [RAM], regions, conv='t', ret='b' if op == 'Get' else 'v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
