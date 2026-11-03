# The QSound mixer source's calls on the QMixer library (src/game/core/qsoundmixer: 0x6d2110-0x6d2d20) against the originals. The library's imports recorders answering 0 mostly; the source a recorder vtable (slot 2 "open", 25, 26, 33);
# the vectors with NaNs now and then; stack vectors and wave structures logged by their contents. The open runs the real config, flags and device (as soundsource.py), the frequency tables random floats.
# usage: qsoundmixer.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, answers, blob, blob_array, blob_bytes, echo, fb, fill, heap, import_table, log_region, recorder, set_return, strings, vtable  # noqa: E402

B = Bench([(0x6d2110, 0x6d21c8), (0x6d2220, 0x6d2d5d), (0x6d1f40, 0x6d2093), (0x6d20a0, 0x6d2110), (0x6c4040, 0x6c418b), (0x6c4300, 0x6c4410), (0x6c4500, 0x6c4541), (0x6d3a80, 0x6d3b04), (0x6d6c70, 0x6d6cf0),
           (0x6d6db0, 0x6d6dc3), (0x67b410, 0x67b440), (0x6bb220, 0x6bb237), (0x6b9d50, 0x6b9d63)])
INITEX, ACTIVATE, OPENCH, LISTPOS, OPTIONS, BUFFER, CLOSE, PANRATE, ENABLE, CONFIG, OPENWAVE, FREE, PLAY, STOP, PAUSE, RESTART, GETPOS, SRCPOS, SETPOS, VELOCITY, FREQ, CONE, MAPPING, VOLUME, ORIENT, SPEED, SPEAKER, DSCREATE = range(32, 60)
IMPORTS = {'InitEx': (INITEX, 0x6fd2ec, 1, 0, 0), 'Activate': (ACTIVATE, 0x6fd2f0, 2, 0, 0), 'OpenChannel': (OPENCH, 0x6fd2fc, 3, 0, 0), 'ListenerPosition': (LISTPOS, 0x6fd2f8, 3, 0, 0), 'Options': (OPTIONS, 0x6fd2f4, 3, 0, 0),
           'Buffer': (BUFFER, 0x6fd2e8, 3, 0, 0), 'Close': (CLOSE, 0x6fd2e4, 1, 0, 0), 'PanRate': (PANRATE, 0x6fd2dc, 4, 0, 0), 'Enable': (ENABLE, 0x6fd2e0, 4, 0, 0), 'Configure': (CONFIG, 0x6fd2cc, 5, 0, 0),
           'OpenWave': (OPENWAVE, 0x6fd2d0, 3, 0, 0), 'Free': (FREE, 0x6fd2d4, 2, 0, 0), 'Play': (PLAY, 0x6fd2d8, 6, 0, 0), 'Stop': (STOP, 0x6fd2c8, 3, 0, 0), 'Pause': (PAUSE, 0x6fd2c4, 3, 0, 0), 'Restart': (RESTART, 0x6fd2c0, 3, 0, 0),
           'GetPosition': (GETPOS, 0x6fd2bc, 3, 0, 0b100), 'SourcePosition': (SRCPOS, 0x6fd2b4, 4, 0, 0), 'Position': (SETPOS, 0x6fd2b8, 4, 0, 0), 'Velocity': (VELOCITY, 0x6fd2b0, 3, 0, 0), 'Frequency': (FREQ, 0x6fd2ac, 4, 0, 0),
           'Cone': (CONE, 0x6fd2a8, 6, 0, 0), 'Mapping': (MAPPING, 0x6fd2a4, 4, 0, 0), 'Volume': (VOLUME, 0x6fd2a0, 4, 0, 0), 'Orientation': (ORIENT, 0x6fd298, 4, 0, 0), 'Speed': (SPEED, 0x6fd29c, 3, 0, 0), 'Speaker': (SPEAKER, 0x6fd294, 2, 0, 0),
           'DirectSoundCreate': (DSCREATE, 0x6fd028, 3, 0, 0b10)}
OPS_TABLE = {'open': 0x6d2110, 'options': 0x6d2220, 'buffer': 0x6d2260, 'close': 0x6d22b0, 'enable': 0x6d2320, 'play': 0x6d2380, 'stopch': 0x6d24c0, 'pausech': 0x6d2500, 'restartch': 0x6d2540,
             'stopall': 0x6d2580, 'pauseall': 0x6d25a0, 'restartall': 0x6d25c0, 'getpos': 0x6d25e0, 'setpos': 0x6d2640, 'velocity': 0x6d27b0, 'freqstep': 0x6d2820, 'freq': 0x6d28a0, 'cone': 0x6d28d0, 'mapping': 0x6d2930,
             'volume': 0x6d2980, 'deldtor': 0x6d29f0, 'listpos': 0x6d2a10, 'orient': 0x6d2b20, 'panall': 0x6d2ca0, 'speed': 0x6d2ce0, 'speaker': 0x6d2d20}
