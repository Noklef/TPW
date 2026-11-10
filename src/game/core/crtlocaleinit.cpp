#include "crtlocaleinit.h"

#include <cstdint>

#include "crt.h"
#include "crtlocale.h"
#include "crtlocaleinfo.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// The code page's info as GetCPInfo fills it; two zero bytes past the lead ranges, where the original reads its frame (sic).
struct CodePageInfo {
    u32 max_size;
    u8 fallback[2];
    u8 lead[14];
};

}  // namespace

extern "C" {

s32 __cdecl CRT_get_lc_lconv(u32 *lconv) {
    // (kind, type, offset) in the order asked; the grouping (+0x1c) fixed after the fifth.
    static const u8 fields[][3] = {{1, 0x15, 0xc}, {1, 0x14, 0x10}, {1, 0x16, 0x14}, {1, 0x17, 0x18}, {1, 0x18, 0x1c}, {1, 0x50, 0x20}, {1, 0x51, 0x24}, {0, 0x1a, 0x28},
                                   {0, 0x19, 0x29}, {0, 0x54, 0x2a}, {0, 0x55, 0x2b}, {0, 0x56, 0x2c}, {0, 0x57, 0x2d}, {0, 0x52, 0x2e}, {0, 0x53, 0x2f}};
    const u32 lcid = CRT_lc_id[3].country;
    if (!lconv) return -1;
    u8 *const at = reinterpret_cast<u8 *>(lconv);
    s32 failed = 0;
    for (u32 i = 0; i < sizeof fields / sizeof fields[0]; ++i) {
        failed |= CRT_Getlocaleinfo(fields[i][0], lcid, fields[i][1], at + fields[i][2]);
        if (i == 4) CRT_fix_grouping(*reinterpret_cast<char **>(at + 0x1c));
    }
    return failed;
}

s32 __cdecl CRT_init_monetary() {
    u32 *&current = global<u32 *>(0x00781fc8);
    u32 *&owned = CRT_monetary_owned_fb18ac;
    const bool c_locale = !CRT_lc_handle[3];
    u32 *lconv = reinterpret_cast<u32 *>((uintptr_t)0x00781f98);
    if (!c_locale) {
        lconv = static_cast<u32 *>(CrtInner().calloc(1, 0x30));
        if (!lconv) return 1;
        if (CRT_get_lc_lconv(lconv)) {
            CRT_free_lconv_mon(lconv);
            Crt().free(lconv);
            return 1;
        }
    }
    for (u32 i = 0; i < 3; ++i) lconv[i] = current[i];
    current = lconv;
    CRT_free_lconv_mon(owned);
    Crt().free(owned);
    owned = c_locale ? nullptr : lconv;
    return 0;
}

s32 __cdecl CRT_init_ctype() {
    u16 *&ctype = CRT_pctype;
    u16 *&wctype = CRT_pwctype;
    void *&owned = CRT_ctype_owned_fb18b0;
    void *&owned_wide = CRT_ctype_wide_owned_fb18b4;
    if (!CRT_lc_handle[2]) {
        ctype = wctype = &CRT_ctype[1];
        Crt().free(owned);
        Crt().free(owned_wide);
        owned = owned_wide = nullptr;
        return 0;
    }
    // shortcut: the original also frees two uninitialised frame slots here (sic); not reproduced, the bench keeps this call succeeding.
    if (!CRT_lc_codepage && CRT_Getlocaleinfo(0, CRT_lc_id[2].language, 0xb, &CRT_lc_codepage)) return 1;
    u16 *const types = static_cast<u16 *>(CRT_malloc(0x202));
    u16 *const wide_types = static_cast<u16 *>(CRT_malloc(0x202));
    u8 *const bytes = static_cast<u8 *>(CRT_malloc(0x101));
    u16 *const units = static_cast<u16 *>(CRT_malloc(0x202));
    CodePageInfo info;
    info.lead[12] = info.lead[13] = 0;
    if (types && wide_types && bytes && units) {
        for (u32 i = 0; i < 0x100; ++i) bytes[i] = static_cast<u8>(i);
        if (reinterpret_cast<s32(__stdcall *const *)(u32, CodePageInfo *)>((uintptr_t)0x006fd180)[0](CRT_lc_codepage, &info) && info.max_size <= 2) {
            CRT_mb_cur_max = static_cast<s32>(info.max_size);
            if (info.max_size > 1) {
                for (const u8 *range = info.lead; range[0] && range[1]; range += 2) {
                    for (u32 c = range[0]; c <= range[1]; ++c) bytes[c] = 0;
                }
            }
            if (CRT_GetStringTypeA(1, reinterpret_cast<const char *>(bytes), 0x100, types + 1, 0, 0, 0)) {
                types[0] = 0;
                for (u32 i = 0; i < 0x100; ++i) units[i] = static_cast<u16>(i);
                if (CRT_GetStringTypeW(1, units, 0x100, wide_types + 1, 0, 0)) {
                    wide_types[0] = 0;
                    if (CRT_mb_cur_max > 1) {
                        for (const u8 *range = info.lead; range[0] && range[1]; range += 2) {
                            for (u32 c = range[0]; c <= range[1]; ++c) types[c + 1] = 0x8000;
                        }
                    }
                    ctype = types + 1;
                    wctype = wide_types + 1;
                    if (owned) Crt().free(owned);
                    owned = types;
                    if (owned_wide) Crt().free(owned_wide);
                    owned_wide = wide_types;
                    Crt().free(bytes);
                    Crt().free(units);
                    return 0;
                }
            }
        }
    }
    Crt().free(types);
    Crt().free(wide_types);
    Crt().free(bytes);
    Crt().free(units);
    return 1;
}
}
