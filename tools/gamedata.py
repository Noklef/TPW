"""The game's globals as named objects (.docs/09-globals.md). A module header declares each once (src/game/core/gamedata.h):
    GAME_DATA(type, name, 0xADDRESS, size);
This generates each target's definitions at build time (CMake), into the build folder; nothing of the exe goes into the repo.

usage: gamedata.py address OUT        the hook DLL's, the tests' and the driver's: each name bound to its original address. Runs the
                                      check first, so a build with a converted address still used as a literal fails.
       gamedata.py object EXE OUT H   tpw.exe's: each name its own object, its initial bytes the exe's (zero past a section's raw data),
                                      kGameDataRanges, which the loader zeroes in the copy, and kGameDataFixups, which it applies at
                                      start-up: a data dword (in the copy or in an object) that points into a converted range gets the
                                      object's address, an object's dword that holds a code address the port (pointers()). H: the
                                      same in original addresses for tpw_tests. Fails on a pointer the fixups cannot express.
       gamedata.py check              the declarations (in a data section, no overlaps, one name each) and every source of src/: no
                                      literal inside a converted range outside its GAME_DATA line (comments and strings left out).
"""
import bisect
import os
import re
import struct
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools'))

DECL = re.compile(r'^\s*GAME_DATA\(\s*([^,]+?)\s*,\s*(\w+)\s*,\s*(0x[0-9a-fA-F]+)\s*,\s*(0x[0-9a-fA-F]+|\d+)\s*\)\s*;', re.M)
NOISE = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'', re.S)
LITERAL = re.compile(r'\b0[xX]([0-9a-fA-F]+)')
DATA_LO, DATA_HI = 0x6fd000, 0xfc1000          # .rdata to the last small data section (not .text, cseg or .rsrc)
CODE = ((0x401000, 0x6fc800),)                 # .text and cseg


def sources():
    for root, _, files in os.walk(os.path.join(ROOT, 'src')):
        for f in sorted(files):
            if f.endswith(('.h', '.cpp')):
                yield os.path.join(root, f)


def declarations():
    """(type, name, address, size, header) for every GAME_DATA line of src/game, by address."""
    out = []
    for path in sources():
        if path.endswith('gamedata.h'):
            continue
        text = open(path, encoding='utf-8').read()
        for m in DECL.finditer(text):
            out.append((m.group(1), m.group(2), int(m.group(3), 16), int(m.group(4), 0), path))
    return sorted(out, key=lambda d: d[2])


def finder(decls):
    """inside(v): the name of the converted range holding v, else None."""
    starts = [d[2] for d in decls]

    def inside(v):
        i = bisect.bisect_right(starts, v) - 1
        return decls[i][1] if i >= 0 and v < decls[i][2] + decls[i][3] else None
    return inside


def check(decls):
    errors = []
    names = set()
    for i, (_, name, a, n, path) in enumerate(decls):
        if not (DATA_LO <= a and a + n <= DATA_HI and n > 0):
            errors.append('%s %#x+%#x: not inside the data sections' % (name, a, n))
        if i and decls[i - 1][2] + decls[i - 1][3] > a:
            errors.append('%s %#x overlaps %s' % (name, a, decls[i - 1][1]))
        if name in names:
            errors.append('%s declared twice' % name)
        names.add(name)
    inside = finder(decls)
    for path in sources():
        text = NOISE.sub(lambda m: '\n' * m.group(0).count('\n'), open(path, encoding='utf-8').read())
        for line_no, line in enumerate(text.split('\n'), 1):
            if DECL.match(line):
                continue
            for m in LITERAL.finditer(line):
                v = int(m.group(1), 16)
                if inside(v):
                    errors.append('%s:%d: %#x is inside %s: use the name' % (os.path.relpath(path, ROOT), line_no, v, inside(v)))
        errors += reach_errors(os.path.relpath(path, ROOT), text, decls)
    text = ''.join(NOISE.sub('', open(os.path.join(ROOT, 'src', 'game', 'core', f), encoding='utf-8').read()) for f in STATIC_SOURCES)
    return errors + static_errors(decls, text)


