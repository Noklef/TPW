# The front-end scene's id queue, its step (src/game/frontend/scenequeue: 0x5d5f80 Class702da0_Step) against the original: the advisor speaking or not (0xf7966c), a count with a tail node of 0 or more back
# links (rarely no tail node: faults in both), the armed flag +0x18, the clock's +0x28, the time before, at and past +0x20 + +0x1c (some wrapping), the owner [[0xf85614] + 0x14] any, a failed allocation.
# Recorders (driver.cpp's kSceneQueueEnv): Advisor_Say, Time_GetMs, List_PushHead.
# usage: scenequeuestep.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, put, set_return  # noqa: E402

SAY, NOW, PUSH = 5, 6, 7
B = Bench([(0x5d5f80, 0x5d6052)], hooks={0x599050: _recorder(SAY, 1, cdecl=True), 0x5f5fa0: _recorder(NOW, 0, cdecl=True), 0x5e2a30: _recorder(PUSH, 1)})
OBJECT, NODE, PREVIOUS, ITEM, FRONT = RAM, RAM + 0x40, RAM + 0x60, RAM + 0x80, RAM + 0xa0


def case(r):
    path = r.deal(['speaking', 'pop', 'off', 'arm', 'wait', 'fire'])
    log = log_region()
    start, time = r.choice([r.randrange(0x10000), r.getrandbits(32)]), r.choice([0, 1, r.randrange(0x10000), r.getrandbits(32)])
    limit = (start + time) & 0xffffffff
    now = limit + r.choice([1, 2, r.randrange(1, 0x10000)]) if path == 'fire' else r.choice([limit, limit, limit - 1, limit + 1, r.getrandbits(32)])
    now &= 0xffffffff
    set_return(log, NOW, now)
    set_return(log, SAY, r.getrandbits(32))
    set_return(log, PUSH, r.getrandbits(32))
    this = bytearray(r.bytes(0x30))
    put(this, 0x1c, 'II', time, start)
    put(this, 0x10, 'I', NODE if r.randrange(20) else 0)
    put(this, 0x14, 'I', r.choice([1, 2, r.getrandbits(32)]) if path == 'pop' else 0)
    put(this, 0x18, 'I', 0 if path == 'off' else r.choice([1, r.getrandbits(32) | 1]))
    put(this, 0x28, 'I', r.choice([1, r.getrandbits(32) | 1]) if path == 'arm' else 0)
    node = bytearray(r.bytes(0xc))
    put(node, 0, 'I', ITEM)
    put(node, 8, 'I', r.choice([0, PREVIOUS]))
    regions = {LOG: log, HEAP: heap(fail=r.choice([0, 0, 0, 0, 0, 1])), OBJECT: this, NODE: node, PREVIOUS: r.bytes(0xc), ITEM: r.bytes(4), FRONT: put(bytearray(r.bytes(0x20)), 0x14, 'I', r.getrandbits(32)),
               0xf85614: struct.pack('<I', FRONT), 0xf7966c: struct.pack('<I', r.choice([1, r.getrandbits(32) | 1]) if path == 'speaking' else 0)}
    if path == 'speaking':   # (and anything else set)
        put(this, 0x14, 'I', r.choice([0, 1]))
    return Call(0x5d5f80, [OBJECT], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
