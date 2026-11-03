# The stream buffer's initialiser and its locale reference (src/game/core/wfilebuf: 0x4f5990, 0x4f5a50) against the originals: the locale 0x699da0 (agent B's) the cdecl recorder 15 of 0, the
# global's counted object [0xfb1dd0] with its count about -1 (the STL lock left out of both). Init: which 0, 1 or any; with 0, the old locale reference none or a counted object at
# its last use or not (its slot 0 a recorder), the file any, the global [0xfb34b4] any.
# The buffer's destructor (0x4f08c0) and the two streams' (0x4f0540, 0x4f10d0): fclose 0x67b620 the cdecl recorder 18.
# usage: wfilebuf.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Locale,Init,Dtor,Ofstream,Ifstream)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, put, set_return, vtable  # noqa: E402

FCLOSE = 18
B = Bench([(0x4f5990, 0x4f5a4f), (0x4f5a50, 0x4f5acc), (0x4ef7b0, 0x4ef800), (0x5da3c0, 0x5da3c1), (0x4f08c0, 0x4f0a10), (0x4f0540, 0x4f0650), (0x4f10d0, 0x4f11e0), (0x45b340, 0x45b9a0), (0x4f6020, 0x4f6080)],
          hooks={0x699da0: _recorder(15, 0, cdecl=True), 0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None, 0x67b620: _recorder(FCLOSE, 1, cdecl=True)})
BUF, OLD, OLDVT, GLOBAL, OLD2, OLD2VT, VBT = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500, RAM + 0x600
STREAM = RAM + 0x800
OPS = [n for n in ('Locale', 'Init', 'Dtor', 'Ofstream', 'Ifstream') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def counted_regions(r, regions, at, table, slot):
    old = bytearray(r.bytes(0x10))
    put(old, 0, 'I', table)
    put(old, 4, 'I', r.choice([0, 1, 1, 2, 3, 0xfffffffe, 0xffffffff]))
    regions.update({at: old, table: vtable({slot: 1})})


def dtor(r, op, log, regions):
    """The buffer's fields: the file open or not (fclose's answer), the string +0x44 null or a block with its text's use count, the two locale references null or counted (their deletes told apart by the slot's recorder)."""
    counted_regions(r, regions, OLD, OLDVT, 0)
    counted_regions(r, regions, OLD2, OLD2VT, 0)
    stream = bytearray(r.bytes(0x80))
    buf = 0 if op == 'Dtor' else 4 if op == 'Ofstream' else 8
    put(stream, buf + 0x48, 'B', r.choice([0, 1, 1, r.getrandbits(8)]))
    put(stream, buf + 0x50, 'I', r.choice([0, r.getrandbits(32), r.getrandbits(32)]))
    put(stream, buf + 0x44, 'I', r.choice([0, HEAP + 0x10]))
    put(stream, buf + 0x4c, 'I', r.choice([0, OLD]))
    put(stream, buf + 0x34, 'I', r.choice([0, OLD2]))
    set_return(log, FCLOSE, r.choice([0, 0, 1, 0xffffffff]))
    block = heap(0x100)
    text = r.choice([0, HEAP + 0x40])
    put(block, 0x14, 'I', text)
    put(block, 0x3f, 'B', r.choice([0, 1, 2, 0xff, 0xfe]))
    regions[HEAP] = block
    if op == 'Dtor':
        regions[BUF] = stream
        return Call(0x4f08c0, [BUF], regions, conv='t', ret='v')
    # (A stream: its vbtable first, the virtual base's word at the offset in it, within the stream.)
    regions.pop(BUF)
    regions[STREAM] = stream
    put(stream, 0, 'I', VBT)
    regions[VBT] = struct.pack('<II', r.getrandbits(32), r.choice([0x70, 0x74, 0x7c]))
    if op == 'Ofstream':
        return Call(0x4f0540, [STREAM + 0x58], regions, conv='t', ret='v')
    return Call(0x4f10d0, [STREAM + 0x5c], regions, conv='t', ret='v')


def case(r):
    op = r.deal(OPS)
    log = log_region()
    set_return(log, 15, r.getrandbits(32))
    counted = bytearray(r.bytes(0x10))
    put(counted, 4, 'I', r.choice([0, 1, 2, 0xfffffffe, 0xffffffff, r.getrandbits(32)]))
    regions = {LOG: log, BUF: bytearray(r.bytes(0x60)), 0xfb1dd0: struct.pack('<I', GLOBAL), GLOBAL: counted, 0xfb34b4: r.bytes(4)}
    if op in ('Dtor', 'Ofstream', 'Ifstream'):
        return dtor(r, op, log, regions)
    if op == 'Locale':
        return Call(0x4f5a50, [BUF], regions, conv='t', ret='a')
    old = bytearray(r.bytes(0x10))
    put(old, 0, 'I', OLDVT)
    put(old, 4, 'I', r.choice([0, 1, 1, 2, 3, 0xfffffffe, 0xffffffff]))
    regions.update({OLD: old, OLDVT: vtable({0: 1})})
    put(regions[BUF], 0x4c, 'I', r.choice([0, OLD, OLD, OLD]))
    set_return(log, 0, r.getrandbits(32))
    return Call(0x4f5990, [BUF, r.getrandbits(32), r.choice([0, 0, 0, 1, 1, 2, r.getrandbits(32)])], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
