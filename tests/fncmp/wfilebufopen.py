# A wide file buffer's open and its conversion (src/game/core/wfilebuf: 0x4f5d60, 0x4f7180, 0x4f5ad0, 0x4f3920, 0x4f5dd0, 0x4f5e80) against the originals. Recorders: _Mbrtowc, _Wcrtomb, _Locinfo's constructor and destructor,
# locale::_Addfac (it answers a locale; this logged as 0), the throw, the file calls of __Fiopen, the facet's always_noconv; a facet made here gets recorders through a copy of its vtable at 0x709ce4.
# Masked: the new string's allocator byte (the original stores a stack byte; 0x4f5cdd's store is all, nothing reads it: StdString's allocator is empty).
# usage: wfilebufopen.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, Mask, _cdecl_log, _recorder, answers, blob, fill, heap, import_table, log_region, mask_this, put, set_return, strings, vtable  # noqa: E402

DELETE, NOCONV = 0, 1   # (the slots)
LOCK, UNLOCK, INC, DEC, THROW, FOPEN, FSEEK, FCLOSE, MBRTOWC, LOCINFO, UNINFO, ADDFAC, WCRTOMB = 7, 8, 12, 13, 14, 16, 17, 18, 19, 26, 27, 28, 29
B = Bench([(0x4f5d60, 0x4f5dca), (0x4f7180, 0x4f728d), (0x4f5ad0, 0x4f5d06), (0x4f3920, 0x4f39d1), (0x4f5dd0, 0x4f5e7d), (0x4f5e80, 0x4f5fa2), (0x5d0dc0, 0x5d0e00), (0x4f5d40, 0x4f5d60), (0x4f5d10, 0x4f5d40), (0x4ef7b0, 0x4ef7e8),
           (0x699ce0, 0x699d47), (0x699d50, 0x699d9b), (0x4f77f0, 0x4f7870), (0x4f2180, 0x4f2199), (0x67d250, 0x67d340), (0x69a560, 0x69a5c8), (0x6939f0, 0x693a90)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None, 0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x67d3a0: _recorder(THROW, 2, com=True),
                 0x67b8a0: _recorder(FOPEN, 2, cdecl=True), 0x67c100: _recorder(FSEEK, 3, cdecl=True), 0x67b620: _recorder(FCLOSE, 1, cdecl=True), 0x693bc0: _recorder(MBRTOWC, 5, cdecl=True), 0x69a460: _recorder(WCRTOMB, 4, cdecl=True),
                 0x698fc0: _recorder(LOCINFO, 1), 0x699100: _recorder(UNINFO, 0), 0x693f50: _recorder(ADDFAC, 3), 0x67a7d0: _cdecl_log})
