#include "research.h"

#include <cstdint>

#include "../core/savegame.h"
#include "objectdefs.h"
#include "things.h"
#include "world.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const u8 *project(const u8 *lab, s32 index) { return lab + 0xc + index * 20; }
const u8 *group_of(const u8 *lab, u32 group) { return lab + 0x140c + group * 16; }
bool not_done(const u8 *p) { return !(at<float>(p, 0x10) >= at<float>(p, 0xc)); }

// The object control record of a definition's id (ObjectControls_Find, not quiet, in the game's table).
u8 *controls(const u8 *def) { return ObjectControls_Find(*reinterpret_cast<u8 *const *>(0x0080239c) + 0x1d943c, nullptr, at<u16>(def, 0), 0); }

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

u32 __fastcall ResearchGroup_Serialize(u8 *group, void *, Stream *stream, u32 save, s32 version, u32) {
    static const SaveSpec kFields[] = {{0x0, 4, 0x0075e254}, {0x4, 4, 0x0075e24c}, {0x8, 4, 0x0075e23c}, {0xc, 1, 0x0075e224}};
    if (version < 0 || version > 1)
        return 0;
    return Save_Fields(stream, save, group, kFields, 4) ? 0 : 1;
}

u32 __fastcall ResearchLab_Serialize(u8 *lab, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    static const SaveSpec kFields[] = {{0x145c, 4, 0x0075e1dc}, {0x1460, 4, 0x0075e1d0}, {0x1464, 4, 0x0075e1b8}, {0x1468, 1, 0x0075e1ac}};
    if (Thing_Serialize(lab, nullptr, stream, save, version, unused))
        return 1;
    if (version < 0 || version > 1)
        return 0;
    for (u32 i = 0; i < 0x100; ++i)
        if (ResearchProject_Serialize(lab + 0xc + i * 0x14, nullptr, stream, save, version, unused))
            return 1;
    for (u32 i = 0; i < 5; ++i)
        if (ResearchGroup_Serialize(lab + 0x140c + i * 0x10, nullptr, stream, save, version, unused))
            return 1;
    return Save_Fields(stream, save, lab, kFields, 4) ? 0 : 1;
}

u8 __fastcall ResearchProject_SetNextUpgrade(u8 *project, void *, const u8 *def) {
    if (at<s32>(def, 0x17c) == 4) return 0;
    for (s32 level = at<s32>(controls(def), 0x14) + 1; level < 3; ++level) {
        const s32 cost = at<s32>(def, (level + 6) * 0x40);
        if (cost > 0) {
            at<u8>(project, 9) = (u8)level;
            at<u8>(project, 8) = 0;
            at<u32>(project, 4) = 4;
            at<float>(project, 0xc) = (float)cost;
            at<u32>(project, 0x10) = 0;
            at<u16>(project, 0) = at<u16>(def, 0);
            return 1;
        }
        at<s32>(controls(def), 0x14) = level;
    }
    return 0;
}

void __fastcall ResearchLab_FillProjects(u8 *lab) {
    u8 *const defs = reinterpret_cast<u8 *>(0x007890a0);
    for (u8 *def = ObjectDefs_First(defs); def; def = ObjectDefs_Next(defs)) {
        if (at<u32>(def, 0x508) || at<s32>(def, 0x4ac) >= 4) continue;
        if (at<s32>(def, 0x180)) {
            u8 *const p = lab + 0xc + at<s32>(lab, 0x145c) * 20;
            at<u16>(p, 0) = at<u16>(def, 0);
            at<u8>(p, 8) = at<u8>(def, 0x178);
            at<float>(p, 0xc) = (float)at<s32>(def, 0x180);
            at<u8>(p, 9) = 0;
            at<u32>(p, 4) = at<u32>(def, 0x17c);
            at<u32>(p, 0x10) = 0;
        } else {
            at<u8>(controls(def), 0x10) = 1;
            if (!ResearchProject_SetNextUpgrade(lab + 0xc + at<s32>(lab, 0x145c) * 20, nullptr, def)) continue;
        }
        const s32 best = at<s32>(lab, 0x1464);
        if (best < 0 || !(at<float>(project(lab, best), 0xc) >= at<float>(project(lab, at<s32>(lab, 0x145c)), 0xc)))
            at<s32>(lab, 0x1464) = at<s32>(lab, 0x145c);
        ++at<s32>(lab, 0x145c);
    }
    u32 effort = 0;
    for (u32 g = 0; g < 5; ++g) effort += at<u32>(group_of(lab, g), 0);
    for (u32 g = 0; g < 5; ++g)
        at<u32>(lab, 0x1410 + g * 16) = effort ? at<u32>(group_of(lab, g), 0) * 100u / effort * at<u32>(lab, 0x1460) / 100u : 0;
}

u8 __cdecl Research_AllDone() {
    u8 *const world = *reinterpret_cast<u8 *const *>(0x0080239c);
    const u8 *const lab = reinterpret_cast<const u8 *>(GameTable7cfb90_Get1da722_519550(world));
    if ((s32)ResearchLab_CountNotDone(lab, nullptr, 5) > 0) return 0;
    u8 *const defs = reinterpret_cast<u8 *>(0x007890a0);
    for (u8 *def = ObjectDefs_First(defs); def; def = ObjectDefs_Next(defs)) {
        if (at<s32>(def, 0x4ac) == 4) continue;
        u8 *const table = *reinterpret_cast<u8 *const *>(0x0080239c) + 0x1d943c;
        if (at<u32>(ObjectControls_FindUncached(table, nullptr, at<u16>(def, 0), 0), 0x18) == 0 && at<u32>(def, 0xc4) == 0) return 0;
    }
    return 1;
}
}
