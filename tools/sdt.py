"""Theme Park World .sdt sound banks: list them and extract every sound to .wav (or the raw .mp2).

Bank layout (little-endian):
  u32 count, then count x u32 absolute entry offsets
  entry: u32 header size (40)
         u32 data size
         char name[16]       e.g. "blnl_error1.mp2" (cut at 16 bytes, so "rj_laypath_2.mp")
         u32 format          low 16 bits = sample rate, then 0x10 = bits per sample,
                             then 0x24 (mono) / 0x25 (stereo)
         u32 0
         u32 pcm bytes       length of the original sound. Decoded audio lags it by the 481-sample
                             delay of the polyphase filterbank and the last frame is padded, so
                             the sound is pcm[481 * channels:][:pcm bytes / 2]
         u32 0
         data size bytes of plain MPEG audio frames
The frames are MPEG-2 (22,050 Hz) Layer I or Layer II, plus a few MPEG-1 Layer I at 44,100 Hz.
Nothing on Windows decodes Layer I out of the box, so this file carries a small pure-Python
Layer I/II decoder (ISO 11172-3 / 13818-3). It is slow but exact enough; the synthesis
window round-trips the standard analysis filterbank at about 84 dB SNR. All 3,310 entries in the
43 shipped banks decode, and every Layer II frame is consumed to within 2 bytes of zero padding.

Usage:
  python -I tools/sdt.py list <bank.sdt>
  python -I tools/sdt.py extract <bank.sdt> <outdir> [--mp2] [name ...]
"""
import math
import os
import struct
import sys
import wave

# --- bank -------------------------------------------------------------------------------------


def read_bank(path):
    d = open(path, 'rb').read()
    count = struct.unpack_from('<I', d)[0]
    entries = []
    for i, off in enumerate(struct.unpack_from('<%dI' % count, d, 4)):
        hsize, dsize = struct.unpack_from('<II', d, off)
        name = d[off + 8:off + 24].split(b'\0')[0].decode('latin-1')
        fmt, _, pcm_bytes, _ = struct.unpack_from('<4I', d, off + 24)
        entries.append({
            'index': i, 'name': name, 'rate': fmt & 0xffff, 'bits': (fmt >> 16) & 0xff,
            'channels': 2 if (fmt >> 24) & 1 else 1, 'pcm_bytes': pcm_bytes,
            'data': d[off + hsize:off + hsize + dsize],
        })
    return entries


# --- MPEG audio Layer I / II decoder ----------------------------------------------------------

BITRATES = {  # (mpeg1, layer) -> kbit/s by index
    (True, 1): [0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448],
    (True, 2): [0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384],
    (False, 1): [0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256],
    (False, 2): [0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160],
}
RATES = {True: [44100, 48000, 32000], False: [22050, 24000, 16000]}

