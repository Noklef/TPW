# A saved game's stream (src/game/world/gamefile: 0x415270, 0x4164c0) against the originals: the stream a vtable of recorders under the real Stream_Tell, Stream_Read and Stream_Write (the reads sequenced from the tags, one wrong now and
# then, the data short now and then), each part a recorder, most cases with all parts good so that they all run, else one failing; the guards made or not; those given the stream's address logged by the dword there.
# (Thiscall recorders have only slots below 32: a load's take the numbers of cdecl ones only a save calls, and the other way round; a slot logs a cdecl call apart from a thiscall one, and each case sets only its own function's answers.)
# usage: gamefile.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, log_fold, log_region, seq_read, set_return, strings, vtable  # noqa: E402

READ, WRITE, TELL = 2, 3, 6   # (the stream's slots)
LOAD, SAVE = 0x415270, 0x4164c0
# (address, slot, arguments, a good answer is not 0 / is 0 / any, by the stream's address)
CD = {LOAD: [(0x475730, 1, 1, False), (0x51f7a0, 4, 1, False), (0x5200b0, 5, 0, None), (0x5506e0, 7, 1, True), (0x4647a0, 8, 1, True), (0x543560, 9, 1, True), (0x55e020, 10, 1, False), (0x5597a0, 11, 1, False), (0x42cec0, 12, 1, True),
            (0x437b60, 13, 1, True), (0x599d00, 14, 1, None), (0x51c400, 15, 1, None)],
      SAVE: [(0x475650, 16, 1, None), (0x51f680, 17, 2, False), (0x550520, 18, 1, True), (0x464140, 19, 1, True), (0x5428e0, 20, 1, True), (0x55de70, 21, 1, False), (0x559350, 22, 1, False), (0x42cdc0, 23, 1, True), (0x437300, 24, 1, True),
            (0x599c30, 25, 1, None), (0x51c350, 26, 1, None)]}
ATEXIT, INSTANCE_OF = (0x67a7d0, 27, 1), (0x415e00, 28, 0)
TH = {LOAD: [(0x5179c0, 16, 1, True, False), (0x40fd60, 17, 1, False, True), (0x402e30, 18, 1, True, False), (0x403260, 19, 1, True, False), (0x405570, 20, 1, True, True), (0x59c860, 21, 1, True, True), (0x481680, 22, 1, None, False)],
      SAVE: [(0x516c80, 1, 1, True, False), (0x40fc80, 4, 1, False, True), (0x402e00, 7, 1, True, False), (0x403220, 8, 1, True, False), (0x4055d0, 9, 1, True, True), (0x59c890, 10, 1, True, True), (0x4816a0, 11, 1, None, False)]}
ACTIONS = (0x403780, 29, 4, False, False)
MADE = [(0x4053b0, 30, 0, None, False), (0x59bea0, 31, 0, None, False)]
HOOKS = {a: _recorder(k, n, cdecl=True) for a, k, n, *_ in CD[LOAD] + CD[SAVE] + [ATEXIT, INSTANCE_OF]}
HOOKS.update({a: _recorder(k, n) for a, k, n, *_ in TH[LOAD] + TH[SAVE] + [ACTIONS] + MADE})
B = Bench([(0x415270, 0x415dfb), (0x4164c0, 0x416e0e), (0x5f5bb0, 0x5f5bc0), (0x5f5e20, 0x5f5e80), (0x5da3c0, 0x5da3c1)], hooks=HOOKS)
STREAM, VT, DATA, CENTRE = RAM, RAM + 0x80, RAM + 0x200, RAM + 0x400
TAGS = [b'DLRW', b'CSPS', b'TRAP', b'SSEM', b'KOLC', b'TNAV', b'SYSG', b'SYSR', b'KART', b'RYLF', b'ESSR', b'EMAK', b'SAOC', b'SVDA', b'NUOS', b'STHC', b'CSDA']


def case(r):
    log = log_region()
    log_fold(log)
    op = r.choice([LOAD, SAVE])
    rows = CD[op] + TH[op] + [ACTIONS] + MADE
    fail = r.choice([None] * 3 + [row for row in rows if row[3] is not None])
    for row in rows:
        k, good = row[1], row[3]
        if good is not None:
            set_return(log, k, (1 if row is not fail else 0) if good else (0 if row is not fail else r.choice([1, -1, r.getrandbits(32)])))
        else:
            set_return(log, k, r.getrandbits(32))
        if len(row) == 5 and row[4]:
            blob(log, k, 0, 1)
    set_return(log, INSTANCE_OF[1], CENTRE)
    set_return(log, TELL, r.getrandbits(32))
    set_return(log, WRITE, r.choice([4, 4, 4, 4, 0, -1]))
    tags = list(TAGS)
    if r.randrange(4) == 0:
        tags[r.randrange(len(tags))] = r.bytes(4)
    data = b''.join(tags)
    length = len(data) if r.randrange(5) else r.randrange(len(data))
    seq_read(log, READ, DATA, length, buffer=0, count=1)
    strings(log, READ, 0, 1)   # (the buffer: a stack slot)
    regions = {LOG: log, STREAM: struct.pack('<I', VT) + r.bytes(0x20), VT: vtable({READ: 2, WRITE: 2, TELL: 0}), DATA: data, CENTRE: r.bytes(4),
               0x7cf83c: struct.pack('<I', r.getrandbits(32)), 0xfb3518: bytes([r.getrandbits(8), r.choice([0, 1, 0xfe, 0xff]), r.getrandbits(8), r.getrandbits(8)]), 0xfb352c: bytes([r.choice([0, 1, 0xfe, 0xff])]) + r.bytes(3)}
    mode = r.choice([0, 0, 0, 0, 2, 1, r.getrandbits(32)])
    return Call(op, [r.getrandbits(32), STREAM, mode], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
