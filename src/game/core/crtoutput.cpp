#include "crtoutput.h"

#include <cstdint>
#include <cstring>

#include "crt.h"
#include "crtstream.h"
#include "initializers.h"

namespace {

using u64 = Crt64;
using s64 = long long;

u32 word(u32 address) { return *reinterpret_cast<u32 *>((uintptr_t)address); }

// For ' ' to 'x' (0x708ab0): in the low half the character's class; in the high half of [state + 8 * class] the next state.
const u8 kStates[0x59] = {
    0x06, 0x00, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x10, 0x00, 0x03, 0x06, 0x00, 0x06, 0x02, 0x10, 0x04, 0x45, 0x45, 0x45, 0x05, 0x05, 0x05, 0x05, 0x05, 0x35, 0x30, 0x00, 0x50, 0x00,
    0x00, 0x00, 0x00, 0x20, 0x28, 0x38, 0x50, 0x58, 0x07, 0x08, 0x00, 0x37, 0x30, 0x30, 0x57, 0x50, 0x07, 0x00, 0x00, 0x20, 0x20, 0x08, 0x00, 0x00, 0x00, 0x00, 0x08, 0x60, 0x68, 0x60,
    0x60, 0x60, 0x60, 0x00, 0x00, 0x70, 0x70, 0x78, 0x78, 0x78, 0x78, 0x08, 0x07, 0x08, 0x00, 0x00, 0x07, 0x00, 0x08, 0x08, 0x08, 0x00, 0x00, 0x08, 0x00, 0x08, 0x00, 0x07, 0x08,
};

enum { kNormal, kPercent, kFlag, kWidth, kDot, kPrecision, kSize, kType };

enum : u32 {
    kPlus = 1,
    kSpace = 2,
    kLeft = 4,
    kZeros = 8,
    kLong = 0x10,
    kShort = 0x20,
    kSigned = 0x40,
    kAlternate = 0x80,
    kNegative = 0x100,
    kForceOctal = 0x200,
    kWide = 0x800,
    kInt64 = 0x8000,
};

}  // namespace

s32 *__cdecl CRT_output_write_char(s32 c, CrtFile *file, s32 *written) {
    s32 result;
    if (--file->count >= 0) {
        *file->ptr++ = static_cast<char>(c);
        result = static_cast<u8>(c);
    } else {
        result = CrtInner().flsbuf(c, file);
    }
    if (result == -1) {
        *written = -1;
        return reinterpret_cast<s32 *>(static_cast<intptr_t>(-1));
    }
    ++*written;
    return written;
}

s32 __cdecl CRT_output_write_multi_char(s32 c, s32 count, CrtFile *file, s32 *written) {
    s32 last = count;
    for (s32 left = count; left > 0; --left) {
        CRT_output_write_char(c, file, written);
        last = *written;
        if (last == -1) break;
    }
    return last;
}

s32 __cdecl CRT_output_write_string(const char *text, s32 count, CrtFile *file, s32 *written) {
    s32 last = count;
    for (s32 left = count; left > 0; --left) {
        CRT_output_write_char(*text++, file, written);
        last = *written;
        if (last == -1) break;
    }
    return last;
}

u32 __cdecl CRT_output_get_int_arg(char **args) {
    *args += 4;
    return *reinterpret_cast<u32 *>(*args - 4);
}

u16 __cdecl CRT_output_get_short_arg(char **args) {
    *args += 4;
    return *reinterpret_cast<u16 *>(*args - 4);
}

u64 __cdecl CRT_output_get_int64_arg(char **args) {
    *args += 8;
    return *reinterpret_cast<u64 *>(*args - 8);
}

