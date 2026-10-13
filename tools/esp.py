"""Theme Park World people and effect sprites: data/esprites.wad (.esp, .tpc, .fpc).

The wad holds one folder per sprite group (Generic/Kids, Fantasy/Entertainers, Generic/Litter,
...). Only the first entry of each folder carries the folder path; later entries are bare names,
so names such as SPR_SP.ESP repeat between folders. Each sprite has:

  SPR_xx.ESP  animation table, 350 bytes, read by Sprite_LoadEsp 0x540d90:
    char magic[12]     "ESP_FILE2.00" (only its length is checked)
    char name[256]     the sprite's file stem with a .TPS extension ("SPR_KI.TPS")
    u8   ground frames 1 = also use the .fpc frames for the ground-level cameras (see below)
    u8   ?             stored at +0x206 but never read; it splits the kids into two groups
                       (CH, KI, SA, SU = 1; BE, BI, FR, TA = 0), the same in their heads
    16 x (u16 first frame, u8 frame count, u8 directions)
                       an animation covers frames first + direction * count + frame, for
                       max(directions, 1) directions. People have 5 directions, from behind
                       to the front (the other 3 of 8 are presumably mirrored); effects and
                       litter have 0. Heads (Kidsheads, Costumeheads) have one record,
                       (0, 8, 7): 8 headings x 7 tilts, drawn over the body so the head can
                       turn on its own. Unused slots are zero.
    4 x (u8 animation slot + 1, u8 entry + 1, u8, u8)
                       special actions 0..3, read only if present (0 = unused).
                       Sprite_GetAction 0x540c70 hands back the animation slot, an entry in the
                       pointer table at 0x74f7c8 and the two other bytes. Only entertainers
                       use one (01 05 00 00; Dracula 01 05 02 28).
  SPR_xx.TPC  frames for the normal park camera
  SPR_xx.FPC  the same frames drawn from a lower, more side-on camera (taller images). When the
              camera drops to ground level (walking the park as a visitor, or following a ride
              or a person: Camera_SetMode 0x42ab20 modes 2 and 4), Sprites_UseGroundFrames
              0x542420 reloads the .fpc frames of every sprite whose ground-frames flag is set,
              and Sprites_UseParkFrames 0x542640 switches back.

  .tpc / .fpc layout (little-endian):
    u16 3, u16 3, u32 frame count
    palette: 256 x (B, G, R, unused); index 0 is transparent
    per frame:
      u32 size of the row data
      u16 width, u16 height, u16 128, u16 128
      i32 dx, dy        offset from the anchor (the feet) to the top-left pixel
      height rows: u8 byte count, then ops until that many bytes are used:
        signed byte c < 0: repeat the next byte -c times;  c >= 0: copy the next c bytes
      every row decodes to exactly width pixels

Shipped data quirks: Generic/Kidsheads/SPR_BE.TPC has 55 frames but its .ESP asks for 56, and
Jungle/Entertainers/SPR_EX.FPC has 210 frames, 9 short of its walk (frames 179-218); its
ground-frames flag is 0, so the game never loads that .fpc.

Sprites_LoadAll 0x541d90 loads the folders in the order of the table at 0x763f88 (kids,
kidsheads, costumes, costumeheads, entertainers, handymen, mechanics, guards, researchers,
thoughts, balloons, litter, specialfx, particles) with Sprites_LoadFolder 0x541310. That finds
every *.ESP in the folder and loads the four walk-mode avatars (SPR_BI, SPR_KI, SPR_TA,
SPR_SU; table 0x764030) first.

Usage:
  python -I tools/esp.py <esprites.wad> [out_dir]   list the animations, and write sprite sheets
                                                    (one PNG per animation: a row per direction)
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from wadtool import read_wad  # noqa: E402
from wct2png import write_png  # noqa: E402

ESP_MAGIC = b'ESP_FILE2.00'


def parse_esp(data):
    if data[:12] != ESP_MAGIC:
        raise ValueError('not an ESP_FILE2.00 file')
    name = data[12:268].split(b'\0')[0].decode('latin-1')
    anims = [struct.unpack_from('<HBB', data, 270 + 4 * i) for i in range(16)]
    actions = [tuple(data[334 + 4 * i:338 + 4 * i]) for i in range(4)] if len(data) >= 350 else []
    return {'name': name, 'ground_frames': data[268], 'unknown': data[269], 'anims': anims,
            'actions': actions}


def parse_frames(data):
    """Return (palette as RGBA tuples, [frame dicts with indexed pixels])."""
    a, b, count = struct.unpack_from('<HHI', data)
    if (a, b) != (3, 3):
        raise ValueError('unknown sprite header %d %d' % (a, b))
    palette = [(data[8 + 4 * i + 2], data[8 + 4 * i + 1], data[8 + 4 * i], 0 if i == 0 else 255)
               for i in range(256)]
    pos, frames = 8 + 1024, []
    for _ in range(count):
        size, w, h, _, _, dx, dy = struct.unpack_from('<IHHHHii', data, pos)
        pos += 20
        end, px = pos + size, bytearray()
        for _ in range(h):
            row_end, row_start = pos + 1 + data[pos], len(px)
            pos += 1
            while pos < row_end:
                c = struct.unpack_from('b', data, pos)[0]
                if c < 0:
                    px += data[pos + 1:pos + 2] * -c
                    pos += 2
                else:
                    px += data[pos + 1:pos + 1 + c]
                    pos += 1 + c
            if len(px) - row_start != w:
                raise ValueError('row does not decode to the frame width')
        if pos != end or len(px) != w * h:
            raise ValueError('frame size mismatch')
        frames.append({'width': w, 'height': h, 'dx': dx, 'dy': dy, 'pixels': bytes(px)})
    if pos != len(data):
        raise ValueError('%d bytes left over' % (len(data) - pos))
    return palette, frames


def sprites(wad_path):
    """Group the wad's entries into {folder/stem: {'esp': ..., 'tpc': ..., 'fpc': ...}}."""
    out, folder = {}, ''
    for name, data, _ in read_wad(wad_path):
        parts = name.replace('\\', '/').rsplit('/', 1)
        if len(parts) == 2:
            folder = parts[0]
        stem, ext = os.path.splitext(parts[-1])
        out.setdefault(folder + '/' + stem.upper(), {})[ext[1:].lower()] = data
    return out


