"""The codemod of step 6 (.docs/09-globals.md): a .data scalar that the code reaches only as global<T>(0xADDRESS) (any accessor a file
defines as `template <typename T> T &NAME(u32 address)`, its default type included), with one type, becomes a GAME_DATA line in its owner's
header, and every use becomes the name. Rerunnable: a converted address is no longer a literal, so a second run finds only the rest.

usage: gamedata_codemod.py list                 the candidates by owner folder, and the addresses left alone, by reason
       gamedata_codemod.py skips                every address left alone with its reason (the pointer ones are the fixup round's)
       gamedata_codemod.py apply N [DIR...]     convert the first N candidates (by owner header) whose owner is under src/game/DIR
       gamedata_codemod.py benches REV          the fncmp benches that run a ported function whose lines changed since REV

The owner: the one file that uses it (a .cpp's header: its own, or the one it includes first); for a global several files use, the one
file that writes it, the others include its header. The name: re/symbols.csv's data row; otherwise named for its use, unconfirmed: g, the
ported function (a symbols.csv row) that uses it, or the common Prefix_ of several, else the file's name, then _ and the address
(gSoundVoice_Load_77dd20). A new name gets a symbols.csv row, low.
Left alone: a use in another form (a cast, an offset, asm), two types, an overlap, an unknown type, a use in a header or outside src/game,
agent B's C library files (crt*, staticinit*), a non-literal neighbour below it within NEAR bytes (an array or struct base whose extent
may reach it), and what gamedata.py object would reject: an initial dword that is a code address, or an exe dword pointing into it.
"""
import bisect
import collections
import os
import re
import struct
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import gamedata   # noqa: E402

DATA = (0x727000, 0xfb5000)     # .data
NEAR = 0x100                     # how far below a candidate a base with unknown extent must be
SKIP = {}                        # address: reason, for what the build or a bench finds (none yet)
KEYWORDS = {'const', 'volatile', 'unsigned', 'signed', 'char', 'short', 'int', 'long', 'float', 'double', 'bool', 'void', 'wchar_t',
            '__int64', 'struct', 'class', 'union', 'enum'}
SIZES = {'u8': 1, 's8': 1, 'char': 1, 'bool': 1, 'unsigned char': 1, 'signed char': 1, 'BYTE': 1,
         'u16': 2, 's16': 2, 'short': 2, 'unsigned short': 2, 'wchar_t': 2, 'WORD': 2,
         'u32': 4, 's32': 4, 'int': 4, 'unsigned': 4, 'unsigned int': 4, 'long': 4, 'unsigned long': 4, 'float': 4, 'f32': 4,
         'DWORD': 4, 'BOOL': 4, 'u64': 8, 's64': 8, 'double': 8, 'f64': 8, '__int64': 8}
ACCESSOR = re.compile(r'template\s*<\s*typename\s+T\s*(?:=\s*([^>]+?))?\s*>\s*T\s*&\s*(\w+)\s*\(\s*(?:const\s+)?(?:u32|uintptr_t|std::uintptr_t)\s+\w+\s*\)')
FUNC = re.compile(r'^(?![ \t#])[^;{}()\n]*?\b([A-Za-z_]\w*(?:::~?[A-Za-z_]\w*)*)\s*\([^;{}]*\)\s*(?:const\s*)?\{', re.M)
DEREF = re.compile(r'\*\s*reinterpret_cast\s*<\s*(?:const\s+)?(\w+(?:\s+\w+)?)\s*(?:const\s*)?\*\s*>\s*\(\s*(?:\(\s*(?:std::)?uintptr_t\s*\)\s*)?$')
WRITE_AFTER =re.compile(r'\s*(?:=(?!=)|\+=|-=|\*=|/=|%=|\|=|&=|\^=|<<=|>>=|\+\+|--)')
WRITE_BEFORE = re.compile(r'(?:\+\+|--|(?<!&)&)\s*$')
B_FILES = re.compile(r'^staticinit')   # (generated from the exe's initialiser table: names need the generator)
MARK = '// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).'


def read(path):
    with open(path, encoding='utf-8', newline='') as f:
        return f.read()


def write(path, text):
    with open(path, 'w', encoding='utf-8', newline='') as f:
        f.write(text)


def clean(text):
    """The text with comments and strings blanked, every offset kept."""
    return gamedata.NOISE.sub(lambda m: re.sub(r'[^\n]', ' ', m.group(0)), text)


def rel(path):
    return os.path.relpath(path, ROOT).replace('\\', '/')


def includes(path, text):
    return [os.path.normpath(os.path.join(os.path.dirname(path), m)) for m in re.findall(r'^\s*#include "([^"]+)"', text, re.M)]


