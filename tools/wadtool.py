"""Theme Park World DWFB .wad reader and writer with EA RefPack (QFS, magic 0x10FB).

  wadtool.py WAD [OUT_DIR]     extract (OUT_DIR also gets _wad.txt: entry order and tags)
  wadtool.py --pack DIR WAD    rebuild a wad from an extracted directory
  wadtool.py --check WAD...    rebuild each wad and compare with the original bytes
"""
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


def _crc16_table():
    table = []
    for i in range(256):
        c = i
        for _ in range(8):
            c = (c >> 1) ^ 0xA001 if c & 1 else c >> 1
        table.append(c)
    return table


_CRC16 = _crc16_table()


def refpack_compress(src):
    """EA's encoder from tp.exe (0x641a50; C++ port in src/game/files/refpack.cpp).

    Greedy LZ77 over a 128 KB window. Candidates come from chains keyed by the CRC-16 of 3
    bytes, newest first; a candidate wins when it saves strictly more bytes after the cost
    of its command. Gives the shipped .wad entries byte for byte.
    """
    n = len(src)
    t = _CRC16
    pad = bytes(src) + b'\0\0'  # hashes near the end read zeros (no match can win there)
    head = [-1] * 0x10000
    link = [0] * 0x20000

    def hash3(p):
        h = t[pad[p]]
        h = t[(h & 0xFF) ^ pad[p + 1]] ^ (h >> 8)
        return t[(h & 0xFF) ^ pad[p + 2]] ^ (h >> 8)

    out = bytearray(b'\x10\xfb' + n.to_bytes(3, 'big'))
    lit = pos = 0  # first pending literal, current position

    def flush(lit, end):  # literal runs of 4-112 bytes until at most 3 are left
        while end - lit > 3:
            k = min((end - lit) & ~3, 0x70)
            out.append(0xE0 + (k >> 2) - 1)
            out.extend(src[lit:lit + k])
            lit += k
        return lit

    while pos < n:
        mx = min(n - pos, 0x404)
        best_len = best_cost = 2
        best_off = 0
        lowest = max(pos - 0x1FFFF, 0)
        h = hash3(pos)
        cand = head[h]
        while cand >= lowest:
            if best_len < mx and src[pos + best_len] == src[cand + best_len]:
                k = 0
                while k + 16 <= mx and src[pos + k:pos + k + 16] == src[cand + k:cand + k + 16]:
                    k += 16
                while k < mx and src[pos + k] == src[cand + k]:
                    k += 1
                if k > best_len:
                    off = pos - cand - 1
                    cost = 2 if off < 0x400 and k <= 10 else 3 if off < 0x4000 and k <= 0x43 else 4
                    if k - cost > best_len - best_cost:
                        best_len, best_cost, best_off = k, cost, off
                        if k >= 0x404:
                            break
            cand = link[cand & 0x1FFFF]
        if best_cost >= best_len:  # literal
            link[pos & 0x1FFFF] = head[h]
            head[h] = pos
            pos += 1
            continue
        lit = flush(lit, pos)
        lits = pos - lit
        if best_cost == 2:
            out += bytes((((best_len - 3) << 2) + ((best_off >> 3) & 0x60) + lits, best_off & 0xFF))
        elif best_cost == 3:
            out += bytes((0x80 + best_len - 4, (best_off >> 8) + (lits << 6), best_off & 0xFF))
        else:
            out += bytes((0xC0 + (((best_len - 5) >> 8) << 2) + ((best_off >> 12) & 0x10) + lits,
                          (best_off >> 8) & 0xFF, best_off & 0xFF, (best_len - 5) & 0xFF))
        out += src[lit:pos]
        for p in range(pos, pos + best_len):
            h = hash3(p)
            link[p & 0x1FFFF] = head[h]
            head[h] = p
        pos += best_len
        lit = pos
    lit = flush(lit, n)
    out.append(0xFC + n - lit)
    out += src[lit:n]
    return bytes(out)


