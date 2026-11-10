"""The codemod of step 6 (.docs/09-globals.md): a .data scalar that the code reaches only as global<T>(0xADDRESS) (any accessor a file
defines as `template <typename T> T &NAME(u32 address)`, its default type included), with one type, becomes a GAME_DATA line in its owner's
header, and every use becomes the name. Rerunnable: a converted address is no longer a literal, so a second run finds only the rest.

usage: gamedata_codemod.py list                 the candidates by owner folder, and the addresses left alone, by reason
       gamedata_codemod.py skips                every address left alone with its reason (the pointer ones are the fixup round's)
       gamedata_codemod.py apply N [DIR...]     convert the first N candidates (by owner header) whose owner is under src/game/DIR
       gamedata_codemod.py benches REV          the fncmp benches that run a ported function whose lines changed since REV
       gamedata_codemod.py objects [--avoid F]  the objects mode (round 77): every remaining data literal grouped into objects, by owner
                                                folder, and the groups left alone, by reason. F: files not to touch (one path a line,
                                                as git diff --name-only prints them: another agent's)
       gamedata_codemod.py object-skips [--avoid F]           each group left alone with its reason
       gamedata_codemod.py apply-objects N [DIR...] [--avoid F]  convert the first N objects (whole headers) whose owner header's path
                                                starts with src/game/DIR

The objects mode: an object starts at a literal address and runs to the end its uses reach: a typed scalar's size, the largest constant
offset (+ 4), a text's zero (padded to 4 with zeros), else (an index, a pointer passed on) to the next base of unknown extent in src, a
static object or a converted object; a base within GAP above the last takes it in (one table's columns). Every literal in the object
becomes its name and offset: (u32)(uintptr_t)&gName + 0xOFF (after a (uintptr_t) cast, &gName + 0xOFF); a lone scalar used with one
type becomes the name. The type: that scalar's, char[n] for a text, else u8[n] (const in .rdata). Left alone: an object with any use
that cannot be an address expression (a file-scope initialiser, constexpr, case, a template argument, asm, a header, outside
src/game, a file in --avoid), a bare literal that may be a value (no memory operand of _re/text.asm at it, no text there, or a
pattern 0xRRRRRR / 0xXXXX0000), the import address table, and what gamedata.py would reject (checked on the new set).

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
BASIC = ('u8', 's8', 'u16', 's16', 'u32', 's32', 'u64', 's64', 'f32', 'f64')   # (re/types.h's, which apply includes when a header lacks it)
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
                uses[v].append((p, s.start(), s.end(), t, w, fn, s.start(3) - 2, s.end(3)))
            else:
                d = DEREF.search(c, max(0, m.start() - 80), m.start())
                close = re.compile(r'[uU]?\s*\)(?!\s*(?:\[|->|\.(?!\d)))').match(c, m.end())
                if d and close and SIZES.get(d.group(1)):   # *reinterpret_cast<T *>(0xADDRESS): the scalar form too
                    t = ' '.join(d.group(1).split())
                    w = bool(WRITE_AFTER.match(c, close.end()) or WRITE_BEFORE.search(c[max(0, d.start() - 4):d.start()]))
                    uses[v].append((p, d.start(), close.end(), t, w, fn, m.start(), m.end()))
                    continue
                uses[v].append((p, m.start(), m.end(), None, None, fn, m.start(), m.end()))
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
        lines_by_header[c['header']].append('GAME_DATA(%s, %s, %#010x, %#x);' % (c['type'], c['name'], c['addr'], c['size']))
        for p, s, e, text in c.get('edits') or [(u[0], u[1], u[2], c['name']) for u in c['uses']]:
            edits[p].append((s, e, text))
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
                if not re.search(r'#include "[^"]*re/types\.h"', text) and any(re.search(r'\b(%s)\b' % '|'.join(BASIC), l) for l in lines_by_header[p]):
                    head = ['#include "%s"' % os.path.relpath(os.path.join(ROOT, 're', 'types.h'), os.path.dirname(p)).replace('\\', '/')] + (head or [''])
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


ASM_BRACKET = re.compile(r'\[([^\]]*)\]')
ASM_NUMBER = re.compile(r'\b0x([0-9a-fA-F]{6,8})\b')
TEMPLATE_ARG = re.compile(r'\w\s*<[^<>()]*$')
GAP = 0x40   # an unknown extent ending at a base within this of the last base takes it in (objects())
UNSAFE_LINE = re.compile(r'^\s*#|\bconstexpr\b|\bstatic_assert\b|\benum\b|\b__asm\b|\bptr\b')


def asm_refs():
    """The data addresses inside a memory operand of _re/text.asm ([A], [EAX*0x4 + A]). Not taken for object starts: the compiler
    moves an indexed displacement (a[i + 1] is [EAX*8 + A + 8], 0xfbe4d8 in rastertriangle's 0xfbe2e0 table), and an object that ends
    too early is the unsafe error (a larger one only converts more together)."""
    memory = set()
    with open(os.path.join(ROOT, '_re', 'text.asm'), encoding='utf-8', errors='replace') as f:
        for line in f:
            for b in ASM_BRACKET.finditer(line[10:]):
                memory |= {int(x, 16) for x in ASM_NUMBER.findall(b.group(1))}
    return {v for v in memory if gamedata.DATA_LO <= v < gamedata.DATA_HI}


ADDRESS_WRAP = re.compile(r'(?:\breinterpret_cast\s*<[^<>;]*\*\s*(?:const\s*)?>\s*\(|\(\s*(?:const\s+)?\w+(?:\s+\w+)?\s*\*+\s*(?:const\s*)?\)\s*|'
                          r'\b(?!static_cast\b)\w+\s*<[^<>;()]*>\s*\()\s*(?:\(\s*(?:std::)?uintptr_t\s*\)\s*|(?:std::)?uintptr_t\s*\{\s*)?$')


def address_use(pe, c, u, v, memory):
    """True when the literal is an address, not a value that lies in the data range (a colour 0x808080, a mask): a typed accessor or
    a deref, the first argument of a pointer cast or a template accessor; else (bare) the exe reads memory at it, or it is a text."""
    if u[3] or ADDRESS_WRAP.search(c, max(0, u[6] - 80), u[6]):
        return True
    b = v.to_bytes(4, 'little')
    if b[0] == b[1] == b[2] or not v & 0xffff:   # (a pattern value, never taken for an address bare)
        return False
    if v in memory:
        return True
    s = pe.section_of(v)
    raw = pe.d[s[3] + v - s[1]:s[3] + v - s[1] + 64] if s and v - s[1] < s[4] else b''
    return bool(re.match(rb'[\t\n\r\x20-\x7e]{2,}\0', raw))


def bodies(c):
    """[(start, end)] of the function bodies of a cleaned text."""
    out = []
    for m in FUNC.finditer(c):
        depth, i = 0, m.end() - 1
        for i in range(m.end() - 1, len(c)):
            depth += {'{': 1, '}': -1}.get(c[i], 0)
            if not depth:
                break
        out.append((m.end(), i))
    return out


def unsafe(path, c, inside_fn, s, e, avoid):
    """Why the literal at c[s:e] cannot become an address expression, else None."""
    if rel(path) in avoid:
        return 'a file another agent changes'
    if not rel(path).startswith('src/game/') or path.endswith('.h'):
        return 'used in a header or outside src/game'
    if not inside_fn:
        return 'a file-scope initialiser'   # (it would need a dynamic initialiser)
    ls = c.rfind('\n', 0, s) + 1
    if UNSAFE_LINE.search(c[ls:c.find('\n', s)]) or re.search(r'\bcase\s*$', c[ls:s]) or TEMPLATE_ARG.search(c[ls:s]):
        return 'a constant expression, a template argument or asm'
    statement = c[max(c.rfind(';', 0, s), c.rfind('}', 0, s), c.rfind('{', 0, s)) + 1:s]
    if re.search(r'\bconstexpr\b|\b__asm\b', statement) or '__asm' in c[max(0, s - 400):s] and c.rfind('}', 0, s) < c.rfind('__asm', 0, s):
        return 'a constant expression, a template argument or asm'
    return None


def objects(avoid=()):
    """(objects, skips): the remaining data literals grouped into objects of their real extent (.docs/09-globals.md, "The codemod"),
    each with its declaration and the rewrite of every use, or the reason it is left alone."""
    from pe_read import PE
    pe = PE(os.path.join(ROOT, '.data', 'nocd', 'tp.exe'))
    d = struct.unpack_from('<I', pe.d, 0x3c)[0] + 24 + 96 + 12 * 8
    iat, iat_size = struct.unpack_from('<II', pe.d, d)
    iat += pe.base
    uses = scan()
    decls = gamedata.declarations()
    inside, starts = gamedata.finder(decls), [x[2] for x in decls]
    texts, fns = {}, {}
    for us in uses.values():
        for u in us:
            if u[0] not in texts:
                texts[u[0]] = clean(read(u[0]))
                fns[u[0]] = bodies(texts[u[0]])
    st = ''.join(gamedata.NOISE.sub('', read(os.path.join(ROOT, 'src', 'game', 'core', f))) for f in gamedata.STATIC_SOURCES)
    reach = collections.defaultdict(lambda: None)   # address: the end its uses reach, None when unknown (a base)
    known = collections.defaultdict(int)
    for p, c in texts.items():
        for r in (gamedata.REACH_PLUS, gamedata.REACH_AT):
            for m in r.finditer(c):
                known[int(m.group(1), 16)] = max(known[int(m.group(1), 16)], int(m.group(2), 0) + 4)
    memory = asm_refs()
    bases = set(gamedata.statics(st))
    indexed, textual = set(), set()
    for v, us in uses.items():
        ends = []
        for u in us:
            c = texts[u[0]]
            if u[3]:
                ends.append(v + size_of(u[3]) if size_of(u[3]) else None)
            elif re.match(r'[uU]?\s*\+\s*' + gamedata.OFFSET + r'(?!\s*[\w*(\[/%.])', c[u[7]:u[7] + 24]) or re.match(r'[uU]?\s*\)\s*\)?\s*,\s*' + gamedata.OFFSET + r'\s*\)', c[u[7]:u[7] + 24]):
                ends.append(v + known[v])
            else:
                ends.append(None)
                indexed |= {v} if re.match(r'[uU]?\s*(?:[+\-]|[)}]\s*\)?\s*\[)', c[u[7]:u[7] + 24]) else set()
        reach[v] = None if None in ends else max(ends)
        if reach[v] is None and v not in indexed:   # (a text passed on: to its zero, padded to 4 with zeros)
            sec = pe.section_of(v)
            raw = pe.d[sec[3] + v - sec[1]:sec[3] + sec[4]][:0x400] if sec and v - sec[1] < sec[4] else b''
            t = re.match(rb'[\t\n\r\x20-\x7e]{2,}\0', raw)
            if t:
                end = v + len(t.group(0))
                while end % 4 and end - v < len(raw) and not raw[end - v]:
                    end += 1
                reach[v] = end
                textual.add(v)
        if not all(u[3] for u in us):   # (a text's start too: an unknown extent below it ends there)
            bases.add(v)
    stops = set(starts) | set(gamedata.statics(st))
    bases = sorted(bases | set(starts))
    names, ported = symbols()
    taken = set()
    for p in gamedata.sources():
        taken |= set(re.findall(r'\b[A-Za-z_]\w*\b', clean(read(p))))
    addrs = sorted(v for v in uses if gamedata.DATA_LO <= v < gamedata.DATA_HI and not inside(v))

    def next_base(w):
        i = bisect.bisect_right(bases, w)
        return bases[i] if i < len(bases) else gamedata.DATA_HI
    groups, i = [], 0
    while i < len(addrs):
        a = addrs[i]
        end, last, members = (a + 1, False), a, []   # (end: (address, whether unknown); last: the last base of unknown extent)
        while True:
            while i < len(addrs) and addrs[i] < end[0]:
                w = addrs[i]
                members.append(w)
                end = max(end, (reach[w], False) if reach[w] else (next_base(w), True))
                last = last if reach[w] else w
                i += 1
            # an unknown extent that ends at another base close above the last: columns of one table ([EBX + A], [EBX + A + 8]),
            # or a part of a struct passed on, so one object
            b, unknown = end
            if unknown and b not in stops and b - last <= GAP and b < gamedata.DATA_HI:
                last = b
                end = (b + 1, False) if i < len(addrs) and addrs[i] == b else (next_base(b), True)
                continue
            break
        groups.append((a, end[0], members))
    objs, skips = {}, {}
    for a, end, members in groups:
        j = bisect.bisect_right(starts, a)
        if j < len(starts) and starts[j] < end:
            if members[-1] > starts[j] or all(reach[w] for w in members):
                skips[a] = 'reaches a converted object'
                continue
            end = starts[j]   # (an unknown extent ends at a converted object)
        s = pe.section_of(a)
        if iat <= a < iat + iat_size:
            skips[a] = 'the import address table'
            continue
        if not s or s[0] in ('.text', 'cseg', '.rsrc'):
            skips[a] = 'not in a data section'
            continue
        if end > s[1] + s[2]:
            if members[-1] >= s[1] + s[2] or all(reach[w] for w in members):
                skips[a] = 'across a section end'
                continue
            end = s[1] + s[2]
        us = [(w, u) for w in members for u in uses[w]]
        why = None
        for w, u in us:
            c = texts[u[0]]
            why = unsafe(u[0], c, any(lo <= u[1] < hi for lo, hi in fns[u[0]]), u[6], u[7], avoid)
            if not why and not address_use(pe, c, u, w, memory):
                why = 'a value, not known to be an address'
            if why:
                break
        if why:
            skips[a] = why
            continue
        n = end - a
        files = sorted({u[0] for _, u in us})
        writers = sorted({u[0] for _, u in us if u[4]})
        count = collections.Counter(u[0] for _, u in us)
        own = files[0] if len(files) == 1 else writers[0] if len(writers) == 1 else max(files, key=lambda f: (count[f], f))
        header = owner_header(own)
        if not header or not rel(header).startswith('src/game/') or rel(header) in avoid:
            skips[a] = 'no header'
            continue
        ro = s[0] == '.rdata'
        types = {u[3] for _, u in us}
        htext = clean(read(header))
        missing = lambda t: [x for x in re.findall(r'[A-Za-z_]\w*', re.sub(r'\[[^\]]*\]', '', t))   # noqa: E731
                             if x not in KEYWORDS and x not in BASIC and not re.search(r'\b%s\b' % x, htext)]
        scalar = len(members) == 1 and len(types) == 1 and None not in types and n == size_of(next(iter(types))) and not missing(next(iter(types)))
        text = members == [a] and a in textual and n == reach[a] - a
        t = next(iter(types)) if scalar else 'char[%#x]' % n if text else 'u8[%#x]' % n
        if ro and not t.startswith('const '):
            t = 'const ' + t
        name = names.get(a)
        how = 'symbols.csv'
        if not name or name in taken or not re.match(r'^[A-Za-z_]\w*$', name):
            fs = sorted({u[5] if u[5] in ported else '' for _, u in us})
            common = []
            for parts in zip(*[f.split('_') for f in fs]):
                if len(set(parts)) > 1:
                    break
                common.append(parts[0])
            prefix = '_'.join(common) or ''.join(x.capitalize() for x in re.split(r'[^A-Za-z0-9]', os.path.basename(own)[:-4]) if x)
            name = 'g%s%s_%06x' % ('Text_' if text else '', prefix, a)
            how = 'its use in %s' % (', '.join(fs) if all(fs) else os.path.basename(own))
        edits = []
        for w, u in us:
            if scalar and u[3]:
                edits.append((u[0], u[1], u[2], name))
            else:
                c = texts[u[0]]
                e = u[7] + len(re.match(r'[uU]?', c[u[7]:]).group(0))
                if re.search(r'\(\s*(?:std::)?uintptr_t\s*\)\s*$', c[max(0, u[6] - 30):u[6]]):   # ((uintptr_t)0xADDRESS: the cast is there)
                    edits.append((u[0], u[6], e, '&%s' % name if w == a else '&%s + %#x' % (name, w - a)))
                else:   # (no parentheses around a sum that is a whole argument)
                    bare = re.search(r'[(,]\s*$', c[max(0, u[6] - 8):u[6]]) and re.match(r'\s*[),]', c[e:e + 8])
                    edits.append((u[0], u[6], e, '(u32)(uintptr_t)&%s' % name if w == a else ('(u32)(uintptr_t)&%s + %#x' if bare else
                                                                                              '((u32)(uintptr_t)&%s + %#x)') % (name, w - a)))
        objs[a] = dict(addr=a, type=t, size=n, name=name, how=how, header=header, uses=[u for _, u in us], files=files, edits=edits,
                       new=a not in names, members=members)
    # what gamedata.py would reject: a static object's member, a pointer the fixups cannot express, a literal base that reaches one
    from gen_loader_slots import hooked_addresses, slots
    code_slots = set(slots(pe, hooked_addresses()))
    while True:
        new = [(o['type'], o['name'], o['addr'], o['size'], o['header']) for o in objs.values()]
        alld = sorted(decls + new, key=lambda x: x[2])
        errors = gamedata.static_errors(alld, st) + gamedata.pointers(pe, alld, code_slots)[1]
        for p, c in texts.items():
            errors += gamedata.reach_errors(rel(p), c, alld)
        by_name = {o['name']: a for a, o in objs.items()}
        bad = {}
        for err in errors:
            for word in re.findall(r'\b[A-Za-z_]\w*\b', err):
                if word in by_name:
                    bad.setdefault(by_name[word], err)
        if not bad:
            return objs, skips
        for a, err in bad.items():
            skips[a] = 'gamedata.py: ' + re.sub(r'0x[0-9a-f]+|\b\w+_[0-9a-f]{6}\b', 'N', err)[:70]
            del objs[a]


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
    avoid = set()
    if '--avoid' in a:
        i = a.index('--avoid')
        avoid = set(read(a[i + 1]).split())
        del a[i:i + 2]
    if a[:1] in (['objects'], ['object-skips'], ['apply-objects']):
        objs, skips = objects(avoid)
        if a[0] == 'objects':
            print('%d objects (%d addresses, %d uses), %d left alone' % (
                len(objs), sum(len(o['members']) for o in objs.values()), sum(len(o['uses']) for o in objs.values()), len(skips)))
            for k, n in collections.Counter(rel(o['header']).split('/')[2] for o in objs.values()).most_common():
                print('  %-12s %d' % (k, n))
            for k, n in collections.Counter(re.sub(r' \(.*', '', r) for r in skips.values()).most_common():
                print('  skip %-70s %d' % (k, n))
        elif a[0] == 'object-skips':
            for v in sorted(skips):
                print('%#010x %s' % (v, skips[v]))
        else:
            dirs = a[2:]
            todo = sorted((o for o in objs.values() if not dirs or any(rel(o['header']).startswith('src/game/%s' % d) for d in dirs)),
                          key=lambda o: (rel(o['header']), o['addr']))
            n, chosen = int(a[1]), []
            for o in todo:
                if len(chosen) >= n and o['header'] != chosen[-1]['header']:
                    break
                chosen.append(o)
            apply(chosen)
            print('converted %d objects (%d addresses, %d uses) in %d headers' % (
                len(chosen), sum(len(o['members']) for o in chosen), sum(len(o['edits']) for o in chosen), len({o['header'] for o in chosen})))
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