OFFSET = r'(0[xX][0-9a-fA-F]+|\d+)[uU]?'
POINTER = r'(?:reinterpret_cast\s*<[^<>;()]*>\s*\(\s*(?:\(\s*(?:std::)?uintptr_t\s*\)\s*)?|&\s*\w+\s*<[^<>;()]*>\s*\(\s*)'
REACH_PLUS = re.compile(r'\b0[xX]([0-9a-fA-F]+)[uU]?\s*\+\s*' + OFFSET + r'(?![\w*(])')
REACH_AT = re.compile(r'\bat\s*<[^<>;()]*>\s*\(\s*' + POINTER + r'0[xX]([0-9a-fA-F]+)[uU]?\s*\)\s*\)?\s*,\s*' + OFFSET + r'\s*\)')
REACH_ALIAS = re.compile(r'\b(\w+)\s*=\s*' + POINTER + r'0[xX]([0-9a-fA-F]+)[uU]?\s*\)\s*\)?\s*;')


def reach_errors(rel, text, decls):
    """A literal base whose constant offset reaches a converted object it is not inside: 0xBASE + n, at<T>(cast(0xBASE), n), and a
    local set to cast(0xBASE) then at<T>(local, n) or local + n in the rest of its function. The base and the object would split
    in tpw.exe (the literal check sees only the base). Offsets from an index, a call's argument or another variable are not seen."""
    inside, starts = finder(decls), [d[2] for d in decls]
    found = [(m.start(), int(m.group(1), 16), int(m.group(2), 0)) for r in (REACH_PLUS, REACH_AT) for m in r.finditer(text)]
    for m in REACH_ALIAS.finditer(text):
        end = re.compile(r'^\}', re.M).search(text, m.end())
        name = re.escape(m.group(1))
        use = re.compile(r'\bat\s*<[^<>;()]*>\s*\(\s*%s\s*,\s*%s\s*\)|\b%s\s*\+\s*%s(?![\w*(])' % (name, OFFSET, name, OFFSET))
        found += [(u.start(), int(m.group(2), 16), int(u.group(1) or u.group(2), 0))
                  for u in use.finditer(text, m.end(), end.start() if end else len(text))]
    errors = []
    for at, base, offset in found:
        i = bisect.bisect_right(starts, base)
        if DATA_LO <= base < DATA_HI and not inside(base) and i < len(starts) and starts[i] <= base + offset:
            errors.append('%s:%d: %#x + %#x reaches %s at %#x: use its name, or return it to a literal' % (
                rel, text.count('\n', 0, at) + 1, base, offset, decls[i][1], starts[i]))
    return errors


