# The tiny functions of src/game/core/getters_setters (written by tools/tinygen.py) against the originals: any
# object bytes, any global values, any arguments.
# usage: getters_setters.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

# (address, end, convention, stack arguments, returns, globals touched)
FUNCS = [
    (0x405520, 0x405560, 't', 0, 'v', []),
    (0x405560, 0x405570, 't', 0, 'b', []),
    (0x4056e0, 0x4056f0, 't', 0, 'a', []),
    (0x4145e0, 0x4145f0, 't', 0, 'a', []),
    (0x41ce80, 0x41ce90, 't', 0, 'a', []),
    (0x41ce90, 0x41cea0, 't', 0, 'a', []),
    (0x4217e0, 0x4217f0, 't', 0, 'a', []),
    (0x4217f0, 0x421800, 't', 1, 'a', []),
    (0x424ec0, 0x424ed0, 'c', 0, 'a', []),
    (0x429b50, 0x429b60, 't', 0, 'v', []),
    (0x47eaa0, 0x47eab0, 'c', 0, 'a', [8080264]),
    (0x4b4480, 0x4b4486, 'c', 0, 'a', [0x7cc2f0]),
    (0x48f690, 0x48f6a0, 't', 1, 'a', []),
    (0x4a8bf0, 0x4a8c00, 't', 0, 'v', []),
    (0x4bc420, 0x4bc430, 't', 0, 'v', []),
    (0x4c75f0, 0x4c7600, 't', 0, 'v', []),
    (0x4d19f0, 0x4d1a00, 't', 0, 'v', []),
    (0x4d5260, 0x4d5270, 't', 0, 'v', []),
    (0x4db7c0, 0x4db7d0, 't', 0, 'v', []),
    (0x4dd310, 0x4dd320, 't', 0, 'v', []),
    (0x4e03d0, 0x4e03e0, 't', 0, 'a', []),
    (0x4e11f0, 0x4e1200, 't', 0, 'v', []),
    (0x4e1200, 0x4e1210, 't', 0, 'v', []),
    (0x4e1210, 0x4e1220, 't', 0, 'v', []),
    (0x4e21b0, 0x4e21c0, 't', 0, 'b', []),
    (0x4e21e0, 0x4e21f0, 't', 1, 'a', []),
    (0x4e21f0, 0x4e2200, 't', 0, 'b', []),
    (0x4e5b90, 0x4e5bc0, 't', 0, 'v', []),
    (0x4f6080, 0x4f6090, 'c', 0, 'a', []),
    (0x4f8570, 0x4f8580, 't', 0, 'v', []),
    (0x4f8580, 0x4f8590, 't', 0, 'v', []),
    (0x4f8590, 0x4f85a0, 't', 0, 'v', []),
    (0x4fb320, 0x4fb330, 't', 0, 'v', []),
    (0x4ff4e0, 0x4ff4f0, 't', 0, 'v', []),
    (0x4ff4f0, 0x4ff500, 'c', 0, 'a', []),
    (0x503fe0, 0x504040, 't', 0, 'v', []),
    (0x506190, 0x5061a0, 't', 0, 'v', []),
    (0x5061a0, 0x5061b0, 'c', 0, 'a', []),
    (0x509350, 0x509360, 't', 0, 'v', []),
    (0x50b970, 0x50b980, 't', 0, 'v', []),
    (0x50c1f0, 0x50c200, 's', 1, 'a', []),
    (0x512c30, 0x512c40, 't', 0, 'v', []),
    (0x512c40, 0x512c50, 't', 0, 'v', []),
    (0x519410, 0x519420, 'c', 0, 'a', [7886212]),
    (0x519420, 0x519430, 'c', 0, 'a', [7886216]),
    (0x519430, 0x519440, 'c', 0, 'a', [7886220]),
    (0x519440, 0x519450, 'c', 0, 'a', [7886224]),
    (0x51a280, 0x51a290, 't', 0, 'a', []),
    (0x51c050, 0x51c060, 't', 0, 'a', []),
    (0x52f1f0, 0x52f200, 'c', 0, 'a', [8509348]),
    (0x52f840, 0x52f850, 'c', 0, 'a', [8498732]),
    (0x52f850, 0x52f860, 'c', 0, 'a', [8499044]),
    (0x52fb10, 0x52fb20, 'c', 1, 'a', [8500956]),
    (0x52fb20, 0x52fb30, 'c', 0, 'a', [8500956]),
    (0x52fb30, 0x52fb40, 'c', 1, 'a', [8514068]),
    (0x52fb40, 0x52fb50, 'c', 0, 'a', [8514068]),
    (0x52fb50, 0x52fb60, 'c', 1, 'a', [8498876]),
    (0x52fb60, 0x52fb70, 'c', 0, 'a', [8498876]),
    (0x52fb70, 0x52fb80, 'c', 1, 'a', [8500284]),
    (0x52fb80, 0x52fb90, 'c', 0, 'a', [8500284]),
    (0x52fb90, 0x52fba0, 'c', 1, 'a', [8499020]),
    (0x52fbc0, 0x52fbd0, 'c', 0, 'a', [8499020]),
    (0x52fbe0, 0x52fbf0, 'c', 0, 'a', [8522380]),
    (0x5508b0, 0x5508c0, 'c', 1, 'a', [8884360]),
    (0x550510, 0x550520, 'c', 1, 'a', [0x878128]),
    (0x564ee0, 0x564ef0, 't', 0, 'a', []),
    (0x565230, 0x565240, 't', 0, 'v', []),
    (0x569d00, 0x569d10, 's', 1, 'a', []),
    (0x56a430, 0x56a440, 's', 3, 'a', []),
    (0x56a440, 0x56a450, 's', 4, 'a', []),
    (0x56dd60, 0x56dd70, 't', 0, 'a', []),
    (0x591970, 0x591980, 't', 0, 'v', []),
    (0x591a70, 0x591a80, 't', 0, 'a', []),
    (0x59b540, 0x59b550, 't', 0, 'a', []),
    (0x59b860, 0x59b870, 't', 0, 'v', []),
    (0x5b0d60, 0x5b0d70, 't', 0, 'a', []),
    (0x5cec60, 0x5cec70, 'c', 0, 'a', []),
    (0x5d60c0, 0x5d60d0, 'c', 0, 'a', []),
    (0x5dc650, 0x5dc660, 't', 0, 'a', []),
    (0x5dc850, 0x5dc860, 't', 0, 'v', []),
    (0x5e26f0, 0x5e2700, 't', 0, 'v', []),
    (0x5e3bb0, 0x5e3bc0, 't', 0, 'v', []),
    (0x5e3bc0, 0x5e3bd0, 't', 0, 'v', []),
    (0x66f4d0, 0x66f4e0, 'c', 0, 'a', []),
    (0x691530, 0x691540, 'c', 0, 'a', [7872456]),
    (0x695af0, 0x695b00, 't', 0, 'b', []),
    (0x695b00, 0x695b10, 't', 0, 'b', []),
    (0x699ca0, 0x699cb0, 'c', 0, 'v', []),
    (0x69c8e0, 0x69c8f0, 't', 0, 'w', []),
    (0x69c8f0, 0x69c900, 't', 0, 'w', []),
    (0x6a8cc0, 0x6a8cd0, 't', 0, 'a', []),
    (0x6a9180, 0x6a9190, 't', 0, 'v', []),
    (0x6ad000, 0x6ad010, 't', 0, 'v', []),
    (0x6add30, 0x6add40, 't', 0, 'v', []),
    (0x6adf80, 0x6adf90, 't', 0, 'a', []),
    (0x6b4a50, 0x6b4a60, 't', 0, 'v', []),
    (0x6b4a90, 0x6b4aa0, 't', 0, 'v', []),
    (0x6b70d0, 0x6b70e0, 's', 2, 'v', []),
    (0x6b71c0, 0x6b71d0, 'c', 0, 'a', []),
    (0x6b9220, 0x6b9230, 't', 1, 'a', []),
    (0x6b9bd0, 0x6b9be0, 's', 2, 'a', []),
    (0x6baa00, 0x6baa10, 't', 1, 'a', []),
    (0x6bb190, 0x6bb1a0, 't', 1, 'a', []),
    (0x6bb1a0, 0x6bb1b0, 't', 0, 'a', []),
    (0x6bb1e0, 0x6bb1f0, 't', 0, 'v', []),
    (0x6bd790, 0x6bd7a0, 'c', 0, 'a', []),
    (0x6c54a0, 0x6c54b0, 's', 1, 'a', []),
    (0x6c54b0, 0x6c54c0, 's', 4, 'a', []),
    (0x6c5590, 0x6c55a0, 't', 0, 'a', []),
    (0x6c7090, 0x6c70a0, 't', 0, 'v', []),
    (0x6d1bc0, 0x6d1bd0, 'c', 1, 'a', [16459528]),
]
B = Bench([(a, e) for a, e, *_ in FUNCS])


def case(r):
    address, _end, conv, n, ret, globs = FUNCS[r.deal(range(len(FUNCS)))]
    regions = {RAM: r.bytes(0x400)}
    for g in globs:
        regions[g & ~0xf] = r.bytes(0x20)
    args = [r.choice([0, 1, 0xff, 0x7fffffff, 0xffffffff, r.getrandbits(32)]) for _ in range(n)]
    if conv == 't':
        args = [RAM + 0x80] + args
    return Call(address, args, regions, conv=conv, ret=ret)


if __name__ == '__main__':
    B.main(case)
