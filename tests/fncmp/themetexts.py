# A theme's two texts shown (src/game/core/namedobjects: 0x409b10) against the original: the name one of the four themes (case varied, cut short or not) or another; the definitions' name and suffix recorders fill the stack buffers; the labels get them. No match: the buffers are stack garbage, logged as 0.
# usage: themetexts.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import HEAP, LOG, LOG_BYTES, RAM, Bench, Call, _recorder, blob, blob_when, fill, heap, log_region, strings, vtable  # noqa: E402

NAME_TEXT, SUFFIX_TEXT = 1, 2
B = Bench([(0x409b10, 0x409ccc), (0x409cd0, 0x409cdd), (0x424d10, 0x424d2e), (0x424e10, 0x424e6e), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000), (0x5f5660, 0x5f5690), (0x5da3c0, 0x5da3c1)],
          hooks={0x413020: _recorder(NAME_TEXT, 3), 0x4130b0: _recorder(SUFFIX_TEXT, 3)})
THEMES = [(0x786478, b'fantasy'), (0x786808, b'halloween'), (0x786970, b'jungle'), (0x7869e8, b'space')]
FIRST, SECOND, NAME, THEMETEXT = RAM + 0x1000, RAM + 0x1100, RAM + 0x1200, RAM + 0x1400


def wide(s):
    return b''.join(bytes([c, 0]) for c in s) + bytes(2)


def case(r):
    log = log_region()
    # (the code a WORD over a dword whose upper half stays garbage; the out buffer's address differs between the sides)
    strings(log, NAME_TEXT, 0, 2)
    strings(log, SUFFIX_TEXT, 0, 2)
    struct.pack_into('<I', log, LOG_BYTES + 4 * NAME_TEXT, 1 << 16)
    struct.pack_into('<I', log, LOG_BYTES + 4 * SUFFIX_TEXT, 1 << 16)
    name_text, suffix_text = bytes(r.randrange(1, 0x100) for _ in range(0x40)), bytes(r.randrange(1, 0x100) for _ in range(0x40))
    fill(log, NAME_TEXT, 1, name_text, which=0)
    fill(log, SUFFIX_TEXT, 1, suffix_text, which=1)
    # (the label's slot: logged by what the buffer holds, and 0 when it holds neither text, the buffers never written)
    blob(log, 5, 0, 0xffff, only_when=True)
    blob_when(log, 5, 0, struct.unpack('<I', name_text[:4])[0], 0xffff, entry=0, deref=True)
    blob_when(log, 5, 0, struct.unpack('<I', suffix_text[:4])[0], 0xffff, entry=1, deref=True)
    regions = {LOG: log, HEAP: heap(fail=r.choice([0] * 30 + [1, 2])), FIRST: struct.pack('<I', FIRST + 0x40) + bytes(0x3c) + vtable({5: 1}), SECOND: struct.pack('<I', SECOND + 0x40) + bytes(0x3c) + vtable({5: 1})}
    texts = bytearray(0x100)
    themes = [w for _, w in THEMES]
    if r.random() < 0.2:   # (two themes of one name: the first that matches is the only one)
        i, j = r.sample(range(4), 2)
        themes[i] = themes[j]
    for i, (at, _) in enumerate(THEMES):
        word = themes[i]
        regions[at] = struct.pack('<5I', 0x7030c8, len(word), THEMETEXT + 0x40 * i, len(word) + 1, 8)
        texts[0x40 * i:0x40 * i + 2 * len(word) + 2] = wide(word)
    regions[THEMETEXT] = bytes(texts)
    word = bytes(c ^ 0x20 if r.random() < 0.3 else c for c in r.choice(themes * 3 + [b'', b'spac', b'jungles', b'zoo']))
    if r.random() < 0.1:
        word = word[:r.randrange(len(word) + 1)]
    regions[NAME] = struct.pack('<5I', 0x7030c8, len(word), NAME + 0x40, 0, 8) + bytes(0x2c) + wide(word) + bytes(0x10)
    return Call(0x409b10, [r.getrandbits(32), NAME, FIRST, SECOND], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
