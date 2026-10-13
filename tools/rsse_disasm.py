"""Disassembler for Theme Park World RSSE ride/object scripts (*.RSE).

File layout (as observed; little-endian u32 words):
  0x00  "RSSE"
  0x04  version (0x00010F51)
  0x08  variable count (the last N strings are variable names)
  0x0C  stack size, 0x10 slice budget, 0x14 limbo slots, 0x18 rider slots, 0x1C walker slots
  0x20  16 bytes "Pad Pad Pad Pad "
  0x30  code_len (words), then code_len code words
  then  length-prefixed (u32, includes NUL) strings until EOF:
        first the string table, then exported variable names

Code word tags (top nibble):
  0x8xxxxxxx  opcode (index into re/rsse_opcodes.csv, which comes from the exe's table)
  0x4xxxxxxx  variable reference
  0x2xxxxxxx  code label (word offset into code)
  0x1xxxxxxx  string-table reference
  otherwise   immediate integer

Usage:
  python -I tools/rsse_disasm.py <file.RSE | file.wad> [--check-all <data_dir>]
"""
import csv
import glob
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from wadtool import read_wad  # noqa: E402

OPCODES_CSV = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 're', 'rsse_opcodes.csv')
with open(OPCODES_CSV) as f:
    OPS = [(r['mnemonic'], int(r['operands'])) for r in csv.DictReader(f)]

TAG_OP, TAG_VAR, TAG_LABEL, TAG_STR = 0x8, 0x4, 0x2, 0x1


class Script:
    def __init__(self, data):
        if data[:4] != b'RSSE':
            raise ValueError('not an RSSE script')
        self.header = struct.unpack_from('<7I', data, 4)
        self.code_len = struct.unpack_from('<I', data, 0x30)[0]
        self.code = list(struct.unpack_from(f'<{self.code_len}I', data, 0x34))
        pos = 0x34 + 4 * self.code_len
        self.strings = []
        while pos + 4 <= len(data):
            n = struct.unpack_from('<I', data, pos)[0]
            pos += 4
            self.strings.append(data[pos:pos + n].rstrip(b'\0').decode('latin1'))
            pos += n
        self.trailing = len(data) - pos
        # header[1] is the variable count; the last that-many strings are variable names
        # (the engine<->script contract, e.g. VAR_LETMEON, VAR_CAPACITY).
        nvars = self.header[1]
        if 0 < nvars <= len(self.strings):
            self.var_names = self.strings[-nvars:]
            self.strings = self.strings[:-nvars]
        else:
            self.var_names = []

    def decode(self):
        """Yield (word_offset, mnemonic, operands). Raises on malformed code."""
        i = 0
        while i < len(self.code):
            w = self.code[i]
            if w >> 28 != TAG_OP:
                raise ValueError(f'word {i}: expected opcode, got {w:#010x}')
            idx = w & 0x0FFFFFFF
            if idx >= len(OPS):
                raise ValueError(f'word {i}: opcode index {idx} out of range')
            name, argc = OPS[idx]
            args = self.code[i + 1:i + 1 + argc]
            if len(args) != argc:
                raise ValueError(f'word {i}: {name} truncated')
            yield i, name, args
            i += 1 + argc

    def fmt_operand(self, w):
        tag, val = w >> 28, w & 0x0FFFFFFF
        if tag == TAG_VAR:
            return self.var_names[val] if val < len(self.var_names) else f'var[{val}]'
        if tag == TAG_LABEL:
            return f'L{val:04d}'
        if tag == TAG_STR:
            s = self.strings[val] if val < len(self.strings) else '?'
            return f'str[{val}] "{s}"'
        return str(w if w < 0x80000000 else w - (1 << 32))

    def listing(self):
        labels = {w & 0x0FFFFFFF for w in self.code if w >> 28 == TAG_LABEL}
        out = [f'; header {" ".join(f"{h:#x}" for h in self.header)}',
               f'; strings {self.strings}',
               f'; vars    {self.var_names}']
        for off, name, args in self.decode():
            if off in labels:
                out.append(f'L{off:04d}:')
            out.append(f'    {name:<16}{", ".join(self.fmt_operand(a) for a in args)}')
        return '\n'.join(out)


def scripts_in(path):
    if path.lower().endswith('.wad'):
        for name, data, _ in read_wad(path):
            if name.lower().endswith('.rse'):
                yield f'{path}:{name}', data
    else:
        yield path, open(path, 'rb').read()


if __name__ == '__main__':
    if sys.argv[1] == '--check-all':
        ok, bad, used = 0, 0, set()
        for wad in glob.glob(os.path.join(sys.argv[2], '**', '*.wad'), recursive=True):
            for name, data in scripts_in(wad):
                try:
                    s = Script(data)
                    used.update(n for _, n, _ in s.decode())
                    ok += 1
                except Exception as e:
                    bad += 1
                    print('FAIL', name, e)
        print(f'decoded {ok} scripts, {bad} failures; opcodes used: {len(used)}/{len(OPS)}')
        print('never used:', sorted(set(n for n, _ in OPS) - used))
    else:
        for name, data in scripts_in(sys.argv[1]):
            print(f';;; {name}')
            print(Script(data).listing())