STATIC_FUNCS = {   # function: (size, why) of the object it reaches from this, for the static objects staticinit calls it on
    0x5f60b0: (0x14, 'String_CtorStr: String is 0x14 bytes (core/string.h asserts it)'),
    0x5f61e0: (0x14, 'String_Dtor: String'),
    0x5f6980: (0x14, 'WString_CtorStr: WString is 0x14 bytes (core/string.h asserts it)'),
    0x5f6a60: (0x14, 'WString_Dtor: WString'),
    0x5fa680: (0x28, 'SurfaceInfo_Ctor: SurfaceInfo is 0x28 bytes (gfx/surface.h asserts it)'),
    0x5fa6a0: (0x28, 'SurfaceInfo_Init: SurfaceInfo (writes up to +0x24)'),
    0x6307e0: (0x4, 'Semaphore_Dtor: the semaphore\'s handle (CloseHandle on +0, nothing else)'),
    0x4f08c0: (0x58, 'WFileBuf_Dtor: basic_filebuf<wchar_t>, 0x58 bytes: the static ones at 0xfb1c18 and 0xfb1c70 lie 0x58 apart, and the '
                     'destructor reaches +0x54'),
    0x47ddc0: (0x5, 'PalColour_Set: the colour\'s bytes +0-+3 and its palette index +4'),
    0x5c8b90: (0x18, 'CritSec_Ctor: a CRITICAL_SECTION (0x18 bytes)'),
    0x5c8ba0: (0x18, 'CritSec_Dtor: a CRITICAL_SECTION (0x18 bytes)'),
    0x492020: (0x1c, 'Wrap_Fn_669fa8_492020: Fn_669fa8 writes +0-+0x14, then +0 and +0x18'),
    0x565110: (0x28, 'Vec3_DirectionsFromAngle: writes +4-+0x24 (staticinit the angle +0 before it)'),
    0x419570: (0x5a0, 'Language_Ctor: the language object, its last field +0x59c (core/language.h)'),
    0x4196a0: (0x5a0, 'Language_Dtor: the language object'),
    0x659d00: (0xc, 'GuiRect_Ctor: the vtable and four words (a dialog holds two 0xc apart, +0x14 and +0x20)'),
    0x659d90: (0xc, 'GuiRect_Dtor: GuiRect'),
}
STATIC = {         # static object: (size, why), where its functions do not say it
    0x74c348: (0x6, 'the text "Dutch", an argument'),
    0x77537c: (0x19, 'the text ".\\data\\2dmap\\vsprite.tga", an argument'),
    0x7c24c0: (0x8, 'Dialup (WEARASR.dll\'s class, its size not in tp.exe): the game\'s own globals start at 0x7c24c8 (0x485b40 reads '
                    'it), nothing reads 0x7c24c4'),
    0x8751f0: (0x240, '16 elements of 0x24 (CRT_VectorDtorIterator\'s arguments)'),
    0xfa71f8: (0x400, 'the palette Palette_Nearest searches: 256 colours of 4 bytes'),
    0x7b0562: (0x3a, 'the loop pointer of 10 records of 5 bytes from 0x7b0560; the colour ramps (0x7b0480) end with 0x7b0594 and 0x7b0598'),
    0x879df0: (0x40, 'staticinit zeroes 0x10 dwords from it'),
    0x87b0d0: (0x40, 'staticinit zeroes 0x10 dwords from it'),
    0x8c4858: (0x40, 'staticinit zeroes 0x10 dwords from it'),
    0x7ca178: (0x50, 'the panel (Fn_48c9e0 writes +0-+0x1c, its code reaches +0x2c); 0x7ca1c8 is the dialog\'s window pointer, a global of its '
                     'own (ui/dialog7ca1c8.h)'),
    0x7cc298: (0x50, 'the panel (Fn_48c9e0 writes +0-+0x1c, its code reaches +0x34); 0x7cc2e8 is a float staticinit sets on its own'),
    0x785a08: (0x3d8, 'ActionRec (ActionRec_Ctor writes up to +0x3d0); 0x785de0 and 0x785de4 are a tree\'s globals (core/stdtree), 0x785de8 '
                      'the game error'),
    0x7b4ac0: (0xc4,'ScreenSurfaces (ScreenSurfaces_Ctor writes up to +0xb0); 0x7b4b84 is the pointer\'s global (gfx/pointer)'),
    0x7cc408: (0x3c, 'the panel (Fn_48c9e0 writes +0-+0x1c); the dialog at 0x7cc444 follows (ui/dialog7cc444)'),
    0x7cf0d8: (0x30, 'SteerSeparation (its constructor writes the vtable and the name at +4); the route at 0x7cf108 follows (world/nav)'),
    0x7cf360: (0x2c, 'SteerAvoidObstacle (its constructor writes the vtable and the name at +4); 0x7cf38c on are the line stepper\'s '
                     'and mapcell\'s globals (world/linestep, world/mapcell)'),
    0x8bcbc8: (0x940, 'the view (View_Defaults writes up to +0x924); 0x8bd508-0x8bd52c are the renderer\'s globals '
                      '(staticinitmanual sets them on their own)'),
    0x8c1828: (0x2f20, 'the landscape (Landscape_Init writes up to +0x2ecc); the load screen at 0x8c4748 follows (gfx/loadscreen)'),
    0xf75b20: (0x68, 'Panel7b9940, as the one at 0x7b9940 (0x68 to the next static object; Panel7b9940_Ctor writes up to +0x5c); the '
                     'advisor\'s globals follow (0xf79660 on)'),
    0x7b4e38: (0x3ac,'ImeHost (its code reaches +0x1aa); 0x7b51e4 and 0x7b51e8 are float constants of their own (core/floatconsts: their own '
                      'initialisers set them)'),
    0x784e30: (0x210, 'balance file (hoardings A), to the level\'s file'),
    0x785040: (0x7e8, 'balance file (the level: its challenge indices reach +0x7e4), to the online file'),
    **{0xf869e0 + 0x24 * i: (0x24, 'a park-map colour of 23 (Init_5f0700): four colour bytes and 0x10 zeroed (Init_5ec210 on +4, '
                                   'or Fn_5f0970), nine dwords copied (texsprite)') for i in range(23)},
}
STATIC_SOURCES = ('staticinit.cpp', 'staticinitmanual.cpp')
STATIC_TABLE = re.compile(r'^\s*\{0x[0-9a-f]+, [^,{}]+, 0x([0-9a-f]+), &\w+\},', re.M)   # (kMain: init, text, ctor, exit)
STATIC_OBJECTS = re.compile(r'\bkObjects\[\] = \{([^}]*)\};')                            # (kMain's objects, row by row)
STATIC_CALL = re.compile(r'\.Th\(0x([0-9a-f]+), 0x([0-9a-f]+)u')   # (an exit thunk's too: g_calls.Th(dtor, object))
STATIC_REG = re.compile(r'\b(e[a-d]x|e[sd]i|ebp) = 0x([0-9a-f]+)u;|M<u32>\(sp - 0x[0-9a-f]+\) = 0x([0-9a-f]+)u;')


