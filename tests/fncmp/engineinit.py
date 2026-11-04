# The engine's parts created and the device for the driver kind (src/game/render/enginestart: 0x572780, 0x5632b0, 0x5836d0, 0x583700) against the originals, the callees recorders (driver.cpp's kEngineStartCalls):
# each driver kind, each part failing or none, the log on or off, the procedural texture's file or none; DirectDraw's object there already, its allocation failing, its open failing.
# usage: engineinit.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Init,Open,Device,Destroy)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, _recorder, global_heap, heap, log_fold, log_region, put, set_return, strings  # noqa: E402

PARTS = [0x5913f0, 0x56b6c0, 0x5781d0, 0x569e10, 0x5666f0, 0x573a30, 0x5821e0, 0x573fd0, 0x5804b0, 0x56ddf0, 0x5760a0, 0x58aff0, 0x561840, 0x58a430, 0x57c360, 0x574a20, 0x57d100, 0x564b20]   # slots 0-17
DOWN = [0x564b60, 0x57c4a0, 0x57d1a0, 0x58a490, 0x561a00, 0x58b040, 0x574b10, 0x5761b0, 0x56de90, 0x58d090, 0x580500, 0x5740e0]   # slots 20-31
TEXT, TEXCACHE, DOME = 18, 19, 23
FAILS = [None, None, None, None, 1, 2, 3, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, TEXCACHE, DOME]
HOOKS = {a: _recorder(k, 0, cdecl=True) for k, a in enumerate(PARTS)}
HOOKS.update({a: _recorder(20 + k, 0, cdecl=True) for k, a in enumerate(DOWN)})
HOOKS.update({0x582280: _recorder(6, 0, cdecl=True), 0x5667a0: _recorder(4, 0, cdecl=True), 0x569ef0: _recorder(3, 0, cdecl=True), 0x573b30: _recorder(5, 0, cdecl=True), 0x578390: _recorder(2, 0, cdecl=True), 0x56b760: _recorder(1, 0, cdecl=True)})
HOOKS.update({0x5645e0: _recorder(TEXT, 1, cdecl=True), 0x58cff0: _recorder(TEXCACHE, 1, cdecl=True), 0x5839f0: _recorder(20, 0), 0x5839a0: _recorder(21, 0), 0x56e550: _recorder(22, 2), 0x585720: _recorder(DOME, 0),
              0x57d920: _recorder(24, 2), 0x583cc0: _recorder(25, 0), 0x5835c0: _recorder(26, 0), 0x5833c0: _recorder(27, 0), 0x584790: _recorder(28, 0),
              0x563460: _recorder(0, 6), 0x5640a0: _recorder(1, 0), 0x572c40: _recorder(20, 1, cdecl=True), 0x573960: _recorder(21, 1, cdecl=True), 0x572d50: _recorder(7, 0, cdecl=True), 0x564650: _cdecl_log})
B = Bench([(0x572780, 0x572b60), (0x5632b0, 0x56339d), (0x5836d0, 0x58371d), (0x56af20, 0x56af9c), (0x56a770, 0x56ae60), (0x67a400, 0x6902c0), (0x5775d0, 0x577600)], hooks=HOOKS)
OPS = os.environ.get('OPS', 'Init,Init,Init,Init,Init,Open,Open,Device,Destroy').split(',')
KINDS = [1, 1, 1, 4, 4, 4, 0, 2, 3, 5]


def flagged(r, bit, p, value):
    return value | bit if r.random() < p else value & ~bit


def init(r):
    log = log_region()
    log_fold(log)
    strings(log, TEXT, 1)
    for k in range(18):
        set_return(log, k, r.choice([1, 1, 1, 2, 0x80000000]))
    set_return(log, TEXCACHE, 1)
    set_return(log, DOME, 1)
    fail = r.deal(FAILS)
    if fail is not None:
        set_return(log, fail, 0)
    flags = flagged(r, 0x1000000, 0.4, flagged(r, 0x2000000, 0.7, r.getrandbits(32)))
    regions = {LOG: log, 0x8bd508: struct.pack('<I', flags), 0x8bd524: struct.pack('<I', r.choice(KINDS)), 0x8bcbc8: struct.pack('<I', flagged(r, 0x200000, 0.5, r.getrandbits(32))),
               0x879ab0: r.bytes(4), 0x879ac0: r.bytes(4), 0x879acc: r.bytes(4), 0x879d60: r.bytes(4), 0x879cd4: r.bytes(4), 0x8bcbac: r.bytes(4)}
    return Call(0x572780, [r.getrandbits(32), r.getrandbits(32), r.choice([0, r.getrandbits(32)])], regions, conv='c', ret='a')


def open_(r):
    log = log_region()
    strings(log, 0x3f, 1)
    set_return(log, 0, r.choice([0, 0, 1, r.getrandbits(32)]))
    errors = put(bytearray(r.bytes(0x204)), 0x200, 'i', r.choice([r.randrange(0x41), 0, 0x3f, 0x40]))
    regions = {LOG: log, RAM: r.bytes(0x10), 0x879a24: struct.pack('<I', RAM if r.random() < 0.15 else 0), HEAP: heap(0x7d0, 1 if r.random() < 0.15 else 0), 0x879ad0: errors,
               0x8bd508: struct.pack('<I', flagged(r, 0x1000000, 0.5, r.getrandbits(32))), 0x8bd518: r.bytes(4), 0x8bd524: struct.pack('<I', r.choice(KINDS))}
    return Call(0x5632b0, [r.getrandbits(32) for _ in range(6)], regions, conv='c', ret='a')


def device(r):
    log = log_region()
    set_return(log, 20, r.choice([0, 1, r.getrandbits(32)]))
    set_return(log, 21, r.choice([0, 1, r.getrandbits(32)]))
    return Call(0x5836d0, [r.getrandbits(32)], {LOG: log, 0x8bd524: struct.pack('<I', r.choice(KINDS))}, conv='c', ret='a')


def destroy(r):
    log = log_region()
    set_return(log, 7, r.choice([0, 1, r.getrandbits(32)]))
    return Call(0x583700, [], {LOG: log, 0x8bd524: struct.pack('<I', r.choice(KINDS))}, conv='c', ret='a')


FUNCS = {'Init': init, 'Open': open_, 'Device': device, 'Destroy': destroy}


def case(r):
    return FUNCS[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)
