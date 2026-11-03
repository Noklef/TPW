# The dialog [0x7cc188]'s address box (src/game/ui/dialog7cc188address: Dialog7cc188_AddAddress 0x4a77f0, Dialog7cc188_AddressHandler 0x4a77c0) against the originals. Window_FindChild, the std::string and the CRT's text functions real, the heap heap()'s; recorders
# (driver.cpp's kDialog7cc188AddressEnv): 0x5cd510 10 (the string's length and first 15 bytes hashed), Dialog7cc188_FillList 11, Window7059f0_ScrollTo 12, Language_WideText 13, Charset_FormatWide 14 (filling its buffer; the argument hashed), the box 0x47f020 15 (the record's set bytes and text hashed),
# Window_CallDefault 16, slot 9 17, 0x5c7420 27.
# usage: dialog7cc188address.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from dialog7cb400 import VT, kids  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob, blob_bytes, fill, heap, log_region, put, recorder, set_return, strings, vtable  # noqa: E402
from window705950draw import INTERLOCKED, LOCALE, interlocked  # noqa: E402

C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
HOOKS = {0x5cd510: _recorder(10, 1), 0x4a7a80: C(11, 1), 0x665739: _recorder(12, 1), 0x485b00: C(13, 1), 0x6acd60: C(14, 5), 0x47f020: C(15, 8), 0x65f66e: _recorder(16, 3), 0x5c7420: _recorder(27, 0)}
HOOKS.update({at: interlocked(step) for at, (_, step) in INTERLOCKED.items()})
B = Bench([(0x4a77c0, 0x4a77ef), (0x4a77f0, 0x4a7a7c), (0x45b380, 0x45b9b0), (0x65d7bc, 0x65d840), (0x67a400, 0x690000), (0x6918f0, 0x691a30), (0x691ba0, 0x691ce0)], hooks=HOOKS)
DIALOG, LIST, SLOTS, PLACE, KIDS = RAM + 0x100, RAM + 0x300, RAM + 0x800, RAM + 0x600, RAM + 0x1000
GOOD = ['a@b.c', 'name@host.org', 'a.b@c.d', 'x@yy.z', 'long.name.here@some.host.example.org', 'p' * 40 + '@' + 'q' * 20 + '.c', 'p' * 40 + '@' + 'q' * 30 + '.c']
BAD = ['', 'ab', '@b.c', 'a@.c', 'a@b', 'a.b@c', '@', '.', 'a@', 'b.c']
PLACEHOLDER = 'to whom'


def wide(text):
    return text.encode('utf-16le') + b'\0\0'


def case(r):
    log = log_region()
    imports = bytearray(0x400)
    for at, (slot, _) in INTERLOCKED.items():
        put(imports, slot - IMPORT_TABLE, 'I', at)
    regions = {LOG: log, HEAP: heap(), 0x78ba94: r.bytes(4), LOCALE: bytes(4), IMPORT_TABLE: imports, 0xfb3ca4: bytes(8)}  # (the CRT's wctomb: see window705950draw.py)
    first, top = kids(r, regions, KIDS, (0x26, 0x27))
    regions[LIST] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', first)
    inner, _ = kids(r, regions, KIDS + 0x1000, (0x5da5, 0x5da4))
    regions[DIALOG] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', inner)
    regions[VT] = put(vtable({}, size=0x10), 0x24, 'I', recorder(17, 1))
    regions[0x7cc184] = struct.pack('<2I', DIALOG, LIST)
    regions[SLOTS] = r.bytes(0x110)
    regions[0xf81ebc] = struct.pack('<I', r.choice([SLOTS, SLOTS, SLOTS, 0]))
    set_return(log, 27, SLOTS)
    regions[PLACE] = wide(PLACEHOLDER)
    set_return(log, 13, PLACE)
    address = r.deal(GOOD + GOOD + BAD)
    regions[0x7cb8d8] = bytearray(wide(address)) + bytearray(0x80)
    regions[0x7cb4d0] = bytearray(wide(r.deal([PLACEHOLDER, PLACEHOLDER, '', ';other@x.y', ';z@z.zz;' + address + ';u@v.w', 'a@b.c.d', address[:-1] or 'k', address + 'x', PLACEHOLDER + 'z']))).ljust(0x400, b'\0')
    blob_bytes(log, 10, 0, 0xf00, (4, 0x7fff))
    fill(log, 14, 1, wide('no address'))
    strings(log, 14, 0, 1 << 1)
    blob(log, 14, 4, 0xf)
    blob_bytes(log, 15, 0, 0xf70, (0, 0x7fff))
    set_return(log, 16, r.getrandbits(32))
    if r.deal([0, 1, 2]):
        return Call(0x4a77f0, [], regions, ret='v')
    message, a = r.deal([(0x1000b, 0xd), (0x1000b, 0xd), (0x1000b, 0xc), (0x1000a, 0xd), (0x1000b, r.getrandbits(32)), (r.getrandbits(32), 0xd)])
    return Call(0x4a77c0, [DIALOG, message, a, r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
