# The narrow streams' virtual base destructors and ctype<char>'s case calls (src/game/core/nstream) against the originals. NIos_Dtor, the file streams' ios destructors and the C library's case calls are recorders; each has a bench of its own.
# usage: nstreamdtors.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=lower,upper)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, answers, heap, log_fold, log_region, put, set_return  # noqa: E402

IOS, IFS, OFS, LOWER, UPPER = range(5)
B = Bench([(0x5cf740, 0x5cf76f), (0x5cf770, 0x5cf78e), (0x5cf790, 0x5cf7bf), (0x5cf840, 0x5cf86d), (0x5cf870, 0x5cf885), (0x5cf8b0, 0x5cf8dd), (0x5cf8e0, 0x5cf8f5), (0x5cb700, 0x5cb714), (0x5cba50, 0x5cba64),
           (0x5cfc90, 0x5cfca9), (0x5cfcb0, 0x5cfcde), (0x5cfce0, 0x5cfcf9), (0x5cfd00, 0x5cfd2e)],
          hooks={0x5cde40: _recorder(IOS, 0), 0x5ce920: _recorder(IFS, 0), 0x5cf630: _recorder(OFS, 0), 0x699a20: _recorder(LOWER, 2, cdecl=True), 0x69a2b0: _recorder(UPPER, 2, cdecl=True)})
SELF, VBTABLE, TEXT, TABLE = RAM + 0x100, RAM, RAM + 0x300, RAM + 0x400


def world(r):
    log = log_region()
    log_fold(log)
    # (the object far enough in that every ios offset stays inside; the vbtable's second dword, the ios's offset, varied)
    obj = bytearray(r.bytes(0x100))
    put(obj, 0, 'I', VBTABLE)
    regions = {LOG: log, HEAP: heap(0x100), SELF: obj, VBTABLE: put(bytearray(r.bytes(8)), 4, 'I', r.choice([0x10, 0x14, 0x28, 0x40, 0x60]))}
    return log, regions


FLAGS = [0, 1, 2, 3, 0x100, 0xffffffff]


def plain(address, conv='v'):
    def case(r):
        log, regions = world(r)
        return Call(address, [SELF], regions, conv='t', ret=conv)
    return case


def deleting(address, back):
    def case(r):
        log, regions = world(r)
        return Call(address, [SELF + back, r.choice(FLAGS)], regions, conv='t', ret='a')
    return case


def one(address, k):
    def case(r):
        log, regions = world(r)
        set_return(log, k, r.getrandbits(32))
        return Call(address, [SELF, r.choice([0, 0x41, 0xc1, 0x1ff, r.getrandbits(32)])], regions, conv='t', ret='a')
    return case


def many(address, k):
    def case(r):
        log, regions = world(r)
        regions[TEXT] = r.bytes(0x20)
        regions[TABLE] = struct.pack('<40I', *[r.getrandbits(32) for _ in range(40)])
        answers(log, k, TABLE)
        return Call(address, [SELF, TEXT, TEXT + r.choice([0, 1, 2, 5, 16])], regions, conv='t', ret='a')
    return case


CASES = {'osdel': deleting(0x5cf740, 4), 'iosdel': deleting(0x5cf770, 0), 'isdel': deleting(0x5cf790, 8), 'ifsdel': deleting(0x5cf840, 0x5c), 'ofsdel': deleting(0x5cf8b0, 0x58),
         'isvb': plain(0x5cf870), 'osvb': plain(0x5cf8e0), 'ifsvb': plain(0x5cb700), 'ofsvb': plain(0x5cba50),
         'lower': one(0x5cfc90, LOWER), 'upper': one(0x5cfce0, UPPER), 'lowers': many(0x5cfcb0, LOWER), 'uppers': many(0x5cfd00, UPPER)}
OPS = [n for n in CASES if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def case(r):
    return CASES[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)
