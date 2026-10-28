# The asynchronous pointer (src/game/gfx/pointer: 0x5fb8e0-0x5fbdd9) against the originals: the pointer object random within reason (a thread
# handle or none, stopped, paused and park bytes, the next tick, the cursor the singleton 0xfa7a80 or none), the display a vtable of recorders
# (back and front surfaces, bounds, flipping; their answers set per case), the cursor's state and surfaces as in softcursor.py but plainer. The
# imports (the semaphore's CreateSemaphore / WaitForSingleObject / ReleaseSemaphore / CloseHandle, GetExitCodeThread filling its exit code (the
# stack, logged as 0), OutputDebugString's messages hashed, timeGetTime, Sleep) are COM-style recorders, QueryPerformanceCounter fncmp.COUNTER
# stepping on each call; _beginthreadex / _endthreadex cdecl recorders (driver.cpp's AsyncPointer env), atexit fncmp's cdecl log.
# The thread procedure's loop (Run, ThreadProc) ends only when the stop byte is set: their cases give the semaphore the stop byte's address as its
# handle and make WaitForSingleObject write 1 through it, with the time moving on, the cursor set and not paused, so the first Tick sets it.
# Show / Hide (0x47ebc0 / 0x47ec30, the 0x6fefc8 class's slots 2 and 3): the object a frame (small, negative or any), a hot spot of signed
# bytes and IMAGE's surface inline at +0x14, the pointer [0x7b4b84] P.
# Step (0x47eca0, PointerFrames_Step): Show's object shown or not, the pointer or none, its frame count +8 (1-4, 0 or any), the last step
# +0x10 and now 300 apart or about, any or wrapping.
# usage: asyncpointer.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,... to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import (COUNTER, HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _cdecl_log, _recorder, blob, counter, fill, heap, import_table,  # noqa: E402
                   log_region, put, recorder, set_return, strings, vtable)

BEGIN_THREAD, END_THREAD = 30, 31   # (the cdecl recorders driver.cpp's AsyncPointer env points at)
B = Bench([(0x5f5ee0, 0x5f5fd0), (0x5f9000, 0x5f92f0), (0x5fa680, 0x5fa6b0), (0x5fad40, 0x5fb8f0), (0x5fb8e0, 0x5fbdf0), (0x61b090, 0x61b0f0),
           (0x61b130, 0x61b1f0), (0x62f810, 0x62f8e0), (0x6306c0, 0x630890), (0x67a740, 0x67a800), (0x67acf0, 0x67ada0), (0x67afa0, 0x67b060),
           (0x67fa20, 0x67fa40), (0x682460, 0x6824f0), (0x47ebc0, 0x47ec21), (0x47ec30, 0x47ec75), (0x47eca0, 0x47ed32)],
          hooks={0x67a7d0: _cdecl_log, 0x67f780: _recorder(BEGIN_THREAD, 6, cdecl=True), 0x67f8b0: _recorder(END_THREAD, 1, cdecl=True)})
CALLS = {'Ctor': 0x5fb8e0, 'DeletingDtor': 0x5fb920, 'Dtor': 0x5fb940, 'Detach': 0x5fb990, 'Stop': 0x5fb9b0, 'ThreadProc': 0x5fba00,
         'Tick': 0x5fba10, 'Run': 0x5fba70, 'Lock': 0x5fbad0, 'Unlock': 0x5fbb70, 'Attach': 0x5fbbd0, 'Start': 0x5fbc30, 'SetPaused': 0x5fbc80,
         'SetImage': 0x5fbcf0, 'BeginFlip': 0x5fbd40, 'EndFlip': 0x5fbd90, 'Show': 0x47ebc0, 'Hide': 0x47ec30, 'Step': 0x47eca0}
