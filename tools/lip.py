"""Theme Park World advisor lip-sync (.lip) files and the advisor response table.

Speech/lips/sp_NNN.lip (lips.wad, one per speech sample) is a list of i32 times in
microseconds from the start of the sample, ending with -1. Advisor_UpdateSpeech 0x599880
opens the advisor's mouth when the sample starts and toggles it each time playback passes the
next time (it divides by 1000 and compares in milliseconds); most lists have an odd length, so
the mouth ends closed.

Advisor_Say 0x599050 looks a response ID up in gAdvisorResponses (0x768fb8): 0x20-byte rows,
ending with ID 9999:
  [0] response ID           [1] sample number in the speech sound category
  [2] lip file number (sp_%03d.lip)
  [3] -1, or a gesture (Advisor_Say passes value - 0x10 to the gesture player)
  [4] high 16 bits: 1 = use the theme's speech category and folder, 0 = Data/Global;
      low 16 bits: an animation number
  [5] [6] [7] not traced yet
It starts the sample 200 ms later than the animation, and the mouth follows the .lip times.

Usage:
  python -I tools/lip.py <file.lip | lips.wad>...    print the mouth toggle times
  python -I tools/lip.py --table <tp.exe>             dump the response table
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from wadtool import read_wad  # noqa: E402

RESPONSES = 0x768fb8


def parse_lip(data):
    times = []
    for (t,) in struct.iter_unpack('<i', data[:len(data) & ~3]):
        if t == -1:
            return times
        times.append(t)
    raise ValueError('no -1 terminator')


def read_responses(pe):
    rows, va = [], RESPONSES
    while True:
        row = struct.unpack('<8i', pe.bytes(va, 32))
        if row[0] == 9999:
            return rows
        rows.append(row)
        va += 32


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    if sys.argv[1] == '--table':
        from pe_read import PE
        for r in read_responses(PE(sys.argv[2])):
            print('response %3d: sample %3d lip sp_%03d gesture %3d %s anim %d  %d %d %d' %
                  (r[0], r[1], r[2], r[3], 'theme ' if r[4] >> 16 else 'global', r[4] & 0xffff, r[5], r[6], r[7]))
        return
    for path in sys.argv[1:]:
        items = ([(n, d) for n, d, _ in read_wad(path) if n.lower().endswith('.lip')]
                 if path.lower().endswith('.wad') else [(path, open(path, 'rb').read())])
        for name, data in items:
            print('%s: %s' % (name, ' '.join('%d' % (t // 1000) for t in parse_lip(data))))


if __name__ == '__main__':
    main()
