"""Theme Park World .sam text config files, checked against the key schemas inside the exe.

A .sam file is lines of `Section.Key value  [comment]`; `#` starts a comment line and anything
after the value is ignored (often `//` or plain prose). Values are numbers or "strings". `Group.A.B.C v1 v2 v3` sets several fields of one group.
Table keys take an index: `Upgrades[0].InitCapacity 1`. Shape keys (type 11) take a block of
grid rows between two `---` lines on the following lines.

The game describes each kind of .sam with a schema: an array of 0x3c-byte key records in .data,
returned by vtable slot 0 of the class that loads it (slot 1 returns the value storage):
  +0x00 u32 type     0 blank, 1 section (names the keys above it, back to the previous section),
                     2 table start, 3 table end (names the keys since the start; +0x34 = rows),
                     4..11 a value: 4 int, 5 int >= 0, 6 int in [min, max), 7 float, 8 float >= 0,
                     9 float in [fmin, fmax), 10 string, 11 shape block; 12 end of schema
  +0x04 char name[32]
  +0x24 i32 min, +0x28 i32 max        (type 6; min is also the default)
  +0x2c f32 fmin, +0x30 f32 fmax      (type 9; fmin is also the default)
  +0x34 u32 rows                      (type 3)
  +0x38 u32 shape kind                (type 11)
The parser (0x4017a0) rejects out-of-range values ("Balance Problem: bounded value out of range").
Every value takes one 4-byte slot of the storage, in schema order from slot 1; a table reserves
rows * keys slots plus one (base constructor 0x401030). The object defs (rides, shops, ...) keep
the storage at def + 0xc, so slot s lives at def + 0xc + 4 * s (ThirstEffect = def + 0x144).

Usage:
  python -I tools/samtool.py schema <exe> [name]       print a schema with slot offsets
  python -I tools/samtool.py check <exe> <file.sam>...  parse files and flag unknown keys
"""
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pe_read import PE  # noqa: E402

# Schema tables: name -> (table VA, offset of slot 0 from the owning object, accessor)
SCHEMAS = {
    'object': (0x744b30, 0x0c, 0x412c90),       # ride/shop/sideshow/feature descriptions
    'track': (0x739ea8, None, 0x4143f0),        # coaster/track types: textures, cross sections, cars
    'graphics': (0x7442c0, None, 0x402bf0),     # low/med/high.sam
    'resolution': (0x744a40, None, 0x402c60),   # Resolution.sam
    'sound': (0x746a20, None, 0x402cd0),
    'advisor': (0x72e120, None, 0x415e90),      # advisor message scoring
    'challenges': (0x739b98, None, 0x4d1e60),
    'theme': (0x73f8a8, None, 0x5b0d40),        # tickets, keys, park gate
    'level': (0x73fc70, None, 0x402b10),        # Standard.sam: peeps, bank, map, challenges per level
    'hoardings': (0x73fb40, None, 0x402a50),    # Hoardings.sam / EndHoardings.sam click points
    'online': (0x743f00, None, 0x402b80),       # Online.sam
}
TYPE_NAMES = {4: 'int', 5: 'uint', 6: 'int', 7: 'float', 8: 'ufloat', 9: 'float', 10: 'string', 11: 'shape'}


def read_schema(pe, va):
    """Key records up to the end marker, as dicts with the full dotted key and storage slot."""
    raw = []
    for i in range(1000):
        rec = pe.bytes(va + i * 0x3c, 0x3c)
        kind = struct.unpack_from('<I', rec)[0]
        if kind == 12:
            break
        if kind > 12:
            raise ValueError('bad schema record %d at %#x' % (i, va + i * 0x3c))
        lo, hi, flo, fhi, rows, shape = struct.unpack_from('<iiffII', rec, 0x24)
        raw.append({'index': i, 'type': kind, 'name': rec[4:0x24].split(b'\0')[0].decode('latin-1'),
                    'min': lo, 'max': hi, 'fmin': flo, 'fmax': fhi, 'rows': rows, 'shape': shape})
    # Slots, as the base constructor assigns them.
    slot, table = 1, None
    for r in raw:
        if r['type'] == 2:
            table = {'start': slot, 'keys': 0}
        elif r['type'] == 3 and table:
            r['table_keys'] = table['keys']
            slot = table['start'] + r['rows'] * table['keys'] + 1
            table = None
        elif 4 <= r['type'] <= 11:
            r['slot'] = slot
            slot += 1
            if table:
                table['keys'] += 1
    # Names: a section record names the keys above it; a table end names the keys in the table
    # (tables sit at the top level: `Upgrades[0].InitCapacity`, not `AddOn.Upgrades[0]...`).
    keys, pending, in_table = [], [], None
    for r in raw:
        if r['type'] == 1:
            for k in pending:
                k['key'] = '%s.%s' % (r['name'], k['key'])
            keys += pending
            pending = []
        elif r['type'] == 2:
            in_table = []
        elif r['type'] == 3:
            for k in in_table:
                k['key'] = '%s[].%s' % (r['name'], k['key'])
                k['rows'], k['stride'] = r['rows'], r['table_keys'] * 4
            keys += in_table
            in_table = None
        elif 'slot' in r:
            r['key'] = r['name']
            (in_table if in_table is not None else pending).append(r)
    return sorted(keys + pending, key=lambda k: k['slot'])


