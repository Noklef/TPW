"""Theme Park World particle effects: Data/Particle/Tp2.plb and loose .emt emitters.

Tp2.plb is the particle library. ParticleLib_Load 0x51f370 reads it once at start-up:
  u32 effect count (105; at most 0x69 are kept), u32 record size (0x140), effect records
  u32 effector count (20), u32 record size (0x68), effector records
  u32, u32   two engine settings (33, 1024; the first is raised to at least 10)
par_lib.h beside it names the indices (P_EFFECT_Sparks = 1, ..., E_EFFECT_Coll = 0, ...);
the same names are stored in the records. Effect numbers in Scape.omp, RSSE scripts and the
code index this table.

An .emt (Ptcl1.emt and Ptcl2.emt in jungle/features/speaker1.wad) is one effect record on its
own: a library effect with some fields changed (Ptcl2 is BeamUp moved and retuned).

Effect record (0x140 bytes). Particle_Spawn 0x521e60 copies it into a live slot (pool at
0x80cefc), so some fields are runtime state that the editor saved along with the rest:
  0x04 u8   sprite set for the particles        0x05 u8  bounds tracking flag
  0x0e u16  start mode (0 = emit the 0x60 burst at spawn, 1 = start on the next update)
  0x12 u16  random jitter added to the offset at 0x2c (per axis)
  0x14 i32  x, y, z position (world units * 64; Particle_Spawn sets it from its arguments)
  0x20 i32  emitter duration (copied to 0xd4 and counted down)
  0x2c i32  x, y, z offset (also where a child effect attaches)
  0x44 i32  spawn volume: radius (or half width) in x, height in y, in z
  0x58 u32  colour mode: 0 = walk the ramp with age, bit 0 = random ramp entry each frame,
            other non-zero = a random entry at birth
  0x60 i32  particles emitted at once at spawn
  0x6d u8   random starting rotation
  0x74 u16  particle size at birth     0x78 i32 particle lifetime (+ up to a quarter at random)
  0x7c i32  drag: each update velocity -= velocity * drag / 1024
  0x84 i32  x, y, z random velocity kick each update, up to +-n (0x6f is set at spawn when
            any is non-zero)
  0x96 u16  sprite frames played over a particle's life
  0x98 i32  effect spawned where a particle dies (-1 = none)
  0xa0 u16  y spawn offset is signed (else 0..height)
  0xa6 u16  particle size at death (size moves linearly from 0x74 to here)
  0xa8 u16  spawn on the rim of the volume only
  0xaa u8   particles die with the emitter
  0xac i32  rotation of the spawn volume (4096 = a full turn)
  0xb4 i32  child effect started with this one (-1 = none)
  0xb8 u16  child sits at its own 0x2c offset   0xba u8 child is an effector, not an effect
  0xc1 u8   optional: not spawned while Particles_SetReduced 0x51fd80 is on (set at start-up
            from an option at 0x78d8e0)
  0xc2 u8   spawn volume is a box (else a cylinder around y)
  0xd8 u32  colour ramp x 16, D3DCOLOR (bytes B, G, R, A): entry 15 at birth, 0 at death
  0x118 char name[16]
  runtime only: 0x00 0x02 0x06 0x08 0x0a 0x0c 0x6f 0xab 0xc0 0xd0 0xd2 0xd4, 0x128.. bounds
  the rest is not traced yet.

Effector record (0x68 bytes): forces that act on particles (Coll, UIAttr, UIRepel, KeyAttr,
UiBigRepel, ...). 0x18 i32 x, y, z offset; 0x50 char name[16]; the rest is not traced yet.

Usage:
  python -I tools/plb.py <Tp2.plb | file.emt | wad>...
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from wadtool import read_wad  # noqa: E402

EFFECT_SIZE = 0x140
EFFECTOR_SIZE = 0x68


def _name(raw):
    return raw.split(b'\0')[0].decode('latin-1')


def parse_effect(rec):
    i32 = lambda o: struct.unpack_from('<i', rec, o)[0]
    u16 = lambda o: struct.unpack_from('<H', rec, o)[0]
    return {
        'name': _name(rec[0x118:0x128]),
        'sprite_set': rec[0x04],
        'emit_mode': u16(0x0e),
        'jitter': u16(0x12),
        'position': struct.unpack_from('<3i', rec, 0x14),
        'duration': i32(0x20),
        'offset': struct.unpack_from('<3i', rec, 0x2c),
        'volume': struct.unpack_from('<3i', rec, 0x44),
        'colour_mode': struct.unpack_from('<I', rec, 0x58)[0],
        'burst': i32(0x60),
        'random_rotation': rec[0x6d],
        'size': (u16(0x74), u16(0xa6)),
        'lifetime': i32(0x78),
        'drag': i32(0x7c),
        'kick': struct.unpack_from('<3i', rec, 0x84),
        'sprite_frames': u16(0x96),
        'death_effect': i32(0x98),
        'signed_y': u16(0xa0),
        'rim_only': u16(0xa8),
        'die_with_emitter': rec[0xaa],
        'volume_angle': i32(0xac),
        'child': i32(0xb4),
        'child_at_offset': u16(0xb8),
        'child_is_effector': rec[0xba],
        'optional': rec[0xc1],
        'box': rec[0xc2],
        'ramp': [struct.unpack_from('<I', rec, 0xd8 + 4 * i)[0] for i in range(16)],
        'raw': rec,
    }


def parse_effector(rec):
    return {'name': _name(rec[0x50:0x60]), 'offset': struct.unpack_from('<3i', rec, 0x18), 'raw': rec}


def parse_plb(data):
    pos, tables = 0, []
    for size, parse in ((EFFECT_SIZE, parse_effect), (EFFECTOR_SIZE, parse_effector)):
        count, rec_size = struct.unpack_from('<II', data, pos)
        if rec_size != size:
            raise ValueError('record size 0x%x, expected 0x%x' % (rec_size, size))
        pos += 8
        tables.append([parse(data[pos + i * size:pos + (i + 1) * size]) for i in range(count)])
        pos += count * size
    settings = struct.unpack_from('<II', data, pos)
    if pos + 8 != len(data):
        raise ValueError('%d bytes left over' % (len(data) - pos - 8))
    return tables[0], tables[1], settings


def describe(e):
    parts = ['burst %d' % e['burst'] if e['burst'] and e['emit_mode'] == 0 else 'stream',
             'life %d' % e['lifetime'], 'size %d->%d' % e['size'],
             'volume %s%s' % ('x'.join(map(str, e['volume'])), ' box' if e['box'] else '')]
    if e['sprite_frames'] > 1:
        parts.append('%d frames' % e['sprite_frames'])
    if any(e['kick']):
        parts.append('kick %s' % (e['kick'],))
    if e['drag']:
        parts.append('drag %d' % e['drag'])
    if e['child'] >= 0:
        parts.append('child %s%d' % ('effector ' if e['child_is_effector'] else '', e['child']))
    if e['death_effect'] >= 0:
        parts.append('on death %d' % e['death_effect'])
    if e['optional']:
        parts.append('optional')
    return ', '.join(parts)


def inputs(path):
    if path.lower().endswith('.wad'):
        for name, data, _ in read_wad(path):
            if name.lower().endswith(('.plb', '.emt')):
                yield '%s:%s' % (path, name), data
    else:
        yield path, open(path, 'rb').read()


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    for path in sys.argv[1:]:
        for label, data in inputs(path):
            if len(data) == EFFECT_SIZE:
                e = parse_effect(data)
                print('%s: effect "%s" at %s: %s' % (label, e['name'], e['position'], describe(e)))
                print('  ramp ' + ' '.join('%08x' % c for c in e['ramp']))
                continue
            effects, effectors, settings = parse_plb(data)
            print('%s: %d effects, %d effectors, settings %s' % (label, len(effects), len(effectors), settings))
            for i, e in enumerate(effects):
                if e['name']:
                    print('  %3d %-15s %s' % (i, e['name'], describe(e)))
            for i, e in enumerate(effectors):
                if e['name']:
                    print('  effector %d %s offset %s' % (i, e['name'], e['offset']))


if __name__ == '__main__':
    main()