def owner_header(path):
    if path.endswith('.h'):
        return path
    h = path[:-4] + '.h'
    if os.path.exists(h):
        return h
    inc = includes(path, read(path))
    return inc[0] if inc and inc[0].endswith('.h') and os.path.exists(inc[0]) else None


def size_of(t):
    return 4 if t.endswith('*') else SIZES.get(re.sub(r'^const\s+', '', t))


def scan():
    """uses: address -> [(path, start, end, type or None, writes, function)] for every data literal in src."""
    texts = {p: read(p) for p in gamedata.sources()}
    accessors = {}
    for p, t in texts.items():
        accessors[p] = {m.group(2): (m.group(1) or '').strip() for m in ACCESSOR.finditer(t)}
    uses = collections.defaultdict(list)
    for p, text in texts.items():
        c = clean(text)
        acc = dict(accessors[p])
        for inc in includes(p, text):   # (an accessor from a header the file includes)
            for k, v in accessors.get(inc, {}).items():
                acc.setdefault(k, v)
        funcs = [(m.start(), m.group(1).replace('::', '_')) for m in FUNC.finditer(c)]
        simple = {}
        if acc:
            pat = re.compile(r'\b(%s)\s*(?:<\s*([^<>()]+?)\s*>)?\s*\(\s*0[xX]([0-9a-fA-F]+)[uU]?\s*\)' % '|'.join(map(re.escape, acc)))
            for m in pat.finditer(c):
                t = m.group(2) or acc[m.group(1)]
                if t:
                    simple[m.start(3) - 2] = (m, ' '.join(t.split()))
        for m in gamedata.LITERAL.finditer(c):
            v = int(m.group(1), 16)
            if not (gamedata.DATA_LO <= v < gamedata.DATA_HI):
                continue
            fn = ''
            for start, name in funcs:
                if start > m.start():
                    break
                fn = name
            if m.start() in simple:
                s, t = simple[m.start()]
                w = bool(WRITE_AFTER.match(c, s.end()) or WRITE_BEFORE.search(c[max(0, s.start() - 4):s.start()]))
                uses[v].append((p, s.start(), s.end(), t, w, fn))
            else:   # (a scalar read through a cast has a known extent, so it is no base for its neighbours)
                d = DEREF.search(c, max(0, m.start() - 80), m.start())
                n = SIZES.get(d.group(1)) if d and re.match(r'[uU]?\s*\)', c[m.end():m.end() + 3]) else None
                uses[v].append((p, m.start(), m.end(), None, n, fn))
    return uses


def symbols():
    """({address: name} of the data rows, {names of the function rows})"""
    rows, funcs = {}, set()
    for line in read(os.path.join(ROOT, 're', 'symbols.csv')).splitlines()[1:]:
        f = line.split(',', 4)
        if len(f) > 3 and f[2] == 'data':
            rows[int(f[0], 16)] = f[1]
        elif len(f) > 3 and f[2] == 'function':
            funcs.add(f[1])
    return rows, funcs


