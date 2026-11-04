# The C library's locks (src/game/core/crtlock: _lock 0x682720, _unlock 0x6827a0, _lock_file 0x6827c0, _lock_file2 0x682800, _unlock_file 0x682830, _unlock_file2 0x682870) against the originals: the
# critical section calls COM recorders in their import slots, _nh_malloc and _amsg_exit cdecl recorders (this _amsg_exit returns), free the heap's hash. Lock 0x11 always exists: without it _lock calls itself
# for ever, in the game too. Entering it may fill the slot being made, as another thread would.
# usage: crt_lock.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _free, _recorder, fill, heap, import_table, log_region, set_return  # noqa: E402

INIT, ENTER, LEAVE, AMSG, NHMALLOC = 14, 15, 16, 26, 28
B = Bench([(0x682720, 0x682796), (0x6827a0, 0x6827b6), (0x6827c0, 0x6827f6), (0x682800, 0x682828), (0x682830, 0x682866), (0x682870, 0x682898), (0x67dcd0, 0x67dcf0)],
          hooks={0x67dcf0: _recorder(NHMALLOC, 2, cdecl=True), 0x67efc0: _recorder(AMSG, 1, cdecl=True), 0x67ad90: _free})
SECTIONS, FRESH = RAM, RAM + 0x1000
TABLE = 0x77ece0
OPS = [n for n in ('lock', 'unlock', 'lock_file', 'unlock_file', 'lock_file2', 'unlock_file2') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x40), IMPORT_TABLE: import_table(log, {'init': (INIT, 0x6fd144, 1, 0, 0), 'enter': (ENTER, 0x6fd140, 1, 0, 0), 'leave': (LEAVE, 0x6fd130, 1, 0, 0)})}
    set_return(log, LEAVE, r.getrandbits(32))
    regions[0xfb1548] = struct.pack('<I', r.randrange(2))
    set_return(log, NHMALLOC, r.choice([FRESH, FRESH, FRESH, 0]))
    which = r.choice([0, 2, 9, 0x13, 0x19, 0x1c, 0x1f, 0x2f])
    table = [r.choice([0, SECTIONS + 0x18 * i]) for i in range(0x30)]
    table[0x11] = SECTIONS + 0x18 * 0x11
    if name == 'lock' and not table[which] and r.random() < 0.3:   # (Lock 0x11's section, for this case, is the empty slot itself: entering it fills the slot.)
        table[0x11] = TABLE + 4 * which
        fill(log, ENTER, 0, struct.pack('<I', SECTIONS))
    regions[TABLE] = struct.pack('<48I', *table)
    regions[SECTIONS] = bytes(0x500)
    if name == 'lock':
        return Call(0x682720, [which], regions, conv='c', ret='v')
    if name == 'unlock':
        return Call(0x6827a0, [which], regions, conv='c', ret='a')
    if name in ('lock_file', 'unlock_file'):
        file = r.choice([0x77e800, 0x77e820, 0x77e81f, 0x77e7ff, 0x77ea60, 0x77ea61, 0x77ea40, RAM + 0x2000])
        return Call(0x6827c0 if name == 'lock_file' else 0x682830, [file], regions, conv='c', ret='v')
    return Call(0x682800 if name == 'lock_file2' else 0x682870, [r.choice([0, 1, 3, 0x13]) if r.random() < 0.5 else r.choice([0x14, 0x15, 0x100, -1]) & 0xffffffff, RAM + 0x2000], regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)
