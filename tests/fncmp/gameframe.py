# The game's step each frame (src/game/core/gameframe: 0x54e360) against the original. Every state; its callees recorders (driver.cpp's kGameFrameCalls, made from the lists below: gameframe.py --table prints them), but for
# the real strings, the grab's name (screengrab.py's file imports) and the C library's sprintf. Answers: the clock, the keys, the timers, each decision's call; the singletons had or made (their allocation failing now and then);
# the steps behind and the step counter about the steps every 2, 8 and 32; the wait for the chat: its 300 s in the park's short path (a longer one fills the log, whose count freezes the answers), else ended by the chat at once.
# usage: gameframe.py [CASES [SEED]] [--wild|--both] [--show N]   (env STATES=9,10)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, IMPORT_TABLE, LOG, LOG_BYTES, RAM, Bench, Call, _recorder, answers, blob, fail_after, fb, fill, heap, import_table, log_fold, log_region, put, recorder, set_return, strings  # noqa: E402
import fileopen  # noqa: E402

# Slots with an answer or an argument's setting, each its own: (kind: T thiscall, C cdecl, S stdcall; stack arguments, slot).
FIXED = {0x402d70: ('T', 0, 1), 0x407d80: ('T', 0, 2), 0x407e00: ('T', 1, 3), 0x619240: ('T', 0, 4), 0x5b96b0: ('T', 0, 5), 0x5bd270: ('T', 1, 6), 0x5c7d00: ('T', 1, 7), 0x4e6150: ('T', 0, 8), 0x5afc40: ('T', 0, 9),
         0x5b0cb0: ('T', 0, 10), 0x5d5cf0: ('T', 0, 11), 0x4c81e0: ('T', 0, 12), 0x4c8d30: ('T', 4, 13), 0x4e5260: ('T', 0, 14), 0x5b50b0: ('T', 0, 15), 0x5c7420: ('T', 0, 16), 0x5aef50: ('T', 0, 17), 0x5d5770: ('T', 0, 18),
         0x6595a8: ('C', 0, 19), 0x51b300: ('C', 0, 20), 0x457a90: ('C', 0, 21), 0x457c30: ('C', 0, 22), 0x40ef40: ('C', 0, 23), 0x5f5fa0: ('C', 0, 24), 0x429ba0: ('C', 4, 25), 0x40fb10: ('T', 1, 26), 0x5c5580: ('T', 1, 27),
         0x409480: ('T', 1, 28), 0x4088a0: ('T', 1, 29), 0x5c7f40: ('T', 3, 30), 0x414d40: ('T', 3, 31), 0x67acf0: ('S', 5, 36), 0x67afa0: ('S', 4, 37)}
K = {addr: v[2] for addr, v in FIXED.items()}
KEY, TICKS, SAY, QUERY = 32, 33, 34, 35
# Thiscalls of no arguments, by the object they are called on: more of them than slots, so a slot serves one call on each object (the log has the object too).
THIS0 = [0x4813b0, 0x4813e0, 0x4813f0, 0x481400, 0x481410, 0x481420, 0x481430,   # the panel
         0x402ed0,   # the clocks
         0x409180, 0x409300, 0x5b6480, 0x5c5520,   # the owner
         0x41a820, 0x5879c0,
         0x5d5970, 0x5d5c50, 0x5d5cc0, 0x5d5840,   # 0xf82884's
         0x576820,
         0x4e5950, 0x4e5be0, 0x4e5ef0, 0x4e6130, 0x4e6170, 0x4e6560,   # the chat
         0x516380, 0x5166b0, 0x519510,   # the world
         0x5accf0, 0x5b5920, 0x5b96a0, 0x5c74b0, 0x5c8650, 0x5af250, 0x5b0cc0, 0x550ca0]