def read_wad_entries(path):
    """[(name, data, tag)] with the decoded data and each entry's first directory word.

    The tag's meaning is unknown (it depends on the name, not the data); write_wad needs it
    to reproduce a wad.
    """
    d = open(path, 'rb').read()
    assert d[:4] == b'DWFB', "not a DWFB wad"
    out = []
    for i in range(struct.unpack_from('<I', d, 0x48)[0]):
        tag, name_off, name_len, off, stored, ctype, size, *_ = struct.unpack_from('<10I', d, 0x58 + i * 40)
        name = d[name_off:name_off + name_len - 1].decode('latin1')
        blob = d[off:off + stored]
        out.append((name, refpack_decompress(blob) if ctype == 4 else blob, tag))
    return out


def write_wad(path, entries):
    """Writes [(name, data, tag)] the way the shipped wads were made: version 2, data in
    entry order after the directory, names last; an entry is RefPack-compressed only when
    that makes it smaller. Rebuilding any shipped wad from read_wad_entries gives the same bytes.
    """
    count = len(entries)
    blobs = []
    for name, data, tag in entries:
        packed = refpack_compress(data) if data else b''
        blobs.append((packed, 4, len(data)) if len(packed) < len(data) else (bytes(data), 0, 0))
    names = [name.encode('latin1') + b'\0' for name, _, _ in entries]
    data_off = 0x58 + 40 * count
    names_off = data_off + sum(len(b) for b, _, _ in blobs)
    head = b'DWFB' + struct.pack('<I', 2) + bytes(0x40) + struct.pack('<4I', count, names_off, sum(map(len, names)), 0)
    directory = bytearray()
    for (_, _, tag), (blob, ctype, size), nm in zip(entries, blobs, names):
        directory += struct.pack('<10I', tag, names_off, len(nm), data_off, len(blob), ctype, size, 0, 0, 0)
        names_off += len(nm)
        data_off += len(blob)
    with open(path, 'wb') as f:
        f.write(head + directory + b''.join(b for b, _, _ in blobs) + b''.join(names))


MANIFEST = '_wad.txt'


def main():
    args = sys.argv[1:]
    if args[:1] == ['--pack'] and len(args) == 3:
        # Repack a directory made by extraction; MANIFEST keeps the order and tags.
        src_dir, wad = args[1], args[2]
        entries = []
        for line in open(os.path.join(src_dir, MANIFEST), encoding='latin1').read().splitlines():
            tag, name = line.split(' ', 1)
            data = open(os.path.join(src_dir, name.replace('\\', os.sep)), 'rb').read()
            entries.append((name, data, int(tag, 16)))
        write_wad(wad, entries)
        print(f"{wad}: {len(entries)} entries packed")
        return
    if args[:1] == ['--check']:
        # Rebuild each wad into a temporary file and compare with the original bytes.
        import tempfile
        ok = bad = 0
        for wad in args[1:]:
            with tempfile.TemporaryDirectory() as tmp:
                out = os.path.join(tmp, 'x.wad')
                write_wad(out, read_wad_entries(wad))
                same = open(out, 'rb').read() == open(wad, 'rb').read()
            ok += same
            bad += not same
            print(f"{wad}: {'identical' if same else 'DIFFERENT'}")
        print(f"{ok} identical, {bad} different")
        return
    if not args or args[0].startswith('--'):
        sys.exit("usage: wadtool.py WAD [OUT_DIR] | --pack DIR WAD | --check WAD...")
    wad, out_dir = args[0], (args[1] if len(args) > 1 else None)
    entries = read_wad_entries(wad)
    if out_dir:
        for name, data, _ in entries:
            p = os.path.join(out_dir, name.replace('\\', os.sep))
            os.makedirs(os.path.dirname(p) or out_dir, exist_ok=True)
            open(p, 'wb').write(data)
        with open(os.path.join(out_dir, MANIFEST), 'w', encoding='latin1', newline='\n') as f:
            f.writelines(f"{tag:08x} {name}\n" for name, _, tag in entries)
    print(f"{wad}: {len(entries)} entries extracted")


if __name__ == '__main__':
    main()
