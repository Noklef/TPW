#include "signapply.h"

#include <cstdint>
#include <cstring>

#include "../core/crt.h"
#include "../core/gamewindow.h"
#include "../core/staticobjects.h"

namespace {

AddrCalls g_calls;

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T slot(u32 address) { return *reinterpret_cast<const T *>((uintptr_t)address); }
u32 address(const void *p) { return (u32)(uintptr_t)p; }

// The part whose faces name this entry, else null.
u8 *partOf(const u8 *model, u32 entry) {
    u8 *part = at<u8 *>(model, 0x70);
    for (u32 parts = at<u16>(model, 0x42); parts; --parts, part += 0xa0) {
        const u8 *face = at<const u8 *>(part, 0x6c);
        for (u32 faces = at<u16>(part, 0x5a); faces; --faces, face += 0x10)
            if (at<u32>(face, 0) == entry) return part;
    }
    return nullptr;
}

// RideBase_TidyUVs of this part, or with none of each part without flag 0x200.
void tidy(u8 *object, u8 *part, u32 *warned) {
    const u32 tolerance = 0x3ca3d70au;   // (0.02f)
    u8 *model = at<u8 *>(at<u8 *>(object, 8), 4);
    *warned = 0;
    if (part) {
        g_calls.Cd(0x00467310, address(model), address(part), tolerance, 0u, 0u, address(warned));
        return;
    }
    for (u32 i = 0; i < at<u16>(model, 0x42); ++i) {
        const u32 main = at<u16>(model, 0x44);
        u8 *one = i < main ? at<u8 *>(model, 0x70) + i * 0xa0 : at<u8 *>(model, 0x74) + (i - main) * 0x58;
        if (!(at<u32>(one, 0) & 0x200)) g_calls.Cd(0x00467310, address(model), address(one), tolerance, 0u, 0u, address(warned));
    }
}

u32 draw(u8 *renderer, u8 *object, const char *folder, const char *style, const char *fonts, const u16 *text, const u16 *text2) {
    u16 lines[2][0x100];
    u8 *model = at<u8 *>(at<u8 *>(object, 8), 4);
    u8 *record = at<u8 *>(model, 0x54);
    u8 *entry = at<u8 *>(model, 0x50);
    u8 *records[2] = {nullptr, nullptr}, *entries[2] = {nullptr, nullptr}, *textures[2] = {nullptr, nullptr};
    s32 indices[2] = {-1, -1};
    u8 *block = nullptr;
    u32 found = 0;
    bool savedPairs = false;

    if (text2) {
        Crt().wcscpy(lines[0], text);
        Crt().wcscpy(lines[1], text2);
    } else {
        bool split = false;
        const u32 length = Crt().wcslen(text);
        const u32 half = length >> 1;
        textures[1] = reinterpret_cast<u8 *>((uintptr_t)length);   // sic (what the stack keeps; see the header)
        for (u32 i = 0; i < length && !split; ++i) {
            const u32 from = i & 1 ? half - ((i + 1) >> 1) : half + ((i + 1) >> 1);
            textures[0] = reinterpret_cast<u8 *>(const_cast<u16 *>(text + from));   // sic
            if (text[from] == 0x20) {
                Crt().wcsncpy(lines[0], text, from);
                lines[0][from] = 0;
                Crt().wcsncpy(lines[1], text + from + 1, length - from);
                split = true;
            }
        }
        if (!split) {
            Crt().wcscpy(lines[0], text);
            lines[1][0] = 0;
        }
    }

    for (u32 i = 0; i < at<u16>(model, 0x36); ++i, record += 0x10, entry += 8) {
        const u32 flags = at<u32>(record, 0);
        if (flags & 0x18) {
            const u32 side = flags & 8 ? 0 : 1;
            at<u8>(entry, 1) |= 4;
            indices[side] = (s32)i;
            u8 *texture = at<u8 *>(record, 0xc);
            ++found;
            if (!(at<u8>(texture, 0) & 2)) savedPairs = true;
            textures[side] = texture;
            records[side] = record;
            entries[side] = entry;
        } else if (!(flags & 2) && at<u16>(record, 0xa) == 1) {
            const char *name = at<const char *>(record, 0xc);
            u32 side;
            if (Crt().stricmp(name, "sign1.tga") == 0)
                side = 0;
            else if (Crt().stricmp(name, "sign2.tga") == 0)
                side = 1;
            else
                continue;
            if (indices[side] != -1) continue;
            indices[side] = (s32)i;
            if (!block) block = reinterpret_cast<u8 *>((uintptr_t)g_calls.Cd(0x0045b9d0, 0x2cu, 1u));
            u8 *texture = block + (side ? 0x16 : 0);
            at<u16>(texture, 6) = 0x80;
            at<u16>(texture, 8) = 0x80;
            at<u8 *>(record, 0xc) = texture;
            at<u32>(record, 0) |= side ? 0x12 : 0xa;
            at<u16>(entry, 0) |= 0x480;
            ++found;
            textures[side] = texture;
            records[side] = record;
            entries[side] = entry;
            savedPairs = true;
        }
    }

    bool ok = found == 2;
    if (ok && savedPairs) {
        u8 *holder = at<u8 *>(object, 8);
        const u32 owner = at<u32>(holder, 0x1c) ? at<u32>(holder, 0x1c) : at<u32>(holder, 0x20);
        ok = g_calls.Th(0x0057b120, owner, address(indices), 2u) != 0;
        at<s32>(textures[0], 0x12) = indices[0];
        at<s32>(textures[1], 0x12) = indices[1];
        at<u32>(textures[0], 0) |= 2;
        at<u32>(textures[1], 0) |= 2;
    }
    if (!ok) {
        for (u32 side = 0; side < 2; ++side) {
            if (records[side]) at<u32>(records[side], 0) &= ~2u;
            if (entries[side]) at<u16>(entries[side], 0) &= 0xff7f;
        }
        if (block) g_calls.Cd(0x0045ba00, address(block));
        return 0;
    }

    u8 *first = partOf(model, address(entries[0]));
    u8 *second = partOf(model, address(entries[1]));
    u32 warned;
    tidy(object, first, &warned);
    if (second != first) tidy(object, second, &warned);

    char base[0x104], path[0x104];
    std::strcpy(base, folder);
    const size_t length = std::strlen(base);
    if (length && base[length - 1] == '\\') base[length - 1] = 0;   // (an empty folder: the original tests and clears a stack byte nothing reads)
    const char *name = style;
    if (!name) {
        const char *last = Crt().strrchr(base, 0x5c);
        name = last ? last + 1 : base;
    }
    const char *language = reinterpret_cast<const char *>((uintptr_t)g_calls.Th(0x0041a820, (u32)(uintptr_t)&gLanguage_78b530));
    Crt().sprintf(path, "%s\\%s_%s.sgn", base, language, name);
    if (!g_calls.Cd(0x0046f120, address(path))) Crt().sprintf(path, "%s\\%s.sgn", base, name);
    g_calls.Th(0x005ec3a0, address(renderer), address(path));
    g_calls.Th(0x005ec680, address(renderer), address(fonts));

    const u32 mode = global<u32>((u32)(uintptr_t)&gTexSettings_8bd550 + 0x4);
    const u32 params[7] = {address(lines[0]), address(lines[1]), at<u32>(textures[0], 0x12), at<u32>(textures[1], 0x12), 0x80u, 0x80u, mode == 2 ? 2u : mode == 3 ? 1u : 2u};
    const u32 dc = slot<u32(__stdcall *)(u32)>(0x006fd34c)(gGamewindow_7a130c);   // GetDC
    g_calls.Th(0x005ecb40, address(renderer), dc, address(params));
    slot<u32(__stdcall *)(u32, u32)>(0x006fd348)(gGamewindow_7a130c, dc);   // ReleaseDC

    at<u32>(model, 0x30) |= 0x8000;
    if (!(at<u32>(object, 4) & 1)) at<u32>(object, 4) |= 0x8000000;
    return 1;
}

}  // namespace

