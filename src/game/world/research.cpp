#include "research.h"

#include <cstdint>

#include "../core/globalsave.h"
#include "../core/savegame.h"
#include "../math/x87.h"
#include "advisor.h"
#include "advisorscore.h"
#include "messagecentre.h"
#include "objectdefs.h"
#include "things.h"
#include "thingscan.h"
#include "world.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const u8 *project(const u8 *lab, s32 index) { return lab + 0xc + index * 20; }
const u8 *group_of(const u8 *lab, u32 group) { return lab + 0x140c + group * 16; }
bool not_done(const u8 *p) { return !(at<float>(p, 0x10) >= at<float>(p, 0xc)); }

// The object control record of a definition's id (ObjectControls_Find, not quiet, in the game's table).
u8 *controls(const u8 *def) { return ObjectControls_Find(*reinterpret_cast<u8 *const *>(0x0080239c) + 0x1d943c, nullptr, at<u16>(def, 0), 0); }

// Whether the group can take the project up (the module comment in research.h).
bool available(const u8 *lab, u32 group, const u8 *p) {
    if (at<u32>(p, 4) != group || at<u8>(p, 8) > at<u8>(group_of(lab, group), 0xc) || !not_done(p)) return false;
    if (group != 4) return true;
    u8 *const defs = reinterpret_cast<u8 *>(0x007890a0);
    u16 id = at<u16>(ObjectDefs_Find(defs, nullptr, at<u16>(p, 0)), 0x284);
    if (!id) id = at<u16>(p, 0);
    bool ok = at<u8>(ObjectControls_Find(*reinterpret_cast<u8 *const *>(0x0080239c) + 0x1d943c, nullptr, id, 0), 0x10) != 0;
    if (at<s32>(ObjectDefs_Find(defs, nullptr, id), 0xc4) > 0 && !(u8)GlobalSave_HasKey(id)) ok = false;
    return ok;
}

// The group's next (or with back, previous) project it can take up, after (before) its current one, else the first (last) before (after)
// it; none: a current project that is done is dropped (-1).
template <bool back> void chooseProject(u8 *lab, u32 group) {
    u8 *const g = lab + 0x140c + group * 16;
    s32 chosen = -1;
    auto consider = [&](s32 i) {   // (true: stop there)
        if (!available(lab, group, project(lab, i))) return false;
        const s32 current = at<s32>(g, 8);
        if (back ? i > current : i < current) {
            if (chosen < 0) chosen = i;
            return false;
        }
        if (i == current) return false;
        chosen = i;
        return true;
    };
    if (back) {
        for (s32 i = (s32)(at<u32>(lab, 0x145c) - 1); i >= 0; --i)   // (a count of -2^31 runs from 2^31 - 1, sic)
            if (consider(i)) break;
    } else {
        for (s32 i = 0; i < at<s32>(lab, 0x145c); ++i)
            if (consider(i)) break;
    }
    if (chosen >= 0) {
        at<s32>(g, 8) = chosen;
        return;
    }
    const s32 current = at<s32>(g, 8);
    if (current >= 0 && !not_done(project(lab, current))) at<s32>(g, 8) = -1;
}

// The research lab: the game object's ([0x80239c]) entry GameTable7cfb90_Get1da722_519550.
const u8 *labOfGame() { return reinterpret_cast<const u8 *>(GameTable7cfb90_Get1da722_519550(*reinterpret_cast<u8 *const *>(0x0080239c))); }

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

u32 __fastcall ResearchLab_LeftPercent(const u8 *lab, void *, u32 group) {
    const s32 current = at<s32>(group_of(lab, group), 8);
    if (current < 0) return 0;
    const float dearest = at<float>(project(lab, at<s32>(lab, 0x1464)), 0xc);
    const u8 *const p = project(lab, current);
    if (!(at<float>(p, 0xc) > 0.1f) || !(dearest > 1.0f)) return 0;
    return (u32)(long long)((X87Reg(at<float>(p, 0xc)) - X87Reg(at<float>(p, 0x10))) * X87Reg(100.0) / X87Reg(dearest)).v;  // __ftol
}