THIS1 = [0x402ea0, 0x404140, 0x4093d0, 0x46c0d0, 0x46c350, 0x518f90, 0x519060, 0x550d80, 0x56b790, 0x576260, 0x5765c0, 0x58dd60, 0x5ac5f0, 0x5ac610, 0x5c5560, 0x5c83b0]
THIS3 = [0x578be0, 0x587380]
# Cdecl of no arguments: past the cdecl slots, stdcall ones (nothing to pop).
PLAIN0 = [0x42b1c0, 0x435a30, 0x436490, 0x443340, 0x44e510, 0x44e540, 0x44e550, 0x4628a0, 0x473440, 0x4752e0, 0x475360, 0x48aff0, 0x51b350, 0x51bcb0, 0x51bd70, 0x51e730, 0x51e770, 0x51eda0, 0x51ee40, 0x51fd20, 0x520130, 0x541ed0,
          0x546c80, 0x54df20, 0x54e070, 0x550950, 0x550b30, 0x551600, 0x5516b0, 0x557ab0, 0x5584b0, 0x55a470, 0x55a5a0, 0x55ab50, 0x55abf0, 0x55d4d0, 0x598ad0, 0x5989c0, 0x599800, 0x599880, 0x6591e3, 0x659201]
PLAIN = {1: [0x429df0, 0x429e20, 0x44e410, 0x457d50, 0x458590, 0x462870, 0x51c700, 0x51e6f0, 0x51e790, 0x51e7b0, 0x51e830, 0x51ec50, 0x51f370, 0x51fd80, 0x53e760, 0x55ce60, 0x5996d0, 0x67a7d0],
         2: [0x540900], 3: [0x44a280, 0x51faa0], 9: [0x51c1d0]}


def table():
    """Every recorded call: {address: (kind, stack arguments, slot)}."""
    calls = dict(FIXED)
    free = [k for k in range(32) if k not in [v[2] for v in FIXED.values() if v[:2] == ('T', 0)]]
    for i, addr in enumerate(THIS0):
        calls[addr] = ('T', 0, free[i % len(free)])
    settled = [k for k in range(25) if k != 13]   # (not those with an argument's setting)
    for i, addr in enumerate(THIS1):
        calls[addr] = ('T', 1, [k for k in settled if k not in (3, 6, 7)][i])
    for i, addr in enumerate(THIS3):
        calls[addr] = ('T', 3, settled[i])
    free = [k for k in range(32) if k not in [v[2] for v in FIXED.values() if v[:2] == ('C', 0)]]
    for i, addr in enumerate(PLAIN0):
        calls[addr] = ('C', 0, free[i]) if i < len(free) else ('S', 0, i - len(free))
    for n, addrs in PLAIN.items():
        for i, addr in enumerate(addrs):
            calls[addr] = ('C', n, settled[i])
    return calls


CALLS = table()
HOOKS = {addr: _recorder(k, n, com=kind == 'S', cdecl=kind == 'C') for addr, (kind, n, k) in CALLS.items()}
B = Bench(sorted(set([(0x54e360, 0x5503a2), (0x40f630, 0x40f649), (0x5617c0, 0x5617fe)] + fileopen.RANGES)), hooks=HOOKS)
STATES = [int(k) for k in os.environ.get('STATES', '0,1,1,2,2,3,3,4,5,5,6,6,7,7,8,8,9,9,9,9,9,9,10,10,10,10,10,10,10,10,11,11,11,12,12,13,14,15,16').split(',')]
IMPORTS = dict(fileopen.IMPORTS)
IMPORTS.update({'GetAsyncKeyState': (KEY, 0x6fd320, 1, 0, 0), 'timeGetTime': (TICKS, 0x6fd1d0, 0, 0, 0), 'OutputDebugStringA': (SAY, 0x6fd1e0, 1, 1, 0)})
OBJECT, COM, COMVT, CHAT, RECORDS, SAVES, GET3A4, TEXT, TEXT2, PATHSOBJ, TABLE = RAM, RAM + 0x40, RAM + 0x80, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x480, RAM + 0x500, RAM + 0x580, RAM + 0x5c0
ONLINE, ANSWERS, WORLD_KIND = RAM + 0x1000, RAM + 0x2000, RAM + 0x3000
WORLD = WORLD_KIND - 0x1da738
TEXTS = [b'', b'park', b'a longer name of a park']
BEHIND = [-5, 0, 1, 30, 31, 32, 62, 63, 100, 248, 640, 0x1f4, 0x1f5, 2000, 0x7d1]
GRAB_ANSWERS = {'GetFileAttributesA': [0x10, 0x10, 0x10, 0x11, 0xffffffff, 0x20], 'ReadFile': [1, 1, 0], 'IsDBCSLeadByte': [0, 0, 1], 'InterlockedDecrement': [0, 1, 1, 2], 'InterlockedIncrement': [1, 2],
                'CreateSemaphoreA': [0, 0x44, 0x44], 'WaitForSingleObject': [0, 0, 0x102], 'GetCommandLineA': [0], 'GetFileSize': [0, 0x58, 0x1000]}


