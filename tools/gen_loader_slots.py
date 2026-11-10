"""Generates tpw.exe's code-slot table (codeslots.h, in the build folder): the dwords of tp.exe's data sections that hold the address of a
function with a port, which the loader (src/loader/loader.cpp) points at the port when it copies the exe in. CMake runs it at build time,
from the user's exe and src/hook/hooks.cpp, into the build folder; nothing of the exe goes into the repo.

A slot: a 4-aligned dword of a section other than .text, cseg (code: its tables are read as data, the rasteriser's switch compares them)
and .rsrc, whose value is in .text and is
- a hooked function (hooks.cpp's rows): vtables, handler and callback tables;
- an adjustor thunk (SUB ECX,[ECX-4] / SUB ECX,n / ADD ECX,n ; JMP hooked): the loader makes an equivalent of it with the port's address;
- a JMP thunk (JMP rel32 at a 16-aligned function start, at most three in a row, to a hooked function: the COM AddRef / Release thunks
  0x6acea0 / 0x6aceb0; a JMP inside a function is taken for text that happens to point there): the loader
  follows the jumps (Resolve), so the slot gets the port itself; an adjustor thunk's JMP may lead through them too;
- PROVIDED: the empty Debug_Printf (a RET, in many vtables), which the loader gives an empty function for.
Any other code address in the data stays the original's: a call through it traps (.docs/08-loader.md). The CRT's initialiser tables
are among those: StaticInit_Run reads them live by the original's addresses.

usage: gen_loader_slots.py EXE OUT         write the table (CMake)
       gen_loader_slots.py --stats [EXE]   the code addresses in the data by kind, and the by-address calls in src/game with no port
"""
import collections
import os
import re
import struct
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
from pe_read import PE  # noqa: E402

PROVIDED = {0x5da3c0}   # Debug_Printf: RET (src/loader/loader.cpp's kProvided)
CODE = ('.text', 'cseg', '.rsrc')   # not scanned
THUNKS = (b'\x2b\x49\xfc', b'\x83\xe9', b'\x83\xc1')   # SUB ECX,[ECX-4] ; SUB ECX,imm8 ; ADD ECX,imm8 (3 bytes each), then JMP rel32


def hooked_addresses():
    src = open(os.path.join(ROOT, 'src', 'hook', 'hooks.cpp')).read()
    return set(int(a, 16) for a in re.findall(r'^\s*\{"\w+",\s*0x([0-9a-fA-F]{8}),', src, re.M))


def thunk_target(pe, va):
    """The JMP target of an adjustor thunk at va, else None."""
    b = pe.bytes(va, 8)
    if b and len(b) == 8 and (b[:3] == THUNKS[0] or b[:2] in THUNKS[1:]) and b[3] == 0xe9:
        return va + 8 + struct.unpack_from('<i', b, 4)[0]
    return None


def resolve(pe, va, hooked):
    """The hooked function (or PROVIDED) that va is or reaches by JMP rel32s, as the loader's Resolve (four looks): else None."""
    for _ in range(4):
        if va in hooked or va in PROVIDED:
            return va
        b = pe.bytes(va, 5)
        if not b or len(b) < 5 or b[0] != 0xe9:
            return None
        va = (va + 5 + struct.unpack_from('<i', b, 1)[0]) & 0xffffffff
    return None


def scan(pe, hooked):
    """(section, slot, value, kind) for every 4-aligned dword in a data section whose value lies in .text."""
    text = next(s for s in pe.sections if s[0] == '.text')
    t0, t1 = text[1], text[1] + text[2]
    for name, va, size, ro, rs in pe.sections:
        if name in CODE:
            continue
        raw = pe.d[ro:ro + rs]
        for o in range(0, len(raw) - 3, 4):
            v = struct.unpack_from('<I', raw, o)[0]
            if not t0 <= v < t1:
                continue
            t = thunk_target(pe, v) if v not in hooked else None
            # (a JMP thunk starts a function, 16-aligned: a JMP inside one is a text's bytes that happen to point there, "UE" 0x450055)
            thunk = v not in hooked and v not in PROVIDED and (v % 16 == 0 and resolve(pe, v, hooked) or t is not None and resolve(pe, t, hooked))
            kind = ('hooked' if v in hooked else 'provided' if v in PROVIDED else 'thunk' if thunk else
                    'crt init' if 0x727000 <= va + o < 0x72e118 else 'C library' if 0x679000 <= v < 0x6b0000 else 'other')
            yield name, va + o, v, kind


