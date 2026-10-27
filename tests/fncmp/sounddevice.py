# The sound library's DirectSound device and its COM initialisation (src/game/core/sounddevice) against the originals. The imports
# (CoInitialize, CoUninitialize, CoCreateInstance, GetForegroundWindow) are recorders answering 0, a failure or their hash; CoCreateInstance
# fills its out with the IDirectSound at DS (failing first and answering second now and then), whose CreateSoundBuffer (the description
# logged as a hash) fills its out with the primary buffer at PRIM; its SetFormat logs the format as a hash. The device at DEV holds DS /
# PRIM or null; the singletons' guards set or not (0x67a7d0, atexit, logged).
# usage: sounddevice.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _cdecl_log, blob_bytes, fail_mask, fill, log_region, recorder, set_return  # noqa: E402

CALLS = {'cominit': (0x6d3b10, 'a'), 'comdtor': (0x6d3b30, 'v'), 'comget': (0x6d3b40, 'a'), 'comsingle': (0x6d3b80, 'v'),
         'ctor': (0x6c42e0, 'a'), 'create': (0x6c4300, 'a'), 'createof': (0x6c4370, 'a'), 'setformat': (0x6c4390, 'a'),
         'open': (0x6c4410, 'a'), 'release': (0x6c4480, 'v'), 'cocreate': (0x6c44b0, 'a'), 'get': (0x6c4500, 'a')}
B = Bench([(0x6c42e0, 0x6c4541), (0x6d3b10, 0x6d3b8a), (0x6d3a80, 0x6d3b04)], hooks={0x67a7d0: _cdecl_log})
DEV, DS, DS_VT, PRIM, PRIM_VT, OUT = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x600
DEVICE, DEVICE_GUARD, COM, COM_GUARD = 0xfb2030, 0xfb2028, 0xfb271c, 0xfb2720
INIT_K, UNINIT_K, CREATE_K, WINDOW_K = 32, 33, 34, 35
DS_REL_K, BUFFER_K, LEVEL_K, INITIALIZE_K, PRIM_REL_K, FORMAT_K = 36, 37, 38, 39, 40, 41
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def table(slots):
    """A vtable of 16 slots whose slots {slot: (k, arguments)} call COM recorder k (the others recorders 48-63, never called)."""
    return b''.join(struct.pack('<I', recorder(slots[i][0], slots[i][1], True) if i in slots else recorder(48 + i, 0)) for i in range(16))


def case(r):
    name = r.deal(OPS)
    address, ret = CALLS[name]
    log = log_region()
    imports = bytearray(0x460)   # (import_table() covers 0x400 bytes: the ole32 slots are past them)
    for k, at, n in ((INIT_K, 0x6fd44c, 1), (UNINIT_K, 0x6fd450, 0), (CREATE_K, 0x6fd454, 5), (WINDOW_K, 0x6fd370, 0)):
        struct.pack_into('<I', imports, at - IMPORT_TABLE, recorder(k, n, True))
    set_return(log, INIT_K, r.choice([0, 0, 1, 0x80010106, r.getrandbits(32)]))
    how = r.randrange(5)
    if how < 2:
        set_return(log, CREATE_K, [0, 0x80040154][how])
    elif how < 4:
        fail_mask(log, CREATE_K, [1, 3][how - 2])   # (1: the first call answers its hash, the second 0; 3: both their hashes)
    fill(log, CREATE_K, 4, struct.pack('<I', DS), which=0)
    fill(log, BUFFER_K, 2, struct.pack('<I', PRIM), which=1)
    blob_bytes(log, BUFFER_K, 1, 0xfffff)
    blob_bytes(log, FORMAT_K, 1, 0x3ffff)
    for k in (INITIALIZE_K, LEVEL_K, BUFFER_K, FORMAT_K):
        set_return(log, k, r.choice([0, 0, 0, 1, 0x88780032, 0x80000000]))
    for k in (UNINIT_K, WINDOW_K, DS_REL_K, PRIM_REL_K):
        set_return(log, k, r.getrandbits(32))
    device = struct.pack('<II', r.choice([DS, DS, 0]), r.choice([PRIM, PRIM, 0]))
    regions = {LOG: log, IMPORT_TABLE: imports, DEV: device, OUT: r.bytes(4),
               DS: struct.pack('<I', DS_VT) + r.bytes(0xc),
               DS_VT: table({2: (DS_REL_K, 1), 3: (BUFFER_K, 4), 6: (LEVEL_K, 3), 10: (INITIALIZE_K, 2)}),
               PRIM: struct.pack('<I', PRIM_VT) + r.bytes(0xc), PRIM_VT: table({2: (PRIM_REL_K, 1), 14: (FORMAT_K, 2)}),
               DEVICE_GUARD: bytes([r.choice([0, 0, 1, r.getrandbits(8)])]) + r.bytes(7) + device,
               COM: bytes([r.choice([0, 1, r.getrandbits(8)])]) + r.bytes(3) + bytes([r.choice([0, 1, r.getrandbits(8)])]) + r.bytes(3)}
    if name in ('comget', 'comsingle', 'get'):
        args = []
    elif name in ('cominit', 'comdtor'):
        args = [COM]
    elif name in ('ctor', 'release'):
        args = [DEV]
    elif name == 'setformat':
        args = [DEV, OUT, r.choice([1, 2, r.getrandbits(32)]), r.choice([8, 0x10, r.getrandbits(32)]),
                r.choice([22050, 44100, r.getrandbits(16), r.getrandbits(32)])]
    else:
        args = [DEV, OUT]
    return Call(address, args, regions, conv='c' if not args or name in ('comget', 'comsingle', 'get') else 't', ret=ret)


if __name__ == '__main__':
    B.main(case)