s32 __cdecl CRT_output(CrtFile *file, const char *format, char *args) {
    using TextCall = void(__cdecl *)(char *);
    s32 written = 0;
    u32 state = 0;
    u32 flags = 0;
    s32 width = 0;
    s32 precision = 0;
    s32 length = 0;
    s32 prefix_length = 0;
    s32 capital = 0;
    s32 no_output = 0;
    s32 wide_text = 0;
    u32 radix = 0;
    s32 hex_add = 0;
    char *text = nullptr;
    char prefix[2] = {0, 0};
    char buffer[0x200];
    char ch = *format++;
    while (ch && written >= 0) {
        const u32 kind = (ch < 0x20 || ch > 0x78) ? 0 : kStates[ch - 0x20] & 0xf;
        state = kStates[state + kind * 8] >> 4;
        switch (state) {
            case kPercent:
                capital = 0;
                no_output = 0;
                width = 0;
                prefix_length = 0;
                flags = 0;
                precision = -1;
                wide_text = 0;
                break;
            case kFlag:
                switch (ch) {
                    case ' ': flags |= kSpace; break;
                    case '#': flags |= kAlternate; break;
                    case '+': flags |= kPlus; break;
                    case '-': flags |= kLeft; break;
                    case '0': flags |= kZeros; break;
                    default: break;
                }
                break;
            case kWidth:
                if (ch == '*') {
                    width = static_cast<s32>(CRT_output_get_int_arg(&args));
                    if (width < 0) {
                        flags |= kLeft;
                        width = static_cast<s32>(0u - static_cast<u32>(width));
                    }
                } else {
                    width = static_cast<s32>(static_cast<u32>(width) * 10 + static_cast<u32>(static_cast<s32>(ch)) - 0x30);
                }
                break;
            case kDot:
                precision = 0;
                break;
            case kPrecision:
                if (ch == '*') {
                    precision = static_cast<s32>(CRT_output_get_int_arg(&args));
                    if (precision < 0) precision = -1;
                } else {
                    precision = static_cast<s32>(static_cast<u32>(precision) * 10 + static_cast<u32>(static_cast<s32>(ch)) - 0x30);
                }
                break;
            case kSize:
                switch (ch) {
                    case 'I':
                        if (format[0] == '6' && format[1] == '4') {
                            format += 2;
                            flags |= kInt64;
                            break;
                        }
                        state = kNormal;
                        goto normal;
                    case 'h': flags |= kShort; break;
                    case 'l': flags |= kLong; break;
                    case 'w': flags |= kWide; break;
                    default: break;
                }
                break;
            case kNormal:
            normal:
                wide_text = 0;
                if (CRT_pctype[static_cast<u8>(ch)] & 0x8000) {
                    CRT_output_write_char(ch, file, &written);
                    ch = *format++;
                }
                CRT_output_write_char(ch, file, &written);
                break;
            case kType: {
                bool number = false;
                switch (ch) {
                    case 'C':
                        if (!(flags & (kLong | kShort | kWide))) flags |= kWide;
                        [[fallthrough]];
                    case 'c':
                        if (flags & (kLong | kWide)) {
                            const u16 c = CRT_output_get_short_arg(&args);
                            length = CRT_wctomb(buffer, c);
                            if (length < 0) no_output = 1;
                        } else {
                            buffer[0] = static_cast<char>(CRT_output_get_int_arg(&args));
                            length = 1;
                        }
                        text = buffer;
                        break;
                    case 'Z': {
                        struct Counted {
                            s16 length;
                            s16 room;
                            char *text;
                        };
                        const Counted *const counted = reinterpret_cast<const Counted *>((uintptr_t)CRT_output_get_int_arg(&args));
                        if (counted && counted->text) {
                            text = counted->text;
                            if (flags & kWide) {
                                length = static_cast<s32>(static_cast<u32>(static_cast<s32>(counted->length)) >> 1);
                                wide_text = 1;
                            } else {
                                length = counted->length;
                                wide_text = 0;
                            }
                        } else {
                            text = CRT_nullstring;
                            length = static_cast<s32>(std::strlen(text));
                        }
                        break;
                    }
                    case 'S':
                        if (!(flags & (kLong | kShort | kWide))) flags |= kWide;
                        [[fallthrough]];
                    case 's': {
                        u32 most = precision == -1 ? 0x7fffffff : static_cast<u32>(precision);
                        text = reinterpret_cast<char *>((uintptr_t)CRT_output_get_int_arg(&args));
                        if (flags & (kLong | kWide)) {
                            if (!text) text = reinterpret_cast<char *>(CRT_wnullstring);
                            wide_text = 1;
                            const u16 *at = reinterpret_cast<const u16 *>(text);
                            while (most-- && *at) ++at;
                            length = static_cast<s32>(at - reinterpret_cast<const u16 *>(text));
                        } else {
                            if (!text) text = CRT_nullstring;
                            const char *at = text;
                            while (most-- && *at) ++at;
                            length = static_cast<s32>(at - text);
                        }
                        break;
                    }
                    case 'n': {
                        void *const to = reinterpret_cast<void *>((uintptr_t)CRT_output_get_int_arg(&args));
                        if (flags & kShort) {
                            *static_cast<u16 *>(to) = static_cast<u16>(written);
                        } else {
                            *static_cast<s32 *>(to) = written;
                        }
                        no_output = 1;
                        break;
                    }
                    case 'E':
                    case 'G':
                        capital = 1;
                        ch = static_cast<char>(ch + 0x20);
                        [[fallthrough]];
                    case 'e':
                    case 'f':
                    case 'g': {
                        flags |= kSigned;
                        text = buffer;
                        if (precision < 0) {
                            precision = 6;
                        } else if (!precision && ch == 'g') {
                            precision = 1;
                        }
                        u64 value;
                        std::memcpy(&value, args, 8);
                        args += 8;
                        reinterpret_cast<void(__cdecl *)(u64 *, char *, s32, s32, s32)>((uintptr_t)CRT_cfltcvt_tab[0])(&value, buffer, ch, precision, capital);
                        const u32 alternate = flags & kAlternate;
                        if (alternate && !precision) reinterpret_cast<TextCall>((uintptr_t)CRT_cfltcvt_tab[3])(buffer);
                        if (ch == 'g' && !alternate) reinterpret_cast<TextCall>((uintptr_t)CRT_cfltcvt_tab[1])(buffer);
                        if (buffer[0] == '-') {
                            flags |= kNegative;
                            text = buffer + 1;
                        }
                        length = static_cast<s32>(std::strlen(text));
                        break;
                    }
                    case 'd':
                    case 'i':
                        flags |= kSigned;
                        radix = 10;
                        number = true;
                        break;
                    case 'u':
                        radix = 10;
                        number = true;
                        break;
                    case 'p':
                        precision = 8;
                        [[fallthrough]];
                    case 'X':
                    case 'x':
                        hex_add = ch == 'x' ? 0x27 : 7;
                        radix = 16;
                        if (flags & kAlternate) {
                            prefix[0] = '0';
                            prefix[1] = static_cast<char>(hex_add + 0x51);
                            prefix_length = 2;
                        }
                        number = true;
                        break;
                    case 'o':
                        radix = 8;
                        if (flags & kAlternate) flags |= kForceOctal;
                        number = true;
                        break;
                    default:   // ('B', a type by the table: the last text again)
                        break;
                }
                if (number) {
                    u64 value;
                    if (flags & kInt64) {
                        value = CRT_output_get_int64_arg(&args);
                    } else if (flags & kShort) {
                        const u32 got = CRT_output_get_int_arg(&args);
                        value = (flags & kSigned) ? static_cast<u64>(static_cast<s64>(static_cast<s16>(got))) : static_cast<u16>(got);
                    } else {
                        const u32 got = CRT_output_get_int_arg(&args);
                        value = (flags & kSigned) ? static_cast<u64>(static_cast<s64>(static_cast<s32>(got))) : got;
                    }
                    if ((flags & kSigned) && static_cast<s64>(value) < 0) {
                        value = 0 - value;
                        flags |= kNegative;
                    }
                    if (!(flags & kInt64)) value &= 0xffffffff;
                    s32 digits = precision;
                    if (digits < 0) {
                        digits = 1;
                    } else {
                        flags &= ~static_cast<u32>(kZeros);
                    }
                    if (!value) prefix_length = 0;
                    char *at = buffer + 0x1ff;
                    while (digits-- > 0 || value) {
                        s32 digit = static_cast<s32>(CRT_aullrem(value, radix)) + 0x30;
                        value = CRT_aulldiv(value, radix);
                        if (digit > 0x39) digit += hex_add;
                        *at-- = static_cast<char>(digit);
                    }
                    length = static_cast<s32>(buffer + 0x1ff - at);
                    text = at + 1;
                    if ((flags & kForceOctal) && (!length || *text != '0')) {
                        *--text = '0';
                        ++length;
                    }
                }
                if (no_output) break;
                if (flags & kSigned) {
                    if (flags & kNegative) {
                        prefix[0] = '-';
                        prefix_length = 1;
                    } else if (flags & kPlus) {
                        prefix[0] = '+';
                        prefix_length = 1;
                    } else if (flags & kSpace) {
                        prefix[0] = ' ';
                        prefix_length = 1;
                    }
                }
                const s32 padding = width - length - prefix_length;
                if (!(flags & (kLeft | kZeros))) CRT_output_write_multi_char(' ', padding, file, &written);
                CRT_output_write_string(prefix, prefix_length, file, &written);
                if ((flags & kZeros) && !(flags & kLeft)) CRT_output_write_multi_char('0', padding, file, &written);
                if (wide_text && length > 0) {
                    const u16 *at = reinterpret_cast<const u16 *>(text);
                    s32 left = length - 1;
                    do {
                        char bytes[4];
                        const s32 count = CRT_wctomb(bytes, *at++);
                        if (count <= 0) break;
                        CRT_output_write_string(bytes, count, file, &written);
                    } while (left-- != 0);
                } else {
                    CRT_output_write_string(text, length, file, &written);
                }
                if (flags & kLeft) CRT_output_write_multi_char(' ', padding, file, &written);
                break;
            }
            default:
                break;
        }
        ch = *format++;
    }
    return written;
}

