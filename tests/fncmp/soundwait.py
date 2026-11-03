# The sound threads' waits (src/game/core/soundwait) against the originals: WaitForMultipleObjects' import slot a COM recorder (its first argument, the count, logged as 0 by the recorder's kind)
# answering in turn, its handles the flags' own address so that each call writes the flags (the way another thread would), the last with the loop's end; the refills recorders 20, 21 (the
# voice's address, the position). The handshake 0x6b9ed0 with byte +5 already set (the other thread: a zero would spin in both); the atexit 0x6b9ec0 nothing.
# usage: soundwait.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=voices,source,handshake,atexit)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, STUBS, Bench, Call, _recorder, answers, blob, fill, heap, import_table, log_region, put, set_return, strings, vtable  # noqa: E402

# The engine's thread (0x6b9e30, 0x6b9ef0, 0x6b9f40, 0x6ba020) with the handshake's singleton mapped: the mixer real, _beginthread and the object's open recorders 24 and 23. The thread's loop ends only when something sets the
# handshake's byte +4: the list's last voice lies on the singleton at +3 (its vtable word the bytes +3 to +6, reading 0x30000000 = OBJ with the bytes zero) and its slot 0x2c is the driver's and the emulator's stub_mark, whose word 0x6fd800 sets +4.
B = Bench([(0x6c6180, 0x6c61d6), (0x6d3800, 0x6d384c), (0x6b9ec0, 0x6b9ec1), (0x6b9ed0, 0x6b9ee9), (0x6b9e80, 0x6b9eba), (0x6b9fa0, 0x6ba018), (0x6b5e20, 0x6b5e3e), (0x6ba080, 0x6ba089), (0x6b6260, 0x6b62a1), (0x6b9d90, 0x6b9d98),
           (0x6b9e30, 0x6b9e73), (0x6b9ef0, 0x6b9f3f), (0x6b9f40, 0x6b9f92), (0x6ba020, 0x6ba07e), (0x6b85a0, 0x6b85f0), (0x6b6240, 0x6b6260), (0x6b7190, 0x6b7198)],
          hooks={0x6bb590: _recorder(20, 0), 0x6d3850: _recorder(21, 1), 0x67a7d0: _recorder(22, 1, cdecl=True), 0x6b8480: _recorder(23, 2), 0x67e050: _recorder(24, 3, cdecl=True)})
OPS = [k for k in os.environ.get('OPS', 'voices,source,handshake,atexit,instance,engine,deleting,touch,thread,start,open,threaded,lock').split(',') if k]
OBJ, TABLE = RAM, RAM + 0x400
INNER, INNER_VT, INNER2, INNER2_VT = RAM + 0x800, RAM + 0x900, RAM + 0xa00, RAM + 0xb00
MIXER, VOICE, VOICE_VT, OUT, STATUS, TARGET = RAM + 0xc00, RAM + 0xd00, RAM + 0xe00, RAM + 0xf00, RAM + 0xf10, RAM + 0xf20
GUARD = 0xfb1f08   # (the guard byte, then at +8 the singleton 0xfb1f10)
WAIT_K, BUMP_K, NAP_K, PRIORITY_K, TOUCHED_K, LOCK_K = 1, 2, 3, 4, 5, 6
OPEN_K, BEGIN_K = 23, 24


def singleton(r, stopped=None):
    """The guard's bytes: its bit 0 set (or clear, 1 in 4, unless a stop byte is asked), and the singleton's +4 and +5 as asked (random otherwise)."""
    guard = bytearray(r.bytes(0x60))
    guard[0] = guard[0] | 1 if stopped is not None or r.random() < 0.75 else guard[0] & ~1
    if stopped is not None:
        guard[0xc] = stopped
    return guard


def mixer_chain(r, log, regions, obj, last=0):
    """The engine's mixer and its voices: one with or without the flag and a recorder for slot 0x2c, then `last`. InterlockedIncrement's recorder writes its count or leaves it 0."""
    mixer = bytearray(r.bytes(0x60))
    voice = bytearray(r.bytes(0x40))
    put(voice, 0, 'I', VOICE_VT)
    put(voice, 0x30, 'I', r.getrandbits(32) | 0x400 if r.random() < 0.5 else r.getrandbits(32) & ~0x400)
    put(voice, 0x34, 'I', last)
    put(mixer, 0x24, 'I', r.choice([VOICE, VOICE, last]))
    regions.update({MIXER: mixer, VOICE: voice, VOICE_VT: vtable({0xb: 0}), IMPORT_TABLE: import_table(log, {'InterlockedIncrement': (BUMP_K, 0x6fd0c0, 1, 0, 0), 'Sleep': (NAP_K, 0x6fd1d4, 1, 0, 0), 'SetThreadPriority': (PRIORITY_K, 0x6fd1fc, 2, 0, 0)})})
    strings(log, BUMP_K, 0, out=1)
    if r.random() < 0.5:
        fill(log, BUMP_K, 0, struct.pack('<I', r.choice([1, 2, r.getrandbits(32)])))
    put(obj, 0x20, 'I', MIXER)