def statics(text):
    """{base: the functions called with it as this} of the static objects that staticinit's text constructs or touches by a literal
    base: kMain's object (its constructor), a Th call's this (an exit thunk's: its destructor), a register set to it (the Th calls
    with that register before it is set again), a pushed argument."""
    out = {}
    ctors = [int(m.group(1), 16) for m in STATIC_TABLE.finditer(text)]
    m = STATIC_OBJECTS.search(text)
    objects = [e.strip() for e in m.group(1).split(',') if e.strip()] if m else []
    assert len(objects) == len(ctors), 'kMain and kObjects: %d rows, %d objects' % (len(ctors), len(objects))
    for ctor, obj in zip(ctors, objects):
        lit = re.fullmatch(r'0x([0-9a-f]+)u?', obj)
        if lit:
            out.setdefault(int(lit.group(1), 16), set()).add(ctor)
    for m in STATIC_CALL.finditer(text):
        out.setdefault(int(m.group(2), 16), set()).add(int(m.group(1), 16))
    for m in STATIC_REG.finditer(text):
        base = out.setdefault(int(m.group(2) or m.group(3), 16), set())
        if m.group(1):
            end = re.compile(r'\b%s = |^\}' % m.group(1), re.M).search(text, m.end())
            body = text[m.end():end.start() if end else len(text)]
            base |= {int(f, 16) for f in re.findall(r'\.Th\(0x([0-9a-f]+), %s\b' % m.group(1), body)}
    for s, (n, _) in STATIC.items():   # (a base inside a STATIC object is a member: a colour's Init_5ec210 on +4)
        for b in [b for b in out if s < b < s + n]:
            out.setdefault(s, set()).update(out.pop(b))
    return {b: fs for b, fs in out.items() if DATA_LO <= b < DATA_HI}


def static_extents(bases):
    """{base: (end, how)} of the static objects (statics()): STATIC's size, else the largest of STATIC_FUNCS' sizes when every function
    called on it has one, else up to the next static base. tools/gen_static_init.py declares the constructed ones so."""
    order = sorted(bases)
    out = {}
    for i, b in enumerate(order):
        fs = bases[b]
        if b in STATIC:
            out[b] = (b + STATIC[b][0], 'STATIC')
        elif fs and all(f in STATIC_FUNCS for f in fs):
            out[b] = (b + max(STATIC_FUNCS[f][0] for f in fs), 'STATIC_FUNCS')
        else:
            out[b] = (order[i + 1] if i + 1 < len(order) else DATA_HI, 'the next static base')
    return out


def static_errors(decls, text):
    """A converted object inside a static object that staticinit reaches by its literal base: the object's code writes the original
    address and the name reads its copy in tpw.exe (the hook DLL binds both to the original, so no bench sees it). The extent:
    static_extents(). The static objects staticinit constructs are GAME_DATA objects of that extent (core/staticobjects.h, generated):
    staticinit names them, so this checks only the ones left literal (their next static base: a literal one or a declared one), and the
    overlap check keeps the rest from declaring their members."""
    bases = statics(text)
    named = {d[2] for d in decls if os.path.basename(d[4]) == 'staticobjects.h'}
    starts = [d[2] for d in decls]
    errors = []
    for b, (end, _) in sorted(static_extents({**{n: set() for n in named}, **bases}).items()):
        fs = bases.get(b)
        if fs is None:
            continue
        for j in range(bisect.bisect_right(starts, b), bisect.bisect_left(starts, end)):
            errors.append('%s %#x is inside the static object at %#x (to %#x; staticinit: %s): convert the object as a whole, return '
                          'the member to a literal, or settle the extent in STATIC / STATIC_FUNCS' % (
                              decls[j][1], decls[j][2], b, end, ', '.join('%#x' % f for f in sorted(fs)) or 'its address'))
    return errors


