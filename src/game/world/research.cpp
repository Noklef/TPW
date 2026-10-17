#include "research.h"

#include <cstdint>

#include "../core/savegame.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const u8 *project(const u8 *lab, s32 index) { return lab + 0xc + index * 20; }
const u8 *group_of(const u8 *lab, u32 group) { return lab + 0x140c + group * 16; }
bool not_done(const u8 *p) { return !(at<float>(p, 0x10) >= at<float>(p, 0xc)); }

}  // namespace

extern "C" {

u32 __fastcall ResearchLab_GroupEffort(const u8 *lab, void *, u32 group) { return at<u32>(group_of(lab, group), 0); }

u16 *__fastcall ResearchLab_CurrentObjId(const u8 *lab, void *, u16 *out, u32 group) {
    const s32 current = at<s32>(group_of(lab, group), 8);
    *out = current < 0 ? 0 : at<u16>(project(lab, current), 0);
    return out;
}

u32 __fastcall ResearchLab_CountNotDone(const u8 *lab, void *, u32 cat) {
    u32 count = 0;
    // From the last project down (the count is the same either way).
    for (s32 i = (s32)(at<u32>(lab, 0x145c) - 1); i >= 0; --i) {
        const u8 *const p = project(lab, i);
        if ((cat == 5 || at<u32>(p, 4) == cat) && not_done(p)) ++count;
    }
    return count;
}

u8 __fastcall ResearchLab_CurrentNotDone(const u8 *lab, void *, u32 group) {
    const s32 current = at<s32>(group_of(lab, group), 8);
    return current >= 0 && not_done(project(lab, current));
}

u8 *__fastcall ResearchGroup_Ctor(u8 *group) {
    at<u8>(group, 0xc) = 1;
    at<s32>(group, 8) = -1;
    return group;
}

u32 __fastcall ResearchProject_Serialize(u8 *project, void *, Stream *stream, u32 save, s32 version, u32) {
    static const SaveSpec kFields[] = {{0x0, 2, 0x0075e21c}, {0x4, 4, 0x0075e214}, {0x8, 1, 0x0075e204},
                                       {0x9, 1, 0x0075b3d0}, {0xc, 4, 0x0075e1fc}, {0x10, 4, 0x0075e1ec}};
    if (version < 0 || version > 1)
        return 0;
    return Save_Fields(stream, save, project, kFields, 6) ? 0 : 1;
}
}