s32 __cdecl CRT_woutput(CrtFile *file, const u16 *format, char *args) {
    using TextCall = void(__cdecl *)(char *);
    s32 written = 0;
    u32 state = 0;
    u32 flags = 0;
    s32 width = 0;
    s32 precision = 0;
    s32 length = 0;
    s32 prefix_length = 0;
    s32 capital = 0;
    s32 no_output = 0;
    s32 wide_text = 0;
    u32 radix = 0;
    s32 hex_add = 0;
    u32 wchar = 0;   // (one slot: %c's argument, then each character mbtowc makes)
    char *text = nullptr;
    u16 prefix[2] = {0, 0};
    char buffer[0x200];
    u16 ch = *format++;
    while (ch && written >= 0) {
        const u32 kind = (ch < 0x20 || ch > 0x78) ? 0 : kStates[ch - 0x20] & 0xf;
        state = kStates[state + kind * 8] >> 4;
        switch (state) {
            case kPercent:
                capital = 0;
                no_output = 0;
                width = 0;
                prefix_length = 0;
                flags = 0;
                precision = -1;
                wide_text = 0;
                break;
            case kFlag:
                switch (ch) {
                    case ' ': flags |= kSpace; break;
                    case '#': flags |= kAlternate; break;
                    case '+': flags |= kPlus; break;
                    case '-': flags |= kLeft; break;
                    case '0': flags |= kZeros; break;
                    default: break;
                }
                break;
            case kWidth:
                if (ch == '*') {
                    width = static_cast<s32>(CRT_output_get_int_arg(&args));
                    if (width < 0) {
                        flags |= kLeft;
                        width = static_cast<s32>(0u - static_cast<u32>(width));
                    }
                } else {
                    width = static_cast<s32>(static_cast<u32>(width) * 10 + ch - 0x30);
                }
                break;
            case kDot:
                precision = 0;
                break;
            case kPrecision:
                if (ch == '*') {
                    precision = static_cast<s32>(CRT_output_get_int_arg(&args));
                    if (precision < 0) precision = -1;
                } else {
                    precision = static_cast<s32>(static_cast<u32>(precision) * 10 + ch - 0x30);
                }
                break;
            case kSize:
                switch (ch) {
                    case 'I':
                        if (format[0] == '6' && format[1] == '4') {
                            format += 2;
                            flags |= kInt64;
                            break;
                        }
                        state = kNormal;
                        goto normal;
                    case 'h': flags |= kShort; break;
                    case 'w': flags |= kWide; break;
                    default: break;   // ('l' too, sic)
                }
                break;
            case kNormal:
            normal:
                wide_text = 1;
                CRT_woutput_write_char(ch, file, &written);
                break;
            case kType: {
                bool number = false;
                switch (ch) {
                    case 'C':
                        if (!(flags & (kLong | kShort | kWide))) flags |= kShort;
                        [[fallthrough]];
                    case 'c':
                        wide_text = 1;
                        wchar = CRT_output_get_int_arg(&args);
                        if (flags & kShort) {
                            const char bytes[2] = {static_cast<char>(wchar), 0};
                            if (CRT_mbtowc(reinterpret_cast<u16 *>(buffer), bytes, static_cast<u32>(CRT_mb_cur_max)) < 0) no_output = 1;
                        } else {
                            *reinterpret_cast<u16 *>(buffer) = static_cast<u16>(wchar);
                        }
                        text = buffer;
                        length = 1;
                        break;
                    case 'Z': {
                        struct Counted {
                            s16 length;
                            s16 room;
                            char *text;
                        };
                        const Counted *const counted = reinterpret_cast<const Counted *>((uintptr_t)CRT_output_get_int_arg(&args));
                        if (counted && counted->text) {
                            text = counted->text;
                            if (flags & kWide) {
                                length = static_cast<s32>(static_cast<u32>(static_cast<s32>(counted->length)) >> 1);
                                wide_text = 1;
                            } else {
                                length = counted->length;
                                wide_text = 0;
                            }
                        } else {
                            text = CRT_nullstring;
                            length = static_cast<s32>(std::strlen(text));
                        }
                        break;
                    }
                    case 'S':
                        if (!(flags & (kLong | kShort | kWide))) flags |= kShort;
                        [[fallthrough]];
                    case 's': {
                        const s32 most = precision == -1 ? 0x7fffffff : precision;
                        text = reinterpret_cast<char *>((uintptr_t)CRT_output_get_int_arg(&args));
                        if (flags & kShort) {
                            if (!text) text = CRT_nullstring;
                            const u16 *const lead = CRT_pctype;
                            const char *at = text;
                            for (length = 0; length < most && *at; ++length) {
                                if (lead[static_cast<u8>(*at)] & 0x8000) ++at;
                                ++at;
                            }
                        } else {
                            if (!text) text = reinterpret_cast<char *>(CRT_wnullstring);
                            wide_text = 1;
                            u32 left = static_cast<u32>(most);
                            const u16 *at = reinterpret_cast<const u16 *>(text);
                            while (left-- && *at) ++at;
                            length = static_cast<s32>(at - reinterpret_cast<const u16 *>(text));
                        }
                        break;
                    }
                    case 'n': {
                        void *const to = reinterpret_cast<void *>((uintptr_t)CRT_output_get_int_arg(&args));
                        if (flags & kShort) {
                            *static_cast<u16 *>(to) = static_cast<u16>(written);
                        } else {
                            *static_cast<s32 *>(to) = written;
                        }
                        no_output = 1;
                        break;
                    }
                    case 'E':
                    case 'G':
                        capital = 1;
                        ch = static_cast<u16>(ch + 0x20);
                        [[fallthrough]];
                    case 'e':
                    case 'f':
                    case 'g': {
                        flags |= kSigned;
                        text = buffer;
                        if (precision < 0) {
                            precision = 6;
                        } else if (!precision && ch == 'g') {
                            precision = 1;
                        }
                        u64 value;
                        std::memcpy(&value, args, 8);
                        args += 8;
                        reinterpret_cast<void(__cdecl *)(u64 *, char *, s32, s32, s32)>((uintptr_t)CRT_cfltcvt_tab[0])(&value, buffer, static_cast<s8>(ch), precision, capital);
                        const u32 alternate = flags & kAlternate;
                        if (alternate && !precision) reinterpret_cast<TextCall>((uintptr_t)CRT_cfltcvt_tab[3])(buffer);
                        if (ch == 'g' && !alternate) reinterpret_cast<TextCall>((uintptr_t)CRT_cfltcvt_tab[1])(buffer);
                        if (buffer[0] == '-') {
                            flags |= kNegative;
                            text = buffer + 1;
                        }
                        length = static_cast<s32>(std::strlen(text));
                        break;
                    }
                    case 'd':
                    case 'i':
                        flags |= kSigned;
                        radix = 10;
                        number = true;
                        break;
                    case 'u':
                        radix = 10;
                        number = true;
                        break;
                    case 'p':
                        precision = 8;
                        [[fallthrough]];
                    case 'X':
                    case 'x':
                        hex_add = ch == 'x' ? 0x27 : 7;
                        radix = 16;
                        if (flags & kAlternate) {
                            prefix[0] = '0';
                            prefix[1] = static_cast<u16>(hex_add + 0x51);
                            prefix_length = 2;
                        }
                        number = true;
                        break;
                    case 'o':
                        radix = 8;
                        if (flags & kAlternate) flags |= kForceOctal;
                        number = true;
                        break;
                    default:   // (a type by the table: the last text again)
                        break;
                }
                if (number) {
                    u64 value;
                    if (flags & kInt64) {
                        value = CRT_output_get_int64_arg(&args);
                    } else if (flags & kShort) {
                        const u32 got = CRT_output_get_int_arg(&args);
                        value = (flags & kSigned) ? static_cast<u64>(static_cast<s64>(static_cast<s16>(got))) : static_cast<u16>(got);
                    } else {
                        const u32 got = CRT_output_get_int_arg(&args);
                        value = (flags & kSigned) ? static_cast<u64>(static_cast<s64>(static_cast<s32>(got))) : got;
                    }
                    if ((flags & kSigned) && static_cast<s64>(value) < 0) {
                        value = 0 - value;
                        flags |= kNegative;
                    }
                    if (!(flags & kInt64)) value &= 0xffffffff;
                    s32 digits = precision;
                    if (digits < 0) {
                        digits = 1;
                    } else {
                        flags &= ~static_cast<u32>(kZeros);
                    }
                    if (!value) prefix_length = 0;
                    char *at = buffer + 0x1ff;
                    while (digits-- > 0 || value) {
                        s32 digit = static_cast<s32>(CRT_aullrem(value, radix)) + 0x30;
                        value = CRT_aulldiv(value, radix);
                        if (digit > 0x39) digit += hex_add;
                        *at-- = static_cast<char>(digit);
                    }
                    length = static_cast<s32>(buffer + 0x1ff - at);
                    text = at + 1;
                    if ((flags & kForceOctal) && (!length || *text != '0')) {
                        *--text = '0';
                        ++length;
                    }
                }
                if (no_output) break;
                if (flags & kSigned) {
                    if (flags & kNegative) {
                        prefix[0] = '-';
                        prefix_length = 1;
                    } else if (flags & kPlus) {
                        prefix[0] = '+';
                        prefix_length = 1;
                    } else if (flags & kSpace) {
                        prefix[0] = ' ';
                        prefix_length = 1;
                    }
                }
                const s32 padding = width - prefix_length - length;
                if (!(flags & (kLeft | kZeros))) CRT_woutput_write_multi(' ', padding, file, &written);
                CRT_woutput_write_string(prefix, prefix_length, file, &written);
                if ((flags & kZeros) && !(flags & kLeft)) CRT_woutput_write_multi('0', padding, file, &written);
                if (!wide_text && length > 0) {
                    const char *at = text;
                    s32 left = length - 1;
                    do {
                        const s32 count = CRT_mbtowc(reinterpret_cast<u16 *>(&wchar), at, static_cast<u32>(CRT_mb_cur_max));
                        if (count <= 0) break;
                        CRT_woutput_write_char(wchar, file, &written);
                        at += count;
                    } while (left-- > 0);
                } else {
                    CRT_woutput_write_string(reinterpret_cast<const u16 *>(text), length, file, &written);
                }
                if (flags & kLeft) CRT_woutput_write_multi(' ', padding, file, &written);
                break;
            }
            default:
                break;
        }
        ch = *format++;
    }
    return written;
}

s32 __cdecl CRT_sprintf(char *to, const char *format, ...) {
    CrtFile file;
    file.base = to;
    file.ptr = to;
    file.flags = 0x42;
    file.count = 0x7fffffff;
    const s32 result = CRT_output(&file, format, reinterpret_cast<char *>(&format + 1));
    if (--file.count >= 0) {
        *file.ptr = 0;
    } else {
        CrtInner().flsbuf(0, &file);
    }
    return result;
}

u64 __stdcall CRT_aulldiv(u64 value, u64 divisor) { return value / divisor; }

u64 __stdcall CRT_aullrem(u64 value, u64 divisor) { return value % divisor; }
