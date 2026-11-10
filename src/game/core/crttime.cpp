#include "crttime.h"

#include <cstdint>
#include <cstring>

#include "crt.h"
#include "crtenv.h"

namespace {

template <typename F> F import_slot(u32 slot) { return *reinterpret_cast<F *>((uintptr_t)slot); }

u32 &alternate_form() { return CRT_alternate_form; }
u32 &no_lead_zeros() { return CRT_no_lead_zeros; }

// The struct tm's fields, in dwords.
enum { kSecond, kMinute, kHour, kDay, kMonth, kYear, kWeekday, kYearday, kDaylight };

bool lead_byte(char c) { return CRT_pctype[static_cast<u8>(c)] & 0x8000; }

char *const *current_names() { return *reinterpret_cast<char *const *const *>((uintptr_t)0x00781f8c); }

u32 text_size(const char *text) { return static_cast<u32>(std::strlen(text)); }

char *copy_field(char *to, const char *text) {
    *to++ = ':';
    const u32 size = text_size(text);
    std::memcpy(to, text, size + 1);
    return to + size;
}

// ":short:long" `count` times; the short names from `shorts` in the table, the long ones `longs` after.
char *name_pairs(u32 shorts, u32 longs, u32 count) {
    char *const *const names = current_names();
    u32 size = 0;
    for (u32 at = 0; at < count; ++at) size += text_size(names[shorts + at]) + text_size(names[longs + at]) + 2;
    char *const block = static_cast<char *>(CRT_malloc(size + 1));
    if (!block) return nullptr;
    char *to = block;
    for (u32 at = 0; at < count; ++at) {
        to = copy_field(to, names[shorts + at]);
        to = copy_field(to, names[longs + at]);
    }
    *to = 0;
    return block;
}

const s32 *day_table(uintptr_t at) { return reinterpret_cast<const s32 *>(at); }

}  // namespace