def analyse():
    """(candidates, skips): candidates are dicts by address; skips {address: reason}."""
    from pe_read import PE
    pe = PE(os.path.join(ROOT, '.data', 'nocd', 'tp.exe'))
    uses = scan()
    taken = set()
    for p in gamedata.sources():
        taken |= set(re.findall(r'\b[A-Za-z_]\w*\b', clean(read(p))))
    names, ported = symbols()
    pointed = set()
    for sname, va, size, ro, rs in pe.sections:
        if sname in ('.text', 'cseg', '.rsrc'):
            continue
        raw = pe.d[ro:ro + rs]
        pointed |= {struct.unpack_from('<I', raw, o)[0] for o in range(0, len(raw) - 3, 4)}
    addrs = sorted(uses)

    def extent(w):   # the bytes the code reaches from w, None when unknown (a pointer passed on, an offset, a table entry)
        ns = [size_of(u[3]) if u[3] else u[4] for u in uses[w]]
        return None if not all(ns) else max(ns)
    extents = {w: extent(w) for w in addrs}
    others = [w for w in addrs if extents[w] is None]   # bases of unknown extent
    cands, skips = {}, {}
    for v in addrs:
        if not (DATA[0] <= v < DATA[1]):
            continue
        us = uses[v]
        types = {u[3] for u in us}
        files = sorted({u[0] for u in us})
        n = size_of(us[0][3]) if us[0][3] else None
        if v in SKIP:
            skips[v] = SKIP[v]
        elif None in types:
            skips[v] = 'another form'
        elif len(types) > 1:
            skips[v] = 'two types'
        elif '[' in us[0][3] or not n:
            skips[v] = 'not a known scalar type (%s)' % us[0][3]
        elif any(B_FILES.match(os.path.basename(f)) for f in files):
            skips[v] = "agent B's file"
        elif any(not rel(f).startswith('src/game/') for f in files):
            skips[v] = 'used outside src/game'
        elif any(f.endswith('.h') for f in files):
            skips[v] = 'used in a header'
        elif any(v < w < v + n for w in addrs) or any(extents[w] and w + extents[w] > v for w in addrs[max(0, bisect.bisect_left(addrs, v) - 8):bisect.bisect_left(addrs, v)]):
            skips[v] = 'overlap'
        elif bisect.bisect_left(others, v) and v - others[bisect.bisect_left(others, v) - 1] < NEAR:
            skips[v] = 'a base %#x below it' % others[bisect.bisect_left(others, v) - 1]
        else:
            s = pe.section_of(v)
            o = v - s[1]
            init = pe.d[s[3] + o:s[3] + o + 4] if o + 4 <= s[4] else b'\0\0\0\0'
            iv = struct.unpack('<I', init.ljust(4, b'\0'))[0] if n >= 4 and v % 4 == 0 else 0
            if any(lo <= iv < hi for lo, hi in gamedata.CODE):
                skips[v] = 'pointer: holds the code address %#x' % iv
            elif any(w in pointed for w in range(v, v + n)):
                skips[v] = 'pointer: an exe dword points at it'
            else:
                writers = sorted({u[0] for u in us if u[4]})
                own = files[0] if len(files) == 1 else (writers[0] if len(writers) == 1 else None)
                header = own and owner_header(own)
                if not own:
                    skips[v] = 'shared, %d writers' % len(writers)
                elif not header or not rel(header).startswith('src/game/'):
                    skips[v] = 'no header'
                else:
                    htext = clean(read(header))
                    missing = [w for w in re.findall(r'[A-Za-z_]\w*', us[0][3]) if w not in KEYWORDS and not re.search(r'\b%s\b' % w, htext)]
                    name = names.get(v)
                    if missing:
                        skips[v] = 'type %s not in %s' % (missing[0], rel(header))
                    elif name and (name in taken or not re.match(r'^[A-Za-z_]\w*$', name)):
                        skips[v] = 'the symbols.csv name %s is taken' % name
                    else:
                        fns = sorted({u[5] if u[5] in ported else '' for u in us})
                        how = 'symbols.csv'
                        if not name:
                            common = []
                            for parts in zip(*[f.split('_') for f in fns]):   # (a use outside a ported function: the file's name)
                                if len(set(parts)) > 1:
                                    break
                                common.append(parts[0])
                            prefix = '_'.join(common) or ''.join(w.capitalize() for w in re.split(r'[^A-Za-z0-9]', os.path.basename(own)[:-4]) if w)
                            name = 'g%s_%06x' % (prefix, v)
                            how = 'its use in %s' % (', '.join(fns) if all(fns) else os.path.basename(own))
                        cands[v] = dict(addr=v, type=us[0][3], size=n, name=name, how=how, header=header, uses=us, files=files, new=v not in names)
    return cands, skips


def apply(cands):
    lines_by_header = collections.defaultdict(list)
    edits = collections.defaultdict(list)    # path -> [(start, end, text)]
    incs = collections.defaultdict(set)      # path -> headers to include
    for c in cands:
        lines_by_header[c['header']].append('GAME_DATA(%s, %s, %#010x, %d);' % (c['type'], c['name'], c['addr'], c['size']))
        for p, s, e, _, _, _ in c['uses']:
            edits[p].append((s, e, c['name']))
            if p != c['header'] and owner_header(p) != c['header'] and c['header'] not in includes(p, read(p)):
                incs[p].add(c['header'])
    for p in sorted(set(edits) | set(lines_by_header)):
        text = read(p)
        nl = '\r\n' if '\r\n' in text else '\n'
        for s, e, name in sorted(edits.get(p, []), reverse=True):
            text = text[:s] + name + text[e:]
        if p in incs:
            lines = text.split(nl)
            last = max(i for i, l in enumerate(lines) if l.startswith('#include'))
            for h in sorted(incs[p]):
                lines.insert(last + 1, '#include "%s"' % os.path.relpath(h, os.path.dirname(p)).replace('\\', '/'))
            text = nl.join(lines)
        if p in lines_by_header:
            lines = text.split(nl)
            if MARK in lines:
                i = lines.index(MARK) + 1
                j = i
                while j < len(lines) and lines[j].startswith('GAME_DATA('):
                    j += 1
                block = lines[i:j] + lines_by_header[p]
                lines[i:j] = sorted(block, key=lambda l: int(re.search(r', (0x[0-9a-f]+), ', l).group(1), 16))
            else:
                end = len(lines)
                while end and not lines[end - 1].strip():
                    end -= 1
                if lines[end - 1].startswith('#endif'):
                    end -= 1
                gd = os.path.relpath(os.path.join(ROOT, 'src', 'game', 'core', 'gamedata.h'), os.path.dirname(p)).replace('\\', '/')
                head = [] if re.search(r'#include "[^"]*gamedata\.h"', text) else ['#include "%s"' % gd, '']
                lines[end:end] = [''] + head + [MARK] + sorted(lines_by_header[p], key=lambda l: int(re.search(r', (0x[0-9a-f]+), ', l).group(1), 16))
            text = nl.join(lines)
        write(p, text)
    sym = os.path.join(ROOT, 're', 'symbols.csv')
    text = read(sym)
    nl = '\r\n' if '\r\n' in text else '\n'
    rows = ['%#010x,%s,data,low,"GAME_DATA (%s): %s, named for %s (tools/gamedata_codemod.py)"' % (
        c['addr'], c['name'], rel(c['header'])[:-2], c['type'], c['how']) for c in cands if c['new']]
    if rows:
        write(sym, text.rstrip('\r\n') + nl + nl.join(rows) + nl)