def engine_case(name, r, log, regions, obj):
    if name == 'lock':
        regions[IMPORT_TABLE] = import_table(log, {'DeleteCriticalSection': (LOCK_K, 0x6fd0d0, 1, 0, 0)})
        return Call(0x6b7190, [OBJ], regions, conv='t', ret='v')
    if name == 'touch':
        mixer_chain(r, log, regions, obj)
        return Call(0x6ba080, [OBJ], regions, conv='t', ret='a')
    if name == 'thread':
        stop_now = r.random() < 0.2
        guard = singleton(r)
        if not guard[0] & 1 or not stop_now:
            guard[0xb:0xf] = struct.pack('<I', OBJ)   # (the voice's vtable word; reads as OBJ after the singleton's first making too)
        else:
            guard[0xc] = r.randrange(1, 256)
        guard[0x3b:0x3f] = struct.pack('<I', r.getrandbits(32) | 0x400)
        guard[0x3f:0x43] = bytes(4)
        regions[GUARD] = guard
        put(obj, 0x2c, 'I', STUBS + 0x40)
        mixer_chain(r, log, regions, obj, last=0xfb1f13)
        put(regions[MIXER], 0x24, 'I', r.choice([VOICE, 0xfb1f13]))
        return Call(0x6b9e30, [OBJ], regions, conv='t', ret='v')
    regions[OUT] = r.bytes(8)
    regions[GUARD] = singleton(r, stopped=None)
    regions[STATUS] = struct.pack('<I', r.choice([0, 1, 0x7fffffff, 0x80000000, 0xffffffff]))
    regions[IMPORT_TABLE] = import_table(log, {'SetThreadPriority': (PRIORITY_K, 0x6fd1fc, 2, 0, 0)})
    set_return(log, BEGIN_K, r.choice([0, 0x1234, r.getrandbits(32) | 1]))
    if name == 'start':
        put(obj, 0, 'I', r.choice([0, 0, r.getrandbits(32) | 1]))
        return Call(0x6b9ef0, [OBJ, OUT, r.getrandbits(32)], regions, conv='t', ret='a')
    if regions[GUARD][0] & 1:
        put(regions[GUARD], 8, 'I', r.choice([0, 0, r.getrandbits(32) | 1]))
    else:
        regions[GUARD][0] |= r.choice([0, 1])
    if name == 'open':
        strings(log, OPEN_K, 0, out=1)
        set_return(log, OPEN_K, STATUS)
        mixer = bytearray(r.bytes(0x60))
        regions[MIXER] = mixer
        put(obj, 0x20, 'I', r.choice([0, MIXER, MIXER]))
        return Call(0x6b9f40, [OBJ, OUT, r.getrandbits(32)], regions, conv='t', ret='a')
    on = r.choice([0, 1, 1, r.getrandbits(32) | 1])
    if not on:
        regions[GUARD][0] |= 1
        regions[GUARD][0xd] = r.randrange(1, 256)
    put(obj, 0x24, 'I', r.choice([0, TARGET, TARGET]))
    put(obj, 0x20, 'I', 0)
    regions[TARGET] = r.bytes(0x30)
    return Call(0x6ba020, [OBJ, OUT, on], regions, conv='t', ret='a')


def case(r):
    name = r.deal(OPS)
    log = log_region()
    obj = bytearray(r.bytes(0x40))
    regions = {LOG: log, OBJ: obj}
    if name in ('lock', 'touch', 'thread', 'start', 'open', 'threaded'):
        return engine_case(name, r, log, regions, obj)
    if name == 'deleting':
        guard = bytearray(r.bytes(0x10))
        guard[0] |= 1
        guard[0xd] = r.randrange(1, 256)
        regions.update({GUARD: guard, HEAP: heap(0x40), INNER: struct.pack('<I', INNER_VT) + r.bytes(0xc), INNER_VT: vtable({0: 1}), INNER2: struct.pack('<I', INNER2_VT) + r.bytes(0xc), INNER2_VT: vtable({})})
        put(obj, 0x20, 'I', r.choice([0, INNER]))
        put(obj, 0x24, 'I', r.choice([0, INNER2]))
        return Call(0x6b5e20, [OBJ, r.choice([0, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')
    if name == 'instance':   # (the guard's bit 0 either way: the other bits kept)
        regions[GUARD] = r.bytes(0x10)
        return Call(0x6b9e80, [], regions, conv='c', ret='a')
    if name == 'engine':   # (the handshake made already, +5 set: a zero would spin in both. The inner objects null or a recorder vtable)
        guard = bytearray(r.bytes(0x10))
        guard[0] |= 1
        guard[0xd] = r.randrange(1, 256)
        regions.update({GUARD: guard, INNER: struct.pack('<I', INNER_VT) + r.bytes(0xc), INNER_VT: vtable({0: 1}),
                        INNER2: struct.pack('<I', INNER2_VT) + r.bytes(0xc), INNER2_VT: vtable({})})
        put(obj, 0x20, 'I', r.choice([0, INNER]))
        put(obj, 0x24, 'I', r.choice([0, INNER2]))
        return Call(0x6b9fa0, [OBJ], regions, conv='t', ret='v')
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
    untouched = r.random() < 0.4   # (no fill: the stop handle ends the loop, and the hash below sees the running bit the code set)
    if not untouched:
        fill(log, WAIT_K, 1, struct.pack('<%dI' % waits, *turns), records=waits)
    blob(log, WAIT_K, 1, 1)   # (the flags as each wait leaves them)
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
    table = [r.choice([other, other, 0, 1, 2, r.randrange(0, 8), r.getrandbits(16)]) for _ in range(40)]
    if untouched:
        table[waits - 1] = other
    regions[TABLE] = struct.pack('<40I', *table)
    answers(log, WAIT_K, TABLE)
    return Call(0x6c6180 if voices else 0x6d3800, [OBJ], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