void __fastcall ResearchLab_FlagProjectControls(const u8 *lab) {
    const u32 last = at<u32>(lab, 0x145c) - 1;
    if ((s32)last < 0) return;
    const u8 *p = lab + 0xc + last * 20;
    for (u32 left = last + 1; left != 0; --left, p -= 20) at<u8>(controls(p), 0x10) = 1;   // (none found faults, sic)
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

u32 __cdecl Research_CountNotDoneCat0() { return ResearchLab_CountNotDone(labOfGame(), nullptr, 0); }
u32 __cdecl Research_CountNotDoneCat1() { return ResearchLab_CountNotDone(labOfGame(), nullptr, 1); }
u32 __cdecl Research_CountNotDoneCat2() { return ResearchLab_CountNotDone(labOfGame(), nullptr, 2); }
u32 __cdecl Research_CountNotDoneCat3() { return ResearchLab_CountNotDone(labOfGame(), nullptr, 3); }

void __fastcall ResearchLab_NextProject(u8 *lab, void *, u32 group) { chooseProject<false>(lab, group); }

void __fastcall ResearchLab_PrevProject(u8 *lab, void *, u32 group) { chooseProject<true>(lab, group); }
void __fastcall ResearchLab_UpdateLevels(u8 *lab, void *, u32 cat) {
    u8 &highest = *const_cast<u8 *>(group_of(lab, cat) + 0xc);
    bool changed = false;
    for (;;) {
        u32 count = 0, done = 0;
        for (s32 i = at<s32>(lab, 0x145c) - 1; i >= 0; --i) {
            const u8 *p = project(lab, i);
            if (at<u32>(p, 4) != cat || (s32)at<u8>(p, 8) >= (s32)highest + 1) continue;
            ++count;
            if (!not_done(p)) ++done;
        }
        const u32 share = count ? done * 100 / count : 0;
        if (share < reinterpret_cast<const u32 *>(0x00785540)[highest] || highest >= 7) break;
        ++highest;
        changed = true;
    }
    if (!changed || !ResearchLab_CountNotDone(lab, nullptr, cat) || cat > 3) return;
    u8 message[0x18];   // (its bytes +2, +3 and +0x17 never written, sic)
    AdvisorMsg_Init(message, nullptr, 0x85 + cat, 0, 0, 0xffffffffu, 0, -1);
    Advisor_Post(reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da71c_5194d0(*reinterpret_cast<u8 *const *>(0x0080239c))), nullptr, message);
}

void __fastcall ResearchLab_Init(u8 *lab) {
    at<u32>(lab, 0x145c) = 0;
    at<u32>(lab, 0x1464) = 0xffffffff;
    at<u32>(lab, 0x1460) = *reinterpret_cast<const u32 *>(0x00785580);
    GameTable7cfb90_Get1da720_519510(*reinterpret_cast<u8 *const *>(0x0080239c));   // (into ECX for the stdcall count, which ignores it)
    at<u8>(lab, 0x1468) = Things_CountStaffOfKind(8) > 0;
    ResearchLab_FillProjects(lab);
    ResearchLab_ApplyEfforts(lab);
    MessageCentre_AddThing(MessageCentre_Instance(), nullptr, lab, 0x11);
}

void __fastcall ResearchLab_ApplyEfforts(u8 *lab) {
    const u32 *const settings = reinterpret_cast<const u32 *>(0x00785568);
    for (u32 cat = 0; cat < 5; ++cat) {
        const u32 setting = settings[cat];
        at<u32>(lab, 0x140c + cat * 16) = setting < 100 ? setting : 100;
        u32 effort = 0;
        for (u32 g = 0; g < 5; ++g) effort += at<u32>(group_of(lab, g), 0);
        for (u32 g = 0; g < 5; ++g)
            at<u32>(lab, 0x1410 + g * 16) = effort ? at<u32>(group_of(lab, g), 0) * 100u / effort * at<u32>(lab, 0x1460) / 100u : 0;
        const s32 n = at<s32>(lab, 0x145c);
        for (s32 i = 0; i < n; ++i) {
            const u8 *p = project(lab, i);
            if (at<u32>(p, 4) == cat && at<u8>(p, 8) <= at<u8>(group_of(lab, cat), 0xc) && not_done(p)) {
                at<s32>(lab, 0x1414 + cat * 16) = i;
                break;
            }
        }
        ResearchLab_UpdateLevels(lab, nullptr, cat);
    }
}
}