def header(decls):
    lines = ['// Generated by tools/gamedata.py: do not edit (.docs/09-globals.md).', '#include <cstdint>',
             '#include "%s"' % os.path.join(ROOT, 'src', 'game', 'core', 'gamedata.h').replace('\\', '/')]
    lines += ['#include "%s"' % p.replace('\\', '/') for p in sorted(set(d[4] for d in decls))]
    return lines


def write(path, lines):
    text = '\n'.join(lines) + '\n'
    if os.path.exists(path) and open(path, encoding='utf-8').read() == text:
        return   # unchanged: nothing recompiles
    with open(path, 'w', encoding='utf-8', newline='\n') as f:
        f.write(text)


def address(out):
    decls = declarations()
    errors = check(decls)
    if errors:
        sys.exit('gamedata.py: %d problems:\n  ' % len(errors) + '\n  '.join(errors[:40]))
    lines = header(decls)
    lines += ['GameData<%s> &%s = *reinterpret_cast<GameData<%s> *>(uintptr_t{%#010x});' % (t, name, t, a) for t, name, a, _, _ in decls]
    write(out, lines)


TEXT = {        # slot: why, for a dword of text whose bytes read as a pointer into a converted range or to its end (left as it is)
    0x74a418: 'a code page table\'s u16 pairs (0x2013, 0x7b), not a pointer into gSinTable',
    0x73d864: '"dex" of "PointBlockIndex", not a pointer to gString_786550\'s end',   # (round 78: the static objects)
    0x73dbe8: '"dex" of "...ossSectionIndex"',
    0x73eae8: '"dex" of "...ontCarTypeIndex"',
    0x745300: '"dex" of "TypeStringIndex"',
    0x747b10: '"dTex" of "Small_SharedTex", not a pointer into gString_786550',
    0x74c8e8: '"hTex" of "PathTex"',
    0x75af4c: '"dex" of "mPylonIndex"',
    0x7740c4: '"tbox" of "/outbox", not a pointer into gString_786f50',
    0x779684: '" fix" of "..., please fix", not a pointer into gWString_786958',
    0x77b350: '" fix" of "..., please fix"',
}
POINTERS = {}   # slot: why, for a pointer whose bytes read as text
NEXT = {        # slot: why, for a dword at a converted object's end that points at what follows it (left as it is), not an end pointer
    0x7061c4: 'the text "DDERR_UN..." at 0x77dd30, after gImaAdpcmState',
    0x7655d0: 'a name of RSSE_OpcodeNameTable: 0x786de4 (.bss, after gString_786dd0, a String of 0x14), a global of its own',
    0x779b18: 'a name table\'s entry: 0x786de4, the global after gString_786dd0',
    0x779b34: 'a name table\'s entry: 0x786de4, the global after gString_786dd0',
    0x751a38: 'its wide text at 0x751a3c, after the pointer itself',
    0x763750: 'gPieceTables\' last row\'s name, the text "ho_fob4..." after the tables',
    0x7655c8: 'RSSE_OpcodeNameTable\'s last name, the text after the table',
    0x779a84: 'gZlibVersion\'s text "1.1.3" at 0x7780f0',
    0x781704: 'the text "\\r\\n" at 0x773cdc',
    0x78259c: 'the text "a" at 0x766344',
    0x78170c: 'the text "runtime error" at 0x708dac, after CRT_text_empty_wide_708da8',
    0x781f88: 'the text "H:mm:ss" at 0x709750, after CRT_text_SNAN_709748',
    0x781f8c: 'the C library\'s __lc_time_curr: its __lc_time_c (the time names) at 0x781ee0, after gCrtfpexcept_781edc',
    0x782d24: 'the static object at 0xfb1edc (staticinit gives it the vtable 0x70a114), after gLocalefacet_fb1ed8',
}
NEXT.update({s: 'the C library locale tables\' text at 0x711c30, after CRT_zero_711c28'
             for s in (0x711c48, 0x711cf8, 0x711d2c, 0x711d80, 0x712188, 0x7121bc, 0x71237c, 0x7129dc)})