def dwords(*values):
    return struct.pack('<%dI' % len(values), *[v & 0xffffffff for v in values])


def case(r):
    state = r.deal(STATES)
    log = log_region()
    log_fold(log)
    grab = state == 10 and r.random() < 0.15
    mode = r.choice([0, 1, 1, 2, 3])
    regions = {LOG: log, IMPORT_TABLE: import_table(log, IMPORTS)}

    # The clock and the fixed steps.
    now = r.randrange(0x10000, 0x7fff0000)
    set_return(log, K[0x402d70], now)
    regions[0x878c74] = struct.pack('<i', now - r.choice(BEHIND))
    regions[0x875028] = dwords(r.getrandbits(32), r.choice([0, -1, 1, 0x12c, r.getrandbits(32)]), r.getrandbits(32), r.getrandbits(32))
    regions[0x877d34] = dwords(r.choice([0, 1, 6, 7, 8, 14, 15, 30, 31, 62, 63, r.getrandbits(32)]))
    regions[0x877eb8] = r.bytes(0xa0)
    regions[0x878088] = r.bytes(0xa0) + dwords(1 if grab else 0)
    regions[0x878688] = r.bytes(0x14)
    for at in (0x8785dc, 0x8786bc, 0x8786c0, 0x8786dc, 0x87879c, 0x878a1c, 0x7858c8, 0x78d908, 0x8bcbc8, 0x8bd508, 0x786ba4):
        regions[at] = r.bytes(4)
    regions[0x878e84] = bytes([r.choice([0, 0, 1, 2])])
    regions[0x879064] = dwords(r.choice([0, 1, 2, 3, 4]), r.choice([1, 1, 1, 1, 1, 1, 1, 0]), state, r.choice([0, 0, 0, 1]), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32),
                               r.choice([0, 0, 0, 0, 1, 2, 3, 4, -1]))
    regions[0x78d8e0] = dwords(r.choice([0, 1, 2]))
    flags, online_flags = r.choice([0, 8, 2, r.getrandbits(32)]), r.choice([0, 0x200, 1, 0x201, r.getrandbits(32)])
    regions[0x7a1a14] = dwords(flags)
    regions[0x7b05cc] = dwords(r.getrandbits(32), r.choice([0, 0, 1]))
    regions[0x7b05e8] = dwords(r.choice([0, RAM + 0x4100]))
    regions[0x788268] = dwords(r.choice([0, 0, 1]))
    # The camera: (0x7909c0 its place, 0x790ab8 where it was), a flag.
    camera = bytearray(r.bytes(0x2c))
    was = bytearray(r.bytes(0x14))
    put(was, 0, 'I', r.choice([0, 0, 2, 4, 0x10, 1, r.getrandbits(32)]))
    for at in (0x20, 0x24, 0x28):
        put(camera, at, 'I', fb(r.uniform(-1000, 1000)) if r.random() < 0.9 else r.f())
    for at in (8, 0xc, 0x10):
        put(was, at, 'I', fb(r.uniform(-1000, 1000)) if r.random() < 0.9 else r.f())
    regions[0x7909a0], regions[0x790ab0] = camera, was
    regions[0xfb34a8] = bytes([r.choice([1, 1, 1, 0, 3, 0xfe])])
    regions[0xfb3b7c] = dwords(mode)
    # Pointers only passed on.
    for at, value in ((0x8c4b44, RAM + 0x4000), (0x87a6d0, RAM + 0x4010), (0x879d54, RAM + 0x4020), (0x7cf83c, WORLD), (0x7cf6ec, TABLE), (0x879a24, COM)):
        regions[at] = dwords(value)
    regions[TABLE] = dwords(RAM + 0x4030)
    regions[WORLD_KIND] = dwords(r.choice([4, 3, 0, 1]))

    # The keys and the timers.
    start = r.getrandbits(32)
    ticks = [r.getrandbits(32) for _ in range(40)]
    keys = [r.choice([0, 0, 0, 0x8000, 0x10000, 1]) for _ in range(40)]
    times = [start, start + 5, start + 0x493e0, start + 0x493e1] + [start + 0x493e1] * 36
    regions[ANSWERS] = dwords(*keys, *ticks, *times)
    answers(log, KEY, ANSWERS)
    answers(log, TICKS, ANSWERS + 0xa0)
    answers(log, K[0x5f5fa0], ANSWERS + 0x140)

    # The objects.
    regions[OBJECT] = bytearray(r.bytes(0x18))
    put(regions[OBJECT], 0x14, 'I', r.choice([1, 1, 0, 2]))
    chat = bytearray(r.bytes(0x30))
    chat[0x28] = r.choice([0, 1, 1, 2])
    online = bytearray(r.bytes(0x400))
    put(online, 0x3f0, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    put(online, 0xf8, 'I', RAM + 0x4040)
    records = bytearray(r.bytes(0x70))
    put(records, 0x60, 'i', r.choice([-1, -1, 0, 1]))
    regions[SAVES] = r.bytes(0x50)
    regions[COM] = dwords(*[r.getrandbits(32) for _ in range(5)], COM + 0x20)
    regions[COM + 0x20] = dwords(COMVT)
    regions[COMVT] = dwords(recorder(QUERY, 3, True))
    strings(log, QUERY, 0, 4)
    fill(log, QUERY, 2, dwords(r.getrandbits(32)))
    regions[GET3A4] = r.bytes(0x50)
    regions[TEXT] = r.choice(TEXTS) + b'\0'
    regions[TEXT2] = r.choice(TEXTS) + b'\0'
    regions[PATHSOBJ] = dwords(0, 0, TEXT)
    for ctor, value in ((0x4e5260, CHAT), (0x5b50b0, ONLINE), (0x5c7420, RECORDS), (0x5aef50, SAVES), (0x5d5770, OBJECT)):
        set_return(log, K[ctor], value)
    regions[0x7cddec] = dwords(CHAT if r.random() < 0.6 else 0)
    regions[0xf7d88c] = dwords(ONLINE if r.random() < 0.6 else 0)
    regions[0xf81ebc] = dwords(RECORDS if r.random() < 0.6 else 0)
    regions[0xf7c58c] = dwords(SAVES if r.random() < 0.6 else 0)
    regions[0xf82884] = dwords(OBJECT)
    regions[0x788d3c] = dwords(r.choice([0, RAM + 0x4050]))

    # Each decision's answer.
    set_return(log, K[0x619240], PATHSOBJ)
    set_return(log, K[0x5b96b0], GET3A4)
    set_return(log, K[0x5bd270], TEXT)
    set_return(log, K[0x40ef40], TEXT2)
    set_return(log, K[0x6595a8], r.choice([0, 0, 0, 0x10000, 1, 0x8000]))
    set_return(log, K[0x51b300], r.choice([0, 0, 1]))
    set_return(log, K[0x5c7d00], r.choice([0, 1]))
    loads = r.random() < 0.7
    for call in (0x457a90, 0x407d80, 0x407e00, 0x457c30):
        set_return(log, K[call], 1 if loads or r.random() < 0.5 else 0)
    set_return(log, K[0x5afc40], r.choice([0x100, 0, 1, 0xff00ff01]))
    set_return(log, K[0x5b0cb0], r.choice([0x100, 0, 1, 0xff00ff01]))
    set_return(log, K[0x5d5cf0], r.choice([2, 2, 0, 1, 3, 0x102]))
    set_return(log, K[0x4c81e0], r.choice([0, 1, 0x58, 0x59, 0x5a, 0x7fffffff, -1 & 0xffffffff]))
    set_return(log, K[0x4c8d30], r.choice([0, 1, 0x63, 0x64, 0x65, 0x7fffffff, -1 & 0xffffffff]))
    # The chat's wait must end: by the 300 s (in the short path: the log still counting) or by the chat.
    ended = r.choice([0x100, 0, 1, 0xff00ff01])
    if state == 9 and mode == 1:
        if r.random() < 0.3:
            put(online, 0x3f0, 'I', 0)
            online_flags |= 0x200
            set_return(log, K[0x457a90], 0)
            regions[0x7b05cc] = regions[0x7b05cc][:4] + dwords(0)
            chat[0x28] = 0
            ended = 0x100
        elif chat[0x28] != 1 and not ended & 0xff:
            ended = r.choice([1, 0xff00ff01])
    set_return(log, K[0x4e6150], ended)
    regions[0x7a1a8c] = dwords(online_flags)
    regions[CHAT], regions[RECORDS], regions[ONLINE] = chat, records, online

    # What it passes by address on the stack.
    strings(log, K[0x429ba0], 1)
    blob(log, K[0x40fb10], 0, 7)
    blob(log, K[0x5c5580], 0, 0x1f)
    blob(log, K[0x409480], 0, 0x1f)
    blob(log, K[0x4088a0], 0, 0x1f)
    blob(log, K[0x5c7f40], 1, 0x1f)
    blob(log, K[0x414d40], 1, 0)   # (the save's buffer, not set)
    struct.pack_into('<I', log, LOG_BYTES + 4 * K[0x4c8d30], 1 << 17)   # (its second argument a word: 0x7b05cc's over a stale high word)

    # The grab's name: screengrab.py's fixture.
    size = 0x4000
    if grab:
        size = 0x10000
        for imp, k in fileopen.K.items():
            if imp != 'CreateFileA' and r.random() < 0.9:
                set_return(log, k, r.choice(GRAB_ANSWERS.get(imp, [0, 1, r.getrandbits(32)])))
        fail_after(log, fileopen.K['CreateFileA'], r.choice([0, 1, 1, 2, 3]))
        base = r.choice([b'C:\\tp\\', b'D:\\'])
        names = bytearray(r.bytes(0x318))
        names[0:8] = b'tp.exe\0\0'
        names[0x108:0x109 + len(base)] = base + b'\0'
        regions[fileopen.GLOBALS] = names
        regions[fileopen.PATHS] = r.bytes(0x210)
        regions[fileopen.LOCALE] = bytes(4)
        regions[0x8798d0] = r.bytes(0x100)
    # (0x38 for the object in state 1, a singleton elsewhere: a null one faults on its next use in both)
    fail = 1 if (state == 1 and r.random() < 0.2) or (state in (9, 11, 12) and r.random() < 0.03) else 0
    regions[HEAP] = heap(size, fail=fail)
    return Call(0x54e360, [], regions, ret='a')


if __name__ == '__main__':
    if '--table' in sys.argv:
        for addr, (kind, n, k) in sorted(CALLS.items()):
            print('    {0x%06x, %s(%d, %d)},' % (addr, {'T': 'recorder', 'C': 'jpeg_recorder', 'S': 'stdcall_recorder'}[kind], k, n))
        sys.exit(0)
    B.main(case)