def describe(k, base):
    t = TYPE_NAMES.get(k['type'], str(k['type']))
    if k['type'] == 6:
        t += ' [%d,%d)' % (k['min'], k['max'])
    if k['type'] == 9:
        t += ' [%g,%g)' % (k['fmin'], k['fmax'])
    if k['type'] == 11:
        t += ' kind %d' % k['shape']
    off = '' if base is None else '+%#05x' % (base + 4 * k['slot'])
    rows = ' x%d stride %#x' % (k['rows'], k['stride']) if 'stride' in k else ''
    return '%-4d %-7s %-44s %s%s' % (k['slot'], off, k['key'], t, rows)


LINE = re.compile(r'\s*([A-Za-z_]\w*(?:\[\d+\])?(?:\.[A-Za-z_]\w*(?:\[\d+\])?)*)\s*(.*)$')


def parse_sam(text):
    """(key, value, line number) triples; shape blocks become a list of rows."""
    out, lines, i = [], text.splitlines(), 0
    while i < len(lines):
        line = lines[i]
        i += 1
        if not line.strip() or line.lstrip().startswith(('#', '//', ';')):
            continue
        m = LINE.match(line)
        if not m:
            out.append((None, line, i))
            continue
        key, rest = m.group(1), m.group(2).strip()
        if not rest and i < len(lines) and lines[i].strip() == '---':
            j = i + 1
            while j < len(lines) and lines[j].strip() != '---':
                j += 1
            out.append((key, lines[i + 1:j], i))
            i = j + 1
            continue
        group, *fields = key.split('.')
        values = [a if a is not None else b for a, b in re.findall(r'"([^"]*)"|(-?\d[\d.]*|-?\.\d+)', rest)]
        if len(fields) > 1:   # Group.A.B.C v1 v2 v3 sets several fields of one group
            for f, v in zip(fields, values + [None] * len(fields)):
                out.append(('%s.%s' % (group, f), v, i))
            continue
        out.append((key, values[0] if values else (rest.split() or [None])[0], i))
    return out


def sam_files(paths):
    """(label, bytes) for .sam files given directly, or found in folders and inside .wad files."""
    from wadtool import read_wad
    for p in paths:
        if os.path.isdir(p):
            for root, _, files in os.walk(p):
                for f in sorted(files):
                    full = os.path.join(root, f)
                    if f.lower().endswith('.sam'):
                        yield full, open(full, 'rb').read()
                    elif f.lower().endswith('.wad'):
                        for name, data, _ in read_wad(full):
                            if name.lower().endswith('.sam'):
                                yield '%s:%s' % (full, name), data
        else:
            yield p, open(p, 'rb').read()


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    cmd, pe = sys.argv[1], PE(sys.argv[2])
    if cmd == 'schema':
        for name in sys.argv[3:] or SCHEMAS:
            va, base, acc = SCHEMAS[name]
            print('=== %s: table %#x (accessor %#x)%s' % (name, va, acc, '' if base is None else ', slot 0 at +%#x' % base))
            for k in read_schema(pe, va):
                print('  ' + describe(k, base))
        return
    if cmd == 'check':
        known = {}
        for name, (va, _, _) in SCHEMAS.items():
            for k in read_schema(pe, va):
                known.setdefault(k['key'].lower(), []).append(name)
        for path, data in sam_files(sys.argv[3:]):
            entries = parse_sam(data.decode('latin-1'))
            bad, used = [], set()
            for key, value, line in entries:
                norm = re.sub(r'\[\d+\]', '[]', key).lower() if key else None
                if norm not in known:
                    bad.append('%d: %s' % (line, key or value))
                else:
                    used.update(known[norm])
            print('%s: %d keys [%s]%s' % (path, len(entries), ','.join(sorted(used)),
                                          (' unknown: ' + '; '.join(bad[:6])) if bad else ''))
        return
    sys.exit(__doc__)


if __name__ == '__main__':
    main()
