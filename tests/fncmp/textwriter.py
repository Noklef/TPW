# The fixed-buffer text writer (src/game/files/textwriter) against the originals: writers over buffers of capacity 0-0x100 (holding text, its length at,
# under or over what is there, the overflow flag either way), assigned and appended chars (NUL among them), C strings (null, empty, longer than the buffer),
# Strings (their length not always their text's) and _vsnprintf formats (integers, chars, strings; too long for the buffer: -1, unterminated). The writer's
# vtable is the game's (0x703e08; the driver points its slots at the replacements), so the methods size through their own SetLength as in the game; strncpy,
# strncat and _vsnprintf are the CRT's (driver.cpp's libc env has MSVC6 stand-ins).
# usage: textwriter.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Append,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, heap  # noqa: E402

B = Bench([(0x636b00, 0x636e40), (0x5f7600, 0x5f7900), (0x67a400, 0x690000), (0x405000, 0x406000)])
THIS, BUFFER, TEXT = RAM, RAM + 0x100, RAM + 0x300   # the writer, its buffer (0x100 bytes; one before it is the writer's region), texts
VTABLE = 0x703e08
STRING_VTABLE = 0x703090
WORDS = [b'', b'a', b'ab', b'abc', b'hello', b'path\\to\\file.wad', b'x' * 31, b'y' * 32, b'z' * 0x60, b'\xe9t\xe9', b'w' * 0x120]
FORMATS = [(b'abc', ''), (b'%d', 'i'), (b'%s-%d', 'si'), (b'%x%x%x', 'iii'), (b'', ''), (b'%5d|%-4s|', 'is'), (b'%%', ''), (b'%c%c', 'cc'),
           (b'%s', 's'), (b'%08x %u', 'ii'), (b'%s%s%s', 'sss'), (b'%.3s', 's'), (b'%i,%d', 'ii')]
# name: (address, stack arguments, conv, returns)
OPS = {
    'SetLength': (0x636b80, 'n', 't', 'v'), 'DeletingDtor': (0x636b60, 'f', 't', 'a'), 'Dtor': (0x636c70, '', 't', 'v'),
    'Ctor': (0x636bc0, 'Bkf', 't', 'a'), 'AssignChar': (0x636c80, 'c', 't', 'a'), 'AssignStr': (0x636cc0, 's', 't', 'a'),
    'Assign': (0x636d10, 'S', 't', 'a'), 'AppendStr': (0x636d40, 's', 't', 'a'), 'AppendChar': (0x636d80, 'c', 't', 'a'),
    'Append': (0x636dc0, 'S', 't', 'a'), 'Format': (0x636e00, 'F', 'c', 'v'),
}


class Texts:
    """TEXT's region: NUL-terminated texts (and Strings) laid end to end."""
    def __init__(self, r):
        self.r, self.data = r, bytearray()

    def put(self, b):
        at = TEXT + len(self.data)
        self.data += b
        self.data += bytes(-len(self.data) % 4)
        return at

    def text(self, s=None):
        return self.put((self.r.choice(WORDS) if s is None else s) + b'\0')

    def string(self):
        s = self.r.choice(WORDS)
        length = len(s) if self.r.random() < 0.8 else self.r.choice([0, 1, max(len(s) - 1, 0), len(s) + 2])
        data = self.text(s + b'\0\0\0')
        return self.put(struct.pack('<5I', STRING_VTABLE, length, data, len(s) + 1, 0))


def case(r):
    op = r.choice(PICK)
    address, args, conv, ret = OPS[op]
    capacity = r.choice([0, 1, 2, 5, 0x10, 0x20, 0x40, 0x100])
    text = r.choice(WORDS)[:max(capacity - 1, 0)] if r.random() < 0.9 else b''
    buffer = bytearray(r.bytes(0x100))
    buffer[:len(text) + 1] = text + b'\0'
    if capacity == 0 or r.random() < 0.1:
        buffer[0x80] = 0   # (a text for Ctor's strlen to stop in)
    length = len(text) if r.random() < 0.8 else r.choice([0, capacity - 1 if capacity else 0, capacity, len(text) + 1]) & 0xffffffff
    this = bytearray(r.bytes(0x100))
    struct.pack_into('<5I', this, 0, VTABLE, length, BUFFER, capacity, r.choice([0, 0, 1, 0x100]))
    t = Texts(r)
    stack = []
    for x in args:
        if x == 'n':
            stack.append(r.choice([0, 1, length, length + 1, capacity - 1 if capacity else 0, capacity, capacity + 1, 0x7fffffff, 0x80000000, 0xffffffff]))
        elif x == 'f':
            stack.append(r.choice([0, 1, 2, 3]))
        elif x == 'B':
            stack.append(BUFFER)
        elif x == 'k':
            stack.append(r.choice([0, 1, 2, 5, 0x10, 0x81, 0x100]))
        elif x == 'c':
            stack.append(t.text(bytes([r.choice([0, 0x41, 0x5c, 0xe9, r.randrange(1, 256)])])))
        elif x == 's':
            stack.append(t.text() if r.random() < 0.9 else 0)
        elif x == 'S':
            stack.append(t.string())
        elif x == 'F':
            if r.random() < 0.08:
                stack.append(0)
                continue
            fmt, kinds = r.choice(FORMATS)
            stack.append(t.text(fmt))
            for k in kinds:
                stack.append(t.text() if k == 's' else r.choice([0, 7, 0x41, 0xe9]) if k == 'c' else r.choice([0, 1, 42, 0xffffffff, 0x80000000, r.getrandbits(32)]))
    regions = {HEAP: heap(0x1000), THIS: this, BUFFER: buffer, TEXT: t.data or bytearray(4)}
    return Call(address, [THIS] + stack, regions, conv=conv, ret=ret)


PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
