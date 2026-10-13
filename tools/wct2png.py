"""Convert Theme Park World .wct wavelet textures to PNG (pure Python, no dependencies).

Reimplements the game's decoders (tp.exe WaveletCodec_DecodeV13 0x595920 and
WaveletCodec_DecodeV12 0x593ab0, dispatched by WaveletCodec_Decode 0x592060):

  header (28 bytes, little-endian)
    0x00 u8  version (0x12/0x13)  0x01 u8 alpha flag     0x02 u8 bpp (24/32)   0x03 u8 ?
    0x04 u16 width                0x06 u16 height
    0x08 u16 quality Y, Cr, Cb, A (note: Cr before Cb)
    0x10 u32 main stream bytes    0x14 u32 alpha stream bytes
    0x18 u32 average colour (ARGB)
    0x1C main stream, then alpha stream

  quantiser step per plane (the texture object's caller, e.g. FUN_00564e20, constants 0x7011b8..):
    Y: 1 + q/2     Cb, Cr: 1 + q/4     A: q + 1

  v0x13 streams: LZ77 bitstream (16-bit LE words, LSB-first bits):
    flag 0 -> 8-bit literal;  flag 1 -> 12-bit distance, 7-bit length-1; distance 0 ends
  v0x12 streams: "BILZ" container = 28-byte header (magic, unpacked size, packed size,
    zlib windowBits, level, 0, 0) + a zlib 1.1.3 stream; a few small streams are stored raw
  decompressed: signed-byte coefficients, 0x80 escapes a following int16
  planes in stream order: Y (SxS), Cb, Cr ((S/2)x(S/2)); alpha in its own stream (SxS);
    S = next pow2 >= max(w,h), min 8
  one-level 2D inverse Daubechies-4 wavelet, periodic boundaries; columns hold low rows then
    high rows. Rows hold interleaved (low, high) pairs in v0x13, [all low, all high] in v0x12
  colour: JPEG YCbCr -> RGB (B = Y + 1.772 Cb, G = Y - 0.344136 Cb - 0.714136 Cr, R = Y + 1.402 Cr)

Usage:
  python -I tools/wct2png.py <file.wct | archive.wad> <out_dir>
"""
import math
import os
import struct
import sys
import zlib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from wadtool import read_wad  # noqa: E402

# Daubechies-4 low-pass filter; the exe keeps these at 0xc27208.. (h0..h3).
R3, S2 = math.sqrt(3), math.sqrt(2)
H0, H1, H2, H3 = (1 + R3) / (4 * S2), (3 + R3) / (4 * S2), (3 - R3) / (4 * S2), (1 - R3) / (4 * S2)



def q_luma(q):
    return 1 + q / 2


def q_chroma(q):
    return 1 + q / 4


def q_alpha(q):
    return q + 1


def lz_decompress(data):
    """The codec's LZ77 stage (exe: inline loop + bit reader FUN_00597090)."""
    pos, acc, nbits = 0, 0, 0
    out = bytearray()

    def bits(n):
        nonlocal pos, acc, nbits
        if nbits < n:
            word = data[pos] | (data[pos + 1] << 8) if pos + 1 < len(data) else (data[pos] if pos < len(data) else 0)
            pos += 2
            acc |= word << nbits
            nbits += 16
        v = acc & ((1 << n) - 1)
        acc >>= n
        nbits -= n
        return v

    while True:
        if bits(1) == 0:
            out.append(bits(8))
            continue
        dist = bits(12)
        length = bits(7) + 1
        if dist == 0:
            return bytes(out)
        start = len(out) - dist
        for i in range(length):  # may overlap
            out.append(out[start + i])


class Coeffs:
    """Signed-byte coefficient reader; 0x80 escapes a little-endian int16."""

    def __init__(self, buf):
        self.b, self.p = buf, 0

    def next(self):
        v = self.b[self.p]
        if v == 0x80:
            v = struct.unpack_from('<h', self.b, self.p + 1)[0]
            self.p += 3
            return v
        self.p += 1
        return v - 256 if v > 127 else v


def inverse_d4(s, d):
    """One inverse D4 step with periodic wrap: lists s, d (length n) -> 2n samples."""
    n = len(s)
    out = [0.0] * (2 * n)
    for k in range(n):
        sp, dp = s[k - 1], d[k - 1]  # k-1 wraps to the end for k=0
        out[2 * k] = H2 * sp + H1 * dp + H0 * s[k] + H3 * d[k]
        out[2 * k + 1] = H3 * sp - H0 * dp + H1 * s[k] - H2 * d[k]
    return out


