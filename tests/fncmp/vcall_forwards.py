# The methods that hand the call to a virtual method of an inner object, against the originals: 0x47f990 (src/game/gfx/text; the object at +0x14, its call 8, a 16-bit answer), 0x590ff0 (gfx/texture;
# a counter raised, the device generation stored, the object at +0 and its call 5) and 0x569f20 (render/rasteriser; EMMS, returns 1) and 0x492320 (frontend/helpbg; the object's own call 0 when the word at +4 is set). The inner objects have vtables of recorders (tools/fncmp.py recorder()),
# so the comparison covers the slot, the object it is called on and the answer.
# usage: vcall_forwards.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Text,Texture,Emms,HelpBg)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, LOG, RAM, log_region, put, set_return, vtable  # noqa: E402

CALLS = {'Text': 0x47f990, 'Texture': 0x590ff0, 'Emms': 0x569f20, 'HelpBg': 0x492320}
B = Bench([(0x47f990, 0x47f9a0), (0x590ff0, 0x59100d), (0x569f20, 0x569f28), (0x492320, 0x49232c)])
INNER, VT = RAM + 0x100, RAM + 0x200


def case(r):
    op = r.choice(OPS)
    if op == 'Emms':
        return Call(CALLS[op], [], {}, conv='c', ret='a')
    obj = bytearray(r.bytes(0x40))
    log = log_region()
    regions = {RAM: obj, INNER: put(bytearray(r.bytes(0x20)), 0, 'I', VT), VT: vtable({}, size=16), LOG: log}
    if op == 'HelpBg':   # (a tail call: what EAX holds when the word is clear is the caller's, so only the call is compared)
        put(obj, 0, 'I', VT)
        put(obj, 4, 'H', r.choice([0, 0, 1, 0x8000, r.getrandbits(16)]))
        return Call(CALLS[op], [RAM], regions, conv='t', ret='v')
    if op == 'Text':
        put(obj, 0x14, 'I', r.choice([INNER, INNER, 0]))
        set_return(log, 8, r.getrandbits(32))
        return Call(CALLS[op], [RAM], regions, conv='t', ret='w')
    put(obj, 0, 'I', INNER)
    regions[0x8c4b60] = r.bytes(4)
    regions[0x8bcc40] = r.bytes(4)
    set_return(log, 5, r.getrandbits(32))
    return Call(CALLS[op], [RAM], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
