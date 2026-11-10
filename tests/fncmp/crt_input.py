# The C library's _input and sscanf (src/game/core/crtinput) against the originals: over string files (flags 0x49), with _inc, _un_inc, _whiteout, _filbuf, _ungetc_lk,
# _isctype, _hextodec, mbtowc and the 64-bit helpers as the originals on each side; _fassign a cdecl recorder in its table, the locks and _getptd cdecl recorders,
# MultiByteToWideChar a COM recorder in its import slot.
# Bench limits: no empty format and no %n before the first character read (the original's character starts uninitialised); every %[ set is closed (a format
# cut short reads on, sic); a set locale ([0xfb1440]) only with MB_CUR_MAX 2 (with 1, mbtowc leaves a lead byte's word unwritten: stack garbage).
# usage: crt_input.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob_array, fill, import_table, log_region, recorder, set_return, strings  # noqa: E402

GETPTD, LOCK, UNLOCK, M2W, FASSIGN, INC, DEC = 2, 7, 8, 3, 9, 14, 15
B = Bench([(0x68a710, 0x68b389), (0x67f620, 0x67f662), (0x68b450, 0x68b489), (0x68b490, 0x68b4b1), (0x68b4c0, 0x68b4d8), (0x68b4e0, 0x68b528), (0x67a590, 0x67a5ba),
           (0x680430, 0x6804c5), (0x67c7f0, 0x67c824), (0x68f040, 0x68f05f), (0x687440, 0x68752f), (0x67cc10, 0x67cc93), (0x687690, 0x687701), (0x687710, 0x68780e),
           (0x686a00, 0x686a19)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x682460: _recorder(GETPTD, 0, cdecl=True)})