NEXT.update({s: 'the .bss global at 0xfb3cc0, after CRT_piob' for s in (0x77e800, 0x77e808)})
NEXT.update({s: 'the small-block heap\'s list head at 0x780dc8 (rows of 0xc), after CRT_sbh_threshold'
             for s in range(0x780de0, 0x780e14, 0xc)})
NEXT.update({s: 'the .bss global at 0xfb1940, after CRT_Getlocaleinfo_buffer_fb1938' for s in range(0x781f9c, 0x781fc0, 4)})

def printable(b):
    return 0x20 <= b < 0x7f


def pointers(pe, decls, code_slots):
    """The exe's pointers that tpw.exe relocates at start-up, from every 4-aligned dword of a data section (not .text, cseg, .rsrc):
    (slot, value) where the value lies inside a converted range (the slot in the copy or in a converted object), or the slot is one of
    the copy's code slots (code_slots: tools/gen_loader_slots.py's, a hooked function, an adjustor thunk to one, Debug_Printf) inside a
    converted object. Also the problems the fixups cannot express: such a dword across an object's edge, an object over the import address
    table (the loader fills that in the copy), and a pointer that reads as the end of a text (three printable bytes and a zero, after a
    printable one: settle it in TEXT or POINTERS), and a dword at an object's end (outside every object), which the fixups leave as it
    is: right for what follows the object (settle it in NEXT), wrong for a loop's end pointer. Any other dword that holds the value of a pointer is taken for one."""
    inside = finder(decls)
    ends = {a + n: name for _, name, a, n, _ in decls if not inside(a + n)}
    found, errors = [], []
    d = struct.unpack_from('<I', pe.d, 0x3c)[0] + 24 + 96 + 12 * 8   # (IMAGE_DIRECTORY_ENTRY_IAT of the optional header)
    iat, iat_size = struct.unpack_from('<II', pe.d, d)
    for _, name, a, n, _ in decls:
        if iat_size and a < pe.base + iat + iat_size and pe.base + iat < a + n:
            errors.append('%s %#x+%#x overlaps the import address table: the loader fills it in the copy' % (name, a, n))
    for sname, va, _, ro, rs in pe.sections:
        if sname in ('.text', 'cseg', '.rsrc'):
            continue
        raw = pe.d[ro:ro + rs]
        for o in range(0, len(raw) - 3, 4):
            slot, v = va + o, struct.unpack_from('<I', raw, o)[0]
            if v in ends and slot not in NEXT and slot not in TEXT:
                errors.append('%#x holds %#x, the end of %s: an end pointer (the fixups cannot express it) or the next global\'s address '
                              '(settle it in NEXT)' % (slot, v, ends[v]))
            into = DATA_LO <= v < DATA_HI and inside(v)
            if not (into or slot in code_slots and (inside(slot) or inside(slot + 3))) or slot in TEXT:
                continue
            if inside(slot) != inside(slot + 3):
                errors.append('%#x holds %#x across the edge of %s' % (slot, v, inside(slot) or inside(slot + 3)))
            elif into and o and all(map(printable, raw[o - 1:o + 3])) and slot not in POINTERS:
                errors.append('%#x holds %#x (%s), which reads as text %r: settle it in TEXT or POINTERS' % (slot, v, into, bytes(raw[o - 1:o + 3])))
            else:
                found.append((slot, v))
    return found, errors