BUF, VT, IMP, ARRAY, GLOBAL, FACET, FACET_VT, REF, NEW, MADE, STRING, OUT, NAME, FROM, TO, STATE, NEXT, TABLE = (RAM + 0x100 * i for i in range(18))
OTHER = RAM + 0x7000   # (never read)
FILE = 0x12345678
OPS = [n for n in ('ctor', 'with', 'init', 'open', 'in', 'out') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def common(r, regions, log):
    regions[IMPORT_TABLE] = import_table(log, {'inc': (INC, 0x6fd0c0, 1, 0, 0), 'dec': (DEC, 0x6fd12c, 1, 0, 0)})
    regions[0xfb3ca4] = struct.pack('<2I', r.choice([0, 0, 1]), r.randrange(4))
    regions[0xfb1440] = struct.pack('<5I', r.getrandbits(32), 0, 0, 0, r.getrandbits(32))
    regions[VT] = vtable({DELETE: 1})
    regions[FACET_VT] = vtable({NOCONV: 0})
    regions[0x709ce4] = vtable({NOCONV: 0})
    regions[FACET] = struct.pack('<2I', FACET_VT, 1)
    mask_this(log, LOCINFO)
    mask_this(log, UNINFO)
    mask_this(log, ADDFAC)
    strings(log, LOCINFO, 1)
    blob(log, THROW, 0, 0b111)
    regions[HEAP] = heap(0x400, fail=r.choice([0, 0, 0, 0, 0, 0, 1, 2]))


def counted(r, at, regions):
    regions[at] = struct.pack('<2I', VT, r.choice([0, 1, 1, 2, 0xffffffff]))


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log}
    common(r, regions, log)
    if name == 'ctor':
        regions[OUT] = r.bytes(0x20)
        return Call(0x4f5d60, [OUT, r.getrandbits(32)], regions, conv='t', ret='a')
    if name in ('in', 'out'):
        regions[BUF] = r.bytes(0x20)
        regions[STATE] = r.bytes(8)
        regions[NEXT] = r.bytes(8)
        regions[TO] = r.bytes(0x40)
        table = [r.choice([-2, -1, 0, 1, 1, 1, 2, 3]) if name == 'in' else r.choice([0, -1, 1, 1, 1, 2, 3, 4]) for _ in range(40)]
        regions[TABLE] = struct.pack('<40i', *table)
        if name == 'in':
            answers(log, MBRTOWC, TABLE)
            regions[FROM] = bytes(r.choice([0x41, 0x81, 0, r.randrange(1, 256)]) for _ in range(0x20)) + b'\0'
            return Call(0x4f5dd0, [BUF, STATE, FROM, FROM + r.choice([0, 1, 2, 4, 8, 16]), NEXT, TO, TO + r.choice([0, 2, 4, 8, 32]), NEXT + 4], regions, conv='t', ret='a')
        answers(log, WCRTOMB, TABLE)
        strings(log, WCRTOMB, 0, out=1)
        fill(log, WCRTOMB, 0, r.bytes(4), which=0)
        fill(log, WCRTOMB, 2, r.bytes(4), which=1)
        regions[0x77e7dc] = struct.pack('<i', r.choice([1, 2, 2, 2, 3, 5]))
        regions[FROM] = r.bytes(0x20)
        return Call(0x4f5e80, [BUF, STATE, FROM, FROM + r.choice([0, 2, 4, 8, 16]), NEXT, TO, TO + r.choice([0, 1, 2, 3, 4, 5, 8, 32]), NEXT + 4], regions, conv='t', ret='a')
    regions[0xfb1dd4] = struct.pack('<I', r.randrange(4))
    regions[0xfb1e00] = struct.pack('<I', r.choice([0, 1, 2, 3]))
    counted(r, REF, regions)
    counted(r, NEW, regions)
    regions[MADE] = struct.pack('<I', r.choice([NEW, NEW, REF]))
    set_return(log, ADDFAC, MADE)
    if name == 'with':
        regions[OUT] = r.bytes(8)
        return Call(0x4f7180, [OUT, r.choice([REF, REF, REF, 0]), FACET], regions, conv='c', ret='a')
    how = r.choice(['found', 'found', 'kept', 'make', 'make', 'global', 'bad'])
    imp = bytearray(r.bytes(0x20))
    put(imp, 0, 'I', VT)
    put(imp, 4, 'I', r.choice([0, 1, 1, 2, 0xffffffff]))
    put(imp, 8, 'I', ARRAY)
    put(imp, 0xc, 'I', r.choice([0, 2, 8]) if how in ('kept', 'make', 'bad', 'global') else 8)
    imp[0x14] = 1 if how == 'global' else r.choice([0, 1])
    regions[IMP] = imp
    regions[ARRAY] = struct.pack('<8I', *[FACET if how == 'found' else 0 for _ in range(8)])
    other = bytearray(0x20)
    put(other, 8, 'I', ARRAY + 0x40)
    put(other, 0xc, 'I', 8 if how == 'global' else 0)
    regions[GLOBAL] = other
    regions[ARRAY + 0x40] = struct.pack('<8I', *[FACET if how == 'global' else 0 for _ in range(8)])
    regions[0xfb1dcc] = struct.pack('<2I', OTHER if how == 'bad' else IMP, GLOBAL)
    regions[0x7ced40] = struct.pack('<3I', FACET if how == 'kept' else 0, r.getrandbits(32), r.getrandbits(32))
    set_return(log, NOCONV, r.choice([0, 0, 1, 0x100]))
    buf = bytearray(r.bytes(0x60))
    put(buf, 0x34, 'I', IMP)
    put(buf, 0x4c, 'I', REF)
    put(buf, 0x44, 'I', r.choice([0, 0, RAM]))
    masks = [Mask(HEAP, [0x20 if how == 'make' else 0x10], 'the new string\'s allocator byte: a stack byte in the original, read by nothing')]
    if name == 'init':
        regions[BUF] = buf
        return Call(0x4f5ad0, [BUF], regions, conv='t', masks=masks)
    put(buf, 0x50, 'I', r.choice([0, 0, 0, FILE]))
    regions[BUF] = buf
    regions[NAME] = b'name\0'
    regions[0xfb34b4] = r.bytes(4)
    set_return(log, FOPEN, r.choice([FILE, FILE, FILE, 0]))
    strings(log, FOPEN, 0b11)
    set_return(log, FSEEK, r.choice([0, 0, 1]))
    return Call(0x4f3920, [BUF, NAME, r.choice([1, 2, 0x21, 0x22, 5, 0])], regions, conv='t', ret='a', masks=masks)


if __name__ == '__main__':
    B.main(case)
