"""Theme Park World sound category maps: cat_<name>BANK.map and cat_<name>SFX.map.

Each sound category (ambient, kids, rides, ui, staff, speech, the lobby ones, and a ride's own
cat_ride) is a pair of maps beside its .sdt banks. They are read by the statically linked sound
library (SoundCategory_Load 0x6bf330: BANK.map, then SFX.map through 0x6c0450). Both start with
a 16-byte class GUID and a u32 0:
  BANK.map  {e9612c01-31d0-11d2-b409-00a0c993f203}
  SFX.map   {e9612c00-31d0-11d2-b409-00b0c993f203}

BANK.map lists the sample banks the category's sounds draw on:
  u32 0, u32 count
  count x 11 bytes   two stale heap pointers and 3 bytes (not used)
  count x (u32 length, char path[length])   e.g. "Sound\\Ambient"; the bank is <path>HD.sdt
                     under the data folder, so a category can share another's samples

SFX.map is a memory image that the loader fixes up in place (0x6c0510 and below):
  u32 0, u32 category count (1), then the image:
  category x count (0x18): u32 sounds, u32 (pointer), u16, u16, f32 x 3
  then per category its sounds (0x14 each):
      u32 sound number (what the game and Scape.omp ask for), u32 variations, u32 (pointer),
      u32 (4000, 1700, 4300, ...), u16 flags, u16
  then per sound its variations (0x2a each), then per variation its samples and links:
    variation: u16 samples, u16, u32 links, u32 (pointer), u8 u8 u16 u32 u8 u8 u16 u16 u16 u16
               (not traced), u32 cumulative pick weight at 0x1e (out of 0xffff, delta-coded on
               load), f32 at 0x22 (100.0), u32 (pointer to the links)
    sample (0x10): u32 sample number in the bank (1-based), u16 cumulative pick weight,
                   u16, u32 length in milliseconds, u16 bank (1-based in BANK.map), u16
    link (8):      u32 next variation (1-based), u8, u8, u8 first and u8 last percentage
  So a sound is a small Markov chain: it plays a variation (one of its samples, picked by
  weight), then rolls 0..100 and follows the link whose range holds the roll. The bus in
  cat_ambient, for instance, idles, pulls in and stops through such links.

SndReverb.map (install folder; read by 0x6c5d20): u32 version 2000, u32 slot count (32), then
per slot a u8 present flag and, if set, 13 dwords (floats and ints; reverb preset parameters,
not named yet). Slots 0..14 are present.

Usage:
  python -I tools/sndmap.py <cat_xxxSFX.map | folder | wad>...   print the sound graphs, naming
                                                                 the samples from the .sdt banks
  python -I tools/sndmap.py <SndReverb.map>                       print the reverb slots
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from wadtool import read_wad  # noqa: E402

BANK_GUID = bytes.fromhex('012c61e9d031d211b40900a0c993f203')
SFX_GUID = bytes.fromhex('002c61e9d031d211b40900b0c993f203')


def parse_bank_map(data):
    if data[:16] != BANK_GUID:
        raise ValueError('not a BANK.map')
    count = struct.unpack_from('<I', data, 0x18)[0]
    pos, paths = 0x1c + 11 * count, []
    for _ in range(count):
        n = struct.unpack_from('<I', data, pos)[0]
        paths.append(data[pos + 4:pos + 4 + n].split(b'\0')[0].decode('latin-1'))
        pos += 4 + n
    if pos != len(data):
        raise ValueError('%d bytes left over' % (len(data) - pos))
    return paths


def parse_sfx_map(data):
    if data[:16] != SFX_GUID:
        raise ValueError('not an SFX.map')
    count = struct.unpack_from('<I', data, 0x18)[0]
    pos = 0x1c
    cats = []
    for _ in range(count):
        n, _, a, b, f1, f2, f3 = struct.unpack_from('<IIHHfff', data, pos)
        cats.append({'count': n, 'values': (a, b, f1, f2, f3), 'sounds': []})
        pos += 0x18
    for cat in cats:
        for _ in range(cat['count']):
            number, nvar, _, value, flags, extra = struct.unpack_from('<IIIIHH', data, pos)
            cat['sounds'].append({'number': number, 'count': nvar, 'value': value, 'flags': flags,
                                  'extra': extra, 'variations': []})
            pos += 0x14
        for snd in cat['sounds']:
            for _ in range(snd['count']):
                raw = data[pos:pos + 0x2a]
                snd['variations'].append({'raw': raw, 'samples': [], 'links': [],
                                          'weight': struct.unpack_from('<I', raw, 0x1e)[0],
                                          'scale': struct.unpack_from('<f', raw, 0x22)[0]})
                pos += 0x2a
            for var in snd['variations']:
                nsamples = struct.unpack_from('<H', var['raw'], 0)[0]
                nlinks = struct.unpack_from('<I', var['raw'], 4)[0]
                for _ in range(nsamples):
                    sample, weight, _, ms, bank, _ = struct.unpack_from('<IHHIHH', data, pos)
                    var['samples'].append({'sample': sample, 'weight': weight, 'ms': ms, 'bank': bank})
                    pos += 0x10
                for _ in range(nlinks):
                    target, _, _, lo, hi = struct.unpack_from('<IBBBB', data, pos)
                    var['links'].append((target, lo, hi))
                    pos += 8
    if pos != len(data):
        raise ValueError('%d bytes left over' % (len(data) - pos))
    return cats


def sdt_names(data):
    """Entry names of an .sdt bank (see tools/sdt.py)."""
    count = struct.unpack_from('<I', data)[0]
    return [data[off + 8:off + 24].split(b'\0')[0].decode('latin-1')
            for off in struct.unpack_from('<%dI' % count, data, 4)]


def bank_names(where, bank_paths, wad_files=None):
    """Sample names per bank (1-based lists). Paths are relative to data/global or a level
    folder; a ride wad carries its own bank (Speaker\\Bank -> BankHD.sdt in the wad)."""
    names = []
    for path in bank_paths:
        stem = path.split('\\')[-1] + 'HD.sdt'
        found = None
        if wad_files is not None:
            found = next((d for n, d in wad_files.items() if n.lower() == stem.lower()), None)
        d = os.path.dirname(os.path.abspath(where))
        for _ in range(4):
            if found is not None:
                break
            d = os.path.dirname(d)
            candidate = os.path.join(d, *path.split('\\')) + 'HD.sdt'
            if os.path.exists(candidate):
                found = open(candidate, 'rb').read()
        names.append(sdt_names(found) if found is not None else None)
    return names


def sample_name(names, s):
    bank = names[s['bank'] - 1] if 0 < s['bank'] <= len(names) else None
    if bank and 0 < s['sample'] <= len(bank):
        return bank[s['sample'] - 1]
    return 'bank %d sample %d' % (s['bank'], s['sample'])


def show(label, sfx, banks, names):
    print('%s: banks %s' % (label, ', '.join(banks)))
    for cat in sfx:
        print('  category %s' % (cat['values'],))
        for snd in cat['sounds']:
            print('  sound %d (%d, flags %#x)' % (snd['number'], snd['value'], snd['flags']))
            for i, var in enumerate(snd['variations']):
                picks = ', '.join('%s %dms' % (sample_name(names, s), s['ms']) for s in var['samples'])
                links = ' '.join('%d-%d%%->%d' % (lo, hi, t) for t, lo, hi in var['links'])
                print('    %d: %s%s' % (i + 1, picks, '  then ' + links if links else ''))


def map_pairs(path):
    if path.lower().endswith('.wad'):
        files = {n.rsplit('\\', 1)[-1]: d for n, d, _ in read_wad(path)}
        for name, data in sorted(files.items()):
            if name.endswith('SFX.map'):
                bank = files.get(name[:-7] + 'BANK.map')
                yield '%s:%s' % (path, name), data, bank, path, files
    elif os.path.isdir(path):
        for root, _, fs in os.walk(path):
            for f in sorted(fs):
                full = os.path.join(root, f)
                if f.endswith('SFX.map'):
                    yield from map_pairs(full)
                elif f.lower().endswith('.wad'):
                    yield from map_pairs(full)
    else:
        bank = path[:-7] + 'BANK.map'
        yield (path, open(path, 'rb').read(), open(bank, 'rb').read() if os.path.exists(bank) else None,
               path, None)


def parse_reverb(data):
    version, count = struct.unpack_from('<II', data)
    if version != 2000:
        raise ValueError('unknown SndReverb.map version %d' % version)
    pos, slots = 8, []
    for _ in range(count):
        present = data[pos]
        pos += 1
        slots.append(data[pos:pos + 52] if present else None)
        pos += 52 if present else 0
    if pos != len(data):
        raise ValueError('%d bytes left over' % (len(data) - pos))
    return slots


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    for arg in sys.argv[1:]:
        if os.path.basename(arg).lower() == 'sndreverb.map':
            for i, raw in enumerate(parse_reverb(open(arg, 'rb').read())):
                if raw:
                    vals = [f if 1e-6 < abs(f) < 1e7 else n
                            for f, n in zip(struct.unpack('<13f', raw), struct.unpack('<13i', raw))]
                    print('slot %2d: %s' % (i, ' '.join('%g' % v for v in vals)))
            continue
        for label, data, bank_data, where, wad_files in map_pairs(arg):
            banks = parse_bank_map(bank_data) if bank_data else []
            show(label, parse_sfx_map(data), banks, bank_names(where, banks, wad_files))


if __name__ == '__main__':
    main()