void Sign_SetApplyCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

u32 __cdecl Sign_Apply(u8 *object, const char *folder, const char *style, const char *fonts, const u16 *text, const u16 *text2) {
    u8 renderer[0x460];
    g_calls.Th(0x005ebfc0, address(renderer));
    const u32 result = text ? draw(renderer, object, folder, style, fonts, text, text2) : 0;
    g_calls.Th(0x005ec220, address(renderer));
    return result;
}

extern "C" u32 __cdecl Sign_ApplyObject(u8 *object, const u16 *text, const u16 *text2) {
    const u32 index = at<u32>(object, 0x20);
    u8 *const definition = reinterpret_cast<u8 *>((uintptr_t)g_calls.Th(0x004dd4e0, address(object)));   // Object_GetDef
    if (!definition || !index) return 0;
    const char *const path = at<const char *>(reinterpret_cast<const void *>((uintptr_t)&gStrings5_f816c0), 8);
    if (!path) return 0;
    char fonts[0x104];
    Crt().sprintf(fonts, path[std::strlen(path) - 1] == '\\' ? "%s%s" : "%s\\%s", path, "Data\\Fonts");   // (an empty path reads the byte before it, sic)
    u8 *const holder = global<u8 *>(0x007a4610 + 4 * index);
    u32 result = 1;
    if (global<u32>((u32)(uintptr_t)&gOptions_78d8d8)) {
        u8 wide[0x14], name[0x14];
        g_calls.Th(0x005f6980, address(wide), address(definition + 0x390));   // WString_CtorStr
        g_calls.Th(0x005f61a0, address(name));   // String_Ctor
        if (g_calls.Cd(0x00424db0, name, wide))   // String_AssignFromWide
            result = g_calls.Cd(0x00467a80, holder, definition + 0x28c, at<u32>(name, 8), fonts, text, text2);   // Sign_Apply
        else
            result = 0;   // (the original returns a stack slot it never set, sic)
        g_calls.Th(0x005f61e0, address(name));   // String_Dtor
        g_calls.Th(0x005f6a60, address(wide));   // WString_Dtor
    }
    if (holder && !(at<u32>(holder, 4) & 1))
        at<u32>(holder, 4) |= 0x8000000;
    return result;
}

