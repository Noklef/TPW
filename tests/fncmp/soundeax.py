# The EAX 2.0 wrappers (src/game/core/soundeax) against the originals: a buffer at BUF whose +4 is an IKsPropertySet at SET (a COM
# recorder vtable; Get answers 0 or a failure now and then, else its hash); the stack block (data and returned size) is logged as 0. The
# listener's set: an object at OBJ whose slot 4 (a recorder) fills its out with a buffer at BUFFER (or null); the buffer's QueryInterface
# (a COM recorder) filling its out with SET and answering 0 or a failure; Set's data logged as a hash of its 13 dwords.
# usage: soundeax.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, blob, fill, log_region, recorder, set_return, strings, vtable  # noqa: E402

CALLS = {'query': 0x6c4760, 'listener': 0x6d6cf0}
B = Bench([(0x6c4760, 0x6c47a3), (0x6d6cf0, 0x6d6db0)])
BUF, OUT, SET, SVT = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300
OBJ, OVT, BUFFER, BVT, PROPS = RAM + 0x400, RAM + 0x500, RAM + 0x600, RAM + 0x700, RAM + 0x800
SLOT_K, QUERY_K, RELEASE_K, SET_K = 20, 21, 22, 23
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def case(r):
    name = r.deal(OPS)
    log = log_region()
    if name == 'listener':
        return listener(r, log)
    strings(log, 3, 0, out=1 << 5 | 1 << 7)   # Get's data and returned-size outs: one stack block
    if r.random() < 0.5:
        set_return(log, 3, r.choice([0, 0, 1, 0x80004005, 0x80070057, 0x80000000, 0x10000]))   # (failures with a low word of 0 too)
    regions = {BUF: r.bytes(4) + struct.pack('<I', SET) + r.bytes(8), OUT: r.bytes(4), SET: struct.pack('<I', SVT) + r.bytes(0xc),
               SVT: vtable({}, com={3: 8}), LOG: log}
    return Call(CALLS[name], [BUF, OUT, r.getrandbits(32)], regions, conv='t', ret='a')


def table(slots):
    """A vtable of 8 slots whose slots {slot: (k, arguments, com)} call recorder k (the others recorders 56-63, never called)."""
    return b''.join(struct.pack('<I', recorder(*slots[i]) if i in slots else recorder(56 + i, 0)) for i in range(8))


def listener(r, log):
    fill(log, SLOT_K, 1, struct.pack('<I', r.choice([BUFFER, BUFFER, BUFFER, 0])))
    strings(log, SLOT_K, 0, out=1 << 1)
    fill(log, QUERY_K, 2, struct.pack('<I', SET), which=1)
    strings(log, QUERY_K, 0, out=1 << 2)
    set_return(log, QUERY_K, r.choice([0, 0, 0, 1, 0x80004002, 0x80000000]))
    blob(log, SET_K, 5, 0x1fff)
    set_return(log, SET_K, r.getrandbits(32))
    set_return(log, RELEASE_K, r.getrandbits(32))
    regions = {OBJ: struct.pack('<I', OVT) + r.bytes(0xc), OVT: table({4: (SLOT_K, 2)}), BUFFER: struct.pack('<I', BVT) + r.bytes(0xc),
               BVT: table({0: (QUERY_K, 3, True)}), SET: struct.pack('<I', SVT) + r.bytes(0xc),
               SVT: table({2: (RELEASE_K, 1, True), 4: (SET_K, 7, True)}), PROPS: r.bytes(0x30), LOG: log}
    return Call(CALLS['listener'], [OBJ, r.getrandbits(32), PROPS], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