def sheet(palette, frames, first, count, dirs):
    """RGBA sheet for one animation: a row per direction, frames aligned on their anchors."""
    cells = [[frames[first + d * count + f] for f in range(count)] for d in range(dirs)]
    flat = [fr for row in cells for fr in row]
    left = max(-fr['dx'] for fr in flat)
    right = max(fr['width'] + fr['dx'] for fr in flat)
    up = max(-fr['dy'] for fr in flat)
    down = max(fr['height'] + fr['dy'] for fr in flat)
    cw, ch = left + right + 2, up + down + 2
    W, H = cw * count, ch * dirs
    img = bytearray(W * H * 4)
    for d, row in enumerate(cells):
        for f, fr in enumerate(row):
            ox, oy = f * cw + 1 + left + fr['dx'], d * ch + 1 + up + fr['dy']
            w, px = fr['width'], fr['pixels']
            for y in range(fr['height']):
                o = ((oy + y) * W + ox) * 4
                for x in range(w):
                    i = px[y * w + x]
                    if i:
                        img[o + 4 * x:o + 4 * x + 4] = bytes(palette[i])
    return W, H, bytes(img)


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    out = sys.argv[2] if len(sys.argv) > 2 else None
    for key, parts in sorted(sprites(sys.argv[1]).items()):
        esp = parse_esp(parts['esp'])
        views = {v: parse_frames(parts[v]) for v in ('tpc', 'fpc') if v in parts}
        print('%s (%s): %s, ground frames %d, ? %d' % (key, esp['name'], ', '.join(
            '%s %d frames' % (v, len(fr)) for v, (_, fr) in views.items()), esp['ground_frames'], esp['unknown']))
        for i, (anim, entry, c, d) in enumerate(esp['actions']):
            if anim:
                print('  action %d: anim %d, entry %d, %d %d' % (i, anim - 1, entry - 1, c, d))
        for slot, (first, count, dirs) in enumerate(esp['anims']):
            if not count:
                continue
            n = max(dirs, 1)
            print('  anim %2d: frames %3d-%3d (%d x %d directions)' % (slot, first, first + count * n - 1, count, n))
            if not out:
                continue
            for v, (palette, frames) in views.items():
                if first + count * n > len(frames):
                    print('    %s: only %d frames, skipped' % (v, len(frames)))
                    continue
                path = os.path.join(out, key.replace('/', '_') + '_%s_anim%02d.png' % (v, slot))
                os.makedirs(out, exist_ok=True)
                write_png(path, *sheet(palette, frames, first, count, n))


if __name__ == '__main__':
    main()
