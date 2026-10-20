# The render state setters (src/game/render/state: 0x4080e0, 0x5836b0, 0x5d9680-0x5d96c0) against the
# originals: flags words in any state, fog settings of any bits; and 0x583720's call through the object at 0x8bd52c (a recorder vtable, any result).
# The display's restore (0x583730): the focus (GetFocus, a recorder) the game's window (0x8bd518) or not, the device's slot 1 any result,
# the error list as engineerrors.py's lost_list(). The ambient colour's setter (0x5d9230): on (any dword, 0 a third of the time) with a 0xRRGGBB dword of any bits, or off.
# usage: renderstate.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Touch,FogOn,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, RAM, log_region, put, recorder, set_return, vtable  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import engineerrors  # noqa: E402

B = Bench([(0x44dd50, 0x44de16), (0x423b50, 0x423b58), (0x4080e0, 0x4081e0), (0x52fb10, 0x52fb20), (0x52fb30, 0x52fb40), (0x52fb50, 0x52fb60), (0x52fb70, 0x52fb80), (0x5836b0, 0x5836c2), (0x583720, 0x58372a), (0x5d9680, 0x5d96c4), (0x5d96c0, 0x5d974f), (0x5d9230, 0x5d92d8), (0x583730, 0x583786), (0x56afc0, 0x56b02a)])
CALLS = ['Device0', 'Touch', 'FogOn', 'FogOff', 'SetFog', 'FadeFog', 'Apply', 'Options', 'Ambient', 'Restore']


def case(r):
    op = r.deal(OPS)
    regions = {RAM: r.bytes(0x10), 0x8bcbc0: r.bytes(0x10), 0x8bcc40: r.bytes(8), 0x8bd470: r.bytes(0x30)}
    if op == 'Options':
        flag = lambda: r.choice([0, 0xffffffff, r.getrandbits(32), r.getrandbits(32) & ~0x40, r.getrandbits(32) | 0x40])
        regions = {0x79d860: put(bytearray(r.bytes(0x20)), 0xc, 'I', flag()), 0x7a1a80: put(bytearray(r.bytes(0x20)), 0xc, 'I', flag()),
                   0x8bcbc0: r.bytes(0x20), 0x8bd500: r.bytes(0x60), 0x79d9c0: put(bytearray(r.bytes(0x20)), 8, 'I', r.choice([0, 0, 1, 7, 0xfff])),
                   0x8c1620: r.bytes(8), 0x78d8d0: r.bytes(0x10)}
        return Call(0x44dd50, [], regions, ret='a')
    if op == 'Apply':
        values = lambda n: b''.join(struct.pack('<I', r.choice([0, 1, 0xffffffff, 0x80000000, 5, r.getrandbits(32)])) for _ in range(n))
        regions = {0x7855f0: values(8), 0x7856d0: values(0x18), 0x8bd470: r.bytes(0x20), 0x7a0b30: r.bytes(0x20),
                   0x7a0060: r.bytes(0x20), 0x7a01b8: r.bytes(0x20), 0x870ec0: r.bytes(0x20), 0xf79790: r.bytes(0x40),
                   0x81b6d0: r.bytes(0x20), 0x81ea10: r.bytes(0x20), 0x81aeb0: r.bytes(0x20), 0x81b430: r.bytes(0x20)}
        return Call(0x4080e0, [], regions, ret='a')
    if op == 'FadeFog':
        fade = bytearray(r.bytes(0x60))
        for off in (0x44, 0x48, 0x4c, 0x50, 0x54, 0x58):
            put(fade, off, 'i', r.choice([0, 0x10000, -0x10000, 0xff0000, r.randrange(-2**31, 2**31), r.randrange(-0x2000000, 0x2000000)]))
        if r.random() < 0.15:
            put(fade, 0x54, 'i', struct.unpack_from('<i', fade, 0x48)[0])
            put(fade, 0x58, 'i', struct.unpack_from('<i', fade, 0x4c)[0])
        elif r.random() < 0.3:
            put(fade, 0x50, 'ii', *struct.unpack_from('<ii', fade, 0x44)[:2])
            put(fade, 0x58, 'i', struct.unpack_from('<i', fade, 0x4c)[0])
        regions.update({0x785880: bytes(8) + struct.pack('<i', r.choice([0, 1, 2, 5, -1])) + bytes(4),
                        0x78d8d0: bytes(8) + struct.pack('<I', r.choice([1, 1, 0, 2])) + bytes(4), 0x8c2780: r.bytes(0x410)})
        regions[RAM] = fade
        return Call(0x5d96c0, [RAM], regions, conv='t', ret='a')
    if op == 'Device0':
        log = log_region()
        set_return(log, 0, r.getrandbits(32))
        regions = {0x8bd520: put(bytearray(r.bytes(0x10)), 0xc, 'I', RAM + 0x100), RAM + 0x100: struct.pack('<I', RAM + 0x200) + r.bytes(0xc),
                   RAM + 0x200: vtable({}), LOG: log}
        return Call(0x583720, [], regions, ret='a')
    if op == 'Restore':
        log = log_region()
        window = r.getrandbits(32)
        set_return(log, 0, r.choice([window, window, r.getrandbits(32)]))
        set_return(log, 1, r.choice([0, 0, 1, r.getrandbits(32)]))
        regions = {0x8bd510: put(bytearray(r.bytes(0x1c)), 8, 'I', window), 0x8bd52c: struct.pack('<I', RAM + 0x100),
                   RAM + 0x100: struct.pack('<I', RAM + 0x200) + r.bytes(0xc), RAM + 0x200: vtable({}), LOG: log,
                   0x6fd328: struct.pack('<I', recorder(0, 0, True)), 0x879ad0: engineerrors.lost_list(r), 0x8bcc40: r.bytes(8),
                   0x8bd508: r.bytes(4)}
        return Call(0x583730, [], regions, ret='a')
    if op == 'Ambient':   # (thiscall on [0xf83778] in its callers, unused; on, colour)
        regions[0x879dd8] = r.bytes(0x10)
        return Call(0x5d9230, [RAM, r.choice([0, r.getrandbits(32), 1]), r.choice([r.getrandbits(32), r.getrandbits(24), 0xffffff, 0])], regions, conv='t',
                    ret='v')
    if op == 'Touch':
        return Call(0x5836b0, [RAM], regions, conv='t')
    if op == 'SetFog':
        return Call(0x5d96a0, [RAM, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)], regions, conv='t')
    return Call({'FogOn': 0x5d9680, 'FogOff': 0x5d9690}[op], [RAM], regions, conv='t')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