def slots(pe, hooked):
    return [slot for _, slot, _, kind in scan(pe, hooked) if kind in ('hooked', 'provided', 'thunk')]


def write(exe, out):
    pe = PE(exe)
    table = slots(pe, hooked_addresses())
    assert table and table == sorted(table)
    lines = ['// GENERATED at build time by tools/gen_loader_slots.py from tp.exe and src/hook/hooks.cpp: do not edit.',
             '// The slots of the exe\'s data that hold a ported function\'s address (or an adjustor thunk to one, or the empty Debug_Printf).',
             '#pragma once', '', '#include <cstdint>', '', 'const std::uint32_t kCodeSlots[] = {']
    for i in range(0, len(table), 8):
        lines.append('    ' + ' '.join('0x%08x,' % a for a in table[i:i + 8]))
    lines += ['};', '']
    text = '\n'.join(lines)
    if not os.path.exists(out) or open(out).read() != text:   # (unchanged: no rebuild)
        with open(out, 'w', newline='\n') as f:
            f.write(text)


def stats(exe):
    pe = PE(exe)
    hooked = hooked_addresses()
    count, targets = collections.Counter(), collections.defaultdict(set)
    for name, _, v, kind in scan(pe, hooked):
        count[(name, kind)] += 1
        targets[(name, kind)].add(v)
    for key in sorted(count):
        print('%-9s %-10s %6d slots %6d targets' % (key + (count[key], len(targets[key]))))
    print('rewritten: %d slots' % len(slots(pe, hooked)))
    # The by-address calls in the ports with no port behind them: a function pointer type, then the address (Cd / Th / St / Sd, casts, envs).
    call = re.compile(r'(?:\b(?:Cd|Th|St|Sd)\(|\(__(?:cdecl|stdcall|fastcall|thiscall) ?\*[^;{}]*?\(\(?(?:uintptr_t\)\s*)?)(0x00[4-6][0-9a-f]{5})u?\b')
    text = next(s for s in pe.sections if s[0] == '.text')
    left = collections.defaultdict(set)
    for d, _, files in os.walk(os.path.join(ROOT, 'src', 'game')):
        for f in files:
            if f.endswith('.cpp') and f != 'staticinit.cpp':
                src = re.sub(r'//[^\n]*', '', open(os.path.join(d, f), errors='replace').read())
                for a in call.findall(src):
                    v = int(a, 16)
                    if text[1] <= v < 0x6fc800 and v not in hooked and v not in PROVIDED:
                        left[v].add(f)
    print('by-address calls in src/game with no port: %d addresses' % len(left))
    for v in sorted(left):
        print('  %08x %s' % (v, ' '.join(sorted(left[v])[:4])))


def self_check():
    class Fake:
        def __init__(self, data):
            self.data = data

        def bytes(self, va, n):
            return self.data[va:va + n]
    assert thunk_target(Fake(b'\x2b\x49\xfc\xe9\x10\x00\x00\x00'), 0) == 0x18
    assert thunk_target(Fake(b'\x83\xe9\x40\xe9\xf8\xff\xff\xff'), 0) == 0
    assert thunk_target(Fake(b'\x83\xc1\x0c\xe9\x00\x00\x00\x00'), 0) == 8
    assert thunk_target(Fake(b'\x83\xc0\x0c\xe9\x00\x00\x00\x00'), 0) is None   # ADD EAX: not a thunk
    assert thunk_target(Fake(b'\x2b\x49\xfc\xe8\x00\x00\x00\x00'), 0) is None   # a CALL after it
    jumps = Fake(b'\xe9\x00\x00\x00\x00' * 4 + b'\xc3')   # JMP, JMP, JMP, JMP, then the function at 0x14
    assert resolve(jumps, 0x5, {0x14}) == 0x14 and resolve(jumps, 0, {0x14}) is None   # three jumps followed, four not
    assert resolve(jumps, 0x14, set()) is None


if __name__ == '__main__':
    self_check()
    if sys.argv[1:2] == ['--stats']:
        stats(sys.argv[2] if len(sys.argv) > 2 else os.path.join(ROOT, '.data', 'nocd', 'tp.exe'))
    elif len(sys.argv) == 3:
        write(sys.argv[1], sys.argv[2])
    else:
        sys.exit(__doc__)
