"""Theme Park World .md2 motion blocks: keyframed animation for a model's nodes.

Motion-only .md2 files (header flags 0x10 or 0x20, e.g. Advisorm1.MD2, flamingoI1.MD2,
Spa_gateM1.MD2) and some geometry files carry a motion block at header word 0x26. MD2_Load
0x46d6d0 relocates its pointers (file offsets) only for sub-version 203; it drops the block for
older files and clears flag 0x20 for newer ones. Motion_Apply 0x4721f0 runs Track_Apply
0x471860 on each track at the instance's current frame.

Motion header (offsets from the block):
  0x00 u32 flags (bit 1: material animation, table at 0x30)     0x08 u32 length in frames
  0x0c u16 position curves   0x0e u16 scale keys   0x10 u16 rotation keys   0x12 u16 tracks
  0x14 u16 path tracks       0x16 u16 vertices over all raw vertex tracks (0x4000)
  0x18 u16 material animation entries   0x1a u16 nodes used
  0x20 -> position curves (16 B each)    0x24 -> all scale keys     0x28 -> all rotation keys
  0x2c -> tracks (64 B each)             0x30 -> material animation (8 B each, below)
  0x38 -> u16 node indices the motion uses (shown when it starts)  0x3c -> all ease tables
All five totals always equal the sums over the tracks.
Material animation (0x470c90, run after the tracks): {u16 material, u16 keys, -> u32 keys
(u16 frame, u16 value)}; the last key at or before the frame writes value into the model's
material (8-byte records at model +0x50) at +2 and marks it changed (+1 |= 4). The shipped
ones blink between 0 and 1 (gates, islands).

Track (64 B):
  0x00 u32 index   0x04 u32 channels (below)   0x0c u32 (a frame number; not read here)
  0x10 u16 rotation keys   0x12 u16 scale keys   0x14 u16 node   0x16 u16 visibility keys
  0x18 -> position curve   0x1c -> rotation keys   0x20 -> scale keys   0x24 -> path
  0x28 -> vertex animation 0x2c -> UV animation    0x30 -> visibility   0x34 -> ease tables
  0x3c   runtime: the node the track drives
Channels (bits of 0x04):
  0x0001 position: a curve {u32 type, u16 points, u16 keys, -> float3 points, -> keys
         (u16 frame, u16)}; type bit 0x02 = cubic Bezier (3 points a segment, 0x474840),
         bit 0x08 = linear (0x474bf0), else Catmull-Rom (0x474cc0); indices wrap, so curves
         can close. Writes the node's translation row. Shipped files use only type 0x12
         (Bezier, 3 * (keys - 1) + 1 points) and 0x18 (linear, a point per key).
  0x0008 rotation: 20-byte keys {u16 frame, s16 ease, f32 x, y, z, w}; slerp (0x4742a0), or
         plain lerp under a global flag; keys loop. The ease number counts up from 0 per key
         and selects an 8-byte ease table in the track's tables (0x34) for the segment after
         the key; -1 = none. With table e and segment fraction t, u = 9t, i = floor(u): the
         eased t lerps (u - i) between points i and i+1 of 0, e[0]/255 .. e[7]/255, 1.
         0x10/0x20/0x40 come with it (not traced).
  0x0080 scale: 16-byte keys {u16 frame, u16, f32 x, y, z}, lerped; rescales the node's axes
         (0x46f910). 0x0100 comes with it.
  0x0200 path (0x0400: also face along it): {u32 start frame, u32 frames, u32 0, -> float per
         frame}, the floats the position along the path in percent (mod 100, lerped). The
         curve (a position-curve record) is the geometry model's path table (+0xac, 16 B each)
         entry named by the node (+0x52); 0x400 turns the node along its tangent.
  0x1000 vertex animation: with 0x4000 a {-> index map or 0, u32 vertices a frame,
         -> float3 per vertex per frame} block, lerped between frames; without it, packed keys
         (0x4714a0): 10:10:10 signed ints per vertex, value * scale + offset (floats at
         0x14/0x20 of the block). 0x2000 comes with some.
  0x10000 UV animation: {u32 UVs, -> (u16 first key, u16 keys) per UV, ..., -> u16 frames,
         -> float2 UVs}, lerped (0x4745c0)
  0x20000 visibility: s16 frames; the last whose magnitude <= the frame decides: > 0 shown,
         <= 0 hidden (node flag 0x10)

Usage:
  python -I tools/md2motion.py <file.md2 | archive.wad | folder>... [-v]   list motions
  python -I tools/md2motion.py --check <file | wad | folder>...           validate all
"""
import math
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from wadtool import read_wad  # noqa: E402

