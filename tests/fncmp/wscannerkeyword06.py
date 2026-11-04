# The wide text scanner's keyword handler with a default word (src/game/core/wscanner: 0x5e5540) against the original: as wscannerkeywords.py, its last call (the rest, 0x5e7200) a recorder as it throws (_Xran) when the word failed: past the text's end; the default text at 0x78a374 of 0-5 characters.
# usage: wscannerkeyword06.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import LOG, Bench, Call, _recorder, log_region, set_return  # noqa: E402
import wscanner  # noqa: E402
import wscannerkeywords  # noqa: E402

REST = 1
B = Bench([(0x405280, 0x405390), (0x4051d0, 0x405272), (0x479150, 0x4791b0), (0x47b750, 0x47bcf0), (0x4f23f0, 0x4f2450), (0x4f6080, 0x4f60a0),
           (0x67acd0, 0x67ad00), (0x4ea740, 0x4ea7e0), (0x47ae60, 0x47b060), (0x47b600, 0x47b750), (0x479230, 0x479256),
           (0x47cc10, 0x47cd00), (0x47b460, 0x47b5b0), (0x47d9d0, 0x47db00), (0x47d620, 0x47d780), (0x47b320, 0x47b460),
           (0x5e6fc0, 0x5e7093), (0x5b30b0, 0x5b3110), (0x5e70a0, 0x5e72ff), (0x5e7520, 0x5e76b9), (0x5e76c0, 0x5e7720),
           (0x45b380, 0x45b5f0), (0x45b680, 0x45b9b0), (0x4e6410, 0x4e6560), (0x4f5860, 0x4f5988), (0x4f5560, 0x4f5852), (0x4e51a0, 0x4e520e),
           (0x4f67b0, 0x4f67e1), (0x4f67f0, 0x4f6819), (0x4f7410, 0x4f7423), (0x4f34a0, 0x4f36c0), (0x4f36f0, 0x4f3711), (0x4f6ef0, 0x4f7170), (0x4f6e70, 0x4f6ee9), (0x4f0150, 0x4f0168), (0x5e5540, 0x5e57c0)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None, 0x5e7200: _recorder(REST, 2)})   # (_Lockit's constructor and destructor)


def case(r):
    regions, ignored = wscannerkeywords.scene(r, 'Next')
    if r.randrange(3) == 0:
        wscannerkeywords.next_fails(regions)
    log = log_region()
    set_return(log, REST, r.choice([1, 1, 1, 0, 0x100, r.getrandbits(32)]))
    regions[LOG] = log
    size = r.randrange(6)
    regions[0x78a374] = b''.join(struct.pack('<H', r.randrange(1, 0x10000)) for _ in range(size)) + bytes(2)
    return Call(0x5e5540, [wscanner.SCANNER], regions, conv='t', ret='b', ignore=ignored)


if __name__ == '__main__':
    B.main(case)