def self_check():
    """pointers() on a made-up exe: .text 0x401000 (not scanned), .rdata 0x700000 with the objects A 0x700010+0x10 and B 0x700020+6,
    .data 0x800000 with the IAT at 0x800008+8."""
    class Fake:
        base = 0x400000
        sections = [('.text', 0x401000, 0x10, 0x400, 0x10), ('.rdata', 0x700000, 0x30, 0x410, 0x30), ('.data', 0x800000, 0x10, 0x440, 0x10)]
    pe = Fake()
    pe.d = bytearray(0x450)
    struct.pack_into('<I', pe.d, 0x3c, 0x80)
    struct.pack_into('<II', pe.d, 0x80 + 24 + 96 + 96, 0x400008, 8)
    struct.pack_into('<4I', pe.d, 0x400, *[0x700010] * 4)
    # the copy: a code slot (the table's), B+4, B's end (an error, then settled in NEXT: kept), A+4; A: a code slot, B, the copy (kept), a code address that is no code
    # slot (kept); B: A, then a pointer across B's end; the copy: text "aaaa", then "bap" (0x706162, inside C)
    struct.pack_into('<12I', pe.d, 0x410, 0x401000, 0x700024, 0x700026, 0x700014, 0x401008, 0x700020, 0x700008, 0x401004, 0x700010,
                     0x700010, 0x61616161, 0x706162)
    struct.pack_into('<I', pe.d, 0x440, 0x700010)
    decls = [('u8[16]', 'A', 0x700010, 0x10, ''), ('u8[6]', 'B', 0x700020, 6, ''), ('u8[4]', 'C', 0x706160, 4, '')]
    assert 'the end of B' in pointers(pe, decls, {0x700000, 0x700010})[1][0]
    NEXT[0x700008] = 'self_check'
    found, errors = pointers(pe, decls, {0x700000, 0x700010})
    assert found == [(0x700004, 0x700024), (0x70000c, 0x700014), (0x700010, 0x401008), (0x700014, 0x700020), (0x700020, 0x700010),
                     (0x800000, 0x700010)], found
    assert len(errors) == 2 and 'across the edge of B' in errors[0] and 'reads as text' in errors[1], errors
    TEXT[0x70002c] = 'self_check'
    assert pointers(pe, decls, set())[1] == errors[:1]
    del TEXT[0x70002c]
    POINTERS[0x70002c] = 'self_check'
    assert pointers(pe, decls, set())[0][-2] == (0x70002c, 0x706162)
    del POINTERS[0x70002c]
    assert len(pointers(pe, decls + [('u32[2]', 'I', 0x800008, 8, '')], set())[1]) == 3   # + an object on the IAT
    del NEXT[0x700008]
    # staticinit: a String at 0x900000 (0x14), a register base 0x900100 (an unsized function: to the next base), a pushed 0x900200
    # (to the end), the register set again to 0x900400
    text = ('    {0x401000, 0x401010, 0x5f60b0, &Exit_401020},\n    {0x401030, 0x401010, 0x5f60b0, &Exit_401040},\n'
            'const u32 kObjects[] = {\n    0x900000, (u32)(uintptr_t)&gNamed,\n};\nvoid Exit_401020() { g_calls.Th(0x005f61e0, 0x900000u); }\nvoid F() {\n'
            '    ecx = 0x900100u; g_calls.Th(0x123456, ecx); ecx = 0x900400u; g_calls.Th(0x5f60b0, ecx);\n'
            '    M<u32>(sp - 0x4) = 0x900200u;\n}\n')
    assert statics(text) == {0x900000: {0x5f60b0, 0x5f61e0}, 0x900100: {0x123456}, 0x900200: set(), 0x900400: {0x5f60b0}}
    decls = [('u32', n, a, 4, '') for n, a in (('In', 0x900004), ('Out', 0x900014), ('R', 0x900180), ('P', 0x900200), ('Q', 0x900300))]
    assert [e.split()[0] for e in static_errors(decls, text)] == ['In', 'R', 'Q']
    STATIC[0x900200] = (0x100, 'self_check')
    assert [e.split()[0] for e in static_errors(decls, text)] == ['In', 'R']
    del STATIC[0x900200]
    # a literal base's reach: + (short of A, then into it), at<>, a local's at<> in its function (its + after the function: not seen)
    text = ('x = 0x900000 + 0x8; y = 0x900000 + 0x20;\nz = at<u32>(reinterpret_cast<u8 *>((uintptr_t)0x900000), 0x10);\n'
            'void G() {\n    u8 *p = reinterpret_cast<u8 *>((uintptr_t)0x900000);\n    w = at<u32>(p, 0x14);\n}\nv = p + 0x20;\n')
    assert [e.split(':')[1] for e in reach_errors('f', text, [('u32', 'A', 0x900010, 4, '')])] == ['1', '2', '5']