MAGIC = 0x1CD15D46
CHANNELS = [(0x1, 'pos'), (0x8, 'rot'), (0x80, 'scale'), (0x200, 'path'), (0x400, 'face-path'),
            (0x1000, 'vertex'), (0x4000, 'raw'), (0x10000, 'uv'), (0x20000, 'vis')]
CURVES = {0x2: 'bezier', 0x8: 'linear'}


def curve_kind(kind):
    return 'bezier' if kind & 2 else 'linear' if kind & 8 else 'catmull-rom'


class Motion:
    def __init__(self, data):
        self.d = data
        w = struct.unpack_from('<45I', data)
        if w[0] != MAGIC:
            raise ValueError('not a .md2')
        self.subversion, self.file_flags, base = w[2], w[0xc], w[0x26]
        if not base:
            raise ValueError('no motion block')
        self.base = base
        h = struct.unpack_from('<16I', data, base)
        self.flags, self.length = h[0], h[2]
        (self.n_curves, self.n_scale, self.n_rot, self.n_tracks, self.n_paths, self.n_raw_vertices,
         self.n_material, self.n_nodes) = struct.unpack_from('<8H', data, base + 0xc)
        self.nodes = list(struct.unpack_from('<%dH' % self.n_nodes, data, h[0xe])) if self.n_nodes else []
        self.tracks = [self._track(h[0xb] + 0x40 * i) for i in range(self.n_tracks)]
        self.material = []
        for i in range(self.n_material if self.flags & 2 else 0):
            o = self._ptr(h[0xc] + 8 * i, 8)
            mat, n, keys = struct.unpack_from('<HHI', data, o)
            self._ptr(keys, 4 * n)
            self.material.append((mat, [struct.unpack_from('<HH', data, keys + 4 * k) for k in range(n)]))

    def _ptr(self, p, size=1):
        if p and p + size > len(self.d):
            raise ValueError('pointer %#x (+%#x) past the end (%#x)' % (p, size, len(self.d)))
        return p

    def _track(self, o):
        d = self.d
        r = struct.unpack_from('<16I', d, o)
        n_rot, n_scale, node, n_vis = struct.unpack_from('<4H', d, o + 0x10)
        t = {'index': r[0], 'channels': r[1], 'frame': r[3], 'node': node}
        ch = r[1]
        if ch & 1:
            c = self._ptr(r[6], 16)
            kind, npts, nkeys, pts, keys = struct.unpack_from('<IHHII', d, c)
            self._ptr(pts, 12 * npts)
            self._ptr(keys, 4 * nkeys)
            t['position'] = {'kind': kind, 'points': [struct.unpack_from('<3f', d, pts + 12 * i) for i in range(npts)],
                             'frames': [struct.unpack_from('<H', d, keys + 4 * i)[0] for i in range(nkeys)]}
        if ch & 8:
            self._ptr(r[7], 20 * n_rot)
            t['rotation'] = [struct.unpack_from('<Hh4f', d, r[7] + 20 * i) for i in range(n_rot)]
        if ch & 0x80:
            self._ptr(r[8], 16 * n_scale)
            t['scale'] = [struct.unpack_from('<HH3f', d, r[8] + 16 * i) for i in range(n_scale)]
        if ch & 0x20000:
            self._ptr(r[12], 2 * n_vis)
            t['visibility'] = list(struct.unpack_from('<%dh' % n_vis, d, r[12])) if n_vis else []
        if ch & 0x1000:
            t['vertex'] = self._ptr(r[10], 16)
        if ch & 0x10000:
            t['uv'] = self._ptr(r[11], 20)
        if ch & 0x200:
            start, frames, _, table = struct.unpack_from('<4I', d, self._ptr(r[9], 16))
            self._ptr(table, 4 * frames)
            t['path'] = {'start': start, 'percent': struct.unpack_from('<%df' % frames, d, table)}
        if ch & 0x4000:
            t['raw_vertices'] = struct.unpack_from('<I', d, t['vertex'] + 4)[0]
        t['ease'] = [d[r[13] + 8 * i:r[13] + 8 * i + 8]
                     for i in range(max([k[1] for k in t.get('rotation', ())] + [-1]) + 1)]
        if t['ease']:
            self._ptr(r[13], 8 * len(t['ease']))
        return t

    def check(self):
        """Raise ValueError on anything inconsistent."""
        sums = [sum(1 for t in self.tracks if 'position' in t), sum(len(t.get('scale', ())) for t in self.tracks),
                sum(len(t.get('rotation', ())) for t in self.tracks), sum(1 for t in self.tracks if 'path' in t),
                sum(t.get('raw_vertices', 0) for t in self.tracks)]
        totals = [self.n_curves, self.n_scale, self.n_rot, self.n_paths, self.n_raw_vertices]
        if sums != totals:
            raise ValueError('header totals %s, tracks %s' % (totals, sums))
        for mat, keys in self.material:
            if [k[0] for k in keys] != sorted(k[0] for k in keys):
                raise ValueError('material %d keys out of order' % mat)
        for t in self.tracks:
            for name in ('rotation', 'scale'):
                frames = [k[0] for k in t.get(name, ())]
                if frames != sorted(frames):
                    raise ValueError('track %d %s frames out of order' % (t['index'], name))
            for k in t.get('rotation', ()):
                if abs(math.sqrt(sum(c * c for c in k[2:])) - 1) > 0.01:
                    raise ValueError('track %d: rotation key at frame %d is not a unit quaternion' % (t['index'], k[0]))
            if 'position' in t:
                p = t['position']
                if p['frames'] != sorted(p['frames']):
                    raise ValueError('track %d position frames out of order' % t['index'])
                if p['kind'] & 2 and len(p['points']) != 3 * (len(p['frames']) - 1) + 1:
                    raise ValueError('track %d: %d Bezier points for %d keys' % (t['index'], len(p['points']), len(p['frames'])))


