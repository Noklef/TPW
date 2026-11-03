# The sound mixer's thread (src/game/core/mixerthread: 0x6b63a0) against the original: the sleep, the signal, the clock, the voices' methods recorders. It ends only when the stop byte +0x35 is set: an audible voice's method 0x14 sets it, through the time (the clock answers that byte's address), so a pass always has one.
# usage: mixerthread.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, fill, import_table, log_region, recorder, set_return, strings  # noqa: E402

NOW, AUDIBLE, SIGNAL, NAP, METHOD, RELEASE = range(1, 7)
B = Bench([(0x6b63a0, 0x6b6436)], hooks={0x5f5fa0: _recorder(NOW, 0, cdecl=True), 0x6bca10: _recorder(AUDIBLE, 0)})
MIXER = RAM + 0x1000
ARRAY = RAM + 0x1100
VOICES = RAM + 0x2000
VOICE_VT = RAM + 0x1200
OWNED_VT = RAM + 0x1300
SIGNALS = RAM + 0x1400
HEARD = RAM + 0x1500


def case(r):
    log = log_region()
    table = import_table(log, {'Signal': (SIGNAL, 0x6fd200, 2, 0, 0), 'Nap': (NAP, 0x6fd1d4, 1, 0, 0)})
    mixer = bytearray(r.randbytes(0x60))
    mixer[0x35] = r.choice([0, 0, 0, 1, r.getrandbits(8)])
    struct.pack_into('<I', mixer, 0x40, ARRAY)
    count = r.randrange(1, 6)
    struct.pack_into('<I', mixer, 0x4c, count)
    set_return(log, NOW, MIXER + 0x35)
    fill(log, METHOD, 1, b'\1')
    strings(log, METHOD, 0, out=1)
    answers(log, SIGNAL, SIGNALS)
    answers(log, AUDIBLE, HEARD)
    pointers = [r.choice([0, 1, 1, 1]) for _ in range(count)]
    pointers[r.randrange(count)] = 1
    heard = [r.choice([0, 0x100, 1, 0x101, 0xff]) for _ in range(sum(pointers))]
    heard[r.randrange(len(heard))] = 1
    regions = {LOG: log, IMPORT_TABLE: table, MIXER: mixer, VOICE_VT: struct.pack('<6I', 0, 0, 0, 0, 0, recorder(METHOD, 2)), OWNED_VT: struct.pack('<I', recorder(RELEASE, 0)),
               SIGNALS: struct.pack('<40I', *[r.choice([1, 1, 0, 2, r.getrandbits(32)]) if i < 6 else 1 for i in range(40)]), HEARD: struct.pack('<40I', *(heard + [1] * 40)[:40])}
    array = bytearray(4 * count)
    for i, used in enumerate(pointers):
        if used:
            voice = bytearray(r.randbytes(0x40))
            struct.pack_into('<I', voice, 0, VOICE_VT)
            owned = VOICES + 0x100 * i + 0x80
            struct.pack_into('<I', voice, 0x28, owned)
            regions[VOICES + 0x100 * i] = voice
            regions[owned] = struct.pack('<I', OWNED_VT)
            struct.pack_into('<I', array, 4 * i, VOICES + 0x100 * i)
    regions[ARRAY] = array
    return Call(0x6b63a0, [MIXER], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