SRC, VT, OUT, V1, V2, ARR, INFO, OBJ, OBJ_VT, DS, DS_VT, ANSWERS = (RAM + 0x200 * i for i in range(12))
HELPER, DEVICE_GUARD, STATICS = 0xfb1f04, 0xfb2028, 0xfb201c
OPS = [n for n in OPS_TABLE if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def floats(r, n=3, nan=0.15):
    return struct.pack('<%dI' % n, *[r.choice([0x7fc00000, 0x7f800001, 0xffa00000, 0x7fbfffff, 0xffc12345]) if r.random() < nan else r.choice([fb(r.uniform(-8, 8)), 0, 0x80000000, 0x7f800000, 0xff800000, r.getrandbits(30) | 1 << 30])
                                     for _ in range(n)])


def case(r):
    name = r.deal(OPS)
    at = OPS_TABLE[name]
    log = log_region()
    table = import_table(log, IMPORTS)
    for k in range(INITEX, DSCREATE):
        set_return(log, k, r.choice([0, 0, 0, 0, 1, r.getrandbits(32)]))
    source = bytearray(r.bytes(0x40))
    struct.pack_into('<IIIII', source, 0, VT, r.choice([1, 2, 8, 0x40, r.getrandbits(32)]), r.getrandbits(32), r.choice([0x2b11, 0x5622, r.getrandbits(32)]), r.getrandbits(32))
    struct.pack_into('<III', source, 0x18, r.choice([0, 0x1234, 0x1234, 0x1234, r.getrandbits(32)]), ARR, 0)
    vt = vtable({25: 2, 26: 3, 27: 3})
    vt = bytearray(vt) + bytes(8)
    struct.pack_into('<I', vt, 33 * 4, recorder(27, 3))
    regions = {LOG: log, IMPORT_TABLE: table, SRC: source, VT: vt, OUT: r.bytes(4), V1: floats(r, 5), V2: floats(r, 5), ARR: bytes(struct.pack('<64I', *[r.choice([0, 0, r.getrandbits(32) | 1]) for _ in range(64)])),
               ANSWERS: struct.pack('<40I', *[r.choice([1, 1, 1, 1, 1, 0, r.getrandbits(32)]) for _ in range(40)])}
    answers(log, 2, ANSWERS)
    for which, k in enumerate((26, 27)):   # (their out on the stack, a status written through it, and answered: its address)
        strings(log, k, 0, out=1)
        echo(log, k, 0)
        fill(log, k, 0, struct.pack('<I', r.choice([0, 0, 0, 1, 0xffffffff, 0x80000000])), which=which)
    channel = r.choice([0, 1, 2, 7, 8, r.randrange(0x40), struct.unpack_from('<I', source, 4)[0], r.getrandbits(32)])
    args = {'close': []}
    conv_out = {'buffer': False, 'close': False, 'deldtor': False}
    if name == 'open':
        return open_case(r, log, regions, source)
    if name == 'options':
        args[name] = [r.getrandbits(32) & r.choice([0xe, 0xffffffff])]
    elif name == 'buffer':
        fill(log, BUFFER, 2, r.bytes(4))
        args[name] = [r.getrandbits(32), OUT]
    elif name == 'enable':
        args[name] = [channel, r.getrandbits(32) & r.choice([0x400, 0x145d, 0xffffffff])]
    elif name == 'play':
        return play_case(r, log, regions, source)
    elif name in ('stopch', 'pausech', 'restartch', 'panall', 'speed'):
        args[name] = [channel]
    elif name == 'freq':
        args[name] = [channel, r.getrandbits(32)]
    elif name == 'mapping':
        blob(log, MAPPING, 3, 0xf)
        args[name] = [channel, V1]
    elif name == 'cone':
        blob(log, CONE, 3, 7)
        args[name] = [channel, V1]
    elif name in ('stopall', 'pauseall', 'restartall'):
        args[name] = []
    elif name == 'getpos':
        fill(log, GETPOS, 2, r.bytes(12))
        args[name] = [channel, V2]
    elif name == 'setpos':
        blob(log, SRCPOS, 3, 7)
        blob(log, SETPOS, 3, 7)
        args[name] = [channel, V1, r.choice([0, 1, r.getrandbits(32)])]
    elif name == 'velocity':
        blob(log, VELOCITY, 1, 7)
        args[name] = [V1, V2]
    elif name == 'freqstep':
        step = r.choice([0, 0, r.randint(-0x40, -1), r.randint(1, 0x40)])
        factors = [fb(r.choice([1.0, 0.5, 2.0, r.uniform(0.25, 4), r.uniform(0, 1e6)])) for _ in range(0x41)]
        regions[0x782f40], regions[0x783540] = struct.pack('<65I', *factors), struct.pack('<65I', *reversed(factors))
        args[name] = [r.choice([22050, 11025, 44100, r.getrandbits(16), r.getrandbits(32)]), channel, step & 0xffffffff]
    elif name == 'volume':
        regions[0x782e40] = r.bytes(0x100)
        args[name] = [channel, r.choice([0, 0x7f, 0x80, r.randrange(0x80), r.randrange(0x80), 0xffffffff, r.getrandbits(32)])]
    elif name == 'deldtor':
        regions[HEAP] = heap(0x40)
        regions[HELPER] = struct.pack('<I', 0)
        return Call(at, [HEAP + 0x10, r.choice([0, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')
    elif name == 'listpos':
        blob(log, LISTPOS, 1, 7)
        args[name] = [V2, V1, r.choice([0, 0, 1, r.getrandbits(32)]), r.choice([0, 1, r.getrandbits(32)])]
    elif name == 'orient':
        struct.pack_into('<I', source, 0x18, 1)
        blob_array(log, ORIENT, 1, 0, 12)
        blob_array(log, ORIENT, 2, 0, 12, which=1)
        args[name] = [V1, V2]
    elif name == 'speaker':
        regions[OBJ] = struct.pack('<I', OBJ_VT)
        regions[OBJ_VT] = struct.pack('<I', recorder(28, 0))
        set_return(log, 28, r.getrandbits(32))
        args[name] = [OBJ]
    with_out = conv_out.get(name, True)
    ret = 'v' if name == 'close' else 'a'
    return Call(at, [SRC] + ([OUT] if with_out else []) + args[name], regions, conv='t', ret=ret)


def device(r, log, regions):
    caps = bytearray(r.bytes(0x40))
    struct.pack_into('<I', caps, 0x30, r.choice([0, 0, 1, 4, 30, r.getrandbits(32)]))
    fill(log, 4, 1, bytes(caps), which=1, offset=4)
    fill(log, DSCREATE, 1, struct.pack('<I', DS))
    for k in (DSCREATE, 4):
        set_return(log, k, r.choice([0, 0, 0, 1, 0x88780078]))
    for k in (1, 2):
        set_return(log, k, r.getrandbits(32))
    regions[STATICS] = struct.pack('<2I', r.choice([0, 0, 1, 4, r.getrandbits(32)]), r.getrandbits(32)) + bytes([r.choice([0, 1, 1, r.getrandbits(8)])]) + r.bytes(3)
    regions[DS] = struct.pack('<I', DS_VT) + struct.pack('<I', 0) + r.bytes(8)
    regions[DS_VT] = vtable({}, com={1: 1, 2: 1, 3: 4, 4: 2})
    regions[HEAP] = heap(0x400)
    regions[DEVICE_GUARD] = bytes([r.choice([1, 3, 0xff])]) + r.bytes(7) + struct.pack('<2I', DS, 0)
    strings(log, 3, 0, out=0b10)   # (the primary's description on the stack)
    blob(log, 3, 1, 7)


def open_case(r, log, regions, source):
    device(r, log, regions)
    set_return(log, INITEX, r.choice([0x5678, 0x5678, 0x5678, 0x5678, 0, r.getrandbits(32)]))
    strings(log, 25, 0, out=1)
    blob(log, 25, 1, 1)
    blob(log, LISTPOS, 1, 7)
    struct.pack_into('<I', source, 0x20, 0 if r.random() < 0.85 else r.getrandbits(32))
    info = bytearray(r.bytes(0x70))
    struct.pack_into('<2I', info, 0x40, *[r.choice([0, 1, 2, 3, 0x10, 0x40]) for _ in range(2)])
    struct.pack_into('<I', info, 0x68, r.choice([0, 1, r.getrandbits(32)]))
    regions[INFO] = info
    return Call(0x6d2110, [SRC, OUT, INFO], regions, conv='t', ret='a')


def play_case(r, log, regions, source):
    info = bytearray(r.bytes(0x70))
    shift = r.choice([0, 0, 1, 2, 33])
    struct.pack_into('<I', info, 0x2c, r.randrange(0x40) << (shift & 31))
    struct.pack_into('<I', info, 0x20, r.choice([0, r.getrandbits(32)]))
    regions[INFO] = info
    regions[0x782db8] = bytes([shift])
    set_return(log, OPENWAVE, r.choice([0x9000, 0x9000, 0x9000, 0, r.getrandbits(32)]))
    blob_bytes(log, CONFIG, 3, 0x3ffff)
    blob_bytes(log, OPENWAVE, 1, 0xffff0, (0, 0x3ffff))
    return Call(0x6d2380, [SRC, OUT, INFO], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
