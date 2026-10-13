"""Recover struct field offsets from the load side of the game's save serializers.

Each serializer reads a field straight into the object, so its load branch pairs every field
name with an offset from `this` (param_1):
  Save_FieldName(n, "mName", 1); Stream_Read(&r, stream, param_1 + OFF, n, &got);
  Save_ReadU32(stream, param_1 + OFF, "mName", 0);           (also U16, U8, 8 and 12 bytes)
  Save_ReadU32(stream, &tmp, "mName", 0); *(float *)(param_1 + OFF) = tmp;
  ptr = param_1 + BASE; do { Save_ReadU16(stream, ptr + -8, "mFoo[i]", 0); ptr += 2; } while (i < 4);
Names behind DAT_ labels (short strings Ghidra didn't type) are read from the exe.
Calls to other serializers pass `this` in ECX, which the decompiler drops, so their offsets come
from the machine code instead: the `lea ecx, [reg + disp]` (or `mov ecx, reg` = offset 0, i.e. a
base class) just before each `call`.

Usage:
  python -I tools/serializer_layout.py _re/decomp-raw <exe> addr [addr ...]   (addr like 004fb530)
"""
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pe_read import PE  # noqa: E402
from save_schema import load_functions  # noqa: E402

READ_SIZES = {'004fae10': 4, '004d37e0': 2, '00424c20': 1, '00510240': 8, '005134d0': 12}
NOT_SERIALIZERS = {'00416e10', '004ced70', '004cee50', '004ceec0', '004cede0'}  # FieldName + write helpers
NUM = r'(0x[0-9a-f]+|\d+)'


def name_of(tok, pe):
    m = re.match(r'(?:\(char \*\))?s_(\w+?)_00([0-9a-f]{6})$', tok)
    if m:
        return pe.cstr(int(m.group(2), 16)) if pe else m.group(1)
    # DAT_/PTR_ labels: Ghidra didn't type the string (PTR_ when its bytes look like an address)
    m = re.match(r'(?:\(char \*\))?&?(?:DAT|PTR)_\w*?00([0-9a-f]{6})$', tok)
    if m and pe:
        return pe.cstr(int(m.group(1), 16))
    return tok


def load_branch(body):
    """The decompiler prints the load branch first; cut at the first save-side FieldName."""
    m = re.search(r'FUN_00416e10\([^;]*?,\s*0\)', body)
    return body[:m.start()] if m else body


