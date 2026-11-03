# The sound threads' waits (src/game/core/soundwait) against the originals: WaitForMultipleObjects' import slot a COM recorder (its first argument, the count, logged as 0 by the recorder's kind)
# answering in turn, its handles the flags' own address so that each call writes the flags (the way another thread would), the last with the loop's end; the refills recorders 20, 21 (the
# voice's address, the position). The handshake 0x6b9ed0 with byte +5 already set (the other thread: a zero would spin in both); the atexit 0x6b9ec0 nothing.
# usage: soundwait.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=voices,source,handshake,atexit)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, blob, fill, import_table, log_region, put  # noqa: E402

B = Bench([(0x6c6180, 0x6c61d6), (0x6d3800, 0x6d384c), (0x6b9ec0, 0x6b9ec1), (0x6b9ed0, 0x6b9ee9)], hooks={0x6bb590: _recorder(20, 0), 0x6d3850: _recorder(21, 1)})
OPS = [k for k in os.environ.get('OPS', 'voices,source,handshake,atexit').split(',') if k]
OBJ, TABLE = RAM, RAM + 0x400
WAIT_K = 1


def case(r):
    name = r.deal(OPS)
    log = log_region()
    obj = bytearray(r.bytes(0x40))
    regions = {LOG: log, OBJ: obj}
    if name == 'atexit':
        return Call(0x6b9ec0, [], regions, conv='c', ret='v')
    if name == 'handshake':
        obj[5] = r.randrange(1, 256)
        return Call(0x6b9ed0, [OBJ], regions, conv='t', ret='v')
    voices = name == 'voices'
    at_flags = 0x14 if voices else 8
    running = (lambda v: v & ~1) if voices else (lambda v: v | 0x40)
    ended = (lambda v: v | 1) if voices else (lambda v: v & ~0x40)
    waits = r.randrange(1, 6)
    turns = [running(r.getrandbits(32)) for _ in range(waits - 1)] + [ended(r.getrandbits(32))]
    regions[IMPORT_TABLE] = import_table(log, {'WaitForMultipleObjects': (WAIT_K, 0x6fd160, 4, 0, 0)})
    fill(log, WAIT_K, 1, struct.pack('<%dI' % waits, *turns), records=waits)
    blob(log, WAIT_K, 1, 1)   # (the flags as each wait finds them: the running bit)
    put(obj, at_flags, 'I', (ended if r.random() < 0.15 else running)(r.getrandbits(32)))   # (ended: the loop never entered)
    count = r.randrange(1, 8)
    if voices:
        other = r.randrange(0, 8)
        put(obj, 8, 'I', count)
        put(obj, 0xc, 'I', other)
        put(obj, 0x10, 'I', OBJ + at_flags)
    else:
        other = count - 1
        put(obj, 0x30, 'I', count)
        put(obj, 0x2c, 'I', OBJ + at_flags)
    regions[TABLE] = struct.pack('<40I', *(r.choice([other, other, 0, 1, 2, r.randrange(0, 8), r.getrandbits(16)]) for _ in range(40)))
    answers(log, WAIT_K, TABLE)
    return Call(0x6c6180 if voices else 0x6d3800, [OBJ], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