def describe(t, verbose):
    names = [n for bit, n in CHANNELS if t['channels'] & bit]
    rest = t['channels'] & ~sum(bit for bit, _ in CHANNELS)
    parts = ['track %d node %d: %s%s' % (t['index'], t['node'], '+'.join(names), ' (+%#x)' % rest if rest else '')]
    if 'position' in t:
        p = t['position']
        parts.append('position %s, %d keys, %d points' % (curve_kind(p['kind']), len(p['frames']), len(p['points'])))
    if 'rotation' in t:
        parts.append('%d rotation keys' % len(t['rotation']))
    if 'scale' in t:
        parts.append('%d scale keys' % len(t['scale']))
    if 'visibility' in t:
        parts.append('visibility %s' % t['visibility'])
    if 'path' in t:
        p = t['path']['percent']
        parts.append('path from frame %d, %d frames, %.1f%%..%.1f%%' % (t['path']['start'], len(p), min(p), max(p)))
    if t['ease']:
        parts.append('%d ease tables' % len(t['ease']))
    lines = [', '.join(parts)]
    if verbose:
        for k in t.get('rotation', ()):
            lines.append('    rot   @%-4d %s  ease %d' % (k[0], ' '.join('%7.4f' % c for c in k[2:]), k[1]))
        for k in t.get('scale', ()):
            lines.append('    scale @%-4d %s' % (k[0], ' '.join('%7.4f' % c for c in k[2:])))
        if 'position' in t:
            p = t['position']
            lines.append('    pos   frames %s' % p['frames'])
            for pt in p['points']:
                lines.append('          %s' % ' '.join('%9.3f' % c for c in pt))
        for i, e in enumerate(t['ease']):
            lines.append('    ease %d: %s' % (i, ' '.join('%3d' % v for v in e)))
    return lines


def ease(table, t):
    """The eased fraction for segment fraction t (0 <= t < 1) through an 8-byte ease table."""
    points = [0.0] + [v / 255 for v in table] + [1.0]
    u = t * 8.999995
    i = int(u)
    return points[i] + (points[i + 1] - points[i]) * (u - i)


def inputs(path):
    if os.path.isdir(path):
        for root, _, files in os.walk(path):
            for f in sorted(files):
                if f.lower().endswith(('.wad', '.md2')):
                    yield from inputs(os.path.join(root, f))
    elif path.lower().endswith('.wad'):
        for name, data, _ in read_wad(path):
            if name.lower().endswith('.md2'):
                yield '%s:%s' % (path, name), data
    else:
        yield path, open(path, 'rb').read()


def has_motion(data):
    if len(data) < 0xb4:
        return False
    w = struct.unpack_from('<45I', data)
    return w[0] == MAGIC and w[2] == 203 and w[0x26] != 0


def main():
    args = sys.argv[1:]
    check = '--check' in args
    verbose = '-v' in args
    args = [a for a in args if a not in ('--check', '-v')]
    if not args:
        sys.exit(__doc__)
    good = bad = 0
    for arg in args:
        for label, data in inputs(arg):
            if not has_motion(data):
                continue
            try:
                m = Motion(data)
                if check:
                    m.check()
            except ValueError as e:
                bad += 1
                print('%s: %s' % (label, e))
                continue
            good += 1
            if check:
                continue
            print('%s: %d frames, %d tracks, flags %#x%s' % (label, m.length, m.n_tracks, m.flags,
                                                           ', nodes %s' % m.nodes if m.nodes else ''))
            for t in m.tracks:
                for line in describe(t, verbose):
                    print('  ' + line)
            for mat, keys in m.material:
                print('  material %d: %s' % (mat, ' '.join('@%d=%d' % k for k in keys)))
    if check:
        print('%d motion blocks consistent, %d not' % (good, bad))


if __name__ == '__main__':
    main()
