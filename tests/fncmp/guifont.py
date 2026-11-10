# The GUI library's font (src/game/ui/guifont: 0x658334) against the originals. TextWidth: a font with glyphs (0x200 of 8 bytes, the
# widths any byte) or none, the spacing small or any; a text of 0-12 characters below 0x200 (or none); from / to about the length,
# negative or any (their upper halves random).
# Init (0x658243): a font of random bytes, the count 0 to 0x200 (upper half random), the allocation failing now and then (the heap),
# the bytes and words any (upper bits random).
# CharWidth (0x6583fe): TextWidth's font, its count +4 about 0x200 or any, the character about the count or any (upper half random); now
# and then characters from 0x8000 under a count past them.
# SetGlyph (0x6582ec): CharWidth's font (glyphs or none), the index about the count, -1 / -5 (written before the glyphs: room there) or
# any (upper half random), the two bytes any (upper bits random).
# Spacing (0x657da0): the two words any (upper halves random). Draw (0x657e34) and Clipped (0x657ee0): TextWidth's fonts and texts
# (Draw's never null), the pen x / y any (upper halves random), each drawn character the folded stub 0x416e10 (Save_FieldName: 0, so
# the pen goes to 0, 0). Clipped's clip at CLIP: its slot 0 a thiscall recorder (x, y) answering from ANS in turn (0 / 1, or a high
# word alone: AX is tested); flag 0 or any. Where its first loop shows no character the original returns the pen from an unset stack
# slot (sic): those cases compare AX alone. Now and then the glyphs 0x40000 past GLYPHS with characters from 0x8000 (negative: read
# before the pointer, in the block; the first test's y then carries ECX's sign-extended upper half).
# usage: guifont.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=TextWidth,Init,CharWidth,SetGlyph,Spacing,Draw,Clipped)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, answers, heap, log_region, put, vtable  # noqa: E402

CALLS = {'TextWidth': 0x658334, 'Init': 0x658243, 'CharWidth': 0x6583fe, 'SetGlyph': 0x6582ec, 'Spacing': 0x657da0, 'Draw': 0x657e34,
         'Clipped': 0x657ee0}
B = Bench([(0x658334, 0x6583fe), (0x67acd0, 0x67acee), (0x658243, 0x6582ec), (0x6583fe, 0x658458), (0x6582ec, 0x658334), (0x657da0, 0x657ddd),
           (0x657e34, 0x657ede), (0x657ee0, 0x658243), (0x416e10, 0x416e15)])
FONT, TEXT, GLYPHS, CLIP, VT, ANS = RAM, RAM + 0x100, RAM + 0x1000, RAM + 0x3000, RAM + 0x3100, RAM + 0x3200


def shows(chars, count, ans):
    """Whether Clipped's first loop shows a character (else its pen's x is an unset stack slot): the clip's answers in turn."""
    j = 2
    for ch in chars:
        if ch - (ch >> 15 << 16) >= count:
            continue
        j += 1
        if ans[j - 1] & 0xffff:
            j += 1
            if ans[j - 1] & 0xffff:
                return True
    return False


def draw(r, op, font):
    count = r.choice([0, 1, 0x100, 0x200, 0x200, 0x200, r.randrange(0x201)])
    put(font, 4, 'H', count)
    put(font, 0xa, 'H', r.choice([0, 8, 16, r.getrandbits(16)]))
    negative = op == 'Clipped' and r.random() < 0.1
    if op == 'Clipped':   # (no null test there)
        put(font, 0xc, 'I', GLYPHS + 0x40000 if negative else GLYPHS)
    n = r.choice([0, 1, 2, 3, r.randrange(13)])
    chars = [0x8000 + r.randrange(0x200) if negative else r.choice([0x20, 0x41, 0x61, r.randrange(1, 0x200), count, r.randrange(1, 0x200)])
             for _ in range(n)]
    regions = {FONT: font, TEXT: struct.pack('<%dH' % (n + 1), *(chars + [0])) + r.bytes(4), GLYPHS: r.bytes(0x1000)}
    xy = [r.choice([0, 5, 100, -3 & 0xffff, r.getrandbits(16)]) | r.getrandbits(16) << 16 for _ in range(2)]
    if op == 'Draw':
        return Call(CALLS[op], [FONT] + xy + [TEXT, r.getrandbits(32)], regions, conv='t', ret='a')
    log = log_region()
    ans = [r.choice([0, 1, 1, 1, 0x10000, r.getrandbits(32)]) for _ in range(64)]
    answers(log, 0, ANS)
    regions.update({LOG: log, CLIP: struct.pack('<I', VT) + r.bytes(0xc), VT: vtable({0: 2}), ANS: struct.pack('<64I', *ans)})
    flag = r.choice([0, 1, 1, r.getrandbits(32)])
    text = TEXT if r.random() < 0.95 else 0
    defined = not flag or not text or not (ans[0] & 0xffff and ans[1] & 0xffff) or shows(chars, count, ans)
    return Call(CALLS[op], [FONT] + xy + [text, CLIP, flag], regions, conv='t', ret='a' if defined else 'w')