def benches(rev):
    """The benches that call a ported function whose lines changed since rev (they name its address other than as a range)."""
    import subprocess
    diff = subprocess.run(['git', 'diff', '-U0', rev, '--', 'src/*.cpp'], capture_output=True, text=True, cwd=ROOT).stdout
    changed = collections.defaultdict(set)
    path = None
    for line in diff.splitlines():
        if line.startswith('+++ '):
            path = os.path.join(ROOT, line[6:]) if line != '+++ /dev/null' else None
        m = re.match(r'@@ -\S+ \+(\d+)(?:,(\d+))? @@', line)
        if m and path:
            first = int(m.group(1))
            changed[path] |= set(range(first, first + int(m.group(2) or 1)))
    names, ported = symbols()
    funcs = set()
    for p, lines in changed.items():
        c = clean(read(p))
        defs = [(c.count('\n', 0, m.start()) + 1, m.group(1).replace('::', '_')) for m in FUNC.finditer(c)]
        for n in lines:
            inside = [name for at, name in defs if at <= n]
            if inside and inside[-1] in ported:
                funcs.add(inside[-1])
            elif inside:   # (a helper: every ported function of the file)
                funcs |= {name for _, name in defs}
    addrs = set()
    for line in read(os.path.join(ROOT, 're', 'symbols.csv')).splitlines()[1:]:
        f = line.split(',', 3)
        if len(f) > 2 and f[2] == 'function' and f[1] in funcs:
            addrs.add(int(f[0], 16))
    out = []
    folder = os.path.join(ROOT, 'tests', 'fncmp')
    for f in sorted(os.listdir(folder)):
        if f.endswith('.py'):
            text = re.sub(r'\(\s*0x[0-9a-fA-F]+\s*,\s*0x[0-9a-fA-F]+\s*\)', '', read(os.path.join(folder, f)))   # (not as a callee's range)
            found = {int(x, 16) for x in re.findall(r'\b0x([0-9a-fA-F]{5,8})\b', text)}
            if found & addrs and '.main(' in read(os.path.join(folder, f)):
                out.append('tests/fncmp/' + f)
    return out


def main():
    a = sys.argv[1:]
    if a[:1] == ['benches'] and len(a) == 2:
        print('\n'.join(benches(a[1])))
        return
    if a[:1] not in (['list'], ['skips'], ['apply']):
        sys.exit(__doc__)
    cands, skips = analyse()
    if a[0] == 'list':
        print('%d candidates, %d left alone' % (len(cands), len(skips)))
        for k, n in collections.Counter(rel(c['header']).split('/')[2] for c in cands.values()).most_common():
            print('  %-12s %d' % (k, n))
        for k, n in collections.Counter(re.sub(r' (0x|[A-Za-z_]+ not in |\().*', '', r) for r in skips.values()).most_common():
            print('  skip %-40s %d' % (k, n))
    elif a[0] == 'skips':
        for v in sorted(skips):
            print('%#010x %s' % (v, skips[v]))
    else:
        dirs = a[2:]
        todo = sorted((c for c in cands.values() if not dirs or any(rel(c['header']).startswith('src/game/%s/' % d.strip('/')) for d in dirs)),
                      key=lambda c: (rel(c['header']), c['addr']))
        # whole headers only, so a slice ends at a module's edge
        n, chosen = int(a[1]), []
        for c in todo:
            if len(chosen) >= n and c['header'] != chosen[-1]['header']:
                break
            chosen.append(c)
        apply(chosen)
        print('converted %d addresses, %d uses, in %d headers' % (len(chosen), sum(len(c['uses']) for c in chosen), len({c['header'] for c in chosen})))


if __name__ == '__main__':
    main()
