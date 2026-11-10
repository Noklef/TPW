#include "crtinput.h"

#include <cstdint>
#include <cstring>

#include "crt.h"
#include "crtfloat.h"
#include "crtstream.h"
#include "initializers.h"

namespace {

using u64 = unsigned long long;

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// _pctype[c] & mask as _input tests it: by _isctype when characters may be longer.
s32 ctype(s32 c, s32 mask) { return CRT_mb_cur_max > 1 ? CRT_isctype(c, mask) : CRT_pctype[c] & mask; }

bool lead(s32 c) { return (CRT_pctype[c & 0xff] & 0x8000) != 0; }

void set(u8 *table, u32 c) { table[c >> 3] |= static_cast<u8>(1 << (c & 7)); }

}  // namespace

extern "C" {

s32 __cdecl CRT_input(CrtFile *file, const u8 *format, char *args) {
    u8 table[32];
    char floatstring[0x160];
    char temp[2] = {0, 0};
    u16 wctemp = 0;
    s32 charcount = 0;
    s32 count = 0;
    u8 match = 0;
    s32 ch = 0;   // (sic: uninitialised in the original, which tests it when the format is empty or starts with %n)
    s32 comchr = 0;
    u32 number = 0;
    u64 num64 = 0;
    char *pointer = nullptr;
    char *arglistsave = nullptr;
    char *start = nullptr;
    char *scan = nullptr;
    const u8 *scanptr = nullptr;
    u8 prevchar = 0, done_flag = 0, suppress = 0, negative = 0, reject = 0, fl_wchar_arg = 0;
    s8 widechar = 0, longone = 1;
    s32 width = 0, widthset = 0, started = 0, integer64 = 0;
    const auto inc = [&]() {
        ++charcount;
        return CRT_input_inc(file);
    };
    const auto un_inc = [&](s32 c) {
        --charcount;
        CRT_input_un_inc(c, file);
    };

    while (*format) {
        if (ctype(*format, 8)) {
            --charcount;
            CRT_input_un_inc(CRT_whiteout(&charcount, file), file);
            while (Crt().isspace(*++format)) {
            }
        }
        if (*format == '%') {
            number = 0;
            prevchar = 0;
            width = widthset = started = 0;
            fl_wchar_arg = done_flag = suppress = negative = reject = 0;
            widechar = 0;
            longone = 1;
            integer64 = 0;
            do {
                comchr = *++format;
                if (ctype(comchr, 4)) {
                    ++widthset;
                    width = static_cast<s32>(static_cast<u32>(width) * 10 + static_cast<u32>(comchr) - '0');
                    continue;
                }
                switch (comchr) {
                case 'F':
                case 'N': break;
                case 'h':
                    --longone;
                    --widechar;
                    break;
                case 'I':
                    if (format[1] == '6' && format[2] == '4') {
                        format += 2;
                        ++integer64;
                        num64 = 0;
                        break;
                    }
                    ++done_flag;
                    break;
                case 'L': ++longone; break;
                case 'l':
                    ++longone;
                    ++widechar;
                    break;
                case 'w': ++widechar; break;
                case '*': ++suppress; break;
                default: ++done_flag; break;
                }
            } while (!done_flag);
            if (!suppress) {
                arglistsave = args;
                pointer = *reinterpret_cast<char **>(args);
                args += 4;
            }
            done_flag = 0;
            if (!widechar) widechar = (*format == 'S' || *format == 'C') ? 1 : -1;
            comchr = static_cast<u8>(*format | 0x20);
            if (comchr != 'n') ch = (comchr == 'c' || comchr == '{') ? inc() : CRT_whiteout(&charcount, file);
            if (widthset && !width) goto unget_error;
            switch (comchr) {
            case 'c':
                if (!widthset) {
                    ++widthset;
                    ++width;
                }
                if (widechar > 0) fl_wchar_arg = 1;
                scanptr = reinterpret_cast<const u8 *>((uintptr_t)0x00781718);
                reject = 0xff;
                goto scanit;
            case 's':
                if (widechar > 0) fl_wchar_arg = 1;
                scanptr = reinterpret_cast<const u8 *>((uintptr_t)0x00781710);
                reject = 0xff;
                goto scanit;
            case '{':
                if (widechar > 0) fl_wchar_arg = 1;
                scanptr = ++format;
                if (*scanptr == '^') {
                    ++scanptr;
                    reject = 0xff;
                }
            scanit:
                std::memset(table, 0, sizeof table);
                if (comchr == '{' && *scanptr == ']') {
                    prevchar = ']';
                    ++scanptr;
                    table[']' >> 3] = 1 << (']' & 7);
                }
                // (sic: only ']' ends the set, so a format cut short reads on; the original's test for its end comes after)
                while (*scanptr != ']') {
                    u8 rngch = *scanptr++;
                    if (rngch != '-' || !prevchar || *scanptr == ']') {
                        prevchar = rngch;
                        set(table, rngch);
                    } else {
                        rngch = *scanptr++;
                        u8 last;
                        if (prevchar < rngch) {
                            last = rngch;
                        } else {
                            last = prevchar;
                            prevchar = rngch;
                        }
                        for (u32 c = prevchar; c <= last; ++c) set(table, c);
                        prevchar = 0;
                    }
                }
                if (!*scanptr) goto error_return;
                if (comchr == '{') format = scanptr;
                start = pointer;
                un_inc(ch);
                while (!widthset || width--) {
                    ch = inc();
                    if (ch == -1 || !((table[ch >> 3] ^ reject) & (1 << (ch & 7)))) {
                        un_inc(ch);
                        break;
                    }
                    if (suppress) {
                        ++start;
                    } else if (fl_wchar_arg) {
                        temp[0] = static_cast<char>(ch);
                        if (lead(ch)) temp[1] = static_cast<char>(inc());
                        CRT_mbtowc(&wctemp, temp, static_cast<u32>(CRT_mb_cur_max));
                        *reinterpret_cast<u16 *>(pointer) = wctemp;
                        pointer += 2;
                    } else {
                        *pointer++ = static_cast<char>(ch);
                    }
                }
                if (start == pointer) goto error_return;
                if (!suppress) {
                    ++count;
                    if (comchr != 'c') {
                        if (fl_wchar_arg) *reinterpret_cast<u16 *>(pointer) = 0;
                        else *pointer = 0;
                    }
                }
                break;
            case 'i':
                comchr = 'd';
                [[fallthrough]];
            case 'x':
                if (ch == '-') {
                    ++negative;
                    goto x_incwidth;
                } else if (ch == '+') {
                x_incwidth:
                    if (!--width && widthset) ++done_flag;
                    else ch = inc();
                }
                if (ch == '0') {
                    ch = inc();
                    if (static_cast<u8>(ch) == 'x' || static_cast<u8>(ch) == 'X') {
                        ch = inc();
                        comchr = 'x';
                    } else {
                        started = 1;
                        if (comchr != 'x') {
                            comchr = 'o';
                        } else {
                            un_inc(ch);
                            ch = '0';
                        }
                    }
                }
                goto getnum;
            case 'p':
                longone = 1;
                [[fallthrough]];
            case 'o':
            case 'u':
            case 'd':
                if (ch == '-') {
                    ++negative;
                    goto d_incwidth;
                } else if (ch == '+') {
                d_incwidth:
                    if (!--width && widthset) ++done_flag;
                    else ch = inc();
                }
            getnum:
                if (integer64) {
                    while (!done_flag) {
                        if (comchr == 'x') {
                            if (ctype(ch, 0x80)) {
                                num64 <<= 4;
                                ch = static_cast<s32>(CRT_hextodec(static_cast<u32>(ch)));
                            } else {
                                ++done_flag;
                            }
                        } else if (ctype(ch, 4)) {
                            if (comchr == 'o') {
                                if (ch < '8') num64 <<= 3;
                                else ++done_flag;
                            } else {
                                num64 *= 10;
                            }
                        } else {
                            ++done_flag;
                        }
                        if (!done_flag) {
                            ++started;
                            num64 += static_cast<u64>(static_cast<long long>(ch - '0'));
                            if (widthset && !--width) ++done_flag;
                            else ch = inc();
                        } else {
                            un_inc(ch);
                        }
                    }
                    if (negative) num64 = 0 - num64;
                } else {
                    while (!done_flag) {
                        if (comchr == 'x' || comchr == 'p') {
                            if (ctype(ch, 0x80)) {
                                number <<= 4;
                                ch = static_cast<s32>(CRT_hextodec(static_cast<u32>(ch)));
                            } else {
                                ++done_flag;
                            }
                        } else if (ctype(ch, 4)) {
                            if (comchr == 'o') {
                                if (ch < '8') number <<= 3;
                                else ++done_flag;
                            } else {
                                number *= 10;
                            }
                        } else {
                            ++done_flag;
                        }
                        if (!done_flag) {
                            ++started;
                            number += static_cast<u32>(ch - '0');
                            if (widthset && !--width) ++done_flag;
                            else ch = inc();
                        } else {
                            un_inc(ch);
                        }
                    }
                    if (negative) number = 0 - number;
                }
                if (!started) goto error_return;
                if (!suppress) {
                    ++count;
                    if (integer64) *reinterpret_cast<u64 *>(pointer) = num64;
                    else if (longone) *reinterpret_cast<u32 *>(pointer) = number;
                    else *reinterpret_cast<u16 *>(pointer) = static_cast<u16>(number);
                }
                break;
            case 'n':   // (the characters read; not counted. With I64 it stores the number instead, sic)
                if (!suppress) {
                    if (integer64) *reinterpret_cast<u64 *>(pointer) = num64;
                    else if (longone) *reinterpret_cast<u32 *>(pointer) = static_cast<u32>(charcount);
                    else *reinterpret_cast<u16 *>(pointer) = static_cast<u16>(charcount);
                }
                break;
            case 'e':
            case 'f':
            case 'g':
                scan = floatstring;
                if (ch == '-') {
                    *scan++ = '-';
                    goto f_incwidth;
                } else if (ch == '+') {
                f_incwidth:
                    --width;
                    ch = inc();
                }
                if (!widthset || width > 0x15d) width = 0x15d;
                while (ctype(ch, 4) && width--) {
                    ++started;
                    *scan++ = static_cast<char>(ch);
                    ch = inc();
                }
                if (static_cast<u8>(CRT_decimal_point[0]) == static_cast<u8>(ch) && width--) {
                    ch = inc();
                    *scan++ = CRT_decimal_point[0];
                    while (ctype(ch, 4) && width--) {
                        ++started;
                        *scan++ = static_cast<char>(ch);
                        ch = inc();
                    }
                }
                if (started && (ch == 'e' || ch == 'E') && width--) {
                    *scan++ = 'e';
                    ch = inc();
                    if (ch == '-') {
                        *scan++ = '-';
                        goto f_incwidth2;
                    } else if (ch == '+') {
                    f_incwidth2:
                        if (!width--) ++width;
                        else ch = inc();
                    }
                    while (ctype(ch, 4) && width--) {
                        ++started;
                        *scan++ = static_cast<char>(ch);
                        ch = inc();
                    }
                }
                un_inc(ch);
                if (!started) goto error_return;
                if (!suppress) {
                    ++count;
                    *scan = 0;
                    reinterpret_cast<void(__cdecl *)(s32, char *, char *)>((uintptr_t)CRT_cfltcvt_tab[2])(longone - 1, pointer, floatstring);
                }
                break;
            default:   // ('%' or any other character: matched as it is)
                if (*format != ch) goto unget_error;
                --match;
                if (!suppress) args = arglistsave;
                break;
            }
            ++match;
            ++format;
        } else {
            ch = inc();
            if (*format++ != ch) goto unget_error;
            if (lead(ch)) {
                const s32 ch2 = inc();
                if (*format++ != ch2) {
                    un_inc(ch2);
                    goto unget_error;
                }
                --charcount;
            }
        }
        if (ch == -1 && (*format != '%' || format[1] != 'n')) goto error_return;
    }
    goto error_return;
unget_error:
    un_inc(ch);
error_return:
    if (ch == -1 && !count && !match) return -1;
    return count;
}

s32 __cdecl CRT_sscanf(const char *text, const char *format, ...) {
    CrtFile file;
    file.ptr = const_cast<char *>(text);
    file.base = const_cast<char *>(text);
    file.count = static_cast<s32>(std::strlen(text));
    file.flags = 0x49;
    return CRT_input(&file, reinterpret_cast<const u8 *>(format), reinterpret_cast<char *>(&format + 1));
}
}