def initial(pe, a, n):
    """The n initial bytes at a (zero past the section's raw data)."""
    s = pe.section_of(a)
    raw = pe.d[s[3] + a - s[1]:s[3] + s[4]][:n] if a - s[1] < s[4] else b''
    return bytearray(raw) + bytearray(n - len(raw))


def obj(exe, out, fixups_h):
    """tpw.exe's objects and the start-up fixups (kGameDataFixups, src/loader/loader.cpp): a slot in an object is the object and the
    offset, one in the copy its address; the target is the converted object's own address and offset, or null for a code slot (the
    loader points it at the port). fixups_h: the same in original addresses, for tpw_tests (test_gamedata_fixups)."""
    from pe_read import PE
    pe = PE(exe)
    decls = declarations()
    errors = check(decls)
    from gen_loader_slots import hooked_addresses, slots
    found, problems = pointers(pe, decls, set(slots(pe, hooked_addresses())))
    errors += problems
    if errors:
        sys.exit('gamedata.py: %d problems:\n  ' % len(errors) + '\n  '.join(errors[:40]))
    lines = header(decls) + ['namespace {']
    defs = []
    for t, name, a, n, _ in decls:
        data = initial(pe, a, n)
        init = '{}' if not any(data) else '{' + ','.join('%#x' % b for b in data) + '}'
        lines.append('alignas(16) unsigned char %s_[%#x] = %s;' % (name, n, init))
        defs.append('GameData<%s> &%s = reinterpret_cast<GameData<%s> &>(%s_);' % (t, name, t, name))
    lines += ['}  // namespace'] + defs
    ranges = ', '.join('{%#010x, %#x}' % (a, n) for _, _, a, n, _ in decls) or '{0, 0}'
    lines.append('extern "C" const uint32_t kGameDataRanges[][2] = {%s};' % ranges)
    lines.append('extern "C" const uint32_t kGameDataRangeCount = %d;' % len(decls))
    inside, start = finder(decls), {d[1]: d[2] for d in decls}

    def at(address):   # (an address inside a converted range: its object's name and offset)
        name = inside(address)
        return (name, address - start[name]) if name else None
    rows = []
    for slot, v in found:
        home, to = at(slot), at(v) if DATA_LO <= v < DATA_HI else None
        rows.append('{%s, %s}' % ('%s_, %#x' % home if home else 'nullptr, %#010x' % slot, '%s_ + %#x' % to if to else 'nullptr'))
    lines.append('extern "C" const GameDataFixup kGameDataFixups[] = {%s};' % (', '.join(rows) or '{nullptr, 0, nullptr}'))
    lines.append('extern "C" const uint32_t kGameDataFixupCount = %d;' % len(rows))
    write(out, lines)
    write(fixups_h, ['// Generated by tools/gamedata.py: do not edit. tpw.exe\'s globals and start-up fixups in original addresses (tpw_tests).',
                     '#pragma once', '#include <cstdint>',
                     'const std::uint32_t kFixupRanges[][2] = {%s};' % ranges, 'const unsigned kFixupRangeCount = %d;' % len(decls),
                     'const std::uint32_t kFixups[][2] = {%s};' % (', '.join('{%#010x, %#010x}' % f for f in found) or '{0, 0}'),
                     'const unsigned kFixupCount = %d;' % len(found),
                     'const std::uint32_t kFixupText[] = {%s};' % (', '.join('%#010x' % t for t in sorted(TEXT)) or '0'),
                     'const unsigned kFixupTextCount = %d;' % len(TEXT)])


def main():
    a = sys.argv[1:]
    if a[:1] == ['address'] and len(a) == 2:
        address(a[1])
    elif a[:1] == ['object'] and len(a) == 4:
        self_check()
        obj(a[1], a[2], a[3])
    elif a == ['check']:
        self_check()
        decls = declarations()
        errors = check(decls)
        print('%d globals, %d bytes; %d problems' % (len(decls), sum(d[3] for d in decls), len(errors)))
        for e in errors[:40]:
            print('  ' + e)
        sys.exit(1 if errors else 0)
    else:
        sys.exit(__doc__)


if __name__ == '__main__':
    main()