GUARD, C, CLOCK = 0xfa7a78, 0xfa7a80, 0xfa71e0
P, D, DVT, OBJS, VT, TARGET, FRONT, IMAGE, OUT, POS, RECT, HOT, BOUNDS, BITS = (RAM, RAM + 0x40, RAM + 0x60, RAM + 0x100, RAM + 0x200, RAM + 0x400,
                                                                               RAM + 0x460, RAM + 0x4c0, RAM + 0x520, RAM + 0x550, RAM + 0x560,
                                                                               RAM + 0x570, RAM + 0x580, RAM + 0x590)
B0, B1, B2, TOBJ, FOBJ, IOBJ = (OBJS + 0x10 * i for i in range(6))
SEM, THREAD = 0x5e3a, 0x7c4
BACK_K, FRONT_K, BOUNDS_K, FLIPPING_K = 11, 12, 13, 14   # (the display's slots 1-4: thiscall recorders)
RELEASE, CREATE, BLTFAST, DESC, ISLOST, RESTORE, SETKEY = 2, 6, 7, 22, 24, 27, 29
DD, VTD, FRAMES = RAM + 0x600, RAM + 0x680, RAM + 0x700   # (the IDirectDraw at [0xfa76b0]: CreateSurface, for the cursor's buffers)
IMPORTS = {'CreateSemaphoreA': (40, 0x6fd16c, 4, 0, 0), 'CloseHandle': (41, 0x6fd13c, 1, 0, 0), 'WaitForSingleObject': (42, 0x6fd128, 2, 0, 0),
           'ReleaseSemaphore': (43, 0x6fd170, 3, 0, 0), 'GetExitCodeThread': (44, 0x6fd0b8, 2, 0, 1 << 1), 'OutputDebugStringA': (45, 0x6fd1e0, 1, 1, 0),
           'timeGetTime': (46, 0x6fd3ec, 0, 0, 0), 'Sleep': (47, 0x6fd1d4, 1, 0, 0)}
WAIT, EXIT_CODE, TIME = 42, 44, 46


def rect(r):
    left, top = r.randrange(-12, 60), r.randrange(-12, 60)
    return struct.pack('<4i', left, top, left + r.randrange(0, 40), top + r.randrange(0, 40))


def surface(r, obj):
    s = bytearray(r.bytes(0x50))
    put(s, 0x24, 'I', obj)
    return s


def cursor(r, created):
    """The cursor singleton: created or not, small sizes and rectangles, the buffer indices 0-2, its buffers' surfaces the shared COM vtable's."""
    c = bytearray(r.bytes(0x230))
    put(c, 5, 'B', created)
    put(c, 0xc, '4I', r.randrange(0, 48), r.randrange(0, 48), r.randrange(0, 40), r.randrange(0, 40))
    put(c, 0x1c, '5i', r.choice([0, 1, 2]), r.choice([0, 1, 2]), r.choice([0, 0, 1, 2]), r.choice([0, 1]), r.choice([0, 1]))
    c[0x30:0x40] = r.choice([struct.pack('<4i', 0, 0, 640, 480), rect(r)])
    put(c, 0x40, '4i', -r.randrange(0, 16), -r.randrange(0, 16), r.randrange(-8, 80), r.randrange(-8, 80))
    for i, obj in enumerate((B0, B1, B2)):
        at = 0x50 + 0x5c * i
        c[at + 4:at + 0x54] = surface(r, obj)
        put(c, at + 0x54, '2I', r.choice([0, 1, 1]), r.choice([0, 0, 0, 1]))
    for i in range(12):
        c[0x164 + 0x10 * i:0x174 + 0x10 * i] = rect(r)
    put(c, 0x224, '3I', *[TARGET if r.random() < 0.9 else 0 for _ in range(3)])
    return c


def desc(r):
    d = bytearray(r.bytes(0x6c))
    put(d, 8, '3i', r.randrange(0, 64), r.randrange(0, 64), r.choice([64, 128]))
    put(d, 0x24, 'I', BITS)
    put(d, 0x54, '5I', *r.choice([(8, 0, 0, 0, 0), (16, 0xf800, 0x7e0, 0x1f, 0), (32, 0xff0000, 0xff00, 0xff, 0)]))
    return bytes(d)