# Half of the synthesis window D[0..256] in units of 2^-16 (the other half mirrors it).
WINDOW_HALF = [
    0, -1, -1, -1, -1, -1, -1, -2, -2, -2, -2, -3, -3, -4, -4, -5, -5, -6, -7, -7, -8, -9, -10,
    -11, -13, -14, -16, -17, -19, -21, -24, -26, -29, -31, -35, -38, -41, -45, -49, -53, -58,
    -63, -68, -73, -79, -85, -91, -97, -104, -111, -117, -125, -132, -139, -147, -154, -161,
    -169, -176, -183, -190, -196, -202, -208, -213, -218, -222, -225, -227, -228, -228, -227,
    -224, -221, -215, -208, -200, -189, -177, -163, -146, -127, -106, -83, -57, -29, 2, 36, 72,
    111, 153, 197, 244, 294, 347, 401, 459, 519, 581, 645, 711, 779, 848, 919, 991, 1064, 1137,
    1210, 1283, 1356, 1428, 1498, 1567, 1634, 1698, 1759, 1817, 1870, 1919, 1962, 2001, 2032,
    2057, 2075, 2085, 2087, 2080, 2063, 2037, 2000, 1952, 1893, 1822, 1739, 1644, 1535, 1414,
    1280, 1131, 970, 794, 605, 402, 185, -45, -288, -545, -814, -1095, -1388, -1692, -2006,
    -2330, -2663, -3004, -3351, -3705, -4063, -4425, -4788, -5153, -5517, -5879, -6237, -6589,
    -6935, -7271, -7597, -7910, -8209, -8491, -8755, -8998, -9219, -9416, -9585, -9727, -9838,
    -9916, -9959, -9966, -9935, -9863, -9750, -9592, -9389, -9139, -8840, -8492, -8092, -7640,
    -7134, -6574, -5959, -5288, -4561, -3776, -2935, -2037, -1082, -70, 998, 2122, 3300, 4533,
    5818, 7154, 8540, 9975, 11455, 12980, 14548, 16155, 17799, 19478, 21189, 22929, 24694,
    26482, 28289, 30112, 31947, 33791, 35640, 37489, 39336, 41176, 43006, 44821, 46617, 48390,
    50137, 51853, 53534, 55178, 56778, 58333, 59838, 61289, 62684, 64019, 65290, 66494, 67629,
    68692, 69679, 70590, 71420, 72169, 72835, 73415, 73908, 74313, 74630, 74856, 74992, 75038,
]
D = [(WINDOW_HALF[i] if i <= 256 else WINDOW_HALF[512 - i]) / 65536.0 * (-1 if (i // 64) & 1 else 1)
     for i in range(512)]
# Matrixing V[i] = sum_k cos((16 + i)(2k + 1)pi/64) S[k], stored per subband k.
N_COLS = [[math.cos((16 + i) * (2 * k + 1) * math.pi / 64) for i in range(64)] for k in range(32)]
FILTERBANK_DELAY = 481  # samples between encoder input and decoder output
SCALEFACTORS = [2.0 * 2.0 ** (-i / 3.0) for i in range(63)] + [0.0]

# MPEG-2 LSF Layer II allocation (ISO 13818-3 table B.1): per subband, the quantisation
# classes by allocation index as (levels, grouped).
_Q4 = [None, (3, True), (5, True), (7, False), (9, True)] + [((1 << b) - 1, False) for b in range(4, 15)]
_Q3 = [None, (3, True), (5, True), (9, True)] + [((1 << b) - 1, False) for b in range(4, 8)]
_Q2 = [None, (3, True), (5, True), (9, True)]
LSF_L2_ALLOC = [(4, _Q4)] * 4 + [(3, _Q3)] * 7 + [(2, _Q2)] * 19


class Bits:
    def __init__(self, data):
        self.v = int.from_bytes(data, 'big')
        self.left = len(data) * 8

    def read(self, n):
        self.left -= n
        if self.left < 0:
            raise ValueError('frame overrun')
        return (self.v >> self.left) & ((1 << n) - 1)


def parse_header(data, pos):
    h = struct.unpack_from('>I', data, pos)[0]
    if h >> 21 != 0x7ff:
        raise ValueError('lost sync at %d' % pos)
    mpeg1 = bool(h >> 19 & 1)
    layer = 4 - (h >> 17 & 3)
    if layer not in (1, 2):
        raise ValueError('layer %d not supported' % layer)
    bitrate = BITRATES[(mpeg1, layer)][h >> 12 & 15] * 1000
    rate = RATES[mpeg1][h >> 10 & 3]
    pad = h >> 9 & 1
    if layer == 1:
        size = (12 * bitrate // rate + pad) * 4
    else:
        size = 144 * bitrate // rate + pad
    mode = h >> 6 & 3
    return {'mpeg1': mpeg1, 'layer': layer, 'rate': rate, 'size': size, 'crc': not (h >> 16 & 1),
            'channels': 1 if mode == 3 else 2,
            'bound': 4 + 4 * (h >> 4 & 3) if mode == 1 else 32}


def dequant(v, levels):
    return (2 * v - (levels - 1)) / levels


def decode_layer1(b, hd):
    nch, bound = hd['channels'], hd['bound']
    alloc = [[0] * 32 for _ in range(nch)]
    for sb in range(32):
        for ch in range(nch):
            if ch == 0 or sb < bound:
                a = b.read(4)
                if a == 15:
                    raise ValueError('bad allocation')
            alloc[ch][sb] = a
    scale = [[0.0] * 32 for _ in range(nch)]
    for sb in range(32):
        for ch in range(nch):
            if alloc[ch][sb]:
                scale[ch][sb] = SCALEFACTORS[b.read(6)]
    out = [[] for _ in range(nch)]
    for _ in range(12):
        s = [[0.0] * 32 for _ in range(nch)]
        for sb in range(32):
            for ch in range(nch):
                nb = alloc[ch][sb] + 1 if alloc[ch][sb] else 0
                if not nb:
                    continue
                if ch == 0 or sb < bound:
                    v = b.read(nb)
                s[ch][sb] = dequant(v, (1 << nb) - 1) * scale[ch][sb]
        for ch in range(nch):
            out[ch].append(s[ch])
    return out


def decode_layer2(b, hd):
    if hd['mpeg1']:
        raise ValueError('MPEG-1 Layer II tables not implemented (none in the game data)')
    nch, bound, table = hd['channels'], hd['bound'], LSF_L2_ALLOC
    sblimit = len(table)
    alloc = [[None] * sblimit for _ in range(nch)]
    for sb in range(sblimit):
        nbal, classes = table[sb]
        for ch in range(nch):
            if ch == 0 or sb < bound:
                a = classes[b.read(nbal)]
            alloc[ch][sb] = a
    scfsi = [[0] * sblimit for _ in range(nch)]
    for sb in range(sblimit):
        for ch in range(nch):
            if alloc[ch][sb]:
                scfsi[ch][sb] = b.read(2)
    scale = [[None] * sblimit for _ in range(nch)]
    for sb in range(sblimit):
        for ch in range(nch):
            if not alloc[ch][sb]:
                continue
            sel = scfsi[ch][sb]
            if sel == 0:
                sf = [b.read(6), b.read(6), b.read(6)]
            elif sel == 1:
                x, y = b.read(6), b.read(6)
                sf = [x, x, y]
            elif sel == 2:
                sf = [b.read(6)] * 3
            else:
                x, y = b.read(6), b.read(6)
                sf = [x, y, y]
            scale[ch][sb] = [SCALEFACTORS[i] for i in sf]
    out = [[] for _ in range(nch)]
    for gr in range(12):
        part = gr // 4
        s = [[[0.0] * 32 for _ in range(3)] for _ in range(nch)]
        for sb in range(sblimit):
            for ch in range(nch):
                q = alloc[ch][sb]
                if not q:
                    continue
                levels, grouped = q
                if ch == 0 or sb < bound:
                    if grouped:
                        c = b.read({3: 5, 5: 7, 9: 10}[levels])
                        vals = [c % levels, c // levels % levels, c // (levels * levels)]
                    else:
                        nb = levels.bit_length()
                        vals = [b.read(nb), b.read(nb), b.read(nb)]
                f = scale[ch][sb][part]
                for i in range(3):
                    s[ch][i][sb] = dequant(vals[i], levels) * f
        for ch in range(nch):
            out[ch].extend(s[ch])
    return out


class Synth:
    def __init__(self):
        self.v = [[0.0] * 64 for _ in range(16)]  # newest first

    def run(self, s):
        new = [0.0] * 64
        for k, x in enumerate(s):
            if x:
                col = N_COLS[k]
                new = [a + x * c for a, c in zip(new, col)]
        self.v.pop()
        self.v.insert(0, new)
        v = self.v
        out = []
        for j in range(32):
            acc = 0.0
            for m in range(8):
                acc += v[2 * m][j] * D[64 * m + j] + v[2 * m + 1][32 + j] * D[64 * m + 32 + j]
            out.append(acc)
        return out


def decode_mpeg(data):
    """Decode a run of Layer I/II frames to (rate, channels, interleaved int16 list)."""
    pos, synth, rate, nch, pcm = 0, None, None, None, []
    while pos + 4 <= len(data):
        hd = parse_header(data, pos)
        if synth is None:
            rate, nch = hd['rate'], hd['channels']
            synth = [Synth() for _ in range(nch)]
        b = Bits(data[pos + 4 + (2 if hd['crc'] else 0):pos + hd['size']])
        subbands = (decode_layer1 if hd['layer'] == 1 else decode_layer2)(b, hd)
        chans = [[x for s in subbands[ch] for x in synth[ch].run(s)] for ch in range(nch)]
        for frame in zip(*chans):
            for x in frame:
                pcm.append(max(-32768, min(32767, int(round(x * 32768)))))
        pos += hd['size']
    return rate, nch, pcm


# --- CLI --------------------------------------------------------------------------------------


def write_wav(path, rate, nch, pcm):
    with wave.open(path, 'wb') as w:
        w.setnchannels(nch)
        w.setsampwidth(2)
        w.setframerate(rate)
        w.writeframes(struct.pack('<%dh' % len(pcm), *pcm))


def main():
    if len(sys.argv) < 3 or sys.argv[1] not in ('list', 'extract'):
        sys.exit(__doc__)
    entries = read_bank(sys.argv[2])
    if sys.argv[1] == 'list':
        for e in entries:
            hd = parse_header(e['data'], 0)
            secs = e['pcm_bytes'] / (2 * e['channels'] * e['rate'])
            print('%4d  %-16s %5d Hz %s  MPEG-%d L%d  %7d bytes  %6.2fs' % (
                e['index'], e['name'], e['rate'], 'stereo' if e['channels'] == 2 else 'mono  ',
                1 if hd['mpeg1'] else 2, hd['layer'], len(e['data']), secs))
        return
    outdir, rest = sys.argv[3], sys.argv[4:]
    want_mp2 = '--mp2' in rest
    names = {n.lower() for n in rest if n != '--mp2'}
    os.makedirs(outdir, exist_ok=True)
    for e in entries:
        if names and e['name'].lower() not in names:
            continue
        base = os.path.join(outdir, '%03d_%s' % (e['index'], os.path.splitext(e['name'])[0]))
        if want_mp2:
            open(base + '.mp2', 'wb').write(e['data'])
            continue
        rate, nch, pcm = decode_mpeg(e['data'])
        pcm = pcm[FILTERBANK_DELAY * nch:][:e['pcm_bytes'] // 2]
        write_wav(base + '.wav', rate, nch, pcm)
        print(base + '.wav', len(pcm) // nch, 'samples')


if __name__ == '__main__':
    main()
