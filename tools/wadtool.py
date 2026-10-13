"""Theme Park World DWFB .wad reader with EA RefPack (QFS, magic 0x10FB) decompression."""
import struct, sys, os


def refpack_decompress(src):
    flags, magic = src[0], src[1]
    assert magic == 0xFB, "not RefPack"
    pos = 2
    size_len = 4 if flags & 0x80 else 3
    if flags & 0x01:          # compressed-size field present; skip it
        pos += size_len
    out_size = int.from_bytes(src[pos:pos + size_len], 'big')
    pos += size_len
    out = bytearray()
    while pos < len(src):
        b0 = src[pos]
        if b0 < 0x80:         # 2-byte command
            b1 = src[pos + 1]; pos += 2
            plain = b0 & 3
            count = ((b0 & 0x1C) >> 2) + 3
            offset = ((b0 & 0x60) << 3) + b1 + 1
        elif b0 < 0xC0:       # 3-byte command
            b1, b2 = src[pos + 1], src[pos + 2]; pos += 3
            plain = b1 >> 6
            count = (b0 & 0x3F) + 4
            offset = ((b1 & 0x3F) << 8) + b2 + 1
        elif b0 < 0xE0:       # 4-byte command
            b1, b2, b3 = src[pos + 1], src[pos + 2], src[pos + 3]; pos += 4
            plain = b0 & 3
            count = ((b0 & 0x0C) << 6) + b3 + 5
            offset = ((b0 & 0x10) << 12) + (b1 << 8) + b2 + 1
        elif b0 < 0xFC:       # literal run only
            pos += 1
            out += src[pos:pos + ((b0 & 0x1F) << 2) + 4]
            pos += ((b0 & 0x1F) << 2) + 4
            continue
        else:                 # end of stream with 0-3 trailing literals
            pos += 1
            out += src[pos:pos + (b0 & 3)]
            break
        out += src[pos:pos + plain]; pos += plain
        start = len(out) - offset
        for i in range(count):  # byte-wise: copies may overlap
            out.append(out[start + i])
    if len(out) != out_size:
        raise ValueError(f"size mismatch {len(out)} != {out_size}")
    return bytes(out)


def read_wad(path):
    d = open(path, 'rb').read()
    assert d[:4] == b'DWFB', "not a DWFB wad"
    count = struct.unpack_from('<I', d, 0x48)[0]
    for i in range(count):
        _, name_off, name_len, off, stored, ctype, size, *_ = struct.unpack_from('<10I', d, 0x58 + i * 40)
        name = d[name_off:name_off + name_len].split(b'\0')[0].decode('latin1')
        blob = d[off:off + stored]
        yield name, (refpack_decompress(blob) if ctype == 4 else blob), ctype


if __name__ == '__main__':
    wad, out_dir = sys.argv[1], (sys.argv[2] if len(sys.argv) > 2 else None)
    ok = bad = 0
    for name, data, ctype in read_wad(wad):
        ok += 1
        if out_dir:
            p = os.path.join(out_dir, name.replace('\\', os.sep))
            os.makedirs(os.path.dirname(p) or out_dir, exist_ok=True)
            open(p, 'wb').write(data)
    print(f"{wad}: {ok} entries extracted")