FILE, ARGS, PTD, TEXT, FORMAT = RAM, RAM + 0x40, RAM + 0x100, RAM + 0x200, RAM + 0x400
PCTYPE, OUT = RAM + 0x800, RAM + 0x1000
SLOTS = 0x18
OPS = [n for n in ('input', 'input', 'input', 'sscanf', 'sscanf', 'game', 'crafted', 'crafted')
       if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
CHARS = b'0123456789' * 3 + b'--++xXabcdefABCDEF' + b'  \t\n,,..eE]^z%\x81\x82y'
SETS = [b'[abc]', b'[^,]', b'[]a-c]', b'[a-]', b'[z-a]', b'[-a]', b'[^]x]', b'[0-9]', b'[^ \t]', b'[\x81\x82a]', b'[]]']
GAME = [(b'%d,%d,%d,%d', [b'1,2,3,4', b'10,-20,+30,0x40', b'7,8', b' 5 , 6,7,8', b'', b'a,b']), (b'%d,%d', [b'640,480', b'-1,2', b'3', b'']),
        (b'%ld%c', [b'12k', b'123', b'-7 ', b'x', b''])]
# Each a way the random formats seldom take: a set with ']', 0X, a negative 64-bit number, %% before the end, %n after it, wide text with lead bytes.
CRAFTED = [(b'%[]a-c]%s', b']a]bcd e'), (b'%[^]x]', b'ab]c'), (b'%x%i', b'0X1f 0Xff'), (b'%I64d %I64x', b'-12345678901 -ff'), (b'%%%d', b'%'), (b'%%%d', b'%5'),
           (b'%d%n', b'5'), (b'%d%hn%d', b'7'), (b'%ls', b'\x81\x82ab cd'), (b'%l[a-c\x81\x82]%S', b'ab\x81\x82c dd'), (b'%lc%3C', b'\x81\x82xyz')]


def table():
    words = [0] * 0x101   # (word -1 first: _input tests EOF as a character)
    for c in range(256):
        w = 0
        if c in b' \t\n\r\x0b\x0c':
            w |= 8
        if 0x30 <= c <= 0x39:
            w |= 4 | 0x80
        if c in b'abcdefABCDEF':
            w |= 0x80 | 0x100
        if 0x81 <= c <= 0x9f:
            w |= 0x8000
        words[c + 1] = w
    return struct.pack('<257H', *words)


def directive(r):
    """One conversion's text and whether it is %n."""
    text = b'%'
    if r.random() < 0.15:
        text += b'*'
    text += r.choice([b'', b'', b'', b'1', b'2', b'3', b'10', b'0', b'400'])
    text += r.choice([b'', b'', b'', b'h', b'l', b'L', b'I64', b'w', b'hh', b'F', b'N', b'I', b'lh'])
    kind = r.choice(['d', 'i', 'o', 'u', 'x', 'X', 'p', 'c', 'C', 's', 'S', 'n', 'e', 'E', 'f', 'g', 'G', 'set', 'set', '%', 'y', ' '])
    text += r.choice(SETS) if kind == 'set' else kind.encode()
    return text, kind == 'n'


def build_format(r):
    out = bytearray()
    read = False
    for _ in range(r.choice([1, 2, 3, 4, 6, 8])):
        kind = r.random()
        if kind < 0.6:
            text, n = directive(r)
            if n and not read:
                continue
            out += text
            read = True
        elif kind < 0.8:
            out += r.choice([b' ', b'\t', b'  '])
        else:
            out += r.choice([b',', b'x', b'\x81\x82', b'\x81y', b'.', b'-'])
            read = True
    if not read:
        out += b'%d'
    return bytes(out)


def build_text(r):
    if r.random() < 0.3:
        return bytes(r.choice(b'0123456789-+ ,') for _ in range(r.randrange(20)))
    return bytes(r.choice(CHARS) for _ in range(r.randrange(40)))


def state(r, log, regions):
    set_return(log, GETPTD, PTD)
    regions[PTD] = bytes(0x80)
    regions[PCTYPE] = table()
    regions[0x77ea90] = struct.pack('<I', PCTYPE + 2)
    most = r.choice([1, 1, 2])
    regions[0x77e7dc] = struct.pack('<i', most)
    regions[0x77e7e0] = r.choice([b'.', b'.', b','])
    regions[0xfb1440] = struct.pack('<5I', r.choice([0, 0x411]) if most == 2 else 0, 0, 0, 0, r.choice([932, 1252]))
    regions[0xfb3ca0] = struct.pack('<3I', 0x40, r.choice([0, 0, 1]), r.randrange(4))
    regions[0x77eca8] = struct.pack('<I', recorder(FASSIGN, 3, cdecl=True))
    strings(log, FASSIGN, 0b100)
    blob_array(log, M2W, 2, 3, 1)
    fill(log, M2W, 4, r.bytes(2), which=1)
    set_return(log, M2W, r.choice([1, 1, 2, 0]))
    regions[OUT] = r.bytes(SLOTS * 0x80)


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, IMPORT_TABLE: import_table(log, {'m2w': (M2W, 0x6fd174, 6, 0, 0b10100), 'inc': (INC, 0x6fd0c0, 1, 0, 0), 'dec': (DEC, 0x6fd12c, 1, 0, 0)})}
    state(r, log, regions)
    if name == 'game':
        form, texts = r.choice(GAME)
        text = r.choice(texts)
    elif name == 'crafted':
        form, text = r.choice(CRAFTED)
        if r.random() < 0.5:
            regions[0x77e7dc] = struct.pack('<i', 2)
            regions[0xfb1440] = struct.pack('<5I', 0x411, 0, 0, 0, 932)
    else:
        form, text = build_format(r), build_text(r)
    regions[FORMAT] = form + bytes(0x10)
    regions[TEXT] = text + bytes(0x40)
    slots = [OUT + 0x80 * i for i in range(SLOTS)]
    if name == 'input':
        regions[FILE] = struct.pack('<8I', TEXT, r.choice([len(text)] * 4 + [0, max(len(text) - 3, 0), len(text) + 2]), TEXT, 0x49, 0, 0, 0, 0)
        regions[ARGS] = struct.pack('<%dI' % SLOTS, *slots)
        return Call(0x68a710, [FILE, FORMAT, ARGS], regions, conv='c', ret='a')
    return Call(0x67f620, [TEXT, FORMAT, *slots], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
