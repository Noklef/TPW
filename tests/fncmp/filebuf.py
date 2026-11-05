# The narrow file stream buffer's overflow and seek (src/game/core/filebuf: 0x5cedd0, 0x5cf490) against the originals. The std::string real; the C library's five file calls and the converter recorders.
# fwrite's and fseek's real code has its own bench. The converter's "no input taken" (its next-input pointer written back as the byte's own stack address) is not reached: a recorder's fill has no way to name that address.
# usage: filebuf.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Overflow,Seek)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob, echo, fill, global_heap, heap, log_fold, log_region, put, set_return, strings, vtable  # noqa: E402

FPUTC, FWRITE, FSETPOS, FSEEK, FGETPOS, OUT, SETVBUF = range(7)   # (OUT is the converter's slot)
B = Bench([(0x5cedd0, 0x5cefd9), (0x5cf490, 0x5cf5fd), (0x5cf410, 0x5cf484), (0x5cf600, 0x5cf62f), (0x45b340, 0x45b9a0), (0x47b280, 0x47b340), (0x4f1520, 0x4f1560)] + gamestring.B_RANGES,
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None, 0x67e570: _recorder(FPUTC, 2, cdecl=True), 0x67b6d0: _recorder(FWRITE, 4, cdecl=True), 0x67cf70: _recorder(FSETPOS, 2, cdecl=True), 0x67c100: _recorder(FSEEK, 3, cdecl=True), 0x67cf40: _recorder(FGETPOS, 2, cdecl=True), 0x67cf90: _recorder(SETVBUF, 4, cdecl=True)})
SELF, PUT, ROOM, CONV, VT, TEXT, STR, FILE, OUTPUT, TO = RAM, RAM + 0x80, RAM + 0x84, RAM + 0x90, RAM + 0xa0, RAM + 0x140, RAM + 0x180, RAM + 0x1a0, RAM + 0x1c0, RAM + 0x200


def base(r):
    log = log_region()
    log_fold(log)
    imports = bytearray(0x600)
    for at, stub in global_heap().items():
        imports[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    regions = {LOG: log, IMPORT_TABLE: imports, HEAP: heap(0x4000, r.choice([0] * 19 + [1]))}
    self = bytearray(r.bytes(0x60))
    put(self, 0x20, 'I', PUT)
    put(self, 0x30, 'I', ROOM)
    put(self, 0x38, 'I', r.choice([0, CONV, CONV]))
    put(self, 0x44, 'I', STR)
    put(self, 0x50, 'I', r.choice([0, FILE, FILE, FILE, FILE]))
    regions[SELF] = self
    regions[FILE] = r.bytes(0x20)
    regions[CONV] = struct.pack('<I', VT)
    regions[VT] = vtable({OUT: 7})
    # The string: empty, a short text or a long one; its block owned alone, shared, or frozen.
    text = r.choice([b'', b'ab', b'abcdefgh', b'abcdefghijklmnopqrstuvwxyz'])
    count = r.choice([0, 0, 1, 2, 0xff])
    regions[TEXT] = bytes([count]) + text + bytes(0x20)
    # The limit [0x6fe5ac]: 0x14 erases part of the long text. (A shared long text's copy would be past it: _Xlen, which has no answer here.)
    regions[0x6fe5ac] = struct.pack('<I', r.choice([0xffffffff, 0x10000] + ([0x14] * 2 if count in (0, 0xff) or len(text) < 0x14 else [])))
    regions[STR] = struct.pack('<4I', r.getrandbits(32), TEXT + 1 if text else 0, len(text), len(text))
    return log, regions, self


def overflow(r):
    log, regions, self = base(r)
    # The put area: none, full or with room.
    area = r.choice([0, OUTPUT, OUTPUT])
    regions[PUT] = struct.pack('<2I', area, r.choice([0, 0, 1, 3, 0xffffffff, 0x80000000]))
    regions[OUTPUT] = bytes(0x10)
    set_return(log, FPUTC, r.choice([0x41, 0, 0xffffffff]))
    if r.choice([0, 1]):
        echo(log, FWRITE, 1)   # (all written, in the compare's own terms)
    else:
        set_return(log, FWRITE, r.choice([1, 0]))
    set_return(log, OUT, r.choice([0, 0, 1, 1, 2, 3, 3, 4, 0xffffffff]))
    strings(log, OUT, 0, 0b1001110)   # (the input's three stack pointers and the output's next: logged as 0)
    fill(log, OUT, 3, struct.pack('<I', TO), which=0)
    fill(log, OUT, 6, struct.pack('<I', r.choice([HEAP + 0x40, HEAP + 0x41, TO])), which=1)
    return Call(0x5cedd0, [SELF, r.choice([0xffffffff, 0x41, 0xc1, 0x1234, -2 & 0xffffffff])], regions, conv='t', ret='a')


def seek(r):
    log, regions, self = base(r)
    regions[RAM + 0x300] = r.bytes(0x18)
    set_return(log, FSETPOS, r.choice([0, 0, 0, 0, 1]))
    set_return(log, FSEEK, r.choice([0, 0, 0, 0, 1]))
    set_return(log, FGETPOS, r.choice([0, 0, 0, 0, 1]))
    blob(log, FSETPOS, 1, 0x3)
    strings(log, FGETPOS, 0, 0b10)
    fill(log, FGETPOS, 1, r.bytes(8))
    return Call(0x5cf490, [SELF, RAM + 0x300, r.choice([0, 0, 5, 0xffffffff]), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')


def seekoff(r):
    log, regions, self = base(r)
    regions[RAM + 0x300] = r.bytes(0x18)
    set_return(log, FSEEK, r.choice([0, 0, 0, 0, 1]))
    set_return(log, FGETPOS, r.choice([0, 0, 0, 0, 1]))
    strings(log, FGETPOS, 0, 0b10)
    fill(log, FGETPOS, 1, r.bytes(8))
    return Call(0x5cf410, [SELF, RAM + 0x300, r.getrandbits(32), r.choice([0, 1, 2]), r.getrandbits(32)], regions, conv='t', ret='a')


def setbuf(r):
    log, regions, self = base(r)
    set_return(log, SETVBUF, r.choice([0, 0, 1, 0xffffffff]))
    return Call(0x5cf600, [SELF, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')


OPS = {'Overflow': overflow, 'Seek': seek, 'SeekOff': seekoff, 'SetBuf': setbuf}


def case(r):
    return OPS[r.deal(os.environ.get('OPS', 'Overflow,Seek,SeekOff,SetBuf').split(','))](r)


if __name__ == '__main__':
    B.main(case)
