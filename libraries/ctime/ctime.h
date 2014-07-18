struct ctime_t {
    uint8_t second, minute, hour, dayOfWeek, dayOfMonth, month;
    uint16_t year;

    bool operator>(const ctime_t & rhs) const
    {
        return (year > rhs.year) ||
               (month > rhs.month) ||
               (dayOfMonth > rhs.dayOfMonth) ||
               (hour > rhs.hour) ||
               (minute > rhs.minute) ||
               (second > rhs.second);
    }

    bool operator<(const ctime_t & rhs) const
    {
        return (year < rhs.year) ||
               (month < rhs.month) ||
               (dayOfMonth < rhs.dayOfMonth) ||
               (hour < rhs.hour) ||
               (minute < rhs.minute) ||
               (second < rhs.second);
    }
};

const uint8_t daysInMonth [] PROGMEM = { 31,28,31,30,31,30,31,31,30,31,30,31 };

// number of days since 2001/01/01
uint16_t date2days(uint16_t y, uint8_t m, uint8_t d) {
    if (y >= 2000)
        y -= 2000;
    uint16_t days = d;
    for (uint8_t i = 1; i < m; ++i)
        days += pgm_read_byte(daysInMonth + i - 1);
    if (m > 2 && y % 4 == 0)
        ++days;
    return days + 365 * y + (y + 3) / 4 - 1;
}

long time2long(uint16_t days, uint8_t h, uint8_t m, uint8_t s) {
    return ((days * 24L + h) * 60 + m) * 60 + s;
}

uint32_t unixTime(ctime_t t) {
    uint32_t secondsUnix;
    uint16_t days = date2days(t.year, t.month, t.dayOfMonth);
    secondsUnix = time2long(days, t.hour, t.minute, t.second);
    secondsUnix += 946684800;  // seconds from 1970 to 2000

    return secondsUnix;
}

ctime_t makeTime (uint32_t t) {
    t -= 946684800;    // bring to 2000 timestamp from 1970

    uint8_t second = t % 60;
    t /= 60;
    uint8_t minute = t % 60;
    t /= 60;
    uint8_t hour = t % 24;
    uint16_t days = t / 24;

    uint8_t leap;
    uint8_t year;
    for (year = 0; ; ++year) {
        leap = year % 4 == 0;
        if (days < 365 + leap)
            break;
        days -= 365 + leap;
    }
    year += 2000;

    uint8_t month;
    for (month = 1; ; ++month) {
        uint8_t daysPerMonth = pgm_read_byte(daysInMonth + month - 1);
        if (leap && month == 2)
            ++daysPerMonth;
        if (days < daysPerMonth)
            break;
        days -= daysPerMonth;
    }
    days += 1;

    ctime_t dateTime;
    dateTime.second = second;
    dateTime.minute = minute;
    dateTime.hour = hour;
    dateTime.dayOfMonth = days;
    dateTime.month = month;
    dateTime.year = year;

    return dateTime;
}