namespace {
AddrCalls g_rideCalls;
}  // namespace

void Sign_SetRideCalls(const AddrCall *table, u32 count) { g_rideCalls.Set(table, count); }

extern "C" u32 __cdecl Ride_ApplySign(u32 id, u32 a1, u32 a2, u32 a3, u32 a4) {
    if (!id) return 0;
    const char *const folder = at<const char *>(reinterpret_cast<const void *>((uintptr_t)&gStrings5_f816c0), 8);
    if (!folder) return 0;
    char fonts[0x104];
    Crt().sprintf(fonts, folder[std::strlen(folder) - 1] == '\\' ? "%s%s" : "%s\\%s", folder, "Data\\Fonts");   // (an empty folder reads the byte before it, sic)
    const u32 ride = global<u32>(0x007a4610 + 4 * id);
    u32 result = 1;
    if (global<u32>((u32)(uintptr_t)&gOptions_78d8d8)) result = g_rideCalls.Cd(0x467a80, ride, a1, a2, address(fonts), a3, a4);
    if (ride && !(at<u32>((void *)(uintptr_t)ride, 4) & 1)) at<u32>((void *)(uintptr_t)ride, 4) |= 0x8000000;
    return result;
}

extern "C" void __cdecl Rides_ReapplySigns() {
    u32 index = at<u16>(global<u8 *>(0x007cf83c), 0x1da746);
    while (index) {
        u8 *const thing = global<u8 *>(0x007cfb90 + index * 0x14);
        u8 *const holder = global<u8 *>(0x007a4610 + 4 * at<u32>(thing, 0x20));
        if (holder && (at<u32>(holder, 4) & 0x8000000)) {
            u16 first[0x100], second[0x100];
            g_rideCalls.Th(0x4e2860, address(thing), address(first), 0x100u);
            g_rideCalls.Th(0x4e2920, address(thing), address(second), 0x100u);
            g_rideCalls.Cd(0x4682b0, address(thing), address(first), address(second));
        }
        index = at<u16>(thing, 0xc);
    }
}