def case(r):
    op = r.deal(OPS)
    loop = op in ('Run', 'ThreadProc')
    log = log_region()
    table = import_table(log, IMPORTS)
    struct.pack_into('<I', table, 0x6fd0cc - IMPORT_TABLE, COUNTER)   # (QueryPerformanceCounter)
    counter(log, r.getrandbits(20), r.choice([0x100, 0x1000, r.randrange(1, 1 << 16)]) if loop else r.getrandbits(16), 0xffffffff)
    strings(log, BEGIN_THREAD, 0, 1 << 5)   # (the thread id: the stack)
    set_return(log, BEGIN_THREAD, r.choice([THREAD, THREAD, 0]))
    fill(log, EXIT_CODE, 1, struct.pack('<I', r.choice([0x103, 0x103, 0, 1])), 1)
    set_return(log, EXIT_CODE, r.choice([1, 1, 0]))
    set_return(log, WAIT, 0 if loop else r.choice([0, 0, 0x102, 0xffffffff]))
    if loop:   # (the first Tick's lock writes the stop byte: the handle is its address)
        fill(log, WAIT, 0, b'\x01', 0)
    else:
        strings(log, DESC, 0, 1 << 1)   # (the description: the stack)
        fill(log, DESC, 1, desc(r), 0)
    blob(log, BLTFAST, 4, 0xf)
    blob(log, CREATE, 1, 0xf | 1 << 26)   # (the size, flags, height, width and caps)
    set_return(log, CREATE, r.choice([0, 0, 0, 1]))
    set_return(log, BLTFAST, r.choice([0, 0, 0, 1]))
    blob(log, SETKEY, 2, 3)
    set_return(log, ISLOST, 0)
    set_return(log, BACK_K, TARGET if loop or r.random() < 0.85 else 0)
    set_return(log, FRONT_K, r.choice([FRONT, TARGET]))
    set_return(log, BOUNDS_K, BOUNDS)
    strings(log, BOUNDS_K, 0, 1)   # (the rectangle it is handed: the stack; what it returns is read)
    set_return(log, FLIPPING_K, r.choice([0, 0, 1, 0x100]))
    p = bytearray(r.bytes(0x24))
    put(p, 4, '3I', D, P + 0x10 if loop else SEM, r.choice([0, THREAD, THREAD]))
    put(p, 0x10, '3B', r.choice([0, 1]), 0 if loop else r.choice([0, 1]), r.choice([0, 1]))
    put(p, 0x14, '4I', r.getrandbits(32) if r.random() < 0.3 else r.getrandbits(8), r.choice([0x42, 0x42, r.getrandbits(32)]),
        C if loop or r.random() < 0.95 else 0, POS)
    dvt = struct.pack('<5I', 0, recorder(BACK_K, 0), recorder(FRONT_K, 0), recorder(BOUNDS_K, 1), recorder(FLIPPING_K, 0))
    regions = {LOG: log, IMPORT_TABLE: table, P: p, D: struct.pack('<I', DVT), DVT: dvt, OBJS: struct.pack('<4I', VT, 0, 0, 0) * 6,
               VT: vtable({}, com={RELEASE: 1, BLTFAST: 6, DESC: 2, ISLOST: 1, RESTORE: 1, SETKEY: 3}),
               GUARD: struct.pack('<I', r.choice([0, 1, 1, 1])) + r.bytes(4) + cursor(r, 0 if loop else r.choice([0, 1, 1])),
               TARGET: surface(r, TOBJ), FRONT: surface(r, FOBJ), IMAGE: surface(r, IOBJ), OUT: r.bytes(4),
               POS: struct.pack('<2i', r.randrange(-8, 80), r.randrange(-8, 80)), RECT: rect(r), HOT: struct.pack('<2i', r.randrange(0, 16), r.randrange(0, 16)),
               BOUNDS: r.choice([struct.pack('<4i', 0, 0, 640, 480), rect(r)]), BITS: r.bytes(4), 0x775874: r.bytes(4), 0xfa76b0: struct.pack('<I', DD) + r.bytes(4), DD: struct.pack('<I', VTD), VTD: vtable({}, com={CREATE: 4}),
               CLOCK: struct.pack('<4IQ', 1 if loop else r.choice([0, 1]), 1 if loop else r.choice([1, 0x3e8]), r.getrandbits(32), 0, 0)}
    address = CALLS[op]
    if op == 'Ctor':
        return Call(address, [P, D], regions, conv='t', ret='a')
    if op in ('Show', 'Hide', 'Step'):
        frames = bytearray(r.bytes(0x14)) + surface(r, IOBJ)
        put(frames, 0xc, 'i', r.choice([0, 1, 2, 5, -1, r.randrange(-0x8000000, 0x8000000)]))
        regions.update({FRAMES: frames, 0x7b4b84: struct.pack('<I', P)})
        set_return(log, CREATE, 1)   # (its recorder leaves the new surface unset: SetImage's own cases cover a success)
        if op == 'Step':
            frames[4] = r.choice([1, 1, 1, 0, r.getrandbits(8)])
            put(frames, 8, 'i', r.choice([1, 2, 3, 4, 4, 0, -1, r.getrandbits(31)]))
            put(frames, 0xc, 'i', r.choice([0, 1, 2, 3, 3, -1, r.randrange(-0x8000000, 0x8000000)]))
            last = r.choice([0, r.getrandbits(32), 0x7fffff00])
            put(frames, 0x10, 'I', last)
            now = (last + r.choice([300, 301, 299, 1000, 0, -5, r.getrandbits(32)])) & 0xffffffff
            if r.random() < 0.1:
                regions[0x7b4b84] = struct.pack('<I', 0)
            return Call(address, [FRAMES, now], regions, conv='t', ret='v')
        return Call(address, [FRAMES], regions, conv='t', ret='v')
    if op == 'DeletingDtor':
        regions[HEAP] = heap()
        return Call(address, [P, r.choice([0, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')
    if op in ('Dtor', 'Detach', 'Stop', 'Unlock', 'BeginFlip', 'EndFlip'):
        return Call(address, [P], regions, conv='t', ret='v')
    if op == 'ThreadProc':
        return Call(address, [P], regions, conv='s', ret='a')
    if op == 'Run':
        return Call(address, [P], regions, conv='t', ret='a')
    if op == 'Tick':   # (now at, just past or before the next tick too)
        now = struct.unpack_from('<I', p, 0x14)[0] + r.choice([0, 0, 1, -1]) if r.random() < 0.4 else r.getrandbits(32) if r.random() < 0.3 else r.getrandbits(9)
        return Call(address, [P, now & 0xffffffff], regions, conv='t', ret='v')
    if op == 'Lock':
        return Call(address, [P, r.choice([0, 1, 0x100, r.getrandbits(32)])], regions, conv='t', ret='b')
    if op == 'Attach':   # (a position other than the one it had)
        regions[POS + 8] = struct.pack('<2i', r.randrange(-8, 80), r.randrange(-8, 80))
        return Call(address, [P, OUT, POS + 8], regions, conv='t', ret='a')
    if op == 'Start':
        return Call(address, [P, OUT], regions, conv='t', ret='a')
    if op == 'SetPaused':
        return Call(address, [P, OUT, r.choice([0, 1, 0x100, r.getrandbits(32)])], regions, conv='t', ret='a')
    return Call(address, [P, OUT, r.choice([IMAGE, IMAGE, 0]), RECT, HOT], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