def decode_plane(cr, size, q, interleaved=True):
    """Read size*size coefficients and run the 2D inverse transform (rows, then columns)."""
    half = size // 2
    rows = []
    for _ in range(size):
        c = [cr.next() * q for _ in range(size)]
        s, d = (c[0::2], c[1::2]) if interleaved else (c[:half], c[half:])
        rows.append(inverse_d4(s, d))
    # columns: first half of rows are low-pass, second half high-pass
    out = [[0.0] * size for _ in range(size)]
    for x in range(size):
        col = inverse_d4([rows[y][x] for y in range(half)], [rows[half + y][x] for y in range(half)])
        for y in range(size):
            out[y][x] = col[y]
    return out


def decode_wct(data):
    ver, alpha_flag, bpp, _unk = data[0], data[1], data[2], data[3]
    if ver not in (0x12, 0x13):
        raise ValueError(f'unsupported .wct version {ver:#x}')
    w, h = struct.unpack_from('<HH', data, 4)
    qy, qcr, qcb, qa = struct.unpack_from('<4H', data, 8)
    n_main, n_alpha = struct.unpack_from('<II', data, 0x10)
    main = data[0x1C:0x1C + n_main]
    alpha = data[0x1C + n_main:0x1C + n_main + n_alpha] if n_alpha else b''
    unpack = lz_decompress if ver == 0x13 else bilz_decompress
    inter = ver == 0x13

    size = max(8, 1 << max(0, math.ceil(math.log2(max(w, h)))))
    cr = Coeffs(unpack(main))
    Y = decode_plane(cr, size, q_luma(qy), inter)
    U = decode_plane(cr, size // 2, q_chroma(qcb), inter)
    V = decode_plane(cr, size // 2, q_chroma(qcr), inter)
    A = decode_plane(Coeffs(unpack(alpha)), size, q_alpha(qa), inter) if alpha else None

    clamp = lambda v: 0 if v < 0 else 255 if v > 255 else int(round(v))
    pixels = bytearray()
    for y in range(h):
        for x in range(w):
            yy, cb, crv = Y[y][x], U[y // 2][x // 2], V[y // 2][x // 2]
            pixels += bytes((clamp(yy + 1.402 * crv),
                             clamp(yy - 0.344136 * cb - 0.714136 * crv),
                             clamp(yy + 1.772 * cb),
                             clamp(A[y][x]) if A else 255))
    return w, h, bytes(pixels), {'version': ver, 'alpha_flag': alpha_flag, 'bpp': bpp, 'q': (qy, qcr, qcb, qa)}


def bilz_decompress(data):
    """v0x12 stream: 'BILZ' + 7 header words, then zlib. Streams without the magic are raw."""
    if data[:4] != b'BILZ':
        return data
    size = struct.unpack_from('<I', data, 4)[0]
    out = zlib.decompress(data[28:])
    if len(out) != size:
        raise ValueError(f'BILZ size mismatch {len(out)} != {size}')
    return out


def write_png(path, w, h, rgba):
    def chunk(tag, body):
        c = tag + body
        return struct.pack('>I', len(body)) + c + struct.pack('>I', zlib.crc32(c) & 0xffffffff)
    raw = b''.join(b'\0' + rgba[y * w * 4:(y + 1) * w * 4] for y in range(h))
    with open(path, 'wb') as f:
        f.write(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 6, 0, 0, 0))
                + chunk(b'IDAT', zlib.compress(raw, 9)) + chunk(b'IEND', b''))


def textures_in(path):
    if path.lower().endswith('.wad'):
        for name, data, _ in read_wad(path):
            if name.lower().endswith('.wct'):
                yield name, data
    else:
        yield os.path.basename(path), open(path, 'rb').read()


if __name__ == '__main__':
    src, out_dir = sys.argv[1], sys.argv[2]
    os.makedirs(out_dir, exist_ok=True)
    for name, data in textures_in(src):
        try:
            w, h, px, info = decode_wct(data)
            dst = os.path.join(out_dir, name.replace('\\', '_').rsplit('.', 1)[0] + '.png')
            write_png(dst, w, h, px)
            print(f'{name}: {w}x{h} {info}')
        except Exception as e:
            print(f'{name}: FAILED {e}')
