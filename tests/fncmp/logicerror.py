# std::logic_error and std::out_of_range's methods and the vector's range error (src/game/core/iosbase: 0x632a40-0x632ee5) against the originals: the source's string empty, shared at each count or
# not shareable, the real heap; _CxxThrowException is a recorder that returns, the exception compared without its allocator byte (stack garbage in the originals), and the code after it runs.
# usage: logicerror.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _free, _recorder, blob, heap, log_region, put  # noqa: E402

THROW = 14
ADDRESS = {'Xran': 0x632a40, 'What': 0x632b70, 'CopyL': 0x632b80, 'RaiseL': 0x632ce0, 'Dtor': 0x632d00, 'RaiseR': 0x632d50, 'Deleting': 0x632d70, 'CopyR': 0x632d90}
OPS = [n for n in ADDRESS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
B = Bench([(0x632a40, 0x632b62), (0x632b70, 0x632b7d), (0x632b80, 0x632cd7), (0x632ce0, 0x632d00), (0x632d00, 0x632d4b), (0x632d50, 0x632d70), (0x632d70, 0x632d8e), (0x632d90, 0x632ee5), (0x67d250, 0x67d340), (0x45b340, 0x45b9a0)],
          hooks={0x67ad90: _free, 0x67d3a0: _recorder(THROW, 2, com=True)})
OBJ, FROM, TEXT, STRING = RAM, RAM + 0x40, RAM + 0x80, RAM + 0x100


def source(r, regions):
    """An exception with a text and a string."""
    data = bytearray(r.bytes(0x1c))
    put(data, 4, 'II', TEXT, r.choice([0, 1, r.getrandbits(32)]))
    regions[TEXT] = b'y' * r.choice([0, 1, 7]) + b'\0'
    n = r.choice([0, 1, 2, 7, 30])
    k = r.random()
    if k < 0.2:
        put(data, 0x10, 'III', 0, 0, r.choice([0, 0x1f]))
    else:
        put(data, 0x10, 'III', STRING + 1, n, n + r.choice([0, 1, 0x20]))
    regions[STRING] = bytes([r.choice([0, 1, 2, 0xfd, 0xfe, 0xff])]) + b's' * n + b'\0' + r.bytes(4)
    regions[FROM] = data


def case(r):
    op = r.deal(OPS)
    log = log_region()
    blob(log, THROW, 0, 0x77)   # (vtable, text, owned, the string; not its allocator byte)
    regions = {LOG: log, HEAP: heap(0x400, fail=1 if r.random() < 0.1 else 0), OBJ: r.bytes(0x1c)}
    address = ADDRESS[op]
    if op == 'Xran':
        regions[0x779a68] = b't' * r.choice([0, 1, 3, 4, 7, 31]) + b'\0'
        return Call(address, [], regions, conv='c', ret='v')
    source(r, regions)
    if op in ('CopyL', 'CopyR'):
        return Call(address, [FROM if r.random() < 0.1 else OBJ, FROM], regions, conv='t', ret='a')
    if op == 'Deleting':
        return Call(address, [FROM, r.choice([0, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')
    return Call(address, [FROM], regions, conv='t', ret='a' if op == 'What' else 'v')


if __name__ == '__main__':
    B.main(case)
