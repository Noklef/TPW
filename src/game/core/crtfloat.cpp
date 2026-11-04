#include "crtfloat.h"

#include "crt.h"

#include <cstdint>
#include <cstring>

namespace {

void zero_mantissa(u32 *mantissa) {
    mantissa[0] = 0;
    mantissa[1] = 0;
    mantissa[2] = 0;
}

// (The shifts take the count as x86 does: its low five bits.)
u32 shl(u32 value, s32 count) { return value << (count & 31); }
u32 shr(u32 value, s32 count) { return value >> (count & 31); }

template <typename T> T &at(void *base, u32 offset) { return *reinterpret_cast<T *>(static_cast<u8 *>(base) + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

void set_infinity(u32 *x, u32 sign) {
    x[1] = 0;
    x[0] = 0;
    x[2] = (sign ? 0x80000000u : 0) + 0x7fff8000u;
}

bool is_digit(u8 c) {
    if (global<s32>(0x0077e7dc) > 1) return CRT_isctype(c, 4) != 0;
    return (global<const u8 *>(0x0077ea90)[c * 2u] & 4) != 0;
}

bool is_space(u8 c) {
    if (global<s32>(0x0077e7dc) > 1) return CRT_isctype(c, 8) != 0;
    return (global<const u8 *>(0x0077ea90)[c * 2u] & 8) != 0;
}

bool one_to_nine(char c) { return c >= '1' && c <= '9'; }
bool exponent_mark(char c) { return c == 'd' || c == 'D' || c == 'e' || c == 'E'; }

}  // namespace

extern "C" {

u32 __cdecl CRT_abstract_cw(u32 word) {
    u32 out = word & 1 ? 0x10 : 0;
    if (word & 4) out |= 8;
    if (word & 8) out |= 4;
    if (word & 0x10) out |= 2;
    if (word & 0x20) out |= 1;
    if (word & 2) out |= 0x80000;
    const u32 rounding = word & 0xc00;
    if (rounding == 0x400) out |= 0x100;
    else if (rounding == 0x800) out |= 0x200;
    else if (rounding == 0xc00) out |= 0x300;
    const u32 precision = word & 0x300;
    if (!precision) out |= 0x20000;
    else if (precision == 0x200) out |= 0x10000;
    if (word & 0x1000) out |= 0x40000;
    return out;
}

u32 __cdecl CRT_hw_cw(u32 word) {
    u32 out = word & 0x10 ? 1 : 0;
    if (word & 8) out |= 4;
    if (word & 4) out |= 8;
    if (word & 2) out |= 0x10;
    if (word & 1) out |= 0x20;
    if (word & 0x80000) out |= 2;
    const u32 rounding = word & 0x300;
    if (rounding == 0x100) out |= 0x400;
    else if (rounding == 0x200) out |= 0x800;
    else if (rounding == 0x300) out |= 0xc00;
    const u32 precision = word & 0x30000;
    if (!precision) out |= 0x300;
    else if (precision == 0x10000) out |= 0x200;
    if (word & 0x40000) out |= 0x1000;
    return out;
}

s32 __cdecl CRT_addl(u32 a, u32 b, u32 *sum) {
    const u32 total = a + b;
    *sum = total;
    return total < a || total < b;
}

s32 __cdecl CRT_ZeroTail(const u32 *mantissa, s32 bit) {
    s32 word = bit / 32;
    if (mantissa[word] & ~shl(0xffffffffu, 31 - bit % 32)) return 0;
    for (++word; word < 3; ++word)
        if (mantissa[word]) return 0;
    return 1;
}

s32 __cdecl CRT_IncMan(u32 *mantissa, s32 bit) {
    s32 word = bit / 32;
    s32 carry = CRT_addl(mantissa[word], shl(1, 31 - bit % 32), &mantissa[word]);
    for (--word; word >= 0 && carry; --word) carry = CRT_addl(mantissa[word], 1, &mantissa[word]);
    return carry;
}

s32 __cdecl CRT_RoundMan(u32 *mantissa, s32 bits) {
    s32 word = bits / 32;
    const s32 place = 31 - bits % 32;
    s32 carry = 0;
    if (mantissa[word] & shl(1, place) && !CRT_ZeroTail(mantissa, bits + 1)) carry = CRT_IncMan(mantissa, bits - 1);
    mantissa[word] &= shl(0xffffffffu, place);
    for (++word; word < 3; ++word) mantissa[word] = 0;
    return carry;
}

void __cdecl CRT_CopyMan(u32 *to, const u32 *from) {
    for (s32 i = 0; i < 3; ++i) to[i] = from[i];
}

s32 __cdecl CRT_IsZeroMan(const u32 *mantissa) {
    for (s32 i = 0; i < 3; ++i)
        if (mantissa[i]) return 0;
    return 1;
}

void __cdecl CRT_ShrMan(u32 *mantissa, s32 bits) {
    const s32 words = bits / 32;
    const s32 count = bits % 32;
    const u32 low = ~shl(0xffffffffu, count);
    u32 carried = 0;
    for (s32 i = 0; i < 3; ++i) {
        const u32 word = mantissa[i];
        mantissa[i] = shr(word, count) | carried;
        carried = shl(word & low, 32 - count);
    }
    for (s32 i = 2; i >= 0; --i) mantissa[i] = i >= words ? mantissa[i - words] : 0;
}

s32 __cdecl CRT_ld12cvt(const u8 *from, u32 *to, const s32 *format) {
    u16 first, top;
    u32 mantissa[3], kept[3];
    std::memcpy(&first, from, 2);
    std::memcpy(&mantissa[1], from + 2, 4);
    std::memcpy(&mantissa[0], from + 6, 4);
    std::memcpy(&top, from + 10, 2);
    mantissa[2] = static_cast<u32>(first) << 16;
    s32 exponent = (top & 0x7fff) - 0x3fff;
    s32 result = 0;
    s32 stored = 0;
    if (exponent == -0x3fff) {
        if (!CRT_IsZeroMan(mantissa)) {
            zero_mantissa(mantissa);
            result = 2;
        }
    } else {
        CRT_CopyMan(kept, mantissa);
        if (CRT_RoundMan(mantissa, format[2])) ++exponent;
        if (exponent < format[1] - format[2]) {
            zero_mantissa(mantissa);
            result = 2;
        } else if (exponent <= format[1]) {
            CRT_CopyMan(mantissa, kept);
            CRT_ShrMan(mantissa, format[1] - exponent);
            CRT_RoundMan(mantissa, format[2]);
            CRT_ShrMan(mantissa, format[3] + 1);
            result = 2;
        } else if (exponent >= format[0]) {
            zero_mantissa(mantissa);
            mantissa[0] |= 0x80000000;
            CRT_ShrMan(mantissa, format[3]);
            stored = format[5] + format[0];
            result = 1;
        } else {
            stored = format[5] + exponent;
            mantissa[0] &= 0x7fffffff;
            CRT_ShrMan(mantissa, format[3]);
        }
    }
    const u32 high = shl(static_cast<u32>(stored), 31 - format[3]) | (top & 0x8000 ? 0x80000000u : 0) | mantissa[0];
    if (format[4] == 0x40) {
        to[1] = high;
        to[0] = mantissa[1];
    } else if (format[4] == 0x20) {
        to[0] = high;
    }
    return result;
}

s32 __cdecl CRT_ld12tod(const u8 *from, u32 *to) { return CRT_ld12cvt(from, to, reinterpret_cast<const s32 *>((uintptr_t)0x00781728)); }

s32 __cdecl CRT_ld12tof(const u8 *from, u32 *to) { return CRT_ld12cvt(from, to, reinterpret_cast<const s32 *>((uintptr_t)0x00781740)); }

void __cdecl CRT_add_12(u32 *x, const u32 *y) {
    if (CRT_addl(x[0], y[0], &x[0]) && CRT_addl(x[1], 1, &x[1])) ++x[2];
    if (CRT_addl(x[1], y[1], &x[1])) ++x[2];
    CRT_addl(x[2], y[2], &x[2]);
}

void __cdecl CRT_shl_12(u32 *x) {
    const u32 low = x[0], middle = x[1];
    x[0] = low << 1;
    x[1] = middle << 1 | low >> 31;
    x[2] = x[2] << 1 | middle >> 31;
}

void __cdecl CRT_shr_12(u32 *x) {
    const u32 middle = x[1];
    x[1] = middle >> 1 | x[2] << 31;
    x[2] >>= 1;
    x[0] = x[0] >> 1 | middle << 31;
}

void __cdecl CRT_mtold12(const char *digits, u32 length, u32 *out) {
    u16 exponent = 0x404e;
    out[0] = 0;
    out[1] = 0;
    out[2] = 0;
    for (; length; --length) {
        const u32 copy[3] = {out[0], out[1], out[2]};
        CRT_shl_12(out);
        CRT_shl_12(out);
        CRT_add_12(out, copy);
        CRT_shl_12(out);
        const u32 digit[3] = {static_cast<u32>(static_cast<s32>(*digits++)), 0, 0};
        CRT_add_12(out, digit);
    }
    while (!out[2]) {
        exponent = static_cast<u16>(exponent - 0x10);
        out[2] = out[1] >> 16;
        out[1] = out[1] << 16 | out[0] >> 16;
        out[0] <<= 16;
    }
    while (!(out[2] & 0x8000)) {
        CRT_shl_12(out);
        exponent = static_cast<u16>(exponent - 1);
    }
    at<u16>(out, 0xa) = exponent;
}

void __cdecl CRT_ld12mul(u32 *x, const u32 *y) {
    u32 sum[4] = {0, 0, 0, 0};   // (sic: the last row's carry goes one word past the twelve bytes, over the original's return address; a product never carries there)
    const u16 x_top = at<u16>(x, 0xa), y_top = at<u16>(const_cast<u32 *>(y), 0xa);
    const u32 sign = (x_top ^ y_top) & 0x8000u;
    const u16 x_exponent = x_top & 0x7fff, y_exponent = y_top & 0x7fff;
    u16 exponent = static_cast<u16>(x_exponent + y_exponent);
    if (x_exponent >= 0x7fff || y_exponent >= 0x7fff || exponent > 0xbffd) {
        set_infinity(x, sign);
        return;
    }
    if (exponent <= 0x3fbf) {
        x[2] = 0;
        x[1] = 0;
        x[0] = 0;
        return;
    }
    if (!x_exponent) {
        exponent = static_cast<u16>(exponent + 1);
        if (!(x[2] & 0x7fffffff) && !x[1] && !x[0]) {
            at<u16>(x, 0xa) = 0;
            return;
        }
    }
    if (!y_exponent) {
        exponent = static_cast<u16>(exponent + 1);
        if (!(y[2] & 0x7fffffff) && !y[1] && !y[0]) {
            x[2] = 0;
            x[1] = 0;
            x[0] = 0;
            return;
        }
    }
    for (s32 row = 0; row < 5; ++row) {
        const u16 *a = reinterpret_cast<const u16 *>(x) + row;
        const u16 *b = reinterpret_cast<const u16 *>(y) + 4;
        u32 &total = at<u32>(sum, static_cast<u32>(row) * 2);
        u16 &carry = at<u16>(sum, 4 + static_cast<u32>(row) * 2);
        for (s32 left = 5 - row; left; --left, ++a, --b) {
            if (CRT_addl(total, static_cast<u32>(*b) * *a, &total)) carry = static_cast<u16>(carry + 1);
        }
    }
    exponent = static_cast<u16>(exponent + 0xc002);
    while (static_cast<s16>(exponent) > 0 && !(sum[2] & 0x80000000)) {
        CRT_shl_12(sum);
        exponent = static_cast<u16>(exponent - 1);
    }
    if (static_cast<s16>(exponent) <= 0) {
        exponent = static_cast<u16>(exponent - 1);
        u32 lost = 0;
        if (static_cast<s16>(exponent) < 0) {
            for (s32 left = -static_cast<s32>(static_cast<s16>(exponent)); left; --left) {
                if (sum[0] & 1) ++lost;
                CRT_shr_12(sum);
            }
            exponent = 0;
        }
        if (lost) sum[0] |= 1;
    }
    if (at<u16>(sum, 0) > 0x8000 || (sum[0] & 0x1ffff) == 0x18000) {
        if (at<u32>(sum, 2) != 0xffffffff) {
            ++at<u32>(sum, 2);
        } else {
            at<u32>(sum, 2) = 0;
            if (at<u32>(sum, 6) != 0xffffffff) {
                ++at<u32>(sum, 6);
            } else {
                at<u32>(sum, 6) = 0;
                if (at<u16>(sum, 0xa) != 0xffff) {
                    ++at<u16>(sum, 0xa);
                } else {
                    at<u16>(sum, 0xa) = 0x8000;
                    exponent = static_cast<u16>(exponent + 1);
                }
            }
        }
    }
    if (exponent >= 0x7fff) {
        set_infinity(x, sign);
        return;
    }
    at<u16>(x, 0) = at<u16>(sum, 2);
    at<u32>(x, 2) = sum[1];
    at<u32>(x, 6) = sum[2];
    at<u16>(x, 0xa) = static_cast<u16>(exponent | sign);
}

void __cdecl CRT_multtenpow12(u32 *value, s32 power, u32 keep_low) {
    if (!power) return;
    u32 table = 0x00781f70;
    if (power < 0) {
        table = 0x007820d0;
        power = static_cast<s32>(0u - static_cast<u32>(power));
    }
    if (!keep_low) at<u16>(value, 0) = 0;
    while (power) {
        table += 0x54;
        const u32 digit = static_cast<u32>(power) & 7;
        power >>= 3;
        if (!digit) continue;
        const u32 *factor = reinterpret_cast<const u32 *>((uintptr_t)(table + digit * 12));
        u32 copy[3];
        if (*reinterpret_cast<const u16 *>(factor) >= 0x8000) {
            copy[0] = factor[0];
            copy[1] = factor[1];
            copy[2] = factor[2];
            --at<u32>(copy, 2);
            factor = copy;
        }
        CRT_ld12mul(value, factor);
    }
}

s32 __cdecl CRT_i10_output(u32 low, u32 high, u32 sign_exp, s32 digits, u32 flags, u8 *out) {
    const u32 exponent = sign_exp & 0x7fff;
    const bool negative = (sign_exp & 0x8000) != 0;
    s16 &point = *reinterpret_cast<s16 *>(out);
    out[2] = negative ? 0x2d : 0x20;
    const auto zero = [&] {
        point = 0;
        out[2] = 0x20;
        out[3] = 1;
        out[4] = 0x30;
        out[5] = 0;
        return 1;
    };
    if (!exponent && !high && !low) return zero();
    if (exponent == 0x7fff) {
        const auto text = [&](u32 address, u8 length) {
            std::memcpy(out + 4, reinterpret_cast<const void *>((uintptr_t)address), length + 1u);
            out[3] = length;
            return 0;
        };
        point = 1;
        if ((high != 0x80000000 || low) && !(high & 0x40000000)) return text(0x00709748, 6);
        if (negative && high == 0xc0000000 && !low) return text(0x00709740, 5);
        if (high == 0x80000000 && !low) return text(0x00709738, 5);
        return text(0x00709730, 6);
    }
    u32 man[3] = {low << 16, low >> 16 | high << 16, high >> 16 | exponent << 16};
    const u32 estimate = (exponent >> 8) + (high >> 24) * 2;
    s32 power = static_cast<s32>(estimate * 77 + exponent * 19728 + 0xecbced0cu) >> 16;
    CRT_multtenpow12(man, -static_cast<s16>(power), 1);
    if (man[2] >> 16 >= 0x3fff) {
        static const u32 kTenth[3] = {0xcccccccc, 0xcccccccc, 0x3ffbcccc};
        CRT_ld12mul(man, kTenth);
        ++power;
    }
    point = static_cast<s16>(power);
    if (flags & 1) {
        digits += static_cast<s16>(power);
        if (digits <= 0) return zero();
    }
    if (digits > 0x15) digits = 0x15;
    const s32 shift = static_cast<s32>(man[2] >> 16) - 0x3ffe;
    man[2] &= 0xffff;
    for (int i = 0; i < 8; ++i) CRT_shl_12(man);
    if (shift < 0) {
        for (u32 n = static_cast<u32>(-shift) & 0xff; n; --n) CRT_shr_12(man);
    }
    u8 *const first = out + 4;
    u8 *end = first;
    for (s32 count = digits + 1; count > 0; --count) {   // (times ten: the top byte the digit)
        const u32 copy[3] = {man[0], man[1], man[2]};
        CRT_shl_12(man);
        CRT_shl_12(man);
        CRT_add_12(man, copy);
        CRT_shl_12(man);
        *end++ = static_cast<u8>((man[2] >> 24) + 0x30);
        man[2] &= 0x00ffffff;
    }
    u8 *last = end - 2;
    if (static_cast<signed char>(end[-1]) >= 0x35) {
        while (last >= first && *last == 0x39) *last-- = 0x30;
        if (last < first) {
            ++last;
            ++point;
        }
        ++*last;
    } else {
        while (last >= first && *last == 0x30) --last;
        if (last < first) return zero();
    }
    const u8 length = static_cast<u8>(static_cast<u8>(last - out) - 3);
    out[3] = length;
    out[4 + static_cast<signed char>(length)] = 0;
    return 1;
}

u32 *__cdecl CRT_dtold(u32 *to, const u32 *x) {
    u32 low = x[0];
    const u16 top = reinterpret_cast<const u16 *>(x)[3];
    const u32 exponent = top >> 4 & 0x7ff;
    u32 high = x[1] & 0xfffff;
    u32 one = 0x80000000;
    u32 e = exponent + 0x3c00;
    if (exponent == 0x7ff) {
        e = 0x7fff;
    } else if (!exponent) {
        one = 0;
        if (!high && !low) {
            to[0] = to[1] = 0;
            *reinterpret_cast<u16 *>(to + 2) = 0;
            return to;
        }
        e = 0x3c01;
    }
    high = low >> 21 | high << 11 | one;
    low <<= 11;
    while (!(high >> 31)) {
        high = high << 1 | low >> 31;
        low <<= 1;
        --e;
    }
    to[0] = low;
    to[1] = high;
    *reinterpret_cast<u16 *>(to + 2) = static_cast<u16>(e | (top & 0x8000));
    return to;
}

u32 *__cdecl CRT_fltout2(u32 low, u32 high, u32 *flt, char *buf) {
    const u32 x[2] = {low, high};
    u32 ten[3] = {0, 0, 0};
    CRT_dtold(ten, x);
    u8 out[0x1c];
    const s32 answer = CRT_i10_output(ten[0], ten[1], ten[2], 0x11, 0, out);
    flt[2] = static_cast<u32>(answer);
    flt[1] = static_cast<u32>(static_cast<s32>(*reinterpret_cast<s16 *>(out)));
    flt[0] = static_cast<u32>(static_cast<s32>(static_cast<signed char>(out[2])));
    std::strcpy(buf, reinterpret_cast<const char *>(out + 4));
    flt[3] = static_cast<u32>(reinterpret_cast<uintptr_t>(buf));
    return flt;
}

u32 __cdecl CRT_fptostr(char *buf, s32 digits, u32 *flt) {
    const char *from = reinterpret_cast<const char *>((uintptr_t)flt[3]);
    char *p = buf + 1;
    *buf = 0x30;
    for (; digits > 0; --digits) *p++ = *from ? *from++ : 0x30;
    *p = 0;
    if (digits >= 0 && static_cast<signed char>(*from) >= 0x35) {
        while (*--p == 0x39) *p = 0x30;
        ++*p;
    }
    if (*buf == 0x31) {
        ++flt[1];
        return static_cast<u32>(reinterpret_cast<uintptr_t>(p));
    }
    const u32 length = static_cast<u32>(std::strlen(buf + 1)) + 1;
    std::memmove(buf, buf + 1, length);
    return length;
}

u32 __cdecl CRT_strgtold12(u32 *out, const char **end, const char *text, u32 keep_low, s32 scale, s32 decimals, s32 implicit_e) {
    enum { kStart, kZeros, kSigned, kWhole, kFraction, kPoint, kMark, kExponentSign, kExponentZeros, kExponent, kDone, kImplicit };
    char digits[0x1c];
    char *to = digits;
    const char *p = text, *saved = text;
    u32 count = 0, sign = 0;
    s32 places = 0, exponent = 0, exponent_sign = 1;
    bool found_digit = false, found_point = false, found_exponent = false;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') ++p;
    s32 state = kStart;
    const auto point = [] { return global<char>(0x0077e7e0); };
    const auto after_digits = [&](char c) {
        if (c == '+' || c == '-') {
            --p;
            state = kImplicit;
        } else if (exponent_mark(c)) {
            state = kMark;
        } else {
            --p;
            state = kDone;
        }
    };
    do {
        char c = *p++;
        switch (state) {
        case kStart:
            if (one_to_nine(c)) {
                state = kWhole;
                --p;
            } else if (c == point()) {
                state = kPoint;
            } else if (c == '+') {
                state = kSigned;
                sign = 0;
            } else if (c == '-') {
                state = kSigned;
                sign = 0x8000;
            } else if (c == '0') {
                state = kZeros;
            } else {
                state = kDone;
                --p;
            }
            break;
        case kZeros:
            found_digit = true;
            if (one_to_nine(c)) {
                state = kWhole;
                --p;
            } else if (c == point()) {
                state = kFraction;
            } else if (c != '0') {
                after_digits(c);
            }
            break;
        case kSigned:
            if (one_to_nine(c)) {
                state = kWhole;
                --p;
            } else if (c == point()) {
                state = kPoint;
            } else if (c == '0') {
                state = kZeros;
            } else {
                state = kDone;
                p = saved;
            }
            break;
        case kWhole:
            found_digit = true;
            for (; is_digit(static_cast<u8>(c)); c = *p++) {
                if (count < 0x19) {
                    ++count;
                    *to++ = static_cast<char>(c - 0x30);
                } else {
                    ++places;
                }
            }
            if (c == point()) state = kFraction;
            else after_digits(c);
            break;
        case kFraction:
            found_digit = true;
            found_point = true;
            if (!count) {
                for (; c == '0'; c = *p++) --places;
            }
            for (; is_digit(static_cast<u8>(c)); c = *p++) {
                if (count < 0x19) {
                    ++count;
                    *to++ = static_cast<char>(c - 0x30);
                    --places;
                }
            }
            after_digits(c);
            break;
        case kPoint:
            found_point = true;
            if (is_digit(static_cast<u8>(c))) {
                state = kFraction;
                --p;
            } else {
                state = kDone;
                p = saved;
            }
            break;
        case kMark:
            saved = p - 2;
            if (one_to_nine(c)) {
                state = kExponent;
                --p;
            } else if (c == '+') {
                state = kExponentSign;
            } else if (c == '-') {
                state = kExponentSign;
                exponent_sign = -1;
            } else if (c == '0') {
                state = kExponentZeros;
            } else {
                state = kDone;
                p = saved;
            }
            break;
        case kExponentSign:
            if (one_to_nine(c)) {
                state = kExponent;
                --p;
            } else if (c == '0') {
                state = kExponentZeros;
            } else {
                state = kDone;
                p = saved;
            }
            break;
        case kExponentZeros:
            found_exponent = true;
            while (c == '0') c = *p++;
            state = one_to_nine(c) ? kExponent : kDone;
            --p;
            break;
        case kExponent:
            found_exponent = true;
            exponent = 0;
            for (; is_digit(static_cast<u8>(c)); c = *p++) {
                exponent = static_cast<s32>(static_cast<u32>(exponent) * 10 + static_cast<u32>(static_cast<s32>(c)) - 0x30);
                if (exponent > 0x1450) {
                    exponent = 0x1451;
                    break;
                }
            }
            while (is_digit(static_cast<u8>(c))) c = *p++;
            state = kDone;
            --p;
            break;
        default:   // kImplicit
            if (!implicit_e) {
                state = kDone;
                --p;
                break;
            }
            saved = p - 1;
            if (c == '+') {
                state = kExponentSign;
            } else if (c == '-') {
                state = kExponentSign;
                exponent_sign = -1;
            } else {
                state = kDone;
                p = saved;
            }
            break;
        }
    } while (state != kDone);
    *end = p;
    u32 result = 0, low_word = 0, middle = 0, high = 0, top = 0;
    if (!found_digit) {
        result = 4;
    } else {
        if (count > 0x18) {
            if (static_cast<signed char>(digits[0x17]) >= 5) ++digits[0x17];
            --to;
            ++places;
            count = 0x18;
        }
        if (count) {
            --to;
            while (!*to) {
                --count;
                ++places;
                --to;
            }
            u32 number[3];
            CRT_mtold12(digits, count, number);
            s32 power = exponent_sign < 0 ? -exponent : exponent;
            power += places;
            if (!found_exponent) power += scale;
            if (!found_point) power -= decimals;
            if (power > 0x1450) {
                result = 2;
                top = 0x7fff;
                high = 0x80000000;
            } else if (power < -0x1450) {
                result = 1;
            } else {
                CRT_multtenpow12(number, power, keep_low);
                low_word = at<u16>(number, 0);
                middle = at<u32>(number, 2);
                high = at<u32>(number, 6);
                top = at<u16>(number, 0xa);
            }
        }
    }
    at<u16>(out, 0) = static_cast<u16>(low_word);
    at<u32>(out, 2) = middle;
    at<u32>(out, 6) = high;
    at<u16>(out, 0xa) = static_cast<u16>(top | sign);
    return result;
}

u32 *__cdecl CRT_fltin2(u32 *flt, const char *text, s32, s32, s32) {
    u32 number[3];
    const char *end;
    const u32 got = CRT_strgtold12(number, &end, text, 0, 0, 0, 0);
    u32 flags = 0;
    u32 value[2] = {0, 0};
    if (got & 4) {
        flags = 0x200;
    } else {
        const s32 converted = CRT_ld12tod(reinterpret_cast<const u8 *>(number), value);
        if ((got & 2) || converted == 1) flags = 0x80;
        if ((got & 1) || converted == 2) flags |= 0x100;
    }
    flt[0] = flags;
    flt[1] = static_cast<u32>(end - text);
    flt[4] = value[0];
    flt[5] = value[1];
    return flt;
}

double __cdecl CRT_strtod(const char *text, const char **end) {
    const char *from = text;
    while (is_space(static_cast<u8>(*from))) ++from;
    u32 flt[6];
    const u32 *const got = CRT_fltin2(flt, from, static_cast<s32>(std::strlen(from)), 0, 0);
    if (end) *end = from + got[1];
    const u32 flags = got[0];
    if (flags & 0x240) {
        if (end) *end = text;
        return global<const double>(0x00711c28);
    }
    if (flags & 0x81) {
        const double huge = *from == '-' ? -global<const double>(0x00784c10) : global<const double>(0x00784c10);
        *CRT_errno() = 0x22;
        return huge;
    }
    if (flags & 0x100) {
        *CRT_errno() = 0x22;
        return 0.0;
    }
    return *reinterpret_cast<const double *>(got + 4);
}

}  // extern "C"
