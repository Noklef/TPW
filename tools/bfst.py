"""Theme Park World BFST string tables (*.str) -> text.

Layout:
  0x00 "BFST"   0x04 u32 ?   0x08 u32 count   0x0C u32 offsets[count] (relative to 0x0C)
  record: u8 type (1), u8 length, u16 0, length bytes of encoded text, pad to 4, u32 0

Text is in a custom 8-bit code page. MBToUni.dat ("BFMU", u16 0, u16 count, then count
UTF-16LE code points) maps code c to table[c - 1]. (For letters this happens to equal
0x114 - c, which is why the text looks like a simple cipher.)

Usage: python -I tools/bfst.py <file.str> [MBToUni.dat]   (defaults to the .str's folder)
"""
import os
import struct
import sys


def load_codepage(path):
    d = open(path, 'rb').read()
    assert d[:4] == b'BFMU'
    count = struct.unpack_from('<H', d, 6)[0]
    return [chr(v) for v in struct.unpack_from(f'<{count}H', d, 8)]


def decode(raw, codepage):
    return ''.join(codepage[c - 1] if 0 < c <= len(codepage) else f'\\x{c:02x}' for c in raw)


def records(data):
    assert data[:4] == b'BFST'
    unk, count = struct.unpack_from('<II', data, 4)
    offs = struct.unpack_from(f'<{count}I', data, 12)
    for o in offs:
        p = 12 + o
        typ, n = data[p], data[p + 1]
        yield typ, data[p + 4:p + 4 + n]


if __name__ == '__main__':
    src = sys.argv[1]
    cp = load_codepage(sys.argv[2] if len(sys.argv) > 2 else os.path.join(os.path.dirname(src), 'MBToUni.dat'))
    for i, (typ, raw) in enumerate(records(open(src, 'rb').read())):
        print(f'{i}\t{decode(raw, cp)!r}')