extern "C" {

void __cdecl CRT_store_str(const char *text, char **out, u32 *left) {
    if (!*left) return;
    while (*text) {
        *(*out)++ = *text++;
        if (!--*left) return;
    }
}

void __cdecl CRT_store_number(s32 number, char **out, u32 *left) {
    char *to = *out;
    if (*left > 1) {
        do {
            *to++ = static_cast<char>(number % 10 + '0');
            --*left;
            number /= 10;
        } while (number > 0 && *left > 1);
    }
    char *low = *out;
    *out = to;
    char *high = to - 1;
    do {
        const char a = *low;
        const char b = *high;
        *high-- = a;
        *low++ = b;
    } while (low < high);
}

void __cdecl CRT_store_num(s32 number, u32 digits, char **out, u32 *left) {
    if (no_lead_zeros()) {
        CRT_store_number(number, out, left);
        return;
    }
    if (digits >= *left) {
        *left = 0;
        return;
    }
    for (u32 at = digits; at; --at) {
        (*out)[at - 1] = static_cast<char>(number % 10 + '0');
        number /= 10;
    }
    *out += digits;
    *left -= digits;
}

void __cdecl CRT_expandtime(char what, const s32 *when, char **out, u32 *left, char *const *names) {
    const auto number = [&](s32 value, u32 digits) {
        no_lead_zeros() = alternate_form();
        CRT_store_num(value, digits, out, left);
    };
    const auto date_format = [&] {
        if (alternate_form()) {
            alternate_form() = 0;
            return names[0xa4 / 4];
        }
        return names[0xa0 / 4];
    };
    const auto week = [&](s32 first) {
        const s32 day = when[kYearday];
        if (day < first) return 0;
        return day / 7 + (day % 7 >= first ? 1 : 0);
    };
    switch (what) {
    case '%':
        *(*out)++ = '%';
        --*left;
        return;
    case 'a':
        CRT_store_str(names[when[kWeekday]], out, left);
        return;
    case 'A':
        CRT_store_str(names[when[kWeekday] + 7], out, left);
        return;
    case 'b':
        CRT_store_str(names[when[kMonth] + 14], out, left);
        return;
    case 'B':
        CRT_store_str(names[when[kMonth] + 26], out, left);
        return;
    case 'c':
        CRT_store_winword(date_format(), when, out, left, names);
        if (!*left) return;
        *(*out)++ = ' ';
        --*left;
        CRT_store_winword(names[0xa8 / 4], when, out, left, names);
        return;
    case 'd':
        number(when[kDay], 2);
        return;
    case 'H':
        number(when[kHour], 2);
        return;
    case 'I': {
        const s32 hour = when[kHour] % 12;
        number(hour ? hour : 12, 2);
        return;
    }
    case 'j':
        number(when[kYearday] + 1, 3);
        return;
    case 'm':
        number(when[kMonth] + 1, 2);
        return;
    case 'M':
        number(when[kMinute], 2);
        return;
    case 'p':
        CRT_store_str(names[when[kHour] > 0xb ? 0x9c / 4 : 0x98 / 4], out, left);
        return;
    case 'S':
        number(when[kSecond], 2);
        return;
    case 'U':
        number(week(when[kWeekday]), 2);
        return;
    case 'w':
        number(when[kWeekday], 1);
        return;
    case 'W':
        number(week(when[kWeekday] ? when[kWeekday] - 1 : 6), 2);
        return;
    case 'x':
        CRT_store_winword(date_format(), when, out, left, names);
        return;
    case 'X':
        alternate_form() = 0;
        CRT_store_winword(names[0xa8 / 4], when, out, left, names);
        return;
    case 'y':
        number(when[kYear] % 100, 2);
        return;
    case 'Y':
        number((when[kYear] / 100 + 19) * 100 + when[kYear] % 100, 4);
        return;
    case 'z':
    case 'Z':
        CRT_tzset_once();
        CRT_store_str(CRT_tzname[when[kDaylight] != 0], out, left);
        return;
    default:
        return;
    }
}

void __cdecl CRT_store_winword(const char *format, const s32 *when, char **out, u32 *left, char *const *names) {
    while (*format && *left) {
        no_lead_zeros() = 0;
        const char c = *format;
        const char *next = format + 1;
        u32 count = 1;
        while (*next == c) {
            ++next;
            ++count;
        }
        char what = 0;
        const auto choose = [&](char short_form, char three, char four) {
            if (count == 1) no_lead_zeros() = 1;
            what = count <= 2 ? short_form : count == 3 ? three : count == 4 ? four : 0;
        };
        switch (c) {
        case '\'':
            format += count;
            if (count & 1) {
                while (*format && *left) {
                    if (*format == '\'') {
                        ++format;
                        break;
                    }
                    if (lead_byte(*format)) {
                        *(*out)++ = *format++;
                        --*left;
                    }
                    *(*out)++ = *format++;
                    --*left;
                }
            }
            continue;
        case 'A':
        case 'a':
            what = 'p';
            if (!CRT_stricmp(format, "am/pm"))
                next = format + 5;
            else if (!CRT_stricmp(format, "a/p"))
                next = format + 3;
            break;
        case 'M':
            choose('m', 'b', 'B');
            break;
        case 'd':
            choose('d', 'a', 'A');
            break;
        case 'H':
            choose('H', 0, 0);
            break;
        case 'h':
            choose('I', 0, 0);
            break;
        case 'm':
            choose('M', 0, 0);
            break;
        case 's':
            choose('S', 0, 0);
            break;
        case 'y':
            what = count == 2 ? 'y' : count == 4 ? 'Y' : 0;
            break;
        default:
            break;
        }
        if (what) {
            CRT_expandtime(what, when, out, left, names);
            format = next;
            continue;
        }
        if (lead_byte(*format)) {
            *(*out)++ = *format++;
            --*left;
        }
        *(*out)++ = *format++;
        --*left;
    }
}

char *__cdecl CRT_Getdays() { return name_pairs(0, 7, 7); }

char *__cdecl CRT_Getmonths() { return name_pairs(14, 26, 12); }

void __cdecl CRT_cvtdate(s32 transition, s32 type, s32 year, s32 month, s32 week, s32 weekday, s32 day, s32 hour, s32 minute, s32 second, s32 millisecond) {
    const bool leap = !(year & 3);
    const s32 *const firsts = day_table(leap ? 0x00781514 : 0x0078154c);
    s32 yearday;
    if (type == 1) {
        const s32 start = firsts[month] + 1;
        const s32 first = (((year - 1) >> 2) + year * 365 + start - 25563) % 7;
        const s32 offset = week * 7 - first + weekday;
        yearday = first < weekday ? start + offset - 7 : start + offset;
        if (week == 5 && yearday > firsts[month + 1]) yearday -= 7;
    } else {
        yearday = firsts[month] + day;
    }
    const s32 seconds = (hour * 60 + minute) * 60 + second;
    if (transition == 1) {
        CRT_dststart.yearday = yearday;
        CRT_dststart.year = year;
        CRT_dststart.ms = seconds * 1000 + millisecond;
        return;
    }
    CRT_dstend.yearday = yearday;
    s32 time = (seconds + CRT_dstbias) * 1000 + millisecond;
    if (time < 0)
        time += 0x5265bff;
    else if (time >= 0x5265c00)
        time -= 0x5265bff;
    CRT_dstend.ms = time;
    CRT_dstend.year = year;
}

s32 __cdecl CRT_isindst_lk(const s32 *when) {
    enum { kSecond, kMinute, kHour, kYear = 5, kYearday = 7 };
    const auto global = [](u32 address) { return *reinterpret_cast<const s32 *>((uintptr_t)address); };
    if (!CRT_daylight) return 0;
    if (when[kYear] != CRT_dststart.year || when[kYear] != CRT_dstend.year) {
        if (static_cast<s32>(CRT_tzapiused)) {
            const auto transition = [&](s32 first, const u16 *d) {   // (a SYSTEMTIME)
                if (d[0]) CRT_cvtdate(first, 0, when[kYear], d[1], 0, 0, d[3], d[4], d[5], d[6], d[7]);
                else CRT_cvtdate(first, 1, when[kYear], d[1], d[3], d[2], 0, d[4], d[5], d[6], d[7]);
            };
            transition(1, CRT_tzinfo.daylight_date);
            transition(0, CRT_tzinfo.standard_date);
        } else {
            CRT_cvtdate(1, 1, when[kYear], 4, 1, 0, 0, 2, 0, 0, 0);
            CRT_cvtdate(0, 1, when[kYear], 10, 5, 0, 0, 2, 0, 0, 0);
        }
    }
    const s32 start = CRT_dststart.yearday;
    const s32 end = CRT_dstend.yearday;
    const s32 day = when[kYearday];
    if (start < end) {
        if (day < start || day > end) return 0;
        if (day > start && day < end) return 1;
    } else {
        if (day < end || day > start) return 1;
        if (day > end && day < start) return 0;
    }
    const s32 ms = ((when[kHour] * 60 + when[kMinute]) * 60 + when[kSecond]) * 1000;
    if (day == start) return ms >= CRT_dststart.ms;
    return ms < CRT_dstend.ms;
}

s32 *__cdecl CRT_gmtime(const s32 *when) {
    enum { kFourYears = 0x7861f80, kYearSeconds = 0x1e13380, kLeapYear = 0x1e28500, kDaySeconds = 86400 };
    s32 left = *when;
    u8 *const thread = CrtInner().getptd();
    if (left < 0) return nullptr;
    s32 *&own = *reinterpret_cast<s32 **>(thread + 0x40);
    if (!own) own = static_cast<s32 *>(CRT_malloc(0x24));
    s32 *const out = own ? own : CRT_tm_buffer;
    const s32 fours = left / kFourYears;
    left -= fours * kFourYears;
    s32 year = fours * 4 + 70;
    bool leap = false;
    if (left >= kYearSeconds) {
        left -= kYearSeconds;
        ++year;
        if (left >= kYearSeconds) {
            left -= kYearSeconds;
            ++year;
            if (left >= kLeapYear) {
                ++year;
                left -= kLeapYear;
            } else {
                leap = true;
            }
        }
    }
    out[kYear] = year;
    const s32 yearday = left / kDaySeconds;
    out[kYearday] = yearday;
    left -= yearday * kDaySeconds;
    const s32 *const lasts = day_table(leap ? 0x00781518 : 0x00781550);
    s32 month = 1;
    while (lasts[month] < yearday) ++month;
    --month;
    out[kMonth] = month;
    out[kDay] = yearday - lasts[month];
    out[kDaylight] = 0;
    out[kWeekday] = (*when / kDaySeconds + 4) % 7;
    out[kHour] = left / 3600;
    left -= out[kHour] * 3600;
    out[kMinute] = left / 60;
    out[kSecond] = left - out[kMinute] * 60;
    return out;
}

char **__cdecl CRT_Gettnames() {
    char *const *const from = current_names();
    enum { kPointers = 0xac / 4 };
    u32 size = 0xac;
    for (u32 at = 0; at < kPointers; ++at) size += static_cast<u32>(std::strlen(from[at])) + 1;
    char **const block = static_cast<char **>(CRT_malloc(size));
    if (!block) return nullptr;
    std::memcpy(block, from, 0xac);
    char *to = reinterpret_cast<char *>(block) + 0xac;
    const auto copy = [&](u32 at) {
        block[at] = to;
        const u32 length = static_cast<u32>(std::strlen(from[at])) + 1;
        std::memcpy(to, from[at], length);
        to += length;
    };
    for (u32 at = 0; at < 7; ++at) {
        copy(at);
        copy(at + 7);
    }
    for (u32 at = 0; at < 12; ++at) {
        copy(at + 14);
        copy(at + 26);
    }
    for (u32 at = 38; at < kPointers - 1; ++at) copy(at);
    block[kPointers - 1] = to;   // (the last text is counted in the size and given its place, but never copied)
    return block;
}

void __cdecl CRT_tzset_once() {
    u32 &done = CRT_tzset_done_fb1510;
    if (done) return;
    CrtInner().lock(0xb);
    if (!done) {
        CrtInner().tzset();
        ++done;
    }
    CrtInner().unlock(0xb);
}

u32 __cdecl CRT_Strftime(char *out, u32 size, const char *format, const s32 *when, char *const *names) {
    if (!names) names = current_names();
    u32 left = size;
    u32 *const readers = &CRT_unguarded_readlc_active;
    import_slot<s32(__stdcall *)(u32 *)>(0x006fd0c0)(readers);    // InterlockedIncrement
    const bool locked = CRT_setlc_active != 0;
    if (locked) {
        import_slot<s32(__stdcall *)(u32 *)>(0x006fd12c)(readers);    // InterlockedDecrement
        CrtInner().lock(0x13);
    }
    while (left && *format) {
        if (*format == '%') {
            ++format;
            alternate_form() = 0;
            if (*format == '#') {
                alternate_form() = 1;
                ++format;
            }
            CRT_expandtime(*format, when, &out, &left, names);
            ++format;
            continue;
        }
        if (lead_byte(*format) && left > 1) {
            *out++ = *format++;
            --left;
        }
        *out++ = *format++;
        --left;
    }
    if (locked)
        CrtInner().unlock(0x13);
    else
        import_slot<s32(__stdcall *)(u32 *)>(0x006fd12c)(readers);
    if (!left) return 0;
    *out = 0;
    return size - left;
}

s32 __cdecl CRT_isindst(const s32 *when) {
    CrtInner().lock(0xb);
    const s32 result = CRT_isindst_lk(when);
    CrtInner().unlock(0xb);
    return result;
}

static s32 global(u32 address) { return *reinterpret_cast<const s32 *>((uintptr_t)address); }

s32 *__cdecl CRT_localtime(const s32 *timer) {
    if (*timer < 0) return nullptr;
    CRT_tzset_once();
    s32 when = *timer;
    if (when > 0x3f480 && when < 0x7ffc0b7f) {
        when -= CRT_timezone;
        s32 *got = CRT_gmtime(&when);
        if (!CRT_daylight || !CRT_isindst(got)) return got;
        when -= CRT_dstbias;
        got = CRT_gmtime(&when);
        got[kDaylight] = 1;
        return got;
    }
    s32 *const got = CRT_gmtime(timer);
    s32 left = got[kSecond];
    if (CRT_isindst(got)) left -= CRT_dstbias;
    left -= CRT_timezone;
    got[kSecond] = left % 60;
    if (got[kSecond] < 0) {
        got[kSecond] += 60;
        left -= 60;
    }
    left = left / 60 + got[kMinute];
    got[kMinute] = left % 60;
    if (got[kMinute] < 0) {
        got[kMinute] += 60;
        left -= 60;
    }
    left = left / 60 + got[kHour];
    got[kHour] = left % 24;
    if (got[kHour] < 0) {
        got[kHour] += 24;
        left -= 24;
    }
    const s32 days = left / 24;
    if (days > 0) {
        got[kWeekday] = (days + got[kWeekday]) % 7;
        got[kDay] += days;
        got[kYearday] += days;
    } else if (days < 0) {
        got[kWeekday] = (days + got[kWeekday] + 7) % 7;
        got[kDay] += days;
        if (got[kDay] <= 0) {
            got[kDay] += 0x1f;
            got[kYearday] = 0x16c;
            got[kMonth] = 0xb;
            --got[kYear];
        } else {
            got[kYearday] += days;
        }
    }
    return got;
}

u32 __cdecl CRT_loctotime_t(s32 year, s32 month, s32 day, s32 hour, s32 minute, s32 second, s32 dst) {
    const s32 years = year - 0x76c;
    if (years < 0x46 || years > 0x8a) return 0xffffffff;
    s32 yearday = reinterpret_cast<const s32 *>((uintptr_t)0x0078154c)[month] + day;
    if (!(years & 3) && month > 2) ++yearday;
    CRT_tzset_once();
    s32 when[9];   // (sic: _isindst reads tm_min and tm_sec, left unset, on a change day)
    when[kYearday] = yearday;
    when[kYear] = years;
    when[kHour] = hour;
    when[kMonth] = month - 1;
    const u32 days = static_cast<u32>(yearday) + 365u * static_cast<u32>(years) + static_cast<u32>((years - 1) >> 2);
    const u32 hours = days * 24 + static_cast<u32>(hour);
    const u32 minutes = hours * 60 + static_cast<u32>(minute);
    u32 total = minutes * 60 + static_cast<u32>(second) + static_cast<u32>(CRT_timezone) + 0x7c558180u;
    if (dst == 1 || (dst == -1 && CRT_daylight && CRT_isindst(when))) total += static_cast<u32>(CRT_dstbias);
    return total;
}

u32 __cdecl CRT_time(u32 *out) {
    u16 local[8], system[8];   // (SYSTEMTIME: year, month, weekday, day, hour, minute, second, ms)
    import_slot<void(__stdcall *)(u16 *)>(0x006fd164)(local);    // GetLocalTime
    import_slot<void(__stdcall *)(u16 *)>(0x006fd220)(system);   // GetSystemTime
    u16 *const last = CRT_time_last_fb1340;   // (the system time the answer below was for)
    s32 dst;
    if (system[5] == last[5] && system[4] == last[4] && system[3] == last[3] && system[1] == last[1] && system[0] == last[0]) {
        dst = CRT_time_dst_fb1338;
    } else {
        u8 zone[0xac];
        const u32 kind = import_slot<u32(__stdcall *)(u8 *)>(0x006fd21c)(zone);   // GetTimeZoneInformation
        if (kind == 0xffffffffu) dst = -1;
        else dst = kind == 2 && *reinterpret_cast<const u16 *>(zone + 0x9a) && *reinterpret_cast<const s32 *>(zone + 0xa8) ? 1 : 0;
        std::memcpy(last, system, 0x10);
        CRT_time_dst_fb1338 = dst;
    }
    const u32 when = CRT_loctotime_t(local[0], local[1], local[3], local[4], local[5], local[6], dst);
    if (out) *out = when;
    return when;
}

}  // extern "C"