def layout(addr, src, pe, serializers):
    name, body = src[addr]
    body = load_branch(re.sub(r'\s+', ' ', body))
    stmts = [s.strip() for s in re.split(r'(?<=[;{}])', body)]
    ptrs = {}              # var -> base offset from this
    fields, calls = [], []
    pending_name = None    # (size, name) from Save_FieldName awaiting its Stream_Read
    pending_tmp = None     # (var, size, name) read into a temporary
    loop_stack = []

    def offset_of(expr):
        expr = expr.strip()
        m = re.match(r'(?:\(\w+ \*\))?\(?param_1 \+ ' + NUM + r'\)?$', expr)
        if m:
            return int(m.group(1), 0)
        if expr == 'param_1':
            return 0
        m = re.match(r'(?:\(\w+ \*\))?\(?(\w+)(?: \+ (-?(?:0x[0-9a-f]+|\d+)))?\)?$', expr)
        if m and m.group(1) in ptrs:
            return ptrs[m.group(1)] + (int(m.group(2), 0) if m.group(2) else 0)
        return None

    for s in stmts:
        m = re.match(r'(\w+) = (?:\(\w+ \*\))?\(?param_1 \+ ' + NUM + r'\)?;', s)
        if m:
            ptrs[m.group(1)] = int(m.group(2), 0)
            continue
        m = re.match(r'(\w+) = \1 \+ ' + NUM + ';', s)
        if m and m.group(1) in ptrs:
            if loop_stack and loop_stack[-1]['stride'] is None:
                loop_stack[-1]['stride'] = int(m.group(2), 0)
            continue
        if s == 'do {':
            loop_stack.append({'start': len(fields), 'stride': None})
            continue
        m = re.match(r'\}? ?while \(.*?< ' + NUM + r'\)+;?', s)
        if m and loop_stack:
            loop = loop_stack.pop()
            for f in fields[loop['start']:]:
                f['count'], f['stride'] = int(m.group(1), 0), loop['stride']
            continue
        m = re.search(r'FUN_00416e10\(' + NUM + r',\s*([^,]+?),\s*1\)', s)
        if m:
            pending_name = (int(m.group(1), 0), name_of(m.group(2), pe))
            continue
        m = re.search(r'Stream_Read\([^,]+,[^,]+,\s*(.+?),\s*' + NUM + r',', s)
        if m and pending_name:
            off = offset_of(m.group(1))
            fields.append({'off': off, 'size': pending_name[0], 'name': pending_name[1], 'via': 'raw'})
            pending_name = None
            continue
        m = re.search(r'FUN_(00[0-9a-f]{6})\(([^,]+),\s*(.+?),\s*([^,]+?),\s*0\)', s)
        if m and m.group(1) in READ_SIZES:
            size, fname, target = READ_SIZES[m.group(1)], name_of(m.group(4).strip(), pe), m.group(3)
            off = offset_of(target)
            if off is None and target.startswith('&'):
                pending_tmp = (target[1:], size, fname)
                continue
            fields.append({'off': off, 'size': size, 'name': fname, 'via': 'read'})
            continue
        m = re.match(r'\*\((\w+(?: \w+)?) \*\)\((.+?)\) = (?:\(\w+\))?(\w+);', s)
        if m and pending_tmp and m.group(3) == pending_tmp[0]:
            fields.append({'off': offset_of(m.group(2)), 'size': pending_tmp[1], 'name': pending_tmp[2],
                           'via': 'tmp', 'type': m.group(1)})
            pending_tmp = None
            continue
        for c in re.finditer(r'FUN_(00[0-9a-f]{6})\(', s):
            if c.group(1) in serializers and c.group(1) != addr and c.group(1) not in NOT_SERIALIZERS:
                calls.append((len(fields), c.group(1)))
    return name, fields, calls


def call_offsets(pe, start, end, target):
    """Offsets of `this` for each `call target` in [start, end), from the preceding ECX setup."""
    code = pe.bytes(start, end - start) or b''
    found = []
    for i in range(len(code) - 4):
        if code[i] != 0xE8 or start + i + 5 + int.from_bytes(code[i + 1:i + 5], 'little', signed=True) != target:
            continue
        off = None
        for j in range(i - 2, max(i - 24, -1), -1):
            op, modrm = code[j], code[j + 1]
            if op == 0x8D and modrm & 0xF8 == 0x48 and modrm != 0x4C:        # lea ecx,[r+d8]
                off = int.from_bytes(code[j + 2:j + 3], 'little', signed=True)
            elif op == 0x8D and modrm & 0xF8 == 0x88 and modrm != 0x8C:      # lea ecx,[r+d32]
                off = int.from_bytes(code[j + 2:j + 6], 'little', signed=True)
            elif op == 0x8B and modrm & 0xF8 == 0xC8:                        # mov ecx,r
                off = 0
            if off is not None:
                break
        found.append(off)
    return found


def main():
    dump, exe, addrs = sys.argv[1], sys.argv[2], sys.argv[3:]
    src = load_functions(dump)
    pe = PE(exe)
    serializers = {a for a, (_, b) in src.items() if 'FUN_00416e10(' in b}
    starts = sorted(int(a, 16) for a in src)
    for addr in addrs:
        name, fields, calls = layout(addr, src, pe, serializers)
        print('=== %s @ %s' % (name, addr))
        ci = 0
        for i, f in enumerate(sorted(fields, key=lambda f: (f['off'] is None, f['off'] or 0))):
            arr = '[%d] stride %s' % (f['count'], f['stride']) if 'count' in f else ''
            off = '  ?   ' if f['off'] is None else '%#06x' % f['off']
            print('  %s %2d %-32s %s %s' % (off, f['size'], f['name'], f.get('type', ''), arr))
        start = int(addr, 16)
        end = next((a for a in starts if a > start), start + 0x4000)
        for c in dict.fromkeys(c for _, c in calls):
            offs = call_offsets(pe, start, end, int(c, 16))
            print('  call %s (%s) this+%s' % (c, src[c][0], ', '.join('?' if o is None else '%#x' % o for o in offs)))


if __name__ == '__main__':
    main()