def index(r, n):
    return r.choice([0, 1, n - 1, n, n + 1, -1, r.randrange(-2, n + 3), 0x7fff, -0x8000, r.getrandbits(16)]) & 0xffff | r.getrandbits(16) << 16


def case(r):
    op = r.deal(OPS)
    if op == 'Init':
        count = r.choice([0, 1, 2, 3, 7, 0x80, 0x200, r.randrange(0x200)]) | r.choice([0, r.getrandbits(16)]) << 16
        regions = {FONT: r.bytes(0x10), HEAP: heap(0x2000, r.choice([0] * 6 + [1]))}
        return Call(CALLS[op], [FONT, count] + [r.getrandbits(32) for _ in range(4)], regions, conv='t', ret='a')
    n = r.choice([0, 1, 2, r.randrange(13)])
    font = bytearray(r.bytes(0x10))
    put(font, 0xc, 'I', GLYPHS if r.random() < 0.9 else 0)
    put(font, 6, 'H', r.choice([0, 1, 2, r.getrandbits(16)]))
    if op == 'Spacing':
        return Call(CALLS[op], [FONT, r.getrandbits(32), r.getrandbits(32)], {FONT: font}, conv='t', ret='v')
    if op in ('Draw', 'Clipped'):
        return draw(r, op, font)
    if op == 'SetGlyph':
        count = r.choice([0, 1, 0x100, 0x200, 0x200, r.randrange(0x201)])
        put(font, 4, 'H', count)
        c = r.choice([0, 1, count - 1, count, count + 1, r.randrange(0x200), -1, -5, r.getrandbits(16)]) & 0xffff | r.getrandbits(16) << 16
        return Call(CALLS[op], [FONT, c, r.getrandbits(32), r.getrandbits(32)], {FONT: font, GLYPHS - 0x40: r.bytes(0x1040)}, conv='t', ret='v')
    if op == 'CharWidth':
        count = r.choice([0, 1, 0x100, 0x200, 0x200, r.randrange(0x201)])
        put(font, 4, 'H', count)
        c = r.choice([0, 1, count - 1, count, count + 1, r.randrange(0x200), r.getrandbits(16)]) & 0xffff | r.getrandbits(16) << 16
        if r.random() < 0.15:   # (characters from 0x8000 under a count past them: the glyphs 0x40000 before the block, so that theirs are in it)
            put(font, 0xc, 'I', GLYPHS - 0x40000)
            put(font, 4, 'H', r.choice([0xffff, 0x8200]))
            c = 0x8000 + r.randrange(0x200) | r.getrandbits(16) << 16
        return Call(CALLS[op], [FONT, c], {FONT: font, GLYPHS: r.bytes(0x1000)}, conv='t', ret='w')
    text = struct.pack('<%dH' % (n + 1), *([r.choice([0x20, 0x41, 0x61, r.randrange(1, 0x200)]) for _ in range(n)] + [0]))
    regions = {FONT: font, TEXT: text + r.bytes(4), GLYPHS: r.bytes(0x1000)}
    return Call(CALLS[op], [FONT, TEXT if r.random() < 0.95 else 0, index(r, n), index(r, n)], regions, conv='t', ret='w')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
